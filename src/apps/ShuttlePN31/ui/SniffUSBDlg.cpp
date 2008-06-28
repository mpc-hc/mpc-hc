// SniffUSBDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "SniffUSB.h"
#include "SniffUSBDlg.h"
#include "devicemgr.h"
#include "multisz.h"

#include <setupapi.h>
#include <winsvc.h>
#include ".\sniffusbdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define PN31SNOOP_TIMER 0x101

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
// CSniffUSBDlg dialog

enum { X_LEFT, X_CENTER, X_RIGHT };
enum { Y_TOP,  Y_CENTER, Y_BOTTOM };

typedef struct {
	UINT nID;
	int  topAlign;
	int  leftAlign;
	int  bottomAlign;
	int  rightAlign;
	RECT saveRect;
} controlAlign_t;

controlAlign_t controlTab[] = {
	{ IDC_USBDEVS, Y_TOP,    X_LEFT,   Y_BOTTOM, X_RIGHT,  },
	{ IDC_TEST,    Y_BOTTOM, X_CENTER, Y_BOTTOM, X_CENTER, },
	{ IDC_FILTERINSTALL, Y_BOTTOM, X_CENTER, Y_BOTTOM, X_CENTER, },
	{ IDC_REPLUG, Y_BOTTOM, X_CENTER, Y_BOTTOM, X_CENTER, },
	{ IDC_REFRESH, Y_BOTTOM, X_CENTER, Y_BOTTOM, X_CENTER, },
	{ IDC_INSTALL, Y_BOTTOM, X_CENTER, Y_BOTTOM, X_CENTER, },
	{ IDC_UNINSTALL, Y_BOTTOM, X_CENTER, Y_BOTTOM, X_CENTER, },
	{ IDCANCEL, Y_BOTTOM, X_CENTER, Y_BOTTOM, X_CENTER, },
	{ IDC_STATIC, Y_BOTTOM, X_CENTER, Y_BOTTOM, X_CENTER, },
	{ IDC_STATIC2, Y_BOTTOM, X_CENTER, Y_BOTTOM, X_CENTER, },
	{ IDC_LOG_FILENAME, Y_BOTTOM, X_CENTER, Y_BOTTOM, X_CENTER, },
	{ IDC_LOG_SIZE, Y_BOTTOM, X_CENTER, Y_BOTTOM, X_CENTER, },
	{ IDC_VIEW, Y_BOTTOM, X_CENTER, Y_BOTTOM, X_CENTER, },
	{ IDC_DELETE, Y_BOTTOM, X_CENTER, Y_BOTTOM, X_CENTER, },
};

const int N_CONTROL = sizeof(controlTab) / sizeof(controlTab[0]);

CSniffUSBDlg::CSniffUSBDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSniffUSBDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSniffUSBDlg)
	m_LogFileName = _T("");
	m_LogSize = 0;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_sLowerFilters.LoadString(IDS_LOWERFILTERS);

	// check Windows 2000 / XP versus Windows 98
	m_bIsWin2K = ((GetVersion() & 0x80000000) == 0);

	// compute the name of the filter
	if (m_bIsWin2K)
		m_sFilterName.LoadString(IDS_FILTERNAME_NT);
	else
		m_sFilterName.LoadString(IDS_FILTERNAME_98);
	m_hSniffer = INVALID_HANDLE_VALUE;
}

void CSniffUSBDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSniffUSBDlg)
	DDX_Control(pDX, IDC_USBDEVS, m_cDevs);
	DDX_Text(pDX, IDC_LOG_FILENAME, m_LogFileName);
	DDX_Text(pDX, IDC_LOG_SIZE, m_LogSize);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSniffUSBDlg, CDialog)
	//{{AFX_MSG_MAP(CSniffUSBDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_REFRESH, OnRefresh)
	ON_BN_CLICKED(IDC_INSTALL, OnInstall)
	ON_BN_CLICKED(IDC_UNINSTALL, OnUninstall)
	ON_NOTIFY(NM_RCLICK, IDC_USBDEVS, OnRclickUsbdevs)
	ON_COMMAND(ID_SNOOPUSB_INSTALL, OnSnoopusbInstall)
	ON_COMMAND(ID_SNOOPUSB_UNINSTALL, OnSnoopusbUninstall)
	ON_BN_CLICKED(IDC_REPLUG, OnReplug)
	ON_COMMAND(ID_SNOOPUSB_REPLUG, OnSnoopusbReplug)
	ON_COMMAND(ID_SNOOPUSB_STARTSERVICE, OnStartService)
	ON_COMMAND(ID_SNOOPUSB_STOPSERVICE, OnStopService)
	ON_COMMAND(ID_SNOOPUSB_CREATESERVICE, OnCreateService)
	ON_COMMAND(ID_SNOOPUSB_DELETESERVICE, OnDeleteService)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_VIEW, OnLogView)
	ON_BN_CLICKED(IDC_DELETE, OnLogDelete)
	ON_BN_CLICKED(IDC_TEST, OnTest)
	ON_WM_CREATE()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_TEST2, OnBnClickedTest2)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSniffUSBDlg message handlers

BOOL CSniffUSBDlg::OnInitDialog()
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
	
    // we want report style and more...
    m_cDevs.ModifyStyle(LVS_TYPEMASK, 
        LVS_AUTOARRANGE | LVS_SORTASCENDING | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS);
    m_cDevs.SetExtendedStyle(LVS_EX_FULLROWSELECT);
  
    // setup the columns
    CString sHeading;
    sHeading.LoadString(IDS_COL_VIDPID);
    m_cDevs.InsertColumn(0, sHeading);

    sHeading.LoadString(IDS_COL_FILTERINSTALLED);
    m_cDevs.InsertColumn(1, sHeading, LVCFMT_CENTER);

    sHeading.LoadString(IDS_COL_DESCRIPTION);
    m_cDevs.InsertColumn(2, sHeading);

	sHeading.LoadString(IDS_COL_PRESENT);
	m_cDevs.InsertColumn(3, sHeading);

	sHeading.LoadString(IDS_COL_DRIVER);
	m_cDevs.InsertColumn(4, sHeading);

	CheckDriver();
	CheckService();

    // fill it up, please
    OnRefresh();

	SetTimer(PN31SNOOP_TIMER, 1000, NULL);

	SaveControlPosition();
    
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSniffUSBDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CSniffUSBDlg::OnPaint() 
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
HCURSOR CSniffUSBDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CSniffUSBDlg::OnRefresh() 
{
	CString selectedDriver;

	// get the currently selected item

	GetSelectedDriver(selectedDriver);

	// reset the list of devices

	m_cDevs.DeleteAllItems();

	// loop through the list of device

	CDeviceMgr mgr;
	POSITION pos;

	pos = mgr.GetHeadPosition();
	while (pos != NULL)
	{
		CString sName, sFilter, sDescription, sPresent, sDriver;
		CDevice& device = mgr.GetNext(pos);
    
		sName = device.GetHardwareID();
		sDescription = device.GetDeviceDesc();
		sDriver = device.GetDriver();

		if (device.GetPresent())
			sPresent = "Yes";
		else
			sPresent = "No";

		sFilter.LoadString(IDS_NOTINSTALLED);
		if (device.GetLowerFilters().FindString(m_sFilterName))
			sFilter.LoadString(IDS_INSTALLED);

		// add to the list control

        int nIndex = m_cDevs.InsertItem(-1, sName);
        m_cDevs.SetItemText(nIndex, 1, sFilter);
        m_cDevs.SetItemText(nIndex, 2, sDescription);
        m_cDevs.SetItemText(nIndex, 3, sPresent);
		m_cDevs.SetItemText(nIndex, 4, sDriver);

		// set again the currently selected item

		if (sDriver == selectedDriver)
			m_cDevs.SetItemState(nIndex, LVIS_SELECTED, LVIS_SELECTED);

    }

	// automatically adjust the width of each column

    m_cDevs.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER );
    m_cDevs.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER );
    m_cDevs.SetColumnWidth(2, LVSCW_AUTOSIZE_USEHEADER );
    m_cDevs.SetColumnWidth(3, LVSCW_AUTOSIZE_USEHEADER );
    m_cDevs.SetColumnWidth(4, LVSCW_AUTOSIZE_USEHEADER );

	// update the log filename and size
	CheckLogFile();
}

