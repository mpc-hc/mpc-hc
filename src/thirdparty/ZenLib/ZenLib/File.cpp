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
#ifdef ZENLIB_USEWX
    #include <wx/file.h>
    #include <wx/filename.h>
    #include <wx/utils.h>
#else //ZENLIB_USEWX
    #ifdef ZENLIB_STANDARD
        #ifdef WINDOWS
        #else
            #include <cstdio>
        #endif
        #include <sys/stat.h>
        #if !defined(WINDOWS)
            #include <unistd.h>
        #endif //!defined(WINDOWS)
        #include <fstream>
        using namespace std;
        #ifndef S_ISDIR
            #define S_ISDIR(mode) (((mode)&S_IFMT) == S_IFDIR)
        #endif
        #ifndef S_ISREG
            #define S_ISREG(mode) (((mode)&S_IFMT) == S_IFREG)
        #endif
    #elif defined WINDOWS
        #undef __TEXT
        #include <windows.h>
    #endif
#endif //ZENLIB_USEWX
#include "ZenLib/File.h"
#include "ZenLib/OS_Utils.h"
#include <map>
//---------------------------------------------------------------------------

namespace ZenLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File::File()
{
    #ifdef ZENLIB_USEWX
        File_Handle=NULL;
    #else //ZENLIB_USEWX
        #ifdef ZENLIB_STANDARD
            File_Handle=NULL;
        #elif defined WINDOWS
            File_Handle=INVALID_HANDLE_VALUE;
        #endif
    #endif //ZENLIB_USEWX
    Position=(int64u)-1;
    Size=(int64u)-1;
}

File::File(Ztring File_Name, access_t Access)
{
    #ifdef ZENLIB_USEWX
        File_Handle=NULL;
    #else //ZENLIB_USEWX
        #ifdef ZENLIB_STANDARD
            File_Handle=NULL;
        #elif defined WINDOWS
            File_Handle=INVALID_HANDLE_VALUE;
        #endif
    #endif //ZENLIB_USEWX
    Position=(int64u)-1;
    Size=(int64u)-1;
    Open(File_Name, Access);
}

//---------------------------------------------------------------------------
File::~File()
{
    Close();
}

//***************************************************************************
// Open/Close
//***************************************************************************

