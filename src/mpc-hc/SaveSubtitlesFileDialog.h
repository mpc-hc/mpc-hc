/*
 * (C) 2012-2014, 2017 see Authors.txt
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
    std::vector<Subtitle::SubType> m_types;

    bool m_bDisableEncoding;
    bool m_bDisableExternalStyleCheckBox;

    int m_delay;
    CSpinButtonCtrl m_delayCtrl;

    BOOL m_bSaveExternalStyleFile;

    void InitCustomization();

    DECLARE_DYNAMIC(CSaveSubtitlesFileDialog)

public:
    CSaveSubtitlesFileDialog(
        CTextFile::enc e, int delay, bool bSaveExternalStyleFile,
        LPCTSTR lpszDefExt = nullptr, LPCTSTR lpszFileName = nullptr,
        LPCTSTR lpszFilter = nullptr, std::vector<Subtitle::SubType> types = {},
        CWnd* pParentWnd = nullptr);
    CSaveSubtitlesFileDialog(
        int delay,
        LPCTSTR lpszDefExt = nullptr, LPCTSTR lpszFileName = nullptr,
        LPCTSTR lpszFilter = nullptr, CWnd* pParentWnd = nullptr);
    virtual ~CSaveSubtitlesFileDialog();

    int GetDelay() const { return m_delay; }
    bool GetSaveExternalStyleFile() const { return !!m_bSaveExternalStyleFile; }

protected:
    DECLARE_MESSAGE_MAP()
    virtual BOOL OnInitDialog();
    virtual BOOL OnFileNameOK();
    virtual void OnTypeChange();
};
