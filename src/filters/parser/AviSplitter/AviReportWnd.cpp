#include "stdafx.h"
#include "AviReportWnd.h"

#define IDC_DONOTSHOWAGAINCHECK 1000
#define TITLE _T("AVI Chunk Viewer")
#define GRAPHFOOTER 10

CAviReportWnd::CAviReportWnd()
{
	m_font.CreateFont(12, 0, 0, 0, 400, 0, 0, 0, 1, 0, 0, 0, 0, _T("MS Shell Dlg"));
}

bool CAviReportWnd::DoModal(CAviFile* pAF, bool fHideChecked, bool fShowWarningText)
{
	m_nChunks = 0;
	m_rtDur = 0;

	for(DWORD i = 0; i < pAF->m_avih.dwStreams; ++i) {
		int cnt = pAF->m_strms[i]->cs2.GetCount();
		if(cnt <= 0) {
			continue;
		}
		CAviFile::strm_t::chunk2& c2 = pAF->m_strms[i]->cs2[cnt-1];
		m_nChunks = max(m_nChunks, c2.n);
		m_rtDur = max(m_rtDur, (REFERENCE_TIME)c2.t<<13);
	}

	CRect r, r2;
	GetDesktopWindow()->GetWindowRect(r);
	r.DeflateRect(r.Width()/4, r.Height()/4);

	LPCTSTR wndclass = AfxRegisterWndClass(
						   CS_VREDRAW|CS_HREDRAW|CS_DBLCLKS,
						   AfxGetApp()->LoadStandardCursor(IDC_ARROW),
						   (HBRUSH)(COLOR_BTNFACE + 1), 0);

	CreateEx(0, wndclass, TITLE, WS_POPUPWINDOW|WS_CAPTION|WS_CLIPCHILDREN, r, NULL, 0);

	CRect cr;
	GetClientRect(cr);
	cr.DeflateRect(10, 10);

	SetFont(&m_font, FALSE);

	CDC* pDC = GetDC();
	CFont* pOldFont = pDC->SelectObject(&m_font);

	//

	CString str(
		_T("This AVI file was not prepared for sequential reading, the alternative ")
		_T("'Avi Splitter' will now let the default one handle it. ")
		_T("The complete reinterleaving of this file is strongly recommended before ")
		_T("burning it onto a slow media like cd-rom."));

	r = cr;

	pDC->DrawText(str, r, DT_WORDBREAK|DT_CALCRECT);
	r.right = cr.right;

	m_message.Create(str, WS_CHILD|WS_VISIBLE, r, this);
	m_message.SetFont(&m_font, FALSE);

	//

	r.SetRect(cr.left, r.bottom + 10, cr.right, cr.bottom);

	str = _T("Do not show this dialog again (hold Shift to re-enable it)");

	pDC->DrawText(str, r, DT_WORDBREAK|DT_CALCRECT);
	r.right = cr.right;

	m_checkbox.Create(str, WS_CHILD|WS_VISIBLE|BS_CHECKBOX|BS_AUTOCHECKBOX, r, this, IDC_DONOTSHOWAGAINCHECK);
	m_checkbox.SetFont(&m_font, FALSE);

	CheckDlgButton(IDC_DONOTSHOWAGAINCHECK, fHideChecked?BST_CHECKED:BST_UNCHECKED);

	//

	if(!fShowWarningText) {
		m_message.ShowWindow(SW_HIDE);
		m_checkbox.ShowWindow(SW_HIDE);
		r = cr;
	} else {
		r.SetRect(cr.left, r.bottom + 10, cr.right, cr.bottom);
	}

	m_graph.Create(pAF, r, this);

	//

	pDC->SelectObject(pOldFont);
	ReleaseDC(pDC);

	SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
	SetForegroundWindow();
	ShowWindow(SW_SHOWNORMAL);

	return !!RunModalLoop();
}

IMPLEMENT_DYNCREATE(CAviReportWnd, CWnd)

BEGIN_MESSAGE_MAP(CAviReportWnd, CWnd)
	ON_WM_CLOSE()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

void CAviReportWnd::OnClose()
{
	EndModalLoop(IsDlgButtonChecked(IDC_DONOTSHOWAGAINCHECK));
	__super::OnClose();
}