//---------------------------------------------------------------------------
bool File::Open (const tstring &File_Name_, access_t Access)
{
    Close();

    File_Name=File_Name_;

    #ifdef ZENLIB_USEWX
        File_Handle=(void*)new wxFile();
        if (((wxFile*)File_Handle)->Open(File_Name.c_str(), (wxFile::OpenMode)Access)==0)
        {
            //Sometime the file is locked for few milliseconds, we try again later
            wxMilliSleep(1000);
            if (((wxFile*)File_Handle)->Open(File_Name.c_str(), (wxFile::OpenMode)Access)==0)
                //File is not openable
                return false;
        }
        return true;
    #else //ZENLIB_USEWX
        #ifdef ZENLIB_STANDARD
            /*
            int access;
            switch (Access)
            {
                case Access_Read         : access=O_BINARY|O_RDONLY          ; break;
                case Access_Write        : access=O_BINARY|O_WRONLY|O_CREAT|O_TRUNC  ; break;
                case Access_Read_Write   : access=O_BINARY|O_RDWR  |O_CREAT  ; break;
                case Access_Write_Append : access=O_BINARY|O_WRONLY|O_CREAT|O_APPEND ; break;
                default                  : access=0                          ; break;
            }
            #ifdef UNICODE
                File_Handle=open(File_Name.To_Local().c_str(), access);
            #else
                File_Handle=open(File_Name.c_str(), access);
            #endif //UNICODE
            return File_Handle!=-1;
            */
            ios_base::openmode mode;
            switch (Access)
            {
                case Access_Read         : mode=ios_base::binary|ios_base::in; break;
                case Access_Write        : mode=ios_base::binary|ios_base::in|ios_base::out; break;
                case Access_Read_Write   : mode=ios_base::binary|ios_base::in|ios_base::out; break;
                case Access_Write_Append : if (!Exists(File_Name))
                                                mode=ios_base::binary|ios_base::out;
                                           else
                                                mode=ios_base::binary|ios_base::out|ios_base::app;
                                           break;
                default                  : ;
            }
            #ifdef UNICODE
                File_Handle=new fstream(File_Name.To_Local().c_str(), mode);
            #else
                File_Handle=new fstream(File_Name.c_str(), mode);
            #endif //UNICODE
            if (!((fstream*)File_Handle)->is_open())
            {
                delete (fstream*)File_Handle; File_Handle=NULL;
                return false;
            }
            return true;
        #elif defined WINDOWS
            DWORD dwDesiredAccess, dwShareMode, dwCreationDisposition;
            switch (Access)
            {
                #ifndef ZENLIB_NO_WIN9X_SUPPORT
                case Access_Read         : dwDesiredAccess=IsWin9X_Fast()?GENERIC_READ:FILE_READ_DATA; dwShareMode=FILE_SHARE_READ|FILE_SHARE_WRITE; dwCreationDisposition=OPEN_EXISTING; break;
                case Access_Write        : dwDesiredAccess=GENERIC_WRITE;   dwShareMode=0;                                 dwCreationDisposition=OPEN_ALWAYS;   break;
                case Access_Read_Write   : dwDesiredAccess=(IsWin9X_Fast()?GENERIC_READ:FILE_READ_DATA)|GENERIC_WRITE;   dwShareMode=0; dwCreationDisposition=OPEN_ALWAYS;   break;
                case Access_Write_Append : dwDesiredAccess=GENERIC_WRITE;   dwShareMode=0;                                 dwCreationDisposition=OPEN_ALWAYS;   break;
                #else // ZENLIB_NO_WIN9X_SUPPORT
                case Access_Read         : dwDesiredAccess=FILE_READ_DATA; dwShareMode=FILE_SHARE_READ|FILE_SHARE_WRITE; dwCreationDisposition=OPEN_EXISTING; break;
                case Access_Write        : dwDesiredAccess=GENERIC_WRITE;   dwShareMode=0;                                 dwCreationDisposition=OPEN_ALWAYS;   break;
                case Access_Read_Write   : dwDesiredAccess=FILE_READ_DATA|GENERIC_WRITE;   dwShareMode=0; dwCreationDisposition=OPEN_ALWAYS;   break;
                case Access_Write_Append : dwDesiredAccess=GENERIC_WRITE;   dwShareMode=0;                                 dwCreationDisposition=OPEN_ALWAYS;   break;
                #endif //ZENLIB_NO_WIN9X_SUPPORT
                default                  : dwDesiredAccess=0;               dwShareMode=0;                                 dwCreationDisposition=0;             break;
            }

            #ifdef UNICODE
                #ifndef ZENLIB_NO_WIN9X_SUPPORT
                if (IsWin9X_Fast())
                    File_Handle=CreateFileA(File_Name.To_Local().c_str(), dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, 0, NULL);
                else
                #endif //ZENLIB_NO_WIN9X_SUPPORT
                    File_Handle=CreateFileW(File_Name.c_str(), dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, 0, NULL);
            #else
                File_Handle=CreateFile(File_Name.c_str(), dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, 0, NULL);
            #endif //UNICODE
            if (File_Handle==INVALID_HANDLE_VALUE)
            {
                //Sometime the file is locked for few milliseconds, we try again later
                DWORD dw = GetLastError();
                if (dw!=ERROR_FILE_NOT_FOUND)
                {
                    /*
                    char lpMsgBuf[1000];
                    FormatMessageA(
                    FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    dw,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    lpMsgBuf,
                    1000, NULL );
                    */
                    Sleep(1000);
                    #ifdef UNICODE
                        #ifndef ZENLIB_NO_WIN9X_SUPPORT
                        if (IsWin9X_Fast())
                            File_Handle=CreateFileA(Ztring(File_Name).To_Local().c_str(), dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, 0, NULL);
                        else
                        #endif //ZENLIB_NO_WIN9X_SUPPORT
                            File_Handle=CreateFileW(File_Name.c_str(), dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, 0, NULL);
                    #else
                        File_Handle=CreateFile(File_Name.c_str(), dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, 0, NULL);
                    #endif //UNICODE
                }
            }
            if (File_Handle==INVALID_HANDLE_VALUE)
                //File is not openable
                return false;

            //Append
            if (Access==Access_Write_Append)
                GoTo(0, FromEnd);

            return true;
        #endif
    #endif //ZENLIB_USEWX
}

