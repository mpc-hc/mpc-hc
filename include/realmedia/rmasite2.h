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

#ifndef _RMASITE2_H_
#define _RMASITE2_H_

/*
 * Forward declarations of some interfaces defined or used here-in.
 */

//typedef _INTERFACE  IRMASite2			    IRMASite2;
typedef _INTERFACE  IRMAVideoSurface		    IRMAVideoSurface;
typedef _INTERFACE  IRMAPassiveSiteWatcher	    IRMAPassiveSiteWatcher;

/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMASite2
 *
 *  Purpose:
 *
 *	Interface for IRMASite2 objects.
 *
 *  IID_IRMASite:
 *
 *	{0x00000D0A-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMASite2, 0x00000D0A, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMASite

DECLARE_INTERFACE_(IRMASite2, IUnknown)
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
     * IRMASite2 method usually called by the "context" 
     * when window attributes (like the window handle) have changed.
     */
    STDMETHOD(UpdateSiteWindow) (THIS_
				PNxWindow* /*IN*/ pWindow) PURE;

    /*
     * IRMASite2 method usually called by the "context" to
     * to hide/show a site.
     */
    STDMETHOD(ShowSite)         (THIS_
                                 BOOL    bShow) PURE;
                                 
    STDMETHOD_(BOOL, IsSiteVisible)         (THIS) PURE;

    /*
     * IRMASite2 method usually called by the "context" to
     * set the site's Z-order
     */
    STDMETHOD(SetZOrder)	(THIS_
				INT32 lZOrder
				) PURE;

    /*
     * IRMASite2 method called to get the site's Z-order
     */
    STDMETHOD(GetZOrder)	(THIS_
				REF(INT32) lZOrder
				) PURE;

    /*
     * IRMASite2 method called to set the site at the top
     * of the Z-order
     */
    STDMETHOD(MoveSiteToTop)	(THIS) PURE;

    /*
     * IRMASite2 method called to get the site's video surface
     */
    STDMETHOD(GetVideoSurface)	(THIS_ 
				REF(IRMAVideoSurface*) pSurface
				) PURE;

    /*
     * IRMASite2 method called to get the number of child sites.
     */
    STDMETHOD_(UINT32,GetNumberOfChildSites) (THIS) PURE;

    /*
     * IRMASite2 method to add a watcher that does not affect the site
     */
    STDMETHOD(AddPassiveSiteWatcher)	(THIS_
    					IRMAPassiveSiteWatcher* pWatcher
					) PURE;

    /*
     * IRMASite2 method to remove a watcher that does not affect the site
     */
    STDMETHOD(RemovePassiveSiteWatcher) (THIS_
    					IRMAPassiveSiteWatcher* pWatcher
					) PURE;

    /*
     * IRMASite2 method used to do cursor management
     */
    STDMETHOD(SetCursor) 		(THIS_
    					PNxCursor ulCursor,
					REF(PNxCursor) ulOldCursor
					) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAPassiveSiteWatcher
 *
 *  Purpose:
 *
 *	Interface for IRMAPassiveSiteWatcher objects.
 *
 *  IID_IRMAPassiveSiteWatcher:
 *
 *	{0x00000D0F-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAPassiveSiteWatcher, 0x00000D0F, 0x901, 0x11d1, 0x8b, 0x6, 
			0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAPassiveSiteWatcher

DECLARE_INTERFACE_(IRMAPassiveSiteWatcher, IUnknown)
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
     * IRMAPassiveSiteWatcher method used to notify
     * about position updates
     */
    STDMETHOD(PositionChanged) (THIS_
				PNxPoint* /*IN*/ pPoint) PURE;

    /*
     * IRMAPassiveSiteWatcher method used to notify
     * about size updates
     */
    STDMETHOD(SizeChanged) 	(THIS_
				PNxSize* /*IN*/ pSize) PURE;

};

#endif //_RMASITE2_H_
