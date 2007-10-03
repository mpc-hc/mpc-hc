/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: frame_compress.cpp,v 1.29 2007/04/11 14:18:28 tjdwave Exp $ $Name: Dirac_0_7_0 $
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
*                 Chris Bowley,
*                 Anuradha Suraparaju,
*                 Tim Borer,
*                 Andrew Kennedy
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

//Compression of frames//
/////////////////////////

#include <libdirac_encoder/frame_compress.h>
#include <libdirac_encoder/comp_compress.h>
#include <libdirac_common/mot_comp.h>
#include <libdirac_motionest/motion_estimate.h>
#include <libdirac_common/mv_codec.h>
#include <libdirac_common/dirac_assertions.h>
using namespace dirac;

#include <iostream>
#include <sstream>

FrameCompressor::FrameCompressor( EncoderParams& encp ) :
    m_encparams(encp),
    m_me_data(0),
    m_skipped(false),
    m_use_global(false),
    m_use_block_mv(true),
    m_global_pred_mode(REF1_ONLY),
    m_medata_avail(false)
{}

FrameCompressor::~FrameCompressor()
{
    if (m_me_data)
        delete m_me_data;
}

FrameByteIO* FrameCompressor::Compress( FrameBuffer& my_buffer ,
                                        const FrameBuffer& orig_buffer ,
                                        int fnum,
                                        int au_fnum)
{
    Frame& my_frame = my_buffer.GetFrame( fnum );

    FrameParams& fparams = my_frame.GetFparams();
    const FrameSort& fsort = fparams.FSort();
    
    m_medata_avail = false;

    m_is_a_cut = false;

    if (m_me_data)
    {
        delete m_me_data;
        m_me_data = 0;
    }

    if ( fsort.IsInter() )
    {
        m_me_data = new MEData( m_encparams.XNumMB() , 
                                m_encparams.YNumMB(), 
                                fparams.NumRefs());

        // Motion estimate first
        MotionEstimator my_motEst( m_encparams );
        my_motEst.DoME( orig_buffer , fnum , *m_me_data );

        // If we have a cut....
        AnalyseMEData( *m_me_data );
        if ( m_is_a_cut )
        {
            if (my_frame.GetFparams().FSort().IsRef())
                my_frame.SetFrameSort (FrameSort::IntraRefFrameSort());
            else
                my_frame.SetFrameSort (FrameSort::IntraNonRefFrameSort());
            
            if ( m_encparams.Verbose() )
                std::cout<<std::endl<<"Cut detected and I-frame inserted!";
        }

    }

    // Set the wavelet filter
    if ( fsort.IsIntra() )
    {
        m_encparams.SetTransformFilter( m_encparams.IntraTransformFilter() );
        m_encparams.SetDefaultCodeBlocks( INTRA_FRAME );
    }
    else
    {
        m_encparams.SetTransformFilter( m_encparams.InterTransformFilter() );
        m_encparams.SetDefaultCodeBlocks( INTER_FRAME );
        // Set the frame weight parameters.
        // FIXME - setting to default at the moment. Need to process command
        // line args in future
        if (fparams.Refs().size() == 1)
        {
            m_encparams.SetFrameWeightsPrecision(0);
            m_encparams.SetRef1Weight(1);
            m_encparams.SetRef2Weight(0);
        }
        else
        {
            m_encparams.SetFrameWeightsPrecision(1);
            m_encparams.SetRef1Weight(1);
            m_encparams.SetRef2Weight(1);
            // TESTING
            //m_encparams.SetFrameWeightsPrecision(2);
            //m_encparams.SetRef1Weight(1);
            //m_encparams.SetRef2Weight(3);
        }
    }

    // Write the frame header. We wait until after motion estimation, since
    // this allows us to do cut-detection and (possibly) to decide whether
    // or not to skip a frame before actually encoding anything. However we
    // can do this at any point prior to actually writing any frame data.
    //WriteFrameHeader( my_frame.GetFparams() );
    FrameByteIO* p_frame_byteio = new FrameByteIO(fparams,
                                                  fnum,  
                                                  au_fnum);
   
    p_frame_byteio->Output();

    if ( !m_skipped )
    {    // If not skipped we continue with the coding ...
        if (m_encparams.Verbose() )
            std::cout<<std::endl<<"Using QF: "<<m_encparams.Qf();

        if (fsort.IsInter() )
        {
             // Code the MV data

            // If we're using global motion parameters, code them
            if (m_use_global)
            {
                /*
                    Code the global motion parameters
                    TBC ....
                */
            }

            // If we're using block motion vectors, code them
            if ( m_use_block_mv )
            {
                MvDataByteIO *mv_data = new MvDataByteIO(fparams, 
                                        static_cast<CodecParams&>(m_encparams));
                p_frame_byteio->SetMvData(mv_data);
                
                CompressMVData( mv_data );
            }

             // Then motion compensate
            MotionCompensator::CompensateFrame( m_encparams , SUBTRACT , 
                                                my_buffer , fnum , 
                                                *m_me_data );
 
        }//?fsort

        //Write Transform Header
        TransformByteIO *p_transform_byteio = new TransformByteIO(fparams, 
                                static_cast<CodecParams&>(m_encparams));
        p_frame_byteio->SetTransformData(p_transform_byteio);
        p_transform_byteio->Output();

        //code component data
        
        CompCompressor my_compcoder(m_encparams , fparams );

        if ( fsort.IsIntra() )
        {
            p_transform_byteio->AddComponent( my_compcoder.Compress( 
                my_buffer.GetComponent( fnum , Y_COMP), m_is_a_cut ) );
            p_transform_byteio->AddComponent( my_compcoder.Compress( 
                my_buffer.GetComponent( fnum , U_COMP), m_is_a_cut ) );
            p_transform_byteio->AddComponent( my_compcoder.Compress( 
                my_buffer.GetComponent( fnum , V_COMP), m_is_a_cut ) );
        }
        else
        {
            p_transform_byteio->AddComponent( my_compcoder.Compress( 
                my_buffer.GetComponent( fnum , Y_COMP), false , 
                    m_intra_ratio , m_me_data ) );
            p_transform_byteio->AddComponent( my_compcoder.Compress( 
                my_buffer.GetComponent( fnum , U_COMP), false , 
                    m_intra_ratio , m_me_data ) );
            p_transform_byteio->AddComponent( my_compcoder.Compress( 
                my_buffer.GetComponent( fnum , V_COMP), false , 
                    m_intra_ratio , m_me_data ) );
        }
                                          

        //motion compensate again if necessary
        if (fsort.IsInter() )
        {
            if ( fsort.IsRef() || m_encparams.LocalDecode() )
            {
                MotionCompensator::CompensateFrame( m_encparams , ADD , 
                                                    my_buffer , fnum , 
                                                    *m_me_data );   
            }
            // Set me data available flag
            m_medata_avail = true;
        }//?fsort

         //finally clip the data to keep it in range
        my_buffer.GetFrame( fnum ).Clip();

    }//?m_skipped

    // return compressed frame
    return p_frame_byteio;
}

