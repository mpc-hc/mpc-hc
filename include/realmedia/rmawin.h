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
 *  RealMedia Architecture Simple Window Interfaces.
 *
 */

#ifndef _RMAWIN_H_
#define _RMAWIN_H_

/*
 * Forward declarations of some interfaces defined or used here-in.
 */
typedef _INTERFACE	IRMASite			IRMASite;
typedef _INTERFACE	IRMASiteUser			IRMASiteUser;
typedef _INTERFACE	IRMASiteWindowed		IRMASiteWindowed;
typedef _INTERFACE	IRMASiteEventHandler            IRMASiteEventHandler;
typedef _INTERFACE	IRMASiteWindowless		IRMASiteWindowless;
typedef _INTERFACE	IRMASiteWatcher			IRMASiteWatcher;
typedef _INTERFACE	IRMAValues			IRMAValues;
typedef _INTERFACE	IRMASiteFullScreen		IRMASiteFullScreen;
typedef _INTERFACE	IRMALayoutSiteGroupManager	IRMALayoutSiteGroupManager;
typedef _INTERFACE	IRMAEventHook			IRMAEventHook;

typedef struct _PNxWindow   PNxWindow;
typedef struct _PNxSize	    PNxSize;
typedef struct _PNxPoint    PNxPoint;
typedef struct _PNxRect	    PNxRect;
typedef void* PNxRegion;


/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMASiteWindowed
 *
 *  Purpose:
 *
 *	Interface for IRMASite objects which are associated with platform
 *	specific window objects on Microsoft Windows and X-Windows.
 *
 *  IID_IRMASiteWindowed:
 *
 *	{00000D01-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMASiteWindowed, 0x00000D01, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#define CLSID_IRMASiteWindowed	IID_IRMASiteWindowed


#undef  INTERFACE
#define INTERFACE   IRMASiteWindowed

DECLARE_INTERFACE_(IRMASiteWindowed, IUnknown)
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
     * IRMASiteWindowed methods called by site suppliers 
     * when they want the site associated with a
     * previously created (and externally managed) window.
     * This method will "sub-class" that window (Win32).
     * On Unix, the site supplier must pass events from 
     * the externally managed window to the core via
     * IRMAClientEngine::EventOccurred(). Please note that
     * The PNxWindow ptr must remain in scope for the life
     * of Site.
     * 
     */
    STDMETHOD(AttachWindow)	(THIS_
				PNxWindow* /*IN*/ pWindow) PURE;

    STDMETHOD(DetachWindow)	(THIS) PURE;

    /*
     * IRMASiteWindowed methods called by Owners of the site
     * in the event that want a default top level window created
     * for the site.
     */
    STDMETHOD(Create)		(THIS_
				void* ParentWindow, 
				UINT32 style) PURE;

    STDMETHOD(Destroy)		(THIS) PURE;

    /*
     * IRMASiteWindowed method. Returns actual window of the site.
     */
    STDMETHOD_(PNxWindow*,GetWindow)(THIS) PURE;
};



