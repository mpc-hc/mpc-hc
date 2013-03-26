#ifndef ISPP_INVOKED
/*
 * (C) 2010-2013 see Authors.txt
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


 * Notes:
 * NO_VERSION_REV_NEEDED is defined in those cases where we don't need the revision
   number, but only the major/minor/patch version so the compiler does not rebuild
   everything for every revision. It's currently used in mpcresources, mpciconlib
   and VideoRenderers projects.
 * MPC_VERSION_ARCH is currently used in VSFilter only.
 * MPC_DX_SDK_MONTH, MPC_DX_SDK_YEAR and MPC_DX_SDK_NUMBER is the month, year and the number
   of the DirectX SDK used for compilation and it's used in the error message in mpc-hc and in
   the installer when the DX runtime is out of date.
 * MPC_DX_SDK_NUMBER is used in the installer when the DirectX runtime is out of date.
 */
#endif // ISPP_INVOKED

#ifndef MPC_VERSION_H
#define MPC_VERSION_H

#include "mpc-hc_config.h"

#ifndef _T
#if !defined(ISPP_INVOKED) && (defined(UNICODE) || defined(_UNICODE))
#define _T(text)    L##text
#else
#define _T(text)    text
#endif
#endif

#ifndef NO_VERSION_REV_NEEDED
#include "version_rev.h"
#endif

#define DO_MAKE_STR(x)      _T(#x)
#define MAKE_STR(x)         DO_MAKE_STR(x)

#define MPC_VERSION_MAJOR   1
#define MPC_VERSION_MINOR   6
#define MPC_VERSION_PATCH   8

#define MPC_BETA_RELEASE    1

#define MPC_COMP_NAME_STR       _T("MPC-HC Team")
#define MPC_COPYRIGHT_STR       _T("Copyright © 2002-2013 all contributors, see Authors.txt")
#define MPC_VERSION_COMMENTS    WEBSITE_URL


#ifndef ISPP_INVOKED

#ifdef NO_VERSION_REV_NEEDED

#define MPC_VERSION_NUM         MPC_VERSION_MAJOR,MPC_VERSION_MINOR,MPC_VERSION_PATCH,0
#define MPC_VERSION_STR         MAKE_STR(MPC_VERSION_MAJOR) _T(".") \
                                MAKE_STR(MPC_VERSION_MINOR) _T(".") \
                                MAKE_STR(MPC_VERSION_PATCH) _T(".0")
#define MPC_VERSION_STR_FULL    MPC_VERSION_STR

#else // !NO_VERSION_REV_NEEDED

#define MPC_VERSION_NUM         MPC_VERSION_MAJOR,MPC_VERSION_MINOR,MPC_VERSION_PATCH,MPC_VERSION_REV
#define MPC_VERSION_STR         MAKE_STR(MPC_VERSION_MAJOR) _T(".") \
                                MAKE_STR(MPC_VERSION_MINOR) _T(".") \
                                MAKE_STR(MPC_VERSION_PATCH) _T(".") \
                                MAKE_STR(MPC_VERSION_REV)
#define MPC_VERSION_STR_FULL    MAKE_STR(MPC_VERSION_MAJOR) _T(".") \
                                MAKE_STR(MPC_VERSION_MINOR) _T(".") \
                                MAKE_STR(MPC_VERSION_PATCH) _T(".") \
                                MPC_VERSION_REV_FULL
#endif // NO_VERSION_REV_NEEDED

#endif // ISPP_INVOKED


#if MPC_BETA_RELEASE
#define MPC_VERSION_BETA        _T("Beta")
#endif

#ifdef _WIN64
#define MPC_VERSION_ARCH        _T("x64")
#else
#define MPC_VERSION_ARCH        _T("x86")
#endif

#define MPC_DX_SDK_MONTH        _T("June")
#define MPC_DX_SDK_YEAR         2010
#define MPC_DX_SDK_NUMBER       43


#endif // MPC_VERSION_H
