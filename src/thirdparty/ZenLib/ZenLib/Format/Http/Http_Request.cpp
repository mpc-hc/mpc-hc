/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a zlib-style license that can
 *  be found in the License.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#include "ZenLib/PreComp.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "ZenLib/Conf_Internal.h"
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "ZenLib/Format/Http/Http_Request.h"
#include "ZenLib/Format/Http/Http_Utils.h"
#include "ZenLib/Ztring.h"
using namespace std;
//---------------------------------------------------------------------------

namespace ZenLib
{

namespace Format
{

namespace Http
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
Request::Request()
{
    //Config
    Http=new ZenLib::Format::Http::Handler;
    IsCopy=false;
}

//---------------------------------------------------------------------------
Request::Request(const Request &Req)
{
    //Config
    Http=Req.Http;
    IsCopy=true;
}

//---------------------------------------------------------------------------
Request::~Request()
{
    //Config
    if (!IsCopy)
        delete Http; //Http=NULL
}

//***************************************************************************
//
//***************************************************************************

//---------------------------------------------------------------------------
bool Request::Http_Begin(std::istream &In, std::ostream &Out)
{
    //First line, "Method Path Norm"
    //-Method
    string Method;
    In>>Method;
         if (Method.size()==3 && Method[0]=='G' && Method[1]=='E' && Method[2]=='T')
        ;
    else if (Method.size()==4 && Method[0]=='P' && Method[1]=='O' && Method[2]=='S' && Method[3]=='T')
        ;
    else if (Method.size()==4 && Method[0]=='H' && Method[1]=='E' && Method[2]=='A' && Method[3]=='D')
        Http->HeadersOnly=true;
    else
    {
        Out << "HTTP/1.0 501\r\n";
        Out << "\r\n";
        return false; //Unknown request
    }
    //-Path
    In>>Http->Path;
    if (Http->Path.empty() || Http->Path[0]!='/')
    {
        Out << "HTTP/1.0 404\r\n";
        Out << "\r\n";
        return false; //Problem
    }
    if (Http->Path.find("%")!=string::npos)
        Http->Path=Format::Http::URL_Encoded_Decode(Http->Path);
    //-Norm
    string Line;
    getline(In, Line); //Drop the first line, no more needed

    //Headers
    do
    {
        //Getting the line
        getline(In, Line);
        if (!Line.empty() && Line[Line.size()-1]=='\r')
            Line.resize(Line.size()-1); //Remove the \r

        //Processing the line, form is "aaa: bbb"
        if (!Line.empty())
        {
            string::size_type Separator_Pos=Line.find(':');
            string::size_type Content_Begin=Line.find_first_not_of(' ', Separator_Pos+1);
            if (Separator_Pos!=string::npos && Content_Begin!=string::npos)
            {
                string Command=Line.substr(0, Separator_Pos);
                if (Command=="Cookie")
                {
                    string Cookie=Line.substr(Separator_Pos+1, string::npos);
                    while (!Cookie.empty())
                    {
                        string::size_type Cookie_Pos=Cookie.rfind(';');
                        if (Cookie_Pos==string::npos)
                            Cookie_Pos=(string::size_type)-1;
                        string Line2=Cookie.substr(Cookie_Pos+1, string::npos);
                        TrimLeft(Line2, ' ');
                        if (Cookie_Pos!=(string::size_type)-1)
                        {
                            Cookie.resize(Cookie_Pos);
                            TrimLeft(Line2, ' ');
                        }
                        else
                            Cookie.clear();

                        string::size_type Separator_Pos2=Line2.find('=');
                        if (Separator_Pos2!=string::npos)
                            Http->Request_Cookies[Line2.substr(0, Separator_Pos2)]=Format::Http::URL_Encoded_Decode(Line2.substr(Separator_Pos2+1, string::npos));
                    }
                }
                else
                    Http->Request_Headers[Command]=Line.substr(Content_Begin, string::npos);
            }
        }
    }
    while (!Line.empty());

    //Info
    if ((Method.size()==3 && Method[0]=='G' && Method[1]=='E' && Method[2]=='T')
     || (Method.size()==4 && Method[0]=='P' && Method[1]=='O' && Method[2]=='S' && Method[3]=='T'))
    {
        do
        {
            string Content;

            //Getting the line
            string::size_type Interogation_Pos=Http->Path.find('?');
            if (Interogation_Pos!=string::npos)
            {
                Content=Http->Path.substr(Interogation_Pos+1, string::npos);
                Http->Path.resize(Interogation_Pos);
            }

            if (Method.size()==4) //Only for POST
            {
                int64u Content_Lengh=0;
                std::map<std::string, std::string>::iterator Header_Content_Lengh_Element=Http->Request_Headers.find("Content-Length");
                if (Header_Content_Lengh_Element==Http->Request_Headers.end())
                    Header_Content_Lengh_Element=Http->Request_Headers.find("Content-length");
                if (Header_Content_Lengh_Element!=Http->Request_Headers.end())
                    #ifdef UNICODE
                        Content_Lengh=Ztring().From_UTF8(Header_Content_Lengh_Element->second).To_int64u();
                    #else
                        Content_Lengh=Ztring(Header_Content_Lengh_Element->second).To_int64u();
                    #endif
                if (Content_Lengh>1024) //Verifying there is no big element
                {
                    Out << "HTTP/1.0 501\r\n";
                    Out << "\r\n";
                    return false; //Malformed request
                }

                size_t Content_Size_Current=Content.size();
                if (Content_Size_Current)
                {
                    Content+='&';
                    Content_Size_Current++;
                }
                Content.resize(Content_Size_Current+(size_t)Content_Lengh+1);
                In.read(&Content[Content_Size_Current], (streamsize)Content_Lengh);
                Content[Content_Size_Current+(size_t)Content_Lengh]='\0';
            }

            //Processing the line, form is "aaa=bbb&..."
            while (!Content.empty())
            {
                string::size_type Content_Pos=Content.rfind('&');
                if (Content_Pos==string::npos)
                    Content_Pos=(string::size_type)-1;
                std::string Line2=Content.substr(Content_Pos+1, string::npos);
                if (Content_Pos!=(string::size_type)-1)
                    Content.resize(Content_Pos);
                else
                    Content.clear();

                string::size_type Separator_Pos=Line2.find('=');
                if (Separator_Pos!=string::npos)
                    Http->Request_Queries[Line2.substr(0, Separator_Pos)]=Format::Http::URL_Encoded_Decode(Line2.substr(Separator_Pos+1, string::npos));
            }
        }
        while (!Line.empty());
    }

    return true;
}

void Request::Http_End(std::ostream &Out)
{
    Out << "HTTP/1.0 "<< Http->Response_HTTP_Code << "\r\n";
    for (std::map<std::string, std::string>::iterator Temp=Http->Response_Headers.begin(); Temp!=Http->Response_Headers.end(); ++Temp)
        Out << Temp->first << ": " << Temp->second << "\r\n";
    Http->Response_Cookies.Create_Lines(Out);
    std::map<std::string, std::string>::iterator Content_Type_Element=Http->Response_Headers.find("Content-Type");
    if (Content_Type_Element!=Http->Response_Headers.end())
        Out << "Content-Type: "<< Content_Type_Element->second << "\r\n";
    else if (Http->Response_HTTP_Code==200)
    {
        if (!Http->Response_Body.empty() && Http->Response_Body[0]=='<')
            Out << "Content-Type: "<< "text/html; charset=utf-8" << "\r\n";
    }
    if (!Http->Response_Body.empty())
        Out << "Content-Length: " << Http->Response_Body.size() << "\r\n";
    Out << "\r\n";
    if (!Http->HeadersOnly && !Http->Response_Body.empty())
        Out << Http->Response_Body.c_str();
}

} //Namespace

} //Namespace

} //Namespace
