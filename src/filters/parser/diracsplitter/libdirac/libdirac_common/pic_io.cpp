/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: pic_io.cpp 280 2005-01-30 05:11:46Z gabest $ $Name$
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
*                 Scott Robert Ladd,
*                 Stuart Cunningham,
*                 Tim Borer
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

#include <libdirac_common/pic_io.h>
#include <libdirac_common/dirac_assertions.h>
using namespace dirac;

/*************************************Output***********************************/

StreamPicOutput::StreamPicOutput()
{}

StreamPicOutput::~StreamPicOutput()
{}

StreamPicOutput::StreamPicOutput (const SeqParams& sp) :
    m_sparams(sp),
    m_op_pic_ptr(0)
{ }


bool StreamPicOutput::WriteNextFrame( const Frame& myframe )
{
    bool ret_val;

    ret_val=WriteComponent(myframe.Ydata() , Y_COMP );

    if ( m_sparams.CFormat() != Yonly )
    {
        ret_val|=WriteComponent( myframe.Udata() , U_COMP );
        ret_val|=WriteComponent( myframe.Vdata() , V_COMP );
    }

    return ret_val;
}

bool StreamPicOutput::WriteComponent( const PicArray& pic_data , const CompSort& cs)
{
    //initially set up for 10-bit data input, rounded to 8 bits on file output
    //This will throw out any padding to the right and bottom of a frame

    int xl,yl;
    if (cs == Y_COMP)
    {
        xl = m_sparams.Xl();
        yl = m_sparams.Yl();
    }
    else
    {
        xl = m_sparams.ChromaWidth();
        yl = m_sparams.ChromaHeight();
    }

    unsigned char* tempc=new unsigned char[xl];
    ValueType tempv;

    if (m_op_pic_ptr)
    {
        for (int j=0 ; j<yl ;++j)
        {
            for (int i=0 ; i<xl ; ++i)
            {                
                tempv = pic_data[j][i]+2;
                tempv >>= 2;
                tempc[i] = (unsigned char) tempv;                
            }//I

            m_op_pic_ptr->write((char*) tempc,xl);

        }//J
    }
    else
    {
        std::cerr<<std::endl<<"Can't open picture data file for writing";

        //tidy up        
        delete[] tempc;

        //exit failure
        return false;
    }

    m_op_pic_ptr->flush();

    delete[] tempc;

    //exit success
    return true;
}

MemoryStreamOutput::MemoryStreamOutput()
{
    //picture input
    m_op_pic_ptr =
        new std::ostream(&m_membuf);
}

MemoryStreamOutput::~MemoryStreamOutput()
{
    delete m_op_pic_ptr;
}

void MemoryStreamOutput::SetMembufReference (unsigned char *buf, int buf_size)
{
    m_membuf.SetMembufReference(buf, buf_size);
}


FileStreamOutput::FileStreamOutput(const char* output_name,
                     const SeqParams& sp,
                     bool write_header_only) : StreamPicOutput(sp)
{
    m_op_head_ptr = NULL;

    OpenHeader(output_name);
    if (! write_header_only)
        OpenYUV(output_name);
}

bool FileStreamOutput::OpenHeader(const char* output_name)
{
    char output_name_hdr[FILENAME_MAX];

    strncpy(output_name_hdr, output_name, sizeof(output_name_hdr));
    strcat(output_name_hdr, ".hdr");

    //header output
    m_op_head_ptr =
        new std::ofstream(output_name_hdr,std::ios::out | std::ios::binary);

    if (!(*m_op_head_ptr))
    {
        std::cerr <<std::endl <<
            "Can't open output header file for output: " << 
             output_name_hdr << std::endl;

        return false;    

    }

    return true;
}

