#ifndef ISPP_INVOKED
/*
 * (C) 2013 see Authors.txt
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

#define WEBSITE_URL  _T("http://mpc-hc.org/")
#define DOWNLOAD_URL _T("http://mpc-hc.org/downloads/")
#define UPDATE_URL   _T("http://mpc-hc.org/version.txt")
#define TRAC_URL     _T("https://trac.mpc-hc.org/")
#define BUGS_URL     _T("https://trac.mpc-hc.org/wiki/How_to_Report_Issues")
#define TOOLBARS_URL _T("https://trac.mpc-hc.org/wiki/Toolbar_images")

#define USE_STATIC_UNRAR 1

#ifndef MPCHC_LITE
#define USE_STATIC_MEDIAINFO 1
#endif

// If you distribute your builds, please disable minidumps by defining ENABLE_MINIDUMP 0.
#define ENABLE_MINIDUMP 1

#endif // MPC_HC_CONFIG_H
