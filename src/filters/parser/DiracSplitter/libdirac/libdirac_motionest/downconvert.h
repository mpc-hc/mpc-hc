/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: downconvert.h,v 1.11 2007/03/19 16:19:00 asuraparaju Exp $ $Name:  $
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
*                 Thomas Davies
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

#ifndef _DOWNCONVERT_H_
#define _DOWNCONVERT_H_

#include <libdirac_common/common.h>
namespace dirac
{

//! A class for fast downconversion of picture data
/*!
    A class for fast down-conversion of picture data. The picture data is
    downconverted by a factor of two in each dimension, using fast
    filtering techniques. The filter is a half-band filter designed to
    trade off frequency response, ringiness,  and aliasing
 */
class DownConverter
{

public:

    //! Constructor
    DownConverter();
    //! Destructor
    ~DownConverter() {};

    //! A function to do the actual down-conversion
    /*!
        A function to do the actual downconversion.
        \param    old_data    the picture data to be downconverted
        \param    new_data    the resulting down-converted data. The array must be of the correct size.
     */
    void DoDownConvert(const PicArray& old_data, PicArray& new_data);

private:
    //Copy constructor
    DownConverter(const DownConverter& cpy);//private, body-less: class should not be copied
    //Assignment=
    DownConverter& operator=(const DownConverter& rhs);//private, body-less: class should not be assigned

    //Applies the filter to a single column
    void RowLoop(const int colpos , PicArray& new_data);

    ValueType* m_row_buffer;

    //Define filter parameters
    static const int Stage_I_Size = 6;
    static const int StageI_I = 86;
    static const int StageI_II = 46;
    static const int StageI_III = 4;
    static const int StageI_IV = -8;
    static const int StageI_V = -4;
    static const int StageI_VI = 4;
    static const int StageI_Shift = 8;
};

} // namespace dirac

#endif
