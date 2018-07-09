// ResizableMsgSupport.cpp: support messages for custom resizable wnds
//
/////////////////////////////////////////////////////////////////////////////
//
// This file is part of ResizableLib
// https://github.com/ppescher/resizablelib
//
// Copyright (C) 2000-2015 by Paolo Messina
// mailto:ppescher@hotmail.com
//
// The contents of this file are subject to the Artistic License 2.0
// http://opensource.org/licenses/Artistic-2.0
//
// If you find this code useful, credits would be nice!
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ResizableMsgSupport.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Registered message to communicate with the library

// static intializer must be called before user code
#pragma warning(disable:4073)
#pragma init_seg(lib)

const UINT WMU_RESIZESUPPORT = ::RegisterWindowMessage(TEXT("WMU_RESIZESUPPORT"));

