// devicemgr.h: interface for the CDeviceMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DEVICEMGR_H__3D8E001E_8F97_4FDF_A282_95C8D77F56D3__INCLUDED_)
#define AFX_DEVICEMGR_H__3D8E001E_8F97_4FDF_A282_95C8D77F56D3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "device.h"

#include <setupapi.h>
#include <afxtempl.h>

/*
	To loop over all USB devices, use the following code:

  	CDeviceMgr mgr;
	POSITION pos;

	pos = mgr.GetHeadPosition();
	while (pos != NULL)
	{
		CDevice& device = mgr.GetNext(pos);

		... use device ...
	}
*/


class CDeviceMgr  
{
public:
	void Dump();
	POSITION FindDevice(CString hardwareID);
	CDevice& GetFoundDevice(POSITION pos);

	inline POSITION GetHeadPosition() { return deviceList.GetHeadPosition(); };
	inline CDevice& GetNext(POSITION& pos) { return deviceList.GetNext(pos); };

	CDeviceMgr();
	virtual ~CDeviceMgr();

private:
	CList<CDevice,const CDevice&> deviceList;
	HDEVINFO m_hDev;
};

#endif // !defined(AFX_DEVICEMGR_H__3D8E001E_8F97_4FDF_A282_95C8D77F56D3__INCLUDED_)