BOOL CSniffUSBDlg::IsThereAFilter(LPCTSTR szVidPid)
{
    BOOL bThereIsAFilter = FALSE;
    TCHAR szEnumKey[MAX_PATH];
    _tcscpy(szEnumKey, "Enum\\USB\\");
    _tcscat(szEnumKey, szVidPid);
    HKEY hKey = NULL;
    if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szEnumKey, 0, KEY_ALL_ACCESS, &hKey))
    {
        DWORD dwIndex = 0;
        TCHAR sName[MAX_PATH];
        while(ERROR_SUCCESS == RegEnumKey(hKey, dwIndex, sName, MAX_PATH))
        {
            TRACE("Enumerated >%s<\n", sName);
            HKEY hInstKey = NULL;
            TCHAR szInstanceKey[MAX_PATH];
            _tcscpy(szInstanceKey, szEnumKey);
            _tcscat(szInstanceKey, _T("\\"));
            _tcscat(szInstanceKey, sName);
            if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szInstanceKey, 0, KEY_ALL_ACCESS, &hInstKey))
            {
                TCHAR szData[MAX_PATH];
                DWORD dwType = REG_SZ;
                DWORD dwSize = MAX_PATH;
                if(ERROR_SUCCESS == RegQueryValueEx(hInstKey, m_sLowerFilters, NULL, &dwType, (LPBYTE) szData, &dwSize))
                {
                    CString sData = szData;
                    sData.MakeLower();
                    if(NULL != _tcsstr(szData, m_sFilterName))
                    {
                        TRACE("Found filter!\n");
                        bThereIsAFilter = TRUE;
                    }
                }
                RegCloseKey(hInstKey);
                hInstKey = NULL;
            }
            dwIndex++;
        }
        RegCloseKey(hKey);
        hKey = NULL;
    }

    return bThereIsAFilter;
}

void CSniffUSBDlg::OnInstall() 
{
    CString sDriver;

	if (GetSelectedDriver(sDriver))
    {
	  	CDeviceMgr mgr;
		POSITION pos;

		pos = mgr.GetHeadPosition();
		while (pos != NULL)
		{
			CDevice& device = mgr.GetNext(pos);

			if (device.GetDriver() == sDriver)
			{
				CMultiSz sz = device.GetLowerFilters();
				sz.AddString(m_sFilterName);
				device.SetLowerFilters(sz);
			}
		}
    }
}

void CSniffUSBDlg::OnUninstall() 
{
    CString sDriver;

	if (GetSelectedDriver(sDriver))
    {
	  	CDeviceMgr mgr;
		POSITION pos;

		pos = mgr.GetHeadPosition();
		while (pos != NULL)
		{
			CDevice& device = mgr.GetNext(pos);

			if (device.GetDriver() == sDriver)
			{
				CMultiSz sz = device.GetLowerFilters();
				sz.RemoveString(m_sFilterName);
				device.SetLowerFilters(sz);
			}
		}
    }
}

void CSniffUSBDlg::OnReplug() 
{
	CString sDriver;

    if(GetSelectedDriver(sDriver))
    {
	  	CDeviceMgr mgr;
		POSITION pos;

		pos = mgr.GetHeadPosition();
		while (pos != NULL)
		{
			CDevice& device = mgr.GetNext(pos);

			if (device.GetDriver() == sDriver)
			{
				CWaitCursor cw;

				if (!device.Restart())
					MessageBox("failed to restart USB device");
			}
		}
	}
}

