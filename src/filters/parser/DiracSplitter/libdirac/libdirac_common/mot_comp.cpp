/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: mot_comp.cpp,v 1.47 2009/04/21 01:33:04 asuraparaju Exp $ $Name:  $
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
* Contributor(s): Richard Felton (Original Author),
*                 Thomas Davies,
*                 Steve Bearcroft
*                 Mike Ferenduros
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


#include <libdirac_common/mot_comp.h>
#if defined(HAVE_MMX)
#include <libdirac_common/mot_comp_mmx.h>
#endif
#include <libdirac_common/motion.h>
#include <libdirac_common/picture_buffer.h>
using namespace dirac;

using std::vector;


#define NUM_USED_BLKS(w,sep,len) ((w+sep+(len-sep)/2-1)/sep)

//--public member functions--//
///////////////////////////////

// Convenience function to perform motion compensation on a picture
// Static function that motion compensates a picture. It uses the
// MV precision value in the PicturePredParams to instantiate the
// appropriate MotionCompensation sub-class.
void MotionCompensator::CompensatePicture(const PicturePredParams &ppp,
        const AddOrSub direction ,
        const MvData& mv_data,
        Picture* in_pic ,
        Picture* refsptr[2])
{
    switch(ppp.MVPrecision())
    {
    case MV_PRECISION_EIGHTH_PIXEL:
    {
        MotionCompensator_EighthPixel my_comp(ppp);
        my_comp.CompensatePicture(direction , mv_data, in_pic, refsptr);
        break;
    }
    case MV_PRECISION_HALF_PIXEL:
    {
        MotionCompensator_HalfPixel my_comp(ppp);
        my_comp.CompensatePicture(direction , mv_data, in_pic, refsptr);
        break;
    }
    case MV_PRECISION_PIXEL:
    {
        MotionCompensator_Pixel my_comp(ppp);
        my_comp.CompensatePicture(direction , mv_data, in_pic, refsptr);
        break;
    }
    case MV_PRECISION_QUARTER_PIXEL:
    default:
    {
        MotionCompensator_QuarterPixel my_comp(ppp);
        my_comp.CompensatePicture(direction , mv_data, in_pic, refsptr);
        break;
    }
    }

    return;
}

// Constructor
// Initialises the lookup tables that is needed for motion
// motion compensation. Creates the necessary arithmetic objects and
// calls ReConfig to create weighting blocks to fit the values within
// m_predparams.
MotionCompensator::MotionCompensator(const PicturePredParams &ppp):
    m_predparams(ppp),
    luma_or_chroma(true)
{
    // Allocate for block weights
    m_block_weights = new TwoDArray<ValueType>[9];
    // Allocate for superblock weights
    m_macro_block_weights = new TwoDArray<ValueType>[9];
    // Allocate for sub superblock weights
    m_sub_block_weights = new TwoDArray<ValueType>[9];

    //Configure weighting blocks for the first time
    ReConfig();
}

// Destructor
MotionCompensator::~MotionCompensator()
{
    //Tidy up the pointers
    delete[] m_block_weights;
    delete[] m_macro_block_weights;
    delete[] m_sub_block_weights;
}

//Called to perform motion compensated addition/subtraction on an entire picture.
void MotionCompensator::CompensatePicture(const AddOrSub direction ,
        const MvData& mv_data,
        Picture* my_picture ,
        Picture* refsptr[2])
{
    m_add_or_sub = direction;

    const PictureSort& psort = my_picture->GetPparams().PicSort();

    m_cformat = my_picture->GetPparams().CFormat();

    if(psort.IsInter())
    {
        //we can motion compensate

        const std::vector<int>& refs = my_picture->GetPparams().Refs();

        // Now check that references are marked correctly
        if(!refsptr[0]->GetPparams().PicSort().IsRef())
        {
            std::cout << std::endl << "WARNING! Reference picture (number " << refs[0];
            std::cout << ") being used is not marked as a reference. Incorrect output is likely.";
        }
        if(refsptr[0]->GetPparams().PictureNum() != refs[0])
        {
            std::cout << std::endl << "WARNING! Reference picture number 0 ";
            std::cout << "does not agree(" << refsptr[0]->GetPparams().PictureNum() << " and ";
            std::cout << refs[0] << "). Incorrect output is likely.";
        }


        if(refs.size() > 1)
        {
            if(!refsptr[1]->GetPparams().PicSort().IsRef())
            {
                std::cout << std::endl << "WARNING! Reference picture (number ";
                std::cout << refs[1] << ") being used is not marked as a reference. Incorrect output is likely.";
            }
            if(refsptr[1]->GetPparams().PictureNum() != refs[1])
            {
                std::cout << std::endl << "WARNING! Reference picture number 1 ";
                std::cout << "does not agree(" << refsptr[1]->GetPparams().PictureNum() << " and ";
                std::cout << refs[1] << "). Incorrect output is likely.";
            }
        }
        else
            refsptr[1] = refsptr[0];

        luma_or_chroma = true;
        //now do all the components
        CompensateComponent(my_picture , refsptr, mv_data , Y_COMP);

        luma_or_chroma = false;
        CompensateComponent(my_picture  , refsptr, mv_data , U_COMP);
        CompensateComponent(my_picture  , refsptr, mv_data , V_COMP);
    }
}

//--private member functions--//
////////////////////////////////

