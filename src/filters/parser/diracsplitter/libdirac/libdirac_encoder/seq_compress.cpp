/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: seq_compress.cpp,v 1.43 2007/09/26 12:18:43 asuraparaju Exp $ $Name: Dirac_0_8_0 $
*
* Version: MPL 1.1/GPL 2.0/LGPL 2.1
*
* The contents of this file are subject to the Mozilla Public License
* Version 1.1 (the "License"); you may not use this file except in compliance
* with the License. You may obtain a copy of the License at
* http://www.mozilla.org/MPL/
*
* Software distributed under the License is distributed on an "AS IS" basis,
* WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for
* the specific language governing rights and limitations under the License.
*
* The Original Code is BBC Research and Development code.
*
* The Initial Developer of the Original Code is the British Broadcasting
* Corporation.
* Portions created by the Initial Developer are Copyright (C) 2004.
* All Rights Reserved.
*
* Contributor(s): Thomas Davies (Original Author),
*                 Scott R Ladd,
*                 Anuradha Suraparaju,
*                 Andrew Kennedy
*                 Myo Tun (Brunel University, myo.tun@brunel.ac.uk)
*
* Alternatively, the contents of this file may be used under the terms of
* the GNU General Public License Version 2 (the "GPL"), or the GNU Lesser
* Public License Version 2.1 (the "LGPL"), in which case the provisions of
* the GPL or the LGPL are applicable instead of those above. If you wish to
* allow use of your version of this file only under the terms of the either
* the GPL or LGPL and not to allow others to use your version of this file
* under the MPL, indicate your decision by deleting the provisions above
* and replace them with the notice and other provisions required by the GPL
* or LGPL. If you do not delete the provisions above, a recipient may use
* your version of this file under the terms of any one of the MPL, the GPL
* or the LGPL.
* ***** END LICENSE BLOCK ***** */

#include <libdirac_encoder/seq_compress.h>

using namespace dirac;

