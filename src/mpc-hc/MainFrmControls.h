/*
 * (C) 2013-2014, 2017 see Authors.txt
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

#include <map>
#include <set>
#include <vector>

class CMainFrame;
class CPlayerBar;

class CMainFrameControls
{
    friend class CPlayerBar; // notifies of panel re-dock
    CMainFrame* m_pMainFrame;

    struct ControlsVisibilityState {
        UINT nVisibleCS;
        UINT nCurrentCS;
        CPoint lastShowPoint;
        bool bLastCanAutoHideToolbars;
        bool bLastCanAutoHidePanels;
        bool bLastHaveExclusiveSeekbar;
        ControlsVisibilityState();
    } m_controlsVisibilityState;

    UINT GetEffectiveToolbarsSelection();
    bool ShowToolbarsSelection();
    bool ShowToolbars(UINT nCS);
    unsigned GetToolbarsHeight(UINT nCS) const;

    bool m_bDelayShowNotLoaded;
    void DelayShowNotLoadedCallback();

    bool InFullscreenWithPermahiddenToolbars();
    bool InFullscreenWithPermahiddenDockedPanels();

public:
    CMainFrameControls(CMainFrame* pMainFrame);
    ~CMainFrameControls();

    enum class Toolbar {
        SEEKBAR,
        CONTROLS,
        INFO,
        STATS,
        STATUS,
    };
    std::map<Toolbar, CControlBar*> m_toolbars;

    enum class Panel {
        SUBRESYNC,
        PLAYLIST,
        CAPTURE,
        NAVIGATION,
        EDL,
    };
    std::map<Panel, CPlayerBar*> m_panels;

    bool ControlChecked(Toolbar toolbar);
    bool ControlChecked(Panel panel);
    void ToggleControl(Toolbar toolbar);
    void ToggleControl(Panel panel);
    void SetToolbarsSelection(UINT nCS, bool bDelayHide = false);

    void LoadState();
    void SaveState();

    void GetDockZones(unsigned& uTop, unsigned& uLeft, unsigned& uRight, unsigned& uBottom, bool bDoEnum = true);
    void GetVisibleDockZones(unsigned& uTop, unsigned& uLeft, unsigned& uRight, unsigned& uBottom, bool bDoEnum = true);
    CSize GetDockZonesMinSize(unsigned uSaneFallback);

    bool PanelsCoverVideo() const;
    bool ToolbarsCoverVideo() const;

    void UpdateToolbarsVisibility();
    unsigned GetVisibleToolbarsHeight() const;
    unsigned GetToolbarsHeight() const;

    void DelayShowNotLoaded(bool bDoDelay);
    bool DelayShowNotLoaded() const;

private:
    enum DockZone { DOCK_NONE, DOCK_TOP = 1, DOCK_BOTTOM = 1 << 1, DOCK_LEFT = 1 << 2, DOCK_RIGHT = 1 << 3 };
    DockZone GetPanelZone(CPlayerBar* pBar);
    DockZone GetPanelZone(Panel panel);
    void EnumPanelZones();
    void GetDockZonesInternal(unsigned& uTop, unsigned& uLeft, unsigned& uRight, unsigned& uBottom, bool bDoEnum, bool bOnlyVisible);
    void LockHideZone(DockZone zone);
    std::map<DockZone, std::vector<Panel>> m_panelZones;
    std::map<DockZone, CDockBar*> m_panelDocks;
    std::map<DockZone, ULONGLONG> m_zoneHideTicks;
    std::set<DockZone> m_zoneHideLocks;
};