void CSniffUSBDlg::OnRclickUsbdevs(NMHDR* pNMHDR, LRESULT* pResult) 
{
    CMenu ctx;
    ctx.LoadMenu(IDR_SNOOPUSB);
    CMenu *popup = ctx.GetSubMenu(0);
    CPoint point;
    GetCursorPos(&point);
    CPoint pt = point;
    m_cDevs.ScreenToClient(&pt);
    UINT uFlags = 0;
    int nIndex = m_cDevs.HitTest(pt, &uFlags);
    if(LVHT_ONITEM & uFlags)
    {
        m_cDevs.SetItem(nIndex, 0, LVIF_STATE, NULL, 0, LVIS_SELECTED, LVIS_SELECTED, 0);
        CString sVidPid = m_cDevs.GetItemText(nIndex, 0);
        if(IsThereAFilter(sVidPid))
        {
            popup->EnableMenuItem(ID_SNOOPUSB_INSTALL, MF_BYCOMMAND | MF_GRAYED);
        }
        else
        {
            popup->EnableMenuItem(ID_SNOOPUSB_UNINSTALL, MF_BYCOMMAND | MF_GRAYED);
        }
        popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
    }
	
	*pResult = 0;
}

void CSniffUSBDlg::OnSnoopusbInstall() 
{
    OnInstall();	
}

void CSniffUSBDlg::OnSnoopusbUninstall() 
{
    OnUninstall();
}

void CSniffUSBDlg::OnSnoopusbReplug() 
{
    OnReplug();	
}

#define PN31SNOOP_SERVICE "pn31snoop"

void CSniffUSBDlg::CheckService()
{
	// check if "pn31snoop" service is installed

	SC_HANDLE hManager = OpenSCManager(NULL,SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);

	if (hManager == NULL)
	{
		MessageBox("Can't open service manager");
		return ;
	}

	SC_HANDLE hService = OpenService(hManager,"pn31snoop",DELETE);
	if (hService == NULL)
	{
		if (GetLastError() == ERROR_SERVICE_DOES_NOT_EXIST)
		{
			if (MessageBox("Service PN31Snoop","Would you like to install?",MB_YESNO)==IDYES)
			{
				hService = CreateService(hManager,"pn31snoop","pn31snoop (display)",
					0,SERVICE_KERNEL_DRIVER,SERVICE_DEMAND_START,SERVICE_ERROR_NORMAL,
					_T("system32\\drivers\\pn31snoop.sys"),
					NULL,NULL,NULL,NULL,NULL);
				if (hService == NULL)
				{
					MessageBox("Can't create service");
				}
				else
					CloseServiceHandle(hService);
			}
		}
	}
	else
	{
/*
		if (!DeleteService(hService))
			MessageBox("Cannot remove existing PN31snoop service");
*/
		CloseServiceHandle(hService);
	}


	CloseServiceHandle(hManager);
}

void CSniffUSBDlg::OnStartService() 
{
	// TODO: Add your command handler code here
	
	SC_HANDLE hManager = OpenSCManager(NULL,SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);

	if (hManager == NULL)
	{
		MessageBox("Can't open service manager");
	}
	else
	{
		SC_HANDLE hService = OpenService(hManager,PN31SNOOP_SERVICE,SC_MANAGER_ALL_ACCESS);
		if (hService == NULL)
		{
			MessageBox("Can't open service");
		}
		else
		{
			if (!::StartService(hService,0,NULL))
				MessageBox("Can't start service");

			CloseServiceHandle(hService);
		}

		CloseServiceHandle(hManager);
	}
}

