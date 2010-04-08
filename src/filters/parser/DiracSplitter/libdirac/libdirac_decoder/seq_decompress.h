/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: seq_decompress.h,v 1.13 2008/05/02 06:05:04 asuraparaju Exp $ $Name:  $
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
*                 Anuradha Suraparaju
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


#ifndef _SEQ_DECOMPRESS_H_
#define _SEQ_DECOMPRESS_H_

///////////////////////////////////////////
//---------------------------------------//
//Class to manage decompressing sequences//
//---------------------------------------//
///////////////////////////////////////////

#include "libdirac_common/common.h"
#include "libdirac_byteio/parseunit_byteio.h"
#include <iostream>

namespace dirac
{
class PictureBuffer;
class Picture;
class PictureDecompressor;

//! Decompresses a sequence of pictures from a stream.
/*!
    This class decompresses a sequence of frames, picture by picture.
*/
class SequenceDecompressor
{
public:

    //! Constructor
    /*!
        Initializes the decompressor with an input stream and level of
        output detail.
        \param  parseunit   First access-unit of new sequence
        \param  verbosity   when true, increases the amount of information displayed during decompression
     */
    SequenceDecompressor(ParseUnitByteIO& parseunit, bool verbosity);

    //! Destructor
    /*!
        Closes files and releases resources.
    */
    ~SequenceDecompressor();

    //! Marks beginning of a new AccessUnit
    /*!
        \param parseunit_byteio AccessUnit info in Dirac-stream format
    */
    void NewAccessUnit(ParseUnitByteIO& parseunit_byteio);


    //! Decompress the next picture in sequence
    /*!
        This function decodes the next picture in coding order and returns
        the next picture in display order. In general these will differ, and
        because of re-ordering there is a delay which needs to be imposed.
        This creates problems at the start and at the end of the sequence
        which must be dealt with. At the start we just keep outputting
        picture 0. At the end you will need to loop for longer to get all
        the pictures out. It's up to the calling function to do something
        with the decoded pictures as they come out -- write them to screen
        or to file, as required.

        \param p_parseunit_byteio Picture information in Dirac-stream format
        \return      reference to the next locally decoded picture available for display
    */
    const Picture* DecompressNextPicture(ParseUnitByteIO* p_parseunit_byteio);

    //! Get the next picture available for display
    const Picture* GetNextPicture();

    //! Get the next picture parameters
    const PictureParams* GetNextPictureParams() const;
    //! Determine if decompression is complete.
    /*!
        Indicates whether or not the last picture in the sequence has been
        decompressed.
        \return     true if last picture has been compressed; false if not
    */
    bool Finished();
    //! Interrogates for parse parameters.
    /*!
        Returns the parse parameters used for this decompression run.

        \return parse parameters.
     */
    ParseParams & GetParseParams()
    {
        return m_parse_params;
    }


    //! Interrogates for source parameters.
    /*!
        Returns the source parameters used for this decompression run.

        \return source parameters.
     */
    SourceParams & GetSourceParams()
    {
        return m_srcparams;
    }


    //! Interrogates for coding parameters.
    /*!
        Returns the decoder parameters used for this decompression run.

        \return decoder parameters.
     */
    DecoderParams & GetDecoderParams()
    {
        return m_decparams;
    }
private:
    //! Copy constructor is private and body-less
    /*!
        Copy constructor is private and body-less. This class should not
        be copied.

    */
    SequenceDecompressor(const SequenceDecompressor& cpy);

    //! Assignment = is private and body-less
    /*!
        Assignment = is private and body-less. This class should not be
        assigned.

    */
    SequenceDecompressor& operator=(const SequenceDecompressor& rhs);


    //Member variables

    //! Completion flag, returned via the Finished method
    bool m_all_done;
    //! Parameters for the decompression, as provided in constructor
    DecoderParams m_decparams;
    //! The parse parameters obtained from the stream header
    ParseParams m_parse_params;
    //! The source parameters obtained from the stream header
    SourceParams m_srcparams;
    //! A picture buffer used for local storage of pictures whilst pending re-ordering or being used for reference.
    PictureBuffer* m_pbuffer;
    //! Number of the picture in coded order which is to be decoded
    int m_current_code_pnum;
    //! A delay so that we don't display what we haven't decoded
    int m_delay;
    //! Index, in display order, of the last picture read
    int m_last_picture_read;
    //! Index, in display order of the picture to be displayed next - computed from delay and current_code_pnum
    int m_show_pnum;
    //! Picture decompressor object
    PictureDecompressor *m_pdecoder;
    //! Highest picture-num processed - for tracking end-of-sequence
    int m_highest_pnum;
};

} // namespace dirac

#endif
