/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: frame_buffer.cpp 280 2005-01-30 05:11:46Z gabest $ $Name$
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
* Contributor(s): Thomas Davies (Original Author), Scott R Ladd
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

//Simple constructor for general operation
FrameBuffer::FrameBuffer(ChromaFormat cf,int xlen,int ylen): 
    m_fparams(cf,xlen,ylen),
    m_num_L1(0),
    m_L1_sep(1),
    m_gop_len(0)
{}    

//Constructor for coding with an initial I-frame only    
FrameBuffer::FrameBuffer(ChromaFormat cf,int L1sep, int xlen, int ylen):
    m_fparams(cf,xlen,ylen),
    m_num_L1(0),
    m_L1_sep(L1sep),
    m_gop_len(0)
{}

//Constructor setting GOP parameters for use with a standard GOP
FrameBuffer::FrameBuffer(ChromaFormat cf,int numL1,int L1sep,int xlen,int ylen): 
    m_fparams(cf,xlen,ylen),
    m_num_L1(numL1),
    m_L1_sep(L1sep)
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
    for (size_t i=0 ; i<m_frame_data.size() ; ++i){
        m_frame_data[i] = new Frame( *(cpy.m_frame_data[i]) );
    }//i

    // now copy the map
    m_fnum_map = cpy.m_fnum_map;

    // and the internal frame parameters
    m_fparams = cpy.m_fparams;
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
        for (size_t i=0 ; i<m_frame_data.size() ; ++i)
        {
            m_frame_data[i] = new Frame( *(rhs.m_frame_data[i]) );
        }//i

        // now copy the map
        m_fnum_map = rhs.m_fnum_map;

        // and the internal frame parameters
        m_fparams = rhs.m_fparams;
    }
    return *this;
}

//Destructor
FrameBuffer::~FrameBuffer()
{
    for (size_t i=0 ; i<m_frame_data.size() ;++i)
        delete m_frame_data[i];

}

Frame& FrameBuffer::GetFrame( unsigned int fnum )
{//get frame with a given frame number, NOT with a given position in the buffer.
 //If the frame number does not occur, the first frame in the buffer is returned.

    std::map<unsigned int,unsigned int>::iterator it = m_fnum_map.find(fnum);

    unsigned int pos = 0;    
    if (it != m_fnum_map.end()) 
        pos = it->second;    

    return *(m_frame_data[pos]);
}

const Frame& FrameBuffer::GetFrame( unsigned int fnum ) const
{    //as above, but const version

    std::map<unsigned int,unsigned int>::const_iterator it = m_fnum_map.find(fnum);

    unsigned int pos=0;
    if (it != m_fnum_map.end()) 
        pos = it->second;

    return *(m_frame_data[pos]);
}

PicArray& FrameBuffer::GetComponent( unsigned int fnum , CompSort c)
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

