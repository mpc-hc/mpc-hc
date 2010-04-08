/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: pic_io.cpp,v 1.28 2008/06/19 10:17:17 tjdwave Exp $ $Name:  $
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
*                 Tim Borer,
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

#include <libdirac_common/pic_io.h>
#include <libdirac_common/dirac_assertions.h>
using namespace dirac;

/*************************************Output***********************************/
StreamPicOutput::~StreamPicOutput()
{
}

StreamPicOutput::StreamPicOutput(std::ostream *op_ptr, const SourceParams& sp) :
    m_sparams(sp),
    m_op_pic_ptr(op_ptr)
{
}

StreamFrameOutput::StreamFrameOutput(std::ostream *op_str,
                                     const SourceParams& sp) :
    StreamPicOutput(op_str, sp)
{}

StreamFrameOutput::~StreamFrameOutput()
{
}

bool StreamFrameOutput::WriteToNextFrame(const Picture& myframe)
{
    bool ret_val;

    ret_val = WriteFrameComponent(myframe.Data(Y_COMP), Y_COMP);
    ret_val &= WriteFrameComponent(myframe.Data(U_COMP), U_COMP);
    ret_val &= WriteFrameComponent(myframe.Data(V_COMP), V_COMP);

    return ret_val;
}

bool StreamFrameOutput::WriteFrameComponent(const PicArray& pic_data , const CompSort& cs)
{
    if(!m_op_pic_ptr)
    {
        std::cerr << std::endl << "Can't open picture data file for writing";
        return false;
    }

    //initially set up for 10-bit data input, rounded to 8 bits on file output
    //This will throw out any padding to the right and bottom of a frame

    int xl, yl;
    if(cs == Y_COMP)
    {
        xl = m_sparams.Xl();
        yl = m_sparams.Yl();
    }
    else
    {
        xl = m_sparams.ChromaWidth();
        yl = m_sparams.ChromaHeight();
    }

    unsigned char* tempc = new unsigned char[xl];

    if(m_op_pic_ptr)
    {
        for(int j = 0 ; j < yl ; ++j)
        {
            for(int i = 0 ; i < xl ; ++i)
            {
                tempc[i] = (unsigned char)(pic_data[j][i] + 128);
            }//I

            m_op_pic_ptr->write((char*) tempc, xl);

        }//J
    }
    m_op_pic_ptr->flush();

    delete[] tempc;

    //exit success
    return true;
}

StreamFieldOutput::StreamFieldOutput(std::ostream *op_str,
                                     const SourceParams& sp) :
    StreamPicOutput(op_str, sp),
    m_frame_store(NULL)
{
    int frame_size = (m_sparams.Xl() * m_sparams.Yl()) +
                     2 * (m_sparams.ChromaWidth() * m_sparams.ChromaHeight());
    m_frame_store = new unsigned char[frame_size];
}

StreamFieldOutput::~StreamFieldOutput()
{
    if(m_frame_store)
        delete [] m_frame_store;
}

bool StreamFieldOutput::WriteToNextFrame(const Picture& myfield)
{
    bool ret_val;

    ret_val = WriteFieldComponent(myfield.Data(Y_COMP) , myfield.GetPparams().PictureNum(), Y_COMP);
    ret_val &= WriteFieldComponent(myfield.Data(U_COMP) , myfield.GetPparams().PictureNum(), U_COMP);
    ret_val &= WriteFieldComponent(myfield.Data(V_COMP) , myfield.GetPparams().PictureNum(), V_COMP);

    return ret_val;
}

