/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: upconvert.h,v 1.13 2007/11/22 15:19:28 tjdwave Exp $ $Name:  $
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
* Contributor(s): Richard Felton (Original Author), Thomas Davies
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

#ifndef _UPCONVERT_H_
#define _UPCONVERT_H_

#include <libdirac_common/common.h>

namespace dirac
{
    //Optimised upconversion class - no array resizes.
    //Uses integer math - no floats!
    //

    //! Upconversion class
    /*!
        Class to upconvert data by a factor of 2 in both dimensions
     */
    class UpConverter
    {

    public:

        //! Constructor
        UpConverter(int min_val, int max_val, int orig_xlen, int orig_ylen);

        //! Destructor
        ~UpConverter() {};

        //! Upconvert the picture data
        /*!
            Upconvert the picture data, where the parameters are
            \param    pic_data   is the original data
            \param    up_data    is the upconverted data
         */
        void DoUpConverter(const PicArray& pic_data, PicArray& up_data);

    private:
        //! Private body-less copy constructor: class should not be copied
        UpConverter(const UpConverter& cpy);

        //! Private body-less assignment: class should not be assigned
        UpConverter& operator=(const UpConverter& rhs);

        //! Applies the filter to a row and its successor 
        void RowLoop(PicArray& up_data, const int row_num,
        const int filter_size, const int filter_shift, const short taps[4] );

    private:
        //Variable to keep the loops in check
        int m_width_old, m_height_old;
        int m_width_new, m_height_new;


        const int m_min_val;
        const int m_max_val;

        const int m_orig_xl;
        const int m_orig_yl;
    };

} // namespace dirac

#endif
