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

#include <windows.h>
#include <streams.h>
#ifdef STANDALONE_FILTER
#include <initguid.h>
#endif
#include <strsafe.h>

#include "RFS.h"
#include "Utils.h"
#include "RAR.h"
#include "Anchor.h"
#include "resource.h"
#include "Mediatype.h"
#include "File.h"


#define RFS_GUID_STRING "{9FFE11D2-29F2-463f-AD5F-C04A5EE2E58D}"


//{1AC0BEBD-4D2B-45AD-BCEB-F2C41C5E3788}
DEFINE_GUID(MEDIASUBTYPE_Matroska,
0x1AC0BEBD, 0x4D2B, 0x45AD, 0xBC, 0xEB, 0xF2, 0xC4, 0x1C, 0x5E, 0x37, 0x88);

#ifdef STANDALONE_FILTER

// Setup information
const AMOVIESETUP_MEDIATYPE sudPinTypes =
{
	&MEDIATYPE_Stream,		// Major type
	&MEDIASUBTYPE_NULL		// Minor type
};

const AMOVIESETUP_PIN sudpPin =
{
	L"Output",		// Pin name
	FALSE,			// Is it rendered
	TRUE,			// Is it an output
	FALSE,			// Zero instances allowed
	FALSE,			// Multiple instances allowed
	&CLSID_NULL,	// Connects to filter
	NULL,			// Connects to pin
	1,				// Number of types
	&sudPinTypes	// Pin types
};

const AMOVIESETUP_FILTER sudRARFileSource =
{
	&__uuidof(CRARFileSource),	// Filter CLSID
	RARFileSourceName,		// Filter name
	MERIT_UNLIKELY,			// Filter merit
	1,						// Number of pins
	&sudpPin,				// Pin information
	CLSID_LegacyAmFilterCategory
};

// List of class IDs and creator functions for the class factory.

CFactoryTemplate g_Templates [] =
{
	{
		RARFileSourceName,
		&__uuidof(CRARFileSource),
		CRARFileSource::CreateInstance,
		NULL,
		&sudRARFileSource
	}
};

int g_cTemplates = sizeof (g_Templates) / sizeof (g_Templates [0]);

#endif

/* static */
const
file_type_t CRARFileSource::s_file_types [] =
{
	{ "avi", &MEDIASUBTYPE_Avi },
	{ "mpg", &MEDIASUBTYPE_MPEG1System },
	{ "vob", &MEDIASUBTYPE_MPEG2_PROGRAM },
	{ "mkv", &MEDIASUBTYPE_Matroska },
	{ "mka", &MEDIASUBTYPE_Matroska },
	{ "mks", &MEDIASUBTYPE_Matroska },
	{ "mov", &MEDIASUBTYPE_QTMovie },
	{ "mp4", &MEDIASUBTYPE_QTMovie },
	{ "wav", &MEDIASUBTYPE_WAVE },
	{ "mp3", &MEDIASUBTYPE_MPEG1Audio },
	{ "mpa", &MEDIASUBTYPE_MPEG1Audio },
	{ "mpv", &MEDIASUBTYPE_MPEG1Video },
	{ "dat", &MEDIASUBTYPE_MPEG1VideoCD },
	{ NULL, NULL }
};

#ifdef STANDALONE_FILTER

extern "C" BOOL WINAPI DllEntryPoint (HINSTANCE, ULONG, LPVOID);

BOOL WINAPI DllMain (HINSTANCE hDllHandle, DWORD dwReason, LPVOID lpReserved)
{
	return DllEntryPoint (hDllHandle, dwReason, lpReserved);
}

