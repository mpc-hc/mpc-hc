/*
 * $Id$
 *
 * (C) 2003-2005 Gabest
 *
 * This file is part of vsrip.
 *
 * Vsrip is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vsrip is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include <afxpriv.h>
#include "VSRip.h"
#include "VSRipDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CVSRipDlg dialog

CVSRipDlg::CVSRipDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CVSRipDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_dlgpos = NULL;

	m_pVSFRipper = new CVobSubFileRipper();
}

CVSRipDlg::~CVSRipDlg()
{
	m_pVSFRipper->SetCallBack(NULL);
}

void CVSRipDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DLGRECT, m_dlgrect);
	DDX_Control(pDX, IDC_HEADERSEP, m_hdrline);
	DDX_Control(pDX, IDC_FOOTERSEP, m_ftrline);
}

void CVSRipDlg::ShowNext()
{
	POSITION prev = m_dlgpos;
	m_dlgpos = GetNext();

	if(prev && prev != m_dlgpos)
	{
		m_dlgs.GetAt(prev)->OnNext();
		m_dlgs.GetAt(prev)->ShowWindow(SW_HIDE);
	}

	if(m_dlgpos)
	{
		CVSRipPage* pDlg = m_dlgs.GetAt(m_dlgpos);
		CRect dr;
		m_dlgrect.GetClientRect(dr);
		m_dlgrect.MapWindowPoints(this, dr);
		pDlg->MoveWindow(dr);
		pDlg->ShowWindow(SW_SHOWNORMAL);
		Invalidate();
	}
}

void CVSRipDlg::ShowPrev()
{
	POSITION next = m_dlgpos;
	m_dlgpos = GetPrev();

	if(next && next != m_dlgpos)
	{
		m_dlgs.GetAt(next)->OnPrev();
		m_dlgs.GetAt(next)->ShowWindow(SW_HIDE);
	}

	if(m_dlgpos)
	{
		CVSRipPage* pDlg = m_dlgs.GetAt(m_dlgpos);
		CRect dr;
		m_dlgrect.GetClientRect(dr);
		m_dlgrect.MapWindowPoints(this, dr);
		pDlg->MoveWindow(dr);
		pDlg->ShowWindow(SW_SHOWNORMAL);
		Invalidate();
	}
}

POSITION CVSRipDlg::GetNext()
{
	POSITION pos = m_dlgpos;
	if(pos && m_dlgs.GetAt(pos)->CanGoNext()) m_dlgs.GetNext(pos);
	else if(pos && !m_dlgs.GetAt(pos)->CanGoNext()) pos = NULL;
	else pos = m_dlgs.GetHeadPosition();
	return(pos);
}

POSITION CVSRipDlg::GetPrev()
{
	POSITION pos = m_dlgpos;
	if(pos && m_dlgs.GetAt(pos)->CanGoPrev()) m_dlgs.GetPrev(pos);
	else if(pos && !m_dlgs.GetAt(pos)->CanGoPrev()) pos = NULL;
	else pos = m_dlgs.GetTailPosition();
	return(pos);
}

BEGIN_MESSAGE_MAP(CVSRipDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE_VOID(WM_KICKIDLE, OnKickIdle)
	ON_BN_CLICKED(IDC_BUTTON1, OnPrev)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON1, OnUpdatePrev)
	ON_BN_CLICKED(IDC_BUTTON2, OnNext)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON2, OnUpdateNext)
	ON_BN_CLICKED(IDCANCEL, OnClose)
	ON_UPDATE_COMMAND_UI(IDCANCEL, OnUpdateClose)
END_MESSAGE_MAP()

// CVSRipDlg message handlers

BOOL CVSRipDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	if(CMenu* pSysMenu = GetSystemMenu(FALSE))
	{
		pSysMenu->RemoveMenu(SC_SIZE, MF_BYCOMMAND);
		pSysMenu->RemoveMenu(SC_MAXIMIZE, MF_BYCOMMAND);
	}

	CRect cr;
	GetClientRect(cr);
	CRect r;
	m_hdrline.GetClientRect(r);
	m_hdrline.MapWindowPoints(this, r);
	r.left = 0;
	r.right = cr.right;
	r.bottom+=2;
	m_hdrline.MoveWindow(r);
	m_ftrline.GetClientRect(r);
	m_ftrline.MapWindowPoints(this, r);
	r.left = 0;
	r.right = cr.right;
	r.bottom+=2;
	m_ftrline.MoveWindow(r);

	m_pVSFRipper = new CVobSubFileRipper();

	CAutoPtr<CVSRipPage> pPage;

	pPage.Attach(new CVSRipFileDlg(m_pVSFRipper));
	pPage->Create(CVSRipFileDlg::IDD, this);
	m_dlgs.AddTail(pPage);

	pPage.Attach(new CVSRipPGCDlg(m_pVSFRipper));
	pPage->Create(CVSRipPGCDlg::IDD, this);
	m_dlgs.AddTail(pPage);

	pPage.Attach(new CVSRipIndexingDlg(m_pVSFRipper));
	pPage->Create(CVSRipIndexingDlg::IDD, this);
	m_dlgs.AddTail(pPage);

	if(__argc > 1)
	{
		m_pVSFRipper->SetCallBack((IVSFRipperCallback*)m_dlgs.GetTail());

		if(S_OK != m_pVSFRipper->LoadParamFile(CString(__targv[1])))
		{
			AfxMessageBox(_T("Error parsing parameter file!"), MB_OK);
			EndDialog(IDCANCEL);
			return FALSE;
		}

		ShowPrev();
	}
	else
	{
		ShowNext();
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CVSRipDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CPaintDC dc(this); // device context for painting

		CWnd* pHdrSep = GetDlgItem(IDC_HEADERSEP);
		CRect r;
		m_hdrline.GetClientRect(r);
		m_hdrline.MapWindowPoints(this, r);
		CRect cr;
		GetClientRect(cr);
		dc.FillSolidRect(CRect(0,0,cr.right,r.top), 0xffffff);

		if(m_dlgpos)
		{
			CVSRipPage* pWnd = m_dlgs.GetAt(m_dlgpos);
			CFont hdrfont, descfont;
			hdrfont.CreateFont(16,0,0,0,FW_BOLD,0,0,0,DEFAULT_CHARSET,
							   OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,DEFAULT_PITCH,
							   _T("Times New Roman"));
			descfont.CreateFont(14,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,
								OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,DEFAULT_PITCH,
								_T("Arial"));
			CFont* pOld = dc.SelectObject(&hdrfont);
			dc.DrawText(pWnd->GetHeaderText(), CRect(10,5,cr.right,r.top), DT_WORDBREAK);
			dc.SelectObject(&descfont);
			dc.DrawText(pWnd->GetDescText(), CRect(20,22,cr.right,r.top), DT_WORDBREAK);
			dc.SelectObject(pOld);
		}

		CDialog::OnPaint();
	}
}

void CVSRipDlg::OnKickIdle()
{
	UpdateDialogControls(this, false);

	for(CWnd* pChild = GetWindow(GW_CHILD); pChild; pChild = pChild->GetNextWindow())
	{
		if(pChild->IsKindOf(RUNTIME_CLASS(CVSRipPage)))
			pChild->UpdateDialogControls(pChild, false);
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CVSRipDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CVSRipDlg::OnPrev()
{
	ShowPrev();
}

void CVSRipDlg::OnUpdatePrev(CCmdUI* pCmdUI)
{
	if(m_dlgpos) pCmdUI->SetText(m_dlgs.GetAt(m_dlgpos)->GetPrevText());
	pCmdUI->Enable(!!GetPrev());
}

void CVSRipDlg::OnNext()
{
	ShowNext();
}

void CVSRipDlg::OnUpdateNext(CCmdUI* pCmdUI)
{
	if(m_dlgpos) pCmdUI->SetText(m_dlgs.GetAt(m_dlgpos)->GetNextText());
	pCmdUI->Enable(!!GetNext());
}

void CVSRipDlg::OnClose()
{
	if(m_dlgpos) m_dlgs.GetAt(m_dlgpos)->OnClose();

	OnCancel();
}

void CVSRipDlg::OnUpdateClose(CCmdUI* pCmdUI)
{
	if(m_dlgpos) pCmdUI->SetText(m_dlgs.GetAt(m_dlgpos)->GetCloseText());
	pCmdUI->Enable(!m_dlgpos || m_dlgs.GetAt(m_dlgpos)->CanClose());
}
