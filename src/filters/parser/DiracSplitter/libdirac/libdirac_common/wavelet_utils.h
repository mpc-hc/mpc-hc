/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: wavelet_utils.h,v 1.32 2008/10/20 04:21:02 asuraparaju Exp $ $Name:  $
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
*                 Scott R Ladd
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

#ifndef _WAVELET_UTILS_H_
#define _WAVELET_UTILS_H_

#include <libdirac_common/arrays.h>
#include <libdirac_common/common.h>
#include <vector>
#include <cmath>
#include <iostream>

//utilities for subband and wavelet transforms
//Includes fast transform using lifting

namespace dirac
{

    class PicArray;
    class Subband;

    //! Class for encapsulating metadata concerning a block of coefficients in a subband
    class CodeBlock
    {

    friend class Subband;

    public:
        //! Constructor
        /*
            Default constructor - sets all dimensions to zero
        */
        CodeBlock();

        //! Constructor
        /*
            Initialise the code block
            \param    xstart  the x-coord of the first coefficient in the block
            \param    xend    one past the last coefficient, horizontally
            \param    ystart  the y-coord of the first coefficient in the block
            \param    yend    one past the last coefficient, vertically
        */
        CodeBlock( const int xstart , const int ystart , const int xend , const int yend);

        //! Returns the horizontal start of the block
        int Xstart() const { return m_xstart; }

        //! Returns the vertical start of the block
        int Ystart() const { return m_ystart; }

        //! Returns one past the last coefficient coord, horizontally
        int Xend() const { return m_xend; }

        //! Returns one past the last coefficient coord, vertically
        int Yend() const { return m_yend; }

        //! Returns the width of the code block
        int Xl() const { return m_xl; }

        //! Returns the height of the code block
        int Yl() const { return m_yl; }

        //! Returns the quantisation index associated to the code block
        int QuantIndex() const{ return m_quantindex; }

        //! Returns true if the code-block is skipped, false if not
        bool Skipped() const { return m_skipped; }

        //! Sets the quantisation index
        void SetQuantIndex( const int quantindex ){ m_quantindex = quantindex; }

        //! Sets whether the code block is skipped or not
        void SetSkip( bool skip ){ m_skipped = skip; }

    private:

        //! Initialise the code block
        /*
            Initialise the code block
            \param    xstart  the x-coord of the first coefficient in the block
            \param    xend    one past the last coefficient, horizontally
            \param    ystart  the y-coord of the first coefficient in the block
            \param    yend    one past the last coefficient, vertically
        */
        void Init( const int xstart , const int ystart , const int xend , const int yend );

    private:

        int m_xstart;
        int m_ystart;
        int m_xend;
        int m_yend;
        int m_xl;
        int m_yl;

        int m_quantindex;

        bool m_skipped;
    };


    //! Class encapsulating all the metadata relating to a wavelet subband
    class Subband
    {
    public:

        //! Default constructor
        Subband();

        //! Constructor
        /*!
            The constructor parameters are
            \param    xpos    the xposition of the subband when packed into a
                              big array with all the others
            \param    ypos    the xposition of the subband
            \param    xlen    the width of the subband
            \param    ylen    the height of the subband
         */
        Subband(int xpos, int ypos, int xlen, int ylen);

