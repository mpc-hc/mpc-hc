#include "StdAfx.h"
#include "shuttlepn31client.h"
#include "../Common/Interface.h"
#include "../../mplayerc/resource.h"

#define BUFSIZE 4096
#define BUFFER_SIZE		300


// ======== MultiSZ
CMultiSz::CMultiSz()
{
	m_MultiSzBuf = NULL;
	m_MultiSzLen = 0;
	m_MultiSzPos = 0;
}

CMultiSz::CMultiSz(const CMultiSz& other)
{
	*this = other;
}

BYTE* CMultiSz::Realloc (BYTE* pBuff, DWORD nNewSize)
{
	BYTE*	pNewBuff = new BYTE [nNewSize];
	memset (pNewBuff, 0, nNewSize);
	if (pBuff) memcpy (pNewBuff, pBuff, min (m_MultiSzLen, nNewSize));
	return pNewBuff;
}

CMultiSz::~CMultiSz()
{
	if (m_MultiSzBuf != NULL)
		delete[] m_MultiSzBuf;
}

BOOL CMultiSz::SetBuffer(BYTE *buf, DWORD len)
{
	BYTE * newBuf;

	// check the content of "buf"

	if (len < 0)
	{
//		TRACE("Error : len < 0 [len = %d], using len = 0\n",len);
		len = 0;
	}
	else if (len != 0 && len < 2)
	{
//		TRACE("Error : len = %d, using len = 0\n",len);
		len = 0;
	}
	else if (len >= 2)
	{
		// check the content of buf

		int i, i_zero = -1;
		char c, lastc = buf[0];

		// i_zero is the index of the last null character or -1

		for (i=1;i<(int)len;i++)
		{
			c = buf[i];

			if (c == 0)
				i_zero = i;

			if (c == 0 && lastc == 0 && i+1 != len)
			{
//				TRACE("Error: len = %d, reallen = %d, using len = %d\n",len,i+1,i+1);
				len = i+1;
				break;
			}

			lastc = c;
		}

		if (buf[len-1] != 0 || buf[len-2] != 0)
		{
			if (i_zero < 0)
				i_zero = 0;

//			TRACE("Error: missing two 0 at end of buffer, using len = %d\n",i_zero+2);

			buf[i_zero    ] = 0;
			buf[i_zero + 1] = 0;

			len = i_zero + 2;
		}
	}


	newBuf = (BYTE *) Realloc(m_MultiSzBuf, len);
	if (newBuf == NULL)
		return FALSE;

	m_MultiSzBuf = newBuf;
	m_MultiSzLen = len;

	if (buf != NULL && len > 0)
		memcpy(m_MultiSzBuf, buf, m_MultiSzLen);

	return TRUE;
}

void CMultiSz::FirstPosition(int * position) const
{
	* position = 0;
}

void CMultiSz::First()
{
	FirstPosition(&m_MultiSzPos);
}

const char * CMultiSz::NextPosition(int * position) const
{
	const char * str;

	// check if the buffer is empty

	if (m_MultiSzLen < 1)
		return NULL;

	// check if we are already at the end of the list

	if (m_MultiSzBuf[*position] == 0)
		return NULL;

	// get the current string

	str = (const char *) m_MultiSzBuf + *position;

	// compute the starting byte position of the next string

	*position += (int)strlen(str) + 1;

	return str;
}

const char * CMultiSz::Next()
{
	return NextPosition(&m_MultiSzPos);
}

BOOL CMultiSz::AddString(const char *str)
{
	// check if the string is already present

	if (FindString(str))
		return TRUE;

	// if the buffer was empty, add another byte
	if (m_MultiSzLen == 0)
		m_MultiSzLen ++;

	// if not present, add the new string to the end

	DWORD newMultiSzLen = m_MultiSzLen + strlen(str) + 1;
	BYTE * newMultiSzBuf = (BYTE *) Realloc(m_MultiSzBuf, newMultiSzLen);
	if (newMultiSzBuf == NULL)
		return FALSE;

	strcpy((char *)newMultiSzBuf + m_MultiSzLen - 1, str);
	newMultiSzBuf[newMultiSzLen - 1] = 0;

	m_MultiSzLen = newMultiSzLen;
	m_MultiSzBuf = newMultiSzBuf;

	return TRUE;
}

