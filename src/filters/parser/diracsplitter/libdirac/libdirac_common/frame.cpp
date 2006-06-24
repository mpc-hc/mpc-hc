/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: frame.cpp 280 2005-01-30 05:11:46Z gabest $ $Name$
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

//Implementation of frame classes in frame.h

#include <libdirac_common/frame.h>
#include <libdirac_common/upconvert.h>
using namespace dirac;

#include <iostream>

///////////////
//---Frame---//
///////////////

Frame::Frame(const FrameParams& fp): 
    m_fparams(fp),
    m_Y_data(0),
    m_U_data(0),
    m_V_data(0),
    m_upY_data(0),
    m_upU_data(0),
    m_upV_data(0)
{
    Init();
}

Frame::Frame( const Frame& cpy ): 
    m_fparams(cpy.m_fparams),
    m_Y_data(0),
    m_U_data(0),
    m_V_data(0),
    m_upY_data(0),
    m_upU_data(0),
    m_upV_data(0)
{
    const ChromaFormat& cformat = m_fparams.CFormat();

    //delete data to be overwritten
    ClearData();

    //now copy the data accross
    m_Y_data = new PicArray( *(cpy.m_Y_data) );
    if (cpy.m_upY_data != 0){
        m_upY_data = new PicArray( *(cpy.m_upY_data) );
    }

    if (cformat != Yonly)
    {
        m_U_data = new PicArray( *(cpy.m_U_data) );
        m_V_data = new PicArray( *(cpy.m_V_data) );

        if ( cpy.m_upU_data != 0 )
        {
            m_upU_data = new PicArray( *(cpy.m_upU_data) );
        }
        if ( cpy.m_upV_data != 0 )
        {
            m_upV_data = new PicArray( *(cpy.m_upV_data) );
        }

    }

}


Frame::~Frame()
{
    ClearData();    
}

Frame& Frame::operator=(const Frame& rhs)
{
    if ( &rhs != this)
    {
        m_fparams=rhs.m_fparams;
        const ChromaFormat& cformat=m_fparams.CFormat();

        // Delete current data
        ClearData();

        // Copy the data across        
        m_Y_data= new PicArray( *(rhs.m_Y_data) );

        if ( rhs.m_upY_data != 0)
            m_upY_data = new PicArray( *(rhs.m_upY_data) );

        if (cformat != Yonly)
        {
            m_U_data= new PicArray( *(rhs.m_U_data) );            
            if (rhs.m_upU_data!=0)
                m_upU_data = new PicArray(*(rhs.m_upU_data) );

            m_V_data= new PicArray( *(rhs.m_V_data) );            
            if (rhs.m_upV_data!=0)
                m_upV_data= new PicArray( *(rhs.m_upV_data) );
        }
    }

    return *this;

}

//Other functions

void Frame::Init()
{
    const ChromaFormat cformat=m_fparams.CFormat();

    //first delete data if we need to
    ClearData();

     m_Y_data=new PicArray( m_fparams.Yl() , m_fparams.Xl());
     m_Y_data->SetCSort( Y_COMP );

     if(cformat == format422) 
     {
         m_U_data = new PicArray( m_fparams.Yl() , m_fparams.Xl()/2 ); 
         m_U_data->SetCSort( U_COMP );

         m_V_data = new PicArray( m_fparams.Yl() , m_fparams.Xl()/2 );
         m_V_data->SetCSort( V_COMP );
     }
     else if (cformat == format420)
     {
         m_U_data = new PicArray( m_fparams.Yl()/2 , m_fparams.Xl()/2 );
         m_U_data->SetCSort( U_COMP );

         m_V_data = new PicArray( m_fparams.Yl()/2 , m_fparams.Xl()/2 ); 
         m_V_data->SetCSort(V_COMP);
     }
     else if (cformat == format411)
     {
         m_U_data = new PicArray( m_fparams.Yl() , m_fparams.Xl()/4 );
         m_U_data->SetCSort( U_COMP );

         m_V_data = new PicArray( m_fparams.Yl() , m_fparams.Xl()/4 );
         m_V_data->SetCSort( V_COMP );
     }
     else if (cformat==format444){
         m_U_data = new PicArray( m_fparams.Yl() , m_fparams.Xl() ); 
         m_U_data->SetCSort( U_COMP );

         m_V_data = new PicArray( m_fparams.Yl() , m_fparams.Xl() );
         m_V_data->SetCSort( V_COMP );
    }
    //(other formats all assumed to be Yonly
}

PicArray& Frame::Data(CompSort cs)
{//another way to access the data

    if (cs == U_COMP) return *m_U_data; 
    else if (cs == V_COMP) return *m_V_data; 
    else return *m_Y_data;
}    

const PicArray& Frame::Data(CompSort cs) const
{//another way to access the data

    if (cs == U_COMP) return *m_U_data; 
    else if (cs == V_COMP) return *m_V_data; 
    else return *m_Y_data;
}