//---------------------------------------------------------------------------
bool File::Create (const Ztring &File_Name_, bool OverWrite)
{
    Close();

    File_Name=File_Name_;

    #ifdef ZENLIB_USEWX
        File_Handle=(void*)new wxFile();
        if (((wxFile*)File_Handle)->Create(File_Name.c_str(), OverWrite)==0)
        {
            //Sometime the file is locked for few milliseconds, we try again later
            wxMilliSleep(3000);
            if (((wxFile*)File_Handle)->Create(File_Name.c_str(), OverWrite)==0)
                //File is not openable
                return false;
        }
        return true;
    #else //ZENLIB_USEWX
        #ifdef ZENLIB_STANDARD
            /*
            int access;
            switch (OverWrite)
            {
                case false        : access=O_BINARY|O_CREAT|O_WRONLY|O_EXCL ; break;
                default           : access=O_BINARY|O_CREAT|O_WRONLY|O_TRUNC; break;
            }
            #ifdef UNICODE
                File_Handle=open(File_Name.To_Local().c_str(), access);
            #else
                File_Handle=open(File_Name.c_str(), access);
            #endif //UNICODE
            return  File_Handle!=-1;
            */
            /*ios_base::openmode mode;

            switch (OverWrite)
            {
                //case false         : mode=          ; break;
                default                  : mode=0                            ; break;
            }*/
            ios_base::openmode access;
            switch (OverWrite)
            {
                case false        : if (Exists(File_Name))
                                        return false;
                default           : access=ios_base::binary|ios_base::in|ios_base::out|ios_base::trunc; break;
            }
            #ifdef UNICODE
                File_Handle=new fstream(File_Name.To_Local().c_str(), access);
            #else
                File_Handle=new fstream(File_Name.c_str(), access);
            #endif //UNICODE
            return ((fstream*)File_Handle)->is_open();
        #elif defined WINDOWS
            DWORD dwDesiredAccess, dwShareMode, dwCreationDisposition;
            switch (OverWrite)
            {
                case false        : dwDesiredAccess=GENERIC_WRITE; dwShareMode=0;                dwCreationDisposition=CREATE_NEW;    break;
                default           : dwDesiredAccess=GENERIC_WRITE; dwShareMode=0;                dwCreationDisposition=CREATE_ALWAYS; break;
            }

            #ifdef UNICODE
                #ifndef ZENLIB_NO_WIN9X_SUPPORT
                if (IsWin9X_Fast())
                    File_Handle=CreateFileA(File_Name.To_Local().c_str(), dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, 0, NULL);
                else
                #endif //ZENLIB_NO_WIN9X_SUPPORT
                    File_Handle=CreateFileW(File_Name.c_str(), dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, 0, NULL);
            #else
                File_Handle=CreateFile(File_Name.c_str(), dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, 0, NULL);
            #endif //UNICODE
            if (File_Handle==INVALID_HANDLE_VALUE)
            {
                //Sometime the file is locked for few milliseconds, we try again later
                Sleep(3000);
                #ifdef UNICODE
                    #ifndef ZENLIB_NO_WIN9X_SUPPORT
                    if (IsWin9X_Fast())
                        File_Handle=CreateFileA(File_Name.To_Local().c_str(), dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, 0, NULL);
                    else
                    #endif //ZENLIB_NO_WIN9X_SUPPORT
                        File_Handle=CreateFileW(File_Name.c_str(), dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, 0, NULL);
                #else
                    File_Handle=CreateFile(File_Name.c_str(), dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, 0, NULL);
                #endif //UNICODE
            }
            if (File_Handle==INVALID_HANDLE_VALUE)
                //File is not openable
                return false;
            return true;
        #endif
    #endif //ZENLIB_USEWX
}

//---------------------------------------------------------------------------
void File::Close ()
{
    #ifdef ZENLIB_USEWX
        delete (wxFile*)File_Handle; File_Handle=NULL;
    #else //ZENLIB_USEWX
        #ifdef ZENLIB_STANDARD
            //close(File_Handle); File_Handle=-1;
            delete (fstream*)File_Handle; File_Handle=NULL;
        #elif defined WINDOWS
            CloseHandle(File_Handle); File_Handle=INVALID_HANDLE_VALUE;
        #endif
    #endif //ZENLIB_USEWX
    Position=(int64u)-1;
    Size=(int64u)-1;
}

//***************************************************************************
// Read/Write
//***************************************************************************

