/*
 * (C) 2012-2013 see Authors.txt
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
#include "MediaPositionList.h"
#include "SettingsDefines.h"

// Helper functions

static void DeserializeHex(LPCTSTR strVal, BYTE* pBuffer, int nBufSize)
{
    long lRes;

    for (int i = 0; i < nBufSize; i++) {
        _stscanf_s(strVal + (i * 2), _T("%02lx"), &lRes);
        pBuffer[i] = (BYTE)lRes;
    }
}

static CString SerializeHex(const BYTE* pBuffer, int nBufSize)
{
    CString strTemp;
    CString strResult;

    for (int i = 0; i < nBufSize; i++) {
        strTemp.Format(_T("%02x"), pBuffer[i]);
        strResult += strTemp;
    }

    return strResult;
}

// CFilePositionList

CFilePositionList::CFilePositionList(CWinApp* pApp, LPCTSTR lpszSection, int nMaxSize)
    : CMediaPositionList(pApp, lpszSection, nMaxSize)
{
}

void CFilePositionList::Load()
{
    bool hasNextEntry = true;
    CString strFilePos;
    CString strValue;
    FILE_POSITION filePosition;

    for (int i = 0; i < m_nMaxSize && hasNextEntry; i++) {
        strFilePos.Format(_T("File Name %d"), i);
        filePosition.strFile = m_pApp->GetProfileString(m_lpszSection, strFilePos);

        strFilePos.Format(_T("File Position %d"), i);
        strValue = m_pApp->GetProfileString(m_lpszSection, strFilePos);
        filePosition.llPosition = _tstoi64(strValue);

        if (!filePosition.strFile.IsEmpty()) {
            AddTail(filePosition);
        } else {
            hasNextEntry = false;
        }
    }
}

void CFilePositionList::Save()
{
    int i;
    POSITION pos;
    CString strPos, strValue;

    for (i = 0, pos = GetHeadPosition(); i < m_nMaxSize && pos; GetNext(pos), i++) {
        FILE_POSITION& filePosition = GetAt(pos);
        strPos.Format(_T("File Name %d"), i);
        m_pApp->WriteProfileString(m_lpszSection, strPos, filePosition.strFile);
        strPos.Format(_T("File Position %d"), i);
        strValue.Format(_T("%I64d"), filePosition.llPosition);
        m_pApp->WriteProfileString(m_lpszSection, strPos, strValue);
    }
}

void CFilePositionList::SaveLatestEntry()
{
    CString  strValue;
    FILE_POSITION& filePosition = GetHead();

    m_pApp->WriteProfileString(IDS_R_SETTINGS, _T("File Name 0"), filePosition.strFile);
    strValue.Format(_T("%I64d"), filePosition.llPosition);
    m_pApp->WriteProfileString(IDS_R_SETTINGS, _T("File Position 0"), strValue);
}

void CFilePositionList::Empty()
{
    CString strFilePos;

    for (int i = 0, len = (int)GetCount(); i < len; i++) {
        strFilePos.Format(_T("File Name %d"), i);
        m_pApp->WriteProfileString(m_lpszSection, strFilePos, nullptr);
        strFilePos.Format(_T("File Position %d"), i);
        m_pApp->WriteProfileString(m_lpszSection, strFilePos, nullptr);
    }

    RemoveAll();
}

bool CFilePositionList::AddEntry(LPCTSTR lpszFileName)
{
    // Look for the file position
    POSITION pos = GetHeadPosition();
    while (pos) {
        FILE_POSITION& filePosition = GetAt(pos);

        // If we find it, we move it at the top of the list
        if (filePosition.strFile == lpszFileName) {
            if (pos != GetHeadPosition()) {
                AddHead(filePosition);
                RemoveAt(pos);

                // Save asynchronously the list
                SaveAsync();
            }

            return false;
        }

        GetNext(pos);
    }

    // Add the new position
    FILE_POSITION filePosition = { lpszFileName, 0 };
    AddHead(filePosition);

    // Ensure the list doesn't grow indefinitely
    if (GetCount() > m_nMaxSize) {
        RemoveTail();
    }

    // Save asynchronously the list
    SaveAsync();

    return true;
}

bool CFilePositionList::RemoveEntry(LPCTSTR lpszFileName)
{
    // Look for the file position
    POSITION pos = GetHeadPosition();
    while (pos) {
        FILE_POSITION& filePosition = GetAt(pos);

        // If we find it, we can remove it
        if (filePosition.strFile == lpszFileName) {
            RemoveAt(pos);

            // Save asynchronously the list
            SaveAsync();

            return true;
        }

        GetNext(pos);
    }

    return false;
}

// CDVDPositionList

CDVDPositionList::CDVDPositionList(CWinApp* pApp, LPCTSTR lpszSection, int nMaxSize)
    : CMediaPositionList(pApp, lpszSection, nMaxSize)
{
}

void CDVDPositionList::Load()
{
    bool hasNextEntry = true;

    for (int i = 0; i < m_nMaxSize && hasNextEntry; i++) {
        CString strDVDPos;
        CString strValue;

        strDVDPos.Format(_T("DVD Position %d"), i);
        strValue = m_pApp->GetProfileString(m_lpszSection, strDVDPos, _T(""));
        if (strValue.GetLength() / 2 == sizeof(DVD_POSITION)) {
            DVD_POSITION dvdPosition;
            DeserializeHex(strValue, (BYTE*)&dvdPosition, sizeof(DVD_POSITION));
            AddTail(dvdPosition);
        } else {
            hasNextEntry = false;
        }
    }
}

void CDVDPositionList::Save()
{
    int i;
    POSITION pos;
    CString strPos, strValue;

    for (i = 0, pos = GetHeadPosition(); i < m_nMaxSize && pos; GetNext(pos), i++) {
        strPos.Format(_T("DVD Position %d"), i);
        strValue = SerializeHex((BYTE*)&GetAt(pos), sizeof(DVD_POSITION));
        m_pApp->WriteProfileString(m_lpszSection, strPos, strValue);
    }
}

void CDVDPositionList::SaveLatestEntry()
{
    CString  strValue;

    strValue = SerializeHex((BYTE*)&GetHead(), sizeof(DVD_POSITION));
    m_pApp->WriteProfileString(m_lpszSection, _T("DVD Position 0"), strValue);
}

void CDVDPositionList::Empty()
{
    CString strDVDPos;

    for (int i = 0, len = (int)GetCount(); i < len; i++) {
        strDVDPos.Format(_T("DVD Position %d"), i);
        m_pApp->WriteProfileString(m_lpszSection, strDVDPos, nullptr);
    }

    RemoveAll();
}

bool CDVDPositionList::AddEntry(ULONGLONG llDVDGuid)
{
    // Look for the file position
    POSITION pos = GetHeadPosition();
    while (pos) {
        DVD_POSITION& dvdPosition = GetAt(pos);

        // If we find it, we move it at the top of the list
        if (dvdPosition.llDVDGuid == llDVDGuid) {
            if (pos != GetHeadPosition()) {
                AddHead(dvdPosition);
                RemoveAt(pos);

                // Save asynchronously the list
                SaveAsync();
            }

            return false;
        }

        GetNext(pos);
    }

    // Add the new position
    DVD_POSITION dvdPosition = { llDVDGuid, 0, 0 };
    AddHead(dvdPosition);

    // Ensure the list doesn't grow indefinitely
    if (GetCount() > m_nMaxSize) {
        RemoveTail();
    }

    // Save asynchronously the list
    SaveAsync();

    return true;
}
