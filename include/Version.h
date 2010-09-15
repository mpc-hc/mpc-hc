#pragma once

#ifndef NO_VERSION_REV_NEEDED
  #include "Version_rev.h"
#endif

#define DO_MAKE_STR(x) #x
#define MAKE_STR(x) DO_MAKE_STR(x)

#define VERSION_MAJOR 1
#define VERSION_MINOR 4
#define VERSION_PATCH 0

// The date of the DirectX SDK used for compilation used in the error message
// in mpc-hc and in the installer when the DirectX runtime is out of date
#define DIRECTX_SDK_DATE "June 2010"
