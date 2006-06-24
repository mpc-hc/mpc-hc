/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: comp_compress.h 280 2005-01-30 05:11:46Z gabest $ $Name$
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


#ifndef _COMP_COMPRESS_H_
#define _COMP_COMPRESS_H_

#include "libdirac_common/arrays.h"
#include "libdirac_common/wavelet_utils.h"
#include "libdirac_common/common.h"

namespace dirac
{
    //! Compress a frame component
    /*!
        This class compresses one of the three components (Y, U, or V) of a
        frame according to a given set or parameters. CompCompressor is used
        by FrameCompressor.
    */
    class CompCompressor
    {
    public:
        //! Constructor
        /*!
            Create and initialize a component compressor with the given
            characteristics.
            \param  encp    encoding parameters
            \param  fp      frame parameters
        */
        CompCompressor( EncoderParams & encp, const FrameParams& fp);

        //! Compress a frame component
        /*!
            Compress a PicArray containing a frame component (Y, U, or V).
            \param  pic_data    the component data to be compressed
        */
        void Compress(PicArray & pic_data);

    private:
        //! Copy constructor is private and body-less. This class should not be copied.
        CompCompressor(const CompCompressor& cpy);

        //! Assignment = is private and body-less. This class should not be assigned.
        CompCompressor& operator=(const CompCompressor& rhs);

        void GenQuantList();

        void SelectQuantisers( PicArray& pic_data , SubbandList& bands ,
                               OneDArray<unsigned int>& est_counts );

        int SelectQuant(PicArray& pic_data,SubbandList& bands,int band_num);

        ValueType PicAbsMax(const PicArray& pic_data,int xp, int yp ,int xl ,int yl) const;

        ValueType PicAbsMax(const PicArray& pic_data) const;

        void SetToVal(PicArray& pic_data,const Subband& node,ValueType val);

        void AddSubAverage(PicArray& pic_data,int xl,int yl,AddOrSub dirn);

        //member variables
        EncoderParams& m_encparams;
        const FrameParams& m_fparams;
        const FrameSort& m_fsort;    
        const ChromaFormat& m_cformat;
        CompSort m_csort;
        OneDArray<int> m_qflist;
        OneDArray<int> m_qfinvlist;
        OneDArray<int> m_offset;
        float m_lambda;

    };

} // namespace dirac



#endif
