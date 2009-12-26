/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: me_utils_mmx.h,v 1.3 2008/05/27 01:29:55 asuraparaju Exp $
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
* The Original Code is contributed by Peter Meerwald.
*
* The Initial Developer of the Original Code is Peter Meerwald.
* Portions created by the Initial Developer are Copyright (C) 2004.
* All Rights Reserved.
*
* Contributor(s): Peter Meerwald (Original Author)
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

#ifndef _ME_UTILS_MMX_H_
#define _ME_UTILS_MMX_H_

#if defined(HAVE_MMX)

#include <mmintrin.h>

#include <libdirac_motionest/me_utils.h>
#include <libdirac_common/common.h>

namespace dirac
{

    CalcValueType simple_block_diff_mmx_4(const BlockDiffParams& dparams, const MVector& mv, const PicArray& pic_data, const PicArray& ref_data, CalcValueType i_best_sum);
    CalcValueType simple_intra_block_diff_mmx_4 ( const BlockDiffParams& dparams, const PicArray& pic_data, ValueType &dc_val);

    CalcValueType bchk_simple_block_diff_mmx_4 ( 
            const BlockDiffParams& dparams, const MVector& mv, 
            const PicArray& pic_data, const PicArray& ref_data,
            CalcValueType i_best_sum);

    float simple_block_diff_up_mmx_4(
            const PicArray& pic_data, const PicArray& ref_data, 
            const ImageCoords& start_pos, const ImageCoords& end_pos, 
            const ImageCoords& ref_start, const ImageCoords& ref_stop,
            const MVector& rmdr, float cost_so_far, 
            float best_cost_so_far);


    void simple_biblock_diff_pic_mmx_4(
        const PicArray& pic_data, const PicArray& ref_data,
        TwoDArray<ValueType>& diff, 
        const ImageCoords& start_pos, const ImageCoords& end_pos, 
        const ImageCoords& ref_start, const ImageCoords& ref_stop,
        const MVector& rmdr);

    CalcValueType simple_biblock_diff_up_mmx_4(
            const TwoDArray<ValueType>& diff_data, const PicArray& ref_data, 
            const ImageCoords& ref_start, const ImageCoords& ref_stop,
            const MVector& rmdr);
}

#endif /* HAVE_MMX */
#endif
