// ZenLib::Dir - Directories functions
// Copyright (C) 2007-2012 MediaArea.net SARL, Info@MediaArea.net
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

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

//***************************************************************************
// Constructor/Destructor
//***************************************************************************


//***************************************************************************
// Open/Close
//***************************************************************************

ZtringList Dir::GetAllFileNames(const Ztring &Dir_Name_, dirlist_t Options)
{
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
                #ifdef UNICODE
                    #ifndef ZENLIB_NO_WIN9X_SUPPORT
                    if (IsWin9X_Fast())
                    {
                        DWORD Path_Size=GetFullPathNameA(Dir_Name.To_Local().c_str(), 0, NULL, NULL);
                        char* PathTemp=new char[Path_Size+1];
                        if (GetFullPathNameA(Dir_Name.To_Local().c_str(), Path_Size+1, PathTemp, NULL))
                            Path=FileName::Path_Get(PathTemp);
                        delete [] PathTemp; //PathTemp=NULL;
                    }
                    else
                    #endif //ZENLIB_NO_WIN9X_SUPPORT
                    {
                        DWORD Path_Size=GetFullPathName(Dir_Name.c_str(), 0, NULL, NULL);
                        Char* PathTemp=new Char[Path_Size+1];
                        if (GetFullPathNameW(Dir_Name.c_str(), Path_Size+1, PathTemp, NULL))
                            Path=FileName::Path_Get(PathTemp);
                        delete [] PathTemp; //PathTemp=NULL;
                    }
                #else
                    DWORD Path_Size=GetFullPathName(Dir_Name.c_str(), 0, NULL, NULL);
                    Char* PathTemp=new Char[Path_Size+1];
                    if (GetFullPathName(Dir_Name.c_str(), Path_Size+1, PathTemp, NULL))
                        Path=FileName::Path_Get(PathTemp);
                    delete [] PathTemp; //PathTemp=NULL;
                #endif //UNICODE
            }

            #ifdef UNICODE
                WIN32_FIND_DATAW FindFileDataW;
                HANDLE hFind;
                #ifndef ZENLIB_NO_WIN9X_SUPPORT
                WIN32_FIND_DATAA FindFileDataA;
                if (IsWin9X_Fast())
                    hFind=FindFirstFileA(Dir_Name.To_Local().c_str(), &FindFileDataA);
                else
                #endif //ZENLIB_NO_WIN9X_SUPPORT
                    hFind=FindFirstFileW(Dir_Name.c_str(), &FindFileDataW);
            #else
                WIN32_FIND_DATA FindFileData;
                HANDLE hFind=FindFirstFile(Dir_Name.c_str(), &FindFileData);
            #endif //UNICODE

            if (hFind==INVALID_HANDLE_VALUE)
                return ZtringList();

            BOOL ReturnValue;
            do
            {
                #ifdef UNICODE
                    Ztring File_Name;
                    #ifndef ZENLIB_NO_WIN9X_SUPPORT
                    if (IsWin9X_Fast())
                        File_Name=FindFileDataA.cFileName;
                    else
                    #endif //ZENLIB_NO_WIN9X_SUPPORT
                        File_Name=FindFileDataW.cFileName;
                #else
                    Ztring File_Name(FindFileData.cFileName);
                #endif //UNICODE
                if (File_Name!=__T(".") && File_Name!=__T("..")) //Avoid . an ..
                {
                    Ztring File_Name_Complete=Path+__T("\\")+File_Name;
                    if (Exists(File_Name_Complete))
                    {
                        if (Options&Parse_SubDirs)
                            ToReturn+=GetAllFileNames(File_Name_Complete, Options); //A SubDir
                    }
                    else if ((Options&Include_Hidden) || (!File_Name.empty() && File_Name[0]!=__T('.')))
                        ToReturn.push_back(File_Name_Complete); //A file
                }
                #ifdef UNICODE
                    #ifndef ZENLIB_NO_WIN9X_SUPPORT
                    if (IsWin9X_Fast())
                        ReturnValue=FindNextFileA(hFind, &FindFileDataA);
                    else
                    #endif //ZENLIB_NO_WIN9X_SUPPORT
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

    return ToReturn;
}

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
bool Dir::Exists(const Ztring &File_Name)
{
    #ifdef ZENLIB_USEWX
        wxFileName FN(File_Name.c_str());
        return FN.DirExists();
    #else //ZENLIB_USEWX
       #ifdef WINDOWS
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
                #ifndef ZENLIB_NO_WIN9X_SUPPORT
                if (IsWin9X_Fast())
                    return CreateDirectoryA(File_Name.To_Local().c_str(), NULL)!=0;
                else
                #endif //ZENLIB_NO_WIN9X_SUPPORT
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
//
//***************************************************************************

} //namespace
