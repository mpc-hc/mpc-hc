/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2012, 2017 see Authors.txt
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

//
// IQTVideoSurface
//

interface __declspec(uuid("A6AE36F7-A6F2-4157-AF54-6599857E4E20"))
    IQTVideoSurface :
    public IUnknown
{
    STDMETHOD(BeginBlt)(const BITMAP& bm) PURE;
    STDMETHOD(DoBlt)(const BITMAP& bm) PURE;
};
