/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2010 see AUTHORS
 *
 * This file is part of mplayerc.
 *
 * Mplayerc is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mplayerc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include "mplayerc.h"
#include "PixelShaderCompiler.h"
#include "ShaderEditorDlg.h"
#include "MainFrm.h"

#undef SubclassWindow


// CShaderLabelComboBox

BEGIN_MESSAGE_MAP(CShaderLabelComboBox, CComboBox)
	ON_WM_CTLCOLOR()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

HBRUSH CShaderLabelComboBox::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if(nCtlColor == CTLCOLOR_EDIT) {
		if(m_edit.GetSafeHwnd() == NULL) {
			m_edit.SubclassWindow(pWnd->GetSafeHwnd());
		}
	}

	return __super::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CShaderLabelComboBox::OnDestroy()
{
	if(m_edit.GetSafeHwnd() != NULL) {
		m_edit.UnsubclassWindow();
	}

	__super::OnDestroy();
}

// CShaderEdit

CShaderEdit::CShaderEdit()
{
	m_acdlg.Create(CShaderAutoCompleteDlg::IDD, NULL);

	m_nEndChar = -1;
	m_nIDEvent = (UINT_PTR)-1;
}

CShaderEdit::~CShaderEdit()
{
	m_acdlg.DestroyWindow();
}

BOOL CShaderEdit::PreTranslateMessage(MSG* pMsg)
{
	if(m_acdlg.IsWindowVisible()
			&& pMsg->message == WM_KEYDOWN
			&& (pMsg->wParam == VK_UP || pMsg->wParam == VK_DOWN
				|| pMsg->wParam == VK_PRIOR || pMsg->wParam == VK_NEXT
				|| pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)) {
		int i = m_acdlg.m_list.GetCurSel();

		if(pMsg->wParam == VK_RETURN && i >= 0) {
			CString str;
			m_acdlg.m_list.GetText(i, str);
			i = str.Find('(')+1;
			if(i > 0) {
				str = str.Left(i);
			}

			int nStartChar = 0, nEndChar = -1;
			GetSel(nStartChar, nEndChar);

			CString text;
			GetWindowText(text);
			while(nStartChar > 0 && _istalnum(text.GetAt(nStartChar-1))) {
				nStartChar--;
			}

			SetSel(nStartChar, nEndChar);
			ReplaceSel(str, TRUE);
		} else if(pMsg->wParam == VK_ESCAPE) {
			m_acdlg.ShowWindow(SW_HIDE);
			return GetParent()->PreTranslateMessage(pMsg);
		} else {
			m_acdlg.m_list.SendMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
		}

		return TRUE;
	}

	return __super::PreTranslateMessage(pMsg);
}

BEGIN_MESSAGE_MAP(CShaderEdit, CLineNumberEdit)
	ON_CONTROL_REFLECT(EN_UPDATE, OnUpdate)
	ON_WM_KILLFOCUS()
	ON_WM_TIMER()
END_MESSAGE_MAP()

void CShaderEdit::OnUpdate()
{
	if(m_nIDEvent == (UINT_PTR)-1) {
		m_nIDEvent = SetTimer(1, 100, NULL);
	}

	CString text;
	int nStartChar = 0, nEndChar = -1;
	GetSel(nStartChar, nEndChar);

	if(nStartChar == nEndChar) {
		GetWindowText(text);
		while(nStartChar > 0 && _istalnum(text.GetAt(nStartChar-1))) {
			nStartChar--;
		}
	}

	if(nStartChar < nEndChar) {
		text = text.Mid(nStartChar, nEndChar - nStartChar);
		text.TrimRight('(');
		text.MakeLower();

		m_acdlg.m_list.ResetContent();

		CString key, value;
		POSITION pos = m_acdlg.m_inst.GetStartPosition();
		while(pos) {
			POSITION cur = pos;
			m_acdlg.m_inst.GetNextAssoc(pos, key, value);

			if(key.Find(text) == 0) {
				CAtlList<CString> sl;
				Explode(value, sl, '|', 2);
				if(sl.GetCount() != 2) {
					continue;
				}
				CString name = sl.RemoveHead();
				CString description = sl.RemoveHead();
				int i = m_acdlg.m_list.AddString(name);
				m_acdlg.m_list.SetItemDataPtr(i, cur);
			}
		}

		if(m_acdlg.m_list.GetCount() > 0) {
			int lineheight = GetLineHeight();

			CPoint p = PosFromChar(nStartChar);
			p.y += lineheight;
			ClientToScreen(&p);
			CRect r(p, CSize(100, 100));

			m_acdlg.MoveWindow(r);
			m_acdlg.SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
			m_acdlg.ShowWindow(SW_SHOWNOACTIVATE);

			m_nEndChar = nEndChar;

			return;
		}
	}

	m_acdlg.ShowWindow(SW_HIDE);
}