STDAPI DllRegisterServer ()
{
	HKEY key;
	LONG ret;
	wchar_t key_name [] = L"Media Type\\Extensions\\.rar";
	wchar_t byte_key_name [] = L"Media Type\\{E436EB83-524F-11CE-9F53-0020AF0BA770}\\{7F1CD2B6-DFC6-4F4C-982B-0472673920AD}";
	char bytes [] = "0,4,,52617221";
	
	ret = RegCreateKey (HKEY_CLASSES_ROOT, key_name, &key);

	if (ret != ERROR_SUCCESS)
	{
		ret = RegOpenKey (HKEY_CLASSES_ROOT, key_name, &key);

		if (ret != ERROR_SUCCESS)
			return ret;
	}
	
	ret = RegSetValueExA (key, "Source Filter", 0, REG_SZ, (BYTE *) RFS_GUID_STRING, (DWORD) strlen (RFS_GUID_STRING) + 1);

	if (ret != ERROR_SUCCESS)
	{
		RegCloseKey (key);
		return ret;
	}

	RegCloseKey (key);

	ret = RegCreateKey (HKEY_CLASSES_ROOT, byte_key_name, &key);

	if (ret != ERROR_SUCCESS)
	{
		ret = RegOpenKey (HKEY_CLASSES_ROOT, byte_key_name, &key);

		if (ret != ERROR_SUCCESS)
			return ret;
	}

	ret = RegSetValueExA (key, "0", 0, REG_SZ, (BYTE *) bytes, (DWORD) strlen (bytes));
	ret = RegSetValueExA (key, "Source Filter", 0, REG_SZ, (BYTE *) RFS_GUID_STRING, (DWORD) strlen (RFS_GUID_STRING) + 1);

	if (ret != ERROR_SUCCESS)
	{
		RegCloseKey (key);
		return ret;
	}

	RegCloseKey (key);

	return AMovieDllRegisterServer2 (TRUE);
}

STDAPI DllUnregisterServer ()
{
	HKEY key;
	LONG ret;
	char value [40];
	DWORD len = sizeof (value);
	wchar_t key_name [] = L"Media Type\\Extensions\\.rar";
	wchar_t byte_key_name [] = L"Media Type\\{E436EB83-524F-11CE-9F53-0020AF0BA770}\\{7F1CD2B6-DFC6-4F4C-982B-0472673920AD}";

	ret = RegOpenKey (HKEY_CLASSES_ROOT, key_name, &key);
	if (ret == ERROR_SUCCESS)
	{
		ret = RegQueryValueExA (key, "Source Filter", NULL, NULL, (BYTE *) value, &len);
		RegCloseKey (key);

		if (ret == ERROR_SUCCESS)
		{
			if (!_stricmp (value, RFS_GUID_STRING))
				RegDeleteKey (HKEY_CLASSES_ROOT, key_name);
		}
	}

	ret = RegOpenKey (HKEY_CLASSES_ROOT, byte_key_name, &key);
	if (ret == ERROR_SUCCESS)
	{
		ret = RegQueryValueExA (key, "Source Filter", NULL, NULL, (BYTE *) value, &len);
		RegCloseKey (key);

		if (ret == ERROR_SUCCESS)
		{
			if (!_stricmp (value, RFS_GUID_STRING))
				RegDeleteKey (HKEY_CLASSES_ROOT, byte_key_name);
		}
	}

	return AMovieDllRegisterServer2 (FALSE);
}

#endif

CRARFileSource::CRARFileSource (LPUNKNOWN punk, HRESULT *phr) :
	CBaseFilter (RARFileSourceName, punk, &m_lock, __uuidof(CRARFileSource)),
	m_pin (this, &m_lock, phr),
	m_file_name (NULL),
	m_file (NULL)
{
	if (phr)
		*phr = S_OK;
} 

CRARFileSource::~CRARFileSource ()
{
	delete m_file_name;
	delete m_file;
}

CUnknown *CRARFileSource::CreateInstance (LPUNKNOWN punk, HRESULT *phr) // OK
{
	CRARFileSource *pNewObject = new CRARFileSource (punk, phr);

	if (pNewObject == NULL)
		*phr = E_OUTOFMEMORY;

	DbgSetModuleLevel (LOG_TRACE, 2);
	DbgLog((LOG_TRACE, 2, L"CRARFileSource::CreateInstance () succeded."));

	return pNewObject;
}

