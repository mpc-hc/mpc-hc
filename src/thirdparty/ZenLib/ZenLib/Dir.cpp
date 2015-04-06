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
    #include <wx/dir.h>
#else //ZENLIB_USEWX
    #ifdef ZENLIB_STANDARD
        #undef WINDOWS
    #endif
    #ifdef WINDOWS
        #undef __TEXT
        #include <windows.h>
    #else
        #include <sys/stat.h>
        #include <sys/types.h>
        #undef __TEXT //dirent include windows.h on Windows/Borland
        #include <dirent.h>
        #include <glob.h>
    #endif
#endif //ZENLIB_USEWX
#include "ZenLib/Dir.h"
#include "ZenLib/File.h"
#include "ZenLib/FileName.h"
#include "ZenLib/OS_Utils.h"
//---------------------------------------------------------------------------

namespace ZenLib
{

//---------------------------------------------------------------------------
// Debug
#ifdef ZENLIB_DEBUG
    #include <stdio.h>
    #include <windows.h>
    namespace ZenLib_Dir_Debug
    {
        FILE* F;
        std::string Debug;
        SYSTEMTIME st_In;

        void Debug_Open(bool Out)
        {
            F=fopen("C:\\Temp\\ZenLib_Debug.txt", "a+t");
            Debug.clear();
            SYSTEMTIME st;
            GetLocalTime( &st );

            char Duration[100];
            if (Out)
            {
                FILETIME ft_In;
                if (SystemTimeToFileTime(&st_In, &ft_In))
                {
                    FILETIME ft_Out;
                    if (SystemTimeToFileTime(&st, &ft_Out))
                    {
                        ULARGE_INTEGER UI_In;
                        UI_In.HighPart=ft_In.dwHighDateTime;
                        UI_In.LowPart=ft_In.dwLowDateTime;

                        ULARGE_INTEGER UI_Out;
                        UI_Out.HighPart=ft_Out.dwHighDateTime;
                        UI_Out.LowPart=ft_Out.dwLowDateTime;

                        ULARGE_INTEGER UI_Diff;
                        UI_Diff.QuadPart=UI_Out.QuadPart-UI_In.QuadPart;

                        FILETIME ft_Diff;
                        ft_Diff.dwHighDateTime=UI_Diff.HighPart;
                        ft_Diff.dwLowDateTime=UI_Diff.LowPart;

                        SYSTEMTIME st_Diff;
                        if (FileTimeToSystemTime(&ft_Diff, &st_Diff))
                        {
                            sprintf(Duration, "%02hd:%02hd:%02hd.%03hd", st_Diff.wHour, st_Diff.wMinute, st_Diff.wSecond, st_Diff.wMilliseconds);
                        }
                        else
                            strcpy(Duration, "            ");
                    }
                    else
                        strcpy(Duration, "            ");

                }
                else
                    strcpy(Duration, "            ");
            }
            else
            {
                st_In=st;
                strcpy(Duration, "            ");
            }

            fprintf(F,"                                       %02hd:%02hd:%02hd.%03hd %s", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, Duration);
        }

        void Debug_Close()
        {
            Debug += "\r\n";
            fwrite(Debug.c_str(), Debug.size(), 1, F); \
            fclose(F);
        }
    }
    using namespace ZenLib_Dir_Debug;

    #define ZENLIB_DEBUG1(_NAME,_TOAPPEND) \
        Debug_Open(false); \
        Debug+=", ";Debug+=_NAME; \
        _TOAPPEND; \
        Debug_Close();

