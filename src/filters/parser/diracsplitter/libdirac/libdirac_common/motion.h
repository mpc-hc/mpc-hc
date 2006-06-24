/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: motion.h 280 2005-01-30 05:11:46Z gabest $ $Name$
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
* Contributor(s): Thomas Davies (Original Author), Chris Bowley
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
            \param  xnumMB  the number of MBs horizontally
            \param  ynumMB  the number of MBs vertically
            \param  xnumblocks  the number of blocks horizontally
            \param  ynumblocks  the number of blocks vertically
            \param  num_refs  the number of references being used for the frame
        */
        MvData( const int xnumMB, int ynumMB , 
                const int xnumblocks, int ynumblocks ,  const int num_refs = 2);

        //! Constructor
        /*! 
            Constructor. Numbers of blocks derived from the number of MBs
            \param  xnumMB  the number of MBs horizontally
            \param  ynumMB  the number of MBs vertically
            \param  num_refs  the number of references being used for the frame
        */
        MvData( const int xnumMB, int ynumMB ,  const int num_refs = 2);

        //! Destructor
        ~MvData();

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
     
        //! Get the MB split level
        TwoDArray<int>& MBSplit(){return m_mb_split;}

        //! Get the MB split level
        const TwoDArray<int>& MBSplit() const{return m_mb_split;}

        //! Get the MB common mode parameters
        TwoDArray<bool>& MBCommonMode(){return m_mb_common;}

        //! Get the MB common mode parameters
        const TwoDArray<bool>& MBCommonMode() const{return m_mb_common;}

        //! Get the global motion model parameters
        OneDArray<float>& GlobalMotionParameters(const int ref_id) { return *( m_gm_params[ref_id] ); }

        //! Get the global motion model parameters
        const OneDArray<float>& GlobalMotionParameters(const int ref_id) const { return *( m_gm_params[ref_id] ); }

    private:
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

        // The MB split levels
        TwoDArray<int> m_mb_split;

        // The MB common mode indicators 
        TwoDArray<bool> m_mb_common;

        // Global motion model parameters
        OneDArray< OneDArray<float>* > m_gm_params;
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
            \param  xnumMB  the number of MBs horizontally
            \param  ynumMB  the number of MBs vertically
            \param  xnumblocks  the number of blocks horizontally
            \param  ynumblocks  the number of blocks vertically
            \param  num_refs  the number of references being used for the frame
        */
        MEData( const int xnumMB, const int ynumMB , 
                const int xnumblocks, const int ynumblocks , const int num_refs = 2);

        //! Constructor
        /*! 
            Constructor. Numbers of blocks derived from the number of MBs
            \param  xnumMB  the number of MBs horizontally
            \param  ynumMB  the number of MBs vertically
            \param  num_refs  the number of references being used for the frame
        */
        MEData( const int xnumMB, const int ynumMB , const int num_refs = 2);

        //! Destructor
        ~MEData();

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

        //! Get the MB costs
        TwoDArray<float>& MBCosts(){ return m_MB_costs; }

        //! Get the MB costs
        const TwoDArray<float>& MBCosts() const { return m_MB_costs; }

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
        TwoDArray<float> m_MB_costs;

        // A map of the lambda values to use
        TwoDArray<float> m_lambda_map;

        // Global motion inliers
        OneDArray< TwoDArray<int>* > m_inliers;

    };

    //motion estimation and coding stuff

    //! Return the median of three motion vectors 
    inline MVector MvMedian(const MVector& mv1,const MVector& mv2,const MVector& mv3) {
        //takes median of each vector component    
        MVector tmp_mv;

        tmp_mv.x=mv1.x;
        tmp_mv.x+=mv2.x;
        tmp_mv.x+=mv3.x;

        tmp_mv.x-=std::max(std::max(mv1.x,mv2.x),mv3.x);
        tmp_mv.x-=std::min(std::min(mv1.x,mv2.x),mv3.x);

        tmp_mv.y=mv1.y;
        tmp_mv.y+=mv2.y;
        tmp_mv.y+=mv3.y;

        tmp_mv.y-=std::max(std::max(mv1.y,mv2.y),mv3.y);
        tmp_mv.y-=std::min(std::min(mv1.y,mv2.y),mv3.y);

        return tmp_mv;
    }

    //! Return the median of a set of motion vectors 
    inline MVector MvMedian(const std::vector<MVector>& vect_list){
        //more general median. Takes the median of each vector component    

        MVector median;
        int num_vals=int(vect_list.size());
        if (num_vals>0)    {
            int pos=0;
            std::vector<int> ordered_vals(vect_list.size());
            //do x first
            ordered_vals[0]=vect_list[0].x;        
            for (int I=1;I<num_vals;++I){
                for (int K=0;K<I;++K){
                    if (vect_list[I].x<ordered_vals[K]){
                        pos=K;
                        break;
                    }
                    else
                        pos=K+1;
                }//K
                if (pos==I)
                    ordered_vals[I]=vect_list[I].x;
                else{
                    for (int K=pos;K>=I-1;--K){
                        ordered_vals[K+1]=ordered_vals[K];
                    }
                    ordered_vals[pos]=vect_list[I].x;
                }
            }//I
            if (vect_list.size()%2!=0)
                median.x=ordered_vals[(num_vals-1)/2];
            else
                median.x=(ordered_vals[(num_vals/2)-1]+ordered_vals[num_vals/2])/2;

            //now do y
            ordered_vals[0]=vect_list[0].y;        
            for (int I=1;I<num_vals;++I){
                for (int K=0;K<I;++K){
                    if (vect_list[I].y<ordered_vals[K]){
                        pos=K;
                        break;
                    }
                    else
                        pos=K+1;
                }//K
                if (pos==I)
                    ordered_vals[I]=vect_list[I].y;
                else{
                    for (int K=pos;K>=I-1;--K){
                        ordered_vals[K+1]=ordered_vals[K];
                    }
                    ordered_vals[pos]=vect_list[I].y;
                }
            }//I
            if (num_vals%2!=0)
                median.y=ordered_vals[(num_vals-1)/2];
            else
                median.y=(ordered_vals[(num_vals/2)-1]+ordered_vals[num_vals/2])/2;        

        }
        else{
            median.x=0;
            median.y=0;
        }
        return median;
    }

    //! Return the mean of two motion vectors
    inline MVector MvMean(MVector& mv1,MVector& mv2) {
        //takes median of each vector component    
        MVector tmp_mv;

        tmp_mv.x=mv1.x;
        tmp_mv.x+=mv2.x;
        tmp_mv.x/=2;

        tmp_mv.y=mv1.y;
        tmp_mv.y+=mv2.y;
        tmp_mv.y/=2;

        return tmp_mv;
    }

    //! Return the squared length of a motion vector
    inline int Norm2(const MVector& mv){//L^2 norm of a motion vector
        return mv.x*mv.x+mv.y*mv.y;
    }

    //! Return the sum of the lengths of a motion vector's componets
    inline int Norm1(const MVector& mv){//L^1 norm of a motion vector
        return abs(mv.x)+abs(mv.y);
    }

    //! Return the mean of a set of integer values
    inline int GetMean(std::vector<int>& values){
        int sum=0;
        for (unsigned int I=0;I<values.size();++I)
            sum+=values[I];
        sum/=int(values.size());
        return sum;
    }

    //! Return the mean of a set of unsigned integer values
    inline unsigned int GetMean(std::vector<unsigned int>& values){
        int sum=0;
        for (unsigned int I=0;I<values.size();++I)
            sum+=values[I];
        sum+=(values.size()>>1);
        sum/=values.size();
        return sum;
    }

} // namespace dirac

#endif
