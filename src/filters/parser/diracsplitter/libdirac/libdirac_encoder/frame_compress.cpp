/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: frame_compress.cpp 280 2005-01-30 05:11:46Z gabest $ $Name$
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
*                 Anuradha Suraparaju
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
#include <libdirac_common/golomb.h>
#include <libdirac_common/bit_manager.h>
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
{
}

FrameCompressor::~FrameCompressor()
{
    if (m_me_data)
        delete m_me_data;
}

void FrameCompressor::Compress( FrameBuffer& my_buffer, const FrameBuffer& orig_buffer , int fnum )
{
    FrameOutputManager& foutput = m_encparams.BitsOut().FrameOutput();

    Frame& my_frame = my_buffer.GetFrame( fnum );
    const FrameParams& fparams = my_frame.GetFparams();
    const FrameSort& fsort = fparams.FSort();
    const ChromaFormat cformat = fparams.CFormat();

    // number of bits written, without byte alignment
    unsigned int num_mv_bits;
    m_medata_avail = false;

    CompCompressor my_compcoder(m_encparams , fparams );

    if (m_me_data)
    {
        delete m_me_data;
        m_me_data = 0;
    }

    if ( fsort != I_frame )
    {

        m_me_data = new MEData( m_encparams.XNumMB() , m_encparams.YNumMB());

        // Motion estimate first
        MotionEstimator my_motEst( m_encparams );
        bool is_a_cut = my_motEst.DoME( orig_buffer , fnum , *m_me_data );

        // If we have a cut, and an L1 frame, then turn into an I-frame
        if ( is_a_cut )
        {
            my_frame.SetFrameSort( I_frame );
            if ( m_encparams.Verbose() )
                std::cerr<<std::endl<<"Cut detected and I-frame inserted!";
        }

    }


    // Write the frame header. We wait until after motion estimation, since
    // this allows us to do cut-detection and (possibly) to decide whether
    // or not to skip a frame before actually encoding anything. However we
    // can do this at any point prior to actually writing any frame data.
    WriteFrameHeader( my_frame.GetFparams() );


    if ( !m_skipped )
    {    // If not skipped we continue with the coding ...

        if ( fsort != I_frame)
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
                MvDataCodec my_mv_coder( &( foutput.MVOutput().Data() ) , 50 , cformat);

                my_mv_coder.InitContexts();//may not be necessary
                num_mv_bits = my_mv_coder.Compress( *m_me_data );            

                UnsignedGolombCode( foutput.MVOutput().Header() , num_mv_bits);
            }

             // Then motion compensate

            MotionCompensator mycomp( m_encparams , SUBTRACT);
            mycomp.CompensateFrame( my_buffer , fnum , *m_me_data );

        }//?fsort

        //code component data
        my_compcoder.Compress( my_buffer.GetComponent( fnum , Y_COMP) );
        if (cformat != Yonly)
        {
            my_compcoder.Compress( my_buffer.GetComponent( fnum , U_COMP) );
            my_compcoder.Compress( my_buffer.GetComponent( fnum , V_COMP) );
        }

        //motion compensate again if necessary
        if ( fsort != I_frame )
        {
            MotionCompensator mycomp( m_encparams , ADD);
            mycomp.CompensateFrame( my_buffer , fnum , *m_me_data );
            // Set me data available flag
            m_medata_avail = true;
        }//?fsort

         //finally clip the data to keep it in range
        my_buffer.GetFrame(fnum).Clip();

    }//?m_skipped
}

void FrameCompressor::WriteFrameHeader( const FrameParams& fparams )
{
    BasicOutputManager& frame_header_op = m_encparams.BitsOut().FrameOutput().HeaderOutput();

    // Write the frame start code
    unsigned char frame_start[5] = { START_CODE_PREFIX_BYTE0, 
                                     START_CODE_PREFIX_BYTE1, 
                                     START_CODE_PREFIX_BYTE2, 
                                     START_CODE_PREFIX_BYTE3, 
                                     IFRAME_START_CODE };
    switch(fparams.FSort())
    {
    case I_frame:
        frame_start[4] = IFRAME_START_CODE;
        break;

    case L1_frame:
        frame_start[4] = L1FRAME_START_CODE;
        break;

    case L2_frame:
        frame_start[4] = L2FRAME_START_CODE;
         break;

    default:
//         dirac_ASSERTM (false, "Frame type is I_frame or L1_frame or L2_frame");
         break;
    }
    frame_header_op.OutputBytes((char *)frame_start, 5);

    // Write the frame number
    UnsignedGolombCode(frame_header_op , fparams.FrameNum());

    //write whether the frame is m_skipped or not
    frame_header_op.OutputBit( m_skipped );

    if (!m_skipped)
    {// If we're not m_skipped, then we write the rest of the metadata

        // Write the expiry time relative to the frame number 
        UnsignedGolombCode( frame_header_op , fparams.ExpiryTime() );

        // Write the frame sort
        UnsignedGolombCode( frame_header_op , (unsigned int) fparams.FSort() );        
        if (fparams.FSort() != I_frame)
        {        
            // If not an I-frame, write how many references there are        
            UnsignedGolombCode( frame_header_op , (unsigned int) fparams.Refs().size() );

            // For each reference, write the reference number relative to the frame number
            for ( size_t i=0 ; i<fparams.Refs().size() ; ++i )
                GolombCode( frame_header_op , fparams.Refs()[i]-fparams.FrameNum() );

            // Indicate whether or not there is global motion vector data
            frame_header_op.OutputBit( m_use_global );

            // Indicate whether or not there is block motion vector data
            frame_header_op.OutputBit( m_use_block_mv );

            // If there is global but no block motion vector data, indicate the 
            // prediction mode to use for the whole frame
            if ( m_use_global && !m_use_block_mv )
            {
                UnsignedGolombCode( frame_header_op , (unsigned int) m_global_pred_mode );
            }
        }

    }// ?m_skipped
}

const MEData* FrameCompressor::GetMEData() const
{
    TESTM (m_me_data != NULL, "m_medata allocated");
    TESTM (m_medata_avail == true, "ME Data available");

    return m_me_data;
}