STDMETHODIMP CRARFileSource::NonDelegatingQueryInterface (REFIID riid, void **ppv)
{
	if (riid == IID_IFileSourceFilter)
		return GetInterface ((IFileSourceFilter *) this, ppv);
	else
		return CBaseFilter::NonDelegatingQueryInterface (riid, ppv);
}

/* static */
void CRARFileSource::UpdateArchiveName (wchar_t *ext, size_t len, int volume, bool new_numbering)
{
	if (new_numbering)
	{
		StringCchPrintf (ext, len + 1, L"%0*d", len, volume + 1);
		ext [len] = L'.';
	}
	else
	{
		ASSERT (len == 2);

		if (volume == 0)
		{
			ext [0] = L'a';
			ext [1] = L'r';
		}
		else
			StringCchPrintf (ext - 1, len + 2, L"%c%02d", 114 + (volume - 1) / 100, (volume - 1) % 100);
	}
}

#define HEADER_SKIP_FILE \
	delete [] rh.fh.filename; \
	SetFilePointerEx (hFile, rh.bytesRemaining, NULL, FILE_CURRENT); \
	continue;

HRESULT CRARFileSource::ScanArchive (wchar_t *archive_name, CRFSList<CRFSFile> *file_list, int *files_found, int *ok_files_found)
{
	DWORD dwBytesRead;
	char *filename = NULL;
	wchar_t *current_rar_filename = NULL, *rar_ext;
	bool first_archive_file = true, rewind;
	bool multi_volume = false, new_numbering = false;
	rar_header_t rh;
	BYTE marker [7];
	BYTE expected [7] = { 0x52, 0x61, 0x72, 0x21, 0x1A, 0x07, 0x00 };
	CRFSFilePart *new_part, *prev_part;
	LONGLONG collected;
	DWORD ret;
	DWORD volumes = 0;
	int volume_digits;
	CRFSFile *file = NULL;

	*ok_files_found = 0;
	int compressed_files_found = 0;
	int encrypted_files_found = 0;
	LARGE_INTEGER zero = {0};

	MediaType *mType;
	CRFSList<MediaType> mediaTypeList (true);

	Anchor<CRFSFile> af (&file);
	ArrayAnchor<wchar_t> acrf (&current_rar_filename);

	HANDLE hFile = INVALID_HANDLE_VALUE;
	Anchor<HANDLE> ha (&hFile);

	int cch = lstrlen (archive_name) + 1;

	current_rar_filename = new wchar_t [cch];
	if (!current_rar_filename)
	{
		ErrorMsg (0, L"Out of memory.");
		return E_OUTOFMEMORY;
	}

	CopyMemory (current_rar_filename, archive_name, cch * sizeof (wchar_t));

	rar_ext = wcsrchr (current_rar_filename, '.');

	if (getMediaTypeList (&mediaTypeList) == -1)
		return E_OUTOFMEMORY;

	// Scan through archive volume(s)
	while (true)
	{
		ha.Close ();
		DbgLog((LOG_TRACE, 2, L"Loading file \"%s\".", current_rar_filename));
		hFile = CreateFile (current_rar_filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			if (first_archive_file || rewind)
			{
				ErrorMsg (GetLastError (), L"Could not open file \"%s\".", current_rar_filename);
				return E_FAIL;
			}
			else
				break;
		}
		rewind = false;

		// Read marker.
		if (!ReadFile (hFile, marker, 7, &dwBytesRead, NULL) || dwBytesRead != 7)
		{
			ErrorMsg (GetLastError (), L"Could not read RAR header.");
			return E_FAIL;
		}

		if (memcmp (marker, expected, 7))
		{
			ErrorMsg (0, L"Incorrect RAR marker.");
			return E_UNEXPECTED;
		}

		// Read archive header.
		if (ret = ReadHeader (hFile, &rh))
			return ret;

		LOG_HEADER(&rh);

		if (rh.ch.type != HEADER_TYPE_ARCHIVE)
		{
			ErrorMsg (0, L"Unexpected RAR header type.");
			return E_UNEXPECTED;
		}

		if (rh.ch.flags & MHD_PASSWORD)
		{
			ErrorMsg (0, L"Encrypted RAR volumes are not supported.");
			return RFS_E_ENCRYPTED;
		}

		if (first_archive_file)
		{
			new_numbering = !!(rh.ch.flags & MHD_NEWNUMBERING);
			multi_volume = !!(rh.ch.flags & MHD_VOLUME);

			if (multi_volume)
			{
				if (!rar_ext)
				{
					ErrorMsg (0, L"Input file does not end with .rar");
					return E_UNEXPECTED;
				}

				// Locate volume counter
				if (new_numbering)
				{
					volume_digits = 0;
					do
					{
						rar_ext --;
						volume_digits ++;
					} while (iswdigit (*(rar_ext - 1)));
				}
				else
				{
					rar_ext += 2;
					volume_digits = 2;
				}

				if (!(rh.ch.flags & MHD_FIRSTVOLUME))
				{
					DbgLog((LOG_TRACE, 2, L"Rewinding to the first file in the set."));
					UpdateArchiveName (rar_ext, volume_digits, volumes, new_numbering);

					first_archive_file = false;
					rewind = true;
					continue;
				}
			}
		}
		else
		{
			ASSERT (new_numbering == !!(rh.ch.flags & MHD_NEWNUMBERING));
			ASSERT (rh.ch.flags & MHD_VOLUME);
		}

		// Find file headers
		while (true)
		{
			// Read file header.
			if (ret = ReadHeader (hFile, &rh))
			{
				if (ret == ERROR_HANDLE_EOF)
					break;
				else
					return ret;
			}

			LOG_HEADER(&rh);

			if (rh.ch.type == HEADER_TYPE_END)
			{
				// TODO: Verify that the volume number in the header matches our volume counter.

				// Check for EARC_NEXT_VOLUME removed as this flag is missing on some archives.

				break;
			}
			if (rh.ch.type != HEADER_TYPE_FILE)
			{
				SetFilePointerEx (hFile, rh.bytesRemaining, NULL, FILE_CURRENT);
				DbgLog((LOG_TRACE, 2, L"Skipping unknown header of type %02x.", rh.ch.type));
				continue;
			}

			DbgLog((LOG_TRACE, 2, L"SIZE %08lx %08lx  OS %02x  CRC %08lx  TIMESTAMP %08lx  VERSION %d  METHOD %02x  LEN %04lx  ATTRIB %08lx",
				rh.fh.size.HighPart, rh.fh.size.LowPart, rh.fh.os, rh.fh.crc, rh.fh.timestamp, rh.fh.version, rh.fh.method, rh.fh.name_len, rh.fh.attributes));

			DbgLog((LOG_TRACE, 2, L"FILENAME \"%S\"", rh.fh.filename));

			if ((rh.ch.flags & LHD_WINDOWMASK) == LHD_DIRECTORY)
			{
				DbgLog((LOG_TRACE, 2, L"Skipping directory."));
				HEADER_SKIP_FILE
			}

			if (filename)
			{
				if (strcmp (filename, rh.fh.filename))
				{
					// TODO: We should probably dump the old file start fresh with this one
					// since the lazy error handling at other places may cause us to end up here.
					DbgLog((LOG_TRACE, 2, L"Incorrect file found."));
					HEADER_SKIP_FILE
				}

				if (!(rh.ch.flags & LHD_SPLIT_BEFORE))
				{
					// TODO: Decide if it's better to ignore the missing flag.
					DbgLog((LOG_TRACE, 2, L"LHD_SPLIT_BEFORE flag was not set as expected."));
					HEADER_SKIP_FILE
				}

				delete [] rh.fh.filename;
			}
			else
			{
				if (!multi_volume && rh.ch.flags & (LHD_SPLIT_BEFORE | LHD_SPLIT_AFTER))
				{
					ErrorMsg (0, L"Split file in a single volume archive.");
					delete [] rh.fh.filename;
					return E_UNEXPECTED;
				}

				if (rh.ch.flags & LHD_SPLIT_BEFORE)
				{
					if (first_archive_file)
					{
						// Some archives incorrectly have MHD_FIRSTVOLUME set on all files, attempt rewind.
						DbgLog((LOG_TRACE, 2, L"Rewinding to the first file in the set."));
						UpdateArchiveName (rar_ext, volume_digits, volumes, new_numbering);

						rewind = true;

						delete [] rh.fh.filename;
						break;
					}
					else
					{
						// TODO: Decide if it's better to just abort the entire scanning process here.
						DbgLog((LOG_TRACE, 2, L"Not at the beginning of the file."));
						HEADER_SKIP_FILE
					}
				}

				(*files_found) ++;
				collected = 0;

				ASSERT (!file);

				file = new CRFSFile ();

				if (!file)
				{
					ErrorMsg (0, L"Out of memory.");
					delete [] rh.fh.filename;
					return E_OUTOFMEMORY;
				}

				file->media_type.SetType (&MEDIATYPE_Stream);
				file->media_type.SetSubtype (&MEDIASUBTYPE_NULL);
				file->filename = rh.fh.filename;
				file->size = rh.fh.size.QuadPart;
				filename = rh.fh.filename;

				if (rh.ch.flags & LHD_PASSWORD)
				{
					encrypted_files_found++;
					DbgLog((LOG_TRACE, 2, L"Encrypted files are not supported."));
					file->unsupported = true;
				}

				if (rh.fh.method != 0x30)
				{
					compressed_files_found++;
					DbgLog((LOG_TRACE, 2, L"Compressed files are not supported."));
					file->unsupported = true;
				}
			}

			if (!file->unsupported)
			{
				new_part = new CRFSFilePart ();

				if (!new_part)
				{
					ErrorMsg (0, L"Out of memory.");
					return E_OUTOFMEMORY;
				}

				// Is this the 1st part?
				if (!file->parts)
				{
					file->parts = 1;
					file->list = new_part;
				}
				else
				{
					file->parts ++;
					prev_part->next = new_part;
				}
				prev_part = new_part;

				SetFilePointerEx (hFile, zero, (LARGE_INTEGER*) &new_part->in_rar_offset, FILE_CURRENT);
				new_part->in_file_offset = collected;
				new_part->size = rh.bytesRemaining.QuadPart;

				new_part->file = CreateFile (current_rar_filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
				if (new_part->file == INVALID_HANDLE_VALUE)
				{
					ErrorMsg (GetLastError (), L"Could not open file \"%s\".", current_rar_filename);
					return E_FAIL;
				}
			}

			collected += rh.bytesRemaining.QuadPart;
			SetFilePointerEx (hFile, rh.bytesRemaining, NULL, FILE_CURRENT);

			// Is file complete?
			if (!(rh.ch.flags & LHD_SPLIT_AFTER))
			{
				if (!file->unsupported && file->size != collected)
					DbgLog((LOG_TRACE, 2, L"The file is not the sum of it's parts. expected = %lld, actual = %lld", file->size, collected));

				if (file->parts)
				{
					file->array = new CRFSFilePart [file->parts];

					if (!file->array)
					{
						ErrorMsg (0, L"Out of memory.");
						return E_OUTOFMEMORY;
					}

					CRFSFilePart *fp = file->list;
					file->list = NULL;
					for (int i = 0; i < file->parts; i ++)
					{
						CRFSFilePart *tmp;
						memcpy (file->array + i, fp, sizeof (CRFSFilePart));
						tmp = fp;
						fp = fp->next;
						tmp->file = INVALID_HANDLE_VALUE;
						delete tmp;
					}
				}

				if (!file->unsupported)
				{
					if (!checkFileForMediaType (file, &mediaTypeList, &mType))
						return E_OUTOFMEMORY;

					if (mType)
					{
#ifdef _DEBUG
						LPOLESTR majorType, subType;
						StringFromCLSID (mType->majorType, &majorType);
						StringFromCLSID (mType->subType, &subType);
						DbgLog((LOG_TRACE, 2, L"Filetype detection determined:"));
						DbgLog((LOG_TRACE, 2, L"  Major type: %s", majorType));
						DbgLog((LOG_TRACE, 2, L"  Subtype: %s", subType));
						CoTaskMemFree (majorType);
						CoTaskMemFree (subType);
#endif
						file->media_type.SetType (&mType->majorType);
						file->media_type.SetSubtype (&mType->subType);
						file->type_known = true;
						(*ok_files_found) ++;
					}
				}

				// TODO: Figure out if checking file extensions is necessary anymore.
				if (!file->type_known)
				{
					char *ext = strrchr (file->filename, '.');

					if (ext)
					{
						ext ++;
						int i;

						for (i = 0; s_file_types [i].extension; i ++)
						{
							if (!_stricmp (ext, s_file_types [i].extension))
								break;
						}

						if (s_file_types [i].extension)
						{
							file->media_type.SetSubtype (s_file_types [i].guid);
							file->type_known = true;
							if (!file->unsupported)
								(*ok_files_found) ++;
						}
						else
						{
							file->unsupported = true;
							DbgLog((LOG_TRACE, 2, L"Unknown file extension."));
						}
					}
					else
					{
						file->unsupported = true;
						DbgLog((LOG_TRACE, 2, L"No file extension."));
					}
				}

				if (filename != file->filename)
					delete filename;

				filename = NULL;

				file_list->InsertLast (file);
				file = NULL;
			}
		}

		first_archive_file = false;

		if (rewind)
			continue;

		if (!multi_volume)
			break;

		// Continue to the next volume.
		volumes ++;
		UpdateArchiveName (rar_ext, volume_digits, volumes, new_numbering);
	}

	if (file)
	{
		// TODO: Decide if we should allow playback of truncated files.
		ErrorMsg (0, L"Couldn't find all archive volumes.");
		return RFS_E_MISSING_VOLS;
	}

	// only show error messages if no usable file was found
	if (*ok_files_found == 0)
	{
		if (encrypted_files_found > 0)
		{
			ErrorMsg (0, L"Encrypted files are not supported.");
			return RFS_E_ENCRYPTED;
		}
		else if (compressed_files_found > 0)
		{
			ErrorMsg (0, L"Compressed files are not supported.");
			return RFS_E_COMPRESSED;
		}
		else
		{
			ErrorMsg (0, L"No media files found in the archive.");
			return RFS_E_NO_FILES;
		}
	}

	return S_OK;
}

/* static */
INT_PTR CALLBACK CRARFileSource::DlgFileList (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int index;

	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		int len;
		CRFSList<CRFSFile> *file_list = (CRFSList<CRFSFile> *) lParam;
		CRFSFile *file = file_list->First ();
		wchar_t *tempString;

		while (file)
		{
			if (!file->unsupported)
			{
				len = (int) strlen (file->filename) + 1;
				tempString = new wchar_t [len];
				MultiByteToWideChar (CP_ACP, 0, file->filename, -1, tempString, len);
				index = ListBox_AddString (GetDlgItem (hwndDlg, IDC_FILELIST), tempString);
				ListBox_SetItemData (GetDlgItem (hwndDlg, IDC_FILELIST), index, file);
				delete [] tempString;
			}

			file = file_list->Next (file);
		}
		ListBox_SetCurSel (GetDlgItem (hwndDlg, IDC_FILELIST), 0);
		SetForegroundWindow (hwndDlg);
		return TRUE;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
				index = ListBox_GetCurSel (GetDlgItem (hwndDlg, IDC_FILELIST));
				EndDialog (hwndDlg, ListBox_GetItemData (GetDlgItem (hwndDlg, IDC_FILELIST), index));
				return TRUE;

		case IDCANCEL:
				EndDialog (hwndDlg, NULL);
				return TRUE;

		case IDC_FILELIST:
			switch (HIWORD (wParam))
			{
				case LBN_DBLCLK:
					index = ListBox_GetCurSel ((HWND) lParam);
					EndDialog (hwndDlg, ListBox_GetItemData ((HWND) lParam, index));
					return TRUE;
			}
		}
		break;

	case WM_CLOSE:
		EndDialog (hwndDlg, NULL);
		return TRUE;
	}

	return FALSE;
}

