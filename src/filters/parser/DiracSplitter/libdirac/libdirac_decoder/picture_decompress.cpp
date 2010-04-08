/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: picture_decompress.cpp,v 1.9 2008/10/01 01:26:47 asuraparaju Exp $ $Name:  $
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
*                 Andrew Kennedy,
*                 Tim Borer
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


//Decompression of pictures
/////////////////////////

#include <libdirac_common/dirac_assertions.h>
#include <libdirac_decoder/picture_decompress.h>
#include <libdirac_decoder/comp_decompress.h>
#include <libdirac_common/mot_comp.h>
#include <libdirac_common/mv_codec.h>
#include <libdirac_byteio/picture_byteio.h>
#include <libdirac_common/dirac_exception.h>
using namespace dirac;

#include <iostream>
#include <memory>

using std::vector;
using std::auto_ptr;

PictureDecompressor::PictureDecompressor(DecoderParams& decp, ChromaFormat cf)
    :
    m_decparams(decp),
    m_cformat(cf)
{
}

PictureDecompressor::~PictureDecompressor()
{
}


bool PictureDecompressor::Decompress(ParseUnitByteIO& parseunit_byteio,
                                     PictureBuffer& my_buffer)
{
    // get current byte position
    //int start_pos = parseunit_byteio.GetReadBytePosition();
    try
    {

        // read picture data
        PictureByteIO picture_byteio(m_pparams,
                                     parseunit_byteio);

        picture_byteio.Input();

        PictureSort fs;

        if(m_pparams.GetPictureType() == INTRA_PICTURE)
            fs.SetIntra();
        else
            fs.SetInter();

        if(m_pparams.GetReferenceType() == REFERENCE_PICTURE)
            fs.SetRef();
        else
            fs.SetNonRef();

        m_pparams.SetPicSort(fs);

        if(m_pparams.GetReferenceType() == REFERENCE_PICTURE)
            // Now clean the reference pictures from the buffer
            CleanReferencePictures(my_buffer);

        // Check if the picture can be decoded
        if(m_pparams.PicSort().IsInter())
        {
            const std::vector<int>& refs = m_pparams.Refs();

            for(unsigned int i = 0; i < refs.size(); ++i)
                if(!my_buffer.IsPictureAvail(refs[i]))
                    return false;
        }

        // decode the rest of the picture

        if(m_decparams.Verbose())
        {
            std::cout << std::endl << "Decoding picture " << m_pparams.PictureNum() << " in display order";
            if(m_pparams.PicSort().IsInter())
            {
                std::cout << std::endl << "References: " << m_pparams.Refs()[0];
                if(m_pparams.Refs().size() > 1)
                    std::cout << " and " << m_pparams.Refs()[1];
            }
        }

        PictureSort psort = m_pparams.PicSort();
        auto_ptr<MvData> mv_data;

        if(psort.IsInter())
            //do all the MV stuff
            DecompressMVData(mv_data, picture_byteio);

        // Read the  transform header
        TransformByteIO transform_byteio(picture_byteio, m_pparams, m_decparams);
        transform_byteio.Input();

        if(m_pparams.PicSort().IsIntra() && m_decparams.ZeroTransform())
        {
            DIRAC_THROW_EXCEPTION(
                ERR_UNSUPPORTED_STREAM_DATA,
                "Intra pictures cannot have Zero-Residual",
                SEVERITY_PICTURE_ERROR);
        }

        // Add a picture to the buffer to decode into
        PushPicture(my_buffer);

        //Reference to the picture being decoded
        Picture& my_picture = my_buffer.GetPicture(m_pparams.PictureNum());

        if(!m_decparams.ZeroTransform())
        {
            //decode components
            Picture& pic = my_buffer.GetPicture(m_pparams.PictureNum());

            CompDecompressor my_compdecoder(m_decparams , pic.GetPparams());

            PicArray* comp_data[3];
            CoeffArray* coeff_data[3];

            const int depth(m_decparams.TransformDepth());
            WaveletTransform wtransform(depth, m_decparams.TransformFilter());

            pic.InitWltData(depth);

            for(int c = 0; c < 3; ++c)
            {
                ComponentByteIO component_byteio((CompSort) c, transform_byteio);
                comp_data[c] = &pic.Data((CompSort) c);
                coeff_data[c] = &pic.WltData((CompSort) c);

                SubbandList& bands = coeff_data[c]->BandList();

                bands.Init(depth , coeff_data[c]->LengthX() , coeff_data[c]->LengthY());
                my_compdecoder.Decompress(&component_byteio, *(coeff_data[c]), bands);

                wtransform.Transform(BACKWARD, *(comp_data[c]), *(coeff_data[c]));
            }
        }
        else
            my_picture.Fill(0);

        if(psort.IsInter())
        {
            Picture* my_pic = &my_buffer.GetPicture(m_pparams.PictureNum());

            const std::vector<int>&  refs = m_pparams.Refs();
            Picture* ref_pics[2];

            ref_pics[0] = &my_buffer.GetPicture(refs[0]);
            if(refs.size() > 1)
                ref_pics[1] = &my_buffer.GetPicture(refs[1]);
            else
                ref_pics[1] = ref_pics[0];

            //motion compensate to add the data back in if we don't have an I picture
            MotionCompensator::CompensatePicture(m_decparams.GetPicPredParams() , ADD , *(mv_data.get()) ,
                                                 my_pic, ref_pics);
        }

        my_picture.Clip();

        if(m_decparams.Verbose())
            std::cout << std::endl;


        //exit success
        return true;

    }// try
    catch(const DiracException& e)
    {
        // skip picture
        throw e;
    }

    //exit failure
    return false;
}

