// ZenLib::Format::Html::Handler - A HTML Handler
// Copyright (C) 2008-2011 MediaArea.net SARL, Info@MediaArea.net
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
// A HTML Handler
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef ZenLib_Server_Html_HandlerH
#define ZenLib_Server_Html_HandlerH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include <string>
#include <ctime>
#include <map>
#include <vector>
#include "ZenLib/Ztring.h"
//---------------------------------------------------------------------------

namespace ZenLib
{

namespace Format
{

namespace Html
{

//***************************************************************************
/// @brief 
//***************************************************************************

struct header
{
    //In
    Ztring                              Title;
    Ztring                              Language;

    //Init
    header()
    {
    }
};

class Handler
{
public:
    //Constructor/Destructor
    Handler();

    //Maintenance
    void CleanUp();

    //The data
    header  Header;
};

} //Namespace

} //Namespace

} //Namespace

#define ENDL "\r\n"
#define HTML_ENDL "<br />\r\n"

#endif




