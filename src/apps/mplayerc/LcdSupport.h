/* $Id: LcdSupport.h,v 1.1 2006-11-04 22:41:51 wild Exp $
 *
 * Logitech LCD Support Header
 */

#ifndef __LCDSUPPORT_H__
#define __LCDSUPPORT_H__

#include <windows.h>

#include <lglcd/lglcd.h>
#include "../../ui/LCDUI/LCDManager.h"
#include "../../ui/LCDUI/LCDOutput.h"
#include "../../ui/LCDUI/LCDScrollingText.h"
#include "../../ui/LCDUI/LCDProgressBar.h"
#include "../../ui/LCDUI/LCDAnimatedBitmap.h"


class CLCDMyManager : public CLCDManager  
{
	public:
		virtual HRESULT Initialize(void);
		virtual void OnLCDButtonUp(int nButton);

		CLCDScrollingText	m_Text1;
		CLCDText		m_Text2;
		CLCDProgressBar		m_ProgBar[2];
		CLCDAnimatedBitmap	m_PlayState;
};

class CMPC_Lcd
{
	public:
		enum PlayState {PS_PLAY   = 0,
				PS_PAUSE  = 1,
				PS_STOP   = 2,
				PS_UNUSED = 3};

	private:
		lgLcdConnectContext	m_ConnCtx;
		HANDLE			hLCD_UpdateThread;
		HBITMAP			hBmp[PS_UNUSED];

	public:
		CLCDOutput		m_Output;
		CLCDMyManager		m_Manager;
		bool			Thread_Loop;
		__time64_t		nThread_tTimeout;

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


#endif /* __LCDSUPPORT_H__ */