void CShaderEdit::OnKillFocus(CWnd* pNewWnd)
{
	CString text;
	GetWindowText(text);
	__super::OnKillFocus(pNewWnd);
	GetWindowText(text);

	m_acdlg.ShowWindow(SW_HIDE);
}

void CShaderEdit::OnTimer(UINT_PTR nIDEvent)
{
	if(m_nIDEvent == nIDEvent) {
		int nStartChar = 0, nEndChar = -1;
		GetSel(nStartChar, nEndChar);
		if(nStartChar != nEndChar || m_nEndChar != nEndChar) {
			m_acdlg.ShowWindow(SW_HIDE);
		}
	}

	__super::OnTimer(nIDEvent);
}

// CShaderEditorDlg dialog

CShaderEditorDlg::CShaderEditorDlg()
	: CResizableDialog(CShaderEditorDlg::IDD, NULL)
	, m_fSplitterGrabbed(false)
	, m_pPSC(NULL)
	, m_pShader(NULL)
{
}

CShaderEditorDlg::~CShaderEditorDlg()
{
	delete m_pPSC;
}

BOOL CShaderEditorDlg::Create(CWnd* pParent)
{
	if(!__super::Create(IDD, pParent)) {
		return FALSE;
	}

	AddAnchor(IDC_COMBO1, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_COMBO2, TOP_RIGHT);
	AddAnchor(IDC_EDIT1, TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_EDIT2, BOTTOM_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_BUTTON1, TOP_RIGHT);

	m_srcdata.SetTabStops(16);

	SetMinTrackSize(CSize(250, 40));

	m_targets.AddString(_T("ps_1_1"));
	m_targets.AddString(_T("ps_1_2"));
	m_targets.AddString(_T("ps_1_3"));
	m_targets.AddString(_T("ps_1_4"));
	m_targets.AddString(_T("ps_2_0"));
	m_targets.AddString(_T("ps_2_a"));
	m_targets.AddString(_T("ps_2_sw"));
	m_targets.AddString(_T("ps_3_0"));
	m_targets.AddString(_T("ps_3_sw"));

	POSITION pos = AfxGetAppSettings().m_shaders.GetHeadPosition();
	while(pos) {
		const AppSettings::Shader& s = AfxGetAppSettings().m_shaders.GetNext(pos);
		m_labels.SetItemDataPtr(m_labels.AddString(s.label), (void*)&s);
	}

	m_nIDEventShader = SetTimer(1, 1000, NULL);

	return TRUE;
}

void CShaderEditorDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_labels);
	DDX_Control(pDX, IDC_COMBO2, m_targets);
	DDX_Control(pDX, IDC_EDIT1, m_srcdata);
	DDX_Control(pDX, IDC_EDIT2, m_output);
}

bool CShaderEditorDlg::HitTestSplitter(CPoint p)
{
	CRect r, rs, ro;
	m_srcdata.GetWindowRect(&rs);
	m_output.GetWindowRect(&ro);
	ScreenToClient(&rs);
	ScreenToClient(&ro);
	GetClientRect(&r);
	r.left = ro.left;
	r.right = ro.right;
	r.top = rs.bottom;
	r.bottom = ro.top;
	return !!r.PtInRect(p);
}

BEGIN_MESSAGE_MAP(CShaderEditorDlg, CResizableDialog)
	ON_CBN_SELCHANGE(IDC_COMBO1, OnCbnSelchangeCombo1)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton2)
	ON_WM_TIMER()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()

// CShaderEditorDlg message handlers

BOOL CShaderEditorDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN
			&& pMsg->hwnd == m_labels.m_edit.GetSafeHwnd()) {
		OnCbnSelchangeCombo1();

		return TRUE;
	} else if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB
			  && pMsg->hwnd == m_srcdata.GetSafeHwnd()) {
		int nStartChar, nEndChar;
		m_srcdata.GetSel(nStartChar, nEndChar);
		if(nStartChar == nEndChar) {
			m_srcdata.ReplaceSel(_T("\t"));
		}
		return TRUE;
	} else if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) {
		return GetParent()->PreTranslateMessage(pMsg);
	}

	return __super::PreTranslateMessage(pMsg);
}

void CShaderEditorDlg::OnCbnSelchangeCombo1()
{
	int i = m_labels.GetCurSel();

	if(i < 0) {
		CString label;
		m_labels.GetWindowText(label);
		label.Trim();

		if(label.IsEmpty()) {
			return;
		}

		CStringA srcdata;
		LoadResource(IDF_SHADER_EMPTY, srcdata, _T("FILE"));

		AppSettings::Shader s;
		s.label = label;
		s.target = _T("ps_2_0");
		s.srcdata = CString(srcdata);

		POSITION pos = AfxGetAppSettings().m_shaders.AddTail(s);

		i = m_labels.AddString(s.label);
		m_labels.SetCurSel(i);
		m_labels.SetItemDataPtr(i, (void*)&AfxGetAppSettings().m_shaders.GetAt(pos));
	}

	m_pShader = (AppSettings::Shader*)m_labels.GetItemDataPtr(i);

	m_targets.SetWindowText(m_pShader->target);

	CString srcdata = m_pShader->srcdata;
	srcdata.Replace(_T("\n"), _T("\r\n"));
	m_srcdata.SetWindowText(srcdata);

	((CMainFrame*)AfxGetMainWnd())->UpdateShaders(m_pShader->label);
}

