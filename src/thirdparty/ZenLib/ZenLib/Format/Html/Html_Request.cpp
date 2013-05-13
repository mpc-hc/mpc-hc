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
#include "ZenLib/Format/Html/Html_Request.h"
//---------------------------------------------------------------------------

namespace ZenLib
{

namespace Format
{

namespace Html
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
Request::Request()
{
    //Config
    Html=new ZenLib::Format::Html::Handler;
    IsCopy=false;
}

//---------------------------------------------------------------------------
Request::Request(const Request &Req)
{
    //Config
    Html=Req.Html;
    IsCopy=true;
}

//---------------------------------------------------------------------------
Request::~Request()
{
    //Config
    if (!IsCopy)
        delete Html; //Html=NULL
}

} //Namespace

} //Namespace

} //Namespace
