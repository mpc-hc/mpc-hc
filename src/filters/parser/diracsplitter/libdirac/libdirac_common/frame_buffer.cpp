/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: frame_buffer.cpp,v 1.22 2007/09/26 12:18:43 asuraparaju Exp $ $Name: Dirac_0_8_0 $
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

#include <libdirac_common/frame_buffer.h>
#include <algorithm>
using namespace dirac;

//Simple constructor for decoder operation
FrameBuffer::FrameBuffer() :
    m_ref_count(0),
    m_num_L1(0),
    m_L1_sep(1),
    m_gop_len(0),
    m_interlace(false)
{}

//Simple constructor for general operation
FrameBuffer::FrameBuffer(ChromaFormat cf,
                         const int orig_xlen,
                         const int orig_ylen,
                         const int dwt_xlen,
                         const int dwt_ylen,
                         const int dwt_cxlen,
                         const int dwt_cylen,
                         unsigned int luma_depth,
                         unsigned int chroma_depth) :
    m_ref_count(0),
    m_fparams(cf, orig_xlen, orig_ylen, dwt_xlen, dwt_ylen, dwt_cxlen, dwt_cylen, luma_depth, chroma_depth),
    m_num_L1(0),
    m_L1_sep(1),
    m_gop_len(0),
    m_interlace(false)
{}

//Constructor setting GOP parameters for use with a standard GOP
FrameBuffer::FrameBuffer(ChromaFormat cf,
                         const int numL1,
                         const int L1sep,
                         const int orig_xlen,
                         const int orig_ylen,
                         const int dwt_xlen,
                         const int dwt_ylen,
                         const int dwt_cxlen,
                         const int dwt_cylen,
                         unsigned int luma_depth,
                         unsigned int chroma_depth,
                         bool interlace) :
    m_ref_count(0),
    m_fparams(cf,orig_xlen, orig_ylen, dwt_xlen, dwt_ylen, dwt_cxlen, dwt_cylen, luma_depth, chroma_depth),
    m_num_L1(numL1),
    m_L1_sep(L1sep),
    m_interlace(interlace)
{
    if (m_num_L1>0)
    {// conventional GOP coding
        m_gop_len = (m_num_L1+1)*m_L1_sep;
    }
    else if (m_num_L1==0)
    {// I-frame only coding
        m_gop_len = 1;
        m_L1_sep = 0;
    }
    else
    {// don't have a proper GOP, only an initial I-frame
        m_gop_len = 0;
    }
}

//Copy constructor. Why anyone would need this I don't know.
FrameBuffer::FrameBuffer(const FrameBuffer& cpy)
    {
    // first delete all frames in the current buffer
    for (size_t i=0 ; i<m_frame_data.size() ; ++i)
    {
        delete m_frame_data[i];
    }//i

    // next create new arrays, copying from the initialising buffer
    m_frame_data.resize(cpy.m_frame_data.size());
    m_frame_in_use.resize(cpy.m_frame_in_use.size());
    for (size_t i=0 ; i<m_frame_data.size() ; ++i){
        m_frame_data[i] = new Frame( *(cpy.m_frame_data[i]) );
        m_frame_in_use[i] = cpy.m_frame_in_use[i];
    }//i

    // now copy the map
    m_fnum_map = cpy.m_fnum_map;

    // and the internal frame parameters
    m_fparams = cpy.m_fparams;

    // and the reference count
    m_ref_count = cpy.m_ref_count;

    // and the gop structure
    m_num_L1 = cpy.m_num_L1;
    m_L1_sep = cpy.m_L1_sep;
    m_gop_len = cpy.m_gop_len;
    m_interlace = cpy.m_interlace;
}

//Assignment=. Not sure why this would be used either.
FrameBuffer& FrameBuffer::operator=(const FrameBuffer& rhs){
    if (&rhs!=this)
    {
        // delete all the frames in the lhs buffer
        for (size_t i=0 ; i<m_frame_data.size() ; ++i)
        {
            delete m_frame_data[i];
        }//i

        // next create new arrays, copying from the rhs
        m_frame_data.resize(rhs.m_frame_data.size());
        m_frame_in_use.resize(rhs.m_frame_in_use.size());
        for (size_t i=0 ; i<m_frame_data.size() ; ++i)
        {
            m_frame_data[i] = new Frame( *(rhs.m_frame_data[i]) );
            m_frame_in_use[i] = rhs.m_frame_in_use[i];
        }//i

        // now copy the map
        m_fnum_map = rhs.m_fnum_map;

        // and the internal frame parameters
        m_fparams = rhs.m_fparams;

        // and the reference count
        m_ref_count = rhs.m_ref_count;

        // and the interlace flag
        m_interlace = rhs.m_interlace;
    }
    return *this;
}