SequenceCompressor::SequenceCompressor( StreamPicInput* pin ,
                                        EncoderParams& encp,
                                        DiracByteStream& dirac_byte_stream):
    m_all_done(false),
    m_just_finished(true),
    m_srcparams(pin->GetSourceParams()),
    m_encparams(encp),
    m_pic_in(pin),
    m_current_display_fnum(-1),
    m_current_code_fnum(0),
    m_show_fnum(-1),m_last_frame_read(-1),
    m_delay(1),
    m_qmonitor( m_encparams ),
    m_fcoder( m_encparams ),
    m_dirac_byte_stream(dirac_byte_stream)
{
    // Set up the compression of the sequence

    //TBD: put into the constructor for EncoderParams
    m_encparams.SetEntropyFactors( new EntropyCorrector(4) );

    //We have to set up the block parameters and file padding. This needs to take into
    //account both blocks for motion compensation and also wavelet transforms

    //Amount of horizontal padding for Y,U and V components
    int xpad_luma,xpad_chroma;

    //Amount of vertical padding for Y,U and V components
    int ypad_luma,ypad_chroma;

    //First, we need to have sufficient padding to take account of the blocksizes.
    //It's sufficient to check for chroma


    int xl_chroma = m_encparams.OrigChromaXl();
    int yl_chroma = m_encparams.OrigChromaYl();
     xpad_chroma = ypad_chroma = 0;
    // The pic dimensions must be a multiple of 2^(transform depth).
    int tx_mul = 1<<m_encparams.TransformDepth();

    if ( xl_chroma%tx_mul != 0 )
        xpad_chroma = ( (xl_chroma/tx_mul)+1 ) *tx_mul - xl_chroma;
    if ( yl_chroma%tx_mul != 0 )
        ypad_chroma = ( (yl_chroma/tx_mul)+1 ) * tx_mul - yl_chroma;

    int xpad_chroma_len = xl_chroma+xpad_chroma;
    int ypad_chroma_len = yl_chroma+ypad_chroma;

    // Make sure we have enough macroblocks to cover the pictures
    m_encparams.SetXNumMB( xl_chroma/m_encparams.ChromaBParams(0).Xbsep() );
    m_encparams.SetYNumMB( yl_chroma/m_encparams.ChromaBParams(0).Ybsep() );
    if ( m_encparams.XNumMB() * m_encparams.ChromaBParams(0).Xbsep() < xl_chroma )
    {
        m_encparams.SetXNumMB( m_encparams.XNumMB() + 1 );
    }

    if ( m_encparams.YNumMB() * m_encparams.ChromaBParams(0).Ybsep() < yl_chroma )
    {
        m_encparams.SetYNumMB( m_encparams.YNumMB() + 1 );
    }


     xpad_luma = ypad_luma = 0;
    int xpad_len = m_encparams.OrigXl();
    int ypad_len = m_encparams.OrigYl();

    // The pic dimensions must be a multiple of 2^(transform depth).
    if ( xpad_len%tx_mul != 0 )
        xpad_luma = ( (xpad_len/tx_mul)+1 ) *tx_mul - xpad_len;
    if ( ypad_len%tx_mul != 0 )
        ypad_luma = ( (ypad_len/tx_mul)+1 ) * tx_mul - ypad_len;

    xpad_len += xpad_luma;
    ypad_len += ypad_luma;

    // NOTE: Do we need to recalculate the number of Macro blocks!!!

    // Note that we do not have an integral number of macroblocks in a picture
    // So it is possible that part of a macro-block and some blocks can fall
    // of the edge of the padded picture. We need to take this into
    // consideration while doing Motion Estimation
    m_encparams.SetXNumBlocks( 4 * m_encparams.XNumMB() );
    m_encparams.SetYNumBlocks( 4 * m_encparams.YNumMB() );

    // Set up the frame buffers with the PADDED picture sizes
    m_fbuffer = new FrameBuffer( m_srcparams.CFormat() ,
                                 m_encparams.NumL1() , m_encparams.L1Sep() ,
                                 m_encparams.OrigXl(), m_encparams.OrigYl(),
                                 xpad_len , ypad_len,
                                 xpad_chroma_len, ypad_chroma_len,
                                 m_encparams.LumaDepth(),
                                 m_encparams.ChromaDepth(),
                                 m_encparams.Interlace());

    // Retain the original frame dimensions for the Motion estimation
    // buffer
    m_mebuffer = new FrameBuffer( m_srcparams.CFormat() ,
                                    m_encparams.NumL1() , m_encparams.L1Sep() ,
                                    m_encparams.OrigXl(), m_encparams.OrigYl(),
                                    m_encparams.OrigXl(), m_encparams.OrigYl(),
                                    xl_chroma, yl_chroma,
                                    m_encparams.LumaDepth(),
                                    m_encparams.ChromaDepth(),
                                    m_encparams.Interlace());

    // Set up a rate controller if rate control being used
    if (m_encparams.TargetRate() != 0)
        m_ratecontrol = new RateController(m_encparams.TargetRate(),
                                           m_pic_in->GetSourceParams(), encp);
}

SequenceCompressor::~SequenceCompressor()
{

    if ( m_encparams.Verbose())
        MakeSequenceReport();

    //TBD: put into the destructor for EncoderParams
    delete &m_encparams.EntropyFactors();

    delete m_fbuffer;
    delete m_mebuffer;

    if (m_encparams.TargetRate()!=0)
        delete m_ratecontrol;
}