//Needs to be called if the blocksize changes (and
//on startup). This method creates an array of weighting
//blocks that are used to acheive correctly overlapping
//blocks.
void MotionCompensator::ReConfig()
{
    if(luma_or_chroma)
        m_bparams = m_predparams.LumaBParams(2);
    else
        m_bparams = m_predparams.ChromaBParams(2);

    // Calculate the shift required in horizontal and vertical direction for
    // OBMC and the weighting bits for each reference picture.

    // Total shift = shift assuming equal picture weights +
    //               picture weights precision
    int blocks_per_mb_row = m_predparams.XNumBlocks() / m_predparams.XNumSB();
    int blocks_per_sb_row = blocks_per_mb_row >> 1;
    int mb_xlen = m_bparams.Xblen() * blocks_per_mb_row - (m_bparams.Xblen() - m_bparams.Xbsep()) * (blocks_per_mb_row - 1);
    int mb_ylen = m_bparams.Yblen();
    int mb_xsep = mb_xlen - (m_bparams.Xblen() - m_bparams.Xbsep());
    int mb_ysep = m_bparams.Ybsep();
    int sb_xlen = m_bparams.Xblen() * blocks_per_sb_row - (m_bparams.Xblen() - m_bparams.Xbsep()) * (blocks_per_sb_row - 1);
    int sb_ylen = m_bparams.Yblen();
    int sb_xsep = sb_xlen - (m_bparams.Xblen() - m_bparams.Xbsep());
    int sb_ysep = m_bparams.Ybsep();

    for(int i = 0; i < 9; i++)
    {
        m_block_weights[i].Resize(m_bparams.Yblen() , m_bparams.Xblen());
        m_macro_block_weights[i].Resize(mb_ylen , mb_xlen);
        m_sub_block_weights[i].Resize(sb_ylen , sb_xlen);
    }

    // Firstly calculate the non-weighted Weighting blocks. i,e, assuming that
    // the picture_weight for each reference picture is 1.

    // Calculate non-weighted Block Weights
    CalculateWeights(m_bparams.Xbsep(), m_bparams.Ybsep(), m_block_weights);

    // Calculate non-weighted "macro" Block Weights
    CalculateWeights(mb_xsep, mb_ysep , m_macro_block_weights);

    // Calculate non-weighted superblock Weights
    CalculateWeights(sb_xsep, sb_ysep , m_sub_block_weights);
}

