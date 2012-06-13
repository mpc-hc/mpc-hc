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

#pragma once


// CSaveThumbnailsDialog

class CSaveThumbnailsDialog : public CFileDialog
{
    DECLARE_DYNAMIC(CSaveThumbnailsDialog)

public:
    CSaveThumbnailsDialog(
        int rows, int cols, int width,
        LPCTSTR lpszDefExt = NULL, LPCTSTR lpszFileName = NULL,
        LPCTSTR lpszFilter = NULL, CWnd* pParentWnd = NULL);
    virtual ~CSaveThumbnailsDialog();

protected:
    DECLARE_MESSAGE_MAP()
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    virtual BOOL OnFileNameOK();

public:
    int m_rows, m_cols, m_width;
    CSpinButtonCtrl m_rowsctrl;
    CSpinButtonCtrl m_colsctrl;
    CSpinButtonCtrl m_widthctrl;
};
