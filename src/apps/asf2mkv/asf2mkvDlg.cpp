/*
 * $Id$
 *
 * (C) 2003-2005 Gabest
 *
 * This file is part of asf2mkv.
 *
 * Asf2mkv is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Asf2mkv is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include "asf2mkv.h"
#include "asf2mkvDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include <initguid.h>
#include "./asf2mkvdlg.h"

//  {6B6D0800-9ADA-11d0-A520-00A0D10129C0}
DEFINE_GUID(CLSID_NetShowSource,
            0x6b6d0800, 0x9ada, 0x11d0, 0xa5, 0x20, 0x0, 0xa0, 0xd1, 0x1, 0x29, 0xc0);

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
    CAboutDlg();

// Dialog Data
    enum { IDD = IDD_ABOUTBOX };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// CUrlDropTarget

BEGIN_MESSAGE_MAP(CUrlDropTarget, COleDropTarget)
END_MESSAGE_MAP()

DROPEFFECT CUrlDropTarget::OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
    return DROPEFFECT_NONE;
}

DROPEFFECT CUrlDropTarget::OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
    UINT CF_URL = RegisterClipboardFormat(_T("UniformResourceLocator"));
    return pDataObject->IsDataAvailable(CF_URL) ? DROPEFFECT_COPY : DROPEFFECT_NONE;
}

BOOL CUrlDropTarget::OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point)
{
    UINT CF_URL = RegisterClipboardFormat(_T("UniformResourceLocator"));

    BOOL bResult = FALSE;

    if(pDataObject->IsDataAvailable(CF_URL))
    {
        FORMATETC fmt = {CF_URL, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
        if(HGLOBAL hGlobal = pDataObject->GetGlobalData(CF_URL, &fmt))
        {
            LPCSTR pText = (LPCSTR)GlobalLock(hGlobal);
            if(AfxIsValidString(pText))
            {
                AfxGetMainWnd()->SendMessage(WM_OPENURL, 0, (LPARAM)pText);
                GlobalUnlock(hGlobal);
                bResult = TRUE;
            }
        }
    }

    return bResult;
}

DROPEFFECT CUrlDropTarget::OnDropEx(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropDefault, DROPEFFECT dropList, CPoint point)
{
    return (DROPEFFECT)-1;
}

void CUrlDropTarget::OnDragLeave(CWnd* pWnd)
{
}

DROPEFFECT CUrlDropTarget::OnDragScroll(CWnd* pWnd, DWORD dwKeyState, CPoint point)
{
    return DROPEFFECT_NONE;
}

// Casf2mkvDlg dialog

#define WM_GRAPHNOTIFY (WM_APP+1)

Casf2mkvDlg::Casf2mkvDlg(CWnd* pParent /*=NULL*/)
    : CResizableDialog(Casf2mkvDlg::IDD, pParent)
    , m_fRecording(false)
    , m_mru(0, _T("MRU"), _T("file%d"), 20, 10)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void Casf2mkvDlg::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO1, m_combo);
    DDX_Control(pDX, IDC_STATIC1, m_video);
}


void Casf2mkvDlg::SetupCombo()
{
    m_combo.ResetContent();
    for(int i = 0; i < m_mru.GetSize(); i++)
        if(!m_mru[i].IsEmpty())
            m_combo.AddString(m_mru[i]);
}

void Casf2mkvDlg::SetVideoRect()
{
    if(pVW)
    {
        CRect r;
        m_video.GetWindowRect(r);
        r -= r.TopLeft();
        pVW->SetWindowPosition(r.left, r.top, r.Width(), r.Height());
    }
}

BEGIN_MESSAGE_MAP(Casf2mkvDlg, CResizableDialog)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_MESSAGE(WM_GRAPHNOTIFY, OnGraphNotify)
    ON_BN_CLICKED(IDC_BUTTON1, OnRecord)
    ON_UPDATE_COMMAND_UI(IDC_BUTTON1, OnUpdateRecord)
    ON_UPDATE_COMMAND_UI(IDC_COMBO1, OnUpdateFileName)
    ON_UPDATE_COMMAND_UI(IDC_BUTTON2, OnUpdateSettings)
    ON_UPDATE_COMMAND_UI(IDC_CHECK2, OnUpdateSettings)
    ON_WM_SIZE()
    ON_WM_TIMER()
    ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedButton2)
    ON_MESSAGE(WM_APP, OnUrlOpen)
