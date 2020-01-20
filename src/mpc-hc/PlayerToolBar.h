/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2014, 2016 see Authors.txt
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

#include "VolumeCtrl.h"
#include "CMPCThemeToolTipCtrl.h"

#include <atlimage.h>

class CMainFrame;

class CPlayerToolBar : public CToolBar
{
    DECLARE_DYNAMIC(CPlayerToolBar)

private:
    CMainFrame* m_pMainFrame;

    bool IsMuted() const;
    void SetMute(bool fMute = true);
    int getHitButtonIdx(CPoint point);
    bool LoadExternalToolBar(CImage& image);
    void LoadToolbarImage();
    bool mouseDown;
    CMPCThemeToolTipCtrl themedToolTip;

    int m_nButtonHeight;
    std::unique_ptr<CImageList> m_pButtonsImages;
    std::unique_ptr<CImageList> m_pDisabledButtonsImages;
    int m_volumeMinSizeInc;

    EventClient m_eventc;
    void EventCallback(MpcEvent ev);

public:
    CPlayerToolBar(CMainFrame* pMainFrame);
    virtual ~CPlayerToolBar();

    bool LoadExternalToolBar(CImage& image, bool useColor);

    int GetVolume() const;
    int GetMinWidth() const;
    void SetVolume(int volume);
    __declspec(property(get = GetVolume, put = SetVolume)) int Volume;

    void ArrangeControls();

    CVolumeCtrl m_volctrl;

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CPlayerToolBar)
    virtual BOOL Create(CWnd* pParentWnd);
    //}}AFX_VIRTUAL

    // Generated message map functions
protected:
    //{{AFX_MSG(CPlayerToolBar)
    afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnInitialUpdate();
    afx_msg BOOL OnVolumeMute(UINT nID);
    afx_msg void OnUpdateVolumeMute(CCmdUI* pCmdUI);
    afx_msg BOOL OnVolumeUp(UINT nID);
    afx_msg BOOL OnVolumeDown(UINT nID);
    afx_msg void OnNcPaint();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    afx_msg BOOL OnToolTipNotify(UINT id, NMHDR* pNMHDR, LRESULT* pResult);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};
