/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: me_utils.h,v 1.14 2008/08/14 00:58:24 asuraparaju Exp $ $Name:  $
*
* Version: MPL 1.1/GPL 2.0/LGPL 2.1-
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
* Contributor(s): Thomas Davies (Original Author)
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

#ifndef _ME_UTILS_H_
#define _ME_UTILS_H_

#include <algorithm>
#include <libdirac_common/motion.h>
#include <libdirac_common/common.h>
namespace dirac
{

    ///////////////////////////////////
    //Utilities for motion estimation//
    //-------------------------------//
    ///////////////////////////////////

    //! A class encapsulating parameters for calculating a block difference value (a single instance of matching)
    class BlockDiffParams
    {

    public:
        //! Constructor
        BlockDiffParams(){}

        //! Constructor
        BlockDiffParams( const int x_p , const int y_p , const int x_l , const int y_l):
            m_xp(x_p),
            m_yp(y_p),
            m_xl(x_l),
            m_yl(y_l),
            m_xend(x_l+x_p),
            m_yend(y_l+y_p)
        {}

        ////////////////////////////////////////////////////////////////////
        //NB: Assume default copy constructor, assignment = and destructor//
        ////////////////////////////////////////////////////////////////////

        // Sets ...


        //! Set the limits of the block to fit in a picture
        
        void SetBlockLimits( const OLBParams& bparams ,
                             const PicArray& pic_data , 
                             const int xbpos , const int ybpos);

        // ... and gets

        //! Return the x-position of the top-left block corner
        int Xp() const {return m_xp;}

        //! Return the y-position of the top-left block corner
        int Yp() const {return m_yp;}

        //! Return the block width
        int Xl() const {return m_xl;}

        //! Return the block height
        int Yl() const {return m_yl;}

        //! Return the block horizontal endpoint
        int Xend() const {return m_xend;}

        //! Return the block vertical endpoint
        int Yend() const {return m_yend;}

    private: 

        int m_xp;
        int m_yp;
        int m_xl;
        int m_yl;
        int m_xend;
        int m_yend;
    };

    //////////////////////////////////////////////////
    //----Different difference classes, so that-----//
    //bounds-checking need only be done as necessary//
    //////////////////////////////////////////////////

    //! An abstract class for doing block difference calculations
    class BlockDiff
    {
    public:
        //! Constructor, initialising the reference and picture data
        /*
             Constructor, initialising the reference and picture data
             \param  ref  the reference picture
             \param  pic  the picture being matched
        */
        BlockDiff( const PicArray& ref , const PicArray& pic );

        //! Destructor    
        virtual ~BlockDiff(){}

        //! Do the difference, returning SAD
        /*!
            Do the difference, returning SAD
            \param    dparams block parameters
            \param    mv      the motion vector being used 
        */
        virtual float Diff(  const BlockDiffParams& dparams , const MVector& mv )=0;

    protected:

        const PicArray& m_pic_data;
        const PicArray& m_ref_data;

    private:
        //! Private, bodyless copy-constructor: class should not be copied
        BlockDiff( const BlockDiff& cpy );            

       //! Private, bodyless assignment=: class should not be assigned
        BlockDiff& operator=( const BlockDiff& rhs );    
    };

    //! A class for doing block differences to pixel accuracy, inherited from BlockDiff
    class PelBlockDiff: public BlockDiff
    {
    public:
        //! Constructor, initialising the reference and picture data
        /*
             Constructor, initialising the reference and picture data
             \param  ref  the reference picture
             \param  pic  the picture being matched
        */
        PelBlockDiff( const PicArray& ref , const PicArray& pic );

        //! Do the difference, returning SAD
        /*!
            Do the difference, returning SAD
            \param    dparams    block parameters
            \param    mv         the motion vector being used 
        */
        float Diff(  const BlockDiffParams& dparams , const MVector& mv );

        //! Do the difference, overwriting the best MV so far if appropriate
        /*!
            Do the difference, overwriting the best MV so far if appropriate,
            and bailing out if we do worse
            \param    dparams    block parameters
            \param    mv         the motion vector being used 
            \param    best_sum   the best SAD value obtain yet
            \param    best_mv    the MV giving the best SAD value so far    
        */
        void Diff(  const BlockDiffParams& dparams ,
                    const MVector& mv ,
                    float& best_sum , 
                    MVector& best_mv ); 