bool FileStreamOutput::OpenYUV(const char* output_name)
{
    char output_name_yuv[FILENAME_MAX];

    strncpy(output_name_yuv,output_name, sizeof(output_name_yuv));
    strcat(output_name_yuv,".yuv");

    //picture output
    m_op_pic_ptr =
        new std::ofstream(output_name_yuv,std::ios::out | std::ios::binary);

    if (!(*m_op_pic_ptr))
    {
        std::cerr << std::endl << 
            "Can't open output picture data file for output: " << 
            output_name_yuv<<std::endl;

        return false;    

    }

    return true;
}

FileStreamOutput::~FileStreamOutput()
{
    if (m_op_head_ptr && *m_op_head_ptr)
    {
        m_op_head_ptr->close();
        delete m_op_head_ptr;
    }

    if (m_op_pic_ptr && *m_op_pic_ptr)
    {
        static_cast<std::ofstream *>(m_op_pic_ptr)->close();
        delete m_op_pic_ptr;
    }
}

//write a human-readable picture header as separate file
bool FileStreamOutput::WritePicHeader()
{
    if (!m_op_head_ptr || !*m_op_head_ptr)
        return false;

    *m_op_head_ptr << m_sparams.CFormat() << std::endl;
    *m_op_head_ptr << m_sparams.Xl() << std::endl;
    *m_op_head_ptr << m_sparams.Yl() << std::endl;
    *m_op_head_ptr << m_sparams.Interlace() << std::endl;
    *m_op_head_ptr << m_sparams.TopFieldFirst() << std::endl;
    *m_op_head_ptr << m_sparams.FrameRate() << std::endl;

    return true;

}

/**************************************Input***********************************/

StreamPicInput::StreamPicInput () :
    m_ip_pic_ptr(0),
    m_xpad(0),
    m_ypad(0)
{}

StreamPicInput::StreamPicInput (std::istream *ip_pic_ptr, const SeqParams &sparams) :
    m_sparams(m_sparams),
    m_ip_pic_ptr(ip_pic_ptr),
    m_xpad(0),
    m_ypad(0)
{}

StreamPicInput::~StreamPicInput ()
{}

void StreamPicInput::SetPadding(const int xpd, const int ypd)
{
    m_xpad=xpd;
    m_ypad=ypd;
}

bool StreamPicInput::ReadNextFrame(Frame& myframe)
{
    //return value. Failure if one of the components can't be read,
    //success otherwise/.

    bool ret_val;
    ret_val=ReadComponent( myframe.Ydata() , Y_COMP);

    if (m_sparams.CFormat() != Yonly)
    {
        ret_val|=ReadComponent(myframe.Udata() , U_COMP);
        ret_val|=ReadComponent(myframe.Vdata() , V_COMP);
    }

    return ret_val;
}


bool StreamPicInput::ReadComponent(PicArray& pic_data, const CompSort& cs)
{
    if (! *m_ip_pic_ptr)
        return false;

    //initially set up for 8-bit file input expanded to 10 bits for array output

    int xl,yl;
    if (cs == Y_COMP){
        xl = m_sparams.Xl();
        yl = m_sparams.Yl();
    }
    else{
        if (m_sparams.CFormat() == format411)
        {
            xl = m_sparams.Xl()/4;
            yl = m_sparams.Yl();
        }
        else if (m_sparams.CFormat()==format420)
        {
            xl = m_sparams.Xl()/2;
            yl = m_sparams.Yl()/2;
        }
        else if (m_sparams.CFormat() == format422)
        {
            xl = m_sparams.Xl()/2;
            yl = m_sparams.Yl();
        }
        else{
            xl = m_sparams.Xl();
            yl = m_sparams.Yl();
        }
    }

    unsigned char * temp = new unsigned char[xl];//array big enough for one line

    for (int j=0 ; j<yl ; ++j)
    {        
        m_ip_pic_ptr->read((char*) temp, xl);        

        for (int i=0 ; i<xl ; ++i)
        {            
            pic_data[j][i] = (ValueType) temp[i];
            pic_data[j][i] <<= 2;
        }//I

        //pad the columns on the rhs using the edge value        
        for (int i=xl ; i<pic_data.LengthX() ; ++i ){
            pic_data[j][i] = pic_data[j][xl-1];
        }//I

    }//J

    delete [] temp;

    //now do the padded lines, using the last true line
    for (int j=yl ; j<pic_data.LengthY() ; ++j )
    {
        for (int i=0 ; i<pic_data.LengthX() ; ++i )
        {
            pic_data[j][i] = pic_data[yl-1][i];
        }//I
    }//J

    return true;
}

