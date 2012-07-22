/****************************************************************************
 * 
 *  Copyright (C) 1995-1999 RealNetworks, Inc. All rights reserved.
 *
 *  http://www.real.com/devzone
 *
 *  This program contains proprietary information of RealNetworks, Inc, 
 *  and is licensed subject to restrictions on use and distribution.
 *
 *
 *  RealMedia Architecture Context Menu Interfaces.
 *
 */

#ifndef _RMACMENU_H_
#define _RMACMENU_H_

/*
 * Forward declarations of some interfaces defined or used here-in.
 */
typedef _INTERFACE	IRMAContextMenu		    IRMAContextMenu;
typedef _INTERFACE	IRMAContextMenuResponse	    IRMAContextMenuResponse;


/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAContextMenu
 *
 *  Purpose:
 *
 *	Interface implemented by top level clients and provided to renderers.
 *	Allows the renderer to show a context menu and the top level client
 *	to add client specitic commands unknown to the renderer to that menu.
 *
 *  IID_IRMAContextMenu:
 *
 *	{00001f00-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAContextMenu, 0x00001f00, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAContextMenu

DECLARE_INTERFACE_(IRMAContextMenu, IUnknown)
{
    /*
     * IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /*
     * IRMAContextMenu methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAContextMenu::InitContextMenu
     *	Purpose:
     *	    Initializes the context menu to a blank menu, and sets the name
     *	    of the "sub menu" for the renderer if appropriate. This will 
     *	    clear any previously added menu items and sub menus.
     */
    STDMETHOD(InitContextMenu)	(THIS_
				const char*	pMenuText
				) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAContextMenu::AddMenuItem
     *	Purpose:
     *	    Returns information vital to the instantiation of rendering 
     *	    plugins.
     */
    STDMETHOD(AddMenuItem)	(THIS_
				UINT16		commandID, 
				const char*	pMenuItemText, 
				BOOL		bChecked,
				BOOL		bRadioOn, 
				BOOL		bDisabled
				) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAContextMenu::AddMenuItem
     *	Purpose:
     *	    Returns information vital to the instantiation of rendering 
     *	    plugins.
     */
    STDMETHOD(AddSeparator)	(THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAContextMenu::AddChildMenu
     *	Purpose:
     *	    Returns information vital to the instantiation of rendering 
     *	    plugins.
     */
    STDMETHOD(AddChildMenu)	(THIS_
				const char*	pMenuText
				) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAContextMenu::EndChildMenu
     *	Purpose:
     *	    Returns information vital to the instantiation of rendering 
     *	    plugins.
     */
    STDMETHOD(EndChildMenu)	(THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAContextMenu::ChangeMenuItem
     *	Purpose:
     *	    Returns information vital to the instantiation of rendering 
     *	    plugins.
     */
    STDMETHOD(ChangeMenuItem)	(THIS_
				UINT16		commandID, 
				const char*	pMenuItemText, 
				BOOL		bChecked,
				BOOL		bRadioOn, 
				BOOL		bDisabled
				) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAContextMenu::ShowMenu
     *	Purpose:
     *	    Shows the setup context menu at the specified point.
     */
    STDMETHOD(ShowMenu)		(THIS_
				IRMAContextMenuResponse* pResonse,
				PNxPoint ptPopup
				) PURE;

};

/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAContextMenuResponse
 *
 *  Purpose:
 *
 *	Interface implemented by renderers that use the context menut.
 *	Is called to inform the renderer that a particular menu item was
 *	chosen.
 *
 *  IRMAContextMenuResponse:
 *
 *	{00001f01-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAContextMenuResponse, 0x00001f01, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);


#undef  INTERFACE
#define INTERFACE   IRMAContextMenuResponse

DECLARE_INTERFACE_(IRMAContextMenuResponse, IUnknown)
{
    /*
     * IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /*
     * IRMAContextMenuResponse methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAContextMenuResponse::OnCommand
     *	Purpose:
     *	    Called to inform the renderer that a command was chosen from
     *	    the context menu.
     */
    STDMETHOD(OnCommand)	(THIS_
				UINT16 commandID
				) PURE;


    /************************************************************************
     *	Method:
     *	    IRMAContextMenuResponse::OnCanceled
     *	Purpose:
     *	    Called to inform the renderer that the context menu was closed
     *	    without a command being chosen from the renders set of commands.
     */
    STDMETHOD(OnCanceled)	(THIS) PURE;

};

#endif /* _RMACMENU_H_ */
