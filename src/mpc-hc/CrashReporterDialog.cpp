/*
 * (C) 2015 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include "CrashReporterDialog.h"
#include "resource.h"
#include "mplayerc.h"
#include "DSUtil.h"

CCrashReporterDialog::CCrashReporterDialog()
    : CDialog()
{
    // Listen for language changes
    EventRouter::EventSelection receives;
    receives.insert(MpcEvent::CHANGING_UI_LANGUAGE);
    GetEventd().Connect(m_eventc, receives, std::bind(&CCrashReporterDialog::EventCallback, this, std::placeholders::_1));
}

BOOL CCrashReporterDialog::Create()
{
    // Try to preallocate memory for the result strings
    m_email.Preallocate(128);
    m_description.Preallocate(4096);

    BOOL res = __super::Create(IDD, CWnd::GetDesktopWindow());

    // Because we set LR_SHARED, there is no need to explicitly destroy the icon
    VERIFY(SetIcon((HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, 48, 48, LR_SHARED), true) == nullptr);

    return res;
}

BOOL CCrashReporterDialog::OnInitDialog()
{
    __super::OnInitDialog();
    return FALSE; // return FALSE so that the dialog does not steal focus
}

void CCrashReporterDialog::LoadTranslatableResources()
{
    // Load the template in a temporary dialog and get the strings from there.
    // We don't use MFC to avoid side effects caused by creating another dialog.
    HWND hDlg = ::CreateDialog(AfxGetResourceHandle(), MAKEINTRESOURCE(IDD), nullptr, nullptr);
    if (hDlg) {
        auto setTextFromDlg = [&](int nID) {
            CWnd* pItem = CWnd::FromHandle(::GetDlgItem(hDlg, nID));
            if (pItem && IsWindow(pItem->m_hWnd)) {
                CString text;
                pItem->GetWindowText(text);
                if (!text.IsEmpty()) {
                    GetDlgItem(nID)->SetWindowText(text);
                }
            }
        };

        setTextFromDlg(IDC_STATIC1);
        setTextFromDlg(IDC_STATIC2);
        setTextFromDlg(IDC_STATIC3);
        setTextFromDlg(IDC_STATIC4);
        setTextFromDlg(IDOK);

        ::DestroyWindow(hDlg);
    }
}

void CCrashReporterDialog::EventCallback(MpcEvent ev)
{
    switch (ev) {
        case MpcEvent::CHANGING_UI_LANGUAGE:
            LoadTranslatableResources();
            break;
        default:
            ASSERT(FALSE);
    }
}

bool CCrashReporterDialog::WaitForUserInput(CString& email, CString& description)
{
    m_eventDataAvailable.Wait();

    if (m_bHasData) {
        email = m_email;
        description = m_description;
    }

    return m_bHasData;
}

void CCrashReporterDialog::OnQuit()
{
    if (m_bHasData) {
        GetDlgItem(IDC_EDIT1)->GetWindowText(m_email);
        GetDlgItem(IDC_EDIT2)->GetWindowText(m_description);
    }
    m_eventDataAvailable.Set();
    m_eventDataRead.Wait();
    DestroyWindow();
}

BEGIN_MESSAGE_MAP(CCrashReporterDialog, CDialog)
END_MESSAGE_MAP()

IMPLEMENT_DYNCREATE(CCrashReporterUIThread, CWinThread)

CCrashReporterUIThread::CCrashReporterUIThread()
    : CWinThread()
    , m_eventThreadReady(TRUE)
    , m_bThreadReady(false)
{}

BOOL CCrashReporterUIThread::InitInstance()
{
    __super::InitInstance();

    SetThreadName(DWORD(-1), "Crash Reporter UI Thread");

    m_dlg.Create();
    m_pMainWnd = &m_dlg;
    return TRUE;
}

int CCrashReporterUIThread::ExitInstance()
{
    m_dlg.DestroyWindow();
    return __super::ExitInstance();
}

BOOL CCrashReporterUIThread::OnIdle(LONG lCount)
{
    if (!m_bThreadReady) {
        m_bThreadReady = true;
        m_eventThreadReady.Set();
    }
    return __super::OnIdle(lCount);
}

BEGIN_MESSAGE_MAP(CCrashReporterUIThread, CWinThread)
END_MESSAGE_MAP()
