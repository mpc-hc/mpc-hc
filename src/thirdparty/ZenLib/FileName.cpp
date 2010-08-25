// ZenLib::FileName - File name functions
// Copyright (C) 2007-2010 MediaArea.net SARL, Info@MediaArea.net
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
#include "ZenLib/Conf_Internal.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#include "ZenLib/FileName.h"
#ifdef ZENLIB_USEWX
    #include <wx/filename.h>
#else //ZENLIB_USEWX
    #ifdef ZENLIB_STANDARD
        #undef WINDOWS
    #endif
    #ifdef WINDOWS
        #undef __TEXT
        #include <windows.h>
    #endif
#endif //ZENLIB_USEWX
//---------------------------------------------------------------------------
#undef ZENLIB_USEWX

namespace ZenLib
{

//***************************************************************************
// Read/Write
//***************************************************************************

//---------------------------------------------------------------------------
Ztring FileName::Path_Get() const
{
    #ifdef ZENLIB_USEWX
        wxFileName FN(c_str());
        return FN.GetPath().c_str();
    #else //ZENLIB_USEWX
        //Path limit
        size_t Pos_Path=rfind(FileName_PathSeparator);
        if (Pos_Path==Ztring::npos)
            return Ztring(); //Not found
        else
            return Ztring(*this, 0, Pos_Path);
    #endif //ZENLIB_USEWX
}

//---------------------------------------------------------------------------
Ztring& FileName::Path_Set(const Ztring &Path)
{
    #ifdef ZENLIB_USEWX
        wxFileName FN(c_str());
        FN.SetPath(Path.c_str());
        assign (FN.GetFullPath().c_str());
    #else //ZENLIB_USEWX
        #ifdef WINDOWS
            //Path limit
            size_t Pos_Path=rfind(_T('\\'));
            if (Pos_Path==Ztring::npos)
            {
                insert(0, 1, _T('\\')); //Not found
                Pos_Path=0;
            }
            replace(0, Pos_Path, Path, 0, Ztring::npos);
        #else
            //Not supported
        #endif
    #endif //ZENLIB_USEWX
    return *this;
}

//---------------------------------------------------------------------------
Ztring FileName::Name_Get() const
{
    #ifdef ZENLIB_USEWX
        wxFileName FN(c_str());
        if (FN==FN.GetName()) //Bug of WxWidgets? if C:\\dir\\(no name), name is C:\\dir\\(no name)
            return Ztring();
        return FN.GetName().c_str();
    #else //ZENLIB_USEWX
        size_t Pos_Path=rfind(FileName_PathSeparator); //Path limit
        if (Pos_Path==Ztring::npos)
            Pos_Path=(size_type)-1; //Not found
        //Extension limit
        size_t Pos_Ext=rfind(_T('.'));
        if (Pos_Ext==Ztring::npos || Pos_Ext<Pos_Path)
            Pos_Ext=size(); //Not found
        return Ztring(*this, Pos_Path+1, Pos_Ext-Pos_Path-1);
    #endif //ZENLIB_USEWX
}

//---------------------------------------------------------------------------
Ztring& FileName::Name_Set(const Ztring &Name)
{
    #ifdef ZENLIB_USEWX
        wxFileName FN(c_str());
        if (FN==FN.GetName()) //Bug of WxWidgets? if C:\\dir\\(no name), name is C:\\dir\\(no name)
            FN.SetPath(c_str());
        FN.SetName(Name.c_str());
        assign ((FN.GetFullPath()+FN.GetPathSeparator()/*FileName_PathSeparator*/+FN.GetFullName()).c_str());
    #else //ZENLIB_USEWX
        #ifdef WINDOWS
            //Path limit
            size_t Pos_Path=rfind(_T('\\'));
            if (Pos_Path==Ztring::npos)
                Pos_Path=0; //Not found
            //Extension limit
            size_t Pos_Ext=rfind(_T('.'));
            if (Pos_Ext==Ztring::npos || Pos_Ext<Pos_Path)
                Pos_Ext=size(); //Not found
            replace(Pos_Path+1, Pos_Ext-Pos_Path-1, Name, 0, Ztring::npos);
        #else
            //Not supported
        #endif
    #endif //ZENLIB_USEWX
    return *this;
}

//---------------------------------------------------------------------------
Ztring FileName::Extension_Get() const
{
    #ifdef ZENLIB_USEWX
        wxFileName FN(c_str());
        return FN.GetExt().c_str();
    #else //ZENLIB_USEWX
        //Path limit
        size_t Pos_Path=rfind(FileName_PathSeparator);
        if (Pos_Path==Ztring::npos)
            Pos_Path=0; //Not found
        //Extension limit
        size_t Pos_Ext=rfind(_T('.'));
        if (Pos_Ext==Ztring::npos || Pos_Ext<Pos_Path)
            return Ztring(); //Not found
        else
            return Ztring(*this, Pos_Ext+1, size()-Pos_Ext-1);
    #endif //ZENLIB_USEWX
}

//---------------------------------------------------------------------------
Ztring& FileName::Extension_Set(const Ztring &Extension)
{
    #ifdef ZENLIB_USEWX
        wxFileName FN(c_str());
        FN.SetExt(Extension.c_str());
        assign (FN.GetFullPath().c_str());
    #else //ZENLIB_USEWX
        #ifdef WINDOWS
            //Path limit
            size_t Pos_Path=rfind(_T('\\'));
            if (Pos_Path==Ztring::npos)
                Pos_Path=0; //Not found
            //Extension limit
            size_t Pos_Ext=rfind(_T('.'));
            if (Pos_Ext==Ztring::npos || Pos_Ext<Pos_Path)
            {
                append(1, _T('.')); //Not found
                Pos_Ext=size()-1;
            }
            replace(Pos_Ext+1, size()-Pos_Ext-1, Extension, 0, Ztring::npos);
        #else
            //Not supported
        #endif
    #endif //ZENLIB_USEWX
    return *this;
}

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
Ztring FileName::TempFileName_Create(const Ztring &Prefix)
{
    #ifdef ZENLIB_USEWX
        return wxFileName::CreateTempFileName(Prefix.c_str()).c_str();
    #else //ZENLIB_USEWX
        #ifdef WINDOWS
            Char Path[MAX_PATH+1];
            if (!GetTempPath(MAX_PATH, Path))
                return Ztring(); //Problem while getting a temp path

            Char FileName[MAX_PATH+1];
            if (!GetTempFileName(Path, Prefix.c_str(), 0, FileName))
                 return Ztring(); //Problem while getting a file name

            return Ztring(FileName);
        #else
            return _T("C:\\xxx.txt");
        #endif
    #endif //ZENLIB_USEWX
}

//***************************************************************************
// Platform differences
//***************************************************************************

//End of line
#ifdef __WINDOWS__
    const Char* FileName_PathSeparator=_T("\\");
#endif
#ifdef UNIX
    const Char* FileName_PathSeparator=_T("/");
#endif
#ifdef MACOS
    const Char* FileName_PathSeparator=_T("/");
#endif
#ifdef MACOSX
    const Char* FileName_PathSeparator=_T("/");
#endif

} //namespace

