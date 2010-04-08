/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: picture_buffer.cpp,v 1.7 2008/06/19 10:07:03 tjdwave Exp $ $Name:  $
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
*                 Scott R Ladd,
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

#include <libdirac_common/picture_buffer.h>
#include <algorithm>
using namespace dirac;

//Simple constructor for decoder operation
PictureBuffer::PictureBuffer() {}

//Copy constructor. Why anyone would need this I don't know.
PictureBuffer::PictureBuffer(const PictureBuffer& cpy)
{
    // first delete all frames in the current buffer
    for(size_t i = 0 ; i < m_pic_data.size() ; ++i)
    {
        delete m_pic_data[i];
    }//i

    // next create new arrays, copying from the initialising buffer
    m_pic_data.resize(cpy.m_pic_data.size());
    for(size_t i = 0 ; i < m_pic_data.size() ; ++i)
    {
        m_pic_data[i] = new Picture(*(cpy.m_pic_data[i]));
    }//i

    // now copy the map
    m_pnum_map = cpy.m_pnum_map;

//    // and the reference count
//    m_ref_count = cpy.m_ref_count;

}

//Assignment=. Not sure why this would be used either.
PictureBuffer& PictureBuffer::operator=(const PictureBuffer& rhs)
{
    if(&rhs != this)
    {
        // delete all the frames in the lhs buffer
        for(size_t i = 0 ; i < m_pic_data.size() ; ++i)
        {
            delete m_pic_data[i];
        }//i

        // next create new arrays, copying from the rhs
        m_pic_data.resize(rhs.m_pic_data.size());
        for(size_t i = 0 ; i < m_pic_data.size() ; ++i)
        {
            m_pic_data[i] = new Picture(*(rhs.m_pic_data[i]));
        }//i

        // now copy the map
        m_pnum_map = rhs.m_pnum_map;

//        // and the reference count
//        m_ref_count = rhs.m_ref_count;

    }
    return *this;
}

//Destructor
PictureBuffer::~PictureBuffer()
{
    for(size_t i = 0 ; i < m_pic_data.size() ; ++i)
        delete m_pic_data[i];
}

Picture& PictureBuffer::GetPicture(const unsigned int pnum)
{
    //get picture with a given picture number, NOT with a given position in the buffer.
//If the picture number does not occur, the first picture in the buffer is returned.

    std::map<unsigned int, unsigned int>::iterator it = m_pnum_map.find(pnum);

    unsigned int pos = 0;
    if(it != m_pnum_map.end())
        pos = it->second;

    return *(m_pic_data[pos]);
}

const Picture& PictureBuffer::GetPicture(const unsigned int pnum) const
{
    //as above, but const version

    std::map<unsigned int, unsigned int>::const_iterator it = m_pnum_map.find(pnum);

    unsigned int pos = 0;
    if(it != m_pnum_map.end())
        pos = it->second;

    return *(m_pic_data[pos]);
}

Picture& PictureBuffer::GetPicture(const unsigned int pnum, bool& is_present)
{
    //get picture with a given picture number, NOT with a given position in the buffer.
//If the picture number does not occur, the first picture in the buffer is returned.

    std::map<unsigned int, unsigned int>::iterator it = m_pnum_map.find(pnum);

    unsigned int pos = 0;
    if(it != m_pnum_map.end())
    {
        is_present = true;
        pos = it->second;
    }
    else
        is_present = false;

    return *(m_pic_data[pos]);
}

const Picture& PictureBuffer::GetPicture(const unsigned int pnum, bool& is_present) const
{
    //as above, but const version

    std::map<unsigned int, unsigned int>::const_iterator it = m_pnum_map.find(pnum);

    unsigned int pos = 0;
    if(it != m_pnum_map.end())
    {
        is_present = true;
        pos = it->second;
    }
    else
        is_present = false;

    return *(m_pic_data[pos]);
}

bool PictureBuffer::IsPictureAvail(const unsigned int pnum) const
{

    std::map<unsigned int, unsigned int>::const_iterator it = m_pnum_map.find(pnum);

    if(it != m_pnum_map.end())
        return true;
    else
        return false;
}

std::vector<int> PictureBuffer::Members() const
{
    std::vector<int> members(0);
    for(unsigned int i = 0; i < m_pic_data.size(); ++i)
    {
        const PictureParams& pparams = m_pic_data[i]->GetPparams();
        members.push_back(pparams.PictureNum());
    }// i

    return members;
}

void PictureBuffer::PushPicture(const PictureParams& pp)
{
    // Put a new picture onto the top of the stack

    // if picture is present - return
    if(IsPictureAvail(pp.PictureNum()))
        return;

//    if ( pp.PicSort().IsRef() )
//        m_ref_count++;

    Picture* pptr = new Picture(pp);
    // add the picture to the buffer
    m_pic_data.push_back(pptr);

    // put the picture number into the index table
    std::pair<unsigned int, unsigned int> temp_pair(pp.PictureNum() , m_pic_data.size() - 1);
    m_pnum_map.insert(temp_pair);
}

void PictureBuffer::CopyPicture(const Picture& picture)
{
    PushPicture(picture.GetPparams());

    bool is_present;

    Picture & p = GetPicture(picture.GetPparams().PictureNum(), is_present);
    if(is_present)
        p = picture;
}

void PictureBuffer::ClearSlot(const unsigned int pos)
{
    // Clear a slot corresponding to position pos to take more data

    std::pair<unsigned int, unsigned int>* tmp_pair;

    if(pos < m_pic_data.size())
    {
        delete m_pic_data[pos];
        m_pic_data.erase(m_pic_data.begin() + pos);

        //make a new map
        m_pnum_map.clear();
        for(size_t i = 0 ; i < m_pic_data.size() ; ++i)
        {
            tmp_pair = new std::pair<unsigned int, unsigned int>(m_pic_data[i]->GetPparams().PictureNum() , i);
            m_pnum_map.insert(*tmp_pair);
            delete tmp_pair;
        }//i
    }
}


void PictureBuffer::SetRetiredPictureNum(const int show_pnum, const int current_coded_pnum)
{
    if(IsPictureAvail(current_coded_pnum))
    {
        PictureParams &pparams = GetPicture(current_coded_pnum).GetPparams();
        pparams.SetRetiredPictureNum(-1);
        for(size_t i = 0 ; i < m_pic_data.size() ; ++i)
        {
            if(m_pic_data[i]->GetPparams().PicSort().IsRef())
            {
                if((m_pic_data[i]->GetPparams().PictureNum() + m_pic_data[i]->GetPparams().ExpiryTime()) <= show_pnum)
                {
                    pparams.SetRetiredPictureNum(m_pic_data[i]->GetPparams().PictureNum());
                    break;
                }
            }
        }//i
    }
}

void PictureBuffer::Remove(const int pnum)
{
    for(size_t i = 0 ; i < m_pic_data.size() ; ++i)
    {
        if(m_pic_data[i]->GetPparams().PictureNum() == pnum)
            ClearSlot(i);
    }//i
}