    #define ZENLIB_DEBUG2(_NAME,_TOAPPEND) \
        Debug_Open(true); \
        Debug+=", ";Debug+=_NAME; \
        _TOAPPEND; \
        Debug_Close();
#else // ZENLIB_DEBUG
    #define ZENLIB_DEBUG1(_NAME,_TOAPPEND)
    #define ZENLIB_DEBUG2(_NAME,_TOAPPEND)
#endif // ZENLIB_DEBUG

//***************************************************************************
// Constructor/Destructor
//***************************************************************************


//***************************************************************************
// Open/Close
//***************************************************************************

ZtringList Dir::GetAllFileNames(const Ztring &Dir_Name_, dirlist_t Options)
{
    ZENLIB_DEBUG1(     "Dir GetAllFileNames",
                        Debug+=", Dir_Name="; Debug+=Ztring(Dir_Name_).To_UTF8(); Debug+=", Options="; Debug +=Ztring::ToZtring(Options).To_UTF8())

    ZtringList ToReturn;
    Ztring Dir_Name=Dir_Name_;

    #ifdef ZENLIB_USEWX
        int Flags=wxDIR_FILES | wxDIR_DIRS;

        //Search for files
        wxArrayString Liste;
        wxFileName FullPath; FullPath=Dir_Name.c_str();
        //-File
        if (FullPath.FileExists())
        {
            FullPath.Normalize();
            Liste.Add(FullPath.GetFullPath());
        }
        //-Directory
        else if (FullPath.DirExists())
        {
            FullPath.Normalize();
            wxDir::GetAllFiles(FullPath.GetFullPath(), &Liste, Ztring(), Flags);
        }
        //-WildCards
        else
        {
            wxString FileName=FullPath.GetFullName();
            FullPath.SetFullName(Ztring()); //Supress filename
            FullPath.Normalize();
            if (FullPath.DirExists())
                wxDir::GetAllFiles(FullPath.GetPath(), &Liste, FileName, Flags);
        }

        //Compatible array
        ToReturn.reserve(Liste.GetCount());
        for (size_t Pos=0; Pos<Liste.GetCount(); Pos++)
            ToReturn.push_back(Liste[Pos].c_str());
    #else //ZENLIB_USEWX
        #ifdef WINDOWS
            //Is a dir?
            if (Exists(Dir_Name))
                Dir_Name+=__T("\\*");

            //Path
            Ztring Path=FileName::Path_Get(Dir_Name);
            if (Path.empty())
            {
                DWORD Path_Size=GetFullPathName(Dir_Name.c_str(), 0, NULL, NULL);
                Char* PathTemp=new Char[Path_Size+1];
                if (GetFullPathName(Dir_Name.c_str(), Path_Size+1, PathTemp, NULL))
                    Path=FileName::Path_Get(PathTemp);
                delete [] PathTemp;
            }

            #ifdef UNICODE
                WIN32_FIND_DATAW FindFileDataW;
                HANDLE hFind=FindFirstFileW(Dir_Name.c_str(), &FindFileDataW);
            #else
                WIN32_FIND_DATA FindFileData;
                HANDLE hFind=FindFirstFile(Dir_Name.c_str(), &FindFileData);
            #endif //UNICODE

            if (hFind==INVALID_HANDLE_VALUE)
            {
                ZENLIB_DEBUG2(   "Dir GetAllFileNames",
                                    Debug+=", returns with files count="; Debug +=Ztring::ToZtring(ToReturn.size()).To_UTF8())

                return ZtringList();
            }

            BOOL ReturnValue;
            do
            {
                #ifdef UNICODE
                    Ztring File_Name(FindFileDataW.cFileName);
                #else
                    Ztring File_Name(FindFileData.cFileName);
                #endif //UNICODE
                if (File_Name!=__T(".") && File_Name!=__T("..")) //Avoid . an ..
                {
                    Ztring File_Name_Complete=Path+__T("\\")+File_Name;
                    if (Exists(File_Name_Complete))
                    {
                        if (Options&Include_Dirs)
                            ToReturn.push_back(File_Name_Complete); //A dir
                        if (Options&Parse_SubDirs)
                            ToReturn+=GetAllFileNames(File_Name_Complete, Options); //A SubDir
                    }
                    else if ((Options&Include_Files) && ((Options&Include_Hidden) || (!File_Name.empty() && File_Name[0]!=__T('.'))))
                        ToReturn.push_back(File_Name_Complete); //A file
                }
                #ifdef UNICODE
                    ReturnValue=FindNextFileW(hFind, &FindFileDataW);
                #else
                    ReturnValue=FindNextFile(hFind, &FindFileData);
                #endif //UNICODE
            }
            while (ReturnValue);

            FindClose(hFind);
        #else //WINDOWS
            //A file?
            if (File::Exists(Dir_Name))
            {
               ToReturn.push_back(Dir_Name); //TODO
               return ToReturn;
            }

            //A dir?
            if (!Dir::Exists(Dir_Name))
                return ToReturn; //Does not exist

            //open
            #ifdef UNICODE
                DIR* Dir=opendir(Dir_Name.To_Local().c_str());
            #else
                DIR* Dir=opendir(Dir_Name.c_str());
            #endif //UNICODE
            if (Dir)
            {
                //This is a dir
                //Normalizing dir (the / at the end)
                size_t Dir_Pos=Dir_Name.rfind(FileName_PathSeparator);
                if (Dir_Pos==std::string::npos)
                    Dir_Name+=FileName_PathSeparator;
                else if (Dir_Pos+Ztring(FileName_PathSeparator).size()!=Dir_Name.size())
                    Dir_Name+=FileName_PathSeparator;

                struct dirent *DirEnt;
                while((DirEnt=readdir(Dir))!=NULL)
                {
                    //A file
                    Ztring File_Name(DirEnt->d_name);
                    if (File_Name!=__T(".") && File_Name!=__T("..")) //Avoid . an ..
                    {
                        Ztring File_Name_Complete=Dir_Name+File_Name;
                        if (Exists(File_Name_Complete))
                        {
                            if (Options&Parse_SubDirs)
                                ToReturn+=GetAllFileNames(File_Name_Complete, Options); //A SubDir
                        }
                        else if ((Options&Include_Hidden) || (!File_Name.empty() && File_Name[0]!=__T('.')))
                            ToReturn.push_back(File_Name_Complete); //A file
                    }
                }

                //Close it
                closedir(Dir);
            }
            else
            {
                glob_t globbuf;
                if (glob(Dir_Name.To_Local().c_str(), GLOB_NOSORT, NULL, &globbuf)==0)
                {
                    for (int Pos=0; Pos<globbuf.gl_pathc; Pos++)
                        ToReturn.push_back(Ztring().From_Local(globbuf.gl_pathv[Pos]));
                }
            }
        #endif
    #endif //ZENLIB_USEWX

    ZENLIB_DEBUG2(   "Dir GetAllFileNames",
                        Debug+=", files count="; Debug +=Ztring::ToZtring(ToReturn.size()).To_UTF8())

    return ToReturn;
}

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
bool Dir::Exists(const Ztring &File_Name)
{
    ZENLIB_DEBUG1(     "Dir Exists",
                        Debug+=", Dir_Name="; Debug+=Ztring(File_Name).To_UTF8();)

    #ifdef ZENLIB_USEWX
        wxFileName FN(File_Name.c_str());
        return FN.DirExists();
    #else //ZENLIB_USEWX
       #ifdef WINDOWS
            #ifdef UNICODE
                DWORD FileAttributes=GetFileAttributesW(File_Name.c_str());
            #else
                DWORD FileAttributes=GetFileAttributes(File_Name.c_str());
            #endif //UNICODE

            ZENLIB_DEBUG2(   "Dir Exists",
                                Debug+=", returns "; Debug +=Ztring::ToZtring(((FileAttributes!=INVALID_FILE_ATTRIBUTES) && (FileAttributes&FILE_ATTRIBUTE_DIRECTORY))?1:0).To_UTF8())

            return ((FileAttributes!=INVALID_FILE_ATTRIBUTES) && (FileAttributes&FILE_ATTRIBUTE_DIRECTORY));
        #else //WINDOWS
            struct stat buffer;
            int         status;
            #ifdef UNICODE
                status=stat(File_Name.To_Local().c_str(), &buffer);
            #else
                status=stat(File_Name.c_str(), &buffer);
            #endif //UNICODE
            return status==0 && S_ISDIR(buffer.st_mode);
        #endif
    #endif //ZENLIB_USEWX
}

//---------------------------------------------------------------------------
bool Dir::Create(const Ztring &File_Name)
{
    #ifdef ZENLIB_USEWX
        return wxFileName::Mkdir(File_Name.c_str());
    #else //ZENLIB_USEWX
        #ifdef WINDOWS
            #ifdef UNICODE
                return CreateDirectoryW(File_Name.c_str(), NULL)!=0;
            #else
                return CreateDirectory(File_Name.c_str(), NULL)!=0;
            #endif //UNICODE
        #else //WINDOWS
            return mkdir(File_Name.To_Local().c_str(), 0700)==0;
        #endif //WINDOWS
    #endif //ZENLIB_USEWX
}

//***************************************************************************
// GetAllFileNames
//***************************************************************************

#ifdef WINDOWS
class GetAllFileNames_Private
{
public:
    Ztring Dir_Name;
    Ztring Path;
    Dir::dirlist_t Options;
    HANDLE hFind;
    #ifdef UNICODE
        WIN32_FIND_DATAW FindFileDataW;
    #else
        WIN32_FIND_DATA FindFileData;
    #endif //UNICODE

