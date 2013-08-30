/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013 see Authors.txt
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
#include <atlpath.h>
#include "ISDb.h"
#include "mplayerc.h"

#define PROBE_SIZE (64 * 1024)

namespace ISDb
{
    bool mpc_filehash(IFileSourceFilter* pFSF, filehash& fh)
    {
        CComQIPtr<IAsyncReader> pAR;
        if (CComQIPtr<IBaseFilter> pBF = pFSF) {
            BeginEnumPins(pBF, pEP, pPin)
            if (pAR = pPin) {
                break;
            }
            EndEnumPins;
        }

        if (!pAR || !pFSF) {
            return false;
        }

        LPOLESTR name;
        if (FAILED(pFSF->GetCurFile(&name, nullptr))) {
            return false;
        }
        fh.name = name;
        CoTaskMemFree(name);

        LONGLONG size, available;
        if (pAR->Length(&size, &available) != S_OK) { // Don't accept estimates
            return false;
        }
        fh.size = size;

        fh.mpc_filehash = fh.size;
        LONGLONG position = 0;
        for (UINT64 tmp = 0, i = 0; i < PROBE_SIZE / sizeof(tmp) && SUCCEEDED(pAR->SyncRead(position, sizeof(tmp), (BYTE*)&tmp)); fh.mpc_filehash += tmp, position += sizeof(tmp), i++) {
            ;
        }
        position = max(0, (INT64)fh.size - PROBE_SIZE);
        for (UINT64 tmp = 0, i = 0; i < PROBE_SIZE / sizeof(tmp) && SUCCEEDED(pAR->SyncRead(position, sizeof(tmp), (BYTE*)&tmp)); fh.mpc_filehash += tmp, position += sizeof(tmp), i++) {
            ;
        }

        return true;
    }

    bool mpc_filehash(LPCTSTR fn, filehash& fh)
    {
        CFile f;
        CFileException fe;
        if (!f.Open(fn, CFile::modeRead | CFile::osSequentialScan | CFile::shareDenyNone, &fe)) {
            return false;
        }

        CPath p(fn);
        p.StripPath();
        fh.name = (LPCTSTR)p;

        fh.size = f.GetLength();

        fh.mpc_filehash = fh.size;
        for (UINT64 tmp = 0, i = 0; i < PROBE_SIZE / sizeof(tmp) && f.Read(&tmp, sizeof(tmp)); fh.mpc_filehash += tmp, i++) {
            ;
        }
        f.Seek(max(0, (INT64)fh.size - PROBE_SIZE), CFile::begin);
        for (UINT64 tmp = 0, i = 0; i < PROBE_SIZE / sizeof(tmp) && f.Read(&tmp, sizeof(tmp)); fh.mpc_filehash += tmp, i++) {
            ;
        }

        return true;
    }

    void mpc_filehash(CPlaylist& pl, CList<filehash>& fhs)
    {
        fhs.RemoveAll();

        POSITION pos = pl.GetHeadPosition();
        while (pos) {
            CString fn = pl.GetNext(pos).m_fns.GetHead();
            if (AfxGetAppSettings().m_Formats.FindExt(CPath(fn).GetExtension().MakeLower(), true)) {
                continue;
            }

            filehash fh;
            if (!mpc_filehash(fn, fh)) {
                continue;
            }

            fhs.AddTail(fh);
        }
    }

    CStringA makeargs(CPlaylist& pl)
    {
        CList<filehash> fhs;
        mpc_filehash(pl, fhs);

        CAtlList<CStringA> args;

        POSITION pos = fhs.GetHeadPosition();
        for (int i = 0; pos; i++) {
            filehash& fh = fhs.GetNext(pos);

            CStringA str;
            str.Format("name[%d]=%s&size[%d]=%016I64x&hash[%d]=%016I64x",
                       i, UrlEncode(CStringA(fh.name), true),
                       i, fh.size,
                       i, fh.mpc_filehash);

            args.AddTail(str);
        }

        return Implode(args, '&');
    }
}
