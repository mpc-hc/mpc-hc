/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: picture.cpp,v 1.4 2008/08/14 00:51:08 asuraparaju Exp $ $Name:  $
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

//Implementation of picture classes in picture.h

#include <libdirac_common/picture.h>
#include <libdirac_common/upconvert.h>
using namespace dirac;

#include <iostream>
#if defined(HAVE_MMX)
#include <mmintrin.h>
#endif

///////////////
//---Picture---//
///////////////

Picture::Picture(const PictureParams& pp):
    m_pparams(pp)
{
    for(int c = 0; c < 3; ++c)
    {
        m_pic_data[c] = NULL;
        m_up_pic_data[c] = NULL;
    }

    Init();
}

Picture::Picture(const Picture& cpy):
    m_pparams(cpy.m_pparams)
{

    //delete data to be overwritten
    for(int c = 0; c < 3; ++c)
    {
        m_pic_data[c] = NULL;
        m_up_pic_data[c] = NULL;
    }

    //now copy the data across
    for(int c = 0; c < 3; ++c)
    {
        m_pic_data[c] = new PicArray(*(cpy.m_pic_data[c]));
        if(cpy.m_up_pic_data[c] != NULL)
            m_up_pic_data[c] = new PicArray(*(cpy.m_up_pic_data[c]));
    }

}


Picture::~Picture()
{
    ClearData();
}

Picture& Picture::operator=(const Picture& rhs)
{
    if(&rhs != this)
    {
        m_pparams = rhs.m_pparams;

        // Delete current data
        ClearData();

        // Copy the data across
        for(int c = 0; c < 3; ++c)
        {
            m_pic_data[c] = new PicArray(*(rhs.m_pic_data[c]));

            if(rhs.m_up_pic_data[c] != NULL)
                m_up_pic_data[c] = new PicArray(*(rhs.m_up_pic_data[c]));
        }
    }

    return *this;

}

void Picture::Fill(ValueType val)
{
    for(int c = 0; c < 3; ++c)
    {
        m_pic_data[c]->Fill(val);
        if(m_up_pic_data[c] != NULL)
            delete m_up_pic_data[c];
    }
}

//Other functions

void Picture::Init()
{
    //const ChromaFormat cformat=m_pparams.CFormat();

    //first delete data if we need to
    ClearData();

    m_pic_data[0] = new PicArray(m_pparams.Yl() , m_pparams.Xl());
    m_pic_data[0]->SetCSort(Y_COMP);

    m_pic_data[1] = new PicArray(m_pparams.ChromaYl() ,
                                 m_pparams.ChromaXl());
    m_pic_data[1]->SetCSort(U_COMP);

    m_pic_data[2] = new PicArray(m_pparams.ChromaYl() ,
                                 m_pparams.ChromaXl());
    m_pic_data[2]->SetCSort(V_COMP);
}

PicArray& Picture::UpData(CompSort cs)
{
    const int c = (int) cs;

    if(m_up_pic_data[c] != NULL)
        return *(m_up_pic_data[c]);
    else
    {
        //we have to do the upconversion

        m_up_pic_data[c] = new PicArray(2 * m_pic_data[c]->LengthY(),
                                        2 * m_pic_data[c]->LengthX());
        UpConverter* myupconv;
        if(c > 0)
            myupconv = new UpConverter(-(1 << (m_pparams.ChromaDepth() - 1)),
                                       (1 << (m_pparams.ChromaDepth() - 1)) - 1,
                                       m_pparams.ChromaXl(), m_pparams.ChromaYl());
        else
            myupconv = new UpConverter(-(1 << (m_pparams.LumaDepth() - 1)),
                                       (1 << (m_pparams.LumaDepth() - 1)) - 1,
                                       m_pparams.Xl(), m_pparams.Yl());

        myupconv->DoUpConverter(*(m_pic_data[c]) , *(m_up_pic_data[c]));

        delete myupconv;

        return *(m_up_pic_data[c]);

    }
}

const PicArray& Picture::UpData(CompSort cs) const
{
    const int c = (int) cs;

    if(m_up_pic_data[c] != NULL)
        return *(m_up_pic_data[c]);
    else
    {
        //we have to do the upconversion

        m_up_pic_data[c] = new PicArray(2 * m_pic_data[c]->LengthY(),
                                        2 * m_pic_data[c]->LengthX());
        UpConverter* myupconv;
        if(c > 0)
            myupconv = new UpConverter(-(1 << (m_pparams.ChromaDepth() - 1)),
                                       (1 << (m_pparams.ChromaDepth() - 1)) - 1,
                                       m_pparams.ChromaXl(), m_pparams.ChromaYl());
        else
            myupconv = new UpConverter(-(1 << (m_pparams.LumaDepth() - 1)),
                                       (1 << (m_pparams.LumaDepth() - 1)) - 1,
                                       m_pparams.Xl(), m_pparams.Yl());

        myupconv->DoUpConverter(*(m_pic_data[c]) , *(m_up_pic_data[c]));

        delete myupconv;

        return *(m_up_pic_data[c]);

    }
}

