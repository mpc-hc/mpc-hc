/*
 * (C) 2012-2015 see Authors.txt
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

#pragma once

#include <afxwin.h>

template<typename T>
class CMediaPositionList : protected CList<T>
{
protected:
    CWinApp* m_pApp;
    LPCTSTR m_lpszSection;
    int m_nMaxSize;

    static UINT SaveStartThread(LPVOID pParam) {
        CMediaPositionList* pMPL = (CMediaPositionList*)pParam;

        pMPL->Save();

        return 0;
    };

public:
    CMediaPositionList(CWinApp* pApp, LPCTSTR lpszSection, int nMaxSize)
        : m_pApp(pApp)
        , m_lpszSection(lpszSection)
        , m_nMaxSize(nMaxSize)
    {};

    int GetMaxSize() const { return m_nMaxSize; };
    void SetMaxSize(int nMaxSize) {
        ENSURE_ARG(nMaxSize >= 0);

        if (m_nMaxSize != nMaxSize) {
            m_nMaxSize = nMaxSize;
            while (GetCount() > m_nMaxSize) {
                RemoveTail();
            }
        }
    };

    virtual void Load() = 0;
    virtual void Save() = 0;
    void SaveAsync() {
        AfxBeginThread(SaveStartThread, this);
    };
    virtual void SaveLatestEntry() = 0;
    virtual void Empty() = 0;

    T* GetLatestEntry() {
        return !IsEmpty() ? &GetHead() : nullptr;
    };
};

struct FILE_POSITION {
    CString     strFile;
    LONGLONG    llPosition;
};

class CFilePositionList : public CMediaPositionList<FILE_POSITION>
{
public:
    CFilePositionList(CWinApp* pApp, LPCTSTR lpszSection, int nMaxSize);

    virtual void Load();
    virtual void Save();
    virtual void SaveLatestEntry();
    virtual void Empty();

    bool AddEntry(LPCTSTR lpszFileName);
    bool RemoveEntry(LPCTSTR lpszFileName);
};

struct DVD_POSITION {
    ULONGLONG           llDVDGuid;
    ULONG               lTitle;
    DVD_HMSF_TIMECODE   timecode;
};

class CDVDPositionList : public CMediaPositionList<DVD_POSITION>
{
public:
    CDVDPositionList(CWinApp* pApp, LPCTSTR lpszSection, int nMaxSize);

    virtual void Load();
    virtual void Save();
    virtual void SaveLatestEntry();
    virtual void Empty();

    bool AddEntry(ULONGLONG llDVDGuid);
};
