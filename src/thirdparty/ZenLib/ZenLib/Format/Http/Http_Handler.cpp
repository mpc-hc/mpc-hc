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
#include "ZenLib/Format/Http/Http_Handler.h"
#include "ZenLib/Ztring.h"
#include <sstream>
using namespace std;
//---------------------------------------------------------------------------

namespace ZenLib
{

namespace Format
{

namespace Http
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
Handler::Handler()
{
    //In
    Foreign_Port=0;
    Local_Port=0;
    HeadersOnly=false;

    //Out
    Response_HTTP_Code=200;
}

} //Namespace

} //Namespace

} //Namespace
