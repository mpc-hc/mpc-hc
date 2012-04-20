/*
 * $Id$
 *
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
#include "resource.h"
#include "MPCVideoDecSettingsWnd.h"
#include "../../../DSUtil/DSUtil.h"

#include "avcodec.h"

#include "../../../apps/mplayerc/InternalFiltersConfig.h"

// ==>>> Resource identifier from "resource.h" present in mplayerc project!
#define ResStr(id) CString(MAKEINTRESOURCE(id))

#define LEFT_SPACING		20
#define VERTICAL_SPACING	25

//
// CMPCVideoDecSettingsWnd
//

int g_AVDiscard[] = {
	AVDISCARD_NONE,
	AVDISCARD_DEFAULT,
	AVDISCARD_NONREF,
	AVDISCARD_BIDIR,
	AVDISCARD_NONKEY,
	AVDISCARD_ALL,
};

int FindDiscardIndex(int nValue)
{
	for (int i=0; i<countof (g_AVDiscard); i++)
		if (g_AVDiscard[i] == nValue) {
			return i;
		}
	return 1;
}

int g_AVErrRecognition[] = {
	AV_EF_CAREFUL,
	AV_EF_COMPLIANT,
	AV_EF_AGGRESSIVE,
};

int FindErrRecognitionIndex(int nValue)
{
	for (int i=0; i<countof (g_AVErrRecognition); i++)
		if (g_AVErrRecognition[i] == nValue) {
			return i;
		}
	return 1;
}

CMPCVideoDecSettingsWnd::CMPCVideoDecSettingsWnd()
{
}

bool CMPCVideoDecSettingsWnd::OnConnect(const CInterfaceList<IUnknown, &IID_IUnknown>& pUnks)
{
	ASSERT(!m_pMDF);

	m_pMDF.Release();

	POSITION pos = pUnks.GetHeadPosition();
	while (pos && !(m_pMDF = pUnks.GetNext(pos))) {
		;
	}

	if (!m_pMDF) {
		return false;
	}

	return true;
}

void CMPCVideoDecSettingsWnd::OnDisconnect()
{
	m_pMDF.Release();
}

bool CMPCVideoDecSettingsWnd::OnActivate()
{
	int		nPosY	= 10;
	GUID*	DxvaGui = NULL;

#if HAS_FFMPEG_VIDEO_DECODERS
	m_grpFFMpeg.Create (ResStr (IDS_VDF_FFSETTINGS), WS_VISIBLE|WS_CHILD | BS_GROUPBOX, CRect (5,  nPosY, 350, nPosY+150), this, (UINT)IDC_STATIC);

	// Decoding threads
	nPosY += VERTICAL_SPACING;
	m_txtThreadNumber.Create (ResStr (IDS_VDF_THREADNUMBER), WS_VISIBLE|WS_CHILD, CRect (LEFT_SPACING,  nPosY, 220, nPosY+15), this, (UINT)IDC_STATIC);
	m_cbThreadNumber.Create  (WS_VISIBLE|WS_CHILD|CBS_DROPDOWNLIST|WS_VSCROLL, CRect (230,  nPosY-4, 290, nPosY+90), this, IDC_PP_THREAD_NUMBER);
	m_cbThreadNumber.AddString(_T("Auto"));
	CString ThreadNumberStr;
	for (int i=0; i<16; i++) {
		ThreadNumberStr.Format(_T("%d"), i+1);
		m_cbThreadNumber.AddString(ThreadNumberStr);
	}

#if INTERNAL_DECODER_H264
	// H264 deblocking mode
	nPosY += VERTICAL_SPACING;
	m_txtDiscardMode.Create (ResStr (IDS_VDF_SKIPDEBLOCK), WS_VISIBLE|WS_CHILD, CRect (LEFT_SPACING,  nPosY, 220, nPosY+15), this, (UINT)IDC_STATIC);
	m_cbDiscardMode.Create  (WS_VISIBLE|WS_CHILD|CBS_DROPDOWNLIST|WS_VSCROLL, CRect (230,  nPosY-4, 345, nPosY+90), this, IDC_PP_DISCARD_MODE);
	m_cbDiscardMode.AddString (ResStr (IDS_VDF_DBLK_NONE));
	m_cbDiscardMode.AddString (ResStr (IDS_VDF_DBLK_DEFAULT));
	m_cbDiscardMode.AddString (ResStr (IDS_VDF_DBLK_NONREF));
	m_cbDiscardMode.AddString (ResStr (IDS_VDF_DBLK_BIDIR));
	m_cbDiscardMode.AddString (ResStr (IDS_VDF_DBLK_NONKFRM));
	m_cbDiscardMode.AddString (ResStr (IDS_VDF_DBLK_ALL));
#endif /* INTERNAL_DECODER_H264 */

	// Error recognition
	nPosY += VERTICAL_SPACING;
	m_txtErrorRecognition.Create (ResStr (IDS_VDF_ERROR_RECOGNITION), WS_VISIBLE|WS_CHILD, CRect (LEFT_SPACING,  nPosY, 220, nPosY+15), this, (UINT)IDC_STATIC);
	m_cbErrorRecognition.Create  (WS_VISIBLE|WS_CHILD|CBS_DROPDOWNLIST|WS_VSCROLL, CRect (230,  nPosY-4, 345, nPosY+90), this, IDC_PP_DISCARD_MODE);
	m_cbErrorRecognition.AddString (ResStr (IDS_VDF_ERR_CAREFUL));
	m_cbErrorRecognition.AddString (ResStr (IDS_VDF_ERR_COMPLIANT));
	m_cbErrorRecognition.AddString (ResStr (IDS_VDF_ERR_AGGRESSIVE));

	// IDCT Algo
	nPosY += VERTICAL_SPACING;
	m_txtIDCTAlgo.Create (ResStr (IDS_VDF_IDCT_ALGO), WS_VISIBLE|WS_CHILD, CRect (LEFT_SPACING,  nPosY, 220, nPosY+15), this, (UINT)IDC_STATIC);
	m_cbIDCTAlgo.Create  (WS_VISIBLE|WS_CHILD|CBS_DROPDOWNLIST|WS_VSCROLL, CRect (230,  nPosY-4, 345, nPosY+90), this, IDC_PP_DISCARD_MODE);
	m_cbIDCTAlgo.AddString (ResStr (IDS_VDF_IDCT_AUTO));
	m_cbIDCTAlgo.AddString (ResStr (IDS_VDF_IDCT_LIBMPEG2));
	m_cbIDCTAlgo.AddString (ResStr (IDS_VDF_IDCT_SIMPLE_MMX));
	m_cbIDCTAlgo.AddString (ResStr (IDS_VDF_IDCT_XVID));
	m_cbIDCTAlgo.AddString (ResStr (IDS_VDF_IDCT_SIMPLE));

	nPosY += VERTICAL_SPACING;
	m_cbARMode.Create (ResStr (IDS_VDF_AR_MODE), WS_VISIBLE|WS_CHILD|BS_AUTOCHECKBOX|BS_LEFTTEXT, CRect (LEFT_SPACING,  nPosY, 345, nPosY+15), this, IDC_PP_AR);
	m_cbARMode.SetCheck(FALSE);

	nPosY = 170;