void MotionCompensator::CompensateComponent(Picture* pic ,
        Picture* refsptr[2] ,
        const MvData& mv_data ,
        const CompSort cs)
{
    // Set up references to pictures and references
    PicArray& pic_data_out = pic->Data(cs);

    // Size of picture component being motion compensated

    const PicArray& ref1up = refsptr[0]->UpData(cs);
    const PicArray& ref2up = refsptr[1]->UpData(cs);

    // Set up a row of blocks which will contain the MC data, which
    // we'll add or subtract to pic_data_out
    TwoDArray<ValueType> pic_data(m_bparams.Yblen(), pic_data_out.LengthX(), 0);

    // Factors to compensate for subsampling of chroma
    int xscale_shift = 0;
    int yscale_shift = 0;
    if(cs != Y_COMP)
    {
        if(m_cformat == format420)
        {
            xscale_shift = 1;
            yscale_shift = 1;
        }
        else if(m_cformat == format422)
        {
            xscale_shift = 1;
            yscale_shift = 0;
        }
    }

    ImageCoords pic_size(pic->GetPparams().Xl(), pic->GetPparams().Yl());
    if(cs != Y_COMP)
    {
        pic_size.x = pic->GetPparams().ChromaXl();
        pic_size.y = pic->GetPparams().ChromaYl();
    }


    // Reference to the relevant DC array
    const TwoDArray<ValueType>& dcarray = mv_data.DC(cs);

    // Set up references to the vectors
    const int num_refs = pic->GetPparams().Refs().size();
    const MvArray* mv_array1;
    const MvArray* mv_array2;
    mv_array1 = &mv_data.Vectors(1);
    if(num_refs == 2)
        mv_array2 = &mv_data.Vectors(2);
    else
        mv_array2 = &mv_data.Vectors(1);

    ReConfig();//set all the weighting blocks up

    //Blocks are listed left to right, line by line.
    MVector mv1, mv2;
    PredMode block_mode;

    //Coords of the top-left corner of a block
    ImageCoords pos;

    //Loop for each block in the output image.
    //The CompensateBlock function will use the image pointed to by ref1up
    //and add the compensated pixels to the image pointed to by pic_data.
    size_t wgt_idx;

    int save_from_row = m_bparams.Ybsep() - m_bparams.Yoffset();

    bool row_overlap = ((m_bparams.Yblen() - m_bparams.Ybsep()) > 0);

    // unpadded picture dimensions
    const int x_end_data = pic_data_out.FirstX() + std::min(pic_data_out.LengthX(), pic_size.x);
    const int y_end_data = pic_data_out.FirstY() + std::min(pic_data_out.LengthY(), pic_size.y);

    const int blocks_per_mb_row = m_predparams.XNumBlocks() / m_predparams.XNumSB();
    const int blocks_per_sb_row = blocks_per_mb_row >> 1;

    // The picture does not contain integral number of blocks. So not all
    // blocks need to be processed. Compute the relevant blocks to be
    // processed
    int y_num_blocks = std::min((NUM_USED_BLKS(pic_size.y, m_bparams.Ybsep(), m_bparams.Yblen())),
                                m_predparams.YNumBlocks());
    int x_num_blocks = std::min((NUM_USED_BLKS(pic_size.x, m_bparams.Xbsep(), m_bparams.Xblen())),
                                m_predparams.XNumBlocks());

    //Loop over all the block rows
    pos.y = -m_bparams.Yoffset();
    for(int yblock = 0; yblock < y_num_blocks; ++yblock)
    {
        pos.x = -m_bparams.Xoffset();
        int xincr, xb_incr = 0;
        //loop over all the blocks in a row
        for(int xblock = 0 ; xblock < x_num_blocks; xblock += xb_incr)
        {
            int split_mode =  mv_data.SBSplit()[yblock/blocks_per_mb_row][xblock/blocks_per_mb_row];

            int blk_x, blk_y = 1;

            switch(split_mode)
            {
            case 0: // processing superblock
                blk_x = blocks_per_mb_row;
                break;
            case 1: // processing sub-superblock
                blk_x = blocks_per_sb_row;
                break;
            case 2: // processing block
            default:
                blk_x = 1;
                break;
            }

            //Decide which weights to use.
            if(pos.x >= 0 && (xblock + blk_x) < x_num_blocks)
            {
                // block is entirely within picture in x direction
                if(pos.y < 0)
                    wgt_idx = 1;
                else if((yblock + blk_y) < y_num_blocks)
                    wgt_idx = 4;
                else
                    wgt_idx = 7;
            }
            else if(pos.x < 0)
            {
                // left edge of block is outside picture in x direction
                if(pos.y < 0)
                    wgt_idx = 0;
                else if((yblock + blk_y) < y_num_blocks)
                    wgt_idx = 3;
                else
                    wgt_idx = 6;
            }
            else
            {
                // right edge of block is outside picture in x direction
                if(pos.y < 0)
                    wgt_idx = 2;
                else if((yblock + blk_y) < y_num_blocks)
                    wgt_idx = 5;
                else
                    wgt_idx = 8;
            }


            block_mode = mv_data.Mode()[yblock][xblock];

            TwoDArray<ValueType> *wt;

            if(split_mode == 0)  //Block part of a MacroBlock
            {
                wt = &m_macro_block_weights[wgt_idx];
                xb_incr = blocks_per_mb_row;
            }
            else if(split_mode == 1)  //Block part of a SubBlock
            {
                wt = &m_sub_block_weights[wgt_idx];
                xb_incr = blocks_per_sb_row;
            }
            else
            {
                wt = &m_block_weights[wgt_idx];
                xb_incr = 1;
            }
            xincr = m_bparams.Xbsep() * xb_incr;

            mv1 = (*mv_array1)[yblock][xblock];
            mv1.x >>= xscale_shift;
            mv1.y >>= yscale_shift;

            mv2 = (*mv_array2)[yblock][xblock];
            mv2.x >>= xscale_shift;
            mv2.y >>= yscale_shift;

            CompensateBlock(pic_data, pos, pic_size, block_mode, dcarray[yblock][xblock], ref1up, mv1, ref2up, mv2, *wt);

            //Increment the block horizontal position
            pos.x += xincr;

        }//xblock

        // Update the pic data
        // Use only the first Ybsep rows since the remaining rows are
        // needed for the next row of blocks since we are using overlapped
        // blocks motion compensation
        if(m_add_or_sub == SUBTRACT)
        {
            int start_y = std::max(pic_data_out.FirstY() , pos.y) ;
            int end_y = std::min(pic_data_out.FirstY() + pos.y + m_bparams.Ybsep() , y_end_data);

            if(yblock == y_num_blocks - 1)
            {
                end_y = pic_data_out.LengthY();
                if(end_y > y_end_data)
                    end_y = y_end_data;

            }

            for(int i = start_y, pos_y = 0; i < end_y; i++, pos_y++)
            {
                ValueType *pic_row = pic_data[pos_y];
                ValueType *out_row = pic_data_out[i];

                for(int j = pic_data_out.FirstX(); j < x_end_data; ++j)
                {
                    out_row[j] -= static_cast<ValueType>((pic_row[j] + 32) >> 6);
                }

                // Okay, we've done all the actual blocks. Now if the picture is further padded
                // we need to set the padded values to zero beyond the last block in the row,
                // for all the picture lines in the block row. Need only do this when we're
                // subtracting.

                for(int j = pic_size.x; j < pic_data_out.LengthX() ; ++j)
                {
                    out_row[pic_data_out.FirstX()+j] = 0;
                }
            }
        }
        else // (m_add_or_sub == ADD)
        {
            int start_y = std::max(pic_data_out.FirstY() , pos.y) ;
            int end_y = std::min(pic_data_out.FirstY() + pos.y + m_bparams.Ybsep() , pic_data_out.FirstY() + pic_data_out.LengthY());
            if(yblock == (y_num_blocks - 1))
            {
                end_y += (m_bparams.Yblen() - m_bparams.Ybsep());
                if(end_y > pic_size.y)
                    end_y = pic_size.y;
            }
#if defined (HAVE_MMX)
            CompensateComponentAddAndShift_mmx(start_y, end_y, 6, pic_size,
                                               pic_data, pic_data_out);
#else
            for(int i = start_y, pic_y = 0; i < end_y; i++, pic_y++)
            {
                ValueType *pic_row = pic_data[pic_y];
                ValueType *out_row = pic_data_out[i];

                for(int j = 0; j < pic_size.x; j++)
                {
                    out_row[j] += static_cast<ValueType>((pic_row[j] + 32) >> 6);
                }
                // Pad the remaining pixels of the row with last truepic pixel val
                for(int j = pic_size.x; j < pic_data.LengthX(); j++)
                {
                    out_row[j] = out_row[pic_size.x-1];
                }
            }
#endif
        }
        //Increment the block vertical position
        pos.y += m_bparams.Ybsep();

        if(row_overlap)
        {
            // Copy the rows required to motion compensate the next row of
            // blocks. This is usually Yblen-Ybsep rows.
            memmove(pic_data[0], pic_data[save_from_row], (m_bparams.Yblen() - save_from_row)*pic_data.LengthX()*sizeof(ValueType));
            memset(pic_data[m_bparams.Yblen() - save_from_row], 0, save_from_row * pic_data.LengthX()*sizeof(ValueType));
            save_from_row = m_bparams.Ybsep();
        }
        else
        {
            // no row overlap. So reset pic_data to 0.
            memset(pic_data[0], 0,  m_bparams.Yblen()*pic_data.LengthX()*sizeof(ValueType));
        }
    }//yblock

    if(m_add_or_sub == SUBTRACT)
    {
        // Finally, now we've done all the blocks, we must set all padded lines
        // below the last row equal to 0, if we're subtracting
        for(int y = pic_size.y ; y < pic_data_out.LengthY() ; ++y)
        {
            ValueType *out_row = pic_data_out[y];
            for(int x = 0 ; x < pic_data_out.LengthX() ; ++x)
            {
                out_row[x] = 0;
            }

        }
    }
    else if(m_add_or_sub == ADD)
    {
        // Edge extension
        // Finally, now we've done all the blocks, we must set all padded lines
        // below the last row equal to same as last row, if we're adding
        ValueType *last_row = &pic_data_out[pic_size.y-1][0];
        for(int y = pic_size.y ; y < pic_data_out.LengthY() ; ++y)
        {
            ValueType *out_row = pic_data_out[y];
            for(int x = 0 ; x < pic_data_out.LengthX() ; ++x)
            {
                out_row[x] = last_row[x];
            }

        }
    }
}

