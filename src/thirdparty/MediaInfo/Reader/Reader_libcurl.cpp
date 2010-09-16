// Reader_libcurl - All info about media files
// Copyright (C) 2002-2010 MediaArea.net SARL, Info@MediaArea.net
//
// This library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library. If not, see <http://www.gnu.org/licenses/>.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
// For user: you can disable or enable it
//#define MEDIAINFO_DEBUG
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Compilation conditions
#include "MediaInfo/Setup.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_LIBCURL_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#define LIBCURL_DLL_RUNTIME
#include "MediaInfo/Reader/Reader_libcurl.h"
#include "MediaInfo/File__Analyze.h"
#if defined LIBCURL_DLL_RUNTIME
    //Copy of cURL include files
    #include "MediaInfo/Reader/Reader_libcurl_Include.h"
#else
    #undef __TEXT
    #include "curl/curl.h"
#endif
using namespace ZenLib;
using namespace std;
#ifdef MEDIAINFO_DEBUG
    #include <iostream>
#endif // MEDIAINFO_DEBUG
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

Reader_libcurl::Reader_libcurl ()
{
    #if defined LIBCURL_DLL_RUNTIME
        if (libcurl_Module_Count)
            return;    
        
        size_t Errors=0;

        /* Load library */
        #ifdef MEDIAINFO_GLIBC
            libcurl_Module=g_module_open(MEDIAINFODLL_NAME, G_MODULE_BIND_LAZY);
        #elif defined (_WIN32) || defined (WIN32)
            libcurl_Module=LoadLibrary(_T(MEDIAINFODLL_NAME));
        #else
            libcurl_Module=dlopen(MEDIAINFODLL_NAME, RTLD_LAZY);
            if (!libcurl_Module)
                libcurl_Module=dlopen("./"MEDIAINFODLL_NAME, RTLD_LAZY);
            if (!libcurl_Module)
                libcurl_Module=dlopen("/usr/local/lib/"MEDIAINFODLL_NAME, RTLD_LAZY);
            if (!libcurl_Module)
                libcurl_Module=dlopen("/usr/local/lib64/"MEDIAINFODLL_NAME, RTLD_LAZY);
            if (!libcurl_Module)
                libcurl_Module=dlopen("/usr/lib/"MEDIAINFODLL_NAME, RTLD_LAZY);
            if (!libcurl_Module)
                libcurl_Module=dlopen("/usr/lib64/"MEDIAINFODLL_NAME, RTLD_LAZY);
        #endif
        if (!libcurl_Module)
            return ;

        /* Load methods */
        MEDIAINFO_ASSIGN    (init,"init")
        MEDIAINFO_ASSIGN    (setopt,"setopt")
        MEDIAINFO_ASSIGN    (perform,"perform")
        MEDIAINFO_ASSIGN    (cleanup,"cleanup")
        MEDIAINFO_ASSIGN    (getinfo,"getinfo")
        if (Errors>0)
           return;

        libcurl_Module_Count++;
    #endif //defined LIBCURL_DLL_RUNTIME
}

//***************************************************************************
// libcurl stuff
//***************************************************************************

struct curl_data
{
    MediaInfo_Internal* MI;
    CURL*               Curl;
    String              File_Name;
    int64u              File_Offset;
    int64u              File_Size;
    int64u              File_GoTo;
    bool                Init_AlreadyDone;
    #ifdef MEDIAINFO_DEBUG
        int64u          Debug_BytesRead_Total;
        int64u          Debug_BytesRead;
        int64u          Debug_Count;
    #endif // MEDIAINFO_DEBUG

    curl_data()
    {
        MI=NULL;
        Curl=NULL;
        File_Offset=0;
        File_Size=(int64u)-1;
        File_GoTo=(int64u)-1;
        Init_AlreadyDone=false;
        #ifdef MEDIAINFO_DEBUG
            Debug_BytesRead_Total=0;
            Debug_BytesRead=0;
            Debug_Count=1;
        #endif // MEDIAINFO_DEBUG
    }
};