Frame& SequenceCompressor::CompressNextFrame()
{

    // This function codes the next frame in coding order and returns the next frame in display order
    // In general these will differ, and because of re-ordering there is a m_delay which needs to be imposed.
    // This creates problems at the start and at the end of the sequence which must be dealt with.
    // At the start we just keep outputting frame 0. At the end you will need to loop for longer to get all
    // the frames out. It's up to the calling function to do something with the decoded frames as they
    // come out - write them to screen or to file, or whatever. TJD 13Feb04.

    // current_fnum is the number of the current frame being coded in display order
    // m_current_code_fnum is the number of the current frame in coding order. This function increments
    // m_current_code_fnum by 1 each time and works out what the number is in display order.
    // m_show_fnum is the index of the frame number that can be shown when current_fnum has been coded.
    // Var m_delay is the m_delay caused by reordering (as distinct from buffering)

    TESTM (m_last_frame_read >= 0, "Data loaded before calling CompressNextFrame");
    m_current_display_fnum = CodedToDisplay( m_current_code_fnum );

    m_show_fnum = std::max( m_current_code_fnum - m_delay , 0 );

    // Flag saying we're ready to encode
    bool can_encode = false;

    if (m_last_frame_read >= m_current_display_fnum )
        can_encode = true;

    if ( can_encode )
    {   // We haven't coded everything, so compress the next frame

        if ( m_encparams.Verbose() )
        {
            if (m_encparams.Interlace())
                std::cout<<std::endl<<std::endl<<"Compressing field "<<m_current_code_fnum<<", ";
            else
                std::cout<<std::endl<<std::endl<<"Compressing frame "<<m_current_code_fnum<<", ";
            std::cout<<m_current_display_fnum<<" in display order";
        }


        // stream access-unit data if first frame in unit
        if(IsNewAccessUnit())
        {
            // The access unit frame number must be equal to the frame
            // number in display order of the I_Frame that follows it and
            // not the coded order.
            AccessUnitByteIO *p_accessunit_byteio = new AccessUnitByteIO
                                        (
                                            m_pic_in->GetSourceParams(),
                                            m_encparams
                                        );
            p_accessunit_byteio->Output();

            // add the unit to the byte stream
            m_dirac_byte_stream.AddAccessUnit(p_accessunit_byteio);

        }

        // Compress the frame//
        ///////////////////////

        m_fbuffer->SetRetiredList( m_show_fnum, m_current_display_fnum );


        Frame& my_frame = m_fbuffer->GetFrame( m_current_display_fnum );

        FrameParams& fparams = my_frame.GetFparams();

        // Do motion estimation using the original (not reconstructed) data
        if (m_encparams.Verbose() && my_frame.GetFparams().FSort().IsInter())
        {
            std::cout<<std::endl<<"References "
                     << (m_encparams.Interlace() ? "field " : "frame ")
                     << fparams.Refs()[0];
            if (fparams.Refs().size() > 1)
            {
                std::cout<<" and "<< fparams.Refs()[1];
            }
        }
        bool is_a_cut( false );
        if ( my_frame.GetFparams().FSort().IsInter() )
        {
           is_a_cut = m_fcoder.MotionEstimate(  *m_mebuffer,
                                                m_current_display_fnum );
            if ( is_a_cut )
            {
                // Set the frame type to intra
                if (my_frame.GetFparams().FSort().IsRef())
                    my_frame.SetFrameSort (FrameSort::IntraRefFrameSort());
                else
                    my_frame.SetFrameSort (FrameSort::IntraNonRefFrameSort());

                if ( m_encparams.Verbose() )
                    std::cout<<std::endl<<"Cut detected and I-frame inserted!";
            }
        }


        // Now code the residual data
        if (m_encparams.TargetRate() == 0)
        {
            FrameByteIO *p_frame_byteio;
            // Coding Without using Rate Control Algorithm
            p_frame_byteio =  m_fcoder.Compress(*m_fbuffer ,
                                         m_current_display_fnum);

            // add the frame to the byte stream

            m_dirac_byte_stream.AddFrame(p_frame_byteio);
        }
        else
        {
            RateControlCompress(my_frame, is_a_cut);
        }

       // Measure the encoded frame quality
       if ( m_encparams.LocalDecode() )
       {
           const Frame &orig_frame = OriginalFrame( m_current_display_fnum );
           if (m_current_display_fnum != orig_frame.GetFparams().FrameNum())
           {
               std::cerr << "Error in frame buffer:"
                         << " Requested : " << m_current_display_fnum
                         << "  Retrieved : " << orig_frame.GetFparams().FrameNum() << std::endl;
           }
           m_qmonitor.UpdateModel(
               m_fbuffer->GetFrame( m_current_display_fnum ) ,
               OriginalFrame(m_current_display_fnum) );
       }
        // Increment our position
        m_current_code_fnum++;

        CleanBuffers();

    }

    // Return the latest frame that can be shown
    if ( m_encparams.Verbose() )
    {
           std::cout<<std::endl<<"Return " <<
                 (m_encparams.Interlace() ? "field " : "frame ")  <<
                  m_show_fnum << " in display order";
    }
    return m_fbuffer->GetFrame(m_show_fnum );
}