//---------------------------------------------------------------------------
size_t File::Read (int8u* Buffer, size_t Buffer_Size_Max)
{
    #ifdef ZENLIB_USEWX
        if (File_Handle==NULL)
    #else //ZENLIB_USEWX
        #ifdef ZENLIB_STANDARD
            if (File_Handle==NULL)
        #elif defined WINDOWS
            if (File_Handle==INVALID_HANDLE_VALUE)
        #endif
    #endif //ZENLIB_USEWX
        return 0;

    #ifdef ZENLIB_USEWX
        size_t ByteRead=((wxFile*)File_Handle)->Read(Buffer, Buffer_Size_Max);
        Position+=ByteRead;
        return ByteRead;
    #else //ZENLIB_USEWX
        #ifdef ZENLIB_STANDARD
            //return read((int)File_Handle, Buffer, Buffer_Size_Max);
            if (Position==(int64u)-1)
                Position_Get();
            if (Size==(int64u)-1)
                Size_Get();
            if (Position!=(int64u)-1 && Position+Buffer_Size_Max>Size)
                Buffer_Size_Max=(size_t)(Size-Position); //We don't want to enable eofbit (impossible to seek after)
            ((fstream*)File_Handle)->read((char*)Buffer, Buffer_Size_Max);
            size_t ByteRead=((fstream*)File_Handle)->gcount();
            Position+=ByteRead;
            return ByteRead;
        #elif defined WINDOWS
            DWORD Buffer_Size;
            if (ReadFile(File_Handle, Buffer, (DWORD)Buffer_Size_Max, &Buffer_Size, NULL))
            {
                Position+=Buffer_Size;
                return Buffer_Size;
            }
            else
                return 0;
        #endif
    #endif //ZENLIB_USEWX
}

//---------------------------------------------------------------------------
size_t File::Write (const int8u* Buffer, size_t Buffer_Size)
{
    #ifdef ZENLIB_USEWX
        if (File_Handle==NULL)
    #else //ZENLIB_USEWX
        #ifdef ZENLIB_STANDARD
            if (File_Handle==NULL)
        #elif defined WINDOWS
            if (File_Handle==INVALID_HANDLE_VALUE)
        #endif
    #endif //ZENLIB_USEWX
        return 0;

    #ifdef ZENLIB_USEWX
        return ((wxFile*)File_Handle)->Write(Buffer, Buffer_Size);
    #else //ZENLIB_USEWX
        #ifdef ZENLIB_STANDARD
            //return write(File_Handle, Buffer, Buffer_Size);
            ((fstream*)File_Handle)->write((char*)Buffer, Buffer_Size);
            if (((fstream*)File_Handle)->bad())
            {
                Position=(int64u)-1;
                return 0;
            }
            else
            {
                if (Position!=(int64u)-1)
                    Position+=Buffer_Size;
                return Buffer_Size;
            }
        #elif defined WINDOWS
            DWORD Buffer_Size_Written;
            if (WriteFile(File_Handle, Buffer, (DWORD)Buffer_Size, &Buffer_Size_Written, NULL))
            {
                if (Position!=(int64u)-1)
                    Position+=Buffer_Size_Written;
                return Buffer_Size_Written;
            }
            else
            {
                Position=(int64u)-1;
                return 0;
            }
        #endif
    #endif //ZENLIB_USEWX
}

//---------------------------------------------------------------------------
bool File::Truncate (int64u Offset)
{
    if (File_Handle==NULL)
        return false;

    #ifdef ZENLIB_USEWX
        return false; Not supported
    #else //ZENLIB_USEWX
        #ifdef ZENLIB_STANDARD
            #if defined(WINDOWS)
                return false; //No supported
            #else //defined(WINDOWS)
                //Need to close the file, use truncate, reopen it
                if (Offset==(int64u)-1)
                    Offset=Position_Get();
                Ztring File_Name_Sav=File_Name;
                Close();
                truncate(File_Name_Sav.To_Local().c_str(), Offset);
                if (!Open(File_Name_Sav, Access_Read_Write))
                    return false;
                GoTo(0, FromEnd);
                return true;
            #endif //!defined(WINDOWS)
        #elif defined WINDOWS
            if(Offset!=(int64u)-1 && Offset!=Position_Get())
                if (!GoTo(Offset))
                    return false;
            SetEndOfFile(File_Handle);
            return true;
        #endif
    #endif //ZENLIB_USEWX
}

//---------------------------------------------------------------------------
size_t File::Write (const Ztring &ToWrite)
{
    std::string AnsiString=ToWrite.To_UTF8();
    return Write((const int8u*)AnsiString.c_str(), AnsiString.size());
}

//***************************************************************************
// Moving
//***************************************************************************

