/*
 * (C) 2010-2012, 2017 see Authors.txt
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

// {F9F62627-E3EF-4a2e-B6C9-5D4C0DC3326B}
DEFINE_GUID(CLSID_SyncAllocatorPresenter, 0xf9f62627, 0xe3ef, 0x4a2e, 0xb6, 0xc9, 0x5d, 0x4c, 0xd, 0xc3, 0x32, 0x6b);

interface __declspec(uuid("F891C2A9-1DFF-45e0-9129-30C0990C5A9F"))
    ISyncClockAdviser :
    public IUnknown
{
    STDMETHOD(AdviseSyncClock)(ISyncClock* sC) PURE;
};

HRESULT CreateSyncRenderer(const CLSID& clsid, HWND hWnd, bool bFullscreen, ISubPicAllocatorPresenter** ppAP);