BOOL CMultiSz::RemoveString(const char *str)
{
	const char * s;
	int curPos = 0;

	// check if the string is already present

	First();

	while ((s = Next()) != NULL)
	{
		if (strcmp(str,s) == 0)
		{
			// m_MultiSzPos is initialized to point to the
			// beginning of the next string

			memcpy(m_MultiSzBuf + curPos , m_MultiSzBuf + m_MultiSzPos , m_MultiSzLen - m_MultiSzPos);

			m_MultiSzLen -= m_MultiSzPos - curPos;

			return TRUE;
		}

		// save the position of the beginning of the next
		// string

		curPos = m_MultiSzPos;
	}

	// if not, returns TRUE
	return TRUE;
}

const CMultiSz& CMultiSz::operator = (const CMultiSz& other)
{
	m_MultiSzBuf = NULL;
	if (other.m_MultiSzLen > 0)
	{
		m_MultiSzBuf = (BYTE *) Realloc(m_MultiSzBuf, other.m_MultiSzLen);
		if (m_MultiSzBuf != NULL)
			memcpy(m_MultiSzBuf, other.m_MultiSzBuf, other.m_MultiSzLen);
	}

	m_MultiSzLen = other.m_MultiSzLen;
	m_MultiSzPos = other.m_MultiSzPos;

	return *this;
}

BOOL CMultiSz::FindString(const char *str) const
{
	const char * s;
	int position;

	// check if the string is already present

	FirstPosition(&position);
	while ((s = NextPosition(&position)) != NULL)
	{
		if (strcmp(str,s) == 0)
			return TRUE;
	}

	return FALSE;
}

void CMultiSz::DisplayBuffer()
{
	char msg [ 1000 ];
	char ns [ 10 ];
	int i;

	msg [0] = 0;
	for (i=0;i<(int)m_MultiSzLen; i++)
	{
		if (strlen(msg) > sizeof(msg) - 10)
		{
			TRACE("Truncating buffer\n");
			break;
		}

		char c = m_MultiSzBuf[i];
		if (c < ' ' || c > 0x7f)
			c = ' ';

		sprintf(ns, "%02x/%c ",m_MultiSzBuf [i], c);
		strcat(msg,ns);
	}

}



// ======== CShuttlePN31Client
CShuttlePN31Client::CShuttlePN31Client(void)
{
	m_hPipe		= INVALID_HANDLE_VALUE;
	m_hThread	= NULL;
	m_pWnd		= NULL;
}

CShuttlePN31Client::~CShuttlePN31Client()
{
	Disconnect();
}

void CShuttlePN31Client::SetHWND(HWND hWnd)
{
	m_pWnd = CWnd::FromHandle(hWnd);
}

void CShuttlePN31Client::Connect()
{
	DWORD		dwThreadId;
    m_hPipe = CreateNamedPipe (APP_PIPENAME, PIPE_ACCESS_DUPLEX, 
							   PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
							   PIPE_UNLIMITED_INSTANCES, BUFSIZE, BUFSIZE,
							   NMPWAIT_USE_DEFAULT_WAIT, NULL);

	if (m_hPipe != INVALID_HANDLE_VALUE)
		m_hThread = CreateThread (NULL, 0, ThreadStart, this, 0, &dwThreadId);
}

void CShuttlePN31Client::Disconnect()
{
	DWORD		dwSize;
//	if (m_hPipe != INVALID_HANDLE_VALUE)
//	{
//		HANDLE	hEndMsg;
////		FlushFileBuffers(m_hPipe); 
//		DWORD dwMode = PIPE_NOWAIT;
//		SetNamedPipeHandleState (m_hPipe, &dwMode, NULL, NULL);
//		DisconnectNamedPipe(m_hPipe); 
//		CloseHandle (m_hPipe);
//		m_hPipe = INVALID_HANDLE_VALUE;
//	}

	//if (m_hThread)
	//{
	//	if (WaitForSingleObject (m_hThread, 4000) == WAIT_TIMEOUT)
			TerminateThread (m_hThread, 0xDEAD);
			// TODO !!!!!!
	//	m_hThread = NULL;
	//}
}


