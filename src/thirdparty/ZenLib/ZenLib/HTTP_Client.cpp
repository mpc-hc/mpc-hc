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
#include "ZenLib/HTTP_Client.h"
#ifdef WINDOWS
    #undef __TEXT
    #include <windows.h>
#endif //WINDOWS
#include "ZenLib/HTTP_Client/HTTPClient.h"
using namespace ZenLib;
//---------------------------------------------------------------------------

namespace ZenLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
HTTP_Client::HTTP_Client ()
{
    #ifdef WINDOWS
        WSADATA WsaData;
        WSAStartup(MAKEWORD(1, 2), &WsaData);
    #endif
    Handle=0;
}

//---------------------------------------------------------------------------
HTTP_Client::~HTTP_Client ()
{
    #ifdef WINDOWS
        WSACleanup();
    #endif
}

//***************************************************************************
// Open/Close
//***************************************************************************

//---------------------------------------------------------------------------
int HTTP_Client::Open (Ztring Url)
{
    if (Handle)
        Close();

    //init
    Handle=HTTPClientOpenRequest(0);

    //Mehtod
    if (HTTPClientSetVerb(Handle, VerbGet)!=0)
    {
        Close();
        return 0;
    }

    //Send request
    if (HTTPClientSendRequest(Handle, (char*)(Url.To_Local().c_str()), NULL, 0, FALSE, 0, 0)!=0)
    {
        Close();
        return 0;
    }

    //Receive response
    if (HTTPClientRecvResponse(Handle, 3)!=0)
    {
        Close();
        return 0;
    }

    return 1;
}

//---------------------------------------------------------------------------
void HTTP_Client::Close ()
{
    HTTPClientCloseRequest(&Handle);
    Handle=0;
}

//***************************************************************************
// Read
//***************************************************************************

//---------------------------------------------------------------------------
Ztring HTTP_Client::Read ()
{
    if (Handle==0)
        return Ztring();

    char* Buffer=new char[16384];
    int32u Size=0;

    int32u ReturnValue=HTTPClientReadData(Handle, Buffer, 16384, 0, &Size);
    if (ReturnValue!=0 && ReturnValue!=1000) //End of stream
    {
        Close();
        delete[] Buffer;
        return Ztring();
    }

    Ztring ToReturn; ToReturn.From_Local(Buffer, Size);
    delete[] Buffer;
    return ToReturn;
}

} //namespace
