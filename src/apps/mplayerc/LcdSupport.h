/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2010 see AUTHORS
 *
 * This file is part of mplayerc.
 *
 * Mplayerc is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mplayerc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include <windows.h>

#include <lglcd/lglcd.h>
#include "../../thirdparty/ui/LCDUI/LCDManager.h"
#include "../../thirdparty/ui/LCDUI/LCDOutput.h"
#include "../../thirdparty/ui/LCDUI/LCDScrollingText.h"
#include "../../thirdparty/ui/LCDUI/LCDProgressBar.h"
#include "../../thirdparty/ui/LCDUI/LCDAnimatedBitmap.h"


class CLCDMyManager : public CLCDManager
{
public:
	virtual HRESULT Initialize(void);
	virtual void OnLCDButtonUp(int nButton);

	CLCDScrollingText	m_Text1;
	CLCDText			m_Text[2];
	CLCDProgressBar		m_ProgBar[2];
	CLCDAnimatedBitmap	m_PlayState;
};

class CMPC_Lcd
{
public:
	enum PlayState
	{
		PS_PLAY = 0,
		PS_PAUSE  = 1,
		PS_STOP   = 2,
		PS_UNUSED = 3
	};

private:
	lgLcdConnectContext	m_ConnCtx;
	HANDLE				hLCD_UpdateThread;
	HBITMAP				hBmp[PS_UNUSED];

public:
	CLCDOutput			m_Output;
	CLCDMyManager		m_Manager;
	bool				Thread_Loop;
	__time64_t			nThread_tTimeout;
	CRITICAL_SECTION	cs;

	CMPC_Lcd();
	~CMPC_Lcd();

	void SetMediaTitle(const _TCHAR * text);
	void SetMediaRange(__int64 nStart, __int64 nStop);
	void SetMediaPos(__int64 nPos);
	void SetVolumeRange(__int64 nStart, __int64 nStop);
	void SetVolume(__int64 nVol);
	void SetStatusMessage(const _TCHAR * text, int nTimeOut);
	void SetPlayState(PlayState ps);
};