const MEData* FrameCompressor::GetMEData() const
{
    TESTM (m_me_data != NULL, "m_medata allocated");
    TESTM (m_medata_avail == true, "ME Data available");

    return m_me_data;
}

void FrameCompressor::CompressMVData(MvDataByteIO* mv_data)
{
    SplitModeCodec smode_coder( mv_data->SplitModeData()->DataBlock(), TOTAL_MV_CTXS);
    smode_coder.Compress( *m_me_data );
    mv_data->SplitModeData()->Output();
    
    PredModeCodec pmode_coder( mv_data->PredModeData()->DataBlock(), TOTAL_MV_CTXS);
    pmode_coder.Compress( *m_me_data );
    mv_data->PredModeData()->Output();

    VectorElementCodec vcoder1h( mv_data->MV1HorizData()->DataBlock(), 1, 
                                 HORIZONTAL, TOTAL_MV_CTXS);
    vcoder1h.Compress( *m_me_data );
    mv_data->MV1HorizData()->Output();
    
    VectorElementCodec vcoder1v( mv_data->MV1VertData()->DataBlock(), 1, 
                                 VERTICAL, TOTAL_MV_CTXS);
    vcoder1v.Compress( *m_me_data );
    mv_data->MV1VertData()->Output();

    if ( m_me_data->NumRefs()>1 )
    {
        VectorElementCodec vcoder2h( mv_data->MV2HorizData()->DataBlock(), 2, 
                                     HORIZONTAL, TOTAL_MV_CTXS);
        vcoder2h.Compress( *m_me_data );
        mv_data->MV2HorizData()->Output();
        
        VectorElementCodec vcoder2v( mv_data->MV2VertData()->DataBlock(), 2, 
                                     VERTICAL, TOTAL_MV_CTXS);
        vcoder2v.Compress( *m_me_data );
        mv_data->MV2VertData()->Output();
    }

    DCCodec ydc_coder( mv_data->YDCData()->DataBlock(), Y_COMP, TOTAL_MV_CTXS);
    ydc_coder.Compress( *m_me_data );
    mv_data->YDCData()->Output();

    DCCodec udc_coder( mv_data->UDCData()->DataBlock(), U_COMP, TOTAL_MV_CTXS);
    udc_coder.Compress( *m_me_data );
    mv_data->UDCData()->Output();
    
    DCCodec vdc_coder( mv_data->VDCData()->DataBlock(), V_COMP, TOTAL_MV_CTXS);
    vdc_coder.Compress( *m_me_data );
    mv_data->VDCData()->Output();

    mv_data->Output();    
}

