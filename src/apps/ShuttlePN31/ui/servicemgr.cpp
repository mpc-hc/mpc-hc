// servicemgr.cpp: implementation of the CServiceMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sniffusb.h"
#include "servicemgr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CServiceMgr::CServiceMgr()
{
	m_hManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
}

CServiceMgr::~CServiceMgr()
{
	if (m_hManager != NULL)
		CloseServiceHandle(m_hManager);
}
