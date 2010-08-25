// ZenLib::HTTPClient - Basic HTTP client
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
// Basic HTTP client
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef ZenLib_HTTPClientH
#define ZenLib_HTTPClientH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "ZenLib/Conf.h"
#include "ZenLib/Ztring.h"
//---------------------------------------------------------------------------

namespace ZenLib
{

//***************************************************************************
/// @brief Basic HTTP client
//***************************************************************************

class HTTP_Client
{
public :
    //Constructor/Destructor
    HTTP_Client  ();
    ~HTTP_Client ();

    //Open/Close
    int  Open  (Ztring URL);
    void Close ();

    //Read
    Ztring Read();

private :
    int32u Handle;
};

} //NameSpace

#endif
