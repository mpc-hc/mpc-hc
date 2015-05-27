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

#pragma once

#include <afxwin.h>
#include "BaseClasses/wxutil.h"
#include "resource.h"

class CCrashReporterDialog : public CDialog
{
private:
    CAMEvent m_eventDataAvailable, m_eventDataRead;
    bool m_bHasData = false;
    CString m_email, m_description;

    EventClient m_eventc;

public:
    CCrashReporterDialog();

    BOOL Create();
    BOOL OnInitDialog() override;
    void LoadTranslatableResources();

    bool WaitForUserInput(CString& email, CString& description);
    void SignalDataRead() { m_eventDataRead.Set(); };

    // Dialog Data
    enum { IDD = IDD_CRASH_REPORTER };

protected:
    DECLARE_MESSAGE_MAP()

    virtual void OnOK() { m_bHasData = true; OnQuit(); };
    virtual void OnCancel() { m_bHasData = false; OnQuit(); };

private:
    void EventCallback(MpcEvent ev);
    void OnQuit();
};

class CCrashReporterUIThread : public CWinThread
{
private:
    CAMEvent m_eventThreadReady;
    bool m_bThreadReady;
    CCrashReporterDialog m_dlg;

    DECLARE_DYNCREATE(CCrashReporterUIThread)

protected:
    CCrashReporterUIThread();

public:
    static CCrashReporterUIThread* GetInstance() {
        static CCrashReporterUIThread* pUIThread = (CCrashReporterUIThread*)AfxBeginThread(RUNTIME_CLASS(CCrashReporterUIThread));
        return pUIThread;
    };

    virtual BOOL InitInstance();
    virtual int ExitInstance();

    virtual BOOL OnIdle(LONG lCount);

    void WaitThreadReady() { m_eventThreadReady.Wait(); };

    CCrashReporterDialog& GetCrashDialog() { return m_dlg; };

    DECLARE_MESSAGE_MAP()
};