        //! Constructor
        /*!
            The constructor parameters are
            \param    xpos    the xposition of the subband when packed into a
                              big array with all the others
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
        int Xl() const {return m_xl;}

        //! Return the horizontal position of the subband
        int Xp() const {return m_xp;}

        //! Return the height of the subband
        int Yl() const {return m_yl;}

        //! Return the vertical position of the subband
        int Yp() const {return m_yp;}

        //! Return the index of the maximum bit of the largest coefficient
        int Max() const {return m_max_bit;}

        //! Return the subband perceptual weight
        double Wt() const {return m_wt;}

        //! Return the depth of the subband in the transform
        int Depth() const {return m_depth;}

        //! Return the scale of the subband, viewed as a subsampled version of the picture
        int Scale() const {return ( 1<<m_depth );}

        //! Return a quantisation index
        int QuantIndex() const {return m_qindex;}

        //! Return a flag indicating whether we have separate quantisers for each code block
        bool UsingMultiQuants() const {return m_multi_quants; }

        //! Return the index of the parent subband
        int Parent() const {return m_parent;}

        //! Return the indices of any child subbands
        const std::vector<int>& Children() const {return m_children;}

        //! Return the index of a specific child band
        int Child(const int n) const {return m_children[n];}

        //! Return the code blocks
        TwoDArray<CodeBlock>& GetCodeBlocks(){ return m_code_block_array; }

        //! Return the code blocks
        const TwoDArray<CodeBlock>& GetCodeBlocks() const { return m_code_block_array; }

        //! Returns true if subband is skipped, false if not
        bool Skipped() const { return m_skipped; }

        //! Set the perceptual weight
        void SetWt( const float w );

        //! Set the parent index
        void SetParent( const int p ){ m_parent=p; }

        //! Set the subband depth
        void SetDepth( const int d ){ m_depth=d;}

        //! Set the index of the maximum bit of the largest coefficient
        void SetMax( const int m ){ m_max_bit=m; };

        //! Set the number of (spatial) quantisers in the subband. Creates code block structure
        void SetNumBlocks( const int ynum , const int xnum );

        //! Set the quantisation index
        void SetQuantIndex( const int idx){ m_qindex = idx; }

        //! Set the number of (spatial) quantisers in the subband. Creates code block structure
        void SetUsingMultiQuants( const bool multi){ m_multi_quants = multi; }

        //! Set whether the subband is skipped or not
        void SetSkip(const bool skip ){ m_skipped = skip; }

    private:
        // subband bounds
        int m_xp , m_yp , m_xl , m_yl;

        // perceptual weight for quantisation
        double m_wt;

        // depth in the transform
        int m_depth;

        // quantiser index
        int m_qindex;

        // position of parent in a subband list
        int m_parent;

        // positions of children in the subband list
        std::vector<int> m_children;

        // position of the MSB of the largest absolute value
        int m_max_bit;

        // The code blocks
        TwoDArray<CodeBlock> m_code_block_array;

        // A flag indicating whether we're using one qf for each code block
        bool m_multi_quants;

        // Whether the subband is skipped or not
        bool m_skipped;
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
        Subband& operator()(const int n){return bands[n-1];}

        //! Return the subband at position n (1<=n<=length)
        const Subband& operator()(const int n) const {return bands[n-1];}

        //! Add a band to the list
        void AddBand(const Subband& b){bands.push_back(b);}

        //! Remove all the bands from the list
        void Clear(){bands.clear();}

    private:

        //! Given x and y spatial frequencies in cycles per degree, returns a weighting value
        float PerceptualWeight( const float xf , const float yf , const CompSort cs);

    private:
        std::vector<Subband> bands;
    };
 
    class CoeffArray;   
        //! A virtual parent class to do vertical and horizontal splitting with wavelet filters
        class VHFilter
        {

        public:

            VHFilter(){}

            virtual ~VHFilter(){}

            //! Split a subband into 4
            virtual void Split(const int xp, const int yp, const int xl, const int yl, CoeffArray& coeff_data)=0;

            //! Create a single band from 4 quadrant bands
            virtual void Synth(const int xp, const int yp, const int xl, const int yl, CoeffArray& coeff_data)=0;

            //! Return the value of the additional bitshift
            virtual int GetShift() const =0;

        protected:

            //! Interleave data from separate subbands into even and odd positions for in-place calculation - called by Synth
            inline void Interleave( const int xp, const int yp, const int xl, const int yl, CoeffArray& coeff_data );


            //! De-interleave data even and odd positions into separate subbands - called by Split
            inline void DeInterleave( const int xp, const int yp, const int xl, const int yl, CoeffArray& coeff_data );

            //! Shift all vals in Row by 'shift' bits to the left to increase accuracy by 'shift' bits. Used in Analysis stage of filter
            void ShiftRowLeft(CoeffType *row, int length, int shift);

        //! Shift all vals in Row by 'shift' bits to the right to counter the shift in the Analysis stage. This function is used in the Synthesis stage
            void ShiftRowRight(CoeffType *row, int length, int shift);
        };

        //! Class to do Daubechies (9,7) filtering operations
        class VHFilterDAUB9_7 : public VHFilter
        {

        public:

            //! Split a subband into 4
            void Split(const int xp, const int yp, const int xl, const int yl, CoeffArray& coeff_data);

            //! Create a single band from 4 quadrant bands
            void Synth(const int xp, const int yp, const int xl, const int yl, CoeffArray& coeff_data);

            //! Return the value of the additional bitshift
            int GetShift() const {return 1;}


        };

        //! Class to do (5,3) wavelet filtering operations
        class VHFilterLEGALL5_3 : public VHFilter
        {

        public:

            //! Split a subband into 4
            void Split(const int xp, const int yp, const int xl, const int yl, CoeffArray& coeff_data);

            //! Create a single band from 4 quadrant bands
            void Synth(const int xp, const int yp, const int xl, const int yl, CoeffArray& coeff_data);

           //! Return the value of the additional bitshift
            int GetShift() const {return 1;}


#ifdef HAVE_MMX
            inline void HorizSynth (int xp, int xl, int ystart, int yend, CoeffArray &coeff_data);
#endif

        };

        //! A short filter that's actually close to Daubechies (9,7) but with just two lifting steps
        class VHFilterDD9_7 : public VHFilter
        {

        public:

            //! Split a subband into 4
            void Split(const int xp, const int yp, const int xl, const int yl, CoeffArray& coeff_data);

            //! Create a single band from 4 quadrant bands
            void Synth(const int xp, const int yp, const int xl, const int yl, CoeffArray& coeff_data);

            //! Return the value of the additional bitshift
            int GetShift() const {return 1;}
        };


        //! An extension of DD9_7, with a better low-pass filter but more computation
        class VHFilterDD13_7 : public VHFilter
        {

        public:

            //! Split a subband into 4
            void Split(const int xp, const int yp, const int xl, const int yl, CoeffArray& coeff_data);

            //! Create a single band from 4 quadrant bands
            void Synth(const int xp, const int yp, const int xl, const int yl, CoeffArray& coeff_data);

           //! Return the value of the additional bitshift
            int GetShift() const {return 1;}

        };

        //! Class to do Haar wavelet filtering operations
        class VHFilterHAAR0 : public VHFilter
        {

        public:

            //! Split a subband into 4
            void Split(const int xp, const int yp, const int xl, const int yl, CoeffArray& coeff_data);

            //! Create a single band from 4 quadrant bands
            void Synth(const int xp, const int yp, const int xl, const int yl, CoeffArray& coeff_data);

           //! Return the value of the additional bitshift
            int GetShift() const {return 0;}


        };

        //! Class to do Haar wavelet filtering operations with a single shift per level
        class VHFilterHAAR1 : public VHFilter
        {

        public:

            //! Split a subband into 4
            void Split(const int xp, const int yp, const int xl, const int yl, CoeffArray& coeff_data);

            //! Create a single band from 4 quadrant bands
            void Synth(const int xp, const int yp, const int xl, const int yl, CoeffArray& coeff_data);

           //! Return the value of the additional bitshift
            int GetShift() const {return 1;}

        };


       //! Class to do Haar wavelet filtering operations with a double shift per level
        class VHFilterHAAR2 : public VHFilter
        {

        public:

            //! Split a subband into 4
            void Split(const int xp, const int yp, const int xl, const int yl, CoeffArray& coeff_data);

            //! Create a single band from 4 quadrant bands
            void Synth(const int xp, const int yp, const int xl, const int yl, CoeffArray& coeff_data);

            //! Return a correction factor to compensate for non-unity power gain of low-pass filter
            double GetLowFactor() const { return 1.414213562;}

            //! Return a correction factor to compensate for non-unity power gain of high-pass filter
            double GetHighFactor() const { return 0.707106781;}

            //! Return the value of the additional bitshift
            int GetShift() const {return 2;}

        };



        // Lifting steps used in the filters

        //! Class to do two-tap prediction lifting step
        template<int shift>
        class PredictStepShift
        {

        public:

            //! Constructor
            PredictStepShift(){}

            // Assume default copy constructor, assignment= and destructor //

            //! Do the filtering
            /*
                Do the filtering.
                \param   in_val   the value being predicted
                \param   val1   the first value being used for prediction
                \param   val2   the second value being used for prediction
            */
            inline void Filter(CoeffType& in_val, const CoeffType& val1, const CoeffType& val2) const
            {
                in_val -= (( val1 + val2 + (1<<(shift-1)) ) >>shift );
            }

        };