void PictureDecompressor::CleanReferencePictures(PictureBuffer& my_buffer)
{
    if(m_decparams.Verbose())
        std::cout << std::endl << "Cleaning reference buffer: ";
    // Do picture buffer cleaning
    int retd_pnum = m_pparams.RetiredPictureNum();

    if(retd_pnum >= 0 && my_buffer.IsPictureAvail(retd_pnum) && my_buffer.GetPicture(retd_pnum).GetPparams().PicSort().IsRef())
    {
        my_buffer.Remove(retd_pnum);
        if(m_decparams.Verbose())
            std::cout << retd_pnum << " ";
    }
}

void PictureDecompressor::SetMVBlocks()
{
    PicturePredParams& predparams = m_decparams.GetPicPredParams();
    OLBParams olb_params = predparams.LumaBParams(2);
    predparams.SetBlockSizes(olb_params, m_cformat);

    // Calculate the number of macro blocks
    int xnum_sb = (m_decparams.Xl() + predparams.LumaBParams(0).Xbsep() - 1) /
                  predparams.LumaBParams(0).Xbsep();
    int ynum_sb = (m_decparams.Yl() + predparams.LumaBParams(0).Ybsep() - 1) /
                  predparams.LumaBParams(0).Ybsep();

    predparams.SetXNumSB(xnum_sb);
    predparams.SetYNumSB(ynum_sb);

    // Set the number of blocks
    predparams.SetXNumBlocks(4 * xnum_sb);
    predparams.SetYNumBlocks(4 * ynum_sb);

    // Note that we do not have an integral number of superblocks in a picture
    // So it is possible that part of a superblock and some blocks can fall
    // of the edge of the true picture. We need to take this into
    // consideration while doing Motion Compensation
}

void PictureDecompressor::PushPicture(PictureBuffer &my_buffer)
{
    m_pparams.SetCFormat(m_cformat);

    m_pparams.SetXl(m_decparams.Xl());
    m_pparams.SetYl(m_decparams.Yl());

    m_pparams.SetLumaDepth(m_decparams.LumaDepth());
    m_pparams.SetChromaDepth(m_decparams.ChromaDepth());

    my_buffer.PushPicture(m_pparams);
}

