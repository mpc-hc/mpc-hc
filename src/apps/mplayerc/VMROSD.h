#pragma once

#include <atlbase.h>
#include <D3d9.h>
#include <Vmr9.h>
#include <evr9.h>

typedef enum
{
	OSD_TRANSPARENT,
	OSD_BACKGROUND,
	OSD_BORDER,
	OSD_TEXT,
	OSD_BAR,
	OSD_CURSOR,
	OSD_LAST
} OSD_COLORS;

typedef enum
{
	OSD_NOMESSAGE,
	OSD_TOPLEFT,
	OSD_TOPRIGHT,
} OSD_MESSAGEPOS;


class CVMROSD
{
public:
	CVMROSD(void);
	~CVMROSD(void);

	void		Start (CWnd* pWnd, CComPtr<IVMRMixerBitmap9> pVMB);
	void		Start (CWnd* pWnd, CComPtr<IMFVideoMixerBitmap> pVMB);
	void		Stop();

	void		DisplayMessage (OSD_MESSAGEPOS nPos, LPCTSTR strMsg, int nDuration = 5000);
	void		ClearMessage();

	__int64		GetPos();
	void		SetPos(__int64 pos);
	void		SetRange(__int64 start,  __int64 stop);
	void		GetRange(__int64& start, __int64& stop);

	void		OnSize(UINT nType, int cx, int cy);
	bool		OnMouseMove(UINT nFlags, CPoint point);
	bool		OnLButtonDown(UINT nFlags, CPoint point);
	bool		OnLButtonUp(UINT nFlags, CPoint point);

private :
	CComPtr<IVMRMixerBitmap9>		m_pVMB;
	CComPtr<IMFVideoMixerBitmap>	m_pMFVMB;
	CWnd*							m_pWnd;

	CBitmap							m_Bitmap;
	CDC								m_MemDC;
	VMR9AlphaBitmap					m_VMR9AlphaBitmap;
	MFVideoAlphaBitmap				m_MFVideoAlphaBitmap;
	BITMAP							m_BitmapInfo;

	CFont							m_MainFont;
	CPen							m_penBorder;
	CPen							m_penCursor;
	CBrush							m_brushBack;
	CBrush							m_brushBar;

	CRect							m_rectWnd;
	COLORREF						m_Color[OSD_LAST];

	// Curseur de calage
	CRect							m_rectSeekBar;
	CRect							m_rectCursor;
	CRect							m_rectBar;
	bool							m_bCursorMoving;
	bool							m_bSeekBarVisible;
	__int64							m_llSeekMin;
	__int64							m_llSeekMax;
	__int64							m_llSeekPos;

	// Messages
	CString							m_strMessage;
	OSD_MESSAGEPOS					m_nMessagePos;

	void			UpdateVMRBitmap();
	void			UpdateMFBitmap();
	void			CalcRect();
	void			UpdateSeekBarPos(CPoint point);
	void			DrawSlider(CRect* rect, __int64 llMin, __int64 llMax, __int64 llPos);
	void			DrawRect(CRect* rect, CBrush* pBrush = NULL, CPen* pPen = NULL);
	void			Invalidate();
	void			DrawMessage();

	static void CALLBACK TimerFunc(HWND hWnd, UINT nMsg, UINT nIDEvent, DWORD dwTime);

};