void SequenceCompressor::CleanBuffers()
{
    // If we're not at the beginning, clean the buffer
    if ( m_current_code_fnum != 0 )
    {
        m_fbuffer->Clean( m_show_fnum, m_current_display_fnum );
        m_mebuffer->Clean( m_show_fnum, m_current_display_fnum );
    }
}

const Frame *SequenceCompressor::GetFrameEncoded()
{
    if (m_current_display_fnum >= 0)
        return &m_fbuffer->GetFrame( m_current_display_fnum );

    return 0;
}

const MEData *SequenceCompressor::GetMEData()
{
    if ( m_fcoder.IsMEDataAvail())
        return m_fcoder.GetMEData();

    return 0;
}
DiracByteStats SequenceCompressor::EndSequence()
{
    DiracByteStats seq_stats;

    if (m_just_finished)
    {
        seq_stats=m_dirac_byte_stream.EndSequence();
        m_just_finished = false;
        m_all_done = true;
    }

    return seq_stats;
}



void SequenceCompressor::MakeSequenceReport()
{
    if ( m_encparams.LocalDecode() )
        m_qmonitor.WriteLog();

    std::cout<<std::endl;

}

void SequenceCompressor::Denoise( Frame& frame )
{
    DenoiseComponent( frame.Ydata() );
    DenoiseComponent( frame.Udata() );
    DenoiseComponent( frame.Vdata() );

}

void SequenceCompressor::DenoiseComponent( PicArray& pic_data )
{
    // Do centre-weighted median denoising

    PicArray pic_copy( pic_data );

    const int centre_weight = 5;
    const int list_length = centre_weight+8;
    ValueType val_list[list_length];

    for (int j=1; j<pic_data.LastY(); ++j)
    {
        for (int i=1; i<pic_data.LastX(); ++i)
        {
            // Make the value list
            int pos=0;
            for (; pos<centre_weight-1; ++pos)
                val_list[pos] = pic_copy[j][i];

            for (int s=-1; s<=1; ++s)
            {
                for (int r=-1; r<=1; ++r)
                {
                    val_list[pos]=pic_copy[j+s][i+r];
                    pos++;
                }// r
            }// s

            pic_data[j][i] = Median( val_list, list_length );
        }// i
    }// j

}

ValueType SequenceCompressor::Median( const ValueType* val_list, const int length)
{


    OneDArray<ValueType> ordered_vals( length );

    // Place the values in order
    int pos=0;
    ordered_vals[0] = val_list[0];
    for (int i=1 ; i<length ; ++i )
    {
        for (int k=0 ; k<i ; ++k)
        {
            if (val_list[i]<ordered_vals[k])
            {
                pos=k;
                break;
            }
            else
                pos=k+1;
        }// k

        if ( pos==i)
            ordered_vals[i] = val_list[i];
        else
        {
            for (int k=i-1 ; k>=pos ; --k )
            {
                ordered_vals[k+1] = ordered_vals[k];
            }// k
            ordered_vals[pos] = val_list[i];
        }
    }// i

    // return the middle value
    if ( length%2!=0 )
        return ordered_vals[(length-1)/2];
    else
        return (ordered_vals[(length/2)-1]+ordered_vals[length/2]+1)>>1;

}

FrameSequenceCompressor::FrameSequenceCompressor(
                                  StreamPicInput* pin ,
                                  EncoderParams& encp,
                                  DiracByteStream& dirac_byte_stream):
    SequenceCompressor(pin, encp, dirac_byte_stream)
{
}

bool FrameSequenceCompressor::LoadNextFrame()
{
    m_pic_in->ReadNextFrame( *m_fbuffer, m_last_frame_read+1 );

    if ( m_pic_in->End() )
    {
        m_all_done = true;
        return false;
    }

    if ( m_encparams.Denoise() )
        Denoise(m_fbuffer->GetFrame( m_last_frame_read+1 ) );

    m_last_frame_read++;
    m_mebuffer->PushFrame( m_fbuffer->GetFrame( m_last_frame_read ) );
    return true;
}

