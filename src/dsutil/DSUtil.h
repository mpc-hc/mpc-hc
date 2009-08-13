/* 
 *	Copyright (C) 2003-2006 Gabest
 *	http://www.gabest.org
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

#pragma once

#ifdef UNICODE
#ifdef DEBUG
#pragma comment(lib, "dsutilDU")
#else
#pragma comment(lib, "dsutilRU")
#endif
#else
#ifdef DEBUG
#pragma comment(lib, "dsutilD")
#else
#pragma comment(lib, "dsutilR")
#endif
#endif

#include "NullRenderers.h"
#include "HdmvClipInfo.h"
#include "H264Nalu.h"
//#include "MediaTypes.h"
#include "MediaTypeEx.h"
#include "vd.h"
#include "text.h"

#define LCID_NOSUBTITLES			-1

extern void DumpStreamConfig(TCHAR* fn, IAMStreamConfig* pAMVSCCap);
extern int CountPins(IBaseFilter* pBF, int& nIn, int& nOut, int& nInC, int& nOutC);
extern bool IsSplitter(IBaseFilter* pBF, bool fCountConnectedOnly = false);
extern bool IsMultiplexer(IBaseFilter* pBF, bool fCountConnectedOnly = false);
extern bool IsStreamStart(IBaseFilter* pBF);
extern bool IsStreamEnd(IBaseFilter* pBF);
extern bool IsVideoRenderer(IBaseFilter* pBF);
extern bool IsAudioWaveRenderer(IBaseFilter* pBF);
extern IBaseFilter* GetUpStreamFilter(IBaseFilter* pBF, IPin* pInputPin = NULL);
extern IPin* GetUpStreamPin(IBaseFilter* pBF, IPin* pInputPin = NULL);
extern IPin* GetFirstPin(IBaseFilter* pBF, PIN_DIRECTION dir = PINDIR_INPUT);
extern IPin* GetFirstDisconnectedPin(IBaseFilter* pBF, PIN_DIRECTION dir);
extern void NukeDownstream(IBaseFilter* pBF, IFilterGraph* pFG);
extern void NukeDownstream(IPin* pPin, IFilterGraph* pFG);
extern IBaseFilter* FindFilter(LPCWSTR clsid, IFilterGraph* pFG);
extern IBaseFilter* FindFilter(const CLSID& clsid, IFilterGraph* pFG);
extern CStringW GetFilterName(IBaseFilter* pBF);
extern CStringW GetPinName(IPin* pPin);
extern IFilterGraph* GetGraphFromFilter(IBaseFilter* pBF);
extern IBaseFilter* GetFilterFromPin(IPin* pPin);
extern IPin* AppendFilter(IPin* pPin, CString DisplayName, IGraphBuilder* pGB);
extern IPin* InsertFilter(IPin* pPin, CString DisplayName, IGraphBuilder* pGB);
extern void ExtractMediaTypes(IPin* pPin, CAtlArray<GUID>& types);
extern void ExtractMediaTypes(IPin* pPin, CAtlList<CMediaType>& mts);
extern void ShowPPage(CString DisplayName, HWND hParentWnd);
extern void ShowPPage(IUnknown* pUnknown, HWND hParentWnd);
extern CLSID GetCLSID(IBaseFilter* pBF);
extern CLSID GetCLSID(IPin* pPin);
extern bool IsCLSIDRegistered(LPCTSTR clsid);
extern bool IsCLSIDRegistered(const CLSID& clsid);
extern void CStringToBin(CString str, CAtlArray<BYTE>& data);
extern CString BinToCString(BYTE* ptr, int len);
typedef enum {CDROM_NotFound, CDROM_Audio, CDROM_VideoCD, CDROM_DVDVideo, CDROM_Unknown} cdrom_t;
extern cdrom_t GetCDROMType(TCHAR drive, CAtlList<CString>& files);
extern CString GetDriveLabel(TCHAR drive);
extern bool GetKeyFrames(CString fn, CUIntArray& kfs);
extern DVD_HMSF_TIMECODE RT2HMSF(REFERENCE_TIME rt, double fps = 0);
extern REFERENCE_TIME HMSF2RT(DVD_HMSF_TIMECODE hmsf, double fps = 0);
extern void memsetd(void* dst, unsigned int c, int nbytes);
extern bool ExtractBIH(const AM_MEDIA_TYPE* pmt, BITMAPINFOHEADER* bih);
extern bool ExtractBIH(IMediaSample* pMS, BITMAPINFOHEADER* bih);
extern bool ExtractAvgTimePerFrame(const AM_MEDIA_TYPE* pmt, REFERENCE_TIME& rtAvgTimePerFrame);
extern bool ExtractDim(const AM_MEDIA_TYPE* pmt, int& w, int& h, int& arx, int& ary);
extern bool MakeMPEG2MediaType(CMediaType& mt, BYTE* seqhdr, DWORD len, int w, int h);
extern unsigned __int64 GetFileVersion(LPCTSTR fn);
extern bool CreateFilter(CStringW DisplayName, IBaseFilter** ppBF, CStringW& FriendlyName);
extern IBaseFilter* AppendFilter(IPin* pPin, IMoniker* pMoniker, IGraphBuilder* pGB);
extern CStringW GetFriendlyName(CStringW DisplayName);
extern HRESULT LoadExternalObject(LPCTSTR path, REFCLSID clsid, REFIID iid, void** ppv);
extern HRESULT LoadExternalFilter(LPCTSTR path, REFCLSID clsid, IBaseFilter** ppBF);
extern HRESULT LoadExternalPropertyPage(IPersist* pP, REFCLSID clsid, IPropertyPage** ppPP);
extern void UnloadExternalObjects();
extern CString MakeFullPath(LPCTSTR path);
extern CString GetMediaTypeName(const GUID& guid);
extern GUID GUIDFromCString(CString str);
extern HRESULT GUIDFromCString(CString str, GUID& guid);
extern CString CStringFromGUID(const GUID& guid);
extern CStringW UTF8To16(LPCSTR utf8);
extern CStringA UTF16To8(LPCWSTR utf16);
extern CString ISO6391ToLanguage(LPCSTR code);
extern CString ISO6392ToLanguage(LPCSTR code);
extern LCID    ISO6391ToLcid(LPCSTR code);
extern LCID    ISO6392ToLcid(LPCSTR code);
extern CString ISO6391To6392(LPCSTR code);
extern CString ISO6392To6391(LPCSTR code);
extern CString LanguageToISO6392(LPCTSTR lang);
extern int MakeAACInitData(BYTE* pData, int profile, int freq, int channels);
extern BOOL CFileGetStatus(LPCTSTR lpszFileName, CFileStatus& status);
extern bool DeleteRegKey(LPCTSTR pszKey, LPCTSTR pszSubkey);
extern bool SetRegKeyValue(LPCTSTR pszKey, LPCTSTR pszSubkey, LPCTSTR pszValueName, LPCTSTR pszValue);
extern bool SetRegKeyValue(LPCTSTR pszKey, LPCTSTR pszSubkey, LPCTSTR pszValue);
extern void RegisterSourceFilter(const CLSID& clsid, const GUID& subtype2, LPCTSTR chkbytes, LPCTSTR ext = NULL, ...);
extern void RegisterSourceFilter(const CLSID& clsid, const GUID& subtype2, const CAtlList<CString>& chkbytes, LPCTSTR ext = NULL, ...);
extern void UnRegisterSourceFilter(const GUID& subtype);
extern LPCTSTR GetDXVAMode(const GUID* guidDecoder);
extern void DumpBuffer(BYTE* pBuffer, int nSize);
extern CString ReftimeToString(const REFERENCE_TIME& rtVal);
REFERENCE_TIME StringToReftime(LPCTSTR strVal);
extern COLORREF YCrCbToRGB_Rec601(BYTE Y, BYTE Cr, BYTE Cb);
extern COLORREF YCrCbToRGB_Rec709(BYTE Y, BYTE Cr, BYTE Cb);
extern DWORD	YCrCbToRGB_Rec601(BYTE A, BYTE Y, BYTE Cr, BYTE Cb);
extern DWORD	YCrCbToRGB_Rec709(BYTE A, BYTE Y, BYTE Cr, BYTE Cb);

class CPinInfo : public PIN_INFO
{
public:
	CPinInfo() {pFilter = NULL;}
	~CPinInfo() {if(pFilter) pFilter->Release();}
};

class CFilterInfo : public FILTER_INFO
{
public:
	CFilterInfo() {pGraph = NULL;}
	~CFilterInfo() {if(pGraph) pGraph->Release();}
};

#define BeginEnumFilters(pFilterGraph, pEnumFilters, pBaseFilter) \
	{CComPtr<IEnumFilters> pEnumFilters; \
	if(pFilterGraph && SUCCEEDED(pFilterGraph->EnumFilters(&pEnumFilters))) \
	{ \
		for(CComPtr<IBaseFilter> pBaseFilter; S_OK == pEnumFilters->Next(1, &pBaseFilter, 0); pBaseFilter = NULL) \
		{ \

#define EndEnumFilters }}}

#define BeginEnumCachedFilters(pGraphConfig, pEnumFilters, pBaseFilter) \
	{CComPtr<IEnumFilters> pEnumFilters; \
	if(pGraphConfig && SUCCEEDED(pGraphConfig->EnumCacheFilter(&pEnumFilters))) \
	{ \
		for(CComPtr<IBaseFilter> pBaseFilter; S_OK == pEnumFilters->Next(1, &pBaseFilter, 0); pBaseFilter = NULL) \
		{ \

#define EndEnumCachedFilters }}}

#define BeginEnumPins(pBaseFilter, pEnumPins, pPin) \
	{CComPtr<IEnumPins> pEnumPins; \
	if(pBaseFilter && SUCCEEDED(pBaseFilter->EnumPins(&pEnumPins))) \
	{ \
		for(CComPtr<IPin> pPin; S_OK == pEnumPins->Next(1, &pPin, 0); pPin = NULL) \
		{ \

#define EndEnumPins }}}

#define BeginEnumMediaTypes(pPin, pEnumMediaTypes, pMediaType) \
	{CComPtr<IEnumMediaTypes> pEnumMediaTypes; \
	if(pPin && SUCCEEDED(pPin->EnumMediaTypes(&pEnumMediaTypes))) \
	{ \
		AM_MEDIA_TYPE* pMediaType = NULL; \
		for(; S_OK == pEnumMediaTypes->Next(1, &pMediaType, NULL); DeleteMediaType(pMediaType), pMediaType = NULL) \
		{ \

#define EndEnumMediaTypes(pMediaType) } if(pMediaType) DeleteMediaType(pMediaType); }}

#define BeginEnumSysDev(clsid, pMoniker) \
	{CComPtr<ICreateDevEnum> pDevEnum4$##clsid; \
	pDevEnum4$##clsid.CoCreateInstance(CLSID_SystemDeviceEnum); \
	CComPtr<IEnumMoniker> pClassEnum4$##clsid; \
	if(SUCCEEDED(pDevEnum4$##clsid->CreateClassEnumerator(clsid, &pClassEnum4$##clsid, 0)) \
	&& pClassEnum4$##clsid) \
	{ \
		for(CComPtr<IMoniker> pMoniker; pClassEnum4$##clsid->Next(1, &pMoniker, 0) == S_OK; pMoniker = NULL) \
		{ \

#define EndEnumSysDev }}}

#define QI(i) (riid == __uuidof(i)) ? GetInterface((i*)this, ppv) :
#define QI2(i) (riid == IID_##i) ? GetInterface((i*)this, ppv) :

template <typename T> __inline void INITDDSTRUCT(T& dd)
{
    ZeroMemory(&dd, sizeof(dd));
    dd.dwSize = sizeof(dd);
}

#define countof(array) (sizeof(array)/sizeof(array[0]))

template <class T>
static CUnknown* WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT* phr)
{
	*phr = S_OK;
    CUnknown* punk = DNew T(lpunk, phr);
    if(punk == NULL) *phr = E_OUTOFMEMORY;
	return punk;
}
