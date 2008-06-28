#pragma once

#include <Setupapi.h>


class CMultiSz  
{
public:
	void DisplayBuffer();
	BOOL FindString(const char *str) const;
	BOOL RemoveString(const char * str);
	BOOL AddString(const char * str);
	int m_MultiSzPos;

	void FirstPosition(int * position) const;
	void First();

	const char * NextPosition(int * position) const;
	const char * Next();

	inline const DWORD  GetBufferLen() const { return m_MultiSzLen; };
	inline const BYTE * GetBuffer() const { return m_MultiSzBuf; };
	BOOL SetBuffer(BYTE * buf, DWORD len);

	CMultiSz();
	CMultiSz(const CMultiSz& other);
	virtual ~CMultiSz();

	const CMultiSz& operator = (const CMultiSz& other);

private:
	DWORD m_MultiSzLen;
	BYTE * m_MultiSzBuf;
	BYTE* Realloc (BYTE* pBuff, DWORD nNewSize);
};


class CShuttlePN31Client
{
public:
	CShuttlePN31Client(void);
	~CShuttlePN31Client(void);
	void	SetHWND(HWND hWnd);
	void	Connect();
	void	Disconnect();
	bool	Install();
	bool	Uninstall();

private :
	CWnd*			m_pWnd;
	HANDLE			m_hPipe;
	HANDLE			m_hThread;

	static DWORD WINAPI	ThreadStart(LPVOID lpParam);
	void			WaitKey();
	void			ExecuteCommand(int nKey);

	bool			UpdateDriver(bool bInstall);
	CMultiSz		GetLowerFilters(HDEVINFO hDev, SP_DEVINFO_DATA* hDevInfo);
	void			SetLowerFilters(HDEVINFO hDev, SP_DEVINFO_DATA* hDevInfo, const CMultiSz& sz);
	bool			RestartDevice(HDEVINFO hDev, SP_DEVINFO_DATA* hDevInfo);
	bool			CreateService();
	bool			DeleteService();
};