#endif /* HAS_FFMPEG_VIDEO_DECODERS */

	m_grpDXVA.Create   (ResStr (IDS_VDF_DXVA_SETTING),   WS_VISIBLE|WS_CHILD | BS_GROUPBOX, CRect (5, nPosY, 350, nPosY+135), this, (UINT)IDC_STATIC);

	// DXVA Compatibility check
	nPosY += VERTICAL_SPACING;
	m_txtDXVACompatibilityCheck.Create (ResStr (IDS_VDF_DXVACOMPATIBILITY), WS_VISIBLE|WS_CHILD, CRect (LEFT_SPACING,  nPosY, 225, nPosY+15), this, (UINT)IDC_STATIC);
	m_cbDXVACompatibilityCheck.Create  (WS_VISIBLE|WS_CHILD|CBS_DROPDOWNLIST|WS_VSCROLL, CRect (230,  nPosY-4, 345, nPosY+90), this, IDC_PP_DXVA_CHECK);
	m_cbDXVACompatibilityCheck.AddString(ResStr(IDS_VDF_DXVA_FULLCHECK));
	m_cbDXVACompatibilityCheck.AddString(ResStr(IDS_VDF_DXVA_SKIP_LEVELCHECK));
	m_cbDXVACompatibilityCheck.AddString(ResStr(IDS_VDF_DXVA_SKIP_REFCHECK));
	m_cbDXVACompatibilityCheck.AddString(ResStr(IDS_VDF_DXVA_SKIP_ALLCHECK));

	nPosY += VERTICAL_SPACING;
	m_cbDXVA_SD.Create (ResStr (IDS_VDF_DXVA_SD), WS_VISIBLE|WS_CHILD|BS_AUTOCHECKBOX|BS_LEFTTEXT, CRect (LEFT_SPACING,  nPosY, 345, nPosY+15), this, IDC_PP_DXVA_SD);
	m_cbDXVA_SD.SetCheck(FALSE);

	// DXVA mode
	nPosY += VERTICAL_SPACING;
	m_txtDXVAMode.Create (ResStr (IDS_VDF_DXVA_MODE), WS_VISIBLE|WS_CHILD, CRect (LEFT_SPACING,  nPosY, 120, nPosY+15), this, (UINT)IDC_STATIC);
	m_edtDXVAMode.Create (WS_CHILD|WS_VISIBLE|WS_DISABLED, CRect (120,  nPosY, 345, nPosY+20), this, 0);

	// Video card description
	nPosY += VERTICAL_SPACING;
	m_txtVideoCardDescription.Create (ResStr (IDS_VDF_VIDEOCARD), WS_VISIBLE|WS_CHILD, CRect (LEFT_SPACING,  nPosY, 120, nPosY+15), this, (UINT)IDC_STATIC);
	m_edtVideoCardDescription.Create (ES_MULTILINE|WS_CHILD|WS_VISIBLE|WS_DISABLED, CRect (120,  nPosY, 345, nPosY+30), this, 0);
	m_edtVideoCardDescription.SetWindowText (m_pMDF->GetVideoCardDescription());


	DxvaGui = m_pMDF->GetDXVADecoderGuid();
	if (DxvaGui != NULL) {
		CString DXVAMode = GetDXVAMode (DxvaGui);
		m_edtDXVAMode.SetWindowText (DXVAMode);
	} else {
		m_txtDXVAMode.ShowWindow (SW_HIDE);
		m_edtDXVAMode.ShowWindow (SW_HIDE);
	}

	for (CWnd* pWnd = GetWindow(GW_CHILD); pWnd; pWnd = pWnd->GetNextWindow()) {
		pWnd->SetFont(&m_font, FALSE);
	}

	CorrectComboListWidth(m_cbDXVACompatibilityCheck);
