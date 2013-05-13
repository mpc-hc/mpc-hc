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
// Config
#ifndef MEDIAINFO_LIBMMS_DESCRIBE_SUPPORT //If not defined by the compiler
    #define MEDIAINFO_LIBMMS_DESCRIBE_SUPPORT 0 //0=without, 1=with libmms customized version containing DESCRIBE only API
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_LIBMMS_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Reader/Reader_libmms.h"
#include "MediaInfo/File__Analyze.h"
#if defined LIBMMS_DLL_RUNTIME
#elif defined LIBMMS_DLL_STATIC
#else
    #ifdef MEDIAINFO_LIBMMS_FROMSOURCE
        #include "mmsx.h"
        #include "mmsh.h"
    #else //MEDIAINFO_LIBMMS_FROMSOURCE
        #include "libmms/mmsx.h"
        #include "libmms/mmsh.h"
    #endif //MEDIAINFO_LIBMMS_FROMSOURCE
#endif
#include <iostream>
using namespace ZenLib;
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

const size_t Buffer_NormalSize=64*1024;

//***************************************************************************
// libmms stuff
//***************************************************************************

//---------------------------------------------------------------------------
size_t Reader_libmms::Format_Test(MediaInfo_Internal* MI, String File_Name)
{
    mmsx_t* Handle;

    //Opening the file
    #if MEDIAINFO_LIBMMS_DESCRIBE_SUPPORT
    if (MI->Config.File_Mmsh_Describe_Only_Get())
    {
        // Use MMSH & Send a DESCRIBE request
        mmsh_t* MmshHandle;

        MmshHandle=mmsh_describe_request(0, 0, Ztring(File_Name).To_Local().c_str());
        if (MmshHandle==NULL)
            return 0;

        Handle=mmsx_set_mmsh_handle(MmshHandle);
        if (Handle==NULL)
        {
            mmsh_close(MmshHandle);
            return 0;
        }
    }
    else
    #endif //MEDIAINFO_LIBMMS_DESCRIBE_SUPPORT
    {
       // Use MMS or MMSH (Send a DESCRIBE & PLAY request)
       Handle=mmsx_connect(0, 0, Ztring(File_Name).To_Local().c_str(), (int)-1);
       if (Handle==NULL)
           return 0;
    }

    //Init
    size_t Buffer_Size_Max;
    uint32_t Length;
    if (!MI->Config.File_Mmsh_Describe_Only_Get())
    {
        //Buffer
        Buffer_Size_Max=Buffer_NormalSize;

        //MediaInfo init
        mms_off_t Offset=mmsx_seek(0, Handle, 0, SEEK_SET);
        uint32_t Length=mmsx_get_length(Handle);
        MI->Open_Buffer_Init(Length, File_Name);
    }
    else
    {
        //Buffer
        Buffer_Size_Max=mmsx_get_asf_header_len(Handle);

        //MediaInfo init
        Length=(uint32_t)-1;
        MI->Open_Buffer_Init((int64u)-1, File_Name);
    }
    int8u* Buffer=new int8u[Buffer_Size_Max];

    //Test the format with buffer
    bool StopAfterFilled=MI->Config.File_StopAfterFilled_Get();
    std::bitset<32> Status;
    do
    {
        //Seek (if needed)
        if (MI->Open_Buffer_Continue_GoTo_Get()!=(int64u)-1)
        {
            if (MI->Open_Buffer_Continue_GoTo_Get()>=Length)
                break; //Seek requested, but on a file bigger in theory than what is in the real file, we can't do this
            if (mmsx_seek(0, Handle, mms_off_t(MI->Open_Buffer_Continue_GoTo_Get()), SEEK_SET)!=MI->Open_Buffer_Continue_GoTo_Get())
                break; //File is not seekable

            MI->Open_Buffer_Init((int64u)-1, MI->Open_Buffer_Continue_GoTo_Get());
        }

        //Buffering
        size_t Buffer_Size;
        if (!MI->Config.File_Mmsh_Describe_Only_Get())
            Buffer_Size=mmsx_read(0, Handle, (char*)Buffer, (int)Buffer_Size_Max);
        else
            Buffer_Size=mmsx_peek_header(Handle, (char*)Buffer, (int)Buffer_Size_Max);

        //Parser
        Status=MI->Open_Buffer_Continue(Buffer, Buffer_Size);
        if (Buffer_Size==0 || MI->Config.File_Mmsh_Describe_Only_Get())
            break;
    }
    while (!(Status[File__Analyze::IsFinished] || (StopAfterFilled && Status[File__Analyze::IsFilled])));

    //File
    mmsx_close(Handle);

    //Buffer
    delete[] Buffer; //Buffer=NULL;

    //Is this file detected?
    if (!Status[File__Analyze::IsAccepted])
        return 0;

    MI->Open_Buffer_Finalize();

    return 1;
}

} //NameSpace

#endif //MEDIAINFO_LIBMMS_YES

