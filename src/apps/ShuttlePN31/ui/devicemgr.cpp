// devicemgr.cpp: implementation of the CDeviceMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sniffusb.h"
#include "devicemgr.h"
#include "multisz.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDeviceMgr::CDeviceMgr()
{
	TCHAR sEnumerator[] = _T("USB");
	HDEVINFO hDev;
	DWORD dwIndex;
	SP_DEVINFO_DATA devInfo;

	BYTE Buffer[300];
	DWORD BufferSize = 0;
	DWORD DataType;

	m_hDev = SetupDiGetClassDevs(NULL, sEnumerator, NULL, DIGCF_ALLCLASSES);
	if (m_hDev != INVALID_HANDLE_VALUE)
	{
		dwIndex = 0;

		memset(&devInfo, 0, sizeof(devInfo));
		devInfo.cbSize = sizeof(devInfo);

		while (SetupDiEnumDeviceInfo(m_hDev, dwIndex ++, &devInfo))
		{
			CDevice device(m_hDev, devInfo);

			if (SetupDiGetDeviceRegistryProperty(m_hDev,&devInfo,SPDRP_DEVICEDESC,
				&DataType,Buffer,sizeof(Buffer),&BufferSize))
			{
				device.SetDeviceDesc((char *) Buffer);
			}
			if (SetupDiGetDeviceRegistryProperty(m_hDev,&devInfo,SPDRP_FRIENDLYNAME,
				&DataType,Buffer,sizeof(Buffer),&BufferSize))
			{
				device.SetFriendlyName((char *) Buffer);
			}
			if (SetupDiGetDeviceRegistryProperty(m_hDev,&devInfo,SPDRP_HARDWAREID,
				&DataType,Buffer,sizeof(Buffer),&BufferSize))
			{
				device.SetHardwareID((char *) Buffer);
			}
			if (SetupDiGetDeviceRegistryProperty(m_hDev,&devInfo,SPDRP_MFG,
				&DataType,Buffer,sizeof(Buffer),&BufferSize))
			{
				device.SetMfg((char *) Buffer);
			}
			if (SetupDiGetDeviceRegistryProperty(m_hDev,&devInfo,SPDRP_DRIVER,
				&DataType,Buffer,sizeof(Buffer),&BufferSize))
			{
				device.SetDriver((char *) Buffer);
			}
			deviceList.AddTail(device);
		}
	}

	hDev = SetupDiGetClassDevs(NULL, sEnumerator, NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT);
	if (hDev != INVALID_HANDLE_VALUE)
	{
		dwIndex = 0;

		memset(&devInfo, 0, sizeof(devInfo));
		devInfo.cbSize = sizeof(devInfo);

		while (SetupDiEnumDeviceInfo(hDev, dwIndex ++, &devInfo))
		{
			if (SetupDiGetDeviceRegistryProperty(hDev,&devInfo,SPDRP_HARDWAREID,
				&DataType,Buffer,sizeof(Buffer),&BufferSize))
			{
				CString hardwareID;
				POSITION pos;
				
				hardwareID = ((const char * )Buffer);
				pos = FindDevice(hardwareID);
				if (pos)
					GetFoundDevice(pos).SetPresent(TRUE);
			}
		}
	}
	SetupDiDestroyDeviceInfoList(hDev);

}

CDeviceMgr::~CDeviceMgr()
{
	SetupDiDestroyDeviceInfoList(m_hDev);
}

POSITION CDeviceMgr::FindDevice(CString hardwareID)
{
	POSITION cur, next;

	cur = deviceList.GetHeadPosition();
	while (cur != NULL)
	{
		next = cur;
		CDevice& r = deviceList.GetNext(next);

		if (r.GetHardwareID() == hardwareID)
			break;

		cur = next;
	}

	return cur;
}

CDevice& CDeviceMgr::GetFoundDevice(POSITION pos)
{
	return deviceList.GetAt(pos);
}

void CDeviceMgr::Dump()
{
	POSITION pos;

	pos = GetHeadPosition();
	while (pos != NULL)
	{
		CDevice& r = GetNext(pos);

		TRACE("---> HardwareID = %s\n",(const char *)r.GetHardwareID());
		TRACE("     DeviceDesc = %s\n",(const char *)r.GetDeviceDesc());
		TRACE("     FriendlyName = %s\n",(const char *)r.GetFriendlyName());
		TRACE("     Mfg = %s\n",(const char *)r.GetMfg());
//		TRACE("     LowerFilters = %s\n",(const char *)r.GetLowerFilters());
		TRACE("     Present = %s\n",r.GetPresent() ? _T("TRUE") : _T("FALSE"));
	}
}

