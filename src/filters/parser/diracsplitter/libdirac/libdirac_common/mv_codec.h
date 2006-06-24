/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: mv_codec.h 280 2005-01-30 05:11:46Z gabest $ $Name$
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
* Contributor(s): Thomas Davies (Original Author), Scott R Ladd
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
    //! Codes and decodes all the Motion Vector data
    /*!
        Derived from the ArithCodec class, this codes and decodes all the 
        motion vector data.
     */
    class MvDataCodec: public ArithCodec<MvData>
    {
    public:
        //! Constructor for encoding
            /*!
            Creates a MvDataCodec object to encode MV data, based on parameters
            \param    bits_out    the output for the encoded bits
            \param    number_of_contexts   the contexts used in the encoding process
            \param     cf            the chroma format
         */    
        MvDataCodec(BasicOutputManager* bits_out,
                    size_t number_of_contexts,
                    const ChromaFormat & cf);

        //! Constructor for decoding
            /*!
            Creates a MvDataCodec object to encode MV data, based on parameters
            \param    bits_in        the input for the encoded bits
            \param    number_of_contexts  the contexts used in the encoding process
            \param     cf            the chroma format
         */        
        MvDataCodec(BitInputManager* bits_in,
                    size_t number_of_contexts,
                    const ChromaFormat & cf); 

        //! Initialises the contexts    
        void InitContexts();
        
    private:
        int MB_count;
        const ChromaFormat & m_cformat;

        int b_xp, b_yp;            //position of current block
        int mb_xp, mb_yp;        //position of current MB
        int mb_tlb_x, mb_tlb_y;    //position of top-left block of current MB

        // functions    
        MvDataCodec(const MvDataCodec& cpy);            //private, bodyless copy constructor: class should not be copied
        MvDataCodec& operator=(const MvDataCodec& rhs); //private, bodyless copy operator=: class should not be assigned

        // coding functions    
        void CodeMBSplit(const MvData& in_data);    //code the MB splitting mode
        void CodeMBCom(const MvData& in_data);    //code the MB common ref mode
        void CodePredmode(const MvData& in_data);    //code the block prediction mode
        void CodeMv1(const MvData& in_data);        //code the first motion vector
        void CodeMv2(const MvData& in_data);        //code the second motion vector
        void CodeDC(const MvData& in_data);        //code the dc value of intra blocks

        // decoding functions
        void DecodeMBSplit( MvData& out_data);    //decode the MB splitting mode
        void DecodeMBCom( MvData& out_data);//decode the MB common ref mode
        void DecodePredmode(MvData& out_data);//decode the block prediction mode
        void DecodeMv1( MvData& out_data);    //decode the first motion vector
        void DecodeMv2( MvData& out_data);    //decode the second motion vector
        void DecodeDC( MvData& out_data);    //decode the dc value of intra blocks    

        void DoWorkCode( MvData& in_data );
        void DoWorkDecode(MvData& out_data, int num_bits);

        // Context stuff    
        void Update( const bool symbol , const int context_num );
        void Resize(const int context_num);
        void ResetAll();

        int ChooseContext(const MvData& data, const int BinNumber) const;
        int ChooseContext(const MvData& data) const;
        int ChooseSignContext(const MvData& data) const;

        int ChooseMBSContext(const MvData& data, const int BinNumber) const;
        int ChooseMBCContext(const MvData& data) const;
        int ChoosePredContext(const MvData& data, const int BinNumber) const;
        int ChooseREF1xContext(const MvData& data, const int BinNumber) const;
        int ChooseREF1xSignContext(const MvData& data) const;
        int ChooseREF1yContext(const MvData& data, const int BinNumber) const;
        int ChooseREF1ySignContext(const MvData& data) const;
        int ChooseREF2xContext(const MvData& data, const int BinNumber) const;
        int ChooseREF2xSignContext(const MvData& data) const;
        int ChooseREF2yContext(const MvData& data, const int BinNumber) const;
        int ChooseREF2ySignContext(const MvData& data) const;
        int ChooseYDCContext(const MvData& data, const int BinNumber) const;
        int ChooseUDCContext(const MvData& data, const int BinNumber) const;
        int ChooseVDCContext(const MvData& data, const int BinNumber) const;
        int ChooseYDCSignContext(const MvData& data) const;
        int ChooseUDCSignContext(const MvData& data) const;
        int ChooseVDCSignContext(const MvData& data) const;

        //prediction stuff
        unsigned int MBSplitPrediction(const TwoDArray<int>& mbdata) const;
        bool MBCBModePrediction(const TwoDArray<bool>& mbdata) const;
        unsigned int BlockModePrediction(const TwoDArray<PredMode>& preddata) const;
        MVector Mv1Prediction(const MvArray& mvarray,const TwoDArray<PredMode>& preddata) const;
        MVector Mv2Prediction(const MvArray& mvarray,const TwoDArray<PredMode>& preddata) const;
        ValueType DCPrediction(const TwoDArray<ValueType>& dcdata,const TwoDArray<PredMode>& preddata) const;
    };

} // namespace dirac

#endif