void FrameCompressor::AnalyseMEData( const MEData& me_data )
{
    // Count the number of intra blocks
    const TwoDArray<PredMode>& modes = me_data.Mode();

    int count_intra = 0;
    for ( int j=0 ; j<modes.LengthY() ; ++j )
    {
        for ( int i=0 ; i<modes.LengthX() ; ++i )
        {
            if ( modes[j][i] == INTRA )
                count_intra++;
        }
    }// j
    
    m_intra_ratio = 100.0*static_cast<double>( count_intra ) / 
                          static_cast<double>( modes.LengthX() * modes.LengthY() );

    if ( m_encparams.Verbose() )
        std::cout<<std::endl<<m_intra_ratio<<"% of blocks are intra   ";

    // Check the size of SAD errors across reference 1    
    const TwoDArray<MvCostData>& pcosts = me_data.PredCosts( 1 );

    // averege SAD across all relevant blocks
    long double sad_average = 0.0;
    // average SAD in a given block
    long double block_average; 
    // the block parameters
    const OLBParams& bparams = m_encparams.LumaBParams( 2 ); 
    //the count of the relevant blocks
    int block_count = 0;

    for ( int j=0 ; j<pcosts.LengthY() ; ++j )
    {
        for ( int i=0 ; i<pcosts.LengthX() ; ++i )
        {

            if ( modes[j][i] == REF1_ONLY || modes[j][i] == REF1AND2 )
            {
                block_average = pcosts[j][i].SAD /
                                static_cast<long double>( bparams.Xblen() * bparams.Yblen() * 4 );
                sad_average += block_average;
                block_count++;
            }

        }// i
    }// j

    if ( block_count != 0)
        sad_average /= static_cast<long double>( block_count );
   
    if ( (sad_average > 30.0) || (m_intra_ratio > 33.33) )
        m_is_a_cut = true;
    else
        m_is_a_cut = false;
  
}
