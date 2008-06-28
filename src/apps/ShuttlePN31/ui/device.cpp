// device.cpp: implementation of the CDevice class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sniffusb.h"
#include "device.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDevice::CDevice()
{
	m_hDev = NULL;
	memset(&m_hDevInfo, 0, sizeof(m_hDevInfo));
	b_IsPresent = FALSE;
}

CDevice::CDevice(HDEVINFO hDev,SP_DEVINFO_DATA devInfo)
{
	m_hDev = hDev;
	m_hDevInfo = devInfo;
	b_IsPresent = FALSE;
}

CDevice::~CDevice()
{
}

BOOL CDevice::Restart()
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

	if(!SetupDiSetClassInstallParams(m_hDev, &m_hDevInfo,
		(PSP_CLASSINSTALL_HEADER) &params,sizeof(SP_PROPCHANGE_PARAMS)))
	{
		return FALSE;
	}

	// we call SetupDiCallClassInstaller() twice for the ECI USB ADSL
	// modem, maybe because it is 2 USB devices. But this is not
	// needed for all USB devices, so it's better for the end user to
	// hit the button "replug" twice.

	if(!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, m_hDev, &m_hDevInfo))
	{
		return FALSE;
	}
	
	return TRUE;
}

void Perror(DWORD dwLastError, LPCTSTR str)
{
	char msg [100];

	sprintf(msg, "GetLastError() = %d\n",dwLastError);
	MessageBox(NULL,msg,str,MB_OK);
}

const CMultiSz& CDevice::GetLowerFilters()
{
	DWORD DataType, BufferSize = 0;
	BYTE * Buffer = NULL;
	BOOL result;

	static int once = 0;

	// first, we compute the required size of the buffer, this is
	// need on windows 98 for instance, because otherwise it will
	// return OUR buffer size instead.

	// warning: on windows xp, the following call will alway
	// return FALSE and set GetLastError() = 13
	// [ERROR_INVALID_DATA].

	result = SetupDiGetDeviceRegistryProperty(m_hDev,&m_hDevInfo,SPDRP_LOWERFILTERS,
		&DataType,Buffer,BufferSize,&BufferSize);

	if (BufferSize != 0)
	{
		// we allocate the proper buffer

		Buffer = (BYTE *) malloc(BufferSize);

		// we get the content of the buffer

		if (SetupDiGetDeviceRegistryProperty(m_hDev,&m_hDevInfo,SPDRP_LOWERFILTERS,
			&DataType,Buffer,BufferSize,NULL))
		{
			// DataType is REG_MULTI_SZ
			m_LowerFilters.SetBuffer(Buffer, BufferSize);
/*
			if (!once && BufferSize > 0)
			{
				once = 1;

				char msg[100];
				sprintf(msg,"DataType = %d, BufferSize = %d",
					DataType, BufferSize);
				MessageBox(NULL,msg,"CDevice::GetLowerFilters",MB_OK);

				m_LowerFilters.DisplayBuffer();
			}
*/
		}
		else
		{
			TRACE("SetupDiGetDeviceRegistryProperty [%s] = KO\n",m_DeviceDesc);
/*
			Perror(GetLastError(),m_FriendlyName);

			char msg[100];
			sprintf(msg,"DataType = %d, BufferSize = %d",
				DataType, BufferSize);
			MessageBox(NULL,msg,"CDevice::GetLowerFilters",MB_OK);
*/
			m_LowerFilters.SetBuffer(Buffer, BufferSize);
			//m_LowerFilters.DisplayBuffer();
		}

		free (Buffer);
	}
	else
	{
		m_LowerFilters.SetBuffer(Buffer, BufferSize);
	}

	return m_LowerFilters;
}