size_t libcurl_WriteData_CallBack(void *ptr, size_t size, size_t nmemb, void *data)
{
    #ifdef MEDIAINFO_DEBUG
        ((curl_data*)data)->Debug_BytesRead_Total+=size*nmemb;
        ((curl_data*)data)->Debug_BytesRead+=size*nmemb;
    #endif //MEDIAINFO_DEBUG

    //Init
    if (!((curl_data*)data)->Init_AlreadyDone)
    {
        double File_SizeD;
        CURLcode Result=curl_easy_getinfo(((curl_data*)data)->Curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &File_SizeD);
        if (Result==CURLE_OK && File_SizeD!=-1)
            ((curl_data*)data)->MI->Open_Buffer_Init((int64u)File_SizeD, ((curl_data*)data)->File_Name);
        else
            ((curl_data*)data)->MI->Open_Buffer_Init((int64u)-1, ((curl_data*)data)->File_Name);
        ((curl_data*)data)->Init_AlreadyDone=true;
    }

    //Continue
    std::bitset<32> Result=((curl_data*)data)->MI->Open_Buffer_Continue((int8u*)ptr, size*nmemb);
    ((curl_data*)data)->File_Offset+=size*nmemb;
    
    if (Result[File__Analyze::IsFinished])
    {
        ((curl_data*)data)->MI->Open_Buffer_Finalize();
        return 0;
    }

    //GoTo
    if (((curl_data*)data)->MI->Open_Buffer_Continue_GoTo_Get()!=(int64u)-1)
    {
        if (!(((curl_data*)data)->MI->Open_Buffer_Continue_GoTo_Get()>=((curl_data*)data)->File_Offset && ((curl_data*)data)->MI->Open_Buffer_Continue_GoTo_Get()<=((curl_data*)data)->File_Offset+100000))
        {
            ((curl_data*)data)->File_GoTo=((curl_data*)data)->MI->Open_Buffer_Continue_GoTo_Get();
            return 0;
        }
    }

    //Continue parsing
    return size*nmemb;
}

//---------------------------------------------------------------------------
size_t Reader_libcurl::Format_Test(MediaInfo_Internal* MI, const String &File_Name)
{
    //Configuring
    curl_data Curl_Data;
    Curl_Data.Curl=curl_easy_init();
    Curl_Data.MI=MI;
    Curl_Data.File_Name=File_Name;
    string FileName_String=Ztring(Curl_Data.File_Name).To_Local();
    if (!MI->Config.File_Curl_Get(_T("UserAgent")).empty())
        curl_easy_setopt(Curl_Data.Curl, CURLOPT_USERAGENT, MI->Config.File_Curl_Get(_T("UserAgent")).To_Local().c_str());
    if (!MI->Config.File_Curl_Get(_T("Proxy")).empty())
        curl_easy_setopt(Curl_Data.Curl, CURLOPT_PROXY, MI->Config.File_Curl_Get(_T("Proxy")).To_Local().c_str());
    curl_easy_setopt(Curl_Data.Curl, CURLOPT_URL, FileName_String.c_str());
    curl_easy_setopt(Curl_Data.Curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(Curl_Data.Curl, CURLOPT_MAXREDIRS, 3);
    curl_easy_setopt(Curl_Data.Curl, CURLOPT_WRITEFUNCTION, &libcurl_WriteData_CallBack);
    curl_easy_setopt(Curl_Data.Curl, CURLOPT_WRITEDATA, &Curl_Data);
    
    //Parsing
    CURLcode Result;
    do
    {
        //GoTo
        if (Curl_Data.File_GoTo!=(int64u)-1)
        {
            #ifdef MEDIAINFO_DEBUG
                std::cout<<std::hex<<Curl_Data.File_Offset-Curl_Data.Debug_BytesRead<<" - "<<Curl_Data.File_Offset<<" : "<<std::dec<<Curl_Data.Debug_BytesRead<<" bytes"<<std::endl;
                Curl_Data.File_Offset=Curl_Data.File_GoTo;
                Curl_Data.Debug_BytesRead=0;
                Curl_Data.Debug_Count++;
            #endif //MEDIAINFO_DEBUG

                if (Curl_Data.File_GoTo<0x80000000)
            {
                //We do NOT use large version if we can, because some version (tested: 7.15 linux) do NOT like large version (error code 18)
                long File_GoTo_Long=(long)Curl_Data.File_GoTo;
                curl_easy_setopt(Curl_Data.Curl, CURLOPT_RESUME_FROM, File_GoTo_Long);
            }
            else
            {
                curl_off_t File_GoTo_Off=(curl_off_t)Curl_Data.File_GoTo;
                curl_easy_setopt(Curl_Data.Curl, CURLOPT_RESUME_FROM_LARGE, File_GoTo_Off);
            }
            MI->Open_Buffer_Init((int64u)-1, Curl_Data.File_GoTo);
            Curl_Data.File_GoTo=(int64u)-1;
        }

        //Parsing
        Result=curl_easy_perform(Curl_Data.Curl);
    }
    while (Result==CURLE_WRITE_ERROR && Curl_Data.File_GoTo!=(int64u)-1);

    #ifdef MEDIAINFO_DEBUG
        std::cout<<std::hex<<Curl_Data.File_Offset-Curl_Data.Debug_BytesRead<<" - "<<Curl_Data.File_Offset<<" : "<<std::dec<<Curl_Data.Debug_BytesRead<<" bytes"<<std::endl;
        std::cout<<"Total: "<<std::dec<<Curl_Data.Debug_BytesRead_Total<<" bytes in "<<Curl_Data.Debug_Count<<" blocks"<<std::endl;
    #endif //MEDIAINFO_DEBUG

    MI->Open_Buffer_Finalize();

    //Cleanup
    curl_easy_cleanup(Curl_Data.Curl);
    return 1;
}

} //NameSpace

#endif //MEDIAINFO_LIBCURL_YES