void CSniffUSBDlg::OnStopService() 
{
	// TODO: Add your command handler code here
	
	SC_HANDLE hManager = OpenSCManager(NULL,SERVICES_ACTIVE_DATABASE,SC_MANAGER_ALL_ACCESS);

	if (hManager == NULL)
	{
		MessageBox("Can't open service manager");
	}
	else
	{
		SC_HANDLE hService = OpenService(hManager,PN31SNOOP_SERVICE,SC_MANAGER_ALL_ACCESS);
		if (hService == NULL)
		{
			MessageBox("Can't open service");
		}
		else
		{
			SERVICE_STATUS sStatus;

			memset(&sStatus, 0, sizeof(sStatus));

			if (!ControlService(hService,SERVICE_CONTROL_STOP,&sStatus))
				MessageBox("Can't stop service");

			CloseServiceHandle(hService);
		}

		CloseServiceHandle(hManager);
	}
}

void CSniffUSBDlg::OnCreateService() 
{
	// TODO: Add your command handler code here
	
	SC_HANDLE hManager = OpenSCManager(NULL,SERVICES_ACTIVE_DATABASE,SC_MANAGER_ALL_ACCESS);

	if (hManager == NULL)
	{
		MessageBox("Can't open service manager");
	}
	else
	{
		SC_HANDLE hService = CreateService(hManager,PN31SNOOP_SERVICE,"pn31snoop (display)",
			SERVICE_ALL_ACCESS,SERVICE_KERNEL_DRIVER,SERVICE_DEMAND_START,SERVICE_ERROR_NORMAL,
			"System32\\DRIVERS\\PN31SNOOP.SYS",
			NULL,NULL,NULL,NULL,NULL);

		if (hService == NULL)
		{
			MessageBox("Can't create service");
		}
		else
		{
			CloseServiceHandle(hService);
		}

		CloseServiceHandle(hManager);
	}
}

void CSniffUSBDlg::OnDeleteService() 
{
	// TODO: Add your command handler code here
	
	SC_HANDLE hManager = OpenSCManager(NULL,SERVICES_ACTIVE_DATABASE,SC_MANAGER_ALL_ACCESS);

	if (hManager == NULL)
	{
		MessageBox("Can't open service manager");
	}
	else
	{
		SC_HANDLE hService = OpenService(hManager,PN31SNOOP_SERVICE,SC_MANAGER_ALL_ACCESS);
		if (hService == NULL)
		{
			MessageBox("Can't open service");
		}
		else
		{
			if (!DeleteService(hService))
				MessageBox("Can't delete service");

			CloseServiceHandle(hService);
		}

		CloseServiceHandle(hManager);
	}
}

void CSniffUSBDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default

	if (nIDEvent == PN31SNOOP_TIMER)
	{
		OnRefresh();
	}
	
	CDialog::OnTimer(nIDEvent);
}

