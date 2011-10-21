/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2011 see AUTHORS
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "stdafx.h"
#include "DirectVobSubFilter.h"
#include "DirectVobSubPropPage.h"
#include "VSFilter.h"
#include "../../../DSUtil/MediaTypes.h"

#include <initguid.h>
#include <moreuuids.h>

/////////////////////////////////////////////////////////////////////////////
// CVSFilterApp

BEGIN_MESSAGE_MAP(CVSFilterApp, CWinApp)
END_MESSAGE_MAP()

CVSFilterApp::CVSFilterApp()
{
}

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL CVSFilterApp::InitInstance()
{
	if(!CWinApp::InitInstance()) {
		return FALSE;
	}

	SetRegistryKey(_T("Gabest"));

	DllEntryPoint(AfxGetInstanceHandle(), DLL_PROCESS_ATTACH, 0); // "DllMain" of the dshow baseclasses

	STARTUPINFO si;
	GetStartupInfo(&si);
	m_AppName = CString(si.lpTitle);
	m_AppName.Replace('\\', '/');
	m_AppName = m_AppName.Mid(m_AppName.ReverseFind('/')+1);
	m_AppName.MakeLower();

	return TRUE;
}

int CVSFilterApp::ExitInstance()
{
	DllEntryPoint(AfxGetInstanceHandle(), DLL_PROCESS_DETACH, 0); // "DllMain" of the dshow baseclasses

	return CWinApp::ExitInstance();
}

HINSTANCE CVSFilterApp::LoadAppLangResourceDLL()
{
	CString fn;
	fn.ReleaseBufferSetLength(::GetModuleFileName(m_hInstance, fn.GetBuffer(MAX_PATH), MAX_PATH));
	fn = fn.Mid(fn.ReverseFind('\\')+1);
	fn = fn.Left(fn.ReverseFind('.')+1);
	fn = fn + _T("lang");
	return ::LoadLibrary(fn);
}

CVSFilterApp theApp;

//////////////////////////////////////////////////////////////////////////

const AMOVIESETUP_MEDIATYPE sudPinTypesIn[] = {
	{&MEDIATYPE_NULL, &MEDIASUBTYPE_NULL},
	{&MEDIATYPE_Video, &MEDIASUBTYPE_YUY2},
	{&MEDIATYPE_Video, &MEDIASUBTYPE_YV12},
	{&MEDIATYPE_Video, &MEDIASUBTYPE_I420},
	{&MEDIATYPE_Video, &MEDIASUBTYPE_IYUV},
	{&MEDIATYPE_Video, &MEDIASUBTYPE_RGB32},
	{&MEDIATYPE_Video, &MEDIASUBTYPE_RGB565},
	{&MEDIATYPE_Video, &MEDIASUBTYPE_RGB555},
	{&MEDIATYPE_Video, &MEDIASUBTYPE_RGB24},
};

const AMOVIESETUP_MEDIATYPE sudPinTypesIn2[] = {
	{&MEDIATYPE_Text, &MEDIASUBTYPE_NULL},
	{&MEDIATYPE_Subtitle, &MEDIASUBTYPE_NULL},
};

const AMOVIESETUP_MEDIATYPE sudPinTypesOut[] = {
	{&MEDIATYPE_Video, &MEDIASUBTYPE_None},
};

const AMOVIESETUP_PIN sudpPins[] = {
	{L"Input", FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesIn), sudPinTypesIn},
	{L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesOut), sudPinTypesOut},
	{L"Input2", TRUE, FALSE, FALSE, TRUE, &CLSID_NULL, NULL, countof(sudPinTypesIn2), sudPinTypesIn2}
};

/*const*/
AMOVIESETUP_FILTER sudFilter[] = {
	{&__uuidof(CDirectVobSubFilter), L"DirectVobSub", MERIT_DO_NOT_USE, countof(sudpPins), sudpPins, CLSID_LegacyAmFilterCategory},
	{&__uuidof(CDirectVobSubFilter2), L"DirectVobSub (auto-loading version)", MERIT_PREFERRED+2, countof(sudpPins), sudpPins, CLSID_LegacyAmFilterCategory},
};

CFactoryTemplate g_Templates[] = {
	{sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CDirectVobSubFilter>, NULL, &sudFilter[0]},
	{sudFilter[1].strName, sudFilter[1].clsID, CreateInstance<CDirectVobSubFilter2>, NULL, &sudFilter[1]},
	{L"DVSMainPPage", &__uuidof(CDVSMainPPage), CreateInstance<CDVSMainPPage>},
	{L"DVSGeneralPPage", &__uuidof(CDVSGeneralPPage), CreateInstance<CDVSGeneralPPage>},
	{L"DVSMiscPPage", &__uuidof(CDVSMiscPPage), CreateInstance<CDVSMiscPPage>},
	{L"DVSTimingPPage", &__uuidof(CDVSTimingPPage), CreateInstance<CDVSTimingPPage>},
	{L"DVSZoomPPage", &__uuidof(CDVSZoomPPage), CreateInstance<CDVSZoomPPage>},
	{L"DVSColorPPage", &__uuidof(CDVSColorPPage), CreateInstance<CDVSColorPPage>},
	{L"DVSPathsPPage", &__uuidof(CDVSPathsPPage), CreateInstance<CDVSPathsPPage>},
	{L"DVSAboutPPage", &__uuidof(CDVSAboutPPage), CreateInstance<CDVSAboutPPage>},
};

int g_cTemplates = countof(g_Templates);

//////////////////////////////

STDAPI DllRegisterServer()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if(theApp.GetProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_SEENDIVXWARNING), 0) != 1) {
		theApp.WriteProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_SEENDIVXWARNING), 0);
	}

	if(theApp.GetProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_VMRZOOMENABLED), -1) == -1) {
		theApp.WriteProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_VMRZOOMENABLED), 0);
	}

	if(theApp.GetProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_ENABLEZPICON), -1) == -1) {
		theApp.WriteProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_ENABLEZPICON), 0);
	}

	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
	//	DVS_WriteProfileInt2(IDS_R_GENERAL, IDS_RG_SEENDIVXWARNING, 0);

	return AMovieDllRegisterServer2(FALSE);
}

void CALLBACK DirectVobSub(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow)
{
	if(FAILED(::CoInitialize(0))) {
		return;
	}

	CComPtr<IBaseFilter> pFilter;
	CComQIPtr<ISpecifyPropertyPages> pSpecify;

	if(SUCCEEDED(pFilter.CoCreateInstance(__uuidof(CDirectVobSubFilter))) && (pSpecify = pFilter)) {
		ShowPPage(pFilter, hwnd);
	}

	::CoUninitialize();
}
