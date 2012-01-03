//
// $Id$
//
// (C) 2010-2012 see AUTHORS
//
// This file is part of MPC-HC.
//
// MPC-HC is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// MPC-HC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


// Notes:
// * Do not use /* comments */ since ISPP is buggy and it will throw an error.
//
// * NO_VERSION_REV_NEEDED is defined in those cases where we don't need the revision
//   number, but only the major/minor/patch version so the compiler does not rebuild
//   everything for every revision. It's used in mpcresources and mpciconlib projects.
//
// * ISPP_IS_BUGGY is defined in the installer script only and it's just a workaround
//   for ISPP being buggy and throwing an error for the various defines.
//
// * DIRECTX_SDK_DATE is the date of the DirectX SDK used for compilation and it's used
//   in the error message in mpc-hc and in the installer when the DX runtime is out of date.
//
// * DIRECTX_SDK_NUMBER is used in the installer when the DX runtime is out of date.


#ifndef MPC_VERSION_H
#define MPC_VERSION_H


#ifndef NO_VERSION_REV_NEEDED
#include "Version_rev.h"
#endif

#define DO_MAKE_STR(x) #x
#define MAKE_STR(x)    DO_MAKE_STR(x)

#define MPC_VERSION_MAJOR 1
#define MPC_VERSION_MINOR 5
#define MPC_VERSION_PATCH 3


#ifndef ISPP_IS_BUGGY

#define MPC_COMP_NAME_STR    L"MPC-HC Team"
#define MPC_COPYRIGHT_STR    L"Copyright © 2002-2012 all contributors, see Authors.txt"

#ifdef  NO_VERSION_REV_NEEDED
#define MPC_VERSION_NUM      MPC_VERSION_MAJOR,MPC_VERSION_MINOR,MPC_VERSION_PATCH,0
#define MPC_VERSION_STR      MAKE_STR(MPC_VERSION_MAJOR) ", " MAKE_STR(MPC_VERSION_MINOR) ", " MAKE_STR(MPC_VERSION_PATCH) ", 0"
#else
#define MPC_VERSION_NUM      MPC_VERSION_MAJOR,MPC_VERSION_MINOR,MPC_VERSION_PATCH,MPC_VERSION_REV
#define MPC_VERSION_STR      MAKE_STR(MPC_VERSION_MAJOR) ", " MAKE_STR(MPC_VERSION_MINOR) ", " MAKE_STR(MPC_VERSION_PATCH) ", " MAKE_STR(MPC_VERSION_REV)
#endif // NO_VERSION_REV_NEEDED

#endif // ISPP_IS_BUGGY

#define MPC_VERSION_COMMENTS "http://sourceforge.net/projects/mpc-hc/"

#define DIRECTX_SDK_DATE     "June 2010"
#define DIRECTX_SDK_NUMBER   "43"


#endif // MPC_VERSION_H
