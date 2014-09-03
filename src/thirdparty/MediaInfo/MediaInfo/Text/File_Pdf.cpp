/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//---------------------------------------------------------------------------
// Pre-compilation
#include "MediaInfo/PreComp.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Setup.h"
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_PDF_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Text/File_Pdf.h"
#include "MediaInfo/Tag/File_Xmp.h"
#include <cstdlib>
#include <algorithm>
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Pdf::File_Pdf()
:File__Analyze()
{
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Pdf::Streams_Accept()
{
    Fill(Stream_General, 0, General_Format, "PDF");

    Stream_Prepare(Stream_Text);
    Fill(Stream_Text, 0, "Format", "PDF");
}

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Pdf::FileHeader_Begin()
{
    //Synchro
    if (5>Buffer_Size)
        return false;
    if (Buffer[0]!=0x25 //"%PDF-"
     || Buffer[1]!=0x50
     || Buffer[2]!=0x44
     || Buffer[3]!=0x46
     || Buffer[4]!=0x2D)
    {
        Reject();
        return false;
    }

    Accept();

    //Temp
    Catalog_Level=0;
    Offsets_Max=0;

    //All should be OK...
    return true;
}

//---------------------------------------------------------------------------
void File_Pdf::FileHeader_Parse()
{
    string PdfHeader;
    Get_String(SizeOfLine(), PdfHeader,                         "Header");
    for (;;)
    {
        int64u CommentSize=SizeOfLine();
        if (Buffer_Offset+Element_Offset>=Buffer_Size)
        {
            Element_WaitForMoreData();
            return;
        }
        if (Buffer[Buffer_Offset+Element_Offset]!='%')
            break;
        Skip_String(CommentSize,                                "Comment");
    }

    //Filling
    Fill(Stream_General, 0, General_Format_Version, PdfHeader.substr(5));

    GoToFromEnd(9+2+10+2+5+2); // "startxref" + EOL +  10max digits + EOL + "%%EOF" + EOL
    State=State_Parsing_startxref;
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Pdf::Read_Buffer_Continue()
{
    switch (State)
    {
        case State_Parsing_xref         : xref(); if (!Element_IsWaitingForMoreData()) trailer(); break;
        case State_Parsing_startxref    : eof(); startxref(); break;
        case State_Parsing_object       : break; //Using elements
        default                         : Finish();
    }
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Pdf::Header_Begin()
{
    //Offsets_Current=Offsets.find(Objects_Current->second.Offset);
    //offsets::iterator Offsets_Next=Offsets_Current;
    //Offsets_Next++;
    //if (Offsets_Next!=Offsets.end() && Offsets_Next->first>File_Offset+Buffer_Size)
    //{
    //    Element_WaitForMoreData();
    //    return false;
    //}

    return true;
}

//---------------------------------------------------------------------------
void File_Pdf::Header_Parse()
{
    offsets::iterator Offsets_Next=upper_bound(Offsets.begin(), Offsets.end(), (int32u)(File_Offset+Buffer_Offset));
    if (Offsets_Next!=Offsets.end() && *Offsets_Next>File_Offset+Buffer_Size)
    {
        Element_WaitForMoreData();
        return;
    }

    int64u Size;
    //if (Offsets_Current==Offsets.end())
    //    Size=Offsets_Max-(File_Offset+Buffer_Offset);
    //else
    //    Size=Offsets_Current->first-(File_Offset+Buffer_Offset);
    if (Offsets_Next==Offsets.end())
        Size=Offsets_Max-(File_Offset+Buffer_Offset);
    else
        Size=*Offsets_Next-(File_Offset+Buffer_Offset);

    Header_Fill_Size(Size);
}

//---------------------------------------------------------------------------
void File_Pdf::Data_Parse()
{
    Element_Name("Object");

    string Line;
    Get_String(SizeOfLine(), Line,                              "Header");

    size_t Space_Pos=Line.find(' ');
    int32u ObjectNumber=Ztring().From_UTF8(Line.substr(0, Space_Pos)).To_int32u();
    Element_Info1(ObjectNumber);
    objects::iterator Object=Objects.find(ObjectNumber);
    if (Object==Objects.end())
        Skip_XX(Element_Size-Element_Offset,                    "Data");
    else
        switch(Object->second.Type)
        {
            case Type_Root      : Object_Root(); break;
            case Type_Info      : Object_Info(); break;
            case Type_Metadata  : Object_Metadata(); break;
            default             : Skip_XX(Element_Size-Element_Offset, "Data");
        }

    for (;;)
    {
        Objects_Current->second.BottomPos++;
        if (Objects_Current->second.BottomPos>=Objects_Current->second.Bottoms.size())
        {
            if (Objects_Current->first==(int32u)-1)
            {
                //No more to parse
                Objects_Current=Objects.end();
                Objects.clear();
                Finish();
                break;
            }

            Objects_Current=Objects.find(Objects_Current->second.TopObject);
            continue;
        }

         Objects_Current=Objects.find(Objects_Current->second.Bottoms[Objects_Current->second.BottomPos]);
         GoTo(Objects_Current->second.Offset);
         break;
    }
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_Pdf::xref()
{
    //Parsing
    Element_Begin1("Cross-Reference Table");

    Element_Begin1("Cross-Reference Section");

    string FirstLine;
    Skip_String(SizeOfLine(),                                       "Object name");
    Element_Begin1("Cross-Reference SubSection");
        Get_String(SizeOfLine(), FirstLine,                         "Header");
        size_t FirstLine_Space=FirstLine.find(' ');
        int32u Base=atoi((const char*)FirstLine.c_str());
        int32u Count=0;
        if (FirstLine_Space!=string::npos)
            Count=atoi((const char*)FirstLine.c_str()+FirstLine_Space+1);

        if (0x10000+20*Count>Buffer_Size && File_Offset+Buffer_Size<File_Size)
        {
            // We wait for more data
            Buffer_Offset=0;
            Element_Offset=0;
            Element_DoNotShow();
            Element_End0();
            Element_End0();
            Element_End0();
            Element_WaitForMoreData();
            return;
        }

        while (Element_Offset<Element_Size && (Buffer[Buffer_Offset+(size_t)Element_Offset]=='\r' || Buffer[Buffer_Offset+(size_t)Element_Offset]=='\n'))
            Element_Offset++;
        const int8u* Buffer_Temp=Buffer+Buffer_Offset+(size_t)Element_Offset+17;
        for (int32u Pos=0; Pos<Count; ++Pos)
        {
            if (*Buffer_Temp=='n')
            {
                int32u Offset=(int32u)atoi((const char*)(Buffer_Temp-17));
                Objects[Base+Pos].Offset=Offset;
                //Offsets[Offset]=Base+Pos;
                Offsets.push_back(Offset);
            }
            Buffer_Temp+=20;
            if (Pos>100)
                Element_Offset+=20;
            else
            {
                Skip_String(18,                                 "Entry"); Param_Info1(Base+Pos);
                Element_Offset+=2; //Skipping spaces at end and line return
            }
        }
    Element_End0();

    Element_End0();

    Element_End0();

    if (File_Offset+Buffer_Offset>Offsets_Max)
        Offsets_Max=(int32u)(File_Offset+Buffer_Offset);
}

//---------------------------------------------------------------------------
void File_Pdf::trailer()
{
    Element_Begin1("Trailer");

    //Parsing
    int32u Prev=(int32u)-1;
    string Key;
    Ztring Value;
    Skip_String(SizeOfLine(),                                       "Object name");
    while (Element_Offset<Element_Size)
    {
        if (Get_Next(Key, Value))
        {
            for (;;)
            {
                Get_Next(Key, Value);
                if (Key.empty())
                    break;
                else if (Key=="Root")
                {
                    int32u ObjectNumber=Value.To_int32u();
                    Objects[ObjectNumber].Type=Type_Root;
                    Objects[ObjectNumber].TopObject=(int32u)-1;
                    Objects[(int32u)-1].Bottoms.push_back(ObjectNumber);
                    Param_Info1(__T("Document Catalog is at offset 0x"+Ztring().From_Number(Objects[ObjectNumber].Offset, 16)));
                }
                else if (Key=="Info")
                {
                    int32u ObjectNumber=Value.To_int32u();
                    Objects[ObjectNumber].Type=Type_Info;
                    Objects[ObjectNumber].TopObject=(int32u)-1;
                    Objects[(int32u)-1].Bottoms.push_back(ObjectNumber);
                    Param_Info1(__T("Info is at offset 0x"+Ztring().From_Number(Objects[ObjectNumber].Offset, 16)));
                }
                else if (Key=="Prev")
                {
                    Prev=Value.To_int32u();
                    Param_Info1(__T("Previous Cross-Reference Table is at offset 0x"+Ztring().From_Number(Prev, 16)));
                }
            }

            continue;
        }

        if (Key.empty())
            break;
    }

    Element_End0();

    //Previous Cross-Reference Table
    if (Prev!=(int32u)-1)
    {
        GoTo(Prev);
        return;
    }

    objects::iterator Object_Top=Objects.find((int32u)-1);
    if (Offsets.empty() || Object_Top==Objects.end())
    {
        Finish();
        return;
    }

    sort(Offsets.begin(), Offsets.end());

    //Offsets_Current=Offsets.end(); //No more used for the moment
    Objects[(int32u)-1].BottomPos=0;
    Objects_Current=Objects.find(Object_Top->second.Bottoms[0]);
    GoTo(Objects_Current->second.Offset);
    State=State_Parsing_object;
}

//---------------------------------------------------------------------------
void File_Pdf::startxref()
{
    //We need to find the exact begin
    Buffer_Offset=Buffer_Size-1;
    while (Buffer_Offset && (Buffer[Buffer_Offset]=='\r' || Buffer[Buffer_Offset]=='\n'))
        Buffer_Offset--;
    Buffer_Offset-=5; // "%%EOF"
    while (Buffer_Offset && (Buffer[Buffer_Offset]=='\r' || Buffer[Buffer_Offset]=='\n'))
        Buffer_Offset--;
    while (Buffer_Offset && Buffer[Buffer_Offset]>='0' && Buffer[Buffer_Offset]<='9') // Value
        Buffer_Offset--;
    while (Buffer_Offset && (Buffer[Buffer_Offset]=='\r' || Buffer[Buffer_Offset]=='\n'))
        Buffer_Offset--;

    Buffer_Offset-=8;

    //Parsing
    Element_Begin1("Cross-Reference Table Offset");

    string xrefOffsetS;
    Skip_String(SizeOfLine(),                                   "Object name");
    Get_String (SizeOfLine(), xrefOffsetS,                      "xref Offset");
    while (Buffer_Offset<Buffer_Size && (Buffer[Buffer_Offset]=='\r' || Buffer[Buffer_Offset]=='\n'))
        ++Buffer_Offset;
    int32u xref_Offset=atoi(xrefOffsetS.c_str());

    Element_End0();

    //Going to xref
    if (xref_Offset>Offsets_Max)
        Offsets_Max=xref_Offset;
    GoTo (xref_Offset);
    State=State_Parsing_xref;
}

//---------------------------------------------------------------------------
void File_Pdf::eof()
{
    if (File_Size!=(int64u)-1 && File_Offset+Buffer_Size<File_Size)
    {
        Element_WaitForMoreData();
        return;
    }

    //We need to find the exact begin
    Buffer_Offset=Buffer_Size-1;
    while (Buffer_Offset && (Buffer[Buffer_Offset]=='\r' || Buffer[Buffer_Offset]=='\n'))
        Buffer_Offset--;

    Buffer_Offset-=5;

    //Parsing
    Element_Begin1("End Of File");

    Skip_String(SizeOfLine(),                                   "Object name");

    Element_End0();
}

//---------------------------------------------------------------------------
void File_Pdf::Object_Root()
{
    Element_Info1("Document Catalog");

    //Parsing
    string Key;
    Ztring Value;
    while (Element_Offset<Element_Size)
    {
        if (Get_Next(Key, Value))
        {
            for (;;)
            {
                Get_Next(Key, Value);
                if (Key.empty())
                    break;
                else if (Key=="Metadata")
                {
                    int32u ObjectNumber=Value.To_int32u();
                    Objects[ObjectNumber].Type=Type_Metadata;
                    Objects[ObjectNumber].TopObject=Objects_Current->first;
                    Objects[Objects_Current->first].Bottoms.push_back(ObjectNumber);
                    Param_Info1(__T("Metadata is at offset 0x"+Ztring().From_Number(Objects[ObjectNumber].Offset)));
                }
            }

            continue;
        }

        if (Key.empty())
            break;
    }
}

//---------------------------------------------------------------------------
void File_Pdf::Object_Info()
{
    Element_Info1("Info");

    //Parsing
    string Key;
    Ztring Value;
    while (Element_Offset<Element_Size)
    {
        if (Get_Next(Key, Value))
        {
            for (;;)
            {
                Get_Next(Key, Value);
                if (Key.empty())
                    break;
            }

            continue;
        }

        if (Key.empty())
            break;
    }
}

//---------------------------------------------------------------------------
void File_Pdf::Object_Metadata()
{
    Element_Info1("Metadata");

    //Parsing
    string Key;
    Ztring Value;
    int32u Length=0;
    while (Element_Offset<Element_Size)
    {
        if (Get_Next(Key, Value))
        {
            for (;;)
            {
                Get_Next(Key, Value);
                if (Key.empty())
                    break;
                else if (Key=="Length")
                {
                    Length=Value.To_int32u();
                }
            }

            continue;
        }

        if (Key.empty())
            break;
        else if (Key=="stream")
        {
            //Removig end of lines
            if (Element_Offset<Element_Size && Buffer[Buffer_Offset+(size_t)Element_Offset]=='\r')
                Element_Offset++;
            if (Element_Offset<Element_Size && Buffer[Buffer_Offset+(size_t)Element_Offset]=='\n')
                Element_Offset++;

            File_Xmp MI;
            Open_Buffer_Init(&MI, Length);
            Open_Buffer_Continue(&MI, Buffer+Buffer_Offset+(size_t)Element_Offset, Length);
            Skip_XX(Length,                                     "Stream, Data");
            Open_Buffer_Finalize(&MI);
            Merge(MI, Stream_General, 0, 0);
        }
    }
}

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
int64u File_Pdf::SizeOfLine()
{
    //while (Element_Offset<Element_Size && (Buffer[Buffer_Offset+(size_t)Element_Offset]=='\r' || Buffer[Buffer_Offset+(size_t)Element_Offset]=='\n' || Buffer[Buffer_Offset+(size_t)Element_Offset]=='<' || Buffer[Buffer_Offset+(size_t)Element_Offset]=='>'))
    while (Element_Offset<Element_Size && (Buffer[Buffer_Offset+(size_t)Element_Offset]=='\r' || Buffer[Buffer_Offset+(size_t)Element_Offset]=='\n' || Buffer[Buffer_Offset+(size_t)Element_Offset]==' '))
        Element_Offset++;
    size_t End=Buffer_Offset+(size_t)Element_Offset;
    while (End<Buffer_Size && Buffer[End]!='\r' && Buffer[End]!='\n' && !(End+1<Buffer_Size && Buffer[End]=='<' && Buffer[End+1]=='<') && !(End+1<Buffer_Size && Buffer[End]=='>' && Buffer[End+1]=='>'))
        End++;
    return End-(Buffer_Offset+(size_t)Element_Offset);
}

//---------------------------------------------------------------------------
bool File_Pdf::Get_Next(string &Key, Ztring &Value)
{
    Key.clear();
    Value.clear();

    string Line;

    //Removig end of lines
    while (Element_Offset<Element_Size && (Buffer[Buffer_Offset+(size_t)Element_Offset]=='\r' || Buffer[Buffer_Offset+(size_t)Element_Offset]=='\n' || Buffer[Buffer_Offset+(size_t)Element_Offset]==' '))
        Element_Offset++;

    //End
    if (Element_Offset>=Element_Size)
        return true;

    //Testing Catalog
    Peek_String (2, Line);
    if (Line=="<<")
    {
        Element_Offset+=2;
        Catalog_Level++;
        return true;
    }
    else if (Line==">>")
    {
        Element_Offset+=2;
        Catalog_Level--;
        return true;
    }

    //Getting a complete line
    Peek_String (SizeOfLine(), Line);

    //Testing Catalog
    size_t Catalog_End=Line.find(">>");
    if (Catalog_End!=String::npos)
        Line.resize(Catalog_End);

    //Testing stream
    if (Line=="stream")
    {
        Skip_String(Line.size(),                                "Stream, Header");
        Key=Line;
        return false;
    }
    if (Line=="endstream")
    {
        Skip_String(Line.size(),                                "Stream, Footer");
        Key=Line;
        return false;
    }

    //Testing object
    if (Line=="endobj")
    {
        Skip_String(Line.size(),                                "Footer");
        Key=Line;
        return false;
    }

    //Base
    int64u Line_Base=Element_Offset;

    //Testing next key
    size_t Line_End=0;
    size_t Line_Begin=Line_End;

    // Key-Value
    if (Line_Begin<Line.size() && Line[Line_Begin]=='/')
    {
        Line_End= Line_Begin+1;
        size_t HasParenthesis=0;
        size_t HasBracket=0;
        size_t HasSpace=0;
        size_t HasValue=0;
        for (;;)
        {
            if (Line_End==Line.size())
                break;

            if (!HasParenthesis && !HasBracket && HasValue && Line[Line_End]=='<' && Line_End+1<Line.size() && Line[Line_End+1]=='<')
                break;
            if (!HasParenthesis && !HasBracket && HasValue && Line[Line_End]=='/')
                break;
            else if (!HasValue && Line[Line_End]=='/')
                ++HasValue;
            else if (!HasValue && HasSpace)
                ++HasValue;

            if (Line[Line_End]==' ')
                ++HasSpace;

            if (Line[Line_End]=='(')
                ++HasParenthesis;
            if (HasParenthesis && Line[Line_End]==')')
                --HasParenthesis;

            if (Line[Line_End]=='[')
                ++HasBracket;
            if (HasBracket && Line[Line_End]==']')
                --HasBracket;

            ++Line_End;
        }
        while(Line_End && Line[Line_End-1]==' ')
            Line_End--; //Removing trailing spaces

        Element_Offset=Line_Base+Line_Begin;
        string KeyValue;
        Get_String(Line_End-Line_Begin, KeyValue,               "Key-Value");

        size_t Key_Max=KeyValue.find_first_of(" (");
        if (Key_Max==string::npos)
            Key_Max=KeyValue.size();
        Key=KeyValue.substr(1, Key_Max-1);
        size_t Value_Min=Key_Max;
        while (Value_Min<KeyValue.size() && KeyValue[Value_Min]==' ')
            ++Value_Min;
        if (Value_Min<KeyValue.size() && KeyValue[Value_Min]=='(')
        {
            ++Value_Min;
            size_t Value_Max=KeyValue.find(')', Value_Min);
            if (Value_Max!=string::npos)
            {
                //TODO
                Value.From_UTF8(KeyValue.c_str()+Value_Min, Value_Max-Value_Min);
            }
            else
                Value.From_UTF8(KeyValue.c_str()+Value_Min);
        }
        else
                Value.From_UTF8(KeyValue.c_str()+Value_Min);
        return false;
    }

    return false;
}

} //NameSpace

#endif //MEDIAINFO_PDF_YES