void MotionCompensator::CompensateBlock(
    TwoDArray<ValueType> &pic_data ,
    const ImageCoords& pos ,
    const ImageCoords& pic_size ,
    PredMode block_mode,
    ValueType dc,
    const PicArray &ref1up_data ,
    const MVector &mv1 ,
    const PicArray &ref2up_data ,
    const MVector &mv2 ,
    const TwoDArray<ValueType>& wt_array)
{
    //Coordinates in the image being written to.
    const ImageCoords start_pos(std::max(pos.x, 0) , std::max(pos.y, 0));
    const ImageCoords end_pos(std::min(pos.x + wt_array.LengthX() , pic_size.x) ,
                              std::min(pos.y + wt_array.LengthY() , pic_size.y));

    // Check if we are within original picture bounds
    if(start_pos.x >= end_pos.x || start_pos.y >= end_pos.y)
        return;

    TwoDArray<ValueType> val1(end_pos.y - start_pos.y, end_pos.x - start_pos.x);
    TwoDArray<ValueType> val2(end_pos.y - start_pos.y, end_pos.x - start_pos.x);

    if(block_mode == REF1_ONLY)
    {
        BlockPixelPred(val1, pos, pic_size, ref1up_data, mv1);
    }
    else if(block_mode == REF2_ONLY)
    {
        BlockPixelPred(val1, pos, pic_size, ref2up_data, mv2);
    }
    else if(block_mode == REF1AND2)
    {
        BlockPixelPred(val1, pos, pic_size, ref1up_data, mv1);
        BlockPixelPred(val2, pos, pic_size, ref2up_data, mv2);
    }
    else
    {
        //we have a DC block.
        DCBlock(val1, dc);
    }
    /*
    * Multiply the block by reference weights. Return result in val1
    */
    AdjustBlockByRefWeights(val1, val2, block_mode);

    /*
    * Multiply the block by OBMC spatial weights. Return result val1
    */
    AdjustBlockBySpatialWeights(val1, pos, wt_array);

#if !defined (HAVE_MMX)
    for(int y = 0, py = 0; y < val1.LengthY(); ++y, ++py)
    {
        for(int x = 0, px = start_pos.x; x < val1.LengthX(); ++x, ++px)
        {
            pic_data[py][px] += val1[y][x];
        }
    }
#else
    AddMCBlock_mmx(ImageCoords(start_pos.x, 0), pic_data, val1);
#endif
}

void MotionCompensator::DCBlock(TwoDArray<ValueType> &block_data ,
                                const ValueType dc)
{

    //Quick process where we can just copy from the double size image.
    ValueType *block_curr = &block_data[0][0];
    for(int y = 0; y < block_data.LengthY(); ++y)
    {
        for(int x = 0; x < block_data.LengthX(); ++x, ++block_curr)
        {
            *block_curr = dc;
        }
    }
}

void MotionCompensator::AdjustBlockByRefWeights(
    TwoDArray<ValueType>& val1_block,
    TwoDArray<ValueType>& val2_block,
    PredMode block_mode)
{
    // No need to multiply by reference weights if DC block
    if(block_mode == INTRA)
        return;

    if(m_predparams.CustomRefWeights())
    {
        int ref_wt_prec_bias = 1;
        for(int i = m_predparams.PictureWeightsBits() - 1; i > 0; --i)
        {
            ref_wt_prec_bias <<= 1;
        }
        if(block_mode != REF1AND2)
        {
            for(int y = 0; y < val1_block.LengthY(); ++y)
            {
                for(int x = 0; x < val1_block.LengthX(); ++x)
                {
                    val1_block[y][x] *= (m_predparams.Ref1Weight() +
                                         m_predparams.Ref2Weight());
                }
            }
        }
        else
        {
            for(int y = 0; y < val1_block.LengthY(); ++y)
            {
                for(int x = 0; x < val1_block.LengthX(); ++x)
                {
                    val1_block[y][x] *= m_predparams.Ref1Weight();
                    val2_block[y][x] *= m_predparams.Ref2Weight();
                    val1_block[y][x] += val2_block[y][x];
                }
            }
        }
        for(int y = 0; y < val1_block.LengthY(); ++y)
        {
            for(int x = 0; x < val1_block.LengthX(); ++x)
            {
                val1_block[y][x] = (val1_block[y][x] + ref_wt_prec_bias) >> m_predparams.PictureWeightsBits();
            }
        }
    }
    else
    {
        // Default weights
        if(block_mode == REF1AND2)
        {
            for(int y = 0; y < val1_block.LengthY(); ++y)
            {
                for(int x = 0; x < val1_block.LengthX(); ++x)
                {
                    val1_block[y][x] = (val1_block[y][x] + val2_block[y][x] + 1) >> 1;
                }
            }
        }
        // Nothing to do for other block modes when using default weights
    }
}