    GetAllFileNames_Private()
        : hFind(INVALID_HANDLE_VALUE)
    {
    }
};

//---------------------------------------------------------------------------
GetAllFileNames::GetAllFileNames()
    : p(NULL)
{
}

//---------------------------------------------------------------------------
GetAllFileNames::~GetAllFileNames()
{
    Close();
}

//---------------------------------------------------------------------------
void GetAllFileNames::Start  (const Ztring &Dir_Name_, Dir::dirlist_t Options_)
{
    ZENLIB_DEBUG1(     "GetAllFileNames Start",
                        Debug+=", Dir_Name="; Debug+=Ztring(Dir_Name_).To_UTF8(); Debug+=", Options="; Debug +=Ztring::ToZtring(Options_).To_UTF8())

    delete p; p=new GetAllFileNames_Private;
    p->Dir_Name=Dir_Name_;
    p->Options=Options_;

    #ifdef WINDOWS
        //Is a dir?
        if (Dir::Exists(p->Dir_Name))
            p->Dir_Name+=__T("\\*");

        //Path
        p->Path=FileName::Path_Get(p->Dir_Name);
        if (p->Path.empty())
        {
            DWORD Path_Size=GetFullPathName(p->Dir_Name.c_str(), 0, NULL, NULL);
            Char* PathTemp=new Char[Path_Size+1];
            if (GetFullPathName(p->Dir_Name.c_str(), Path_Size+1, PathTemp, NULL))
                p->Path=FileName::Path_Get(PathTemp);
            delete [] PathTemp;
        }
    #else //WINDOWS
    #endif

    ZENLIB_DEBUG2(   "GetAllFileNames Start",
                        )
}

bool GetAllFileNames::Next (Ztring& Name)
{
    if (!p)
        return false;

    ZENLIB_DEBUG1(     "GetAllFileNames Next",
                        Debug+=",  Dir_Name="; Debug+=Ztring(p->Dir_Name).To_UTF8())

    #ifdef WINDOWS
        for (;;)
        {
            if (p->hFind==INVALID_HANDLE_VALUE)
            {
                #ifdef UNICODE
                    p->hFind=FindFirstFileW(p->Dir_Name.c_str(), &p->FindFileDataW);
                #else
                    p->hFind=FindFirstFile(p->Dir_Name.c_str(), &p->FindFileData);
                #endif //UNICODE

                if (p->hFind==INVALID_HANDLE_VALUE)
                    break;
            }
            else
            {
                BOOL ReturnValue;
                #ifdef UNICODE
                    ReturnValue=FindNextFileW(p->hFind, &p->FindFileDataW);
                #else
                    ReturnValue=FindNextFile(p->hFind, &p->FindFileData);
                #endif //UNICODE
                if (!ReturnValue)
                    break;
            }

            #ifdef UNICODE
                Ztring File_Name(p->FindFileDataW.cFileName);
            #else
                Ztring File_Name(p->FindFileData.cFileName);
            #endif //UNICODE
            if (File_Name!=__T(".") && File_Name!=__T("..")) //Avoid . an ..
            {
                bool IsOk=false;
                #ifdef UNICODE
                    if (p->FindFileDataW.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
                #else
                    if (p->FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
                #endif //UNICODE
                {
                    if (p->Options&Dir::Include_Dirs)
                        IsOk=true; //A dir
                }
                else if ((p->Options&Dir::Include_Files) && ((p->Options&Dir::Include_Hidden) || (!File_Name.empty() && File_Name[0]!=__T('.'))))
                    IsOk=true; //A file
                if (IsOk)
                {
                    Name=p->Path+__T("\\")+File_Name;

                    ZENLIB_DEBUG2(   "GetAllFileNames Next",
                                        Debug+=", File_Name="; Debug +=Name.To_UTF8())

                    return true;
                }
            }
        }
    #else //WINDOWS
    #endif

    Close();
    return false;
}

void GetAllFileNames::Close ()
{
    if (!p)
        return;

    ZENLIB_DEBUG1(     "GetAllFileNames Close",
                        Debug+=", Dir_Name="; Debug+=Ztring(p->Dir_Name).To_UTF8())

    FindClose(p->hFind); p->hFind=INVALID_HANDLE_VALUE;
    delete p; p=NULL;

    ZENLIB_DEBUG2(   "GetAllFileNames Close",
                        )
}
#endif //WINDOWS

//***************************************************************************
//
//***************************************************************************

} //namespace