int FrameSequenceCompressor::CodedToDisplay( const int pnum )
{
    int div;

    if (m_encparams.L1Sep()>0)
    {
        // We have L1 and L2 frames
        if (pnum==0)
            return 0;
        else if ((pnum-1)% m_encparams.L1Sep()==0)
        {//we have L1 or subsequent I frames
            div=(pnum-1)/m_encparams.L1Sep();
            return pnum+m_encparams.L1Sep()-1;
        }
        else//we have L2 frames
            return pnum-1;
    }
    else
    {//we just have I-frames, so no re-ordering
        return pnum;
    }
}

bool FrameSequenceCompressor::IsNewAccessUnit()
{
    return (m_current_display_fnum % m_encparams.GOPLength()==0);
}

void FrameSequenceCompressor::RateControlCompress(Frame& my_frame, bool is_a_cut)
{
    if (m_encparams.TargetRate() == 0)
        return;

    FrameParams& fparams = my_frame.GetFparams();
    const FrameSort& fsort = fparams.FSort();

    FrameByteIO *p_frame_byteio;

    // Coding using Rate Control Algorithm

    if ( fsort.IsIntra() &&
         m_current_display_fnum != 0 &&
         m_encparams.NumL1() != 0)
    {
        // Calculate the new QF for encoding the following I frames in the sequence
        // in normal coding

        if ( is_a_cut )
        {
            // Recompute the QF based on long-term history since recent history is bunk
            m_ratecontrol->SetCutFrameQualFactor();
        }
        else
            m_ratecontrol->CalcNextIntraQualFactor();
    }

    p_frame_byteio =  m_fcoder.Compress(*m_fbuffer,
                                            m_current_display_fnum);

    // Update the quality factor
    m_ratecontrol->CalcNextQualFactor(fparams, p_frame_byteio->GetSize()*8);

    // add the frame to the byte stream
    m_dirac_byte_stream.AddFrame(p_frame_byteio);
}


FieldSequenceCompressor::FieldSequenceCompressor(
                                  StreamPicInput* pin ,
                                  EncoderParams& encp,
                                  DiracByteStream& dirac_byte_stream):
    SequenceCompressor(pin, encp, dirac_byte_stream)
{
    if ( m_encparams.LocalDecode() )
    {
        m_origbuffer = new FrameBuffer(*m_mebuffer);
    }
    m_delay = 2;
}

bool FieldSequenceCompressor::LoadNextFrame()
{
    m_pic_in->ReadNextFrame( *m_fbuffer, m_last_frame_read+1 );

    if ( m_pic_in->End() )
    {
        m_all_done = true;
        return false;
    }

    ++m_last_frame_read;
    if ( m_encparams.Denoise() )
    {
        Denoise(m_fbuffer->GetFrame( m_last_frame_read ) );
        Denoise(m_fbuffer->GetFrame( m_last_frame_read+1 ) );
    }
    m_mebuffer->PushFrame( m_fbuffer->GetFrame( m_last_frame_read ) );

    Frame &field1 = m_mebuffer->GetFrame( m_last_frame_read );
    PreMotionEstmationFilter(field1.Ydata());
    PreMotionEstmationFilter(field1.Udata());
    PreMotionEstmationFilter(field1.Vdata());

    if ( m_encparams.LocalDecode() )
        m_origbuffer->PushFrame( m_fbuffer->GetFrame( m_last_frame_read ) );

    m_mebuffer->PushFrame( m_fbuffer->GetFrame( m_last_frame_read + 1 ) );

    Frame &field2 = m_mebuffer->GetFrame( m_last_frame_read + 1 );
    PreMotionEstmationFilter(field2.Ydata());
    PreMotionEstmationFilter(field2.Udata());
    PreMotionEstmationFilter(field2.Vdata());

    if ( m_encparams.LocalDecode() )
        m_origbuffer->PushFrame( m_fbuffer->GetFrame( m_last_frame_read + 1 ) );

    ++m_last_frame_read;
    return true;
}

