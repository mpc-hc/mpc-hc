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
#include "OggFile.h"

COggFile::COggFile(IAsyncReader* pAsyncReader, HRESULT& hr)
    : CBaseSplitterFile(pAsyncReader, hr, DEFAULT_CACHE_LENGTH, false)
{
    if (FAILED(hr)) {
        return;
    }
    hr = Init();
}

HRESULT COggFile::Init()
{
    Seek(0);
    if (!Sync()) {
        return E_FAIL;
    }

    return S_OK;
}

bool COggFile::Sync(HANDLE hBreak)
{
    __int64 start = GetPos();

    DWORD dw;
    for (__int64 i = 0, j = hBreak ? GetLength() - start : 65536;
            i < j && S_OK == ByteRead((BYTE*)&dw, sizeof(dw))
            && ((i & 0xffff) || !hBreak || WaitForSingleObject(hBreak, 0) != WAIT_OBJECT_0);
            i++, Seek(start + i)) {
        if (dw == 'SggO') {
            Seek(start + i);
            return true;
        }
    }

    Seek(start);

    return false;
}

bool COggFile::Read(OggPageHeader& hdr, HANDLE hBreak)
{
    return Sync(hBreak) && S_OK == ByteRead((BYTE*)&hdr, sizeof(hdr)) && hdr.capture_pattern == 'SggO';
}

bool COggFile::Read(OggPage& page, bool fFull, HANDLE hBreak)
{
    memset(&page.m_hdr, 0, sizeof(page.m_hdr));
    page.m_lens.RemoveAll();
    page.SetCount(0);

    if (!Read(page.m_hdr, hBreak)) {
        return false;
    }

    int pagelen = 0, packetlen = 0;
    for (BYTE i = 0; i < page.m_hdr.number_page_segments; i++) {
        BYTE b;
        if (S_OK != ByteRead(&b, 1)) {
            return false;
        }
        packetlen += b;
        if (1/*b < 0xff*/) {
            page.m_lens.AddTail(packetlen);
            pagelen += packetlen;
            packetlen = 0;
        }
    }

    if (fFull) {
        page.SetCount(pagelen);
        if (S_OK != ByteRead(page.GetData(), page.GetCount())) {
            return false;
        }
    } else {
        Seek(GetPos() + pagelen);
        page.m_lens.RemoveAll();
    }

    return true;
}
