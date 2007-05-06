#include "stdafx.h"
#include ".\webserver.h"
#include ".\webserversocket.h"

CWebServerSocket::CWebServerSocket(CWebServer* pWebServer, int port) 
	: m_pWebServer(pWebServer)
{
	Create(port);
	Listen();
}

CWebServerSocket::~CWebServerSocket()
{
}

void CWebServerSocket::OnAccept(int nErrorCode)
{
	if(nErrorCode == 0 && m_pWebServer) 
		m_pWebServer->OnAccept(this);

	__super::OnAccept(nErrorCode);
}
