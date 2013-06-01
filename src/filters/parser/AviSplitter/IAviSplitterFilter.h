/*
 * (C) 2012-2013 see Authors.txt
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

interface __declspec(uuid("1E41BBCB-F6E0-4A49-91F7-A2AD5B5C7113"))
IAviSplitterFilter :
public IUnknown {
    STDMETHOD(Apply()) PURE;

    STDMETHOD(SetNonInterleavedFilesSupport(BOOL nValue)) PURE;
    STDMETHOD_(BOOL, GetNonInterleavedFilesSupport()) PURE;
};
