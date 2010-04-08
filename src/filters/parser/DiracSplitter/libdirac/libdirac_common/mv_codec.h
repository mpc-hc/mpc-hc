/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: mv_codec.h,v 1.25 2008/10/01 01:26:47 asuraparaju Exp $ $Name:  $
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
*                 Tim Borer,
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

#ifndef _MV_CODEC_H_
#define _MV_CODEC_H_

/////////////////////////////////////////////////
//Class to do motion vector coding and decoding//
//------using adaptive arithmetic coding-------//
/////////////////////////////////////////////////

#include <libdirac_common/arith_codec.h>
#include <libdirac_common/common.h>
#include <libdirac_common/motion.h>
#include <libdirac_common/wavelet_utils.h>
#include <vector>

namespace dirac
{
//! Codes and decodes the split mode
/*!
    Derived from the ArithCodec class, this codes and decodes the split mode
 */
class SplitModeCodec: public ArithCodec<MvData>
{
public:
    //! Constructor
    /*!
    Creates a MvDataCodec object to encode MV data, based on parameters
    \param    p_byteio   Input/output for the encoded bits
    \param    number_of_contexts the number of contexts used
             */
    SplitModeCodec(ByteIO* p_byteio, size_t number_of_contexts);



    //! Initialises the contexts
    void InitContexts();

private:

    // Position of current SB
    int m_sb_xp, m_sb_yp;

private:

    // functions
    //! Private, bodyless copy constructor: class should not be copied
    SplitModeCodec(const SplitModeCodec& cpy);
    //! Private, bodyless copy operator=: class should not be assigned
    SplitModeCodec& operator=(const SplitModeCodec& rhs);

    // coding functions
    // Code the SB splitting mode
    void CodeVal(const MvData& in_data);

    // decoding functions
    // Decode the SB splitting mode
    void DecodeVal(MvData& out_data);

    void DoWorkCode(MvData& in_data);
    void DoWorkDecode(MvData& out_data);

    // Context stuff
    void ResetAll();

    //prediction stuff
    unsigned int Prediction(const TwoDArray<int>& mbdata) const;

};

/******************************************************************************/

//! Codes and decodes the prediction modes
/*!
    Derived from the ArithCodec class, this codes and decodes the prediction mode.
 */
class PredModeCodec: public ArithCodec<MvData>
{
public:
    //! Constructor
    /*!
    Creates a MvDataCodec object to encode MV data, based on parameters
    \param    p_byteio   Input/output for the encoded bits
    \param    number_of_contexts the number of contexts used
    \param    num_refs   Number of references
             */
    PredModeCodec(ByteIO* p_byteio, size_t number_of_contexts, const int num_refs);

    //! Initialises the contexts
    void InitContexts();

private:

    // Position of current block
    int m_b_xp, m_b_yp;
    // Position of current SB
    int m_sb_xp, m_sb_yp;
    // Position of top-left block of current SB
    int m_sb_tlb_x, m_sb_tlb_y;
    // Number of reference pictures
    int m_num_refs;

private:

    // functions
    //! Private, bodyless copy constructor: class should not be copied
    PredModeCodec(const PredModeCodec& cpy);
    //! Private, bodyless copy operator=: class should not be assigned
    PredModeCodec& operator=(const PredModeCodec& rhs);

    // coding functions
    // Code the block prediction mode
    void CodeVal(const MvData& in_data);

    // decoding functions
    // Decode the block prediction mode
    void DecodeVal(MvData& out_data);

    void DoWorkCode(MvData& in_data);
    void DoWorkDecode(MvData& out_data);

    // Context stuff
    void ResetAll();

    //prediction stuff
    unsigned int Prediction(const TwoDArray<PredMode>& preddata) const;

};

/******************************************************************************/

//! Codes and decodes an array of motion vectors
/*!
    Derived from the ArithCodec class, this codes and decodes a motion vector
    element (vertical or horizontal)
 */
class VectorElementCodec: public ArithCodec<MvData>
{
public:
    //! Constructor
    /*!
    Creates a MvDataCodec object to encode MV data, based on parameters
    \param    p_byteio   Input/output for the encoded bits
    \param    ref_id    The identity of the reference (1 or 2)
    \param    horvert    The identity of the vector element (horizontal or vertical)
    \param    number_of_contexts the number of contexts used
             */
    VectorElementCodec(ByteIO* p_byteio, int ref_id, MvElement horvert,
                       size_t number_of_contexts);


    //! Initialises the contexts
    void InitContexts();

private:

    // Position of current block
    int m_b_xp, m_b_yp;

    // Position of current SB
    int m_sb_xp, m_sb_yp;

    // Position of top-left block of current SB
    int m_sb_tlb_x, m_sb_tlb_y;

    // The identity of the reference (1 or 2)
    const int m_ref;

    // Whether it's the vertical or horizontal MV element
    const MvElement m_hv;

private:

    // functions
    //! Private, bodyless copy constructor: class should not be copied
    VectorElementCodec(const VectorElementCodec& cpy);
    //! Private, bodyless copy operator=: class should not be assigned
    VectorElementCodec& operator=(const VectorElementCodec& rhs);

    // coding functions
    // Code the motion vector element
    void CodeVal(const MvData& in_data);

    // decoding functions
    // Decode the motion vector element
    void DecodeVal(MvData& out_data);

    void DoWorkCode(MvData& in_data);
    void DoWorkDecode(MvData& out_data);

    // Context stuff
    void ResetAll();

    //prediction stuff
    int Prediction(const MvArray& mvarray,
                   const TwoDArray<PredMode>& preddata) const;

};

/******************************************************************************/
//! Codes and decodes a set of DC values
/*!
    Derived from the ArithCodec class, this codes and decodes all the DC
    values for a component
 */
class DCCodec: public ArithCodec<MvData>
{
public:
    //! Constructor
    /*!
    Creates a MvDataCodec object to encode MV data, based on parameters
    \param    p_byteio   Input/output for the encoded bits
    \param    csort    The identity of the component (Y, U or V)
    \param    number_of_contexts the number of contexts used
             */
    DCCodec(ByteIO* p_byteio, const CompSort csort, size_t number_of_contexts);

    //! Initialises the contexts
    void InitContexts();

private:

    // The component being coded
    const CompSort m_csort;
    // Position of current block
    int m_b_xp, m_b_yp;
    // Position of current SB
    int m_sb_xp, m_sb_yp;
    // Position of top-left block of current SB
    int m_sb_tlb_x, m_sb_tlb_y;

private:

    // functions
    //! Private, bodyless copy constructor: class should not be copied
    DCCodec(const DCCodec& cpy);
    //! Private, bodyless copy operator=: class should not be assigned
    DCCodec& operator=(const DCCodec& rhs);

    // coding functions
    // Code the dc value of intra blocks
    void CodeVal(const MvData& in_data);

    // decoding functions
    // Decode the dc value of intra blocks
    void DecodeVal(MvData& out_data);

    void DoWorkCode(MvData& in_data);
    void DoWorkDecode(MvData& out_data);

    // Context stuff
    void ResetAll();

    //prediction stuff
    ValueType Prediction(const TwoDArray<ValueType>& dcdata,
                         const TwoDArray<PredMode>& preddata) const;
};


}// end namepace dirac


#endif