#if INTERNAL_DECODER_H264
	CorrectComboListWidth(m_cbDiscardMode);
#endif

	if (m_pMDF) {
#if HAS_FFMPEG_VIDEO_DECODERS
#if INTERNAL_DECODER_H264
		m_cbThreadNumber.SetCurSel		(m_pMDF->GetThreadNumber());
		m_cbDiscardMode.SetCurSel		(FindDiscardIndex (m_pMDF->GetDiscardMode()));
#endif
		m_cbErrorRecognition.SetCurSel	(FindErrRecognitionIndex (m_pMDF->GetErrorRecognition()));
		m_cbIDCTAlgo.SetCurSel			(m_pMDF->GetIDCTAlgo());

		m_cbARMode.SetCheck(m_pMDF->GetARMode());
#endif /* HAS_FFMPEG_VIDEO_DECODERS */

		m_cbDXVACompatibilityCheck.SetCurSel(m_pMDF->GetDXVACheckCompatibility());
		m_cbDXVA_SD.SetCheck(m_pMDF->GetDXVA_SD());
	}

	return true;
}

void CMPCVideoDecSettingsWnd::OnDeactivate()
{
}

bool CMPCVideoDecSettingsWnd::OnApply()
{
	OnDeactivate();

	if (m_pMDF && m_cbDXVACompatibilityCheck.m_hWnd) {
#if HAS_FFMPEG_VIDEO_DECODERS
#if INTERNAL_DECODER_H264
		m_pMDF->SetThreadNumber		(m_cbThreadNumber.GetCurSel());
		m_pMDF->SetDiscardMode		(g_AVDiscard[m_cbDiscardMode.GetCurSel()]);
#endif /* INTERNAL_DECODER_H264 */
		m_pMDF->SetErrorRecognition  (g_AVErrRecognition[m_cbErrorRecognition.GetCurSel()]);
		m_pMDF->SetIDCTAlgo			(m_cbIDCTAlgo.GetCurSel());

		m_pMDF->SetARMode(m_cbARMode.GetCheck());
#endif /* HAS_FFMPEG_VIDEO_DECODERS */

		m_pMDF->SetDXVACheckCompatibility(m_cbDXVACompatibilityCheck.GetCurSel());

		m_pMDF->SetDXVA_SD(m_cbDXVA_SD.GetCheck());

		m_pMDF->Apply();
	}

	return true;
}


