/* 
 *	Copyright (C) 2007 Casimir666
 *	http://tibrium.neuf.fr
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */


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
