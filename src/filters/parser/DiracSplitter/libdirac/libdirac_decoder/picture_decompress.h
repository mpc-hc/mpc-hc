/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: picture_decompress.h,v 1.2 2008/04/29 08:51:52 tjdwave Exp $ $Name:  $
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



#ifndef _PICTURE_DECOMPRESS_H_
#define _PICTURE_DECOMPRESS_H_

#include <libdirac_common/picture_buffer.h>
#include <libdirac_common/common.h>
#include <libdirac_byteio/picture_byteio.h>
#include <libdirac_byteio/transform_byteio.h>

namespace dirac
{
class MvData;

//! Compress a single image picture
/*!
    This class decompresses a single picture at a time, using parameters
    supplied at its construction. PictureDecompressor is used by
    SequenceDecompressor.
*/
class PictureDecompressor
{
public:
    //! Constructor
    /*!
        Creates a PictureDecompressor with specific set of parameters the
        control the decompression process. It decodes motion data before
        decoding each component of the picture.

        \param  decp    decoder parameters
        \param  cf      the chroma format of the picture being decompressed
    */
    PictureDecompressor(DecoderParams& decp, ChromaFormat cf);

    //! Destructor
    /*!
        Releases resources.
    */
    ~PictureDecompressor();

    //! Decompress the next picture into the buffer
    /*!
        Decompresses the next picture from the stream and place at the end
        of a picture buffer.
        Returns true if able to decode successfully, false otherwise

        \param parseunit_byteio Picture info in Dirac-stream format
        \param my_buffer   picture buffer into which the picture is placed
    */
    bool Decompress(ParseUnitByteIO& parseunit_byteio,
                    PictureBuffer& my_buffer);

    //! Returns the picture parameters of the current picture being decoded
    const PictureParams& GetPicParams() const
    {
        return m_pparams;
    }

private:
    //! Copy constructor is private and body-less
    /*!
        Copy constructor is private and body-less. This class should not be copied.

    */
    PictureDecompressor(const PictureDecompressor& cpy);

    //! Assignment = is private and body-less
    /*!
        Assignment = is private and body-less. This class should not be
        assigned.
    */
    PictureDecompressor& operator=(const PictureDecompressor& rhs);

    //! Initialise the padded coefficient data for the IDWT and subband decoding
    void InitCoeffData(CoeffArray& coeff_data, const int xl, const int yl);

    //! Removes all the reference pictures in the retired list
    void CleanReferencePictures(PictureBuffer& my_buffer);

    //! Decodes component data
    void CompDecompress(TransformByteIO *p_transform_byteio,
                        PictureBuffer& my_buffer, int pnum, CompSort cs);

    //! Decodes the motion data
    void DecompressMVData(std::auto_ptr<MvData>& mv_data, PictureByteIO& picture_byteio);


    //! Set the number of superblocks and blocks
    void SetMVBlocks();

    //! Add a picture to the picture buffer
    void PushPicture(PictureBuffer &my_buffer);

    //Member variables

    //! Parameters for the decompression, as provided in constructor
    DecoderParams& m_decparams;

    //! Chroma format of the picture being decompressed
    ChromaFormat m_cformat;

    //! An indicator which is true if the picture has been skipped, false otherwise
    bool m_skipped;

    //! An indicator that is true if we use global motion vectors, false otherwise
    bool m_use_global;

    //! An indicator that is true if we use block motion vectors, false otherwise
    bool m_use_block_mv;

    //! Prediction mode to use if we only have global motion vectors
    PredMode m_global_pred_mode;

    //! Current Picture Parameters
    PictureParams m_pparams;
};

} // namespace dirac

#endif