BEGIN_MESSAGE_MAP(CMPCVideoDecSettingsWnd, CInternalPropertyPageWnd)
END_MESSAGE_MAP()


// ====== Codec filter property page (for standalone filter only)

CMPCVideoDecCodecWnd::CMPCVideoDecCodecWnd()
{
}

bool CMPCVideoDecCodecWnd::OnConnect(const CInterfaceList<IUnknown, &IID_IUnknown>& pUnks)
{
	ASSERT(!m_pMDF);

	m_pMDF.Release();

	POSITION pos = pUnks.GetHeadPosition();
	while (pos && !(m_pMDF = pUnks.GetNext(pos))) {
		;
	}

	if (!m_pMDF) {
		return false;
	}

	return true;
}

void CMPCVideoDecCodecWnd::OnDisconnect()
{
	m_pMDF.Release();
}

bool CMPCVideoDecCodecWnd::OnActivate()
{
	DWORD				dwStyle = WS_VISIBLE|WS_CHILD|WS_BORDER;
	int					nPos	= 0;
	MPC_VIDEO_CODEC		nActiveCodecs = (MPC_VIDEO_CODEC)(m_pMDF ? m_pMDF->GetActiveCodecs() : 0);

	m_grpSelectedCodec.Create (_T("Selected codecs"), WS_VISIBLE|WS_CHILD | BS_GROUPBOX, CRect (10,  10, 330, 280), this, (UINT)IDC_STATIC);

	m_lstCodecs.Create (dwStyle | LBS_OWNERDRAWFIXED | LBS_HASSTRINGS | LBS_NOINTEGRALHEIGHT | WS_VSCROLL | WS_TABSTOP, CRect (20,30, 320, 270), this, 0);

#if INTERNAL_DECODER_H264_DXVA
	m_lstCodecs.AddString (_T("H.264/AVC (DXVA)"));
	m_lstCodecs.SetCheck  (nPos++, (nActiveCodecs & MPCVD_H264_DXVA) != 0);
#endif
#if INTERNAL_DECODER_VC1_DXVA
	m_lstCodecs.AddString (_T("VC1 (DXVA)"));
	m_lstCodecs.SetCheck  (nPos++, (nActiveCodecs & MPCVD_VC1_DXVA) != 0);
#endif
#if INTERNAL_DECODER_WMV3_DXVA
	m_lstCodecs.AddString (_T("WMV3 (DXVA)"));
	m_lstCodecs.SetCheck  (nPos++, (nActiveCodecs & MPCVD_WMV3_DXVA) != 0);
#endif
#if INTERNAL_DECODER_MPEG2_DXVA
	m_lstCodecs.AddString (_T("MPEG2 (DXVA)"));
	m_lstCodecs.SetCheck  (nPos++, (nActiveCodecs & MPCVD_MPEG2_DXVA) != 0);
#endif

#if INTERNAL_DECODER_H264
	m_lstCodecs.AddString (_T("H.264/AVC (FFmpeg)"));
	m_lstCodecs.SetCheck  (nPos++, (nActiveCodecs & MPCVD_H264) != 0);
#endif
#if INTERNAL_DECODER_VC1
	m_lstCodecs.AddString (_T("VC1 (FFmpeg)"));
	m_lstCodecs.SetCheck  (nPos++, (nActiveCodecs & MPCVD_VC1) != 0);
#endif

#if HAS_FFMPEG_VIDEO_DECODERS
	m_lstCodecs.AddString (_T("Xvid/MPEG-4"));
	m_lstCodecs.SetCheck  (nPos++, (nActiveCodecs & MPCVD_XVID) != 0);
	m_lstCodecs.AddString (_T("DivX"));
	m_lstCodecs.SetCheck  (nPos++, (nActiveCodecs & MPCVD_DIVX) != 0);
	m_lstCodecs.AddString (_T("MS-MPEG4"));
	m_lstCodecs.SetCheck  (nPos++, (nActiveCodecs & MPCVD_MSMPEG4) != 0);
	m_lstCodecs.AddString (_T("FLV1/4"));
	m_lstCodecs.SetCheck  (nPos++, (nActiveCodecs & MPCVD_FLASH) != 0);
	m_lstCodecs.AddString (_T("VP3/5/6"));
	m_lstCodecs.SetCheck  (nPos++, (nActiveCodecs & MPCVD_VP356) != 0);
	m_lstCodecs.AddString (_T("VP8"));
	m_lstCodecs.SetCheck  (nPos++, (nActiveCodecs & MPCVD_VP8) != 0);
	m_lstCodecs.AddString (_T("WMV1/2/3"));
	m_lstCodecs.SetCheck  (nPos++, (nActiveCodecs & MPCVD_WMV) != 0);
	m_lstCodecs.AddString (_T("H.263"));
	m_lstCodecs.SetCheck  (nPos++, (nActiveCodecs & MPCVD_H263) != 0);
	m_lstCodecs.AddString (_T("SVQ1/3"));
	m_lstCodecs.SetCheck  (nPos++, (nActiveCodecs & MPCVD_SVQ3) != 0);
	m_lstCodecs.AddString (_T("AMV video"));
	m_lstCodecs.SetCheck  (nPos++, (nActiveCodecs & MPCVD_AMVV) != 0);
	m_lstCodecs.AddString (_T("Theora"));
	m_lstCodecs.SetCheck  (nPos++, (nActiveCodecs & MPCVD_THEORA) != 0);
	m_lstCodecs.AddString (_T("MJPEG"));
	m_lstCodecs.SetCheck  (nPos++, (nActiveCodecs & MPCVD_MJPEG) != 0);
	m_lstCodecs.AddString (_T("Indeo 3/4/5"));
	m_lstCodecs.SetCheck  (nPos++, (nActiveCodecs & MPCVD_INDEO) != 0);
	m_lstCodecs.AddString (_T("Real Video"));
	m_lstCodecs.SetCheck  (nPos++, (nActiveCodecs & MPCVD_RV) != 0);
#endif

	for (CWnd* pWnd = GetWindow(GW_CHILD); pWnd; pWnd = pWnd->GetNextWindow()) {
		pWnd->SetFont(&m_font, FALSE);
	}

	return true;
}