//Destructor
FrameBuffer::~FrameBuffer()
{
    for (size_t i=0 ; i<m_frame_data.size() ;++i)
        delete m_frame_data[i];
}

Frame& FrameBuffer::GetFrame( const unsigned int fnum )
{//get frame with a given frame number, NOT with a given position in the buffer.
 //If the frame number does not occur, the first frame in the buffer is returned.

    std::map<unsigned int,unsigned int>::iterator it = m_fnum_map.find(fnum);

    unsigned int pos = 0;
    if (it != m_fnum_map.end())
        pos = it->second;

    return *(m_frame_data[pos]);
}

const Frame& FrameBuffer::GetFrame( const unsigned int fnum ) const
{    //as above, but const version

    std::map<unsigned int,unsigned int>::const_iterator it = m_fnum_map.find(fnum);

    unsigned int pos=0;
    if (it != m_fnum_map.end())
        pos = it->second;

    return *(m_frame_data[pos]);
}

Frame& FrameBuffer::GetFrame( const unsigned int fnum, bool& is_present )
{//get frame with a given frame number, NOT with a given position in the buffer.
 //If the frame number does not occur, the first frame in the buffer is returned.

    std::map<unsigned int,unsigned int>::iterator it = m_fnum_map.find(fnum);

    unsigned int pos = 0;
    if (it != m_fnum_map.end())
    {
        is_present = true;
        pos = it->second;
    }
    else
        is_present=false;

    return *(m_frame_data[pos]);
}

const Frame& FrameBuffer::GetFrame( const unsigned int fnum, bool& is_present ) const
{    //as above, but const version

    std::map<unsigned int,unsigned int>::const_iterator it = m_fnum_map.find(fnum);

    unsigned int pos=0;
    if (it != m_fnum_map.end())
    {
        is_present = true;
        pos = it->second;
    }
    else
        is_present=false;

    return *(m_frame_data[pos]);
}

bool FrameBuffer::IsFrameAvail( const unsigned int fnum ) const
{

    std::map<unsigned int,unsigned int>::const_iterator it = m_fnum_map.find(fnum);

    if (it != m_fnum_map.end())
        return true;
    else
        return false;
}

PicArray& FrameBuffer::GetComponent( const unsigned int fnum , CompSort c)
{//as GetFrame, but returns corresponding component

    std::map<unsigned int,unsigned int>::iterator it = m_fnum_map.find(fnum);

    unsigned int pos = 0;
    if (it!=m_fnum_map.end())
        pos = it->second;

    if (c == U_COMP)
        return m_frame_data[pos]->Udata();
    else if (c == V_COMP)
        return m_frame_data[pos]->Vdata();
    else
        return m_frame_data[pos]->Ydata();
}

const PicArray& FrameBuffer::GetComponent( const unsigned int fnum , CompSort c ) const
{//as above, but const version

    std::map<unsigned int,unsigned int>::const_iterator it = m_fnum_map.find(fnum);

    // FIXME - how do we handle the condition when a frame matching fnum is
    // not found???
    unsigned int pos = 0;

    if (it!=m_fnum_map.end())
        pos = it->second;

    if (c==U_COMP)
        return m_frame_data[pos]->Udata();
    else if (c==V_COMP)
        return m_frame_data[pos]->Vdata();
    else
        return m_frame_data[pos]->Ydata();
}

// as GetFrame, but returns corresponding upconverted component
PicArray& FrameBuffer::GetUpComponent(const unsigned int fnum, CompSort c){
    std::map<unsigned int,unsigned int>::iterator it = m_fnum_map.find(fnum);

    unsigned int pos = 0;
    if (it!=m_fnum_map.end())
        pos = it->second;

    if (c == U_COMP)
        return m_frame_data[pos]->UpUdata();
    else if (c == V_COMP)
        return m_frame_data[pos]->UpVdata();
    else
        return m_frame_data[pos]->UpYdata();

}

const PicArray& FrameBuffer::GetUpComponent(const unsigned int fnum, CompSort c) const {//as above, but const version
    std::map<unsigned int,unsigned int>::const_iterator it=m_fnum_map.find(fnum);

    unsigned int pos = 0;
    if (it!=m_fnum_map.end())
        pos = it->second;

    if (c == U_COMP)
        return m_frame_data[pos]->UpUdata();
    else if (c == V_COMP)
        return m_frame_data[pos]->UpVdata();
    else
        return m_frame_data[pos]->UpYdata();

}