void PictureDecompressor::DecompressMVData(std::auto_ptr<MvData>& mv_data,
        PictureByteIO& picture_byteio)
{
    PicturePredParams& predparams = m_decparams.GetPicPredParams();
    MvDataByteIO mvdata_byteio(picture_byteio, m_pparams, predparams);

    // Read in the picture prediction parameters
    mvdata_byteio.Input();

    SetMVBlocks();
    mv_data.reset(new MvData(predparams, m_pparams.NumRefs()));

    // decode mv data
    if(m_decparams.Verbose())
        std::cout << std::endl << "Decoding motion data ...";

    int num_bits;

    // Read in the split mode data header
    mvdata_byteio.SplitModeData()->Input();
    // Read the mode data
    num_bits = mvdata_byteio.SplitModeData()->DataBlockSize();
    SplitModeCodec smode_decoder(mvdata_byteio.SplitModeData()->DataBlock(), TOTAL_MV_CTXS);
    smode_decoder.Decompress(*(mv_data.get()) , num_bits);

    // Read in the prediction mode data header
    mvdata_byteio.PredModeData()->Input();
    // Read the mode data
    num_bits = mvdata_byteio.PredModeData()->DataBlockSize();
    PredModeCodec pmode_decoder(mvdata_byteio.PredModeData()->DataBlock(), TOTAL_MV_CTXS, m_pparams.NumRefs());
    pmode_decoder.Decompress(*(mv_data.get()) , num_bits);

    // Read in the MV1 horizontal data header
    mvdata_byteio.MV1HorizData()->Input();
    // Read the MV1 horizontal data
    num_bits = mvdata_byteio.MV1HorizData()->DataBlockSize();
    VectorElementCodec vdecoder1h(mvdata_byteio.MV1HorizData()->DataBlock(), 1,
                                  HORIZONTAL, TOTAL_MV_CTXS);
    vdecoder1h.Decompress(*(mv_data.get()) , num_bits);

    // Read in the MV1 vertical data header
    mvdata_byteio.MV1VertData()->Input();
    // Read the MV1 data
    num_bits = mvdata_byteio.MV1VertData()->DataBlockSize();
    VectorElementCodec vdecoder1v(mvdata_byteio.MV1VertData()->DataBlock(), 1,
                                  VERTICAL, TOTAL_MV_CTXS);
    vdecoder1v.Decompress(*(mv_data.get()) , num_bits);

    if(m_pparams.NumRefs() > 1)
    {
        // Read in the MV2 horizontal data header
        mvdata_byteio.MV2HorizData()->Input();
        // Read the MV2 horizontal data
        num_bits = mvdata_byteio.MV2HorizData()->DataBlockSize();
        VectorElementCodec vdecoder2h(mvdata_byteio.MV2HorizData()->DataBlock(), 2,
                                      HORIZONTAL, TOTAL_MV_CTXS);
        vdecoder2h.Decompress(*(mv_data.get()) , num_bits);

        // Read in the MV2 vertical data header
        mvdata_byteio.MV2VertData()->Input();
        // Read the MV2 vertical data
        num_bits = mvdata_byteio.MV2VertData()->DataBlockSize();
        VectorElementCodec vdecoder2v(mvdata_byteio.MV2VertData()->DataBlock(), 2,
                                      VERTICAL, TOTAL_MV_CTXS);
        vdecoder2v.Decompress(*(mv_data.get()) , num_bits);
    }

    // Read in the Y DC data header
    mvdata_byteio.YDCData()->Input();
    // Read the Y DC data
    num_bits = mvdata_byteio.YDCData()->DataBlockSize();
    DCCodec ydc_decoder(mvdata_byteio.YDCData()->DataBlock(), Y_COMP, TOTAL_MV_CTXS);
    ydc_decoder.Decompress(*(mv_data.get()) , num_bits);

    // Read in the U DC data header
    mvdata_byteio.UDCData()->Input();
    // Read the U DC data
    num_bits = mvdata_byteio.UDCData()->DataBlockSize();
    DCCodec udc_decoder(mvdata_byteio.YDCData()->DataBlock(), U_COMP, TOTAL_MV_CTXS);
    udc_decoder.Decompress(*(mv_data.get()) , num_bits);

    // Read in the Y DC data header
    mvdata_byteio.YDCData()->Input();
    // Read the Y DC data
    num_bits = mvdata_byteio.YDCData()->DataBlockSize();
    DCCodec vdc_decoder(mvdata_byteio.VDCData()->DataBlock(), V_COMP, TOTAL_MV_CTXS);
    vdc_decoder.Decompress(*(mv_data.get()) , num_bits);
}

void PictureDecompressor::InitCoeffData(CoeffArray& coeff_data, const int xl, const int yl)
{

    // First set the dimensions up //
    int xpad_len = xl;
    int ypad_len = yl;

    // The pic dimensions must be a multiple of 2^(transform depth)
    int tx_mul = (1 << m_decparams.TransformDepth());

    if(xpad_len % tx_mul != 0)
        xpad_len = ((xpad_len / tx_mul) + 1) * tx_mul;
    if(ypad_len % tx_mul != 0)
        ypad_len = ((ypad_len / tx_mul) + 1) * tx_mul;

    coeff_data.Resize(ypad_len, xpad_len);
}

