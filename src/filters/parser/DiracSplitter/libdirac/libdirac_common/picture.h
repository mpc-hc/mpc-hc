/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: picture.h,v 1.4 2008/08/14 00:51:08 asuraparaju Exp $ $Name:  $
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

#ifndef _PICTURE_H_
#define _PICTURE_H_

#include <libdirac_common/common.h>
#include <libdirac_common/wavelet_utils.h>

namespace dirac
{
    //! A class for encapsulating all the data relating to a picture.
    /*!
        A class for encapsulating all the data relating to a picture - all the 
        component data, including upconverted data.
     */
    class Picture
    {

    public:

        //! Constructor
        /*!
            Constructor initialises the picture parameters and the data
         */    
        Picture( const PictureParams& pp );

        //! Copy constructor. Private as not currently used [may want to implement reference counting later.]
        Picture(const Picture& cpy);

        //! Destructor
        virtual ~Picture();

        //! Assignment =. Private as not currently used [may want to implement reference counting later.]
        Picture& operator=( const Picture& rhs );

        //! Picture Fill
        /*!
            Initialise contents of picture with value provided
        */
        void Fill(ValueType val );

        //gets and sets
        //! Gets the picture parameters
        PictureParams& GetPparams() const  {return m_pparams;}

        //! Sets the picture sort
        void SetPictureSort( const PictureSort ps ){m_pparams.SetPicSort( ps ); }

        //! Sets the picture type
        void SetPictureType( const PictureType ftype ){m_pparams.SetPictureType( ftype ); }

        //! Sets the picture type
        void SetReferenceType( const ReferenceType rtype ){m_pparams.SetReferenceType( rtype ); }

        //! Reconfigures to the new parameters. 
        void ReconfigPicture( const PictureParams &pp );

        //! Returns a given component 
        PicArray& Data(CompSort cs){return *m_pic_data[(int) cs];}

        //! Returns a given component
        const PicArray& Data(CompSort cs) const{return *m_pic_data[(int) cs];}    

        //! Returns a given upconverted component
        PicArray& UpData(CompSort cs);

        //! Returns a given upconverted component
        const PicArray& UpData(CompSort cs) const;

        //! Returns the wavelet coefficient data
        const CoeffArray& WltData( CompSort c ) const { return m_wlt_data[(int) c]; }

        //! Returns the wavelet coefficient data
        CoeffArray& WltData( CompSort c ) { return m_wlt_data[(int) c]; }

        //! Initialises the wavelet coefficient data arrays;
        void InitWltData( const int transform_depth );

        //! Clip the data to prevent overshoot
        /*!
            Clips the data to lie between 0 and (1<<video_depth)-1 
         */
        void Clip();

        //! Clip the upconverted data to prevent overshoot
        /*!
            Clips the upconverted data to lie between 0 and (1<<video_depth)-1 
         */
        void ClipUpData();

    protected:
        mutable PictureParams m_pparams;
        PicArray* m_pic_data[3];//the picture data 
        mutable PicArray* m_up_pic_data[3];//upconverted data. Mutable because we
                                         //create them on the fly even in const
                                         //functions.

        CoeffArray m_wlt_data[3];// the wavelet coefficient data

        //! Initialises the picture once the picture parameters have been set
        virtual void Init();

        //! Delete all the data
        virtual void ClearData();

        //! Clip an individual component
        void ClipComponent(PicArray& pic_data, CompSort cs) const;

    };

} // namespace dirac


#endif
