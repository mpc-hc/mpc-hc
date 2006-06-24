/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: me_utils.h 280 2005-01-30 05:11:46Z gabest $ $Name$
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
            m_yl(y_l)
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
        const int Xp() const {return m_xp;}

        //! Return the y-position of the top-left block corner
        const int Yp() const {return m_yp;}

        //! Return the block width
        const int Xl() const {return m_xl;}

        //! Return the block height
        const int Yl() const {return m_yl;}

    private: 

        int m_xp;
        int m_yp;
        int m_xl;
        int m_yl;

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

        //! Do the actual difference - virtual function must be overridden
        /*!
            Do the actual difference
            \param    dparams    the parameters in which costs, block parameters etc are stored
            \param    mv    the motion vector being used 
        */
        virtual float Diff(  const BlockDiffParams& dparams , const MVector& mv )=0;

    protected:

        const PicArray& pic_data;
        const PicArray& ref_data;

    private:
        //! Private, bodyless copy-constructor: class should not be copied
        BlockDiff( const BlockDiff& cpy );            

       //! Private, bodyless assignment=: class should not be assigned
        BlockDiff& operator=( const BlockDiff& rhs );    
    };

    //! A class for doing block differences without bounds-checking, inherited from BlockDiff
    class SimpleBlockDiff: public BlockDiff
    {
    public:
        //! Constructor, initialising the reference and picture data
        /*
             Constructor, initialising the reference and picture data
             \param  ref  the reference picture
             \param  pic  the picture being matched
        */
        SimpleBlockDiff( const PicArray& ref , const PicArray& pic );

        //! Do the actual difference without bounds checking
        /*!
            Do the actual difference without bounds checking
            \param    dparams    the parameters in which costs, block parameters etc are stored
            \param    mv    the motion vector being used 
        */
        float Diff(  const BlockDiffParams& dparams , const MVector& mv );

    private:
        //! Private, bodyless copy-constructor: class should not be copied
        SimpleBlockDiff(const SimpleBlockDiff& cpy);

       //! Private, bodyless assignment=: class should not be assigned
        SimpleBlockDiff& operator=(const SimpleBlockDiff& rhs);
    };

    //! A class for doing block differences with bounds-checking, inherited from BlockDiff
    class BChkBlockDiff: public BlockDiff
    {
    public:
        //! Constructor, initialising the reference and picture data
        /*
             Constructor, initialising the reference and picture data
             \param  ref  the reference picture
             \param  pic  the picture being matched
        */
        BChkBlockDiff( const PicArray& ref , const PicArray& pic );

        //! Do the actual difference with bounds checking
        /*!
            Do the actual difference with bounds checking
            \param    dparams    the parameters in which costs, block parameters etc are stored
            \param    mv    the motion vector being used 
        */    
        float Diff(  const BlockDiffParams& dparams , const MVector& mv );

    private:
        //! Private, bodyless copy-constructor: class should not be copied
        BChkBlockDiff(const BChkBlockDiff& cpy); 

       //! Private, bodyless assignment=: class should not be assigned
        BChkBlockDiff& operator=(const BChkBlockDiff& rhs);
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

        //! Do the actual difference
        /*!
            Do the actual difference
            \param    dparams    the parameters in which costs, block parameters etc are stored
            \param    dc_val     DC value
        */        
        float Diff( const BlockDiffParams& dparams , ValueType& dc_val );

    private:
        //! Private, bodyless copy-constructor: class should not be copied
        IntraBlockDiff(const IntraBlockDiff& cpy);

        //! Private, bodyless assignment=: class should not be assigned
        IntraBlockDiff& operator=(const IntraBlockDiff& rhs);

        const PicArray& pic_data;
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

        //! Do the actual difference
        /*!
            Do the actual difference
            \param    dparams    the parameters in which costs, block parameters etc are stored
            \param    mv1    the motion vector being used for reference 1
            \param    mv2    the motion vector being used for reference 2
        */        
        virtual float Diff(  const BlockDiffParams& dparams , const MVector& mv1 , const MVector& mv2 )=0;

    protected:
        const PicArray& pic_data;
        const PicArray& ref_data1;
        const PicArray& ref_data2;

    private:
        //! Private, bodyless copy-constructor: class should not be copied
        BiBlockDiff(const BiBlockDiff& cpy);             
                                                                    
        //! Private, bodyless assignment=: class should not be assigned
        BiBlockDiff& operator=(const BiBlockDiff& rhs);
    };


    //! A class for bi-directional differences with two references, and no bounds checking
    class BiSimpleBlockDiff: public BiBlockDiff
    {
    public:

        //! Constructor, initialising the references and picture data
        /*
             Constructor, initialising the references and picture data
             \param  ref1  the first reference picture
             \param  ref2  the second reference picture
             \param  pic  the picture being matched
        */
        BiSimpleBlockDiff( const PicArray& ref1 , const PicArray& ref2 , const PicArray& pic);

        //! Do the actual difference without bounds checking
        /*!
            Do the actual difference without bounds checking
            \param    dparams    the parameters in which costs, block parameters etc are stored
            \param    mv1    the motion vector being used for reference 1
            \param    mv2    the motion vector being used for reference 2
        */        
        float Diff(  const BlockDiffParams& dparams , const MVector& mv1 , const MVector& mv2 );

    private:
        //! Private, bodyless copy-constructor: class should not be copied
        BiSimpleBlockDiff(const BiSimpleBlockDiff& cpy);

        //! Private, bodyless assignment=: class should not be assigned
        BiSimpleBlockDiff& operator=(const BiSimpleBlockDiff& rhs);    
                                                                    
    };

    //! A class for bi-directional differences with two references, with bounds checking
    class BiBChkBlockDiff: public BiBlockDiff
    {
    public:
        //! Constructor, initialising the references and picture data
        BiBChkBlockDiff( const PicArray& ref1 , const PicArray& ref2 , const PicArray& pic );

        //! Do the actual difference with bounds checking
        /*!
            Do the actual difference with bounds checking
            \param    dparams    the parameters in which costs, block parameters etc are stored
            \param    mv1    the motion vector being used for reference 1
            \param    mv2    the motion vector being used for reference 2
        */        
        float Diff(  const BlockDiffParams& dparams , const MVector& mv1 , const MVector& mv2 );

    private:
        //! Private, bodyless copy-constructor: class should not be copied
        BiBChkBlockDiff(const BiBChkBlockDiff& cpy);             

        //! Private, bodyless assignment=: class should not be assigned
        BiBChkBlockDiff& operator=(const BiBChkBlockDiff& rhs);     
                                                                
    };

    // Classes where the reference is upconverted

    //! An abstract class for doing block differences with an upconverted reference
    class BlockDiffUp: public BlockDiff
    {
    public:    
        //! Constructor, initialising the reference and picture data
        /*
             Constructor, initialising the reference and picture data
             \param  ref  the reference picture
             \param  pic  the picture being matched
        */
        BlockDiffUp( const PicArray& ref , const PicArray& pic);

        //! Destructor
        virtual ~BlockDiffUp(){}

        //! Do the actual difference
        /*!
            Do the actual difference
            \param    dparams    the parameters in which costs, block parameters etc are stored
            \param    mv    the motion vector being used 
        */     
        virtual float Diff(  const BlockDiffParams& dparams , const MVector& mv )=0;

    protected:
         //! A lookup table to simplify the 1/8 pixel accuracy code
        int InterpLookup[9][4];

    private:
        //! Private, bodyless copy-constructor: class should not be copied
        BlockDiffUp(const BlockDiffUp& cpy); 
                                                        
        //! Private, bodyless assignment=: class should not be assigned
        BlockDiffUp& operator=(const BlockDiffUp& rhs);
                                                        
    };

    //! A class for doing block differences without bounds-checking with upconverted references, inherited from BlockDiffUp
    class SimpleBlockDiffUp: public BlockDiffUp
    {
    public:
        //! Constructor, initialising the reference and picture data
        /*
             Constructor, initialising the reference and picture data
             \param  ref  the reference picture
             \param  pic  the picture being matched
        */
        SimpleBlockDiffUp( const PicArray& ref , const PicArray& pic );

        //! Destructor
        ~SimpleBlockDiffUp(){}

        //! Do the actual difference without bounds checking
        /*!
            Do the actual difference without bounds checking
            \param    dparams    the parameters in which costs, block parameters etc are stored
            \param    mv    the motion vector being used 
        */        
        float Diff(  const BlockDiffParams& dparams , const MVector& mv );

    private:
        //! Private, bodyless copy-constructor: class should not be copied
        SimpleBlockDiffUp(const SimpleBlockDiffUp& cpy);

        //! Private, bodyless assignment=: class should not be assigned
        SimpleBlockDiffUp& operator=(const SimpleBlockDiffUp& rhs);
    };

    //! A class for doing block differences with bounds-checking with upconverted references, inherited from BlockDiffUp
    class BChkBlockDiffUp: public BlockDiffUp{

    public:
        //! Constructor, initialising the reference and picture data
        /*
             Constructor, initialising the reference and picture data
             \param  ref  the reference picture
             \param  pic  the picture being matched
        */
        BChkBlockDiffUp(const PicArray& ref,const PicArray& pic);

        //! Destructor
        ~BChkBlockDiffUp(){}

        //! Do the actual difference with bounds checking
        /*!
            Do the actual difference with bounds checking
            \param    dparams    the parameters in which costs, block parameters etc are stored
            \param    mv    the motion vector being used 
        */        
        float Diff(  const BlockDiffParams& dparams , const MVector& mv );
    private:
        //! Private, bodyless copy-constructor: class should not be copied
        BChkBlockDiffUp(const BChkBlockDiffUp& cpy);

        //! Private, bodyless assignment=: class should not be assigned
        BChkBlockDiffUp& operator=(const BChkBlockDiffUp& rhs);    
    };

    //! An abstract class for doing block differences with two upconverted references
    class BiBlockDiffUp: public BiBlockDiff
    {
    public:    
        //! Constructor, initialising the references and picture data
        /*
             Constructor, initialising the reference and picture data
             \param  ref1  the first reference picture
             \param  ref2  the second reference picture
             \param  pic  the picture being matched
        */
        BiBlockDiffUp( const PicArray& ref1 , const PicArray& ref2 , const PicArray& pic);

        //! Destructor
        virtual ~BiBlockDiffUp(){}

        //! Do the actual difference
        /*!
            Do the actual difference
            \param    dparams    the block parameters
            \param    mv1    the motion vector being used for reference 1
            \param    mv2    the motion vector being used for reference 2
        */ 
        virtual float Diff(  const BlockDiffParams& dparams , const MVector& mv1 , const MVector& mv2 )=0;
    protected:
         //! A lookup table to simplify the 1/8 pixel accuracy code
        int InterpLookup[9][4];

    private:
        //! Private, bodyless copy-constructor: class should not be copied
        BiBlockDiffUp(const BlockDiffUp& cpy); 
                                                        
        //! Private, bodyless assignment=: class should not be assigned
        BiBlockDiffUp& operator=(const BlockDiffUp& rhs);
                                                        
    };

    //! A class for doing bi-directional block differences without bounds checking
    class BiSimpleBlockDiffUp: public BiBlockDiffUp
    {
    public:
        //! Constructor, initialising the references and picture data
        /*
             Constructor, initialising the reference and picture data
             \param  ref1  the first reference picture
             \param  ref2  the second reference picture
             \param  pic  the picture being matched
        */
        BiSimpleBlockDiffUp( const PicArray& ref1 , const PicArray& ref2 , const PicArray& pic );

        //! Do the actual difference without bounds checking
        /*!
            Do the actual difference without bounds checking
            \param    dparams    the parameters in which costs, block parameters etc are stored
            \param    mv1    the motion vector being used for reference 1
            \param    mv2    the motion vector being used for reference 2
        */        
        float Diff(  const BlockDiffParams& dparams , const MVector& mv1 , const MVector& mv2 );
    private:
        //! Private, bodyless copy-constructor: class should not be copied
        BiSimpleBlockDiffUp(const BiSimpleBlockDiffUp& cpy);

        //! Private, bodyless assignment=: class should not be assigned
        BiSimpleBlockDiffUp& operator=(const BiSimpleBlockDiffUp& rhs);
    };

    //! A class for doing  bi-directional block differences with bounds checking
    class BiBChkBlockDiffUp: public BiBlockDiffUp
    {
    public:
        //! Constructor, initialising the references and picture data
        /*
             Constructor, initialising the reference and picture data
             \param  ref1  the first reference picture
             \param  ref2  the second reference picture
             \param  pic  the picture being matched
        */
        BiBChkBlockDiffUp( const PicArray& ref , const PicArray& ref2 , const PicArray& pic );

        //! Do the actual difference with bounds checking
        /*!
            Do the actual difference with bounds checking
            \param    dparams    the parameters in which costs, block parameters etc are stored
            \param    mv1    the motion vector being used for reference 1
            \param    mv2    the motion vector being used for reference 2
        */    
        float Diff(  const BlockDiffParams& dparams , const MVector& mv1 , const MVector& mv2 );
    private:
        //! Private, bodyless copy-constructor: class should not be copied
        BiBChkBlockDiffUp(const BiBChkBlockDiffUp& cpy);

        //! Private, bodyless assignment=: class should not be assigned
        BiBChkBlockDiffUp& operator=(const BiBChkBlockDiffUp& rhs);    
    };

} // namespace dirac
#endif