std::vector<int> FrameBuffer::Members() const
{
    std::vector<int> members( 0 );
    for (unsigned int i=0; i<m_frame_data.size(); ++i )
    {
        if ( m_frame_in_use[i] == true )
        {
            const FrameParams& fparams = m_frame_data[i]->GetFparams();
            members.push_back( fparams.FrameNum() );
        }
    }// i

    return members;
}


void FrameBuffer::PushFrame(const unsigned int frame_num)
{// Put a new frame onto the top of the stack using built-in frame parameters
 // with frame number frame_num


    // if frame is present - return
    if (IsFrameAvail(frame_num))
        return;

    m_fparams.SetFrameNum(frame_num);
    if ( m_fparams.FSort().IsRef() )
        m_ref_count++;

    int new_frame_pos = -1;
    // First check if an unused frame is available in the buffer
    for (int i = 0; i < (int)m_frame_in_use.size(); ++i)
    {
        if (m_frame_in_use[i] == false)
        {
            new_frame_pos = i;
            m_frame_data[i]->ReconfigFrame(m_fparams);
            m_frame_in_use[i] = true;
            break;
        }
    }
    if (new_frame_pos == -1)
    {
        // No unused frames in buffer. Allocate a new frame
        Frame* fptr = new Frame(m_fparams);
        // add the frame to the buffer
        m_frame_data.push_back(fptr);
        m_frame_in_use.push_back(true);
        new_frame_pos = m_frame_data.size()-1;
    }

    // put the frame number into the index table
    std::pair<unsigned int,unsigned int> temp_pair(m_fparams.FrameNum() , new_frame_pos);
    m_fnum_map.insert(temp_pair);
}

void FrameBuffer::PushFrame( const FrameParams& fp )
{// Put a new frame onto the top of the stack

    // if frame is present - return
    if (IsFrameAvail(fp.FrameNum()))
        return;

    if ( fp.FSort().IsRef() )
        m_ref_count++;

    int new_frame_pos = -1;
    // First check if an unused frame is available in the buffer
    for (int i = 0; i < (int)m_frame_in_use.size(); ++i)
    {
        if (m_frame_in_use[i] == false)
        {
            new_frame_pos = i;
            m_frame_data[i]->ReconfigFrame(fp);
            m_frame_in_use[i] = true;
            break;
        }
    }
    if (new_frame_pos == -1)
    {
        // No unused frames in buffer. Allocate a new frame
        Frame* fptr = new Frame(fp);
        // add the frame to the buffer
        m_frame_data.push_back(fptr);
        new_frame_pos = m_frame_data.size()-1;
        m_frame_in_use.push_back(true);
    }
    // put the frame number into the index table
    std::pair<unsigned int,unsigned int> temp_pair(fp.FrameNum() , new_frame_pos);
    m_fnum_map.insert(temp_pair);
}

void FrameBuffer::PushFrame( const Frame& frame )
{
    int fnum = frame.GetFparams().FrameNum();
    SetFrameParams( fnum );
    PushFrame(fnum);

    bool is_present;

    Frame &f = GetFrame(frame.GetFparams().FrameNum(), is_present);
    if(is_present)
        frame.CopyContents(f);
}

void FrameBuffer::Remove(const unsigned int pos)
{//remove frame fnum from the buffer, shifting everything above down

    const FrameParams& fparams = m_frame_data[pos]->GetFparams();

    if ( m_frame_in_use[pos] == true && fparams.FSort().IsRef() )
        m_ref_count--;

    std::pair<unsigned int,unsigned int>* tmp_pair;

    if (pos<m_frame_data.size())
    {
        //flag that frame is no longer in use
        m_frame_in_use[pos] = false;

         //make a new map
        m_fnum_map.clear();
        for (size_t i=0 ; i<m_frame_data.size() ; ++i)
        {
            if (m_frame_in_use[i] == true)
            {
                tmp_pair = new std::pair<unsigned int,unsigned int>( m_frame_data[i]->GetFparams().FrameNum() , i);
                m_fnum_map.insert(*tmp_pair);
                delete tmp_pair;
            }
        }//i
    }
}


