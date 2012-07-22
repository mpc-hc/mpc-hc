/****************************************************************************
 * 
 *  Copyright (C) 1995-1999 RealNetworks, Inc. All rights reserved.
 *  
 *  http://www.real.com/devzone
 *
 *  This program contains proprietary 
 *  information of Progressive Networks, Inc, and is licensed
 *  subject to restrictions on use and distribution.
 *
 *
 *  Defines the public classes for cross platform windows used in various
 *  Progressive Networks modules.
 *
 */

#ifndef _PNWINTYP_H_
#define _PNWINTYP_H_

#include "pntypes.h" /* Needed at least for various defines and types. */

#ifdef _WIN16
#define BI_BITFIELDS	3L
#endif

/****************************************************************************
 * 
 *  Structure:
 *
 *	PNxSize
 *
 *  Purpose:
 *
 *	Cross Platform definition of a size.
 *
 */
typedef struct PNEXPORT_CLASS _PNxSize
{
    INT32   cx;
    INT32   cy;
} PNxSize;

/****************************************************************************
 * 
 *  Structure:
 *
 *	PNxPoint
 *
 *  Purpose:
 *
 *	Cross Platform definition of a point.
 *
 */
typedef struct PNEXPORT_CLASS _PNxPoint
{
    INT32   x;
    INT32   y;
} PNxPoint;

/****************************************************************************
 * 
 *  Structure:
 *
 *	PNxRect
 *
 *  Purpose:
 *
 *	Cross Platform definition of a rectangle.
 *
 */
typedef struct PNEXPORT_CLASS _PNxRect
{
    INT32   left;
    INT32   top;
    INT32   right;
    INT32   bottom;
} PNxRect;

#define PNxRECT_WIDTH(r)	((r).right - (r).left)
#define PNxRECT_HEIGHT(r)	((r).bottom - (r).top)

/****************************************************************************
 * 
 *  Structure:
 *
 *	PNxWindow
 *
 *  Purpose:
 *
 *	Cross Platform definition of a window. This struct is sufficiently
 *	wide to describe parent or child windows in Windows, MacOS, and
 *	various flavours of X-Windows.
 *
 *  Data Members:
 *
 *	void*	window
 *	platform specific window handle
 *	
 *	ULONG32	x, y
 *	position of top left corner relative to a client page
 *	
 *	ULONG32	width, height
 *	maximum window size
 *	
 *	PNxRect clipRect;            
 *	clipping rectangle in port coordinates
 *
 */
typedef struct PNEXPORT_CLASS _PNxWindow
{
	/* NOTE: The window parameter is NOT guaranteed to be unique for every
			 corresponding CPNWindow. Use PNxWindowID if this is desired. */
    void*	window;
    ULONG32	x;
    ULONG32	y;                   
    ULONG32	width;
    ULONG32	height;
    PNxRect	clipRect;
    #ifdef _UNIX
    void * display;
    #endif
} PNxWindow;

typedef void* PNxWindowID;

/****************************************************************************
 * 
 *  Structure:
 *
 *	PNxEvent
 *
 *  Purpose:
 *
 *	Cross Platform definition of a event. This struct is sufficiently
 *	wide to describe an event in Windows, MacOS, and various flavours of 
 *	X-Windows.
 *
 *  Data Members:
 *
 *	void*	event
 *	platform specific event ID, can also be one of the several PNxMSG_*
 *	event IDs which map onto existing platform specific event IDs
 *      UNIX: X Event Type
 *	
 *	void*	window
 *	platform specific window handle
 *      UNIX: X Window ID
 *	
 *	void*	param1
 *	message specific parameter
 *      UNIX: Display*
 *	
 *	void*	param2
 *      Mac:  for UpdateEvt, either NULL or RgnHandle to be filled with updated area
 *      UNIX: Native              XEvent*
 *            RMA_SURFACE_UPDATE  PNxWindow*
 *	
 */
typedef struct PNEXPORT_CLASS _PNxEvent
{
    ULONG32	event;	    /* IN  */
    void*	window;	    /* IN  */
    void*	param1;	    /* IN  */
    void*	param2;	    /* IN  */

    UINT32	result;	    /* OUT */
    BOOL	handled;    /* OUT */
} PNxEvent;


/****************************************************************************
 * 
 *  typedef:
 *
 *	PNxRegion
 *
 *  Purpose:
 *
 *	Cross Platform definition of a region. This typedef is redefined as
 *	appropriate to describe a region in Windows, MacOS, and various 
 *	flavours of X-Windows.
 *
 */
typedef void* PNxRegion;

/****************************************************************************
 * 
 *  typedef:
 *
 *	PNxDC
 *
 *  Purpose:
 *
 *	Cross Platform definition of a device context. This typedef is redefined as
 *	appropriate to describe a device context in Windows, MacOS, and various 
 *	flavours of X-Windows.
 *
 */
typedef void* PNxDC;

/****************************************************************************
 * 
 *  typedef:
 *
 *	PNxFont
 *
 *  Purpose:
 *
 *	Cross Platform definition of a font. This typedef is redefined as
 *	appropriate to describe a font in Windows, MacOS, and various 
 *	flavours of X-Windows.
 *
 */
typedef void* PNxFont;

/****************************************************************************
 * 
 *  typedef:
 *
 *	PNxColor
 *
 *  Purpose:
 *
 *	Cross Platform definition of a color. This typedef is redefined as
 *	appropriate to describe a font in Windows, MacOS, and various 
 *	flavours of X-Windows.
 *
 */
typedef ULONG32 PNxColor;

/****************************************************************************
 * 
 *  typedef:
 *
 *	PNxIcon
 *
 *  Purpose:
 *
 *	Cross Platform definition of a icon. This typedef is redefined as
 *	appropriate to describe a font in Windows, MacOS, and various 
 *	flavours of X-Windows.
 *
 */
typedef void* PNxIcon;

/****************************************************************************
 * 
 *  typedef:
 *
 *	PNxMenu
 *
 *  Purpose:
 *
 *	Cross Platform definition of a menu. This typedef is redefined as
 *	appropriate to describe a font in Windows, MacOS, and various 
 *	flavours of X-Windows.
 *
 */
typedef void* PNxMenu;

/****************************************************************************
 * 
 *  typedef:
 *
 *	PNxCursor
 *
 *  Purpose:
 *
 *	Cross Platform definition of a cursor. This typedef is redefined as
 *	appropriate to describe a cursor in Windows, MacOS, and various 
 *	flavours of X-Windows.
 *
 */
typedef void* PNxCursor;

#endif /* _PNWINTYP_H_ */