void CDevice::SetLowerFilters(const CMultiSz& sz)
{
	// check Windows 2000 / XP versus Windows 98
	BOOL isWin98= (GetVersion() & 0x80000000);
	DWORD dwLastError;

	// windows98: the Setup API is too much buggy to be used, so
	// we use the registry instead

	// windowsxp: the registry is read only, so it cannot be used.

	if (isWin98)
	{
		TCHAR szRootKey[MAX_PATH];
		HKEY hRootKey = NULL;

		if (isWin98)
		{
			_tcscpy(szRootKey, _T("Enum\\USB"));
		}
		else
		{
			_tcscpy(szRootKey, _T("SYSTEM\\CurrentControlSet\\Enum\\USB"));
		}                                  

		dwLastError = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRootKey, 0, KEY_READ, &hRootKey);
		if (dwLastError == ERROR_SUCCESS)
		{
			TRACE("hRootKey OK\n");

			DWORD dwSubIndex = 0;
			TCHAR szSubName[MAX_PATH];

			while(RegEnumKey(hRootKey, dwSubIndex, szSubName, MAX_PATH) == ERROR_SUCCESS)
			{
				TRACE("szSubName = %s\n",szSubName);

				HKEY hSubKey = NULL;

				dwLastError = RegOpenKeyEx(hRootKey, szSubName, 0, KEY_READ, &hSubKey);
				if (dwLastError == ERROR_SUCCESS)
				{
					TRACE("hSubKey OK\n");

					DWORD dwInstIndex = 0;
					TCHAR szInstKey[MAX_PATH];

					while (RegEnumKey(hSubKey, dwInstIndex, szInstKey, MAX_PATH) == ERROR_SUCCESS)
					{
						HKEY hInstKey = NULL;

						TRACE("szInstKey = %s\n",szInstKey);

						dwLastError = RegOpenKeyEx(hSubKey,szInstKey,0, KEY_READ | KEY_SET_VALUE, &hInstKey);
						if (dwLastError == ERROR_SUCCESS)
						{
							TRACE("hInstKey OK\n");

							TCHAR szDriver[] = _T("Driver");
							TCHAR szLowerFilters[] = _T("LowerFilters");
							TCHAR szDriverVal[MAX_PATH];
							DWORD  cbDriverVal = MAX_PATH;
							DWORD dwType = REG_SZ;

							dwLastError = RegQueryValueEx(hInstKey,szDriver,NULL,&dwType,(LPBYTE)szDriverVal,&cbDriverVal);
							if (dwLastError == ERROR_SUCCESS)
							{
								TRACE("szDriverVal = %s\n",szDriverVal);

								if (m_Driver == szDriverVal)
								{

									TRACE("Installing ...\n");

									if (sz.GetBufferLen() > 2)
									{
										// set the new value

										dwLastError = RegSetValueEx(hInstKey,szLowerFilters,0, REG_MULTI_SZ, sz.GetBuffer(), sz.GetBufferLen());
										if (dwLastError != ERROR_SUCCESS)
										{
											TRACE("RegSetValueEx = %d\n",dwLastError);
										}
									}
									else
									{
										// remove the key

										dwLastError = RegDeleteValue(hInstKey,szLowerFilters);
										if (dwLastError != ERROR_SUCCESS)
										{
											TRACE("RegDeleteValue = %d\n",dwLastError);
										}
									}
								}
							}

							RegCloseKey(hInstKey);
							hInstKey = NULL;
						}

						dwInstIndex++;
					}

					RegCloseKey(hSubKey);
					hSubKey = NULL;
				}

				dwSubIndex ++;
			}

			RegCloseKey(hRootKey);
			hRootKey = NULL;
		}
	}
	else
	{
		// check if we can remove the key in the registry

		if (sz.GetBufferLen() > 2)
		{
			if (!SetupDiSetDeviceRegistryProperty(m_hDev,&m_hDevInfo,SPDRP_LOWERFILTERS ,
				sz.GetBuffer(),sz.GetBufferLen()))
			{
				MessageBox(NULL,_T("Install failed!"),_T("SetLowerFilters"),MB_OK);
			}
		}
		else
		{
			if (!SetupDiSetDeviceRegistryProperty(m_hDev,&m_hDevInfo,SPDRP_LOWERFILTERS ,
				NULL,0))
			{
				// windows xp:
				// if the entry has already been deleted, the function will fail with
				// GetLastError() = ERROR_INVALID_DATA. strange :-(

				// windows 98:
				// SetupDiSetDeviceRegistryProperty() will NULL parameters always fails withe
				// GetLastError() = ERROR_INVALID_DATA

				BYTE sEmpty[] = { 0, 0, 0, 0, 0, 0 };

				if(!SetupDiSetDeviceRegistryProperty(m_hDev, &m_hDevInfo, SPDRP_LOWERFILTERS,
					sEmpty, 2 * sizeof(TCHAR)))
				{
					DWORD dwLastError = GetLastError();

					if (dwLastError != NO_ERROR)
					{
						char msg [100];
						sprintf(msg,"GetLastError() = %d",dwLastError);
						MessageBox(NULL,msg,_T("SetLowerFilters"),MB_OK);
					}
				}
			}
		}
	}
}