void CAviReportWnd::OnMouseMove(UINT nFlags, CPoint p)
{
	MapWindowPoints(&m_graph, &p, 1);

	CRect r, r2;
	m_graph.GetClientRect(r);
	r2 = r;
	r.bottom -= GRAPHFOOTER;
	r2.top = r.bottom;

	if(r.PtInRect(p)) {
		SetCapture();

		int x = p.x - r.left;
		int y = r.bottom - p.y;

		REFERENCE_TIME rt = m_rtDur * x / r.Width();
		int chunk = (int)(1.0 * m_nChunks * y / r.Height());

		rt /= 10000;
		int ms = (int)(rt%1000);
		rt /= 1000;
		int s = (int)(rt%60);
		rt /= 60;
		int m = (int)(rt%60);
		rt /= 60;
		int h = (int)(rt);

		CString str;
		str.Format(_T("%s (%d - %d:%02d:%02d.%03d)"), TITLE, chunk, h, m, s, ms);
		SetWindowText(str);
	} else if(r2.PtInRect(p)) {
		SetCapture();

		int dist = m_graph.GetChunkDist(p.x - r2.left);
		CString str;
		str.Format(_T("%s (chunk distance: %d"), TITLE, dist);
		if(dist >= 1000) {
			str += _T(" - over the limit!");
		}
		str += ")";
		SetWindowText(str);
	} else if(GetCapture() == this) {
		SetWindowText(TITLE);

		ReleaseCapture();
	}

	__super::OnMouseMove(nFlags, p);
}

//////////////

CAviPlotterWnd::CAviPlotterWnd()
{
}