        //! Class to do two-tap updating lifting step
        template<int shift>
        class UpdateStepShift
        {

        public:
            //! Constructor
            UpdateStepShift(){}

            //! Do the filtering
            /*
                Do the filtering.
                \param   in_val   the value being updated
                \param   val1   the first value being used for updating
                \param   val2   the second value being used for updating
            */
            inline void Filter(CoeffType& in_val, const CoeffType& val1, const CoeffType& val2) const
            {
                in_val += ( ( val1 + val2 + (1<<(shift-1)) ) >>shift );
            }

        };

        //! Class to do symmetric four-tap prediction lifting step
        template <int shift , int tap1, int tap2>
        class PredictStepFourTap
        {
        public:

            //! Constructor
            PredictStepFourTap(){}

            // Assume default copy constructor, assignment= and destructor //

            //! Do the filtering
            inline void Filter(CoeffType& in_val, const CoeffType& val1, const CoeffType& val2 ,
                                                  const CoeffType& val3, const CoeffType& val4 ) const
            {
                in_val -= ( tap1*( val1 + val2 ) + tap2*( val3 + val4 ) + (1<<(shift-1)))>>shift;
            }
        };

        //! Class to do symmetric four-tap update lifting step
        template <int shift , int tap1 , int tap2>
        class UpdateStepFourTap
        {

