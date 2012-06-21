// ZenLib::OS_Utils - Cross platform OS utils
// Copyright (C) 2002-2011 MediaArea.net SARL, Info@MediaArea.net
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
#else //ZENLIB_USEWX
    #ifdef WINDOWS
        #undef __TEXT
        #include <windows.h>
        #include <shlobj.h>
    #endif
#endif //ZENLIB_USEWX
#include "ZenLib/OS_Utils.h"
//---------------------------------------------------------------------------

namespace ZenLib
{

//***************************************************************************
// OS info
//***************************************************************************

//---------------------------------------------------------------------------
bool IsWin9X  ()
{
    #ifdef ZENLIB_USEWX
        return true;
    #else //ZENLIB_USEWX
        #ifdef WINDOWS
            if (GetVersion()<0x80000000)
                return false;
            else
                return true;
        #else //WINDOWS
            return true;
        #endif
    #endif //ZENLIB_USEWX
}

//***************************************************************************
// Shell
//***************************************************************************

void Shell_Execute(const Ztring &ToExecute)
{
    #ifdef ZENLIB_USEWX
    #else //ZENLIB_USEWX
        #ifdef WINDOWS
            #ifdef UNICODE
                if (IsWin9X())
                    ShellExecuteA(NULL, "open", ToExecute.To_Local().c_str(), NULL, NULL, 0);
                else
                    ShellExecute (NULL, _T("open"), ToExecute.c_str(), NULL, NULL, 0);
            #else
                ShellExecute(NULL, _T("open"), ToExecute.c_str(), NULL, NULL, 0);
            #endif
        #else
            //Not supported
        #endif
    #endif //ZENLIB_USEWX
}

//***************************************************************************
// Directories
//***************************************************************************

//---------------------------------------------------------------------------
// Select directory code
// Extracted from TBffolder by Torsten Johann (t-johann@gmx.de)

Ztring Directory_Select_Caption;

#ifdef WINDOWS
    #ifdef UNICODE
        char    InitDirA[MAX_PATH];
        wchar_t InitDir [MAX_PATH];

        int __stdcall ShowOpenFolder_CallbackProc (HWND hwnd, UINT uMsg, LPARAM, LPARAM)
        {
            if (uMsg==BFFM_INITIALIZED)
            {
                if (IsWin9X())
                {
                    SetWindowTextA (hwnd, Directory_Select_Caption.To_Local().c_str());    // Caption
                    SendMessageA   (hwnd, BFFM_ENABLEOK, 0, TRUE);
                    SendMessageA   (hwnd, BFFM_SETSELECTION, true, (LPARAM)&InitDirA);
                }
                else
                {
                    SetWindowText  (hwnd, Directory_Select_Caption.c_str());    // Caption
                    SendMessage    (hwnd, BFFM_ENABLEOK, 0, TRUE);
                    SendMessage    (hwnd, BFFM_SETSELECTION, true, (LPARAM)&InitDir);
                }
            }
            return 0;
        }

        Ztring OpenFolder_Show(void* Handle, const Ztring &Title, const Ztring &Caption)
        {
            //Caption
            Directory_Select_Caption=Caption;

            if (IsWin9X())
            {
                return Ztring(); //Not supported in Win9X
            }
            else
            {
                //Values
                LPMALLOC        Malloc;
                LPSHELLFOLDER   ShellFolder;
                BROWSEINFO      BrowseInfo;
                LPITEMIDLIST    ItemIdList;

                //Initializing the SHBrowseForFolder function
                if (SHGetMalloc(&Malloc)!=NOERROR)
                    return Ztring();
                if (SHGetDesktopFolder(&ShellFolder)!=NOERROR)
                    return Ztring();
                ZeroMemory(&BrowseInfo, sizeof(BROWSEINFOW));
                BrowseInfo.ulFlags+=BIF_RETURNONLYFSDIRS;
                BrowseInfo.hwndOwner=(HWND)Handle;
                BrowseInfo.pszDisplayName=InitDir;
                BrowseInfo.lpszTitle=Title.c_str();
                BrowseInfo.lpfn=ShowOpenFolder_CallbackProc;

                //Displaying
                ItemIdList=SHBrowseForFolder(&BrowseInfo);

                //Releasing
                ShellFolder->Release();
                if (ItemIdList!=NULL)
                {
                    SHGetPathFromIDList(ItemIdList, InitDir);
                    Malloc->Free(ItemIdList);
                    Malloc->Release();

                    //The value
                    return InitDir;
                }
                else
                    return Ztring();
            }
        }

    #else
        char InitDirA[MAX_PATH];

        int __stdcall ShowOpenFolder_CallbackProc (HWND hwnd, UINT uMsg, LPARAM, LPARAM)
        {
            if (uMsg==BFFM_INITIALIZED)
            {
                SetWindowText (hwnd, Directory_Select_Caption.c_str());    // Caption
                SendMessage   (hwnd, BFFM_ENABLEOK, 0, TRUE);
                SendMessage   (hwnd, BFFM_SETSELECTION, true, (LPARAM)&InitDirA);
            }
            return 0;
        }

        Ztring OpenFolder_Show(void* Handle, const Ztring &Title, const Ztring &Caption)
        {
            //Caption
            Directory_Select_Caption=Caption;

            //Values
            LPMALLOC        Malloc;
            LPSHELLFOLDER   ShellFolder;
            BROWSEINFO      BrowseInfo;
            LPITEMIDLIST    ItemIdList;

            //Initializing the SHBrowseForFolder function
            if (SHGetMalloc(&Malloc)!=NOERROR)
                return Ztring();
            if (SHGetDesktopFolder(&ShellFolder)!=NOERROR)
                return Ztring();
            ZeroMemory(&BrowseInfo, sizeof(BROWSEINFO));
            BrowseInfo.ulFlags+=BIF_RETURNONLYFSDIRS;
            BrowseInfo.hwndOwner=(HWND)Handle;
            BrowseInfo.pszDisplayName=InitDirA;
            BrowseInfo.lpszTitle=Title.c_str();
            BrowseInfo.lpfn=ShowOpenFolder_CallbackProc;

            //Displaying
            ItemIdList=SHBrowseForFolder(&BrowseInfo);

            //Releasing
            ShellFolder->Release();
            if (ItemIdList!=NULL)
            {
                SHGetPathFromIDList(ItemIdList, InitDirA);
                Malloc->Free(ItemIdList);
                Malloc->Release();

                //The value
                return InitDirA;
            }
            else
                return Ztring();
        }
    #endif //UNICODE
#endif //WINDOWS

} //namespace ZenLib
