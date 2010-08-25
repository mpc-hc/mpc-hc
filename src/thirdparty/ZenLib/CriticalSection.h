// ZenLib::CriticalSection - CriticalSection functions
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
//
// CriticalSection functions
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef ZenLib_CriticalSectionH
#define ZenLib_CriticalSectionH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#ifdef CS
   #undef CS //Solaris defines this somewhere
#endif
//---------------------------------------------------------------------------

namespace ZenLib
{

//***************************************************************************
/// @brief CriticalSection manipulation
//***************************************************************************

class CriticalSection
{
public :
    //Constructor/Destructor
    CriticalSection  ();
    ~CriticalSection ();

    //Enter/Leave
    void  Enter();
    void  Leave();

private :
    void* CritSect;
};

//***************************************************************************
/// @brief CriticalSectionLocker helper
//***************************************************************************

class CriticalSectionLocker
{
public:
    CriticalSectionLocker (ZenLib::CriticalSection &CS)
    {
        CritSec=&CS;
        CritSec->Enter();
    }

    ~CriticalSectionLocker ()
    {
        CritSec->Leave();
    }

private:
    ZenLib::CriticalSection *CritSec;
};

} //NameSpace

#endif