        public:
            //! Constructor
            UpdateStepFourTap(){}

            //! Do the filtering
            inline void Filter(CoeffType& in_val, const CoeffType& val1, const CoeffType& val2 ,
                                                  const CoeffType& val3, const CoeffType& val4 ) const
            {
                in_val += ( tap1*( val1 + val2 ) + tap2*( val3 + val4 ) + (1<<(shift-1)) )>>shift;
            }
        };

        //! Class to do two-tap prediction lifting step for Daubechies (9,7)
        template <int gain> class PredictStep97
        {
        public:

            //! Constructor
            PredictStep97(){}

            // Assume default copy constructor, assignment= and destructor //

            //! Do the filtering
            /*
                Do the filtering.
                \param   in_val   the value being predicted
                \param   val1   the first value being used for prediction
                \param   val2   the second value being used for prediction
            */
            inline void Filter(CoeffType& in_val, const CoeffType& val1, const CoeffType& val2) const
            {
                in_val -= static_cast< CoeffType >( (gain * static_cast< int >( val1 + val2 )) >>12 );
            }
        };

        //! Class to do two-tap update lifting step for Daubechies (9,7)
        template <int gain> class UpdateStep97
        {

        public:
            //! Constructor
            UpdateStep97(){}

            //! Do the filtering
            /*
                Do the filtering.
                \param   in_val   the value being updated
                \param   val1   the first value being used for updating
                \param   val2   the second value being used for updating
            */
            inline void Filter(CoeffType& in_val, const CoeffType& val1, const CoeffType& val2) const
            {
                in_val += static_cast< CoeffType >( (gain * static_cast< int >( val1 + val2 )) >>12 );
            }
        };   
    
    //! A class for wavelet coefficient data.
    /*!
        A class for encapsulating coefficient data, derived from TwoDArray..
     */
    class CoeffArray: public TwoDArray<CoeffType>
    {
    public:
        //! Default constructor
        /*!
            Default constructor creates an empty array.
        */
        CoeffArray(): TwoDArray<CoeffType>(){}

        //! Constructor.
        /*!
            Contructor creates a two-D array, with specified size and colour
            format.
        */
        CoeffArray(int height, int width, CompSort cs=Y_COMP):
            TwoDArray<CoeffType>(height, width), m_csort(cs){}

        //copy constructor and assignment= derived by inheritance

        //! Destructor
        ~CoeffArray(){}

        //! Return which component is stored
        const CompSort& CSort() const {return m_csort;}

        //! Set the type of component being stored
        void SetCSort(const CompSort cs){ m_csort = cs; }

        //! Returns the set of subbands
        SubbandList& BandList(){return m_band_list;}

        //! Returns the set of subbands
        const SubbandList& BandList() const {return m_band_list;}

        //! Sets the subband weights
        /*!
            Sets perceptual weights for the subbands. Takes into account both perceptual factors
            (weight noise less at higher spatial frequencies) and the scaling needed for the
            wavelet transform.
        */
        void SetBandWeights (const EncoderParams& encparams,
                                 const PictureParams& pparams,
                                 const CompSort csort,
				 const float cpd_scale_factor);

        private:

        CompSort m_csort;

        // The subband list to be used for conventional transform apps
        SubbandList m_band_list;

        private:

         //! Given x and y spatial frequencies in cycles per degree, returns a weighting value
        float PerceptualWeight(float xf,float yf,CompSort cs);

    };

    //! A class to do wavelet transforms
    /*!
        A class to do forward and backward wavelet transforms by iteratively splitting or merging the
        lowest frequency band.
    */
    class WaveletTransform
    {
    public:
        //! Constructor
        WaveletTransform(int d = 4, WltFilter f = DAUB9_7);

        //! Destructor
        virtual ~WaveletTransform();

        //! Transforms the data to and from the wavelet domain
        /*!
            Transforms the data to and from the wavelet domain.
            \param    d    the direction of the transform
            \param    pic_data    the data to be transformed
            \param    coeff_data  array that holds the transform coefficient data
        */
        void Transform(const Direction d, PicArray& pic_data, CoeffArray& coeff_data);

    private:


    private:

        // Private variables

        //! Depth of the transform
        int m_depth;

        //! The (vertical and horizontal) wavelet filter set to be used
        WltFilter m_filt_sort;

        //! A class to do the vertical and horizontal filtering required
        VHFilter* m_vhfilter;

    private:
        // Private functions
        //!    Private, bodyless copy constructor: class should not be copied
        WaveletTransform(const WaveletTransform& cpy);

        //! Private, bodyless copy operator=: class should not be assigned
        WaveletTransform& operator=(const WaveletTransform& rhs);

   };
}// end namespace dirac

#endif