void FieldSequenceCompressor::PreMotionEstmationFilter(PicArray& comp)
{
    //Special case for first row
    for (int i = comp.FirstX(); i <= comp.LastX(); ++i)
    {
        comp[comp.FirstY()][i] = (3*comp[comp.FirstY()][i] +
                                  comp[comp.FirstY()+1][i] +2 )>>2;
    }
    //Middle section
    for (int j = comp.FirstY()+1; j < comp.LastY(); ++j)
    {
        for (int i = comp.FirstX(); i <= comp.LastX(); ++i)
        {
            comp[j][i] = (comp[j-1][i] + 2*comp[j][i] + comp[j+1][i] + 2)>>2;
        }
    }
    //Special case for last row
    for (int i = comp.FirstX(); i <= comp.LastX(); ++i)
    {
        comp[comp.LastY()][i] = (comp[comp.LastY()-1][i] +
                                 3*comp[comp.LastY()][i] + 2)>>2;
    }
}

const Frame& FieldSequenceCompressor::OriginalFrame(int frame_num)
{
    if ( m_encparams.LocalDecode() )
        return m_origbuffer->GetFrame(frame_num);
    else
        return m_mebuffer->GetFrame(frame_num);
}

void FieldSequenceCompressor::CleanBuffers()
{
    // If we're not at the beginning, clean the buffer
    if ( m_current_code_fnum != 0 )
    {
        SequenceCompressor::CleanBuffers();
        if (m_encparams.LocalDecode())
            m_origbuffer->Clean( m_show_fnum, m_current_display_fnum );
    }
}

FieldSequenceCompressor::~FieldSequenceCompressor()
{
    if ( m_encparams.LocalDecode() )
        delete m_origbuffer;
}

int FieldSequenceCompressor::CodedToDisplay( const int pnum )
{
    // Frame the field pnum belongs to
    int fnum = pnum>>1;
    if (m_encparams.L1Sep()>0)
    {
        // We have L1 and L2 frames
        if (fnum==0)
            return pnum;
        else if ((fnum-1)% m_encparams.L1Sep()==0)
        {//we have L1 or subsequent I frames
            return (pnum+(m_encparams.L1Sep()-1)*2);
        }
        else//we have L2 frames
            return (pnum - 2);
    }
    else
    {//we just have I-frames, so no re-ordering
        return (pnum);
    }
}

bool FieldSequenceCompressor::IsNewAccessUnit( )
{
    return ((m_current_display_fnum) % (m_encparams.GOPLength()<<1)==0);
}

void FieldSequenceCompressor::RateControlCompress(Frame& my_frame, bool is_a_cut)
{
    if (m_encparams.TargetRate() == 0)
        return;

    FrameParams& fparams = my_frame.GetFparams();
    const FrameSort& fsort = fparams.FSort();

    FrameByteIO *p_frame_byteio;

    // Coding using Rate Control Algorithm

    if ( fsort.IsIntra() &&
         m_current_display_fnum > 1 &&
         m_encparams.NumL1() != 0)
    {
        // Calculate the new QF for encoding the following I frames in the sequence
        // in normal coding

        if ( is_a_cut )
        {
            // Recompute the QF based on long-term history since recent history is bunk
            m_ratecontrol->SetCutFrameQualFactor();
        }
        else if (m_current_display_fnum%2 == 0)
                m_ratecontrol->CalcNextIntraQualFactor();
    }

    p_frame_byteio =  m_fcoder.Compress(*m_fbuffer,
                                            m_current_display_fnum);

    if (m_current_display_fnum%2 == 0)
        m_field1_bytes = p_frame_byteio->GetSize();
    else
        m_field2_bytes = p_frame_byteio->GetSize();

    // Update the quality factor
    if (fparams.FrameNum()%2)
        m_ratecontrol->CalcNextQualFactor(fparams, (m_field1_bytes+m_field2_bytes)*8);

    // add the frame to the byte stream
    m_dirac_byte_stream.AddFrame(p_frame_byteio);
}