DWORD WINAPI CShuttlePN31Client::ThreadStart(LPVOID lpParam)
{
	CShuttlePN31Client*		pThis = (CShuttlePN31Client*) lpParam;
	pThis->WaitKey();
	return 0;
}

void CShuttlePN31Client::WaitKey()
{
   BYTE		chRequest[BUFSIZE]; 
   DWORD	cbBytesRead; 
   BOOL		fSuccess; 
   PIPE_MSG	Msg;


   ConnectNamedPipe(m_hPipe, NULL);
   while (1) 
   { 
      fSuccess = ReadFile(m_hPipe, &Msg, sizeof(Msg), &cbBytesRead, NULL);
	  if (!fSuccess) break;

	  if (Msg.lSize > 0)
		fSuccess = ReadFile(m_hPipe, &chRequest, min(Msg.lSize, sizeof(chRequest)), &cbBytesRead, NULL);


	  switch (Msg.nCode)
	  {
		case PC_LOG :
			chRequest[cbBytesRead] = 0;
			TRACE ("[PN31] %s\n", chRequest);
			break;
		case PC_BULK_DOWN :
			PN31_KEYCODE* pKey = FindPN31Key(chRequest);
			if (pKey != NULL)
			{
#ifdef _CONSOLE
				  printf ("%s\n", pKey->strName);
#endif

				TRACE ("%s\n", pKey->strName);
				ExecuteCommand(pKey->nKey);
			}
/*			for (int i=1; i<8; i++)
			{
				if (chRequest[i] != 0)
				{
					GetAnswerToRequest (chRequest, cbBytesRead);
					break;
				}
			}*/
			break;
	  }
   }
   TRACE ("Stop listening PN31 pipe\n");
}


CMultiSz CShuttlePN31Client::GetLowerFilters(HDEVINFO hDev, SP_DEVINFO_DATA* pDevInfo)
{
	DWORD DataType, BufferSize = 0;
	BYTE * Buffer = NULL;
	BOOL result;
	CMultiSz cszLowerFilters;

	result = SetupDiGetDeviceRegistryProperty(hDev,pDevInfo,SPDRP_LOWERFILTERS,
		&DataType,Buffer,BufferSize,&BufferSize);

	if (BufferSize != 0)
	{
		// we allocate the proper buffer
		Buffer = new BYTE [BufferSize];

		// we get the content of the buffer

		if (SetupDiGetDeviceRegistryProperty(hDev,pDevInfo,SPDRP_LOWERFILTERS,
			&DataType,Buffer,BufferSize,NULL))
		{
			// DataType is REG_MULTI_SZ
			cszLowerFilters.SetBuffer(Buffer, BufferSize);
		}
		else
		{
			cszLowerFilters.SetBuffer(Buffer, BufferSize);
		}

		delete[] Buffer;
	}
	else
	{
		cszLowerFilters.SetBuffer(Buffer, BufferSize);
	}

	return cszLowerFilters;
}

void CShuttlePN31Client::SetLowerFilters(HDEVINFO hDev, SP_DEVINFO_DATA* hDevInfo, const CMultiSz& sz)
{
	// check Windows 2000 / XP versus Windows 98
	BOOL isWin98= (GetVersion() & 0x80000000);

	// check if we can remove the key in the registry
	if (sz.GetBufferLen() > 2)
	{
		if (!SetupDiSetDeviceRegistryProperty(hDev,hDevInfo,SPDRP_LOWERFILTERS ,
			sz.GetBuffer(),sz.GetBufferLen()))
		{
			TRACE("Install failed! : SetLowerFilters\n");
		}
	}
	else
	{
		if (!SetupDiSetDeviceRegistryProperty(hDev,hDevInfo,SPDRP_LOWERFILTERS ,
			NULL,0))
		{
			// windows xp:
			// if the entry has already been deleted, the function will fail with
			// GetLastError() = ERROR_INVALID_DATA. strange :-(

			// windows 98:
			// SetupDiSetDeviceRegistryProperty() will NULL parameters always fails withe
			// GetLastError() = ERROR_INVALID_DATA

			BYTE sEmpty[] = { 0, 0, 0, 0, 0, 0 };

			if(!SetupDiSetDeviceRegistryProperty(hDev, hDevInfo, SPDRP_LOWERFILTERS,
				sEmpty, 2 * sizeof(TCHAR)))
			{
				DWORD dwLastError = GetLastError();

				if (dwLastError != NO_ERROR)
				{
					char msg [100];
					sprintf(msg,"GetLastError() = %d\n",dwLastError);
					TRACE(msg);
				}
			}
		}
	}
}

