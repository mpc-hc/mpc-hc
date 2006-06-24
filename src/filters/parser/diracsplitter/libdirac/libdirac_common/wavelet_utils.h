/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: wavelet_utils.h 280 2005-01-30 05:11:46Z gabest $ $Name$
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

#ifndef _WAVELET_UTILS_H_
#define _WAVELET_UTILS_H_

#include <libdirac_common/arrays.h>
#include <libdirac_common/common.h>
#include <vector>
#include <cmath>
#include <iostream>

namespace dirac
{
    //utilities for subband and wavelet transforms
    //Includes fast transform using lifting

    class PicArray;

    //! Class encapsulating all the metadata relating to a wavelet subband
    class Subband
    {
    public:

        //! Default constructor
        Subband();

        //! Constructor
        /*!
            The constructor parameters are
            \param    xpos    the xposition of the subband when packed into a big array with all the others
            \param    ypos    the xposition of the subband
            \param    xlen    the width of the subband
            \param    ylen    the height of the subband
         */    
        Subband(int xpos, int ypos, int xlen, int ylen);

        //! Constructor
        /*!
            The constructor parameters are
            \param    xpos    the xposition of the subband when packed into a big array with all the others
            \param    ypos    the xposition of the subband
            \param    xlen    the width of the subband
            \param    ylen    the height of the subband
            \param    d        the depth of the subband in the wavelet transform
         */    
        Subband(int xpos, int ypos, int xlen, int ylen, int d);

        //! Destructor
        ~Subband();

        //Default (shallow) copy constructor and operator= used

        //! Return the width of the subband
        int Xl() const {return xln;}
        
        //! Return the horizontal position of the subband
        int Xp() const {return xps;}
        
        //! Return the height of the subband
        int Yl() const {return yln;}
        
        //! Return the vertical position of the subband
        int Yp() const {return yps;}
        
        //! Return the index of the maximum bit of the largest coefficient
        int Max() const {return max_bit;}
        
        //! Return the subband perceptual weight
        double Wt() const {return wgt;}
        
        //! Return the depth of the subband in the transform
        int Depth() const {return dpth;}
        
        //! Return the scale of the subband, viewed as a subsampled version of the picture
        int Scale() const {return (1<<dpth);}
        
        //! Return a quantisation factor
        int Qf(int n) const {return qfac[n];}
        
        //! Return the index of the parent subband
        int Parent() const {return prt;}
        
        //! Return the indices of any child subbands
        std::vector<int> Children() const {return childvec;}
        
        int Child(int n) const {return childvec[n];}

        // ... and sets
        //! Set the perceptual weight
        void SetQf(int n, int q)
        {
            if (n >= qfac.First() && n<=qfac.Last() )
                qfac[n]=q;
        }
        
        //! Set the perceptual weight
        void SetWt(float w){wgt=w;}
        
        //! Set the parent index
        void SetParent(int p){prt=p;}
        
        //! Set the subband depth
        void SetDepth(int d){dpth=d;}
        
        //! Set the index of the maximum bit of the largest coefficient
        void SetMax(int m){max_bit=m;};
        
        //! Set the indices of the children of the subband
        void SetChildren(std::vector<int>& clist){childvec=clist;}
        
        //! Add a child to the list of child subbands
        void AddChild(int c){childvec.push_back(c);}

    private:
        int xps,yps,xln,yln;        //subband bounds
        double wgt;                    //perceptual weight for quantisation
        int dpth;                    //depth in the transform
        OneDArray<int> qfac;        //quantisers
        int prt;                    //position of parent in a subband list
        std::vector<int> childvec;    //positions of children in the subband list
        int max_bit;                //position of the MSB of the largest absolute value
    };

    //!    A class encapulating all the subbands produced by a transform
    class SubbandList
    {
    public:
        //! Constructor
        SubbandList(){}

        //! Destructor
        ~SubbandList(){}

        //Default (shallow) copy constructor and operator= used
        //! Initialise the list
        void Init(const int depth,const int xlen,const int ylen);
        
        //! Return the length of the subband list    
        int Length() const {return bands.size();}
        
        //! Return the subband at position n (1<=n<=length)
        Subband& operator()(int n){return bands[n-1];}
        
