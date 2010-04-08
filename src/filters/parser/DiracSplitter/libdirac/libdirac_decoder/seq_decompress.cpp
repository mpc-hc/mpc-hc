/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: seq_decompress.cpp,v 1.25 2008/08/14 00:51:08 asuraparaju Exp $ $Name:  $
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
#include <libdirac_common/picture_buffer.h>
#include <libdirac_decoder/picture_decompress.h>
#include <libdirac_byteio/accessunit_byteio.h>
using namespace dirac;

SequenceDecompressor::SequenceDecompressor(ParseUnitByteIO& parseunit, bool verbosity)
    :
    m_all_done(false),
    m_current_code_pnum(0),
    m_delay(1),
    m_show_pnum(-1),
    m_highest_pnum(0)
{
    // read unit
    NewAccessUnit(parseunit);

    if(m_decparams.FieldCoding())
        m_delay = 2;

    m_decparams.SetVerbose(verbosity);

    m_pbuffer = new PictureBuffer();

    m_pdecoder = new PictureDecompressor(m_decparams , m_srcparams.CFormat());

}

SequenceDecompressor::~SequenceDecompressor()
{
    delete m_pbuffer;
    delete m_pdecoder;
}

const PictureParams* SequenceDecompressor::GetNextPictureParams() const
{
    return &m_pdecoder->GetPicParams();
}

void SequenceDecompressor::NewAccessUnit(ParseUnitByteIO& parseunit_byteio)
{
    // read sequence header
    SequenceHeaderByteIO seqheader_byteio(parseunit_byteio, m_parse_params, m_srcparams, m_decparams);
    seqheader_byteio.Input();

}

const Picture* SequenceDecompressor::DecompressNextPicture(ParseUnitByteIO* p_parseunit_byteio)
{
    //this function decodes the next picture in coding order and returns the next picture in display order
    //In general these will differ, and because of re-ordering there is a m_delay which needs to be imposed.
    //This creates problems at the start and at the end of the sequence which must be dealt with.
    //At the start we just keep outputting picture 0. At the end you will need to loop for longer to get all
    //the pictures out. It's up to the calling function to do something with the decoded pictures as they
    //come out - write them to screen or to file, as required.

    TEST(m_pdecoder != NULL);

    // Remove the last displayed picture from the buffer if it wasn't a reference
    if(m_show_pnum > 0)
    {
        if(m_decparams.Verbose())
            std::cout << std::endl << "Cleaning display buffer: ";
        if(m_pbuffer->IsPictureAvail(m_show_pnum - 1) &&
           m_pbuffer->GetPicture(m_show_pnum - 1).GetPparams().PicSort().IsNonRef())
        {
            m_pbuffer->Remove(m_show_pnum - 1);
            if(m_decparams.Verbose())
                std::cout << (m_show_pnum - 1) << " ";
        }
    }

    bool new_picture_to_display = false;

    if(p_parseunit_byteio)
    {
        if(m_decparams.Verbose())
            std::cout << std::endl << "Calling picture decompression function";
        new_picture_to_display = m_pdecoder->Decompress(*p_parseunit_byteio,
                                 *m_pbuffer);
    }

    if(m_show_pnum < 0 && new_picture_to_display == false)
        return NULL;

    if(m_pbuffer->IsPictureAvail(m_show_pnum + 1))
        ++m_show_pnum;
    else if(new_picture_to_display && m_pdecoder->GetPicParams().PicSort().IsNonRef())
    {
        // if a decoded future non reference frame is available it implies
        // that some frames have been skipped because of possible truncation
        // errors
        m_show_pnum =  m_pdecoder->GetPicParams().PictureNum();
    }

    m_highest_pnum = std::max(m_pdecoder->GetPicParams().PictureNum(), m_highest_pnum);

    if(m_pbuffer->IsPictureAvail(m_show_pnum))
        return &m_pbuffer->GetPicture(m_show_pnum);
    else
        return NULL;
}

const Picture* SequenceDecompressor::GetNextPicture()
{
    if(m_pbuffer->IsPictureAvail(m_show_pnum))
        return &m_pbuffer->GetPicture(m_show_pnum);
    else
        return NULL;
}

bool SequenceDecompressor::Finished()
{
    if(m_show_pnum >= m_highest_pnum)
        return true;

    if(!m_pbuffer->IsPictureAvail(m_show_pnum + 1))
        ++m_show_pnum;

    return false;
}