void CMPCVideoDecCodecWnd::OnDeactivate()
{
}

bool CMPCVideoDecCodecWnd::OnApply()
{
	OnDeactivate();

	if (m_pMDF) {
		int			nActiveCodecs = 0;
		int			nPos		  = 0;

#if INTERNAL_DECODER_H264_DXVA
		if (m_lstCodecs.GetCheck  (nPos++)) {
			nActiveCodecs |= MPCVD_H264_DXVA;
		}
#endif
#if INTERNAL_DECODER_VC1_DXVA
		if (m_lstCodecs.GetCheck  (nPos++)) {
			nActiveCodecs |= MPCVD_VC1_DXVA;
		}
#endif
#if INTERNAL_DECODER_WMV3_DXVA
		if (m_lstCodecs.GetCheck  (nPos++)) {
			nActiveCodecs |= MPCVD_WMV3_DXVA;
		}
#endif
#if INTERNAL_DECODER_MPEG2_DXVA
		if (m_lstCodecs.GetCheck  (nPos++)) {
			nActiveCodecs |= MPCVD_MPEG2_DXVA;
		}
#endif
#if INTERNAL_DECODER_H264
		if (m_lstCodecs.GetCheck  (nPos++)) {
			nActiveCodecs |= MPCVD_H264;
		}
#endif
#if INTERNAL_DECODER_VC1
		if (m_lstCodecs.GetCheck  (nPos++)) {
			nActiveCodecs |= MPCVD_VC1;
		}
#endif
#if HAS_FFMPEG_VIDEO_DECODERS
		if (m_lstCodecs.GetCheck  (nPos++)) {
			nActiveCodecs |= MPCVD_XVID;
		}
		if (m_lstCodecs.GetCheck  (nPos++)) {
			nActiveCodecs |= MPCVD_DIVX;
		}
		if (m_lstCodecs.GetCheck  (nPos++)) {
			nActiveCodecs |= MPCVD_MSMPEG4;
		}
		if (m_lstCodecs.GetCheck  (nPos++)) {
			nActiveCodecs |= MPCVD_FLASH;
		}
		if (m_lstCodecs.GetCheck  (nPos++)) {
			nActiveCodecs |= MPCVD_VP356;
		}
		if (m_lstCodecs.GetCheck  (nPos++)) {
			nActiveCodecs |= MPCVD_VP8;
		}
		if (m_lstCodecs.GetCheck  (nPos++)) {
			nActiveCodecs |= MPCVD_WMV;
		}
		if (m_lstCodecs.GetCheck  (nPos++)) {
			nActiveCodecs |= MPCVD_H263;
		}
		if (m_lstCodecs.GetCheck  (nPos++)) {
			nActiveCodecs |= MPCVD_SVQ3;
		}
		if (m_lstCodecs.GetCheck  (nPos++)) {
			nActiveCodecs |= MPCVD_AMVV;
		}
		if (m_lstCodecs.GetCheck  (nPos++)) {
			nActiveCodecs |= MPCVD_THEORA;
		}
		if (m_lstCodecs.GetCheck  (nPos++)) {
			nActiveCodecs |= MPCVD_MJPEG;
		}
		if (m_lstCodecs.GetCheck  (nPos++)) {
			nActiveCodecs |= MPCVD_INDEO;
		}
		if (m_lstCodecs.GetCheck  (nPos++)) {
			nActiveCodecs |= MPCVD_RV;
		}
#endif
		m_pMDF->SetActiveCodecs ((MPC_VIDEO_CODEC)nActiveCodecs);

		m_pMDF->Apply();
	}

	return true;
}


BEGIN_MESSAGE_MAP(CMPCVideoDecCodecWnd, CInternalPropertyPageWnd)
END_MESSAGE_MAP()
