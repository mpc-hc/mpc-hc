/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: frame.h 280 2005-01-30 05:11:46Z gabest $ $Name$
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

#ifndef _FRAME_H_
#define _FRAME_H_

#include <libdirac_common/common.h>

namespace dirac
{
    //! A class for encapsulating all the data relating to a frame.
    /*!
        A class for encapsulating all the data relating to a frame - all the 
        component data, including upconverted data.
     */
    class Frame
    {

    public:

        //! Constructor
        /*!
            Constructor initialises the frame parameters and the data
         */    
        Frame( const FrameParams& fp );

        //! Copy constructor. Private as not currently used [may want to implement reference counting later.]
        Frame(const Frame& cpy);

        //! Destructor
        virtual ~Frame();

        //! Assignment =. Private as not currently used [may want to implement reference counting later.]
        Frame& operator=( const Frame& rhs );

        //gets and sets
        //! Gets the frame parameters
        const FrameParams& GetFparams() const {return m_fparams;}

        //! Sets the frame sort
        void SetFrameSort( const FrameSort fs ){m_fparams.SetFSort( fs ); }

        //! Returns the luma data array
        PicArray& Ydata() {return *m_Y_data;}

        //! Returns the U component
        PicArray& Udata() {return *m_U_data;}

        //! Returns the V component 
        PicArray& Vdata() {return *m_V_data;}

        //! Returns the luma data array
        const PicArray& Ydata() const {return *m_Y_data;}

        //! Returns the U component
        const PicArray& Udata() const {return *m_U_data;}

        //! Returns the V component 
        const PicArray& Vdata() const {return *m_V_data;}

        //! Returns a given component 
        PicArray& Data(CompSort cs);

        //! Returns a given component
        const PicArray& Data(CompSort cs) const;    

        //! Returns upconverted Y data
        PicArray& UpYdata();

        //! Returns upconverted U data
        PicArray& UpUdata();

        //! Returns upconverted V data
        PicArray& UpVdata();

        //! Returns a given upconverted component
        PicArray& UpData(CompSort cs);

        //! Returns upconverted Y data
        const PicArray& UpYdata() const;

        //! Returns upconverted U data
        const PicArray& UpUdata() const;

        //! Returns upconverted V data    
        const PicArray& UpVdata() const;

        //! Returns a given upconverted component
        const PicArray& UpData(CompSort cs) const;

        //! Clip the data to prevent overshoot
        /*!
            Clips the data to lie between 0 and 1020 (4*255) in 10-bit form to prevent overshoot/wraparound.
         */
        void Clip();

    private:
        FrameParams m_fparams;
        PicArray* m_Y_data;//the 
        PicArray* m_U_data;//component
        PicArray* m_V_data;//data
        mutable PicArray* m_upY_data;//upconverted data. Mutable because we
        mutable PicArray* m_upU_data;//create them on the fly even in const
        mutable PicArray* m_upV_data;//functions.

        //! Initialises the frame once the frame parameters have been set
        void Init();

        //! Delete all the data
        void ClearData();

        //! Clip an individual component
        void ClipComponent(PicArray& pic_data);    
    };

} // namespace dirac


#endif
