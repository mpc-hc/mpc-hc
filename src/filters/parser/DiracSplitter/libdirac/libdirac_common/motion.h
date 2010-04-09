/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: motion.h,v 1.30 2008/10/01 01:26:47 asuraparaju Exp $ $Name:  $
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
*                 Chris Bowley,
*                 Tim Borer
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

#include <libdirac_common/common.h>
#include <algorithm>
#ifndef _MOTION_H
#define _MOTION_H

namespace dirac
{
    ////////////////////////////////////////////////////////////////
    //classes and functions for motion estimation and compensation//
    ////////////////////////////////////////////////////////////////

    //classes

    //! Horizontal or vertical
    enum MvElement { HORIZONTAL , VERTICAL };

    //! Motion vector class - just a pair
    template <class T>
    class MotionVector
    {
    public:

        //! Constructor 
        MotionVector<T>(T a, T b) : x(a), y(b) {};
        //! Default construct - sets components to 0
        MotionVector<T>() : x(0), y(0) {};
        //! Constructor 
        MotionVector<T>(T a) : x(a), y(a) {};

        //! Addition 
        inline MotionVector<T> operator+(const MotionVector<T>& argument) const;

        //! Subtraction 
        inline MotionVector<T> operator-(const MotionVector<T>& argument) const;

        //! Scalar multiplication
        inline MotionVector<T> operator*(const float argument) const;

        //! Scalar multiplication
        inline MotionVector<T> operator*(const int argument) const;

        //! Bitshift of each component
        inline MotionVector<T> operator<<(const int argument) const;

        //! Bitshift of each component
        inline MotionVector<T> operator>>(const int argument) const;
        
        //! Array-style element access
        T& operator[](const int pos){return ( ( pos==0) ? x : y );}

        //! Array-style element access.
        const T& operator[](const int pos) const {return ( ( pos==0) ? x : y );}


        //! x and y components 
        T x,y;

    };


    template <class T>
    inline MotionVector<T> MotionVector<T>::operator+(const MotionVector<T>& argument) const 
    {
        MotionVector<T> temp;
        temp.x = x + argument.x;
        temp.y = y + argument.y;

        return temp;
    }

    template <class T>
    inline MotionVector<T>  MotionVector<T>::operator-(const MotionVector<T>& argument) const 
    {
        MotionVector<T> temp;
        temp.x = x-argument.x;
        temp.y = y-argument.y;

        return temp;
    }

    template <class T>
    inline MotionVector<T>  MotionVector<T>::operator*(const float argument) const 
    {
        MotionVector<T> temp;
        temp.x = x*argument;
        temp.y = y*argument;

        return temp;
    }

    template <class T>
    inline MotionVector<T>  MotionVector<T>::operator*(const int argument) const 
    {
        MotionVector<T> temp;
        temp.x = x*argument;
        temp.y = y*argument;

        return temp;
    }

    template <class T>
    inline MotionVector<T>  MotionVector<T>::operator<<(const int argument) const 
    {
        MotionVector<T> temp;
        temp.x = x<<argument;
        temp.y = y<<argument;

        return temp;
    }

    template <class T>
    inline MotionVector<T>  MotionVector<T>::operator>>(const int argument) const 
    {
        MotionVector<T> temp;
        temp.x = x>>argument;
        temp.y = y>>argument;

        return temp;
    }

    //! Overloaded operator<< for MotionVector class for output to stream
    template <class T>
    std::ostream & operator<< (std::ostream & stream, MotionVector<T> & mv)
    {
        stream << mv.x << " " << mv.y;

        return stream;
    }

    //! Overloaded operator>> for MotionVector class for input from stream
    template <class T>
    std::istream & operator>> (std::istream & stream, MotionVector<T> & mv)
    {
        stream >> mv.x;
        stream >> mv.y;

        return stream;
    }

    //! MVector class is a vector of ints 
    typedef MotionVector<int> MVector;

    //! ImageCoords class is a vector of ints 
    typedef MotionVector<int> ImageCoords;

    //! MvArray is a two-D array of MVectors
    typedef TwoDArray<MVector> MvArray;

    //! An array of float-based motion vectors for doing global motion calcs
    typedef TwoDArray< MotionVector<float> > MvFloatArray;