END_MESSAGE_MAP()


// Casf2mkvDlg message handlers

BOOL Casf2mkvDlg::OnInitDialog()
{
    __super::OnInitDialog();

    // Add "About..." menu item to system menu.

    // IDM_ABOUTBOX must be in the system command range.
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL)
    {
        CString strAboutMenu;
        strAboutMenu.LoadString(IDS_ABOUTBOX);
        if (!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);			// Set big icon
    SetIcon(m_hIcon, FALSE);		// Set small icon

    // TODO: Add extra initialization here

    AddAnchor(IDC_STATIC1, TOP_LEFT, BOTTOM_RIGHT);
    AddAnchor(IDC_COMBO1, BOTTOM_LEFT, BOTTOM_RIGHT);
    AddAnchor(IDC_BUTTON1, BOTTOM_RIGHT);
    AddAnchor(IDC_BUTTON2, BOTTOM_RIGHT);

    m_mru.ReadList();
    SetupCombo();

    m_video.ModifyStyle(0, WS_CLIPCHILDREN);

    SetWindowText(ResStr(IDS_TITLE));

    m_urlDropTarget.Register(this);

    return TRUE;  // return TRUE  unless you set the focus to a control
}

BOOL Casf2mkvDlg::DestroyWindow()
{
    m_urlDropTarget.Revoke();

    return __super::DestroyWindow();
}

void Casf2mkvDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else
    {
        __super::OnSysCommand(nID, lParam);
    }
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void Casf2mkvDlg::OnPaint()
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
        __super::OnPaint();
    }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR Casf2mkvDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}


LRESULT Casf2mkvDlg::OnGraphNotify(WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_OK;

    LONG evCode, evParam1, evParam2;
    while(pME && SUCCEEDED(pME->GetEvent(&evCode, (LONG_PTR*)&evParam1, (LONG_PTR*)&evParam2, 0)))
    {
        hr = pME->FreeEventParams(evCode, evParam1, evParam2);

        if(EC_COMPLETE == evCode)
        {
            if(m_fRecording)
            {
                OnRecord();
                break;
            }
        }
    }

    return hr;
}

