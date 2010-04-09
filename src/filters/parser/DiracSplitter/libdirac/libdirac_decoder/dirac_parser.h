/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: dirac_parser.h,v 1.8 2008/02/13 03:36:11 asuraparaju Exp $ $Name:  $
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
* Contributor(s): Anuradha Suraparaju (Original Author)
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

#ifndef DIRAC_PARSER_H
#define DIRAC_PARSER_H

#include <libdirac_common/dirac_types.h>
#include <libdirac_decoder/decoder_types.h>

/*! \file
\brief C interface to Dirac decoder.
 
 A set of 'C' functions that define the public interface to the Dirac decoder.
 Refer to the the reference decoder source code, decoder/decmain.cpp for
 an example of how to use the "C" interface. The pseudocode below gives
 a brief description of the "C" interface usage.

\verbatim
 #include <libdirac_decoder/dirac_parser.h>/n
 Initialise the decoder

 dirac_decoder_t *decoder_handle = dirac_decoder_init();
 do
 {
     dirac_decoder_state_t state = dirac_parse (decoder_handle);
     switch (state)
     {
     case STATE_BUFFER:
         read more data.
         Pass data to the decoder.
         dirac_buffer (decoder_handle, data_start, data_end)
         break;

     case STATE_SEQUENCE:
         handle start of sequence.
         The decoder returns the sequence parameters in the 
         seq_params member of the decoder handle.
         Allocate space for the frame data buffers and pass 
         this to the decoder.
         dirac_set_buf (decoder_handle, buf, NULL);
         break;

     case STATE_SEQUENCE_END:
         Deallocate frame data buffers
         break;

     case STATE_PICTURE_AVAIL:
         Handle picture data.
         The decoder sets the fbuf member in the decoder 
         handle to the frame decoded.
         break;

     case STATE_INVALID:
         Unrecoverable error. Stop all processing
         break;
     }
 } while (data available && decoder state != STATE_INVALID
 
 Free the decoder resources
 dirac_decoder_close(decoder_handle)
 \endverbatim
*/
#ifdef __cplusplus
extern "C" {
#endif

typedef DecoderState dirac_decoder_state_t;

/*! Structure that holds the information returned by the parser */
typedef struct 
{
    /*! parser state */
    dirac_decoder_state_t state;
    /*! parse parameters */
    dirac_parseparams_t parse_params;
    /*! source parameters */
    dirac_sourceparams_t src_params;
    /*! frame (NOT picture) number */
    unsigned int frame_num;
    /*! void pointer to internal parser */
    void *parser;
    /*! frame (NOT picture) buffer to hold luma and chroma data */
    dirac_framebuf_t *fbuf;
    /*! boolean flag that indicates if a decoded frame (NOT picture) is available */
    int frame_avail;
    /*! verbose output */
    int verbose;

} dirac_decoder_t;

/*! 
    Decoder Init
    Initialise the decoder. 
    \param  verbose boolean flag to set verbose output
    \return decoder handle
*/
extern DllExport dirac_decoder_t *dirac_decoder_init(int verbose);

/*!
    Release the decoder resources
    \param decoder  Decoder object
*/
extern DllExport void dirac_decoder_close(dirac_decoder_t *decoder);

/*!
    Parses the data in the input buffer. This function returns the 
    following values.
    \n STATE_BUFFER:         Not enough data in internal buffer to process 
    \n STATE_SEQUENCE:       Start of sequence detected. The seq_params member
                             in the decoder object is set to the details of the
                             next sequence to be processed.
    \n STATE_PICTURE_START:  Start of picture detected. The frame_params member
                             of the decoder object is set to the details of the
                             next frame to be processed.
    \n STATE_PICTURE_AVAIL:  Decoded picture available. The frame_aprams member
                             of the decoder object is set the the details of
                             the decoded frame available. The fbuf member of
                             the decoder object has the luma and chroma data of
                             the decompressed frame.
    \n STATE_SEQUENCE_END:   End of sequence detected.
    \n STATE_INVALID:        Invalid stream. Stop further processing.

    \param decoder  Decoder object
    \return         Decoder state

*/
extern DllExport dirac_decoder_state_t dirac_parse (dirac_decoder_t *decoder);

/*!
    Copy data into internal buffer
    \param decoder  Decoder object
    \param start    Start of data
    \param end      End of data
*/
extern DllExport void dirac_buffer (dirac_decoder_t *decoder, unsigned char *start, unsigned char *end);

/*!
    Set the output buffer into which the decoder copies the decoded data
    \param decoder  Decoder object
    \param buf      Array of char buffers to hold luma and chroma data
    \param id       User data
*/
extern DllExport void dirac_set_buf (dirac_decoder_t *decoder, unsigned char *buf[3], void *id);

#ifdef __cplusplus
}
#endif
#endif