bool CAviPlotterWnd::Create(CAviFile* pAF, CRect r, CWnd* pParentWnd)
{
	if(!CreateEx(WS_EX_CLIENTEDGE, _T("STATIC"), _T(""), WS_CHILD|WS_VISIBLE, r, pParentWnd, 0)) {
		return(false);
	}

	GetClientRect(r);
	int w = r.Width();
	int h = r.Height() - GRAPHFOOTER;

	CDC* pDC = GetDC();
	m_dc.CreateCompatibleDC(pDC);
	m_bm.CreateCompatibleBitmap(pDC, r.Width(), r.Height());
	ReleaseDC(pDC);

	CBitmap* pOldBitmap = m_dc.SelectObject(&m_bm);

	m_dc.FillSolidRect(r, 0);

	{
		CPen pen(PS_DOT, 1, 0x008000);
		CPen* pOldPen = m_dc.SelectObject(&pen);
		for(int y = 0, dy = max(h/10,1); y < h; y += dy) {
			if(y == 0) {
				continue;
			}
			m_dc.MoveTo(0, y);
			m_dc.LineTo(w, y);
		}
		for(int x = 0, dx = max(w/10,1); x < w; x += dx) {
			if(x == 0) {
				continue;
			}
			m_dc.MoveTo(x, 0);
			m_dc.LineTo(x, w);
		}
		m_dc.SelectObject(pOldPen);
	}

	{
		CPen pen(PS_SOLID, 1, 0x00ff00);
		CPen* pOldPen = m_dc.SelectObject(&pen);
		m_dc.MoveTo(15, 30);
		m_dc.LineTo(15, 2);
		m_dc.LineTo(19, 10);
		m_dc.LineTo(11, 10);
		m_dc.LineTo(15, 2);
		m_dc.MoveTo(w-30-10, h-15);
		m_dc.LineTo(w-2-10, h-15);
		m_dc.LineTo(w-10-10, h-19);
		m_dc.LineTo(w-10-10, h-11);
		m_dc.LineTo(w-2-10, h-15);
		m_dc.SelectObject(pOldPen);

		m_dc.SetTextColor(0x008000);
		m_dc.TextOut(20, 10, _T("Chunk"));

		CSize size = m_dc.GetTextExtent(_T("Time"));
		m_dc.TextOut(w - size.cx - 10, h - size.cy - 20, _T("Time"));
	}

	COLORREF clr[] = {0x0000ff,0xff0000,0x40ffff,0xff40ff,0xffff40,0xffffff};

	for(DWORD i = 0, y = 40, dy = m_dc.GetTextExtent(_T("Stream N")).cy + 1; i < pAF->m_avih.dwStreams; ++i, y += dy) {
		m_dc.SetTextColor(clr[i % pAF->m_avih.dwStreams]);
		m_dc.SetBkMode(TRANSPARENT);
		CString str;
		str.Format(_T("Stream %d"), i);
		m_dc.TextOut(10, y, str);
	}

	DWORD nmax = 0, tmax = 0;

	for(DWORD i = 0; i < pAF->m_avih.dwStreams; ++i) {
		int cnt = pAF->m_strms[i]->cs2.GetCount();
		if(cnt <= 0) {
			continue;
		}
		CAviFile::strm_t::chunk2& c2 = pAF->m_strms[i]->cs2[cnt-1];
		nmax = max(nmax, c2.n);
		tmax = max(tmax, c2.t);
	}

	if(nmax > 0 && tmax > 0) {
		CAtlArray<CPen> pen;
		pen.SetCount(pAF->m_avih.dwStreams);
		for(size_t i = 0; i < pen.GetCount(); i++) {
			pen[i].CreatePen(PS_SOLID, 2, clr[i]);
		}

		CAtlArray<CPoint> pp;
		pp.SetCount(pAF->m_avih.dwStreams);
		for(size_t i = 0; i < pen.GetCount(); i++) {
			pp[i].SetPoint(-1, -1);
		}

		m_chunkdist.SetCount(w);
		memset(m_chunkdist.GetData(), 0, sizeof(int)*w);

		DWORD* curchunks = DNew DWORD[pAF->m_avih.dwStreams];
		memset(curchunks, 0, sizeof(DWORD)*pAF->m_avih.dwStreams);

		CAviFile::strm_t::chunk2 cs2last = {(DWORD)-1, 0};

		while(1) {
			CAviFile::strm_t::chunk2 cs2min = {LONG_MAX, LONG_MAX};

			DWORD n = (DWORD)-1;
			for(DWORD i = 0; i < pAF->m_avih.dwStreams; ++i) {
				DWORD curchunk = curchunks[i];
				if(curchunk >= pAF->m_strms[i]->cs2.GetCount()) {
					continue;
				}
				CAviFile::strm_t::chunk2& cs2 = pAF->m_strms[i]->cs2[curchunk];
				if(cs2.t < cs2min.t) {
					cs2min = cs2;
					n = i;
				}
			}
			if(n == -1) {
				break;
			}


			CPoint p;
			p.x = (int)(1.0 * w * cs2min.t / tmax);
			p.y = (int)(h - 1.0 * h * cs2min.n / nmax);
			if(pp[n] != p) {
				CPen* pOldPen = m_dc.SelectObject(&pen[n]);
				if(pp[n] == CPoint(-1, -1)) {
					m_dc.MoveTo(p);
				} else {
					m_dc.MoveTo(pp[n]);
					m_dc.LineTo(p);
				}
				m_dc.SelectObject(pOldPen);
				pp[n] = p;
			}

			int dist = abs((int)cs2min.n - (int)cs2last.n);

			if(cs2last.t >= 0 /*&& dist >= 1000*/) {
				if(p.x >= 0 && p.x < w) {
					m_chunkdist[p.x] = max(m_chunkdist[p.x], dist);
				}
			}

			curchunks[n]++;
			cs2last = cs2min;
		}

		CPen red(PS_SOLID, 1, 0x0000ff);
		CPen green(PS_SOLID, 1, 0x00ff00);

		for(int x = 0; x < w; x++) {
			CPen* pOldPen = m_dc.SelectObject(m_chunkdist[x] >= 1000 ? &red : &green);
			m_dc.MoveTo(x, h);
			m_dc.LineTo(x, h + GRAPHFOOTER);
			m_dc.SelectObject(pOldPen);
		}

		delete [] curchunks;
	}

	m_dc.SelectObject(pOldBitmap);

	return(true);
}

IMPLEMENT_DYNCREATE(CAviPlotterWnd, CWnd)

BEGIN_MESSAGE_MAP(CAviPlotterWnd, CWnd)
	ON_WM_PAINT()
END_MESSAGE_MAP()

void CAviPlotterWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect r;
	GetClientRect(r);

	CBitmap* pOld = m_dc.SelectObject(&m_bm);
	dc.BitBlt(0, 0, r.Width(), r.Height(), &m_dc, 0, 0, SRCCOPY);
	m_dc.SelectObject(pOld);
}