//---------------------------------------------------------------------------
bool File::GoTo (int64s Position_ToMove, move_t MoveMethod)
{
    #ifdef ZENLIB_USEWX
        if (File_Handle==NULL)
    #else //ZENLIB_USEWX
        #ifdef ZENLIB_STANDARD
            if (File_Handle==NULL)
        #elif defined WINDOWS
            if (File_Handle==INVALID_HANDLE_VALUE)
        #endif
    #endif //ZENLIB_USEWX
        return false;

    Position=(int64u)-1; //Disabling memory
    #ifdef ZENLIB_USEWX
        return ((wxFile*)File_Handle)->Seek(Position, (wxSeekMode)MoveMethod)!=wxInvalidOffset; //move_t and wxSeekMode are same
    #else //ZENLIB_USEWX
        #ifdef ZENLIB_STANDARD
            /*
            int fromwhere;
            switch (MoveMethod)
            {
                case FromBegin : fromwhere=SEEK_SET; break;
                case FromCurrent : fromwhere=SEEK_CUR; break;
                case FromEnd : fromwhere=SEEK_END; break;
                default : fromwhere=SEEK_CUR; break;
            }
            return lseek(File_Handle, Position, fromwhere)!=-1;
            */
            ios_base::seekdir dir;
            switch (MoveMethod)
            {
                case FromBegin   : dir=ios_base::beg; break;
                case FromCurrent : dir=ios_base::cur; break;
                case FromEnd     : dir=ios_base::end; break;
                default          : dir=ios_base::beg;
            }
            ((fstream*)File_Handle)->seekg((streamoff)Position_ToMove, dir);
            return !((fstream*)File_Handle)->fail();
        #elif defined WINDOWS
            LARGE_INTEGER GoTo; GoTo.QuadPart=Position_ToMove;
            //return SetFilePointerEx(File_Handle, GoTo, NULL, MoveMethod)!=0; //Not on win9X
            GoTo.LowPart=SetFilePointer(File_Handle, GoTo.LowPart, &GoTo.HighPart, MoveMethod);
            if (GoTo.LowPart==INVALID_SET_FILE_POINTER && GetLastError()!=NO_ERROR)
                return false;
            else
                return true;
        #endif
    #endif //ZENLIB_USEWX
}

//---------------------------------------------------------------------------
int64u File::Position_Get ()
{
    if (Position!=(int64u)-1)
        return Position;

    #ifdef ZENLIB_USEWX
        if (File_Handle==NULL)
    #else //ZENLIB_USEWX
        #ifdef ZENLIB_STANDARD
            if (File_Handle==NULL)
        #elif defined WINDOWS
            if (File_Handle==INVALID_HANDLE_VALUE)
        #endif
    #endif //ZENLIB_USEWX
        return (int64u)-1;

    #ifdef ZENLIB_USEWX
        return (int64u)-1;
    #else //ZENLIB_USEWX
        #ifdef ZENLIB_STANDARD
            Position=((fstream*)File_Handle)->tellg();
            return Position;
        #elif defined WINDOWS
            LARGE_INTEGER GoTo; GoTo.QuadPart=0;
            GoTo.LowPart=SetFilePointer(File_Handle, GoTo.LowPart, &GoTo.HighPart, FILE_CURRENT);
            Position=GoTo.QuadPart;
            return Position;
        #endif
    #endif //ZENLIB_USEWX
}

//***************************************************************************
// Attributes
//***************************************************************************

