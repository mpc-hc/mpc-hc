//************************************************************************
//  The Logitech LCD SDK, including all acompanying documentation,
//  is protected by intellectual property laws.  All use of the Logitech
//  LCD SDK is subject to the License Agreement found in the
//  "Logitech LCD SDK License Agreement" file and in the Reference Manual.  
//  All rights not expressly granted by Logitech are reserved.
//************************************************************************

//************************************************************************
//
// LCDUI.h
//
// Common header file for all LCDUI source files
// This file can be used to compile with pre-compiled headers
// under Visual Studio
//
// Logitech LCD SDK
//
// Copyright 2010 Logitech Inc.
//
//************************************************************************

#ifndef _LCDUI_H_INCLUDED_
#define _LCDUI_H_INCLUDED_

//************************************************************************
// all windows header files
//************************************************************************

#include <windows.h>
#include <tchar.h>
#include <vector>
#include <queue>
#include <Vfw.h>
#include <GdiPlus.h>


//************************************************************************
// all classes
//************************************************************************

class CLCDBase;
class CLCDCollection;
class CLCDPage;
class CLCDPopupBackground;
class CLCDConnection;
class CLCDOutput;
class CLCDGfxBase;
class CLCDGfxMono;
class CLCDGfxColor;
class CLCDText;
class CLCDColorText;
class CLCDScrollingText;
class CLCDStreamingText;
class CLCDPaginateText;
class CLCDIcon;
class CLCDBitmap;
class CLCDAnimatedBitmap;
class CLCDProgressBar;
class CLCDColorProgressBar;
class CLCDSkinnedProgressBar;


//************************************************************************
// all header files
//************************************************************************

#include <lglcd/lglcd.h>
#include "LCDBase.h"
#include "LCDCollection.h"
#include "LCDPage.h"
#include "LCDConnection.h"
#include "LCDOutput.h"
#include "LCDGfxBase.h"
#include "LCDGfxMono.h"
#include "LCDGfxColor.h"
#include "LCDText.h"
#include "LCDColorText.h"
#include "LCDScrollingText.h"
#include "LCDStreamingText.h"
#include "LCDPaginateText.h"
#include "LCDIcon.h"
#include "LCDBitmap.h"
#include "LCDAnimatedBitmap.h"
#include "LCDProgressBar.h"
#include "LCDColorProgressBar.h"
#include "LCDSkinnedProgressBar.h"


#endif //~_LCDUI_H_INCLUDED_

//** end of LCDUI.h ******************************************************
