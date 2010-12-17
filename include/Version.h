// Notes:
// * Do not use /* */ since ISPP is buggy and it will throw an error.
// * NO_VERSION_REV_NEEDED is defined in those cases where we don't need the revision
//   number, but only the major/minor version so the compiler does not rebuild everything.
//   It's used in mpcresources and mpciconlib projects.
// * ISPP_IS_BUGGY is defined in the installer script only and it's just a workaround
//   for ISPP being buggy and throwing an error.
// * DIRECTX_SDK_DATE is the date of the DirectX SDK used for compilation and it's used 
//   in the error message in mpc-hc and in the installer when the DX runtime is out of date.

#pragma once

#ifndef NO_VERSION_REV_NEEDED
  #include "Version_rev.h"
#endif

#define DO_MAKE_STR(x) #x
#define MAKE_STR(x) DO_MAKE_STR(x)

#define MPC_VERSION_MAJOR 1
#define MPC_VERSION_MINOR 4
#define MPC_VERSION_PATCH 0

#define MPC_COMP_NAME        "MPC-HC Team"
#define MPC_COPYRIGHT        "Copyright (C) 2002-2010 see AUTHORS file"

#ifndef ISPP_IS_BUGGY
#define MPC_VERSION           MPC_VERSION_MAJOR,MPC_VERSION_MINOR,MPC_VERSION_REV,MPC_VERSION_PATCH
#define MPC_VERSION_STR       MAKE_STR(MPC_VERSION_MAJOR) ", " MAKE_STR(MPC_VERSION_MINOR) ", " MAKE_STR(MPC_VERSION_REV) ", " MAKE_STR(MPC_VERSION_PATCH)
#define MPC_VERSION_SHORT     MPC_VERSION_MAJOR,MPC_VERSION_MINOR,0,0
#define MPC_VERSION_SHORT_STR MAKE_STR(MPC_VERSION_MAJOR) ", " MAKE_STR(MPC_VERSION_MINOR) ", 0, 0"
#endif

#define DIRECTX_SDK_DATE      "June 2010"
