#pragma once

class CWebServer;

class CWebServerSocket : public CAsyncSocket
{
	CWebServer* m_pWebServer;

protected:
	void OnAccept(int nErrorCode);

public:
	CWebServerSocket(CWebServer* pWebServer, int port = 13579);
	virtual ~CWebServerSocket();
};