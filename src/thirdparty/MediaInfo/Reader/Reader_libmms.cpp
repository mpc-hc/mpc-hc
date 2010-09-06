// Reader_libmms - All info about media files
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
// Compilation conditions
#include "MediaInfo/Setup.h"
#ifdef __BORLANDC__
    #pragma hdrstop
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
    #else //MEDIAINFO_LIBMMS_FROMSOURCE
        #include "libmms/mmsx.h"
    #endif //MEDIAINFO_LIBMMS_FROMSOURCE
#endif
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
size_t Reader_libmms::Format_Test(MediaInfo_Internal* MI, const String &File_Name)
{
    //Opening the file
    mmsx_t* Handle=mmsx_connect(0, 0, Ztring(File_Name).To_Local().c_str(), (int)-1);
    if (Handle==NULL)
        return 0;

    mms_off_t Offset=mmsx_seek(0, Handle, 0, SEEK_SET);
    uint32_t Length=mmsx_get_length(Handle);

    //Buffer
    size_t Buffer_Size_Max=Buffer_NormalSize;
    int8u* Buffer=new int8u[Buffer_Size_Max];

    //Parser
    MI->Open_Buffer_Init(Length, File_Name);

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
        size_t Buffer_Size=mmsx_read(0, Handle, (char*)Buffer, (int)Buffer_Size_Max);
        if (Buffer_Size==0)
            break; //Problem while reading

        //Parser
        Status=MI->Open_Buffer_Continue(Buffer, Buffer_Size);
    }
    while (!(Status[File__Analyze::IsFinished] || (StopAfterFilled && Status[File__Analyze::IsFilled])));
    if (Length==0) //If Size==0, Status is never updated
        Status=MI->Open_Buffer_Continue(NULL, 0);

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

