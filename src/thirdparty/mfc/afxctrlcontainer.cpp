// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.
//
// CWnd support for MFC Control containment (Feature Pack controls)
//

#include "stdafx.h"
#undef SubclassWindow
#include "afxctrlcontainer.h"
#if 0
#include "afxtagmanager.h"
#include "afxbutton.h"
#include "afxcolorbutton.h"
#include "afxeditbrowsectrl.h"
#include "afxfontcombobox.h"
#include "afxlinkctrl.h"
#include "afxmaskededit.h"
#include "afxmenubutton.h"
#include "afxpropertygridctrl.h"
#include "afxshelllistctrl.h"
#include "afxshelltreectrl.h"
#include "afxvslistbox.h"
#endif
////////////////////////////////////////////////////////////////////////////

static void DoRegisterWindowClass(LPCTSTR lpszClassName, LPCTSTR lpszBaseClassName)
{
	ASSERT(lpszClassName != NULL);
	ASSERT(lpszBaseClassName != NULL);

	WNDCLASS wnd = {0};

	HINSTANCE hInst = AfxGetInstanceHandle();
	if (!AfxCtxGetClassInfo(hInst, lpszBaseClassName, &wnd))
	{
		wnd.style = CS_DBLCLKS;
		wnd.hInstance = hInst;
		wnd.lpfnWndProc = ::DefWindowProc;
	}

	wnd.lpszClassName = lpszClassName;
	AfxRegisterClass(&wnd);
}

void AfxRegisterMFCCtrlClasses()
{
	DoRegisterWindowClass(_T("MFCButton"), WC_BUTTON);
	DoRegisterWindowClass(_T("MFCColorButton"), WC_BUTTON);
	DoRegisterWindowClass(_T("MFCEditBrowse"), WC_EDIT);
	DoRegisterWindowClass(_T("MFCFontComboBox"), WC_COMBOBOX);
	DoRegisterWindowClass(_T("MFCLink"), WC_BUTTON);
	DoRegisterWindowClass(_T("MFCMaskedEdit"), WC_EDIT);
	DoRegisterWindowClass(_T("MFCMenuButton"), WC_BUTTON);
	DoRegisterWindowClass(_T("MFCPropertyGrid"), WC_STATIC);
	DoRegisterWindowClass(_T("MFCShellList"), WC_LISTVIEW);
	DoRegisterWindowClass(_T("MFCShellTree"), WC_TREEVIEW);
	DoRegisterWindowClass(_T("MFCVSListBox"), WC_STATIC);
}

////////////////////////////////////////////////////////////////////////////
// CMFCControlContainer

CMFCControlContainer::CMFCControlContainer(CWnd* pWnd) : m_pWnd(pWnd)
{
}

CMFCControlContainer::~CMFCControlContainer()
{
	FreeSubclassedControls();
	ClearControlData();
}

BOOL CMFCControlContainer::SubclassDlgControls()
{
	if (m_pWnd->GetSafeHwnd() != NULL)
	{
		// Subclass Feature Pack controls:
		CWnd* pWndChild = m_pWnd->GetWindow(GW_CHILD);
		while (pWndChild != NULL)
		{
			ASSERT_VALID(pWndChild);

			TCHAR lpszClassName [MAX_CLASS_NAME + 1];

			::GetClassName(pWndChild->GetSafeHwnd(), lpszClassName, MAX_CLASS_NAME);
			CWnd* pWndSubclassedCtrl = CreateDlgControl(lpszClassName);

			if (pWndSubclassedCtrl != NULL)
			{
				m_arSubclassedCtrls.Add((CObject*)pWndSubclassedCtrl);
				pWndSubclassedCtrl->SubclassWindow(pWndChild->GetSafeHwnd());
			}

			pWndChild = pWndChild->GetNextWindow();
		}

		return TRUE;
	}

	return FALSE;
}

