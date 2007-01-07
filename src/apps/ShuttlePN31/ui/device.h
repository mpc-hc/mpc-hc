// device.h: interface for the CDevice class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DEVICE_H__DB919D0F_E009_4D6B_90F8_55286A86D8BA__INCLUDED_)
#define AFX_DEVICE_H__DB919D0F_E009_4D6B_90F8_55286A86D8BA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "multisz.h"

#include <setupapi.h>

class CDevice  
{
public:
	BOOL Restart();
	// we need a default constructor for CList
	CDevice();
	CDevice(HDEVINFO hDev, SP_DEVINFO_DATA devInfo);
	virtual ~CDevice();

	inline const CString& GetFriendlyName() { return m_FriendlyName; };
	inline void SetFriendlyName(const CString& s) { m_FriendlyName = s; };

	inline const CString& GetHardwareID() { return m_HardwareID; };
	inline void SetHardwareID(const CString& s) { m_HardwareID = s; };

	inline const CString& GetDeviceDesc() { return m_DeviceDesc; };
	inline void SetDeviceDesc(const CString& s) { m_DeviceDesc = s; };

	inline const CString& GetMfg() { return m_Mfg; };
	inline void SetMfg(const CString& s) { m_Mfg = s; };

	inline const CString& GetDriver() { return m_Driver; };
	inline void SetDriver(const CString& s) { m_Driver = s; };

	inline BOOL GetPresent() { return b_IsPresent; };
	inline void SetPresent(BOOL isPresent) { b_IsPresent = isPresent; };

	const CMultiSz & GetLowerFilters();
	void SetLowerFilters(const CMultiSz& s);

private:
	CString m_HardwareID;
	CString m_FriendlyName;
	CString m_DeviceDesc;
	CString m_Mfg;
	CString m_Driver;
	CMultiSz m_LowerFilters;
	BOOL    b_IsPresent;
	HDEVINFO m_hDev;
	SP_DEVINFO_DATA m_hDevInfo;
};

#endif // !defined(AFX_DEVICE_H__DB919D0F_E009_4D6B_90F8_55286A86D8BA__INCLUDED_)