    //! Class for recording costs derived in motion estimation
    class MvCostData
    {
    public:
        //! Constructor
        MvCostData():
        SAD(0.0),
        mvcost(0.0),
        total(0.0){}

        void SetTotal( const float lambda ){total = SAD + lambda*mvcost;}

        //! The Sum of Absolute Differences - easier to compute than Sum-Squared Differences
        float SAD;

        //! The (Lagrangian-weighted) motion vector cost - the difference of a motion vector from its neighbouring vectors
        float mvcost;

        //! Total=SAD+mvcost
        float total;
    };


    //! Class for all the motion vector data
    /*!
         Motion vector data: the motion vectors themselves, the blocks 
         and macroblock modes.
    */
    class MvData
    {
    public:
        //! Constructor
        /*! 
            Constructor takes:
            \param  predparams   Picture prediction parameters
            \param  num_refs     the number of references being used for the picture
        */
        MvData( const PicturePredParams& predparams ,  const int num_refs);

        //! Destructor
        ~MvData();

         //! Return a reference to the local picture prediction params
        PicturePredParams& GetPicPredParams(){return m_predparams;}

        //! Return a reference to the local picture prediction params
        const PicturePredParams& GetPicPredParams() const{return m_predparams;}

        //! Get the MVs for a reference
        MvArray& Vectors(const int ref_id){return *( m_vectors[ref_id] );}

        //! Get the MVs for a reference
        const MvArray& Vectors(const int ref_id) const {return *( m_vectors[ref_id] );}

        //! Get the global MVs for a reference
        MvArray& GlobalMotionVectors(const int ref_id){return *( m_gm_vectors[ref_id] );}

        //! Get the global MVs for a reference
        const MvArray& GlobalMotionVectors(const int ref_id) const {return *( m_gm_vectors[ref_id] );} 

        //! Get the DC values for each component
        TwoDArray<ValueType>& DC(CompSort cs){return *( m_dc[cs] );}

        //! Get the DC values for each component
        const TwoDArray<ValueType>& DC(CompSort cs) const {return *( m_dc[cs] );}

        //! Get a reference to the vector holding component DC values
        const OneDArray< TwoDArray<ValueType>* >& DC() const {return m_dc;}

        //! Get the block prediction modes
        TwoDArray<PredMode>& Mode(){return m_modes;}

        //! Get the block prediction modes
        const TwoDArray<PredMode>& Mode() const {return m_modes;}

        //! Get the SB split level
        TwoDArray<int>& SBSplit(){return m_sb_split;}

        //! Get the SB split level
        const TwoDArray<int>& SBSplit() const{return m_sb_split;}

        //! Get the global motion model parameters
        OneDArray<float>& GlobalMotionParameters(const int ref_id) { return *( m_gm_params[ref_id] ); }

        //! Get the global motion model parameters
        const OneDArray<float>& GlobalMotionParameters(const int ref_id) const { return *( m_gm_params[ref_id] ); }

    protected:
        // A local copy of the picture prediction parameters
	PicturePredParams m_predparams;

        // Initialises the arrays of data
        void InitMvData();

        // The motion vectors
        OneDArray<MvArray*> m_vectors;

        // The global motion vectors
        OneDArray<MvArray*> m_gm_vectors;

        // The block modes
        TwoDArray<PredMode> m_modes;

        // The DC values
        OneDArray< TwoDArray<ValueType>* > m_dc;

        // The SB split levels
        TwoDArray<int> m_sb_split;

        // Global motion model parameters
        OneDArray< OneDArray<float>* > m_gm_params;

//        // Number of reference frames
//        unsigned int m_num_refs;
    };

    //! Class for all the motion estimation data
    /*!
         Motion estimation data: derived from MvData class, also
         incorporates costs for blocks and macroblocks
    */

    class MEData: public MvData
    {
    public:

        //! Constructor
        /*! 
            Constructor takes:
	        \param predparams the picture prediction parameters
            \param  num_refs  the number of references being used for the picture
        */
        MEData( const PicturePredParams& predparams , const int num_refs = 2);

        //! Destructor
        ~MEData();
	
	//! drop the data relating to one reference
	void DropRef( int ref_index );

