#ifndef ISPP_INVOKED
/*
 * (C) 2013-2017 see Authors.txt
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
#endif

#ifndef MPC_HC_CONFIG_H
#define MPC_HC_CONFIG_H

#ifndef _T
#if !defined(ISPP_INVOKED) && (defined(UNICODE) || defined(_UNICODE))
#define _T(text) L##text
#else
#define _T(text) text
#endif
#endif

#define WEBSITE_URL  _T("https://mpc-hc.org/")
#define DOWNLOAD_URL _T("https://mpc-hc.org/downloads/")
#define UPDATE_URL   _T("https://mpc-hc.org/version.txt")
#define TRAC_URL     _T("https://trac.mpc-hc.org/")
#define BUGS_URL     _T("https://trac.mpc-hc.org/wiki/How_to_Report_Issues")
#define TOOLBARS_URL _T("https://trac.mpc-hc.org/wiki/Toolbar_images")

#define USE_STATIC_UNRAR 1

#ifndef MPCHC_LITE
#define USE_STATIC_MEDIAINFO 1
#endif

#define SHADERS_DIR _T("Shaders")
#define SHADERS_EXT _T(".hlsl")

// If this is enabled, the registered LAV Filters can be loaded as internal filters
#define ENABLE_LOAD_EXTERNAL_LAVF_AS_INTERNAL 0

#define DO_MAKE_STR(x)          _T(#x)
#define MAKE_STR(x)             DO_MAKE_STR(x)

#define MPC_DX_SDK_MONTH  _T("June")
#define MPC_DX_SDK_YEAR   2010
// Used in the installer
#define MPC_DX_SDK_NUMBER 43
#define MPC_D3D_COMPILER_VERSION 47
#ifndef ISPP_INVOKED
// Used in build scripts
#define MPCHC_WINSDK_VER 8.1
#endif

#endif // MPC_HC_CONFIG_H