//---------------------------------------------------------------------------
int64u File::Size_Get()
{
    #ifdef ZENLIB_USEWX
        if (File_Handle==NULL)
    #else //ZENLIB_USEWX
        #ifdef ZENLIB_STANDARD
            if (File_Handle==NULL)
        #elif defined WINDOWS
            if (File_Handle==INVALID_HANDLE_VALUE)
        #endif
    #endif //ZENLIB_USEWX
        return 0;

    #ifdef ZENLIB_USEWX
        return ((wxFile*)File_Handle)->Length();
    #else //ZENLIB_USEWX
        #ifdef ZENLIB_STANDARD
            /*
            int CurrentPos=lseek(File_Handle, 0, SEEK_CUR);
            int64u File_Size;
            File_Size=lseek(File_Handle, 0, SEEK_END);
            lseek(File_Handle, CurrentPos, SEEK_SET);
            */
            fstream::pos_type CurrentPos=((fstream*)File_Handle)->tellg();
            if (CurrentPos!=(fstream::pos_type)-1)
            {
                ((fstream*)File_Handle)->seekg(0, ios_base::end);
                Size=((fstream*)File_Handle)->tellg();
                ((fstream*)File_Handle)->seekg(CurrentPos);
            }
            else
                Size=(int64u)-1;
        #elif defined WINDOWS
            #ifdef ZENLIB_NO_WIN9X_SUPPORT
                LARGE_INTEGER x = {0};
                BOOL bRet = ::GetFileSizeEx(File_Handle, &x);
                if (bRet == FALSE)
                    return (int64u)-1;
                Size=x.QuadPart;
            #else
                DWORD High;DWORD Low=GetFileSize(File_Handle, &High);
                if (Low==INVALID_FILE_SIZE && GetLastError()!=NO_ERROR)
                    return (int64u)-1;
                Size=0x100000000ULL*High+Low;
            #endif //ZENLIB_NO_WIN9X_SUPPORT
        #endif
        return Size;
    #endif //ZENLIB_USEWX
}

//---------------------------------------------------------------------------
Ztring File::Created_Get()
{
    #ifdef ZENLIB_USEWX
        if (File_Handle==NULL)
    #else //ZENLIB_USEWX
        #ifdef ZENLIB_STANDARD
            if (File_Handle==NULL)
        #elif defined WINDOWS
            if (File_Handle==INVALID_HANDLE_VALUE)
        #endif
    #endif //ZENLIB_USEWX
        return Ztring();

    #ifdef ZENLIB_USEWX
        return __T(""); //Not implemented
    #else //ZENLIB_USEWX
        #ifdef ZENLIB_STANDARD
            return __T(""); //Not implemented
        #elif defined WINDOWS
            FILETIME TimeFT;
            if (GetFileTime(File_Handle, &TimeFT, NULL, NULL))
            {
                int64u Time64=0x100000000ULL*TimeFT.dwHighDateTime+TimeFT.dwLowDateTime;
                Ztring Time; Time.Date_From_Milliseconds_1601(Time64/10000);
                return Time;
            }
            else
                return __T(""); //There was a problem
        #endif
    #endif //ZENLIB_USEWX
}

//---------------------------------------------------------------------------
Ztring File::Created_Local_Get()
{
    #ifdef ZENLIB_USEWX
        if (File_Handle==NULL)
    #else //ZENLIB_USEWX
        #ifdef ZENLIB_STANDARD
            if (File_Handle==NULL)
        #elif defined WINDOWS
            if (File_Handle==INVALID_HANDLE_VALUE)
        #endif
    #endif //ZENLIB_USEWX
        return Ztring();

    #ifdef ZENLIB_USEWX
        return __T(""); //Not implemented
    #else //ZENLIB_USEWX
        #ifdef ZENLIB_STANDARD
            return __T(""); //Not implemented
        #elif defined WINDOWS
            FILETIME TimeFT;
            if (GetFileTime(File_Handle, &TimeFT, NULL, NULL))
            {
                int64u Time64=0x100000000ULL*TimeFT.dwHighDateTime+TimeFT.dwLowDateTime;
                TIME_ZONE_INFORMATION Info;
                DWORD Result=GetTimeZoneInformation(&Info);
                if (Result!=TIME_ZONE_ID_INVALID)
                {
                    Time64-=((int64s)Info.Bias)*60*1000*1000*10;
                    if (Result==TIME_ZONE_ID_DAYLIGHT)
                        Time64-=((int64s)Info.DaylightBias)*60*1000*1000*10;
                    else
                        Time64-=((int64s)Info.StandardBias)*60*1000*1000*10;
                }
                Ztring Time; Time.Date_From_Milliseconds_1601(Time64/10000);
                Time.FindAndReplace(__T("UTC "), __T(""));
                return Time;
            }
            else
                return __T(""); //There was a problem
        #endif
    #endif //ZENLIB_USEWX
}

