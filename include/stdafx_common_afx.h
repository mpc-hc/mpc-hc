// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#if (_MSC_VER < 1600)
#include <afx.h>
#endif

#include <afxwin.h>         // MFC core and standard components

#if (_MSC_VER >= 1600)
#include "afxpriv.h"
#include "afxole.h"
#include "..\src\mfc\oleimpl2.h"
#include "..\src\mfc\afximpl.h"
#endif