void CMFCControlContainer::FreeSubclassedControls()
{
	// Free subclassed controls: 
	for (int i = 0; i < m_arSubclassedCtrls.GetCount(); i++)
	{
		if (m_arSubclassedCtrls [i] != NULL)
		{
			delete m_arSubclassedCtrls [i];
		}
	}
	m_arSubclassedCtrls.RemoveAll();
}

CWnd* CMFCControlContainer::CreateDlgControl(LPCTSTR lpszClassName)
{
	ASSERT(m_pWnd->GetSafeHwnd() != NULL);

	if (lpszClassName != NULL)
	{
		CString strClass = lpszClassName;
		CWnd* pWndSubclassedCtrl = NULL;
#if 0
		if (strClass == _T("MFCButton"))
		{
			pWndSubclassedCtrl = new CMFCButton;
		}
		else if (strClass == _T("MFCColorButton"))
		{
			pWndSubclassedCtrl = new CMFCColorButton;
		}
		else if (strClass == _T("MFCEditBrowse"))
		{
			pWndSubclassedCtrl = new CMFCEditBrowseCtrl;
		}
		else if (strClass == _T("MFCFontComboBox"))
		{
			pWndSubclassedCtrl = new CMFCFontComboBox;
		}
		else if (strClass == _T("MFCLink"))
		{
			pWndSubclassedCtrl = new CMFCLinkCtrl;
		}
		else if (strClass == _T("MFCMaskedEdit"))
		{
			pWndSubclassedCtrl = new CMFCMaskedEdit;
		}
		else if (strClass == _T("MFCMenuButton"))
		{
			pWndSubclassedCtrl = new CMFCMenuButton;
		}
		else if (strClass == _T("MFCPropertyGrid"))
		{
			pWndSubclassedCtrl = new CMFCPropertyGridCtrl;
		}
		else if (strClass == _T("MFCShellList"))
		{
			pWndSubclassedCtrl = new CMFCShellListCtrl;
		}
		else if (strClass == _T("MFCShellTree"))
		{
			pWndSubclassedCtrl = new CMFCShellTreeCtrl;
		}
		else if (strClass == _T("MFCVSListBox"))
		{
			pWndSubclassedCtrl = new CVSListBox;
		}
#endif
		return pWndSubclassedCtrl;
	}

	return NULL;
}

BOOL CMFCControlContainer::IsSubclassedFeaturePackControl(HWND hWndCtrl)
{
	if (hWndCtrl == NULL)
	{
		return FALSE;
	}

	for (int i = 0; i < m_arSubclassedCtrls.GetCount(); i++)
	{
		CWnd* pWnd = (CWnd*)m_arSubclassedCtrls[i];
		if (pWnd->GetSafeHwnd() == hWndCtrl)
		{
			return TRUE;
		}
	}

	return FALSE;
}

void CMFCControlContainer::PreUnsubclassControl(CWnd* pControl)
{
	UNREFERENCED_PARAMETER(pControl);

//	CMFCShellListCtrl* pListCtrl = DYNAMIC_DOWNCAST(CMFCShellListCtrl, pControl);
//	if (pListCtrl != NULL && pListCtrl->GetHeaderCtrl().GetSafeHwnd() != NULL)
//	{
//		pListCtrl->GetHeaderCtrl().UnsubclassWindow();
//	}
}