void FrameBuffer::SetRetiredList(const int show_fnum, const int current_coded_fnum)
{
    bool is_present;
    std::vector<int>& retd_list = GetFrame(current_coded_fnum, is_present).GetFparams().RetiredFrames();
    if (is_present )
    {
        retd_list.clear();
        for (size_t i=0 ; i<m_frame_data.size() ; ++i)
        {
            if (m_frame_in_use[i] == true && (m_frame_data[i]->GetFparams().FrameNum() + m_frame_data[i]->GetFparams().ExpiryTime() ) <= show_fnum)
            {
                // Only _reference_ frames go in the retired list - the
                // decoder will retire non-reference frames as they are displayed
                if (m_frame_data[i]->GetFparams().FSort().IsRef() )
                    retd_list.push_back( m_frame_data[i]->GetFparams().FrameNum());
            }
        }//i
    }
}

void FrameBuffer::Clean(const int show_fnum, const int current_coded_fnum)
{// clean out all frames that have expired
    bool is_present;
    std::vector<int>& retd_list = GetFrame(current_coded_fnum, is_present).GetFparams().RetiredFrames();
    if (is_present )
    {
        retd_list.clear();
        for (size_t i=0 ; i<m_frame_data.size() ; ++i)
        {
            if (m_frame_in_use[i] == true && (m_frame_data[i]->GetFparams().FrameNum() + m_frame_data[i]->GetFparams().ExpiryTime() ) <= show_fnum)
                Remove(i);
        }//i
    }
}

void FrameBuffer::Clean(const int fnum)
{// clean out all frames that have expired
    for (size_t i=0 ; i<m_frame_data.size() ; ++i)
    {
        if (m_frame_in_use[i] == true && m_frame_data[i]->GetFparams().FrameNum() == fnum)
        {
            Remove(i);
        }
    }//i
}

void FrameBuffer::SetFrameParams( const unsigned int fnum )
{
    if (!m_interlace)
        SetProgressiveFrameParams(fnum);
    else
        SetInterlacedFrameParams(fnum);
}

void FrameBuffer::SetProgressiveFrameParams( const unsigned int fnum )
{
    // Set the frame parameters, given the GOP set-up and the frame number in display order
    // This function can be ignored by setting the frame parameters directly if required

    m_fparams.SetFrameNum( fnum );
    m_fparams.Refs().clear();

    if ( m_gop_len>0 )
    {

        if ( fnum % m_gop_len == 0)
        {
            m_fparams.SetFSort( FrameSort::IntraRefFrameSort());

            // I frame expires after we've coded the next I frame
            m_fparams.SetExpiryTime( m_gop_len );
        }
        else if (fnum % m_L1_sep == 0)
        {
            m_fparams.SetFSort( FrameSort::InterRefFrameSort());

            // Ref the previous I or L1 frame
            m_fparams.Refs().push_back( fnum - m_L1_sep );

            // if we don't have the first L1 frame ...
            if ((fnum-m_L1_sep) % m_gop_len>0)
                // ... other ref is the prior I frame
                m_fparams.Refs().push_back( ( fnum/m_gop_len ) * m_gop_len  );

            // Expires after the next L1 or I frame
            m_fparams.SetExpiryTime( m_L1_sep );
        }
        else if ((fnum+1) % m_L1_sep == 0)
        {
            m_fparams.SetFSort( FrameSort::InterNonRefFrameSort());

            // .. and the previous frame
            m_fparams.Refs().push_back(fnum-1);
            // Refs are the next I or L1 frame ...
            m_fparams.Refs().push_back(fnum+1);

            m_fparams.SetExpiryTime( 1 );
        }
        else
        {
            m_fparams.SetFSort( FrameSort::InterRefFrameSort());

            // .. and the previous frame
            m_fparams.Refs().push_back(fnum-1);
            // Refs are the next I or L1 frame ...
            m_fparams.Refs().push_back(((fnum/m_L1_sep)+1)*m_L1_sep);

            m_fparams.SetExpiryTime( 1 );
        }

    }
    else{
        if (fnum==0)
        {
            m_fparams.SetFSort( FrameSort::IntraRefFrameSort());

            m_fparams.SetExpiryTime( 1<<30 );//ie never
        }
        else if (fnum % m_L1_sep==0)
        {
            m_fparams.SetFSort( FrameSort::InterRefFrameSort());

            m_fparams.Refs().push_back(0);//frame 0 is the I frame

            if (fnum != m_L1_sep)//we don't have the first L1 frame
                m_fparams.Refs().push_back(fnum-m_L1_sep);//other ref is the prior L1 frame

            //expires after the next L1 or I frame
            m_fparams.SetExpiryTime( m_L1_sep );
        }
        else
        {
            m_fparams.SetFSort( FrameSort::InterNonRefFrameSort());

            m_fparams.Refs().push_back((fnum/m_L1_sep)*m_L1_sep);
            m_fparams.Refs().push_back(((fnum/m_L1_sep)+1)*m_L1_sep);
            m_fparams.SetExpiryTime( 1 );    //L2 frames could expire directly after being coded, but putting in a delay of 1
                                        //allows for frame-skipping to be done, since the frame will still be around to
                                        //be used if the next frame is skipped.
        }
    }
}