#if !defined (HAVE_MMX)
void MotionCompensator::AdjustBlockBySpatialWeights(
    TwoDArray<ValueType>& val_block,
    const ImageCoords &pos,
    const TwoDArray<ValueType> &wt_array)
{
    ImageCoords start_pos(std::max(0, pos.x), std::max(0, pos.y));
    ImageCoords wt_start(start_pos.x - pos.x, start_pos.y - pos.y);

    for(int y = 0, wt_y = wt_start.y; y < val_block.LengthY(); ++y, ++wt_y)
    {
        for(int x = 0, wt_x = wt_start.x; x < val_block.LengthX(); ++x, ++wt_x)
        {
            val_block[y][x] *= wt_array[wt_y][wt_x];
        }
    }
}
#endif

void MotionCompensator::CalculateWeights(int xbsep, int ybsep,
        TwoDArray<ValueType>* wts_array)
{
    // Firstly calculate the non-weighted Weighting blocks. i,e, assuming that
    // the picture_weight for each reference picture is 1.
    // We can create all nine weighting blocks by calculating values
    // for four blocks and mirroring them to generate the others.
    CreateBlock(xbsep, ybsep, false , false , wts_array[0]);
    CreateBlock(xbsep, ybsep, false , true , wts_array[3]);
    CreateBlock(xbsep, ybsep, true , false , wts_array[1]);
    CreateBlock(xbsep, ybsep, true , true , wts_array[4]);

    // Note order of flipping is important.
    FlipX(wts_array[3] , wts_array[5]);
    FlipX(wts_array[0] , wts_array[2]);
    FlipY(wts_array[0] , wts_array[6]);
    FlipX(wts_array[6] , wts_array[8]);
    FlipY(wts_array[1] , wts_array[7]);

}
// Calculates a weighting block.
// bparams defines the block parameters so the relevant weighting arrays can
// be created.
// FullX and FullY refer to whether the weight should be adjusted for the
// edge of an image.
// eg. 1D Weighting shapes in x direction

//  FullX true        FullX false
//     ***           ********
//   *     *                  *
//  *       *                  *
//*           *                  *
void MotionCompensator::CreateBlock(int xbsep, int ybsep,
                                    bool FullX , bool FullY ,
                                    TwoDArray<ValueType>& WeightArray)
{
    // Create temporary arrays
    int xblen = WeightArray.LengthX();
    int yblen = WeightArray.LengthY();

    OneDArray<ValueType> HWts(xblen);
    OneDArray<ValueType> VWts(yblen);

    // Window in the x direction
    int xoffset = (xblen - xbsep) / 2;
    if(xoffset != 1)
    {
        for(int x = 0; x < 2 * xoffset; ++x)
        {
            HWts[x] = 1 + (6 * x + xoffset - 1) / (2 * xoffset - 1);
            HWts[x+xbsep] = 8 - HWts[x];
        }// x
    }
    else
    {
        HWts[0] = HWts[1+xbsep] = 3;
        HWts[1] = HWts[xbsep] = 5;
    }
    for(int x = 2 * xoffset; x < xbsep; ++x)
        HWts[x] = 8;

    // Window in the y direction
    int yoffset = (yblen - ybsep) / 2;
    if(yoffset != 1)
    {
        for(int y = 0; y < 2 * yoffset; ++y)
        {
            VWts[y] = 1 + (6 * y + yoffset - 1) / (2 * yoffset - 1);
            VWts[y+ybsep] = 8 - VWts[y];
        }// y
    }
    else
    {
        VWts[0] = VWts[1+ybsep] = 3;
        VWts[1] = VWts[ybsep] = 5;
    }
    for(int y = 2 * yoffset; y < ybsep; ++y)
        VWts[y] = 8;

    // Now reflect or pad, as appropriate
    if(!FullX)
    {
        for(int x = 0; x < 2 * xoffset; ++x)
            HWts[x] = 8;
    }

    // Reflect or pad, as appropriate
    if(!FullY)
    {
        for(int y = 0 ; y < 2 * yoffset; ++y)
            VWts[y] = 8;
    }

    for(int y = 0; y < yblen; ++y)
    {
        for(int x = 0; x < xblen; ++x)
        {
            WeightArray[y][x] = VWts[y] * HWts[x];
        }// x
    }// y
}

// Flips the values in an array in the x direction.
void MotionCompensator::FlipX(const TwoDArray<ValueType>& Original ,
                              TwoDArray<ValueType>& Flipped)
{
    int yblen = Original.LengthY();
    int xblen = Original.LengthX();
    for(int y = 0; y < yblen; ++y)
    {
        for(int x = 0; x < xblen; ++x)
        {
            Flipped[y][x] = Original[y][(xblen-1) - x];
        }// y
    }// x
}