        //! Get the block cost structures for each reference
        TwoDArray<MvCostData>& PredCosts(const int ref_id){ return *( m_pred_costs[ref_id] ); }

        //! Get the block cost structures for each reference
        const TwoDArray<MvCostData>& PredCosts(const int ref_id) const { return *( m_pred_costs[ref_id] ); }

        //! Get the intra costs
        TwoDArray<float>& IntraCosts(){ return m_intra_costs; }

        //! Get the intra costs
        const TwoDArray<float>& IntraCosts() const { return m_intra_costs; }

        //! Get the bipred costs
        TwoDArray<MvCostData>& BiPredCosts(){ return m_bipred_costs; }

        //! Get the bipred costs
        const TwoDArray<MvCostData>& BiPredCosts() const { return m_bipred_costs; }

        //! Get the SB costs
        TwoDArray<float>& SBCosts(){ return m_SB_costs; }

        //! Get the SB costs
        const TwoDArray<float>& SBCosts() const { return m_SB_costs; }

	//! Get the proportion of intra blocks
	float IntraBlockRatio() const {return m_intra_block_ratio; }

	//! Set the intra block ratio
	void SetIntraBlockRatio(const float r){ m_intra_block_ratio = r; }

        //! Set up the lambda map by detecting motion discontinuities 
        void SetLambdaMap( const int num_refs , const float lambda );

        //! Set up the lambda map by averaging the lambda map from a lower level 
        void SetLambdaMap( const int level , const TwoDArray<float>& l_map , const float wt );

        //! Get a lambda value for a given block and level
        const TwoDArray<float>& LambdaMap() const { return m_lambda_map; }

        //! Get the inliers for each reference
        TwoDArray<int>& GlobalMotionInliers(const int ref_id){ return *( m_inliers[ref_id] ); }

        //! Get the inliers for each reference
        const TwoDArray<int>& GlobalMotionInliers(const int ref_id) const { return *( m_inliers[ref_id] ); }

        //! Overloaded operator<< for outputing to (file) stream
        friend std::ostream &operator<< (std::ostream & stream, MEData & me_data);

        //! Overloaded operator>> for input of data from (file) stream
        friend std::istream &operator>> (std::istream & stream, MEData & me_data);

    private:
        // Initialises the arrays of data
        void InitMEData();

        // Finds transitions in the motion vectors
        void FindTransitions( TwoDArray<bool>& trans_map , const int ref_num );

        // The costs of predicting each block, for each reference
        OneDArray< TwoDArray<MvCostData>* > m_pred_costs;

        // The costs of predicting each block by DC
        TwoDArray<float> m_intra_costs;

        // The costs of predicting each block bidirectionally
        TwoDArray<MvCostData> m_bipred_costs;

        // The costs for each macroblock as a whole
        TwoDArray<float> m_SB_costs;

        // A map of the lambda values to use
        TwoDArray<float> m_lambda_map;

        // Global motion inliers
        OneDArray< TwoDArray<int>* > m_inliers;

	// Intra block ratio
	float m_intra_block_ratio;

    };

    //motion estimation and coding stuff
    
    //! Return the median of 3 integers
    int Median( const int val1, const int val2, const int val3);

    //! Return the median of three motion vectors 
    MVector MvMedian(const MVector& mv1,const MVector& mv2,const MVector& mv3);


    //! Return the median of a set of integers 
    int Median(const std::vector<int>& val_list);

    //! Return the median of a set of (up to 4) motion vectors 
    MVector MvMedian(const std::vector<MVector>& vect_list);

    //! Return the mean of two motion vectors
    MVector MvMean(const MVector& mv1, const MVector& mv2);

    //! Return the squared length of a motion vector
    inline int Norm2(const MVector& mv){//L^2 norm of a motion vector
        return mv.x*mv.x+mv.y*mv.y;
    }

    //! Return the sum of the lengths of a motion vector's componets
    inline int Norm1(const MVector& mv){//L^1 norm of a motion vector
        return abs(mv.x)+abs(mv.y);
    }
    
    //! Return the mean of a set of unsigned integer values
    unsigned int GetUMean(std::vector<unsigned int>& values);
    
    //! Return the mean of a set of signed integer values
    int GetSMean(std::vector<int>& values);

} // namespace dirac

#endif