void Picture::InitWltData(const int transform_depth)
{

    int xpad_len, ypad_len;
    int tx_mul = 1 << transform_depth;

    for(int c = 0; c < 3; ++c)
    {
        xpad_len = m_pic_data[c]->LengthX();
        ypad_len = m_pic_data[c]->LengthY();

        if(xpad_len % tx_mul != 0)
            xpad_len = ((xpad_len / tx_mul) + 1) * tx_mul;
        if(ypad_len % tx_mul != 0)
            ypad_len = ((ypad_len / tx_mul) + 1) * tx_mul;

        m_wlt_data[c].Resize(ypad_len, xpad_len);
    }

}

void Picture::ClipComponent(PicArray& pic_data, CompSort cs) const
{
    ValueType *pic = &(pic_data[pic_data.FirstY()][pic_data.FirstX()]);
    int count = pic_data.LengthY() * pic_data.LengthX();

    ValueType min_val;
    ValueType max_val;

    min_val = (cs == Y_COMP) ?
              -(1 << (m_pparams.LumaDepth() - 1)) :
              -(1 << (m_pparams.ChromaDepth() - 1));

    max_val = (cs == Y_COMP) ?
              (1 << (m_pparams.LumaDepth() - 1)) - 1 :
              (1 << (m_pparams.ChromaDepth() - 1)) - 1;

#if defined (HAVE_MMX)
    {
        int qcount = count >> 2;
        count = count & 3;

        //__m64 pack_usmax = _mm_set_pi16 (0xffff, 0xffff, 0xffff, 0xffff);
        //__m64 pack_smin = _mm_set_pi16 (0x8000, 0x8000, 0x8000, 0x8000);
        __m64 pack_usmax = _mm_set_pi16(-1, -1, -1, -1);
        __m64 pack_smin = _mm_set_pi16(-32768, -32768, -32768, -32768);
        __m64 high_val = _mm_set_pi16(max_val, max_val, max_val, max_val);
        __m64 lo_val = _mm_set_pi16(min_val, min_val, min_val, min_val);

        __m64 clip_max = _mm_add_pi16(pack_smin, high_val);
        __m64 clip_min = _mm_add_pi16(pack_smin, lo_val);

        __m64 tmp1 =  _mm_subs_pu16(pack_usmax, clip_max);
        __m64 tmp2 =  _mm_adds_pu16(clip_min, tmp1);

        while(qcount--)
        {
            ValueType *p1 = pic;
            *(__m64 *)p1 = _mm_add_pi16(pack_smin, *(__m64 *)p1);
            *(__m64 *)p1 = _mm_adds_pu16(*(__m64 *)p1,  tmp1);
            *(__m64 *)p1 = _mm_subs_pu16(*(__m64 *)p1,  tmp2);
            *(__m64 *)p1 = _mm_add_pi16(lo_val, *(__m64 *)p1);
            pic += 4;
        }
        //Mop up remaining pixels
        while(count--)
        {
            *pic = std::max(min_val, std::min(max_val , *pic)
                           );
            pic++;
        }

        _mm_empty();
        return;
    }
#endif

    // NOTE: depending on a contigous chunk of memory being allocated
    while(count--)
    {
        *pic = std::max(min_val, std::min(max_val, *pic));
        pic++;
    }
}

void Picture::Clip()
{
    //just clips the straight picture data, not the upconverted data

    for(int c = 0; c < 3; ++c)
        ClipComponent(*(m_pic_data[c]), (CompSort) c);
}

void Picture::ClipUpData()
{
    //just clips the upconverted data

    for(int c = 0; c < 3; ++c)
    {
        if(m_up_pic_data[c])
            ClipComponent(*(m_up_pic_data[c]), (CompSort) c);
    }

}

void Picture::ClearData()
{
    for(int c = 0; c < 3; ++c)
    {
        if(m_pic_data[c] != NULL)
        {
            delete m_pic_data[c];
            m_pic_data[c] = NULL;
        }

        if(m_up_pic_data[c] != NULL)
        {
            delete m_up_pic_data[c];
            m_up_pic_data[c] = NULL;
        }
    }

}

void Picture::ReconfigPicture(const PictureParams &pp)
{

    PictureParams old_pp = m_pparams;
    m_pparams = pp;

    // HAve picture dimensions  or Chroma format changed ?
    if(m_pparams.Xl() == old_pp.Xl() &&
       m_pparams.Yl() == old_pp.Yl() &&
       m_pparams.CFormat() == old_pp.CFormat())
        return;

    // Picture dimensions have changed. Re-initialise
    Init();
}