void CShaderEditorDlg::OnBnClickedButton2()
{
	if(!m_pShader) {
		return;
	}

	if(IDYES != AfxMessageBox(ResStr(IDS_SHADEREDITORDLG_0), MB_YESNO)) {
		return;
	}

	AppSettings& s = AfxGetAppSettings();

	for(POSITION pos = s.m_shaders.GetHeadPosition(); pos; s.m_shaders.GetNext(pos)) {
		if(m_pShader == &s.m_shaders.GetAt(pos)) {
			m_pShader = NULL;
			s.m_shaders.RemoveAt(pos);
			int i = m_labels.GetCurSel();
			if(i >= 0) {
				m_labels.DeleteString(i);
			}
			m_labels.SetWindowText(_T(""));
			m_targets.SetWindowText(_T(""));
			m_srcdata.SetWindowText(_T(""));
			m_output.SetWindowText(_T(""));
			((CMainFrame*)AfxGetMainWnd())->UpdateShaders(_T(""));
			break;
		}
	}
}

void CShaderEditorDlg::OnTimer(UINT_PTR nIDEvent)
{
	if(nIDEvent == m_nIDEventShader && IsWindowVisible() && m_pShader) {
		CString srcdata;
		m_srcdata.GetWindowText(srcdata);
		srcdata.Replace(_T("\r"), _T(""));
		srcdata.Trim();

		CString target;
		m_targets.GetWindowText(target);
		target.Trim();

		if(!srcdata.IsEmpty() && !target.IsEmpty() && (m_pShader->srcdata != srcdata || m_pShader->target != target)) {
			KillTimer(m_nIDEventShader);

			m_pShader->srcdata = srcdata;
			m_pShader->target = target;

			if(!m_pPSC) {
				m_pPSC = DNew CPixelShaderCompiler(NULL);
			}

			CString disasm, errmsg;
			HRESULT hr = m_pPSC->CompileShader(CStringA(srcdata), "main", CStringA(target), D3DXSHADER_DEBUG, NULL, &disasm, &errmsg);

			if(SUCCEEDED(hr)) {
				errmsg = _T("D3DXCompileShader succeeded\n");
				errmsg += _T("\n");
				errmsg += disasm;

				((CMainFrame*)AfxGetMainWnd())->UpdateShaders(m_pShader->label);
			}

			errmsg.Replace(_T("\n"), _T("\r\n"));

			m_output.SetWindowText(errmsg);

			// TODO: autosave

			m_nIDEventShader = SetTimer(1, 1000, NULL);
		}
	}

	__super::OnTimer(nIDEvent);
}

void CShaderEditorDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	if(HitTestSplitter(point)) {
		m_fSplitterGrabbed = true;
		SetCapture();
	}

	__super::OnLButtonDown(nFlags, point);
}

void CShaderEditorDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	if(m_fSplitterGrabbed) {
		ReleaseCapture();
		m_fSplitterGrabbed = false;
	}

	__super::OnLButtonUp(nFlags, point);
}

void CShaderEditorDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if(m_fSplitterGrabbed) {
		CRect r, rs, ro;
		GetClientRect(&r);
		m_srcdata.GetWindowRect(&rs);
		m_output.GetWindowRect(&ro);
		ScreenToClient(&rs);
		ScreenToClient(&ro);

		int dist = ro.top - rs.bottom;
		int avgdist = dist / 2;

		rs.bottom = min(max(point.y, rs.top + 40), ro.bottom - 40) - avgdist;
		ro.top = rs.bottom + dist;
		m_srcdata.MoveWindow(&rs);
		m_output.MoveWindow(&ro);

		int div = 100 * ((rs.bottom + ro.top) / 2) / (ro.bottom - rs.top);

		RemoveAnchor(IDC_EDIT1);
		RemoveAnchor(IDC_EDIT2);
		AddAnchor(IDC_EDIT1, TOP_LEFT, CSize(100, div)/*BOTTOM_RIGHT*/);
		AddAnchor(IDC_EDIT2, CSize(0, div)/*BOTTOM_LEFT*/, BOTTOM_RIGHT);
	}

	__super::OnMouseMove(nFlags, point);
}

BOOL CShaderEditorDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CPoint p;
	GetCursorPos(&p);
	ScreenToClient(&p);
	if(HitTestSplitter(p)) {
		::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZENS));
		return TRUE;
	}

	return __super::OnSetCursor(pWnd, nHitTest, message);
}