    private:
        //! Private, bodyless copy-constructor: class should not be copied
        PelBlockDiff(const PelBlockDiff& cpy);

       //! Private, bodyless assignment=: class should not be assigned
        PelBlockDiff& operator=(const PelBlockDiff& rhs);
    };


    //! A class for calculating the difference between a block and its DC value (average)
    class IntraBlockDiff
    {
    public:
        //! Constructor, initialising the picture data
        /*
             Constructor, initialising the picture data
             \param  pic  the picture being matched
        */
        IntraBlockDiff( const PicArray& pic );

        //! Do the difference, calculating the DC value and returning SAD
        /*!
            Do the difference, calculating the DC value and returning SAD
            \param    dparams    block parameters
            \param    dc_val     DC value
        */        
        float Diff( const BlockDiffParams& dparams , ValueType& dc_val );

        //! Calculate a DC value
	ValueType CalcDC( const BlockDiffParams& dparams);

    private:
        //! Private, bodyless copy-constructor: class should not be copied
        IntraBlockDiff(const IntraBlockDiff& cpy);

        //! Private, bodyless assignment=: class should not be assigned
        IntraBlockDiff& operator=(const IntraBlockDiff& rhs);

        const PicArray& m_pic_data;
    };

    //! A virtual class for bi-directional differences
    class BiBlockDiff
    {
    public:
        //! Constructor, initialising the references and picture data
        /*
             Constructor, initialising the references and picture data
             \param  ref1  the first reference picture
             \param  ref2  the second reference picture
             \param  pic  the picture being matched
        */
        BiBlockDiff( const PicArray& ref1 , const PicArray& ref2 , const PicArray& pic);

        //! Virtual destructor
        virtual ~BiBlockDiff( ) {}

        //! Do the difference, returning SAD
        /*!
            Do the difference, returning SAD
            \param    dparams block parameters
            \param    mv1     the motion vector being used for reference 1
            \param    mv2     the motion vector being used for reference 2
        */        
        virtual float Diff(  const BlockDiffParams& dparams , const MVector& mv1 , const MVector& mv2 )=0;

    protected:
        const PicArray& m_pic_data;
        const PicArray& m_ref_data1;
        const PicArray& m_ref_data2;

    private:
        //! Private, bodyless copy-constructor: class should not be copied
        BiBlockDiff(const BiBlockDiff& cpy);             
                                                                    
        //! Private, bodyless assignment=: class should not be assigned
        BiBlockDiff& operator=(const BiBlockDiff& rhs);
    };

    // Classes where the reference is upconverted //
    //////////////////////////////////////////////// 
    //! A virtual class for doing differences with sub-pixel vectors
     class BlockDiffUp: public BlockDiff
     {
     
     public:

         //! Constructor, initialising the reference and picture data
         /*
              Constructor, initialising the reference and picture data
              \param  ref  the reference picture
              \param  pic  the picture being matched
         */
         BlockDiffUp( const PicArray& ref , const PicArray& pic );

         //! Destructor
         virtual ~BlockDiffUp(){}

        //! Do the difference, returning SAD
        /*!
            Do the difference, returning SAD
            \param    dparams    block parameters
            \param    mv         the motion vector being used 
        */
         virtual float Diff(  const BlockDiffParams& dparams , const MVector& mv )=0;

        //! Do the actual difference, overwriting the best MV so far if appropriate
        /*!
            Do the actual difference, overwriting the best MV so far if appropriate,
            and bailing out if we do worse
            \param    dparams    block parameters
            \param    mv         the motion vector being used 
            \param    mvcost     the (prediction) cost of the motion vector mv 
            \param    lambda     the weighting to be given to mvcost
            \param    best_costs the best Lagrangian costs obtained yet
            \param    best_mv    the MV giving the best Lagrangian costs so far    
        */
         virtual void Diff( const BlockDiffParams& dparams,
                            const MVector& mv ,
                            const float mvcost,
                            const float lambda,
                            MvCostData& best_costs ,
                            MVector& best_mv)=0;
     private:
         //! Private, bodyless copy-constructor: class should not be copied
         BlockDiffUp(const BlockDiffUp& cpy);

