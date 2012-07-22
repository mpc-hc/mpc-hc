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
 *  Auto Upgrade Interfaces
 *
 */

#ifndef _RMAUPGRD_H
#define _RMAUPGRD_H


typedef _INTERFACE IRMABuffer IRMABuffer;


/* Enumeration for the upgrade types */
typedef enum _RMAUpgradeType
{
    eUT_Required,
    eUT_Recommended,
    eUT_Optional
} RMAUpgradeType;


/****************************************************************************
 * 
 *  Interface:
 *
 *  	IRMAUpgradeCollection
 *
 *  Purpose:
 *
 *	Interface provided by the Context. This interface allows collection 
 *	of upgrade components by the client core and it's delegates 
 *	(i.e. renderer plugins etc.)
 *
 *  IID_IRMAUpgradeCollection
 *
 *	{00002500-0901-11d1-8B06-00A024406D59}
 *
 */
 
DEFINE_GUID(IID_IRMAUpgradeCollection, 
	    0x00002500, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAUpgradeCollection

DECLARE_INTERFACE_(IRMAUpgradeCollection, IUnknown)
{
    /*
     * IRMAUpgradeCollection methods
     */

    /************************************************************************
     *	Method:
     *		IRMAUpgradeCollection::Add
     *	Purpose:
     *		Adds the specified upgrade information to the collection
     *
     */
    STDMETHOD_(UINT32, Add)	(THIS_ 
    				RMAUpgradeType upgradeType,
				IRMABuffer* pPluginId,
				UINT32 majorVersion,
				UINT32 minorVersion) PURE;

    /************************************************************************
     *	Method:
     *		IRMAUpgradeCollection::Remove
     *	Purpose:
     *		Remove the specified item from the collection
     *
     */
    STDMETHOD(Remove)		(THIS_ 
    				UINT32 index) PURE;

    /************************************************************************
     *	Method:
     *		IRMAUpgradeCollection::RemoveAll
     *	Purpose:
     *		Remove all items from the collection
     *
     */
    STDMETHOD(RemoveAll)    	(THIS) PURE;

    /************************************************************************
     *	Method:
     *		IRMAUpgradeCollection::GetCount
     *	Purpose:
     *		get the count of the collection
     *
     */
    STDMETHOD_(UINT32, GetCount)(THIS) PURE;

    /************************************************************************
     *	Method:
     *		IRMAUpgradeCollection::GetAt
     *	Purpose:
     *		get the specified items upgrade information
     *
     */
    STDMETHOD(GetAt)		(THIS_ 
    				UINT32 index,
				REF(RMAUpgradeType) upgradeType,
				IRMABuffer* pPluginId,
				REF(UINT32) majorVersion, 
				REF(UINT32) minorVersion) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 *
 *  	IRMAUpgradeHandler
 *
 *  Purpose:
 *
 *	Interface provided by the top-level client application.  This
 *	interface allows the client core to request an upgrade.
 *
 *  IID_IRMAUpgradeHandler:
 *
 *	{00002501-0901-11d1-8B06-00A024406D59}
 *
 */
 
DEFINE_GUID(IID_IRMAUpgradeHandler, 
                        0x00002501, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAUpgradeHandler

DECLARE_INTERFACE_(IRMAUpgradeHandler, IUnknown)
{
    /*
     * IRMAUpgradeHandler methods
     */

    /************************************************************************
     *	Method:
     *		IRMAUpgradeHandler::RequestUpgrade
     *	Purpose:
     *		Ask if user wants to do an upgrade and start an upgrade
     *
     */
    STDMETHOD(RequestUpgrade) (THIS_ IRMAUpgradeCollection* pComponents,
				     BOOL bBlocking) PURE;

    /************************************************************************
     *	Method:
     *		IRMAUpgradeHandler::HasComponents
     *	Purpose:
     *		Check if required components are present on the system.
     *  Returns:
     *		PNR_OK - components are here, no upgrade required;
     *			 all components are removed from pComponents.
     *          PNR_FAIL - some components are missing;
     *                   pComponents contains only those components 
     *			 that need upgrade.
     *
     */			      
    STDMETHOD(HasComponents)  (THIS_ IRMAUpgradeCollection* pComponents) PURE;
};



#endif /* _RMAUPGRD_H */
 