bool CShuttlePN31Client::Install(void)
{
	return UpdateDriver(true);
}

bool CShuttlePN31Client::Uninstall(void)
{
	return UpdateDriver(false);
}

bool CShuttlePN31Client::UpdateDriver(bool bInstall)
{
	TCHAR			sEnumerator[] = _T("USB");
	HDEVINFO		hDev;
	DWORD			dwIndex;
	SP_DEVINFO_DATA	devInfo;
	DWORD			BufferSize = 0;
	DWORD			DataType;
	bool			bRet = false;
	CString			strHardwareId;
	CString			strDriver;

	dwIndex = 0;
	hDev = SetupDiGetClassDevs(NULL, sEnumerator, NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT);
	if (hDev != INVALID_HANDLE_VALUE)
	{
		memset(&devInfo, 0, sizeof(devInfo));
		devInfo.cbSize = sizeof(devInfo);
		while (SetupDiEnumDeviceInfo(hDev, dwIndex ++, &devInfo))
		{
			SetupDiGetDeviceRegistryProperty(hDev,&devInfo,SPDRP_HARDWAREID,
				&DataType,(BYTE*)strHardwareId.GetBuffer (BUFFER_SIZE),BUFFER_SIZE,&BufferSize);
			strHardwareId.ReleaseBuffer();

			SetupDiGetDeviceRegistryProperty(hDev,&devInfo,SPDRP_DRIVER,
				&DataType,(BYTE*)strDriver.GetBuffer(BUFFER_SIZE),BUFFER_SIZE,&BufferSize);
			strDriver.ReleaseBuffer();

			if ((strHardwareId.Find (_T("Vid_4572&Pid_4572")) != -1) && (strDriver.Find(_T("36FC9E60-C465-11CF-8056-444553540000")) != -1))
			{
				// TODO : création / destruction du service !!!

				CMultiSz sz = GetLowerFilters(hDev, &devInfo);
				if (bInstall)
				{
					sz.AddString ("pn31snoop");
					bRet = CreateService();
				}
				else
				{
					sz.RemoveString ("pn31snoop");
					bRet = DeleteService();
				}
				SetLowerFilters (hDev, &devInfo, sz);
				RestartDevice (hDev, &devInfo);
				bRet = true;
			}
		}
	}
	return bRet;
}


bool CShuttlePN31Client::RestartDevice(HDEVINFO hDev, SP_DEVINFO_DATA* hDevInfo)
{
	SP_PROPCHANGE_PARAMS params;

	// initialize the structure

	memset(&params, 0, sizeof(params));

	// initialize the SP_CLASSINSTALL_HEADER struct at the beginning
	// of the SP_PROPCHANGE_PARAMS struct, so that
	// SetupDiSetClassInstallParams will work

	params.ClassInstallHeader.cbSize = sizeof(params.ClassInstallHeader);
	params.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;

	// initialize SP_PROPCHANGE_PARAMS such that the device will be
	// stopped.

	params.StateChange = DICS_PROPCHANGE;
	params.Scope       = DICS_FLAG_CONFIGSPECIFIC;
	params.HwProfile   = 0; // current profile

	if(!SetupDiSetClassInstallParams(hDev, hDevInfo,
		(PSP_CLASSINSTALL_HEADER) &params,sizeof(SP_PROPCHANGE_PARAMS)))
	{
		return FALSE;
	}

	// we call SetupDiCallClassInstaller() twice for the ECI USB ADSL
	// modem, maybe because it is 2 USB devices. But this is not
	// needed for all USB devices, so it's better for the end user to
	// hit the button "replug" twice.

	if(!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, hDev, hDevInfo))
	{
		return FALSE;
	}
	
	return TRUE;
}


