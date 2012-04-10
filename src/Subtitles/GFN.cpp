/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2012 see Authors.txt
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
#include <io.h>
#include "TextFile.h"
#include "GFN.h"

TCHAR* exttypestr[] = {
	_T("srt"), _T("sub"), _T("smi"), _T("psb"),
	_T("ssa"), _T("ass"), _T("idx"), _T("usf"),
	_T("xss"), _T("txt"), _T("ssf"), _T("rt"), _T("sup")
};

static TCHAR* ext[3][countof(exttypestr)] = {
	{
		_T(".srt"), _T(".sub"), _T(".smi"), _T(".psb"),
		_T(".ssa"), _T(".ass"), _T(".idx"), _T(".usf"),
		_T(".xss"), _T(".txt"), _T(".ssf"), _T(".rt"), _T(".sup")
	},
	{
		_T(".*.srt"), _T(".*.sub"), _T(".*.smi"), _T(".*.psb"),
		_T(".*.ssa"), _T(".*.ass"), _T(".*.dummyidx"), _T(".*.usf"),
		_T(".*.xss"), _T(".*.txt"), _T(".*.ssf"), _T(".*.rt"), _T(".*.sup")
	},
	{
		_T("-*.srt"), _T("-*.sub"), _T("-*.smi"), _T("-*.psb"),
		_T("-*.ssa"), _T("-*.ass"), _T("-*.dummyidx"), _T("-*.usf"),
		_T("-*.xss"), _T("-*.txt"), _T("-*.ssf"), _T("-*.rt"), _T("-*.sup")
	},
};

#define WEBSUBEXT _T(".wse")

static int SubFileCompare(const void* elem1, const void* elem2)
{
	return(((SubFile*)elem1)->fn.CompareNoCase(((SubFile*)elem2)->fn));
}

void GetSubFileNames(CString fn, CAtlArray<CString>& paths, CAtlArray<SubFile>& ret)
{
	ret.RemoveAll();

	int extlistnum = countof(ext);
	int extsubnum = countof(ext[0]);

	fn.Replace('\\', '/');

	bool fWeb = false;
	{
		//		int i = fn.Find(_T("://"));
		int i = fn.Find(_T("http://"));
		if (i > 0) {
			fn = _T("http") + fn.Mid(i);
			fWeb = true;
		}
	}

	int	l = fn.GetLength(), l2 = l;
	l2 = fn.ReverseFind('.');
	l = fn.ReverseFind('/') + 1;
	if (l2 < l) {
		l2 = l;
	}

	CString orgpath = fn.Left(l);
	CString title = fn.Mid(l, l2-l);
	CString filename = title + _T(".nooneexpectsthespanishinquisition");

	if (!fWeb) {
		// struct _tfinddata_t file, file2;
		// long hFile, hFile2 = 0;

		WIN32_FIND_DATA wfd, wfd2;
		HANDLE hFile, hFile2;

		for (size_t k = 0; k < paths.GetCount(); k++) {
			CString path = paths[k];
			path.Replace('\\', '/');

			l = path.GetLength();
			if (l > 0 && path[l-1] != '/') {
				path += '/';
			}

			if (path.Find(':') == -1 && path.Find(_T("\\\\")) != 0) {
				path = orgpath + path;
			}

			path.Replace(_T("/./"), _T("/"));
			path.Replace('/', '\\');

			// CAtlList<CString> sl;

			bool fEmpty = true;

			if ((hFile = FindFirstFile(path + title + _T("*"), &wfd)) != INVALID_HANDLE_VALUE) {
				do {
					if (filename.CompareNoCase(wfd.cFileName) != 0) {
						fEmpty = false;
						// sl.AddTail(path + file.name);
					}
				} while (FindNextFile(hFile, &wfd));

				FindClose(hFile);
			}

			// TODO: use 'sl' in the next step to find files (already a nice speedup as it is now...)
			if (fEmpty) {
				continue;
			}

			for (ptrdiff_t j = 0; j < extlistnum; j++) {
				for (ptrdiff_t i = 0; i < extsubnum; i++) {
					if ((hFile = FindFirstFile(path + title + ext[j][i], &wfd)) != INVALID_HANDLE_VALUE) {
						do {
							CString fn = path + wfd.cFileName;

							hFile2 = INVALID_HANDLE_VALUE;

							if (j == 0 || (hFile2 = FindFirstFile(fn.Left(fn.ReverseFind('.')) + _T(".avi"), &wfd2)) == INVALID_HANDLE_VALUE) {
								SubFile f;
								f.fn = fn;
								ret.Add(f);
							}

							if (hFile2 != INVALID_HANDLE_VALUE) {
								FindClose(hFile2);
							}
						} while (FindNextFile(hFile, &wfd));

						FindClose(hFile);
					}
				}
			}
		}
	} else if (l > 7) {
		CWebTextFile wtf; // :)
		if (wtf.Open(orgpath + title + WEBSUBEXT)) {
			CString fn;
			while (wtf.ReadString(fn) && fn.Find(_T("://")) >= 0) {
				SubFile f;
				f.fn = fn;
				ret.Add(f);
			}
		}
	}

	// sort files, this way the user can define the order (movie.00.English.srt, movie.01.Hungarian.srt, etc)

	qsort(ret.GetData(), ret.GetCount(), sizeof(SubFile), SubFileCompare);
}
