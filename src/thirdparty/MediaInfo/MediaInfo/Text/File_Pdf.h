/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about PDF files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_PdfH
#define MediaInfo_File_PdfH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Sami
//***************************************************************************

class File_Pdf : public File__Analyze
{
public :
    File_Pdf();

private :
    //Streams management
    void Streams_Accept();

    //Buffer - File header
    bool FileHeader_Begin();
    void FileHeader_Parse();

    //Buffer - Global
    void Read_Buffer_Continue();

    //Buffer - Per element
    bool Header_Begin();
    void Header_Parse();
    void Data_Parse();

    //Elements
    void obj();
    void xref();
    void trailer();
    void startxref();
    void eof();
    void Object_Root();
    void Object_Info();
    void Object_Metadata();

    //Helpers
    int64u SizeOfLine();
    bool   Get_Next(string &Key, Ztring &Value); //Returns true if it is an array

    //Temp
    enum state
    {
        State_ParsingElements,
        State_Parsing_xref,
        State_Parsing_startxref,
        State_Parsing_object,
    };
    state State;
    //typedef std::map<int32u, int32u> offsets;
    //offsets Offsets; //Key is offset
    //offsets::iterator Offsets_Current;
    typedef std::vector<int32u> offsets;
    offsets Offsets;
    int32u Offsets_Max;
    enum type
    {
        Type_Root,
        Type_Info,
        Type_Metadata,
        Type_Other,
    };
    struct object
    {
        int32u Offset;
        type Type;
        int32u TopObject;
        size_t BottomPos;
        std::vector<int32u> Bottoms;

        object()
        :
            Offset((int32u)-1),
            #ifdef __BORLANDC__
            Type(type::Type_Other),
            #else
            Type(Type_Other),
            #endif
            TopObject((int32u)-1),
            BottomPos((size_t)-1)
        {
        }

    };
    typedef std::map<int32u, object> objects;
    objects Objects; //Key is object number
    objects::iterator Objects_Current;

    size_t Catalog_Level;
};

} //NameSpace

#endif