void Casf2mkvDlg::OnRecord()
{
    UpdateData();

    HRESULT hr;

    if(!m_fRecording)
    {
        m_fRecording = true;

        UpdateDialogControls(this, FALSE);

        hr = E_FAIL;

        do
        {
            // i/o

            CString src;
            m_combo.GetWindowText(src);

            CFileDialog fd(
                FALSE, _T("mkv"), m_dst,
                OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT|OFN_EXPLORER|OFN_ENABLESIZING,
                _T("Matroska file (*.mkv;*.mka)|*.mkv;*.mka||"),
                this);
            if(fd.DoModal() != IDOK) break;
            m_dst = fd.GetPathName();

            m_mru.Add(src);
            m_mru.WriteList();
            SetupCombo();
            m_combo.SetWindowText(src);

            if(src.Trim().IsEmpty() || m_dst.Trim().IsEmpty())
                break;

            // filer graph

            if(FAILED(hr = pGB.CoCreateInstance(CLSID_FilterGraph)))
                break;

            pMC = pGB;
            pME = pGB;
            pMS = pGB;
            pVW = pGB;
            pBV = pGB;
            if(!pMC || !pME || !pMS || !pVW || !pBV)
                break;

            if(FAILED(hr = pME->SetNotifyWindow((OAHWND)m_hWnd, WM_GRAPHNOTIFY, 0)))
                break;

            // windows media source filter

            CComPtr<IBaseFilter> pSrc;
            if(FAILED(hr = pSrc.CoCreateInstance(CLSID_NetShowSource))
               || FAILED(hr = pGB->AddFilter(pSrc, CStringW(src)))
               || FAILED(hr = CComQIPtr<IFileSourceFilter>(pSrc)->Load(CStringW(src), NULL)))
                break;

            // matroska muxer

            CComPtr<IBaseFilter> pMux;
            if(FAILED(hr = pMux.CoCreateInstance(__uuidof(CMatroskaMuxerFilter)))
               && !(pMux = new CMatroskaMuxerFilter(NULL, NULL))
               || FAILED(hr = pGB->AddFilter(pMux, L"Matroska Muxer")))
                break;

            BeginEnumPins(pSrc, pEP, pPin)
            if(FAILED(hr = pGB->Connect(pPin, GetFirstDisconnectedPin(pMux, PINDIR_INPUT))))
                break;
            EndEnumPins

            if(FAILED(hr))
                break;

            // file writer

            CComPtr<IBaseFilter> pFW;
            if(FAILED(hr = pFW.CoCreateInstance(CLSID_FileWriter))
               || FAILED(hr = pGB->AddFilter(pFW, CStringW(m_dst)))
               || FAILED(hr = CComQIPtr<IFileSinkFilter2>(pFW)->SetFileName(CStringW(m_dst), NULL))
               || FAILED(hr = CComQIPtr<IFileSinkFilter2>(pFW)->SetMode(AM_FILE_OVERWRITE))
               || FAILED(hr = pGB->Connect(GetFirstDisconnectedPin(pMux, PINDIR_OUTPUT), GetFirstDisconnectedPin(pFW, PINDIR_INPUT))))
                break;

            // insert inf. pin tee

            BeginEnumPins(pMux, pEP, pPin)
            {
                PIN_DIRECTION dir;
                CComPtr<IPin> pPinTo;
                CMediaType mt;
                if(FAILED(pPin->QueryDirection(&dir)) || dir != PINDIR_INPUT
                   || FAILED(pPin->ConnectedTo(&pPinTo))
                   || FAILED(pPin->ConnectionMediaType(&mt)))
                    continue;

                // FIXME: the inf pin tee filter makes the video messed up, like when seeking
                // onto a non-keyframe and starting to decode from that point. (audio seems to be ok)

                CComPtr<IBaseFilter> pInfPinTee;
                if(mt.majortype == MEDIATYPE_Video
                   || mt.majortype == MEDIATYPE_Audio)
                {
                    if(FAILED(pGB->Disconnect(pPin))
                       || FAILED(pGB->Disconnect(pPinTo))
                       || FAILED(pInfPinTee.CoCreateInstance(CLSID_InfTee))
                       || FAILED(pGB->AddFilter(pInfPinTee, L"Infinite Pin Tee")))
                        continue;

                    if(FAILED(pGB->Connect(pPinTo, GetFirstDisconnectedPin(pInfPinTee, PINDIR_INPUT)))
                       || FAILED(pGB->Connect(GetFirstDisconnectedPin(pInfPinTee, PINDIR_OUTPUT), pPin)))
                    {
                        pGB->RemoveFilter(pInfPinTee);
                        pGB->ConnectDirect(pPinTo, pPin, NULL);
                        continue;
                    }
                }

                pPin = GetFirstDisconnectedPin(pInfPinTee, PINDIR_OUTPUT);

                CComPtr<IBaseFilter> pRenderer;

                if(mt.majortype == MEDIATYPE_Video)
                {
                    if(SUCCEEDED(pRenderer.CoCreateInstance(CLSID_VideoRendererDefault))
                       || SUCCEEDED(pRenderer.CoCreateInstance(CLSID_VideoRenderer)))
                        pPinTo = ::GetFirstDisconnectedPin(pRenderer, PINDIR_INPUT);
                }
                else if(mt.majortype == MEDIATYPE_Audio)
                {
                    if(SUCCEEDED(pRenderer.CoCreateInstance(CLSID_DSoundRender)))
                        pPinTo = ::GetFirstDisconnectedPin(pRenderer, PINDIR_INPUT);
                }

                if(pPin && pPinTo && pRenderer)
                {
                    if(SUCCEEDED(pGB->AddFilter(pRenderer, L"Renderer"))
                       && FAILED(pGB->Connect(pPin, pPinTo)))
                        pGB->RemoveFilter(pRenderer);
                }
            }
            EndEnumPins

            // setup video window

            if(SUCCEEDED(pVW->put_Owner((OAHWND)m_video.m_hWnd))
               && SUCCEEDED(pVW->put_WindowStyle(WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN))
               && SUCCEEDED(pVW->put_MessageDrain((OAHWND)m_hWnd)))
            {
                SetVideoRect();
            }

            // timer for polling the position

            SetTimer(1, 500, NULL);

            // go!

            if(FAILED(hr = pMC->Run()))
                break;

            hr = S_OK;
        }
        while(false);

        if(FAILED(hr)) OnRecord();
    }
    else
    {
        if(pMC) pMC->Stop();

        pMC.Release();
        pME.Release();
        pMS.Release();
        pVW.Release();
        pBV.Release();
        pGB.Release();

        m_fRecording = false;

        SetWindowText(ResStr(IDS_TITLE));
    }
}

