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

#pragma once

#include "SaveTextFileDialog.h"


// CSaveTextFileDialog

class CSaveSubtitlesFileDialog : public CSaveTextFileDialog
{
protected:
    bool m_bDisableEncoding;

    int m_delay;
    CSpinButtonCtrl m_delayCtrl;

    void InitCustomization();

    DECLARE_DYNAMIC(CSaveSubtitlesFileDialog)

public:
    CSaveSubtitlesFileDialog(
        CTextFile::enc e, int delay,
        LPCTSTR lpszDefExt = nullptr, LPCTSTR lpszFileName = nullptr,
        LPCTSTR lpszFilter = nullptr, CWnd* pParentWnd = nullptr);
    CSaveSubtitlesFileDialog(
        int delay,
        LPCTSTR lpszDefExt = nullptr, LPCTSTR lpszFileName = nullptr,
        LPCTSTR lpszFilter = nullptr, CWnd* pParentWnd = nullptr);
    virtual ~CSaveSubtitlesFileDialog();

    int GetDelay() { return m_delay; }

protected:
    DECLARE_MESSAGE_MAP()
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    virtual BOOL OnFileNameOK();
};