const PicArray& FrameBuffer::GetComponent( unsigned int fnum , CompSort c ) const 
{//as above, but const version
 
    std::map<unsigned int,unsigned int>::const_iterator it = m_fnum_map.find(fnum);

    unsigned int pos;
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
PicArray& FrameBuffer::GetUpComponent(unsigned int fnum, CompSort c){
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

const PicArray& FrameBuffer::GetUpComponent(unsigned int fnum, CompSort c) const {//as above, but const version
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

void FrameBuffer::PushFrame(unsigned int frame_num)
{// Put a new frame onto the top of the stack using built-in frame parameters
 // with frame number frame_num
    m_fparams.SetFrameNum(frame_num);
    Frame* fptr = new Frame(m_fparams);

    // add the frame to the buffer
    m_frame_data.push_back(fptr);
    
    // put the frame number into the index table
    std::pair<unsigned int,unsigned int> temp_pair(m_fparams.FrameNum() , m_frame_data.size()-1);
    m_fnum_map.insert(temp_pair);
}

void FrameBuffer::PushFrame( const FrameParams& fp )
{// Put a new frame onto the top of the stack

    Frame* fptr = new Frame(fp);

    // add the frame to the buffer
    m_frame_data.push_back(fptr);

    // put the frame number into the index table
    std::pair<unsigned int,unsigned int> temp_pair(fp.FrameNum() , m_frame_data.size()-1);
    m_fnum_map.insert(temp_pair);
}

void FrameBuffer::PushFrame( const Frame& frame )
{
    // Put a copy of a new frame onto the top of the stack

    Frame* fptr = new Frame( frame );

    // Add the frame to the buffer
    m_frame_data.push_back(fptr);

    // Put the frame number into the index table
    std::pair<unsigned int,unsigned int> tmp_pair(frame.GetFparams().FrameNum() ,
                                                   m_frame_data.size()-1);
    m_fnum_map.insert(tmp_pair);
}

void FrameBuffer::PushFrame(StreamPicInput* picin,const FrameParams& fp)
{
    //Read a frame onto the top of the stack

    PushFrame(fp);
    picin->ReadNextFrame( *(m_frame_data[m_frame_data.size()-1]) );
}

void FrameBuffer::PushFrame(StreamPicInput* picin, unsigned int fnum)
{
   //Read a frame onto the top of the stack    
    SetFrameParams( fnum );
    PushFrame( picin , m_fparams );
}

void FrameBuffer::Remove(unsigned int pos)
{//remove frame fnum from the buffer, shifting everything above down

    std::pair<unsigned int,unsigned int>* tmp_pair;

    if (pos<m_frame_data.size())
    {

        delete m_frame_data[pos];

        m_frame_data.erase(m_frame_data.begin()+pos,m_frame_data.begin()+pos+1);

         //make a new map
        m_fnum_map.clear();
        for (size_t i=0 ; i<m_frame_data.size() ; ++i)
        {
            tmp_pair = new std::pair<unsigned int,unsigned int>( m_frame_data[i]->GetFparams().FrameNum() , i);
            m_fnum_map.insert(*tmp_pair);
            delete tmp_pair;
        }//i

    }
}

void FrameBuffer::Clean(int fnum)
{// clean out all frames that have expired

    for (size_t i=0 ; i<m_frame_data.size() ; ++i)
    {
        if ((m_frame_data[i]->GetFparams().FrameNum() + m_frame_data[i]->GetFparams().ExpiryTime() ) <= fnum)
            Remove(i);
    }//i
}

void FrameBuffer::SetFrameParams( unsigned int fnum )
{    
    // Set the frame parameters, given the GOP set-up and the frame number in display order
    // This function can be ignored by setting the frame parameters directly if required

    m_fparams.SetFrameNum( fnum );
    m_fparams.Refs().clear();    

    if ( m_gop_len>0 )
    {

        if ( fnum % m_gop_len == 0)
        {
            m_fparams.SetFSort( I_frame );

            // I frame expires after we've coded the next I frame
            m_fparams.SetExpiryTime( m_gop_len );
        }
        else if (fnum % m_L1_sep == 0)
        {
            m_fparams.SetFSort( L1_frame );

            // Ref the previous I or L1 frame
            m_fparams.Refs().push_back( fnum - m_L1_sep );

            // if we don't have the first L1 frame ...
            if ((fnum-m_L1_sep) % m_gop_len>0)
                // ... other ref is the prior I frame
                m_fparams.Refs().push_back( ( fnum/m_gop_len ) * m_gop_len  );

            // Expires after the next L1 or I frame            
            m_fparams.SetExpiryTime( m_L1_sep );
        }
        else
        {
            m_fparams.SetFSort( L2_frame );
            // Refs are the next or previous I or L1 frame
            m_fparams.Refs().push_back((fnum/m_L1_sep)*m_L1_sep);
            m_fparams.Refs().push_back(((fnum/m_L1_sep)+1)*m_L1_sep);

            m_fparams.SetExpiryTime( 1 );  
            // ( L2 frames could expire directly after being coded, but putting in a delay of 1
            // allows for frame-skipping to be done, since the frame will still be around to
            // be used if the next frame is skipped. )
        }
    }    
    else{        
        if (fnum==0)
        {
            m_fparams.SetFSort( I_frame );
            m_fparams.SetExpiryTime( 1<<30 );//ie never
        }
        else if (fnum % m_L1_sep==0)
        {
            m_fparams.SetFSort( L1_frame );
            m_fparams.Refs().push_back(0);//frame 0 is the I frame

            if (fnum != m_L1_sep)//we don't have the first L1 frame    
                m_fparams.Refs().push_back(fnum-m_L1_sep);//other ref is the prior L1 frame

            //expires after the next L1 or I frame                        
            m_fparams.SetExpiryTime( m_L1_sep );
        }
        else
        {
            m_fparams.SetFSort( L2_frame );
            m_fparams.Refs().push_back((fnum/m_L1_sep)*m_L1_sep);
            m_fparams.Refs().push_back(((fnum/m_L1_sep)+1)*m_L1_sep);
            m_fparams.SetExpiryTime( 1 );    //L2 frames could expire directly after being coded, but putting in a delay of 1
                                        //allows for frame-skipping to be done, since the frame will still be around to
                                        //be used if the next frame is skipped.
        }
    }
}