//  IFileSourceFilter methods

STDMETHODIMP CRARFileSource::Load (LPCOLESTR lpwszFileName, const AM_MEDIA_TYPE *pmt)
{
	CRFSList <CRFSFile> file_list;
	int num_files, num_ok_files;
	CAutoLock lck (&m_lock);
	HRESULT hr;

	if (!lpwszFileName)
		return E_POINTER;

	if (m_file)
	{
		DbgLog((LOG_TRACE, 2, L"CRARFileSource::Load called with file already loaded."));
		return E_UNEXPECTED;
	}

	int cch = lstrlen (lpwszFileName) + 1;

	if (m_file_name)
		delete m_file_name;

	m_file_name = new WCHAR [cch];
	if (!m_file_name)
	{
		ErrorMsg (0, L"Out of memory.");
		return E_OUTOFMEMORY;
	}

	CopyMemory (m_file_name, lpwszFileName, cch * sizeof (WCHAR));

	hr = ScanArchive ((wchar_t *) lpwszFileName, &file_list, &num_files, &num_ok_files);

	DbgLog((LOG_TRACE, 2, L"Found %d files out of which %d are media files.", num_files, num_ok_files));

	if (!num_ok_files)
	{
		file_list.Clear ();
		return hr; // TODO: Figure out a better error code.
	}

	if (num_ok_files == 1)
	{
		m_file = file_list.First ();

		while (m_file->unsupported)
			m_file = file_list.Next (m_file);
	}
	else
	{
#ifdef STANDALONE_FILTER
		m_file = (CRFSFile *) DialogBoxParam (g_hInst, MAKEINTRESOURCE(IDD_FILELIST), 0, DlgFileList, (LPARAM) &file_list);
#else
		m_file = (CRFSFile *) DialogBoxParam (GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_FILELIST), 0, DlgFileList, (LPARAM) &file_list);
#endif

		if (!m_file)
		{
			file_list.Clear ();
			return RFS_E_ABORT;
		}
	}

	if (pmt)
		m_file->media_type = *pmt;

	m_file->Unlink ();
	m_pin.SetFile (m_file);

	file_list.Clear ();

	return S_OK;
}

// Behaves like IPersistFile::Load
STDMETHODIMP CRARFileSource::GetCurFile (LPOLESTR *ppszFileName, AM_MEDIA_TYPE *pmt)
{
	if (!ppszFileName)
		return E_POINTER;

	if (m_file)
	{
		DWORD n = sizeof (WCHAR) * (strlen (m_file->filename) + 1);

		*ppszFileName = (LPOLESTR) CoTaskMemAlloc (n);

		if (*ppszFileName != NULL)
			mbstowcs (*ppszFileName, m_file->filename, n);
		else
			return E_OUTOFMEMORY;
	}
	else
		*ppszFileName = NULL;

	if (pmt != NULL)
		CopyMediaType (pmt, &m_file->media_type);

	return NOERROR;
}

STDMETHODIMP CRARFileSource::QueryFilterInfo (FILTER_INFO* pInfo)
{
	CheckPointer(pInfo, E_POINTER);
	ValidateReadWritePtr(pInfo, sizeof(FILTER_INFO));
	wcscpy_s(pInfo->achName, RARFileSourceName);
	pInfo->pGraph = m_pGraph;
	if (m_pGraph) {
		m_pGraph->AddRef();
	}

	return S_OK;
}