//---------------------------------------------------------------------------
Ztring File::Modified_Get()
{
    #ifdef ZENLIB_USEWX
        if (File_Handle==NULL)
    #else //ZENLIB_USEWX
        #ifdef ZENLIB_STANDARD
            if (File_Handle==NULL)
        #elif defined WINDOWS
            if (File_Handle==INVALID_HANDLE_VALUE)
        #endif
    #endif //ZENLIB_USEWX
        return Ztring();

    #ifdef ZENLIB_USEWX
        return __T(""); //Not implemented
    #else //ZENLIB_USEWX
        #ifdef ZENLIB_STANDARD
            struct stat Stat;
            int Result=stat(File_Name.To_Local().c_str(), &Stat);
            if (Result<0)
                return __T(""); //Error
            Ztring Time; Time.Date_From_Seconds_1970(Stat.st_mtime);
            return Time;
        #elif defined WINDOWS
            FILETIME TimeFT;
            if (GetFileTime(File_Handle, NULL, NULL, &TimeFT))
            {
                int64u Time64=0x100000000ULL*TimeFT.dwHighDateTime+TimeFT.dwLowDateTime;
                Ztring Time; Time.Date_From_Milliseconds_1601(Time64/10000);
                return Time;
            }
            else
                return __T(""); //There was a problem
        #endif
    #endif //ZENLIB_USEWX
}

//---------------------------------------------------------------------------
Ztring File::Modified_Local_Get()
{
    #ifdef ZENLIB_USEWX
        if (File_Handle==NULL)
    #else //ZENLIB_USEWX
        #ifdef ZENLIB_STANDARD
            if (File_Handle==NULL)
        #elif defined WINDOWS
            if (File_Handle==INVALID_HANDLE_VALUE)
        #endif
    #endif //ZENLIB_USEWX
        return Ztring();

    #ifdef ZENLIB_USEWX
        return __T(""); //Not implemented
    #else //ZENLIB_USEWX
        #ifdef ZENLIB_STANDARD
            struct stat Stat;
            int Result=stat(File_Name.To_Local().c_str(), &Stat);
            if (Result<0)
                return __T(""); //Error
            Ztring Time; Time.Date_From_Seconds_1970_Local(Stat.st_mtime);
            return Time;
        #elif defined WINDOWS
            FILETIME TimeFT;
            if (GetFileTime(File_Handle, NULL, NULL, &TimeFT))
            {
                int64u Time64=0x100000000ULL*TimeFT.dwHighDateTime+TimeFT.dwLowDateTime; //100-ns
                TIME_ZONE_INFORMATION Info;
                DWORD Result=GetTimeZoneInformation(&Info);
                if (Result!=TIME_ZONE_ID_INVALID)
                {
                    Time64-=((int64s)Info.Bias)*60*1000*1000*10;
                    if (Result==TIME_ZONE_ID_DAYLIGHT)
                        Time64-=((int64s)Info.DaylightBias)*60*1000*1000*10;
                    else
                        Time64-=((int64s)Info.StandardBias)*60*1000*1000*10;
                }
                Ztring Time; Time.Date_From_Milliseconds_1601(Time64/10000);
                Time.FindAndReplace(__T("UTC "), __T(""));
                return Time;
            }
            else
                return __T(""); //There was a problem
        #endif
    #endif //ZENLIB_USEWX
}

//---------------------------------------------------------------------------
bool File::Opened_Get()
{
    #ifdef ZENLIB_USEWX
        return File_Handle!=NULL;
    #else //ZENLIB_USEWX
        #ifdef ZENLIB_STANDARD
            //return File_Handle!=-1;
            return File_Handle!=NULL && ((fstream*)File_Handle)->is_open();
        #elif defined WINDOWS
            return File_Handle!=INVALID_HANDLE_VALUE;
        #endif
    #endif //ZENLIB_USEWX
}

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
int64u File::Size_Get(const Ztring &File_Name)
{
    File F(File_Name);
    return F.Size_Get();
}

//---------------------------------------------------------------------------
Ztring File::Created_Get(const Ztring &File_Name)
{
    File F(File_Name);
    return F.Created_Get();
}

//---------------------------------------------------------------------------
Ztring File::Modified_Get(const Ztring &File_Name)
{
    File F(File_Name);
    return F.Modified_Get();
}

