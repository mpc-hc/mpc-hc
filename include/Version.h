// Notes:
// * Do not use /* comments */ since ISPP is buggy and it will throw an error.
//
// * NO_VERSION_REV_NEEDED is defined in those cases where we don't need the revision
//   number, but only the major/minor/patch version so the compiler does not rebuild
//   everything. It's used in the mpcresources and the mpciconlib projects.
//
// * ISPP_IS_BUGGY is defined in the installer script only and it's just a workaround
//   for ISPP being buggy and throwing an error.
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
#define MAKE_STR(x) DO_MAKE_STR(x)

#define MPC_VERSION_MAJOR 1
#define MPC_VERSION_MINOR 5
#define MPC_VERSION_PATCH 1

#define MPC_COMP_NAME        "MPC-HC Team"
#define MPC_COPYRIGHT        "Copyright (C) 2002-2011 see AUTHORS file"

#ifndef ISPP_IS_BUGGY
#define MPC_VERSION           MPC_VERSION_MAJOR,MPC_VERSION_MINOR,MPC_VERSION_PATCH,MPC_VERSION_REV
#define MPC_VERSION_STR       MAKE_STR(MPC_VERSION_MAJOR) ", " MAKE_STR(MPC_VERSION_MINOR) ", " MAKE_STR(MPC_VERSION_PATCH) ", " MAKE_STR(MPC_VERSION_REV)
#define MPC_VERSION_SHORT     MPC_VERSION_MAJOR,MPC_VERSION_MINOR,MPC_VERSION_PATCH,0
#define MPC_VERSION_SHORT_STR MAKE_STR(MPC_VERSION_MAJOR) ", " MAKE_STR(MPC_VERSION_MINOR) ", " MAKE_STR(MPC_VERSION_PATCH) ", 0"
#endif

#define DIRECTX_SDK_DATE      "June 2010"
#define DIRECTX_SDK_NUMBER    "43"


#endif
