/****************************************************************************
 * 
 *  Copyright (C) 1995-1999 RealNetworks, Inc. All rights reserved.
 *  
 *  This program contains proprietary information of RealNetworks, Inc.,
 *  and is licensed subject to restrictions on use and distribution.
 *
 *  rmaevent.h
 *
 */

#ifndef _RMAEVENT_H_
#define _RMAEVENT_H_

#define RMA_BASE_EVENT	0x00001000UL

// This class of events are events sent to site users of windowless
// sites to notify them of events on the site with platform independent
// messages.
#define RMA_SURFACE_EVENTS	RMA_BASE_EVENT + 0x00001000

// RMA_SURFACE_UPDATE is sent by the site to the renderer when the
// surface has damage and needs to be updated.  The event struct is
// filled out as follows:
//
//    ULONG32	event;		RMA_SURFACE_UPDATE
//    void*	window;	    Native Window - may be null if no window is associated with the site
//    void*	param1;	    IRMAVideoSurface*
//    void*	param2;	    UNIX - PNxWindow, Mac/Win - UNUSED
//
//    void*	result;	    HRESULT result code of message handling
//    BOOL	handled;    TRUE if handled, FALSE if not handled
//
#define RMA_SURFACE_UPDATE	RMA_SURFACE_EVENTS + 1

// Each event should document the values of PNxEvent struct expected
// on input and output.
// The next surface event should use this ID:
#define RMA_SURFACE_NEXT_EVENT RMA_SURFACE_EVENTS + 2

// This class of events are sent to site users to
// notify them of mouse events.
// All mouse events have the event structure filled out as follows:
//
//    UINT32	event;
//    void*	window;
//    void*	param1;	    PNxPoint struct with mouse position local to the renderer
//    void*	param2;	    UINT32 of flags for modifier keys
//    void*	result;	    HRESULT result code of message handling
//    BOOL	handled;    TRUE if handled, FALSE if not handled
//
#define RMA_MOUSE_EVENTS	(RMA_BASE_EVENT + 0x00002000)

#define RMA_SHIFT_KEY		0x00000001
#define RMA_CTRL_KEY		0x00000002
#define RMA_ALT_COMMAND_KEY	0x00000004	// Apple/Splat key

#define RMA_PRIMARY_BUTTON_DOWN	(RMA_MOUSE_EVENTS + 1)
#define RMA_PRIMARY_BUTTON_UP	(RMA_MOUSE_EVENTS + 2)
#define RMA_CONTEXT_BUTTON_DOWN	(RMA_MOUSE_EVENTS + 3)
#define RMA_CONTEXT_BUTTON_UP	(RMA_MOUSE_EVENTS + 4)
#define RMA_MOUSE_MOVE		(RMA_MOUSE_EVENTS + 5)
#define RMA_MOUSE_ENTER		(RMA_MOUSE_EVENTS + 6)
#define RMA_MOUSE_LEAVE		(RMA_MOUSE_EVENTS + 7)

// This class of events are sent to renderers to
// notify them of the validation of the window
// All window events have the event structure filled out as follows:
//
//    UINT32	event;
//    void*	window;
//    void*	UNUSED;
//    void*	UNUSED;
//    void*	result;	    HRESULT result code of message handling
//    BOOL	handled;    TRUE if handled, FALSE if not handled
//
#define RMA_WINDOW_EVENTS	RMA_BASE_EVENT + 0x00003000

#define RMA_ATTACH_WINDOW	RMA_WINDOW_EVENTS + 1
#define RMA_DETACH_WINDOW	RMA_WINDOW_EVENTS + 2

// Each event class should have a comment describing the kinds
// of events that belong to this class
// The next event class should use this base:
#define RMA_NEXT_EVENT_CLASS	RMA_BASE_EVENT + 0x00004000


#endif // _RMAEVENT_H_