/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMASiteWindowless
 *
 *  Purpose:
 *
 *	Interface for IRMASite objects which are "windowless" or not 
 *	associated with platform specific window objects.
 *
 *  IID_IRMASiteWindowless:
 *
 *	{00000D02-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMASiteWindowless, 0x00000D02, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMASiteWindowless

#define CLSID_IRMASiteWindowless IID_IRMASiteWindowless

DECLARE_INTERFACE_(IRMASiteWindowless, IUnknown)
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
     * IRMASiteWindowless methods called by owners of the site.
     */
    STDMETHOD(EventOccurred)	(THIS_
				PNxEvent* /*IN*/ pEvent) PURE;

    /*
     * IRMASiteWindowless method. Returns some parent window that
     * owns the windowless site. Useful for right-click menus and
     * dialog box calls.
     */
    STDMETHOD_(PNxWindow*,GetParentWindow)(THIS) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMASite
 *
 *  Purpose:
 *
 *	Interface for IRMASite objects.
 *
 *  IID_IRMASite:
 *
 *	{00000D03-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMASite, 0x00000D03, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMASite

DECLARE_INTERFACE_(IRMASite, IUnknown)
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
     * IRMASite methods usually called by the "context" to 
     * associate users with the site, and to create child sites
     * as appropriate.
     */
    STDMETHOD(AttachUser)	(THIS_
				IRMASiteUser*	/*IN*/	pUser) PURE;

    STDMETHOD(DetachUser)	(THIS) PURE;


    STDMETHOD(GetUser)		(THIS_
				REF(IRMASiteUser*) /*OUT*/ pUser) PURE;

    STDMETHOD(CreateChild)	(THIS_
				REF(IRMASite*)	/*OUT*/ pChildSite) PURE;

    STDMETHOD(DestroyChild)	(THIS_
				IRMASite*	/*IN*/	pChildSite) PURE;

    /*
     * IRMASite methods called by the the "context" in which the site
     * is displayed in order to manage its position. Site users should
     * not generally call these methods.
     */
    STDMETHOD(AttachWatcher)	(THIS_
				IRMASiteWatcher* /*IN*/	pWatcher) PURE;

    STDMETHOD(DetachWatcher)	(THIS) PURE;

    STDMETHOD(SetPosition)	(THIS_
				PNxPoint		position) PURE;

    STDMETHOD(GetPosition)	(THIS_
				REF(PNxPoint)		position) PURE;

    /*
     * IRMASite methods called by the user of the site to get
     * information about the site, and to manipulate the site.
     */
    STDMETHOD(SetSize)		(THIS_
				PNxSize			size) PURE;

    STDMETHOD(GetSize)		(THIS_
				REF(PNxSize)		size) PURE;

    STDMETHOD(DamageRect)	(THIS_
				PNxRect			rect) PURE;

    STDMETHOD(DamageRegion)	(THIS_
				PNxRegion		region) PURE;

    STDMETHOD(ForceRedraw)	(THIS) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMASiteUser
 *
 *  Purpose:
 *
 *	Interface for the user of the IRMASite objects.
 *
 *  IID_IRMASiteUser:
 *
 *	{00000D04-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMASiteUser, 0x00000D04, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMASiteUser

DECLARE_INTERFACE_(IRMASiteUser, IUnknown)
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
     * IRMASiteUser methods usually called by the "context" to 
     * associate users with the site.
     */
    STDMETHOD(AttachSite)	(THIS_
				IRMASite*	/*IN*/ pSite) PURE;

    STDMETHOD(DetachSite)	(THIS) PURE;

    /*
     * IRMASiteUser methods called to inform user of an event.
     */
    STDMETHOD(HandleEvent)	(THIS_
				PNxEvent*	/*IN*/ pEvent) PURE;

    STDMETHOD_(BOOL,NeedsWindowedSites)	(THIS) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMASiteWatcher
 *
 *  Purpose:
 *
 *	Interface for IRMASiteWatcher objects.
 *
 *  IID_IRMASite:
 *
 *	{00000D05-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMASiteWatcher, 0x00000D05, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMASiteWatcher

DECLARE_INTERFACE_(IRMASiteWatcher, IUnknown)
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
     * IRMASiteWatcher methods called by the site when a watcher 
     * is attached to or detached from it.
     */
    STDMETHOD(AttachSite)	(THIS_
				IRMASite*	/*IN*/	pSite) PURE;

    STDMETHOD(DetachSite)	(THIS) PURE;

    /*
     * IRMASiteWatcher methods called by the site an attempt is
     * made to change it's position or size. The site watcher must
     * return PNR_OK for the change to occur. If the site watcher
     * returns any value other than PNR_OK then the size or position
     * will not change. The site watcher can also modify the new
     * size of position.
     */
    STDMETHOD(ChangingPosition)	(THIS_
				PNxPoint		posOld,
				REF(PNxPoint)/*IN-OUT*/	posNew) PURE;

    STDMETHOD(ChangingSize)	(THIS_
				PNxSize			sizeOld,
				REF(PNxSize) /*IN-OUT*/	sizeNew) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMASiteUserSupplier
 *
 *  Purpose:
 *
 *	Interface implemented by renderers and objects with provide layouts to
 *	the client core. This interface is called by the core when it needs a 
 *	new IRMASiteUser, or when it is done using an IRMASiteUser.
 *
 *  IID_IRMASiteUserSupplier:
 *
 *	{00000D06-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMASiteUserSupplier, 0x00000D06, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMASiteUserSupplier

DECLARE_INTERFACE_(IRMASiteUserSupplier, IUnknown)
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
     * IRMASiteUserSupplier methods usually called by the 
     * "context" to ask for additional or to release previously
     * created site users.
     */
    STDMETHOD(CreateSiteUser)	(THIS_
				REF(IRMASiteUser*)/*OUT*/ pSiteUser) PURE;

    STDMETHOD(DestroySiteUser)	(THIS_
				IRMASiteUser*	  /*IN*/ pSiteUser) PURE;

    STDMETHOD_(BOOL,NeedsWindowedSites)	(THIS) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMASiteSupplier
 *
 *  Purpose:
 *
 *	Interface implemented by users of the client core. This interface is
 *	called by the core when it needs a new IRMASite, or when it is done
 *	using an IRMASite.
 *
 *  IID_IRMASiteSupplier:
 *
 *	{00000D07-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMASiteSupplier, 0x00000D07, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMASiteSupplier

DECLARE_INTERFACE_(IRMASiteSupplier, IUnknown)
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
     * IRMASiteSupplier methods
     */

    /************************************************************************
     *	Method:
     *	    IRMASiteSupplier::SitesNeeded
     *	Purpose:
     *	  Called to inform the site supplier that a site with a particular
     *	  set of characteristics is needed. If the site supplier can 
     *	  fulfill the request it should call the site manager and add one
     *	  or more new sites.
     *    Note that the request for sites is associated with a Request ID
     *    the client core will inform the site supplier when this requested
     *    site is no longer needed.
     */
    STDMETHOD(SitesNeeded)	(THIS_
				UINT32			uReqestID,
				IRMAValues*		pSiteProps) PURE;

    /************************************************************************
     *  Method:
     *    IRMASiteSupplier::SitesNotNeeded
     *  Purpose:
     *    Called to inform the site supplier that all sites from a previos
     *	  site request are no longer needed. If the site supplier had 
     *	  previously created non-persistant sites (like popup windows)
     *    to fulfill a request for sites it should call the site manager 
     *    and remove those sites.
     */
    STDMETHOD(SitesNotNeeded)	(THIS_
				UINT32			uReqestID) PURE;


    /************************************************************************
     *  Method:
     *    IRMASiteSupplier::BeginChangeLayout
     *  Purpose:
     *    Called to inform the site supplier a layout change has beginning
     *	  it can expect to recieve SitesNeeded() and SitesNotNeeded() calls
     *	  while a layout change is in progress,
     */
    STDMETHOD(BeginChangeLayout) (THIS) PURE;

    /************************************************************************
     *  Method:
     *    IRMASiteSupplier::DoneChangeLayout
     *  Purpose:
     *    Called to inform the site supplier the layout change has been
     *	  completed.
     */
    STDMETHOD(DoneChangeLayout) (THIS) PURE;

};


/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMASiteManager
 *
 *  Purpose:
 *
 *	Interface implemented by the client core. This interface is called
 *	by users of the client core to inform it of IRMASite's which are
 *	available for layout of renderers
 *
 *  IID_IRMASiteManager:
 *
 *	{00000D08-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMASiteManager, 0x00000D08, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMASiteManager

DECLARE_INTERFACE_(IRMASiteManager, IUnknown)
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
     * IRMASiteManager methods
     */

    /************************************************************************
     *	Method:
     *	    IRMASiteManager::AddSite
     *	Purpose:
     *	  Called to inform the site manager of the existance of a site.
     */
    STDMETHOD(AddSite)		(THIS_
				IRMASite*		pSite) PURE;

    /************************************************************************
     *	Method:
     *	    IRMASiteManager::RemoveSite
     *	Purpose:
     *	  Called to inform the site manager that a site is no longer 
     *	  available.
     */
    STDMETHOD(RemoveSite)	(THIS_
				IRMASite*		pSite) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAMultiInstanceSiteUserSupplier
 *
 *  Purpose:
 *
 *	This is the interface for a special default object which is available
 *	from the common class factory. This object will act as a site user 
 *	supplier for any renderer (or other site user object) that wants
 *	default support for multiple instances. The site user must work as
 *	a windowless site for this default implementation to work. The 
 *	default object also implements the IRMASite interfave to allow
 *	the site user object to control all the sites through a single 
 *	interface instance.
 *
 *  IID_IRMAMultiInstanceSiteUserSupplier:
 *
 *	{00000D09-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAMultiInstanceSiteUserSupplier, 0x00000D09, 0x901, 
            0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#define CLSID_IRMAMultiInstanceSiteUserSupplier	\
		IID_IRMAMultiInstanceSiteUserSupplier

#undef  INTERFACE
#define INTERFACE   IRMAMultiInstanceSiteUserSupplier

DECLARE_INTERFACE_(IRMAMultiInstanceSiteUserSupplier, IUnknown)
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
     * IRMAMultiInstanceSiteUserSupplier methods called by site users.
     */
    STDMETHOD(SetSingleSiteUser)		(THIS_ 
						IUnknown*	pUnknown) PURE;

    STDMETHOD(ReleaseSingleSiteUser)		(THIS) PURE;

};

/****************************************************************************
 * 
 *  Interface:
 *	IRMASiteFullScreen
 *
 *  Purpose:
 *
 *	This is the interface for turning on/off the full screen mode
 *
 *  IID_IRMASiteFullScreen:
 *
 *	{00000D0B-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMASiteFullScreen, 0x00000D0B, 0x901, 
            0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMASiteFullScreen

DECLARE_INTERFACE_(IRMASiteFullScreen, IUnknown)
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
     * IRMASiteFullScreen methods
     */
    STDMETHOD(EnterFullScreen)	(THIS) PURE;

    STDMETHOD(ExitFullScreen)	(THIS) PURE;

    STDMETHOD(TestFullScreen)	(THIS_
				void* hTestBitmap,const char* pszStatusText) PURE;

    STDMETHOD_(BOOL, IsFullScreen) (THIS) PURE;
};




/****************************************************************************
 * 
 *  Interface:
 *	IRMAEventHookMgr
 *
 *  Purpose:
 *
 *	Add ability to hook events from a named region
 *
 *  IID_IRMAEventHookMgr:
 *
 *	{00000D0D-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAEventHookMgr, 0x00000D0D, 0x901, 
            0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAEventHookMgr

DECLARE_INTERFACE_(IRMAEventHookMgr, IUnknown)
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
     * IRMAEventHookMgr methods
     */
    STDMETHOD(AddHook)		(THIS_
				IRMAEventHook* pHook,
				const char* pRegionName,
				UINT16 uLayer) PURE;

    STDMETHOD(RemoveHook)	(THIS_
    				IRMAEventHook* pHook,
				const char* pRegionName,
				UINT16 uLayer) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 *	IRMAEventHook
 *
 *  Purpose:
 *
 *	Object that gets hooked events sent by IRMAEventHookMgr
 *
 *  IID_IRMAEventHookMgr:
 *
 *	{00000D0E-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAEventHook, 0x00000D0E, 0x901, 
            0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAEventHook

DECLARE_INTERFACE_(IRMAEventHook, IUnknown)
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
     * IRMAEventHook methods
     */
    STDMETHOD(SiteAdded)	(THIS_
				IRMASite* pSite) PURE;
    STDMETHOD(HandleEvent)	(THIS_
    				IRMASite* pSite,
				PNxEvent* pEvent) PURE;
    STDMETHOD(SiteRemoved)	(THIS_
    				IRMASite* pSite) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 *	IRMAStatusMessage
 *
 *  Purpose:
 *
 *	This is the interface for setting the status text. 
 *
 *  IID_IRMAStatusMessage:
 *
 *	{00000D10-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAStatusMessage, 0x00000D10, 0x901, 
            0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAStatusMessage

DECLARE_INTERFACE_(IRMAStatusMessage, IUnknown)
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
     * IRMAStatusMessage methods
     */

    STDMETHOD(SetStatus)		(THIS_ const char* pText) PURE;
};


#endif /* _RMAWIN_H_ */
