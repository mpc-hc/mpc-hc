// RegisterCopyDataDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MPCTestAPI.h"
#include "MPCTestAPIDlg.h"
#include <psapi.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

LPCTSTR GetMPCCommandName(MPCAPI_COMMAND nCmd)
{
	switch (nCmd)
	{
	case CMD_CONNECT :
		return _T("CMD_CONNECT");
	case CMD_STATE :
		return _T("CMD_STATE");
	case CMD_PLAYMODE :
		return _T("CMD_PLAYMODE");
	case CMD_NOWPLAYING :
		return _T("CMD_NOWPLAYING");
	case CMD_LISTSUBTITLETRACKS :
		return _T("CMD_LISTSUBTITLETRACKS");
	case CMD_LISTAUDIOTRACKS :
		return _T("CMD_LISTAUDIOTRACKS");
	case CMD_PLAYLIST :
		return _T("CMD_PLAYLIST");
	default :
		return _T("CMD_UNK");
	}
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRegisterCopyDataDlg dialog

CRegisterCopyDataDlg::CRegisterCopyDataDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRegisterCopyDataDlg::IDD, pParent)
	, m_strMPCPath(_T(""))
	, m_txtCommand(_T(""))
	, m_nCommandType(0)
	, m_hWndMPC(NULL)
{
	//{{AFX_DATA_INIT(CRegisterCopyDataDlg)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRegisterCopyDataDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRegisterCopyDataDlg)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	DDX_Text(pDX, IDC_EDIT1, m_strMPCPath);
	DDX_Control(pDX, IDC_LOGLIST, m_lbLog);
	DDX_Text(pDX, IDC_EDIT2, m_txtCommand);
	DDX_CBIndex(pDX, IDC_COMBO1, m_nCommandType);
}

BEGIN_MESSAGE_MAP(CRegisterCopyDataDlg, CDialog)
	//{{AFX_MSG_MAP(CRegisterCopyDataDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_FINDWINDOW, OnButtonFindwindow)
	ON_WM_COPYDATA()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_SENDCOMMAND, &CRegisterCopyDataDlg::OnBnClickedButtonSendcommand)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRegisterCopyDataDlg message handlers

BOOL CRegisterCopyDataDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

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
	m_strMPCPath = _T("..\\mplayerc\\Release Unicode\\mplayerc.exe");
	UpdateData(FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CRegisterCopyDataDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CRegisterCopyDataDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

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
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CRegisterCopyDataDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}



void CRegisterCopyDataDlg::OnButtonFindwindow()
{
	CString					strExec;
	STARTUPINFO				StartupInfo;
	PROCESS_INFORMATION		ProcessInfo;

	strExec.Format (_T("%s  /slave %d"), m_strMPCPath, GetSafeHwnd());
	UpdateData(TRUE);

	memset (&StartupInfo,		0, sizeof(StartupInfo));
	StartupInfo.cb			= sizeof(StartupInfo);
	GetStartupInfo(&StartupInfo);
	CreateProcess (NULL, (LPTSTR)(LPCTSTR)strExec, NULL, NULL, FALSE, 0, NULL, NULL, &StartupInfo, &ProcessInfo);
}


struct MyStruct
{
	int nNum;
	TCHAR szData[256];

};

void CRegisterCopyDataDlg::Senddata(MPCAPI_COMMAND nCmd, LPCTSTR strCommand)
{
	if (m_hWndMPC)
	{
		COPYDATASTRUCT		MyCDS;

		MyCDS.dwData	= nCmd;
		MyCDS.cbData	= (_tcslen (strCommand) + 1) * sizeof(TCHAR);
		MyCDS.lpData	= (LPVOID) strCommand;

		::SendMessage (m_hWndMPC, WM_COPYDATA, (WPARAM)GetSafeHwnd(), (LPARAM)&MyCDS);
	}
}

BOOL CRegisterCopyDataDlg::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct)
{
	CString		strMsg;
	MyStruct *tcsBuff=(MyStruct*)(pCopyDataStruct->lpData);

	if (pCopyDataStruct->dwData == CMD_CONNECT)
		m_hWndMPC = (HWND)_wtol((LPCTSTR)pCopyDataStruct->lpData);

	strMsg.Format (_T("%s : %s"), GetMPCCommandName ((MPCAPI_COMMAND)pCopyDataStruct->dwData), (LPCTSTR)pCopyDataStruct->lpData);
	m_lbLog.InsertString (0, strMsg);
	return CDialog::OnCopyData(pWnd, pCopyDataStruct);
}



void CRegisterCopyDataDlg::OnBnClickedButtonSendcommand()
{
	CString		strEmpty (_T(""));
	UpdateData(TRUE);

	switch (m_nCommandType)
	{
	case 0 :
		Senddata (CMD_OPENFILE, m_txtCommand);
		break;
	case 1 :
		Senddata (CMD_STOP, strEmpty);
		break;
	case 2 :
		Senddata (CMD_CLOSEFILE, strEmpty);
		break;
	case 3 :
		Senddata (CMD_PLAYPAUSE, strEmpty);
		break;
	case 4 :
		Senddata (CMD_ADDTOPLAYLIST, m_txtCommand);
		break;
	case 5 :
		Senddata (CMD_STARTPLAYLIST, strEmpty);
		break;
	case 6 :
		Senddata (CMD_CLEARPLAYLIST, strEmpty);
		break;
	case 7 :
		Senddata (CMD_SETPOSITION, m_txtCommand);
		break;
	case 8 :
		Senddata (CMD_SETAUDIODELAY, m_txtCommand);
		break;
	case 9 :
		Senddata (CMD_SETSUBTITLEDELAY, m_txtCommand);
		break;
	case 10 :
		Senddata (CMD_GETAUDIOTRACKS, strEmpty);
		break;
	case 11 :
		Senddata (CMD_GETSUBTITLETRACKS, strEmpty);
		break;
	case 12 :
		Senddata (CMD_GETPLAYLIST, strEmpty);
		break;
	case 13 :
		Senddata (CMD_SETINDEXPLAYLIST, m_txtCommand);
		break;
	case 14 :
		Senddata (CMD_SETAUDIOTRACK, m_txtCommand);
		break;
	case 15 :
		Senddata (CMD_SETSUBTITLETRACK, m_txtCommand);
		break;
	case 16 :
		Senddata (CMD_TOGGLEFULLSCREEN, m_txtCommand);
		break;
	case 17 :
		Senddata (CMD_JUMPFORWARDMED, m_txtCommand);
		break;
	case 18 :
		Senddata (CMD_JUMPBACKWARDMED, m_txtCommand);
		break;
	case 19 :
		Senddata (CMD_INCREASEVOLUME, m_txtCommand);
		break;
	case 20 :
		Senddata (CMD_DECREASEVOLUME, m_txtCommand);
		break;
	case 21 :
		Senddata (CMD_SHADER_TOGGLE, m_txtCommand);
		break;
	case 22 :
		Senddata (CMD_CLOSEAPP, m_txtCommand);
		break;
	}
}
