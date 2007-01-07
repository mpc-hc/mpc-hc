// servicemgr.h: interface for the CServiceMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERVICEMGR_H__DA642D9C_B64E_4D1B_B46E_350B43D13453__INCLUDED_)
#define AFX_SERVICEMGR_H__DA642D9C_B64E_4D1B_B46E_350B43D13453__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <winsvc.h>

class CServiceMgr  
{
public:
	CServiceMgr();
	virtual ~CServiceMgr();

private:
	SC_HANDLE m_hManager;
};

#endif // !defined(AFX_SERVICEMGR_H__DA642D9C_B64E_4D1B_B46E_350B43D13453__INCLUDED_)