// Flips the values in an array in the y direction.
void MotionCompensator::FlipY(const TwoDArray<ValueType>& Original ,
                              TwoDArray<ValueType>& Flipped)
{
    int yblen = Original.LengthY();
    int xblen = Original.LengthX();
    for(int y = 0; y < yblen; ++y)
    {
        for(int x = 0; x < xblen; ++x)
        {
            Flipped[y][x] = Original[(yblen-1) - y][x];
        }// y
    }// x
}


// Concrete Sub-Classes
// Class that implement the BlockPixelPred function based on pixel
// precision values

// Motion Compesation class that provides pixel precision compensation

MotionCompensator_Pixel::MotionCompensator_Pixel(const PicturePredParams &ppp) :
    MotionCompensator(ppp)
{}

void MotionCompensator_Pixel::BlockPixelPred(
    TwoDArray<ValueType> &block_data ,
    const ImageCoords& pos,
    const ImageCoords& pic_size ,
    const PicArray &refup_data ,
    const MVector &mv)
{
    //Coordinates in the image being written to.
    const ImageCoords start_pos(std::max(pos.x, 0) , std::max(pos.y, 0));

    //Where to start in the upconverted image - scaled since ref is upconverted
    const ImageCoords ref_start((start_pos.x + mv.x) << 1 , (start_pos.y + mv.y) << 1);

    //An additional stage to make sure the block to be copied does not fall outside
    //the reference image.
    const int refXlen = refup_data.LengthX();
    //const int refYlen = refup_data.LengthY();
    const int trueRefXlen = (pic_size.x << 1) - 1;
    const int trueRefYlen = (pic_size.y << 1) - 1;
    bool do_bounds_checking = false;

    //Check if there are going to be any problems copying the block from
    //the upvconverted reference image.
    if(ref_start.x < 0)
        do_bounds_checking = true;
    else if(ref_start.x + ((block_data.LengthX() - 1) << 1) >= trueRefXlen)
        do_bounds_checking = true;
    if(ref_start.y < 0)
        do_bounds_checking = true;
    else if(ref_start.y + ((block_data.LengthY() - 1) << 1) >= trueRefYlen)
        do_bounds_checking = true;

    ValueType *block_curr = &block_data[0][0];
    if(!do_bounds_checking)
    {
        ValueType *refup_curr = &refup_data[ref_start.y][ref_start.x];
        const int refup_next(2 *(refXlen - block_data.LengthX()));          // - go down a row and back up
        for(int y = 0; y < block_data.LengthY(); ++y, refup_curr += refup_next)
        {
            for(int x = 0; x < block_data.LengthX(); ++x, ++block_curr, refup_curr += 2)
            {
                *block_curr = *refup_curr;
            }// x
        }// y
    }
    else
    {
        // We're doing bounds checking because we'll fall off the edge of the reference otherwise.

        for(int y = 0, ry = ref_start.y, by = BChk(ry, trueRefYlen);
            y < block_data.LengthY(); ++y, ry += 2 , by = BChk(ry, trueRefYlen))
        {
            for(int x = 0 , rx = ref_start.x , bx = BChk(rx, trueRefXlen);
                x < block_data.LengthX() ; ++x, ++block_curr, rx += 2 , bx = BChk(rx, trueRefXlen))
            {
                *block_curr =  refup_data[by][bx];
            }// x
        }// y

    }
}


// Motion Compesation class that provides half-pixel precision compensation
MotionCompensator_HalfPixel::MotionCompensator_HalfPixel(const PicturePredParams &ppp) :
    MotionCompensator(ppp)
{}

#if !defined (HAVE_MMX)
void MotionCompensator_HalfPixel::BlockPixelPred(
    TwoDArray<ValueType> &block_data ,
    const ImageCoords& pos ,
    const ImageCoords& pic_size ,
    const PicArray &refup_data ,
    const MVector &mv)
{
    //Where to start in the upconverted image
    const ImageCoords start_pos(std::max(pos.x, 0) , std::max(pos.y, 0));
    const ImageCoords ref_start((start_pos.x << 1) + mv.x , (start_pos.y << 1) + mv.y);

    //An additional stage to make sure the block to be copied does not fall
    //outsidethe reference image.
    const int refXlen = refup_data.LengthX();
    //const int refYlen = refup_data.LengthY();
    const int trueRefXlen = (pic_size.x << 1) - 1;
    const int trueRefYlen = (pic_size.y << 1) - 1;

    bool do_bounds_checking = false;

    //Check if there are going to be any problems copying the block from
    //the upvconverted reference image.

    if(ref_start.x < 0)
        do_bounds_checking = true;
    else if(ref_start.x + ((block_data.LengthX() - 1) << 1) >= trueRefXlen)
        do_bounds_checking = true;
    if(ref_start.y < 0)
        do_bounds_checking = true;
    else if(ref_start.y + ((block_data.LengthY() - 1) << 1) >= trueRefYlen)
        do_bounds_checking = true;

    ValueType *block_curr = &block_data[0][0];

    if(!do_bounds_checking)
    {
        ValueType *refup_curr = &refup_data[ref_start.y][ref_start.x];
        const int refup_next((refXlen - block_data.LengthX()) * 2);// go down 2 rows and back up

        for(int y = 0; y < block_data.LengthY(); ++y, refup_curr += refup_next)
        {
            for(int x = 0; x < block_data.LengthX(); ++x, ++block_curr, refup_curr += 2)
            {
                *block_curr = refup_curr[0];
            }
        }
    }
    else
    {
        // We're doing bounds checking because we'll fall off the edge of the reference otherwise.
        for(int y = 0, ry = ref_start.y, by = BChk(ry, trueRefYlen);
            y < block_data.LengthY(); ++y, ry += 2 , by = BChk(ry, trueRefYlen))
        {
            for(int x = 0 , rx = ref_start.x , bx = BChk(rx, trueRefXlen);
                x < block_data.LengthX() ;
                ++x, ++block_curr, rx += 2 , bx = BChk(rx, trueRefXlen))
            {
                *block_curr =  refup_data[by][bx];
            }// x
        }// y
    }
}
#endif