void CShuttlePN31Client::ExecuteCommand(int nKey)
{
	if (m_pWnd)
	{
		switch (nKey)
		{
		// === Commandes vidéo
		case K_PLAYPAUSE :
			m_pWnd->SendMessage(WM_COMMAND, ID_PLAY_PLAYPAUSE);
			break;
		case K_STOP :
			m_pWnd->SendMessage(WM_COMMAND, ID_PLAY_STOP);
			break;
		case K_REWIND :
			m_pWnd->SendMessage(WM_COMMAND, ID_PLAY_DECRATE);
			break;
		case K_FORWARD :
			m_pWnd->SendMessage(WM_COMMAND, ID_PLAY_INCRATE);
			break;

		// === Commandes sous-titre
		case K_PREVIOUS :
			m_pWnd->SendMessage(WM_COMMAND, ID_GOTO_PREV_SUB);
			break;
		case K_NEXT :
			m_pWnd->SendMessage(WM_COMMAND, ID_GOTO_NEXT_SUB);
			break;
		case K_PGDOWN :
			m_pWnd->SendMessage(WM_COMMAND, ID_SHIFT_SUB_DOWN);
			break;
		case K_PGUP :
			m_pWnd->SendMessage(WM_COMMAND, ID_SHIFT_SUB_UP);
			break;

		case K_EXPLORER :
			m_pWnd->SendMessage(WM_COMMAND, ID_VIEW_REMAINING_TIME);
			break;
		case K_FAVORITE :
			m_pWnd->SendMessage(WM_COMMAND, ID_SHADER_TOGGLE);
			break;

			/*
		case  :
			m_pWnd->SendMessage(WM_COMMAND, );
			break;
			*/
		}
	}

	/*
	cmd.Trim();
	if(cmd.IsEmpty()) return;
	cmd.Replace(' ', '_');

	AppSettings& s = AfxGetAppSettings();

	POSITION pos = s.wmcmds.GetHeadPosition();
	while(pos)
	{
		wmcmd wc = s.wmcmds.GetNext(pos);
		CStringA name = CString(wc.name);
		name.Replace(' ', '_');
		if((repcnt == 0 && wc.rmrepcnt == 0 || wc.rmrepcnt > 0 && (repcnt%wc.rmrepcnt) == 0)
		&& (!name.CompareNoCase(cmd) || !wc.rmcmd.CompareNoCase(cmd) || wc.cmd == (WORD)strtol(cmd, NULL, 10)))
		{
			TRACE(_T("CRemoteCtrlClient (calling command): %s\n"), wc.name);
			m_pWnd->SendMessage(WM_COMMAND, wc.cmd);
			break;
		}
	}*/
}


bool CShuttlePN31Client::CreateService() 
{
	bool	bRet = false;
	
	SC_HANDLE hManager = OpenSCManager(NULL,SERVICES_ACTIVE_DATABASE,SC_MANAGER_ALL_ACCESS);

	if (hManager == NULL)
	{
		TRACE ("Can't open service manager");
	}
	else
	{
		SC_HANDLE hService = ::CreateService(hManager,PN31SNOOP_SERVICE,"pn31snoop (display)",
			SERVICE_ALL_ACCESS,SERVICE_KERNEL_DRIVER,SERVICE_DEMAND_START,SERVICE_ERROR_NORMAL,
			"System32\\DRIVERS\\PN31SNOOP.SYS",
			NULL,NULL,NULL,NULL,NULL);

		if (hService == NULL)
		{
			TRACE("Can't create service");
		}
		else
		{
			CloseServiceHandle(hService);
			bRet = true;
		}

		CloseServiceHandle(hManager);
	}

	return bRet;
}

bool CShuttlePN31Client::DeleteService() 
{
	bool		bRet = false;
	
	SC_HANDLE hManager = OpenSCManager(NULL,SERVICES_ACTIVE_DATABASE,SC_MANAGER_ALL_ACCESS);

	if (hManager == NULL)
	{
		TRACE("Can't open service manager");
	}
	else
	{
		SC_HANDLE hService = OpenService(hManager,PN31SNOOP_SERVICE,SC_MANAGER_ALL_ACCESS);
		if (hService == NULL)
		{
			TRACE("Can't open service");
		}
		else
		{
			if (!::DeleteService(hService))
				TRACE("Can't delete service");
			else
				bRet = true;
			CloseServiceHandle(hService);
		}

		CloseServiceHandle(hManager);
	}
	return bRet;
}

