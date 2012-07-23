/*
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

#include "AviFile.h"

class CAviPlotterWnd : public CStatic
{
    DECLARE_DYNCREATE(CAviPlotterWnd)

private:
    CDC m_dc;
    CBitmap m_bm;

    CAtlArray<int> m_chunkdist;

public:
    CAviPlotterWnd();
    bool Create(CAviFile* pAF, CRect r, CWnd* pParentWnd);

    int GetChunkDist(int x) {
        return (x >= 0 && (size_t)x < m_chunkdist.GetCount()) ? m_chunkdist[x] : 0;
    }

    DECLARE_MESSAGE_MAP()
    afx_msg void OnPaint();
};

class CAviReportWnd : public CWnd
{
    DECLARE_DYNCREATE(CAviReportWnd)

protected:
    CFont m_font;
    CStatic m_message;
    CButton m_checkbox;
    CAviPlotterWnd m_graph;

    unsigned int m_nChunks;
    REFERENCE_TIME m_rtDur;

public:
    CAviReportWnd();
    bool DoModal(CAviFile* pAF, bool fHideChecked, bool fShowWarningText);

    DECLARE_MESSAGE_MAP()
    afx_msg void OnClose();
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};