void Casf2mkvDlg::OnUpdateRecord(CCmdUI* pCmdUI)
{
    CString url;
    m_combo.GetWindowText(url);
    url.Trim();
    pCmdUI->Enable(!url.IsEmpty());
    pCmdUI->SetText(ResStr(!m_fRecording ? IDS_RECORD : IDS_STOP));
}

void Casf2mkvDlg::OnUpdateFileName(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(!m_fRecording);
}

void Casf2mkvDlg::OnUpdateSettings(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(!m_fRecording);
}

void Casf2mkvDlg::OnSize(UINT nType, int cx, int cy)
{
    CResizableDialog::OnSize(nType, cx, cy);

    SetVideoRect();
}

void Casf2mkvDlg::OnTimer(UINT nIDEvent)
{
    if(nIDEvent == 1)
    {
        if(pMS)
        {
            REFERENCE_TIME rtPos = 0, rtDur = 0;
            pMS->GetCurrentPosition(&rtPos);
            pMS->GetDuration(&rtDur);

            CString title;
            if(rtDur > 0) title.Format(_T("%s (progress: %I64d%%)"), ResStr(IDS_TITLE), 100i64 * rtPos / rtDur);
            else title = ResStr(IDS_TITLE) + _T(" (live)");

            BeginEnumFilters(pGB, pEF, pBF)
            {
                if(CComQIPtr<IAMNetworkStatus, &IID_IAMNetworkStatus> pAMNS = pBF)
                {
                    long BufferingProgress = 0;
                    if(SUCCEEDED(pAMNS->get_BufferingProgress(&BufferingProgress)) && BufferingProgress > 0)
                    {
                        CString str;
                        str.Format(_T(" (buffer: %d%%)"), BufferingProgress);
                        title += str;
                    }
                    break;
                }
            }
            EndEnumFilters

            SetWindowText(title);
        }
    }

    CResizableDialog::OnTimer(nIDEvent);
}

void Casf2mkvDlg::OnBnClickedButton2()
{
    CComPtr<IBaseFilter> pBF;
    pBF.CoCreateInstance(CLSID_NetShowSource);
    if(pBF) ShowPPage(pBF, m_hWnd);
}

LRESULT Casf2mkvDlg::OnUrlOpen(WPARAM wParam, LPARAM lParam)
{
    m_combo.SetWindowText(CString((char*)lParam));
    return 0;
}

//////////////////

Casf2mkvDlg::CRecentFileAndURLList::CRecentFileAndURLList(UINT nStart, LPCTSTR lpszSection,
        LPCTSTR lpszEntryFormat, int nSize,
        int nMaxDispLen)
    : CRecentFileList(nStart, lpszSection, lpszEntryFormat, nSize, nMaxDispLen)
{
}

//#include <afximpl.h>
extern BOOL AFXAPI AfxFullPath(LPTSTR lpszPathOut, LPCTSTR lpszFileIn);
extern BOOL AFXAPI AfxComparePath(LPCTSTR lpszPath1, LPCTSTR lpszPath2);

void Casf2mkvDlg::CRecentFileAndURLList::Add(LPCTSTR lpszPathName)
{
    ASSERT(m_arrNames != NULL);
    ASSERT(lpszPathName != NULL);
    ASSERT(AfxIsValidString(lpszPathName));

    if(CString(lpszPathName).MakeLower().Find(_T("@device:")) >= 0)
        return;

    bool fURL = (CString(lpszPathName).Find(_T("://")) >= 0);

    // fully qualify the path name
    TCHAR szTemp[_MAX_PATH];
    if(fURL) _tcscpy(szTemp, lpszPathName);
    else AfxFullPath(szTemp, lpszPathName);

    // update the MRU list, if an existing MRU string matches file name
    int iMRU;
    for (iMRU = 0; iMRU < m_nSize-1; iMRU++)
    {
        if((fURL && !_tcscmp(m_arrNames[iMRU], szTemp))
           || AfxComparePath(m_arrNames[iMRU], szTemp))
            break;      // iMRU will point to matching entry
    }
    // move MRU strings before this one down
    for (; iMRU > 0; iMRU--)
    {
        ASSERT(iMRU > 0);
        ASSERT(iMRU < m_nSize);
        m_arrNames[iMRU] = m_arrNames[iMRU-1];
    }
    // place this one at the beginning
    m_arrNames[0] = szTemp;
}