bool StreamFieldOutput::WriteFieldComponent(const PicArray& pic_data , int field_num, const CompSort& cs)
{
    if(!m_op_pic_ptr)
    {
        std::cerr << std::endl << "Can't open picture data file for writing";
        return false;
    }

    unsigned char *comp;
    int xl, yl;
    if(cs == Y_COMP)
    {
        xl = m_sparams.Xl();
        yl = m_sparams.Yl();
        comp = m_frame_store;
    }
    else
    {
        xl = m_sparams.ChromaWidth();
        yl = m_sparams.ChromaHeight();
        if(cs == U_COMP)
        {
            comp = m_frame_store + (m_sparams.Xl() * m_sparams.Yl());
        }
        else
        {
            comp = m_frame_store + (m_sparams.Xl() * m_sparams.Yl()) + (xl * yl);
        }
    }

    // Seek offset before writing field to store
    int start = 0;
    // Seek offset between writing lines to file
    int skip = 0;
    // Seek offset after writing field to file
    int end = 0;

    bool top_field = m_sparams.TopFieldFirst() ? (!(field_num % 2)) :
                     (field_num % 2);

    bool write_to_file = (m_sparams.TopFieldFirst() && !top_field) ||
                         (!m_sparams.TopFieldFirst() && top_field);

    if(m_sparams.TopFieldFirst())
    {
        if(top_field)
        {
            start = 0;
            skip = 2 * xl * sizeof(char);
            end = -(xl * yl);
        }
        else
        {
            start = xl;
            skip = 2 * xl * sizeof(char);
            end = 0;
        }
    }
    else
    {
        if(!top_field)  // i.e. bottom field
        {
            start = xl;
            skip = 2 * xl * sizeof(char);
            end = -(xl * yl);
        }
        else // top field
        {
            start = 0;
            skip = 2 * xl * sizeof(char);
            end = xl;
        }
    }

    unsigned char *tempc = comp + start;

    int field_yl = yl >> 1;
    int field_xl = xl;
    for(int j = 0 ; j < field_yl ; ++j)
    {
        for(int i = 0 ; i < field_xl ; ++i)
        {
            tempc[i] = (unsigned char)(pic_data[j][i] + 128);
        }//I
        tempc += skip;
    }//J
    tempc += end;

    if(write_to_file)
    {
        m_op_pic_ptr->write((char*) comp, xl * yl);
        m_op_pic_ptr->flush();
        return true;
    }
    //exit success
    return false;
}

MemoryStreamOutput::MemoryStreamOutput(SourceParams &sp, bool interlace)
{
    //picture input
    m_op_pic_ptr =
        new std::ostream(&m_membuf);

    if(interlace)
        m_op_pic_str = new StreamFieldOutput(m_op_pic_ptr, sp);
    else
        m_op_pic_str = new StreamFrameOutput(m_op_pic_ptr, sp);
}

MemoryStreamOutput::~MemoryStreamOutput()
{
    delete m_op_pic_str;
    delete m_op_pic_ptr;
}

void MemoryStreamOutput::SetMembufReference(unsigned char *buf, int buf_size)
{
    m_membuf.SetMembufReference(buf, buf_size);
}

FileStreamOutput::FileStreamOutput(const char* output_name,
                                   const SourceParams& sp, bool interlace)
{
    //picture output
    m_op_pic_ptr =
        new std::ofstream(output_name, std::ios::out | std::ios::binary);

    if(!(*m_op_pic_ptr))
    {
        std::cerr << std::endl <<
                  "Can't open output picture data file for output: " <<
                  output_name << std::endl;
        return;

    }
    if(interlace)
        m_op_pic_str = new StreamFieldOutput(m_op_pic_ptr, sp);
    else
        m_op_pic_str = new StreamFrameOutput(m_op_pic_ptr, sp);
}

FileStreamOutput::~FileStreamOutput()
{
    if(m_op_pic_ptr && *m_op_pic_ptr)
    {
        static_cast<std::ofstream *>(m_op_pic_ptr)->close();
        delete m_op_pic_ptr;
    }
    delete m_op_pic_str;
}


/**************************************Input***********************************/


StreamPicInput::StreamPicInput(std::istream *ip_pic_ptr,
                               const SourceParams &sparams) :
    m_sparams(sparams),
    m_ip_pic_ptr(ip_pic_ptr)
{}


StreamPicInput::~StreamPicInput()
{}

bool StreamPicInput::End() const
{
    return m_ip_pic_ptr->eof();
}

StreamFrameInput::StreamFrameInput(std::istream *ip_pic_ptr,
                                   const SourceParams &sparams) :
    StreamPicInput(ip_pic_ptr, sparams)
{}

StreamFrameInput::~StreamFrameInput()
{}

void StreamFrameInput::Skip(const int num)
{
    const int num_pels = m_sparams.Xl() * m_sparams.Yl();
    int num_bytes;

    const ChromaFormat cf = m_sparams.CFormat();

    if(cf == format420)
        num_bytes = (num_pels * 3) / 2;
    else if(cf == format422)
        num_bytes = num_pels * 2;
    else
        num_bytes = num_pels * 3;

    m_ip_pic_ptr->seekg(num * num_bytes , std::ios::cur);
}

bool StreamFrameInput::ReadNextPicture(Picture& myframe)
{
    //return value. Failure if one of the components can't be read,
    //success otherwise/.

    bool ret_val;

    ret_val = ReadFrameComponent(myframe.Data(Y_COMP) , Y_COMP);
    ret_val &= ReadFrameComponent(myframe.Data(U_COMP) , U_COMP);
    ret_val &= ReadFrameComponent(myframe.Data(V_COMP) , V_COMP);

    return ret_val;
}