        //! Return the subband at position n (1<=n<=length)    
        const Subband& operator()(int n) const {return bands[n-1];}    
        
        //! Add a band to the list
        void AddBand(Subband& b){bands.push_back(b);}
        
        //! Remove all the bands from the list    
        void Clear(){bands.clear();}
        
    private:    
        std::vector<Subband> bands;
    };


    //! Class to do two-tap prediction lifting step
    template <int gain> class PredictStep
    {

    public:

        //! Constructor
        PredictStep(){}

        // Assume default copy constructor, assignment= and destructor //

        //! Do the filtering
        /*
            Do the filtering.
            \param   in_val   the value being predicted
            \param   val1   the first value being used for prediction
            \param   val2   the second value being used for prediction
        */
        void Filter(ValueType& in_val, const ValueType& val1, const ValueType& val2) const;

    private:

    }; 

    template <int gain>
    inline void  PredictStep<gain>::Filter( ValueType& in_val, 
                                                const ValueType& val1, 
                                                const ValueType& val2) const
    {
        in_val -= static_cast< ValueType >( (gain * static_cast< int >( val1 + val2 )) >>12 );
    }

    //! Class to do two-tap updating lifting step
    template <int gain> class UpdateStep
    {

    public:
        //! Constructor
        UpdateStep(){}

        //! Do the filtering
        /*
            Do the filtering.
            \param   in_val   the value being updated
            \param   val1   the first value being used for updating
            \param   val2   the second value being used for updating
        */
        void Filter(ValueType& in_val, const ValueType& val1, const ValueType& val2) const;

    private:

    };  

    template <int gain>
    inline void UpdateStep<gain>::Filter(ValueType& in_val,
                                              const ValueType& val1, 
                                              const ValueType& val2) const
    {
        in_val += static_cast< ValueType >( (gain * static_cast< int >( val1 + val2 )) >>12 );
    }





    //! A class to do wavelet transforms
    /*!
        A class to do forward and backward wavelet transforms by iteratively 
        splitting or merging the lowest frequency band.
    */
    class WaveletTransform
    {
    public:
        //! Constructor
        // WaveletTransform(WaveletTransformParams p);
        WaveletTransform(int d = 4, WltFilter f = DAUB);
            
        //! Destructor
        virtual ~WaveletTransform();

        //! Transforms the data to and from the wavelet domain
        /*!
            Transforms the data to and from the wavelet domain.
            \param    d    the direction of the transform
            \param    pic_data    the data to be transformed
        */
        void Transform(const Direction d, PicArray& pic_data);
        
        //! Returns the set of subbands
        SubbandList& BandList(){return band_list;}
        
        //! Returns the set of subbands
        const SubbandList& BandList() const {return band_list;}
        
        //! Sets the subband weights
        /*!
            Sets perceptual weights for the subbands. Takes into account both 
            perceptual factors (weight noise less at higher spatial 
            frequencies) and the scaling needed for the wavelet transform. 

            \param    cpd    perctual factor - the number of cycles per degree
            \param    fsort    the frame sort (I, L1 or L2)
            \param    cformat    the chroma format
            \param    csort    the component type (Y, U or V)  
        */
        void SetBandWeights (const float cpd, 
                             const FrameSort& fsort,
                             const ChromaFormat& cformat,
                             const CompSort csort);

    private:
        //other private variables    
        // WaveletTransformParams params;

        SubbandList band_list;

        //! Depth of the transform
        int depth;
        
        //! The filter set to be used (only Daubechies supported at present)
        WltFilter filt_sort;    

        //functions
        //!    Private, bodyless copy constructor: class should not be copied
        WaveletTransform(const WaveletTransform& cpy);
        
        //! Private, bodyless copy operator=: class should not be assigned
        WaveletTransform& operator=(const WaveletTransform& rhs);

        //! Given x and y spatial frequencies in cycles per degree, returns a weighting value
        float PerceptualWeight(float xf,float yf,CompSort cs);

        //! Split a subband into 4
        void VHSplit(const int xp, const int yp, const int xl, const int yl, PicArray&pic_data); 

        //! Synthesise a picture from 4 subbands
        void VHSynth(const int xp, const int yp, const int xl, const int yl, PicArray& pic_data);
};

} // namespace dirac

#endif