void CSniffUSBDlg::CheckDriver()
{
	HANDLE hFile;
	DWORD dwFileSize = -1;

	// get the size of the installed driver

    TCHAR sDriverPath[MAX_PATH];
    if(GetWindowsDirectory(sDriverPath, MAX_PATH) == 0)
    {
        TRACE("There was an error getting the windows directory!\n");
        return;
    }

	_tcscat(sDriverPath, _T("\\system32\\drivers\\pn31snoop.sys"));

	hFile = CreateFile(sDriverPath,GENERIC_READ|GENERIC_WRITE,0,NULL,
		OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		dwFileSize = GetFileSize(hFile, NULL);

		//  in case of errors, dwSize is -1

		CloseHandle(hFile);
	}

	// check the windows version (windows 98 / windows 2K / windows xp)

	OSVERSIONINFO osVersion;
	UINT nID;

	memset(&osVersion, 0, sizeof(osVersion));
	osVersion.dwOSVersionInfoSize = sizeof(osVersion);

	GetVersionEx(&osVersion);

	TRACE("GetVersionEx = %d.%d.%d %d\n",osVersion.dwMajorVersion,osVersion.dwMinorVersion,
		osVersion.dwBuildNumber,osVersion.dwPlatformId);

	// get the size of the driver contained in this executable

	if (osVersion.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		MessageBox("Using driver for Windows 2000/XP","nID",MB_OK);
		nID = SYS_SNOOPY_NT;
	}
	else
	{
		MessageBox("Windows 98 is not supported !","nID",MB_OK);
		return;
	}

    HRSRC hRsrc = FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(nID), RT_RCDATA);
    if(NULL == hRsrc)
    {
        TRACE("Couldn't locate driver binary in the resources!");
        return;
    }

    DWORD dwImageSize = SizeofResource(AfxGetResourceHandle(), hRsrc);
    if(dwImageSize == 0)
    {
        TRACE("Size of image is 0!");
        return;
    }

	// check if we need to copy the PN31snoop.sys file
	if (dwImageSize != dwFileSize
		&& MessageBox(_T("I need to copy PN31snoop.sys into your system directory"),_T("PN31snoop"),MB_YESNO) == IDYES)
	{

		HGLOBAL hBinImage = LoadResource(AfxGetResourceHandle(), hRsrc);
		if(NULL == hBinImage)
		{
			TRACE("Couldn't load binary image from resources!");
			return;
		}

		PVOID pBinImage = LockResource(hBinImage);
		if(NULL == pBinImage)
		{
			TRACE("Couldn't lock binary image from resources!");
			return;
		}

		// create a new file

		hFile = CreateFile(sDriverPath,GENERIC_READ | GENERIC_WRITE,0,NULL,
			CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			TRACE("Error creating >%s<: might be in use or protected by some rights!\n", sDriverPath);
			return;
		}

		DWORD dwWritten = 0;

		// write the content to the file

		if (!WriteFile(hFile, pBinImage, dwImageSize, &dwWritten, NULL))
		{
			TRACE("Failed to write %d bytes to >%s<\n", dwImageSize, sDriverPath);
			CloseHandle(hFile);
			return;
		}

		// truncate the file if needed

		if (!SetEndOfFile(hFile))
		{
			TRACE("Couldn't truncate disk file to %d bytes! ... proceed at your own risk!", dwImageSize);
		}

		CloseHandle(hFile);

		MessageBox(_T("PN31snoop.sys has been sucessfully copied to your system directory"),_T("PN31snoop"),MB_OK);
	}
}

void CSniffUSBDlg::CheckLogFile()
{
	// this function check the log file presence and size and
	// update the two field in the dialog box

	TCHAR sLogPath[MAX_PATH];
	HANDLE hFile;

	// compute the name of the log file

	if (GetWindowsDirectory(sLogPath, sizeof(sLogPath)) == 0)
	{
		// failure
		return ;
	}

	_tcscat(sLogPath, _T("\\usbsnoop.log"));

	m_LogFileName = sLogPath;
	m_LogSize = -1;

	hFile = CreateFile(sLogPath,GENERIC_READ,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,
		OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		m_LogSize = GetFileSize(hFile, NULL);

		//  in case of errors, dwSize is -1

		CloseHandle(hFile);
	}

	UpdateData(FALSE);
}

void CSniffUSBDlg::OnLogView() 
{
	// TODO: Add your control notification handler code here

	ShellExecute(*this, _T("open"), m_LogFileName, NULL, NULL, SW_SHOWNORMAL );
}

void CSniffUSBDlg::OnLogDelete() 
{
	// TODO: Add your control notification handler code here

	if (!DeleteFile(m_LogFileName))
		MessageBox(_T("Failed to delete log file"),_T("PN31snoop"),MB_OK);
}

void CSniffUSBDlg::OnTest() 
{
	// TODO: Add your control notification handler code here

	MessageBox(m_sFilterName,"m_sFilterName",MB_OK);
	
    CString sDriver;
    if(GetSelectedDriver(sDriver))
    {
	  	CDeviceMgr mgr;
		POSITION pos;

		pos = mgr.GetHeadPosition();
		while (pos != NULL)
		{
			CDevice& device = mgr.GetNext(pos);

			if (device.GetDriver() == sDriver)
			{
				CMultiSz sz = device.GetLowerFilters();
				const char * s;

				sz.DisplayBuffer();

				sz.First();
				while ((s = sz.Next()) != NULL)
					MessageBox(s,"LowerFilters",MB_OK);
			}
		}
    }
}