void FrameBuffer::SetInterlacedFrameParams( const unsigned int fnum )
{
    // Set the frame parameters, given the GOP set-up and the frame number in display order
    // This function can be ignored by setting the frame parameters directly if required

    m_fparams.SetFrameNum( fnum );
    m_fparams.Refs().clear();


    if ( m_gop_len>0 )
    {

        if ( (fnum/2) % m_gop_len == 0)
        {
            // Field 1 is Intra Field
            m_fparams.SetFSort( FrameSort::IntraRefFrameSort());

            // I frame expires after we've coded the next I frame
            m_fparams.SetExpiryTime( m_gop_len * 2);
            if (m_interlace && fnum%2)
            {
                m_fparams.SetFSort( FrameSort::InterRefFrameSort());
                // Ref the previous I field
                m_fparams.Refs().push_back( fnum-1 );
            }
        }
        else if ((fnum/2) % m_L1_sep == 0)
        {
            m_fparams.SetFSort( FrameSort::InterRefFrameSort());

            if (fnum%2)
            {
                // Field 2
                // Ref the first field of same frame
                m_fparams.Refs().push_back( fnum - 1);
                // Ref the previous field 2 of I or L1 frame
                m_fparams.Refs().push_back( fnum - m_L1_sep*2 );
            }
            else
            {
                // Field 1
                // Ref the field 1 of previous I or L1 frame
                m_fparams.Refs().push_back( fnum - m_L1_sep*2 );
                // Ref the field 2 of previous I or L1 frame
                m_fparams.Refs().push_back( fnum - m_L1_sep*2 + 1 );
            }

            // Expires after the next L1 or I frame
            m_fparams.SetExpiryTime( (m_L1_sep+1)*2-1 );
        }
        else if ((fnum/2+1) % m_L1_sep == 0)
        {
            // Bi-directional non-reference fields.
            m_fparams.SetFSort( FrameSort::InterNonRefFrameSort());

            // .. and the same parity field of the previous frame
            m_fparams.Refs().push_back(fnum-1*2);
            // Refs are the same parity fields in the next I or L1 frame ...
            m_fparams.Refs().push_back(fnum+1*2);

            m_fparams.SetExpiryTime( 1 );
        }
        else
        {
            // Bi-directional reference fields.
            m_fparams.SetFSort( FrameSort::InterRefFrameSort());

            // .. and the same parity field of the previous frame
            m_fparams.Refs().push_back(fnum-1*2);
            // Refs are the same parity fields in the next I or L1 frame ...
            m_fparams.Refs().push_back((((fnum/2)/m_L1_sep+1)*m_L1_sep)*2+(fnum%2));

            m_fparams.SetExpiryTime( 2 );
        }

    }
    else{
        if (fnum/2==0)
        {
            m_fparams.SetFSort( FrameSort::IntraRefFrameSort());

            m_fparams.SetExpiryTime( 1<<30 );//ie never
        }
        else if (fnum/2 % m_L1_sep==0)
        {
            m_fparams.SetFSort( FrameSort::InterRefFrameSort());

            m_fparams.Refs().push_back(0);//frame 0 is the I frame

            if (fnum/2 != m_L1_sep)//we don't have the first L1 frame
                m_fparams.Refs().push_back(fnum-m_L1_sep*2);//other ref is the prior L1 frame

            //expires after the next L1 or I frame
            m_fparams.SetExpiryTime( m_L1_sep * 2 );
        }
        else
        {
            m_fparams.SetFSort( FrameSort::InterNonRefFrameSort());

            m_fparams.Refs().push_back((fnum/m_L1_sep)*m_L1_sep);
            m_fparams.Refs().push_back(((fnum/m_L1_sep)+1)*m_L1_sep);
            m_fparams.SetExpiryTime( 1 );    //L2 frames could expire directly after being coded, but putting in a delay of 1
                                        //allows for frame-skipping to be done, since the frame will still be around to
                                        //be used if the next frame is skipped.
        }
    }
}