bool StreamFrameInput::ReadFrameComponent(PicArray& pic_data, const CompSort& cs)
{

    if(! *m_ip_pic_ptr)
        return false;

    int xl, yl;
    if(cs == Y_COMP)
    {
        xl = m_sparams.Xl();
        yl = m_sparams.Yl();
    }
    else
    {
        if(m_sparams.CFormat() == format420)
        {
            xl = m_sparams.Xl() / 2;
            yl = m_sparams.Yl() / 2;
        }
        else if(m_sparams.CFormat() == format422)
        {
            xl = m_sparams.Xl() / 2;
            yl = m_sparams.Yl();
        }
        else
        {
            xl = m_sparams.Xl();
            yl = m_sparams.Yl();
        }
    }

    unsigned char * temp = new unsigned char[xl];//array big enough for one line

    for(int j = 0 ; j < yl ; ++j)
    {
        m_ip_pic_ptr->read((char*) temp, xl);

        for(int i = 0 ; i < xl ; ++i)
        {
            pic_data[j][i] = (ValueType) temp[i];
        }//I
        for(int i = 0 ; i < xl ; ++i)
        {
            pic_data[j][i] -= 128;
        }//I


        //pad the columns on the rhs using the edge value
        for(int i = xl ; i < pic_data.LengthX() ; ++i)
        {
            pic_data[j][i] = pic_data[j][xl-1];
        }//I

    }//J

    delete [] temp;

    //now do the padded lines, using the last true line
    for(int j = yl ; j < pic_data.LengthY() ; ++j)
    {
        for(int i = 0 ; i < pic_data.LengthX() ; ++i)
        {
            pic_data[j][i] = pic_data[yl-1][i];
        }//I
    }//J

    return true;
}

StreamFieldInput::StreamFieldInput(std::istream *ip_pic_ptr,
                                   const SourceParams &sparams) :
    StreamPicInput(ip_pic_ptr, sparams)
{}

StreamFieldInput::~StreamFieldInput()
{}

void StreamFieldInput::Skip(const int num)
{
    REPORTM(num && false, "StreamFieldInput::Skip - Reached unimplemented function");
}

bool StreamFieldInput::ReadNextPicture(Picture& mypic)
{
    // FIXME: this method is BROKEN!

    //return value. Failure if one of the components can't be read,
    //success otherwise/.

    bool ret_val;

    bool is_field1 = ((mypic.GetPparams().PictureNum() % 2) == 0);
    ret_val = ReadFieldComponent(is_field1, mypic.Data(Y_COMP), Y_COMP);
    ret_val &= ReadFieldComponent(is_field1, mypic.Data(U_COMP), U_COMP);
    ret_val &= ReadFieldComponent(is_field1, mypic.Data(V_COMP), V_COMP);

    int picture_size = m_sparams.Xl() * m_sparams.Yl() +
                       2 * m_sparams.ChromaWidth() * m_sparams.ChromaHeight();
    if(is_field1)
    {
        //Seek back to the beginning of frame so that the next field
        //from the frame can be read
        m_ip_pic_ptr->seekg(-picture_size, std::ios::cur);
    }

    return ret_val;
}

bool StreamFieldInput::ReadNextFrame(Picture& field1, Picture& field2)
{
    //return value. Failure if one of the components can't be read,
    //success otherwise/.

    bool ret_val = false;

    ret_val = ReadFieldComponent(field1.Data(Y_COMP), field2.Data(Y_COMP), Y_COMP);
    ret_val &= ReadFieldComponent(field1.Data(U_COMP), field2.Data(U_COMP), U_COMP);
    ret_val &= ReadFieldComponent(field1.Data(V_COMP), field2.Data(V_COMP), V_COMP);

    return ret_val;
}

