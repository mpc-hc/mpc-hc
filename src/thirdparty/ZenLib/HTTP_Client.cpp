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

//---------------------------------------------------------------------------
#include "ZenLib/Conf_Internal.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
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