         //! Private, bodyless assignment=: class should not be assigned
         BlockDiffUp& operator=(const BlockDiffUp& rhs);
     };
 
    //! A class for doing differences with half-pixel accurate vectors
    class BlockDiffHalfPel: public BlockDiffUp
    {

    public:
        //! Constructor, initialising the reference and picture data
        /*
              Constructor, initialising the reference and picture data
              \param  ref  the reference picture
              \param  pic  the picture being matched
        */
        BlockDiffHalfPel( const PicArray& ref , const PicArray& pic );

        //! Destructor
        ~BlockDiffHalfPel(){}

        //! Do the difference, returning SAD
        /*!
            Do the difference, returning SAD
            \param    dparams    block parameters
            \param    mv         the motion vector being used 
        */
        float Diff(  const BlockDiffParams& dparams , const MVector& mv );

        //! Do the actual difference, overwriting the best MV so far if appropriate
        /*!
            Do the actual difference, overwriting the best MV so far if appropriate,
            and bailing out if we do worse
            \param    dparams    block parameters
            \param    mv         the motion vector being used 
            \param    mvcost     the (prediction) cost of the motion vector mv 
            \param    lambda     the weighting to be given to mvcost
            \param    best_costs the best Lagrangian costs obtained yet
            \param    best_mv    the MV giving the best Lagrangian costs so far    
        */
        void Diff( const BlockDiffParams& dparams,
                    const MVector& mv ,
                    const float mvcost,
                    const float lambda,
                    MvCostData& best_costs ,
                    MVector& best_mv);

    private:
        //! Private, bodyless copy-constructor: class should not be copied
        BlockDiffHalfPel(const BlockDiffHalfPel& cpy);

        //! Private, bodyless assignment=: class should not be assigned
        BlockDiffHalfPel& operator=(const BlockDiffHalfPel& rhs);

    };

    //! A class for doing differences with quarter-pixel accurate vectors
    class BlockDiffQuarterPel: public BlockDiffUp
    {
    public:
        //! Constructor, initialising the reference and picture data
        /*
             Constructor, initialising the reference and picture data
             \param  ref  the reference picture
             \param  pic  the picture being matched
        */
        BlockDiffQuarterPel( const PicArray& ref , const PicArray& pic );

        //! Destructor
        ~BlockDiffQuarterPel(){}

        //! Do the difference, returning SAD
        /*!
            Do the difference, returning SAD
            \param    dparams    block parameters
            \param    mv         the motion vector being used 
        */
        float Diff(  const BlockDiffParams& dparams , const MVector& mv );

        //! Do the actual difference, overwriting the best MV so far if appropriate
        /*!
            Do the actual difference, overwriting the best MV so far if appropriate,
            and bailing out if we do worse
            \param    dparams    block parameters
            \param    mv         the motion vector being used 
            \param    mvcost     the (prediction) cost of the motion vector mv 
            \param    lambda     the weighting to be given to mvcost
            \param    best_costs the best Lagrangian costs obtained yet
            \param    best_mv    the MV giving the best Lagrangian costs so far    
        */
        void Diff( const BlockDiffParams& dparams,
                   const MVector& mv ,
                   const float mvcost,
                   const float lambda,
                   MvCostData& best_costs ,
                   MVector& best_mv);

    private:
        //! Private, bodyless copy-constructor: class should not be copied
        BlockDiffQuarterPel(const BlockDiffQuarterPel& cpy);

        //! Private, bodyless assignment=: class should not be assigned
        BlockDiffQuarterPel& operator=(const BlockDiffQuarterPel& rhs);
    };

    //! A class for doing differences with eighth-pixel accurate vectors
    class BlockDiffEighthPel: public BlockDiffUp
    {
    public:
        //! Constructor, initialising the reference and picture data
        /*
             Constructor, initialising the reference and picture data
             \param  ref  the reference picture
             \param  pic  the picture being matched
        */
        BlockDiffEighthPel( const PicArray& ref , const PicArray& pic );

        //! Destructor
        ~BlockDiffEighthPel(){}

        //! Do the difference, returning SAD
        /*!
            Do the difference, returning SAD
            \param    dparams    block parameters
            \param    mv         the motion vector being used 
        */
        float Diff(  const BlockDiffParams& dparams , const MVector& mv );

