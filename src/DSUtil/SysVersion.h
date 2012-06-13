/*
 * $Id$
 *
 * (C) 2012 see Authors.txt
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

#include <Windows.h>

class SysVersion
{
    SysVersion() {};

    static OSVERSIONINFOEX InitFullVersion();

    static const OSVERSIONINFOEX fullVersion;
    static const DWORD version;

public:
    static OSVERSIONINFOEX GetFullVersion() { return fullVersion; }
    static DWORD GetVersion() { return version; }

    static bool IsXPOrLater() { return (version >= 0x0501); }
    static bool IsVista() { return (version == 0x0600); }
    static bool IsVistaOrLater() { return (version >= 0x0600); }
    static bool Is7() { return (version == 0x0601); }
    static bool Is7OrLater() { return (version >= 0x0601); }
    //static bool Is8() { return (version == 0x0602); }
};
