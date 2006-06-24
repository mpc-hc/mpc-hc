/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: golomb.h 280 2005-01-30 05:11:46Z gabest $ $Name$
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
* Contributor(s):
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


#ifndef _GOLOMB_H_
#define _GOLOMB_H_

#include <libdirac_common/bit_manager.h>

namespace dirac
{
    //exp-golomb coding and decoding

    //! Code a value using unsigned exp-Golomb coding and output it
    void UnsignedGolombCode(BasicOutputManager& bitman, const unsigned int val);

    //! Code a value using unsigned exp-Golomb coding and output it to a vector
    void UnsignedGolombCode(std::vector<bool>& bitvec, const unsigned int val);

    //! Code a value using signed exp-Golomb coding and output it
    void GolombCode(BasicOutputManager& bitman, const int val);

    //! Code a value using signed exp-Golomb coding and output it to a vector
    void GolombCode(std::vector<bool>& bitvec, const int val);

    //! Decode a value using unsigned exp-Golomb decoding and output it
    unsigned int UnsignedGolombDecode(BitInputManager& bitman);//returning the value decoded

    //! Decode a value using unsigned exp-Golomb decoding and output it
    unsigned int UnsignedGolombDecode(const std::vector<bool>& bitvec);//returning the value decoded

    //! Decode a value using signed exp-Golomb decoding and output it
    int GolombDecode(BitInputManager& bitman);//returning the value decoded

    //! Decode a value using signed exp-Golomb decoding and output it
    int GolombDecode(const std::vector<bool>& bitvec);//returning the value decoded

} // namespace dirac

#endif
