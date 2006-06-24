#pragma once

class CWebServer;

class CWebClientSocket : public CAsyncSocket
{
	CWebServer* m_pWebServer;
	CMainFrame* m_pMainFrame;

	CString m_hdr;

	struct cookie_attribs {CString path, expire, domain;};
	CAtlStringMap<cookie_attribs> m_cookieattribs;

	void Clear();
	void Header();

protected:
	void OnReceive(int nErrorCode);
	void OnClose(int nErrorCode);

public:
	CWebClientSocket(CWebServer* pWebServer, CMainFrame* pMainFrame);
	virtual ~CWebClientSocket();

	bool SetCookie(CString name, CString value = _T(""), __time64_t expire = -1, CString path = _T("/"), CString domain = _T(""));

	CString m_sessid;
	CString m_cmd, m_path, m_query, m_ver;
	CStringA m_data;
	CAtlStringMap<> m_hdrlines;
	CAtlStringMap<> m_get, m_post, m_cookie;
	CAtlStringMap<> m_request;

	bool OnCommand(CStringA& hdr, CStringA& body, CStringA& mime);
	bool OnIndex(CStringA& hdr, CStringA& body, CStringA& mime);
	bool OnBrowser(CStringA& hdr, CStringA& body, CStringA& mime);
	bool OnControls(CStringA& hdr, CStringA& body, CStringA& mime);
	bool OnStatus(CStringA& hdr, CStringA& body, CStringA& mime);
    bool OnError404(CStringA& hdr, CStringA& body, CStringA& mime);
    bool OnPlayer(CStringA& hdr, CStringA& body, CStringA& mime);
    bool OnSnapShotJpeg(CStringA& hdr, CStringA& body, CStringA& mime);
    bool OnConvRes(CStringA& hdr, CStringA& body, CStringA& mime);
};