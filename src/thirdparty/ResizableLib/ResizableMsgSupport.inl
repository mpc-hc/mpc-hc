// ResizableMsgSupport.inl: some definitions to support custom resizable wnds
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2000-2002 by Paolo Messina
// (http://www.geocities.com/ppescher - ppescher@yahoo.com)
//
// The contents of this file are subject to the Artistic License (the "License").
// You may not use this file except in compliance with the License. 
// You may obtain a copy of the License at:
// http://www.opensource.org/licenses/artistic-license.html
//
// If you find this code useful, credits would be nice!
//
/////////////////////////////////////////////////////////////////////////////


// registered message to communicate with the library
// (defined so that in the same executable it is initialized only once)
const UINT WMU_RESIZESUPPORT = ::RegisterWindowMessage(_T("WMU_RESIZESUPPORT"));

// if the message is implemented the returned value must be non-zero
// the default window procedure returns zero for unhandled messages

// wParam is one of the following RSZSUP_* values, lParam as specified

#define RSZSUP_QUERYPROPERTIES	101	// lParam = LPRESIZEPROPERTIES

#define RSZSUP_LIKESCLIPPING	102	// lParam = LPCLIPPINGPROPERTY

#define RSZSUP_NEEDSREFRESH		103	// lParam = LPREFRESHPROPERTY


/////////////////////////////////////////////////////////////////////////////
// utility functions

inline BOOL Send_QueryProperties(HWND hWnd, LPRESIZEPROPERTIES pResizeProperties)
{
	ASSERT(::IsWindow(hWnd));
	return (0 != SendMessage(hWnd, WMU_RESIZESUPPORT,
		RSZSUP_QUERYPROPERTIES, (LPARAM)pResizeProperties));
}

inline BOOL Send_LikesClipping(HWND hWnd, LPCLIPPINGPROPERTY pClippingProperty)
{
	ASSERT(::IsWindow(hWnd));
	return (0 != SendMessage(hWnd, WMU_RESIZESUPPORT,
		RSZSUP_LIKESCLIPPING, (LPARAM)pClippingProperty));
}

inline BOOL Send_NeedsRefresh(HWND hWnd, LPREFRESHPROPERTY pRefreshProperty)
{
	ASSERT(::IsWindow(hWnd));
	return (0 != SendMessage(hWnd, WMU_RESIZESUPPORT,
		RSZSUP_NEEDSREFRESH, (LPARAM)pRefreshProperty));
}