BOOL CSniffUSBDlg::GetSelectedDriver(CString &sDriver)
{
    UINT nSelected = m_cDevs.GetSelectedCount();
    
	if(nSelected == 0)
		return FALSE;

    int nIndex = m_cDevs.GetNextItem(-1, LVNI_SELECTED);
    sDriver = m_cDevs.GetItemText(nIndex, 4);

    return TRUE;

}

int CSniffUSBDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	if (!m_StatusBar.Create(this))
		return -1;
	
	return 0;
}

void CSniffUSBDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	
	CRect rect;

	GetClientRect(&rect);

	int x, y;

#define STATUSBAR_HEIGHT 20

	x  = 0;
	y  = rect.Height() - STATUSBAR_HEIGHT;
	cx = rect.Width();
	cy = STATUSBAR_HEIGHT;

	m_StatusBar.SetWindowPos(NULL,x,y,cx,cy,
		SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER|SWP_NOCOPYBITS);

	HDWP hdwp = ::BeginDeferWindowPos(N_CONTROL);

	for (int i=0;i<N_CONTROL;i++)
	{
		CWnd * pWnd = GetDlgItem(controlTab[i].nID);
		if (pWnd != NULL)
		{
			int top, left, right, bottom;
			const CRect & saveRect = controlTab[i].saveRect;

			top    = ComputePosition(controlTab[i].topAlign,    saveRect.top   , wholeRect.bottom, rect.bottom);
			left   = ComputePosition(controlTab[i].leftAlign,   saveRect.left  , wholeRect.right , rect.right);
			bottom = ComputePosition(controlTab[i].bottomAlign, saveRect.bottom, wholeRect.bottom, rect.bottom);
			right  = ComputePosition(controlTab[i].rightAlign,  saveRect.right , wholeRect.right , rect.right);

			x  = left;
			y  = top;
			cx = right - left;
			cy = bottom - top;

			hdwp = ::DeferWindowPos(hdwp,(HWND)*pWnd,NULL,x,y,cx,cy,
				SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER|SWP_NOCOPYBITS);
		}
	}

	::EndDeferWindowPos(hdwp);
}

void CSniffUSBDlg::SaveControlPosition()
{
	int i;

	for (i=0;i<N_CONTROL; i++)
	{
		UINT nID = controlTab[i].nID;
		GetControlRect(nID, &controlTab[i].saveRect);
	}

	GetClientRect(&wholeRect);
}

BOOL CSniffUSBDlg::GetControlRect(UINT nID, RECT * lpRect)
{
	CWnd * pWnd = GetDlgItem(nID);
	if (pWnd == NULL)
		return FALSE;

	pWnd->GetWindowRect(lpRect);
	ScreenToClient(lpRect);

	return TRUE;
}


int CSniffUSBDlg::ComputePosition(int align, int saveTop, int wholeBottom, int currentBottom)
{
	int res;

	switch (align)
	{
	case Y_TOP:
		res = saveTop;
		break;

	case Y_CENTER:
		res = (saveTop * currentBottom) / wholeBottom;
		break;

	case Y_BOTTOM:
		res = currentBottom - (wholeBottom - saveTop);
		break;
	}

	return res;
}

void CSniffUSBDlg::OnBnClickedTest2()
{
		m_hSniffer = CreateFile("\\\\.\\pn31snoop",	//PN31SNOOP_W32NAME_2K,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL, // no SECURITY_ATTRIBUTES structure
			OPEN_EXISTING, // No special create flags
			0, // No special attributes
			NULL);
		if (m_hSniffer != INVALID_HANDLE_VALUE) MessageBox("Ok", "Ok", MB_OK);

}
