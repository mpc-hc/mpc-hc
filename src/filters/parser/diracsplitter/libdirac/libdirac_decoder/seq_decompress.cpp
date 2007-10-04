/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: seq_decompress.cpp,v 1.20 2007/09/03 11:31:42 asuraparaju Exp $ $Name: Dirac_0_8_0 $
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
*                 Anuradha Suraparaju,
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


///////////////////////////////////////////
//---------------------------------------//
//Class to manage decompressing sequences//
//---------------------------------------//
///////////////////////////////////////////

#include <libdirac_common/video_format_defaults.h>
#include <libdirac_common/dirac_assertions.h>
#include <libdirac_decoder/seq_decompress.h>
#include <libdirac_common/common.h>
#include <libdirac_common/frame_buffer.h>
#include <libdirac_decoder/frame_decompress.h>
#include <libdirac_byteio/accessunit_byteio.h>
using namespace dirac;

SequenceDecompressor::SequenceDecompressor(ParseUnitByteIO& parseunit,bool verbosity)
: 
m_all_done(false),
m_current_code_fnum(0),
m_delay(1),
m_show_fnum(-1),
m_highest_fnum(0)
{
    // read unit
    NewAccessUnit(parseunit);

    m_decparams.SetVerbose( verbosity );
   
    m_fbuffer= new FrameBuffer( );

    m_fdecoder = new FrameDecompressor (m_decparams , m_srcparams.CFormat());
   
}

SequenceDecompressor::~SequenceDecompressor()
{
    delete m_fbuffer;
    delete m_fdecoder;
}

const FrameParams& SequenceDecompressor::GetNextFrameParams() const
{
    return m_fdecoder->GetFrameParams();
}

void SequenceDecompressor::NewAccessUnit(ParseUnitByteIO& parseunit_byteio)
{
    // read access-unit data
    AccessUnitByteIO accessunit_byteio(parseunit_byteio,
                                       m_parse_params, m_srcparams, m_decparams);
    accessunit_byteio.Input();

}

Frame& SequenceDecompressor::DecompressNextFrame(ParseUnitByteIO* p_parseunit_byteio,
                                                 bool skip /* = false */)
{
    //this function decodes the next frame in coding order and returns the next frame in display order
    //In general these will differ, and because of re-ordering there is a m_delay which needs to be imposed.
    //This creates problems at the start and at the end of the sequence which must be dealt with.
    //At the start we just keep outputting frame 0. At the end you will need to loop for longer to get all
    //the frames out. It's up to the calling function to do something with the decoded frames as they
    //come out - write them to screen or to file, as required.

    TEST (m_fdecoder != NULL);
    
    // Remove the last displayed frame from the buffer if it wasn't a reference
    if ( m_show_fnum>0 )
    {
        if ( m_decparams.Verbose() )
            std::cout<<std::endl<<"Cleaning display buffer: ";         
        if ( m_fbuffer->IsFrameAvail(m_show_fnum-1) && 
            m_fbuffer->GetFrame(m_show_fnum-1).GetFparams().FSort().IsNonRef() )
        {
            m_fbuffer->Clean(m_show_fnum-1);
            if ( m_decparams.Verbose() )
                std::cout<<(m_show_fnum-1)<<" ";
        }
    }

    bool new_frame_to_display=false;
       
    if (!skip && p_parseunit_byteio)
    {
       if (m_decparams.Verbose())
           std::cout<<std::endl<<"Calling frame decompression function";
       new_frame_to_display = m_fdecoder->Decompress(*p_parseunit_byteio,
                                                     *m_fbuffer);
    }
    /***
    //if we've exited with success, there's a new frame to display, so increment
    //the counters. Otherwise, freeze on the last frame shown
    m_show_fnum=std::max(m_current_code_fnum-m_delay,0);
    if (new_frame_to_display || skip)
    {
        m_current_code_fnum++;
    }
    ***/
    // FIXME - temporary fix to fix frame delay for i-frames

    Frame &f = m_fbuffer->GetFrame(m_show_fnum+1 );
    m_show_fnum = m_show_fnum >= 0 ? m_show_fnum : m_fdecoder->GetFrameParams().FrameNum()-1;

    m_highest_fnum = std::max(m_fdecoder->GetFrameParams().FrameNum(), m_highest_fnum);
    if (f.GetFparams().FrameNum() == m_show_fnum+1)
    {
        ++m_show_fnum;
        return f;
    }
    
    return m_fbuffer->GetFrame(m_show_fnum);
}

Frame& SequenceDecompressor::GetNextFrame()
{
    return m_fbuffer->GetFrame(m_show_fnum);
}

bool SequenceDecompressor::Finished()
{
    return m_show_fnum==m_highest_fnum;
}