// Motion Compesation class that provides quarter-pixel precision compensation
MotionCompensator_QuarterPixel::MotionCompensator_QuarterPixel(const PicturePredParams &ppp) :
    MotionCompensator(ppp)
{}

#if !defined (HAVE_MMX)
void MotionCompensator_QuarterPixel::BlockPixelPred(
    TwoDArray<ValueType> &block_data ,
    const ImageCoords& pos ,
    const ImageCoords& pic_size ,
    const PicArray &refup_data ,
    const MVector &mv)
{
    // Set up the start point in the reference image by rounding the motion vector
    // to 1/2 pel accuracy.NB: bit shift rounds negative values DOWN, as required
    const MVector roundvec(mv.x >> 1 , mv.y >> 1);

    //Get the remainder after rounding. NB rmdr values always 0 or 1
    const MVector rmdr(mv.x & 1 , mv.y & 1);

    //Where to start in the upconverted image
    const ImageCoords start_pos(std::max(pos.x, 0) , std::max(pos.y, 0));
    const ImageCoords ref_start((start_pos.x << 1) + roundvec.x , (start_pos.y << 1) + roundvec.y);

    //An additional stage to make sure the block to be copied does not fall outside
    //the reference image.
    const int refXlen = refup_data.LengthX();
    //const int refYlen = refup_data.LengthY();
    const int trueRefXlen = (pic_size.x << 1) - 1;
    const int trueRefYlen = (pic_size.y << 1) - 1;

    ValueType *block_curr = &block_data[0][0];

    bool do_bounds_checking = false;
    //Check if there are going to be any problems copying the block from
    //the upvconverted reference image.
    if(ref_start.x < 0)
        do_bounds_checking = true;
    else if(ref_start.x + (block_data.LengthX() << 1) >= trueRefXlen)
        do_bounds_checking = true;
    if(ref_start.y < 0)
        do_bounds_checking = true;
    else if(ref_start.y + (block_data.LengthY() << 1) >= trueRefYlen)
        do_bounds_checking = true;

    if(!do_bounds_checking)
    {
        ValueType *refup_curr = &refup_data[ref_start.y][ref_start.x];
        const int refup_next((refXlen - block_data.LengthX()) * 2);   //go down 2 rows and back to beginning of block line
        if(rmdr.x == 0 && rmdr.y == 0)
        {
            for(int y = 0; y < block_data.LengthY(); ++y, refup_curr += refup_next)
            {
                for(int x = 0; x < block_data.LengthX(); ++x, ++block_curr, refup_curr += 2)
                {
                    *block_curr = refup_curr[0];
                }
            }
        }
        else if(rmdr.y == 0)
        {
            for(int y = 0; y < block_data.LengthY(); ++y, refup_curr += refup_next)
            {
                for(int x = 0; x < block_data.LengthX(); ++x, ++block_curr, refup_curr += 2)
                {
                    *block_curr = (refup_curr[0]  +  refup_curr[1]  + 1) >> 1;
                }
            }
        }
        else if(rmdr.x == 0)
        {
            for(int y = 0; y < block_data.LengthY(); ++y, refup_curr += refup_next)
            {
                for(int x = 0; x < block_data.LengthX(); ++x, ++block_curr, refup_curr += 2)
                {
                    *block_curr = (refup_curr[0] + refup_curr[refXlen] + 1) >> 1;
                }
            }
        }
        else
        {
            for(int y = 0; y < block_data.LengthY(); ++y, refup_curr += refup_next)
            {
                for(int x = 0; x < block_data.LengthX(); ++x, ++block_curr, refup_curr += 2)
                {
                    *block_curr = (refup_curr[0] +  refup_curr[1]  +
                                   refup_curr[refXlen+0] +
                                   refup_curr[refXlen+1]  + 2) >> 2;
                }
            }
        }
    }
    else
    {
        // We're doing bounds checking because we'll fall off the edge of the reference otherwise.

        //weights for doing linear interpolation, calculated from the remainder values
        const ValueType linear_wts[4] = {  (2 - rmdr.x) *(2 - rmdr.y),     //tl
                                           rmdr.x *(2 - rmdr.y),           //tr
                                           (2 - rmdr.x) * rmdr.y,          //bl
                                           rmdr.x * rmdr.y
                                        };              //br


        for(int c = 0, uY = ref_start.y, BuY = BChk(uY, trueRefYlen), BuY1 = BChk(uY + 1, trueRefYlen);
            c < block_data.LengthY(); ++c, uY += 2, BuY = BChk(uY, trueRefYlen), BuY1 = BChk(uY + 1, trueRefYlen))
        {
            for(int l = 0, uX = ref_start.x, BuX = BChk(uX, trueRefXlen), BuX1 = BChk(uX + 1, trueRefXlen);
                l < block_data.LengthX(); ++l, uX += 2, BuX = BChk(uX, trueRefXlen), BuX1 = BChk(uX + 1, trueRefXlen))
            {

                block_data[c][l] = (linear_wts[0] * refup_data[BuY][BuX] +
                                    linear_wts[1] * refup_data[BuY][BuX1] +
                                    linear_wts[2] * refup_data[BuY1][BuX] +
                                    linear_wts[3] * refup_data[BuY1][BuX1] +
                                    2
                                   ) >> 2;
            }//l
        }//c

    }
}
#endif

