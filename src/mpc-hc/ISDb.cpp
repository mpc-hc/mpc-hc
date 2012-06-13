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
#include <atlpath.h>
#include "ISDb.h"
#include "mplayerc.h"


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
    for (UINT64 tmp = 0, i = 0; i < 65536 / sizeof(tmp) && f.Read(&tmp, sizeof(tmp)); fh.mpc_filehash += tmp, i++) {
        ;
    }
    f.Seek(max(0, (INT64)fh.size - 65536), CFile::begin);
    for (UINT64 tmp = 0, i = 0; i < 65536 / sizeof(tmp) && f.Read(&tmp, sizeof(tmp)); fh.mpc_filehash += tmp, i++) {
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
                   i, UrlEncode(CStringA(fh.name)),
                   i, fh.size,
                   i, fh.mpc_filehash);

        args.AddTail(str);
    }

    return Implode(args, '&');
}

bool OpenUrl(CInternetSession& is, CString url, CStringA& str)
{
    str.Empty();

    try {
        CAutoPtr<CStdioFile> f(is.OpenURL(url, 1, INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_EXISTING_CONNECT));

        char buff[1024];
        for (int len; (len = f->Read(buff, sizeof(buff))) > 0; str += CStringA(buff, len)) {
            ;
        }

        f->Close(); // must close it because the destructor doesn't seem to do it and we will get an exception when "is" is destroying
    } catch (CInternetException* ie) {
        ie->Delete();
        return false;
    }

    return true;
}