BOOL CMFCControlContainer::ReSubclassControl(HWND hWndCtrl, WORD nIDC, CWnd& control)
{
	if (hWndCtrl == NULL)
	{
		return FALSE;
	}

	int nIndex = -1;
	for (int i = 0; i < m_arSubclassedCtrls.GetCount(); i++)
	{
		CWnd* pWnd = (CWnd*)m_arSubclassedCtrls [i];
		if (pWnd->GetSafeHwnd() == hWndCtrl)
		{
			nIndex = i;
			break;
		}
	}

	if (nIndex != -1)
	{
		CWnd* pWnd = DYNAMIC_DOWNCAST(CWnd, m_arSubclassedCtrls [nIndex]);
		
		if (pWnd->GetSafeHwnd() != NULL)
		{
			ASSERT_VALID(pWnd);

			// get init state
			DWORD dwSize = 0;
			BYTE* pbInitData = NULL;
			GetControlData(nIDC, dwSize, pbInitData);

			// Free old subclassed control:
			m_arSubclassedCtrls [nIndex] = NULL;

			// unsubclass
			PreUnsubclassControl(pWnd);
			VERIFY(pWnd->UnsubclassWindow() == hWndCtrl);
			// destroy
			delete pWnd;

			// subclass
			if (!control.SubclassWindow(hWndCtrl))
			{
				ASSERT(FALSE);      // possibly trying to subclass twice?
				AfxThrowNotSupportedException();
			}

			// set init state
			if (dwSize > 0)
			{
				control.SendMessage(WM_MFC_INITCTRL, (WPARAM)dwSize, (LPARAM)pbInitData);
			}

			return TRUE;
		}
	}

	return FALSE;
}

void CMFCControlContainer::SetControlData(WORD nIDC, DWORD dwSize, BYTE* pbData)
{
	CByteArray* pArray = new CByteArray;
	pArray->SetSize(dwSize);

	BYTE* pbBuffer = pArray->GetData();
	if (memcpy_s(pbBuffer, dwSize, pbData, dwSize) != 0)
	{
		delete pArray;
		ASSERT(FALSE);
		return;
	}

	m_mapControlData.SetAt(nIDC, pArray);
}

BOOL CMFCControlContainer::GetControlData(WORD nIDC, DWORD& dwSize, BYTE*& pbData)
{
	CObject* pData = NULL;
	if (m_mapControlData.Lookup(nIDC, pData) && pData != NULL)
	{
		CByteArray* pArray = (CByteArray*)pData;
		dwSize = (DWORD)pArray->GetSize();
		pbData = pArray->GetData();
		return TRUE;
	}

	return FALSE;
}

void CMFCControlContainer::ClearControlData()
{
	WORD nIDC;
	CObject* pData = NULL;
	POSITION pos = m_mapControlData.GetStartPosition();
	while (pos != NULL)
	{
		m_mapControlData.GetNextAssoc(pos, nIDC, pData); 
		CByteArray* pArray = (CByteArray*)pData; 
		delete pArray;
	}

	m_mapControlData.RemoveAll();
}

////////////////////////////////////////////////////////////////////////////
// Accessing dialog DLGINIT helpers

int __stdcall CMFCControlContainer::UTF8ToString(LPCSTR lpSrc, CString& strDst, int nLength)
{
	LPTSTR lpDst = NULL;
	int count = ::MultiByteToWideChar(CP_UTF8, 0, lpSrc, nLength, NULL, 0);
	if (count <= 0)
	{
		return 0;
	}

	LPWSTR lpWide = new WCHAR[count + 1];
	memset(lpWide, 0, (count + 1) * sizeof(WCHAR));

	::MultiByteToWideChar(CP_UTF8, 0, lpSrc, nLength, lpWide, count);

#ifdef _UNICODE
	lpDst = lpWide;
#else
	count = ::WideCharToMultiByte(::GetACP(), 0, lpWide, -1, NULL, 0, NULL, 0);

	if (count > 0)
	{
		lpDst = new char[count + 1];
		memset(lpDst, 0, count + 1);

		::WideCharToMultiByte(::GetACP(), 0, lpWide, -1, lpDst, count, NULL, 0);
	}

	delete [] lpWide;
#endif

	strDst = lpDst;
	delete[] lpDst;
	return count;
}

BOOL __stdcall CMFCControlContainer::ReadBoolProp(CTagManager& /*tagManager*/, LPCTSTR lpszTag, BOOL& bMember)
{
	if (lpszTag == NULL)
	{
		return FALSE;
	}

	CString str;
//	tagManager.ExcludeTag(lpszTag, str);

	if (str.IsEmpty())
	{
		return FALSE;
	}

	bMember = (str.CompareNoCase(PS_True) == 0);
	return TRUE;
}