// Motion Compesation class that provides one eighth-pixel precision
// compensation
MotionCompensator_EighthPixel::MotionCompensator_EighthPixel(const PicturePredParams &ppp) :
    MotionCompensator(ppp)
{}

void MotionCompensator_EighthPixel::BlockPixelPred(
    TwoDArray<ValueType> &block_data ,
    const ImageCoords& pos ,
    const ImageCoords& pic_size ,
    const PicArray &refup_data ,
    const MVector &mv)
{

    //Set up the start point in the reference image by rounding the motion vector
    //NB: bit shift rounds negative values DOWN, as required
    const MVector roundvec(mv.x >> 2 , mv.y >> 2);

    //Get the remainder after rounding. NB rmdr values always 0,1,2 or 3
    const MVector rmdr(mv.x & 3 , mv.y & 3);

    //Where to start in the upconverted image
    const ImageCoords start_pos(std::max(pos.x, 0) , std::max(pos.y, 0));
    const ImageCoords ref_start((start_pos.x << 1) + roundvec.x , (start_pos.y << 1) + roundvec.y);

    //weights for doing linear interpolation, calculated from the remainder values
    const ValueType linear_wts[4] = {  (4 - rmdr.x) *(4 - rmdr.y),     //tl
                                       rmdr.x *(4 - rmdr.y),           //tr
                                       (4 - rmdr.x) * rmdr.y,          //bl
                                       rmdr.x * rmdr.y
                                    };              //br

    //An additional stage to make sure the block to be copied does not fall outside
    //the reference image.
    const int refXlen = refup_data.LengthX();
    //const int refYlen = refup_data.LengthY();
    const int trueRefXlen = (pic_size.x << 1) - 1;
    const int trueRefYlen = (pic_size.y << 1) - 1;
    bool do_bounds_checking = false;

    //Check if there are going to be any problems copying the block from
    //the upvconverted reference image.
    if(ref_start.x < 0)
        do_bounds_checking = true;
    else if(ref_start.x + (block_data.LengthX() << 1) >= trueRefXlen)
        do_bounds_checking = true;
    if(ref_start.y < 0)
        do_bounds_checking = true;
    else if(ref_start.y + (block_data.LengthY() << 1) >= trueRefYlen)
        do_bounds_checking = true;


    if(!do_bounds_checking)
    {
        ValueType *block_curr = &block_data[0][0];
        ValueType *refup_curr = &refup_data[ref_start.y][ref_start.x];

        const int refup_next = (refup_data.LengthX() - block_data.LengthX()) * 2;       //go down 2 rows and back up

        if(rmdr.x == 0 && rmdr.y == 0)
        {
            for(int y = 0; y < block_data.LengthY(); ++y, refup_curr += refup_next)
            {
                for(int x = 0; x < block_data.LengthX(); ++x, ++block_curr, refup_curr += 2)
                {
                    *block_curr = refup_curr[0];
                }
            }
        }
        else if(rmdr.y == 0)
        {
            for(int y = 0; y < block_data.LengthY(); ++y, refup_curr += refup_next)
            {
                for(int x = 0; x < block_data.LengthX(); ++x, ++block_curr, refup_curr += 2)
                {
                    *block_curr = (
                                      linear_wts[0] * refup_curr[0] +
                                      linear_wts[1] * refup_curr[1] +
                                      8
                                  ) >> 4;
                }
            }
        }
        else if(rmdr.x == 0)
        {
            for(int y = 0; y < block_data.LengthY(); ++y, refup_curr += refup_next)
            {
                for(int x = 0; x < block_data.LengthX(); ++x, ++block_curr, refup_curr += 2)
                {
                    *block_curr = (
                                      linear_wts[0] * refup_curr[0]  +
                                      linear_wts[2] * refup_curr[refXlen+0]  +
                                      8
                                  ) >> 4;
                }
            }
        }
        else
        {
            for(int y = 0; y < block_data.LengthY(); ++y, refup_curr += refup_next)
            {
                for(int x = 0; x < block_data.LengthX(); ++x, ++block_curr, refup_curr += 2)
                {
                    *block_curr = (
                                      linear_wts[0] * refup_curr[0]  +
                                      linear_wts[1] * refup_curr[1]  +
                                      linear_wts[2] * refup_curr[refXlen+0]  +
                                      linear_wts[3] * refup_curr[refXlen+1]  +
                                      8
                                  ) >> 4;
                }
            }
        }
    }
    else
    {
        // We're doing bounds checking because we'll fall off the edge of the reference otherwise.

        for(int c = 0, uY = ref_start.y, BuY = BChk(uY, trueRefYlen), BuY1 = BChk(uY + 1, trueRefYlen);
            c < block_data.LengthY(); ++c, uY += 2, BuY = BChk(uY, trueRefYlen), BuY1 = BChk(uY + 1, trueRefYlen))
        {
            for(int l = 0, uX = ref_start.x, BuX = BChk(uX, trueRefXlen), BuX1 = BChk(uX + 1, trueRefXlen);
                l < block_data.LengthX();
                ++l, uX += 2, BuX = BChk(uX, trueRefXlen), BuX1 = BChk(uX + 1, trueRefXlen))
            {

                block_data[c][l] = (
                                       linear_wts[0] * refup_data[BuY][BuX]  +
                                       linear_wts[1] * refup_data[BuY][BuX1] +
                                       linear_wts[2] * refup_data[BuY1][BuX] +
                                       linear_wts[3] * refup_data[BuY1][BuX1] +
                                       8
                                   ) >> 4;
            }//l
        }//c

    }

}