bool StreamFieldInput::ReadFieldComponent(PicArray& pic_data1,
        PicArray& pic_data2,
        const CompSort& cs)
{
    if(! *m_ip_pic_ptr)
        return false;

    //initially set up for 8-bit file input expanded to 10 bits for array output

    int xl, yl;
    if(cs == Y_COMP)
    {
        xl = m_sparams.Xl();
        yl = m_sparams.Yl();
    }
    else
    {
        if(m_sparams.CFormat() == format420)
        {
            xl = m_sparams.Xl() / 2;
            yl = m_sparams.Yl() / 2;
        }
        else if(m_sparams.CFormat() == format422)
        {
            xl = m_sparams.Xl() / 2;
            yl = m_sparams.Yl();
        }
        else
        {
            xl = m_sparams.Xl();
            yl = m_sparams.Yl();
        }
    }

    unsigned char * temp = new unsigned char[xl];//array big enough for one line
    ValueType *pic;

    for(int j = 0 ; j < yl ; j++)
    {
        m_ip_pic_ptr->read((char*) temp, xl);
        if(j % 2 == 0)
        {
            pic = m_sparams.TopFieldFirst() ?
                  &pic_data1[j/2][0] : &pic_data2[j/2][0];
        }
        else
        {
            pic = m_sparams.TopFieldFirst() ?
                  &pic_data2[j/2][0] : &pic_data1[j/2][0];
        }
        for(int i = 0 ; i < xl ; ++i)
        {
            pic[i] = (ValueType) temp[i];
        }//I
        for(int i = 0 ; i < xl ; ++i)
        {
            pic[i] -= 128;
        }//I


        //pad the columns on the rhs using the edge value
        for(int i = xl ; i < pic_data1.LengthX() ; ++i)
        {
            pic[i] = pic[xl-1];
        }//I

    }//J

    delete [] temp;

    //now do the padded lines, using the last true line
    for(int j = yl / 2 ; j < pic_data1.LengthY() ; ++j)
    {
        for(int i = 0 ; i < pic_data1.LengthX() ; ++i)
        {
            pic_data1[j][i] = pic_data1[yl/2-1][i];
            pic_data2[j][i] = pic_data2[yl/2-1][i];
        }//I
    }//J

    return true;
}

bool StreamFieldInput::ReadFieldComponent(bool is_field1,
        PicArray& pic_data,
        const CompSort& cs)
{
    if(! *m_ip_pic_ptr)
        return false;

    //initially set up for 8-bit file input expanded to 10 bits for array output

    int xl, yl;
    if(cs == Y_COMP)
    {
        xl = m_sparams.Xl();
        yl = m_sparams.Yl() >> 1;
    }
    else
    {
        xl = m_sparams.ChromaWidth();
        yl = m_sparams.ChromaHeight() >> 1;
    }

    unsigned char * pic = new unsigned char[2*xl];//array big enough for two lines - one for each field

    int start = 0;
    if((is_field1 && !m_sparams.TopFieldFirst()) ||
       (!is_field1 && m_sparams.TopFieldFirst()))
    {
        start = xl;
    }

    for(int j = 0 ; j < yl ; j++)
    {
        m_ip_pic_ptr->read((char*) pic, 2 * xl);
        // skip to the start of the field
        unsigned char *field = pic + start;
        for(int i = 0 ; i < xl ; ++i)
        {
            pic_data[j][i] = (ValueType) field[i];
        }//I
        for(int i = 0 ; i < xl ; ++i)
        {
            pic_data[j][i] -= 128;
        }
        //pad the columns on the rhs using the edge value
        for(int i = xl ; i < pic_data.LengthX() ; ++i)
        {
            pic_data[j][i] = pic_data[j][xl-1];
        }//I

    }//J
    delete [] pic;

    //now do the padded lines, using the last true line
    for(int j = yl ; j < pic_data.LengthY() ; ++j)
    {
        for(int i = 0 ; i < pic_data.LengthX() ; ++i)
        {
            pic_data[j][i] = pic_data[yl-1][i];
        }//I
    }//J

    return true;
}

MemoryStreamInput::MemoryStreamInput(SourceParams& sparams, bool field_input)
{
    //picture input
    m_ip_pic_ptr =
        new std::istream(&m_membuf);

    if(field_input)
        m_inp_str = new StreamFieldInput(m_ip_pic_ptr, sparams);
    else
        m_inp_str = new StreamFrameInput(m_ip_pic_ptr, sparams);
}

MemoryStreamInput::~MemoryStreamInput()
{
    delete m_ip_pic_ptr;
    delete m_inp_str;
}

void MemoryStreamInput::SetMembufReference(unsigned char *buf, int buf_size)
{
    m_membuf.SetMembufReference(buf, buf_size);
}

FileStreamInput::FileStreamInput(const char* input_name,
                                 const SourceParams &sparams,
                                 bool interlace)
{

    char input_name_yuv[FILENAME_MAX];

    strncpy(input_name_yuv, input_name, sizeof(input_name_yuv));
    //strcat(input_name_yuv, ".yuv");

    //picture input
    m_ip_pic_ptr =
        new std::ifstream(input_name_yuv, std::ios::in | std::ios::binary);

    if(!(*m_ip_pic_ptr))
        std::cerr << std::endl <<
                  "Can't open input picture data file: " <<
                  input_name_yuv << std::endl;

    if(interlace)
        m_inp_str = new StreamFieldInput(m_ip_pic_ptr, sparams);
    else
        m_inp_str = new StreamFrameInput(m_ip_pic_ptr, sparams);

}

FileStreamInput::~FileStreamInput()
{
    static_cast<std::ifstream *>(m_ip_pic_ptr)->close();
    delete m_ip_pic_ptr;
    delete m_inp_str;
}