        //! Do the actual difference, overwriting the best MV so far if appropriate
        /*!
            Do the actual difference, overwriting the best MV so far if appropriate,
            and bailing out if we do worse
            \param    dparams    block parameters
            \param    mv         the motion vector being used 
            \param    mvcost     the (prediction) cost of the motion vector mv 
            \param    lambda     the weighting to be given to mvcost
            \param    best_costs the best Lagrangian costs obtained yet
            \param    best_mv    the MV giving the best Lagrangian costs so far    
        */
        void Diff( const BlockDiffParams& dparams,
                    const MVector& mv ,
                    const float mvcost,
                    const float lambda,
                    MvCostData& best_costs ,
                    MVector& best_mv);

    private:
        //! Private, bodyless copy-constructor: class should not be copied
        BlockDiffEighthPel(const BlockDiffEighthPel& cpy);

        //! Private, bodyless assignment=: class should not be assigned
        BlockDiffEighthPel& operator=(const BlockDiffEighthPel& rhs);
    };

    //! A class for computing a bidirection difference for half-pel vectors
    class BiBlockHalfPel: public BiBlockDiff
    {
    public:
        //! Constructor, initialising the references and picture data
        /*
             Constructor, initialising the reference and picture data
             \param  ref1  the first reference picture
             \param  ref2  the second reference picture
             \param  pic  the picture being matched
        */
        BiBlockHalfPel( const PicArray& ref1 , const PicArray& ref2 , const PicArray& pic );

        //! Do the difference, returning SAD
        /*!
            Do the difference, returning SAD
            \param    dparams block parameters
            \param    mv1     the motion vector being used for reference 1
            \param    mv2     the motion vector being used for reference 2
        */        
        float Diff(  const BlockDiffParams& dparams , const MVector& mv1 , const MVector& mv2 );
    private:
        //! Private, bodyless copy-constructor: class should not be copied
        BiBlockHalfPel(const BiBlockHalfPel& cpy);

        //! Private, bodyless assignment=: class should not be assigned
        BiBlockHalfPel& operator=(const BiBlockHalfPel& rhs);
    };

    //! A class for computing a bidirection difference for quarter-pel vectors
    class BiBlockQuarterPel: public BiBlockDiff
    {
    public:
        //! Constructor, initialising the references and picture data
        /*
             Constructor, initialising the reference and picture data
             \param  ref1  the first reference picture
             \param  ref2  the second reference picture
             \param  pic  the picture being matched
        */
        BiBlockQuarterPel( const PicArray& ref1 , const PicArray& ref2 , const PicArray& pic );

        //! Do the difference, returning SAD
        /*!
            Do the difference, returning SAD
            \param    dparams block parameters
            \param    mv1    the motion vector being used for reference 1
            \param    mv2    the motion vector being used for reference 2
        */        
        float Diff(  const BlockDiffParams& dparams , const MVector& mv1 , const MVector& mv2 );

    private:
        //! Private, bodyless copy-constructor: class should not be copied
        BiBlockQuarterPel(const BiBlockQuarterPel& cpy);

        //! Private, bodyless assignment=: class should not be assigned
        BiBlockQuarterPel& operator=(const BiBlockQuarterPel& rhs);
    };

    //! A class for computing a bidirection difference for eighth-pel vectors
    class BiBlockEighthPel: public BiBlockDiff
    {
    public:
        //! Constructor, initialising the references and picture data
        /*
             Constructor, initialising the reference and picture data
             \param  ref1  the first reference picture
             \param  ref2  the second reference picture
             \param  pic  the picture being matched
        */
        BiBlockEighthPel( const PicArray& ref1 , const PicArray& ref2 , const PicArray& pic );

        //! Do the difference, returning SAD
        /*!
            Do the difference, returning SAD
            \param    dparams block parameters
            \param    mv1     the motion vector being used for reference 1
            \param    mv2     the motion vector being used for reference 2
        */        
        float Diff(  const BlockDiffParams& dparams , const MVector& mv1 , const MVector& mv2 );
    private:
        //! Private, bodyless copy-constructor: class should not be copied
        BiBlockEighthPel(const BiBlockEighthPel& cpy);

        //! Private, bodyless assignment=: class should not be assigned
        BiBlockEighthPel& operator=(const BiBlockEighthPel& rhs);
    };

} // namespace dirac
#endif