//---------------------------------------------------------------------------
bool File::Exists(const Ztring &File_Name)
{
    #ifdef ZENLIB_USEWX
        wxFileName FN(File_Name.c_str());
        return FN.FileExists();
    #else //ZENLIB_USEWX
        #ifdef ZENLIB_STANDARD
            if (File_Name.find(__T('*'))!=std::string::npos || File_Name.find(__T('?'))!=std::string::npos)
                return false;
            struct stat buffer;
            int         status;
            #ifdef UNICODE
                status=stat(File_Name.To_Local().c_str(), &buffer);
            #else
                status=stat(File_Name.c_str(), &buffer);
            #endif //UNICODE
            return status==0 && S_ISREG(buffer.st_mode);
        #elif defined WINDOWS
            if (File_Name.find(__T('*'))!=std::string::npos || (File_Name.find(__T("\\\\?\\"))!=0 && File_Name.find(__T('?'))!=std::string::npos) || (File_Name.find(__T("\\\\?\\"))==0 && File_Name.find(__T('?'), 4)!=std::string::npos))
                return false;
            #ifdef UNICODE
                DWORD FileAttributes;
                #ifndef ZENLIB_NO_WIN9X_SUPPORT
                if (IsWin9X_Fast())
                    FileAttributes=GetFileAttributesA(File_Name.To_Local().c_str());
                else
                #endif //ZENLIB_NO_WIN9X_SUPPORT
                    FileAttributes=GetFileAttributesW(File_Name.c_str());
            #else
                DWORD FileAttributes=GetFileAttributes(File_Name.c_str());
            #endif //UNICODE
            return ((FileAttributes!=INVALID_FILE_ATTRIBUTES) && !(FileAttributes&FILE_ATTRIBUTE_DIRECTORY));
        #endif
    #endif //ZENLIB_USEWX
}

//---------------------------------------------------------------------------
bool File::Copy(const Ztring &Source, const Ztring &Destination, bool OverWrite)
{
    #ifdef ZENLIB_USEWX
        return wxCopyFile(Source.c_str(), Destination.c_str(), OverWrite);
    #else //ZENLIB_USEWX
        #ifdef ZENLIB_STANDARD
            return false;
        #elif defined WINDOWS
            #ifdef UNICODE
                #ifndef ZENLIB_NO_WIN9X_SUPPORT
                if (IsWin9X_Fast())
                    return CopyFileA(Source.To_Local().c_str(), Destination.To_Local().c_str(), !OverWrite)!=0;
                else
                #endif //ZENLIB_NO_WIN9X_SUPPORT
                    return CopyFileW(Source.c_str(), Destination.c_str(), !OverWrite)!=0;
            #else
                return CopyFile(Source.c_str(), Destination.c_str(), !OverWrite)!=0;
            #endif //UNICODE
        #endif
    #endif //ZENLIB_USEWX
}

//---------------------------------------------------------------------------
bool File::Move(const Ztring &Source, const Ztring &Destination, bool OverWrite)
{
    if (OverWrite && Exists(Source))
        Delete(Destination);
    #ifdef ZENLIB_USEWX
        if (OverWrite && Exists(Destination))
            wxRemoveFile(Destination.c_str());
        return wxRenameFile(Source.c_str(), Destination.c_str());
    #else //ZENLIB_USEWX
        #ifdef ZENLIB_STANDARD
            return !std::rename(Source.To_Local().c_str(), Destination.To_Local().c_str());
        #elif defined WINDOWS
            #ifdef UNICODE
                #ifndef ZENLIB_NO_WIN9X_SUPPORT
                if (IsWin9X_Fast())
                    return MoveFileA(Source.To_Local().c_str(), Destination.To_Local().c_str())!=0;
                else
                #endif //ZENLIB_NO_WIN9X_SUPPORT
                    return MoveFileW(Source.c_str(), Destination.c_str())!=0;
            #else
                return MoveFile(Source.c_str(), Destination.c_str())!=0;
            #endif //UNICODE
        #endif
    #endif //ZENLIB_USEWX
}

//---------------------------------------------------------------------------
bool File::Delete(const Ztring &File_Name)
{
    #ifdef ZENLIB_USEWX
        return wxRemoveFile(File_Name.c_str());
    #else //ZENLIB_USEWX
        #ifdef ZENLIB_STANDARD
            #ifdef UNICODE
                return unlink(File_Name.To_Local().c_str())==0;
            #else
                return unlink(File_Name.c_str())==0;
            #endif //UNICODE
        #elif defined WINDOWS
            #ifdef UNICODE
                #ifndef ZENLIB_NO_WIN9X_SUPPORT
                if (IsWin9X_Fast())
                    return DeleteFileA(File_Name.To_Local().c_str())!=0;
                else
                #endif //ZENLIB_NO_WIN9X_SUPPORT
                    return DeleteFileW(File_Name.c_str())!=0;
            #else
                return DeleteFile(File_Name.c_str())!=0;
            #endif //UNICODE
        #endif
    #endif //ZENLIB_USEWX
}

//***************************************************************************
//
//***************************************************************************

} //namespace
