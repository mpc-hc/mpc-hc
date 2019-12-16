/*
 * Copyright (C) 2008-2012, OctaneSnail <os@v12pwr.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RFS_H
#define RFS_H

#include "OutputPin.h"

#define RARFileSourceName   L"RAR File Source"

#define RFS_E_ABORT         MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x200)
#define RFS_E_NO_FILES      MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x201)
#define RFS_E_COMPRESSED    MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x202)
#define RFS_E_ENCRYPTED     MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x203)
#define RFS_E_MISSING_VOLS  MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x204)

typedef struct
{
	const char *extension;
	const GUID *guid;
} file_type_t;

class __declspec(uuid("9FFE11D2-29F2-463f-AD5F-C04A5EE2E58D"))
    CRARFileSource :
	public CBaseFilter,
	public IFileSourceFilter
{
public:
	CRARFileSource (LPUNKNOWN punk, HRESULT *phr);
	~CRARFileSource ();

	DECLARE_IUNKNOWN;

	static CUnknown * WINAPI CreateInstance (LPUNKNOWN punk, HRESULT *phr);

	// Reveals IFileSourceFilter
	STDMETHODIMP NonDelegatingQueryInterface (REFIID riid, void **ppv);

	// CBaseFilter
	int GetPinCount () { return 1; }
	CBasePin *GetPin (int n) { return n == 0 ? &m_pin : NULL; }
	STDMETHODIMP QueryFilterInfo (FILTER_INFO* pInfo);

	// IFileSourceFilter interface
	STDMETHODIMP Load (LPCOLESTR lpwszFileName, const AM_MEDIA_TYPE *pmt);
	STDMETHODIMP GetCurFile (LPOLESTR *ppszFileName, AM_MEDIA_TYPE *pmt);

private:
	static void UpdateArchiveName (wchar_t *ext, size_t len, int volume, bool new_numbering);
    HRESULT ScanArchive(wchar_t* archive_name, CRFSList<CRFSFile>* file_list, int* files_found, int* known_files_found);
    static INT_PTR CALLBACK DlgFileList (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	CRFSOutputPin m_pin;
	CCritSec m_lock;
	LPWSTR m_file_name;
	CRFSFile *m_file;

	static const file_type_t s_file_types [];
};

#endif // RFS_H
