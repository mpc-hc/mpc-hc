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
		return x >= 0 && x < m_chunkdist.GetCount() ? m_chunkdist[x] : 0;
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

	int m_nChunks;
	REFERENCE_TIME m_rtDur;

public:
	CAviReportWnd();
	bool DoModal(CAviFile* pAF, bool fHideChecked, bool fShowWarningText);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnClose();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};