MemoryStreamInput::MemoryStreamInput()
{
    //picture input
    m_ip_pic_ptr =
        new std::istream(&m_membuf);
}

MemoryStreamInput::~MemoryStreamInput()
{
    delete m_ip_pic_ptr;
}

void MemoryStreamInput::SetMembufReference (unsigned char *buf, int buf_size)
{
    m_membuf.SetMembufReference(buf, buf_size);
}

void MemoryStreamInput::Skip(const int num)
{
    REPORTM (false, "MemoryStreamInput::Skip - Reached unimplemented function");
}

FileStreamInput::FileStreamInput(const char* input_name)
{

    char input_name_yuv[FILENAME_MAX];
    char input_name_hdr[FILENAME_MAX];

    strncpy(input_name_yuv, input_name, sizeof(input_name_yuv));
    strncpy(input_name_hdr, input_name, sizeof(input_name_hdr));
    strcat(input_name_yuv, ".yuv");
    strcat(input_name_hdr, ".hdr");

    //header output
    m_ip_head_ptr =
        new std::ifstream(input_name_hdr,std::ios::in | std::ios::binary);
    //picture output
    m_ip_pic_ptr =
        new std::ifstream(input_name_yuv,std::ios::in | std::ios::binary);

    if (!(*m_ip_head_ptr))
        std::cerr << std::endl <<
            "Can't open input header file: " << input_name_hdr << std::endl;
    if (!(*m_ip_pic_ptr))
        std::cerr << std::endl<<
            "Can't open input picture data file: " <<
            input_name_yuv << std::endl;
}

FileStreamInput::~FileStreamInput()
{
    static_cast<std::ifstream *>(m_ip_pic_ptr)->close();
    m_ip_head_ptr->close();
    delete m_ip_pic_ptr;
    delete m_ip_head_ptr;
}

//read a picture header from a separate file
bool FileStreamInput::ReadPicHeader()
{
    if (! *m_ip_head_ptr)
        return false;

    int temp_int;
    bool temp_bool;

    *m_ip_head_ptr >> temp_int;
    m_sparams.SetCFormat( (ChromaFormat)temp_int );

    *m_ip_head_ptr >> temp_int;
    m_sparams.SetXl( temp_int );
 
   *m_ip_head_ptr >> temp_int;
    m_sparams.SetYl( temp_int );

    *m_ip_head_ptr >> temp_bool;
    m_sparams.SetInterlace( temp_bool );

    *m_ip_head_ptr >> temp_bool;
    m_sparams.SetTopFieldFirst( temp_bool );

    *m_ip_head_ptr >> temp_int;    
    m_sparams.SetFrameRate( temp_int );

    return true;
}

void FileStreamInput::Skip(const int num)
{
    const int num_pels = m_sparams.Xl()*m_sparams.Yl();
    int num_bytes;
 
    const ChromaFormat cf = m_sparams.CFormat();

    if ( cf == Yonly)
        num_bytes = num_pels;
    else if ( cf == format411 || cf == format420 )
       num_bytes = (num_pels*3)/2;
    else if ( cf == format422 )
       num_bytes = num_pels*2;
    else
       num_bytes = num_pels*3;

    m_ip_pic_ptr->seekg( num*num_bytes , std::ios::cur );
}

bool StreamPicInput::End() const
{
    return m_ip_pic_ptr->eof();
}