PicArray& Frame::UpYdata()
{
    if (m_upY_data != 0)
        return *m_upY_data;
    else
    {//we have to do the upconversion

        m_upY_data = new PicArray( 2*m_Y_data->LengthY() , 2*m_Y_data->LengthX() );
        UpConverter myupconv;
        myupconv.DoUpConverter( *m_Y_data , *m_upY_data );

        return *m_upY_data;

    }
}

PicArray& Frame::UpUdata()
{
    if (m_upU_data != 0)
        return *m_upU_data;
    else
    {//we have to do the upconversion

        m_upU_data = new PicArray(2*m_U_data->LengthY() , 2*m_U_data->LengthX());
        UpConverter myupconv;
        myupconv.DoUpConverter( *m_U_data , *m_upU_data );

        return *m_upU_data;

    }
}

PicArray& Frame::UpVdata()
{
    if (m_upV_data != 0)
        return *m_upV_data;
    else
    {//we have to do the upconversion
    
        m_upV_data = new PicArray( 2*m_V_data->LengthY() , 2*m_V_data->LengthX() );
        UpConverter myupconv;
        myupconv.DoUpConverter( *m_V_data , *m_upV_data );

        return *m_upV_data;

    }
}

PicArray& Frame::UpData(CompSort cs)
{
    if (cs == U_COMP)
        return UpUdata(); 
    else if (cs == V_COMP) 
        return UpVdata(); 
    else 
        return UpYdata();
}    

const PicArray& Frame::UpYdata() const
{
    if (m_upY_data != 0)
        return *m_upY_data;
    else
    {
        //We have to do the upconversion
        //Although we're changing a value - the pointer to the array - it doesn't affect the state of
        //the object as viewable from outside. So the pointers to the upconveted data have been 
        //declared mutable.

        m_upY_data = new PicArray( 2*m_Y_data->LengthY() , 2*m_Y_data->LengthX() );

        UpConverter myupconv;
        myupconv.DoUpConverter( *m_Y_data , *m_upY_data );

        return *m_upY_data;

    }
}

const PicArray& Frame::UpUdata() const
{
    if (m_upU_data != 0)
        return *m_upU_data;
    else
    {
        //We have to do the upconversion
        //Although we're changing a value - the pointer to the array - it doesn't affect the state of
        //the object as viewable from outside. So the pointers to the upconveted data have been 
        //declared mutable.

        m_upU_data = new PicArray( 2*m_U_data->LengthY() , 2*m_U_data->LengthX() );

        UpConverter myupconv;
        myupconv.DoUpConverter( *m_U_data , *m_upU_data );

        return *m_upU_data;

    }
}

const PicArray& Frame::UpVdata() const
{
    if (m_upV_data != 0)
        return *m_upV_data;
    else
    {
        //We have to do the upconversion
        //Although we're changing a value - the pointer to the array - it doesn't affect the state of
        //the object as viewable from outside. So the pointers to the upconveted data have been 
        //declared mutable.
 
        m_upV_data = new PicArray( 2*m_V_data->LengthY() , 2*m_V_data->LengthX() );

        UpConverter myupconv;
        myupconv.DoUpConverter( *m_V_data , *m_upV_data );

        return *m_upV_data;

    }
}

const PicArray& Frame::UpData(CompSort cs) const
{
    if (cs == U_COMP) 
        return UpUdata(); 
    else if (cs == V_COMP)
        return UpVdata(); 
    else 
        return UpYdata();
}    

void Frame::ClipComponent(PicArray& pic_data)
{
    for (int j=pic_data.FirstY() ; j<=pic_data.LastY() ; ++j)
    {
        for (int i=pic_data.FirstX() ; i<=pic_data.LastX() ; ++i)
        {
            pic_data[j][i] = std::min( pic_data[j][i] , ValueType( 1020 ) );
            pic_data[j][i] = std::max( pic_data[j][i] , ValueType( 0 ) );
        }// i        
    }// j
}

void Frame::Clip()
{
    //just clips the straight picture data, not the upconverted data

    ClipComponent( *m_Y_data );

    if (m_fparams.CFormat() != Yonly)
    {
        ClipComponent( *m_U_data );
        ClipComponent( *m_V_data );    
    }    
}

void Frame::ClearData()
{
    if (m_Y_data != 0)
    {
        delete m_Y_data;
        m_Y_data = 0;
    }

    if (m_U_data!=0)
    {
        delete m_U_data;
        m_U_data = 0;
    }

    if (m_V_data!=0)
    {
        delete m_V_data;
        m_V_data = 0;
    }

    if (m_upY_data != 0)
    {
        delete m_upY_data;
        m_upY_data = 0;
    }

    if (m_upU_data!=0)
    {
        delete m_upU_data;
        m_upU_data = 0;
    }

    if (m_upV_data != 0)
    {
        delete m_upV_data;
        m_upV_data = 0;
    }
}
