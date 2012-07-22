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
 *  RealMedia Architecture Registry Interfaces.
 *
 */

#ifndef _RMAMON_H_
#define _RMAMON_H_

typedef _INTERFACE	IUnknown			IUnknown;
typedef _INTERFACE	IRMAPlugin			IRMAPlugin;
typedef _INTERFACE	IRMABuffer			IRMABuffer;
typedef _INTERFACE	IRMAValues			IRMAValues;
typedef _INTERFACE	IRMAPropWatch			IRMAPropWatch;
typedef _INTERFACE	IRMAPropWatchResponse		IRMAPropWatchResponse;
typedef _INTERFACE	IRMAActiveRegistry		IRMAActiveRegistry;
typedef _INTERFACE	IRMAActivePropUser		IRMAActivePropUser;
typedef _INTERFACE	IRMAActivePropUserResponse	IRMAActivePropUserResponse;
typedef _INTERFACE	IRMAPNRegistryAltStringHandling	IRMAPNRegistryAltStringHandling;

/*
 * Types of the values stored in the registry.
 */
typedef enum _RMAPropType
{
    PT_UNKNOWN,
    PT_COMPOSITE,	/* Contains other values (elements)		     */
    PT_INTEGER,		/* 32-bit signed value				     */
    PT_INTREF,		/* Integer reference object -- 32-bit signed integer */
    PT_STRING,		/* Signed char* value				     */
    PT_BUFFER		/* IRMABuffer object				     */
} RMAPropType;


/*
 * 
 *  Interface:
 *
 *	IRMAPNRegistry
 *
 *  Purpose:
 *
 *	This inteface provides access to the "Registry" in the server and
 *	client.  The "Registry" is a hierarchical structure of Name/Value
 *	pairs (properties) which is capable of storing many different types
 *	of data including strings, buffers, and integers.  The registry
 *	provides various types of information including statistics,
 *	configuration information, and system status.
 *
 *	Note:  This registry is not related to the Windows system registry.
 *
 *  IID_IRMAPNRegistry:
 *
 *	{00000600-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAPNRegistry, 0x00000600, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#define CLSID_IRMAPNRegistry	IID_IRMAPNRegistry

#undef  INTERFACE
#define INTERFACE   IRMAPNRegistry

DECLARE_INTERFACE_(IRMAPNRegistry, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /*
     *	IRMAPNRegistry methods
     */

    /************************************************************************
     *  Method:
     *      IRMAPNRegistry::CreatePropWatch
     *  Purpose:
     *      Create a new IRMAPropWatch object which can then be queried for 
     *  the right kind of IRMAPropWatch object.
     *
     *  pPropWatch - OUT - returns a new addref'ed IRMAPropWatch object 
     */
    STDMETHOD(CreatePropWatch)		(THIS_
					REF(IRMAPropWatch*) pPropWatch) PURE;

    /************************************************************************
     *  Method:
     *      IRMAPNRegistry::AddComp
     *  Purpose:
     *      Add a COMPOSITE property to the registry and return its ID
     *  if successful. It returns ZERO (0) if an error occurred
     *  during the operation.
     *
     *  pName - IN - name of the Property that is going to be added to 
     *               the registry
     */
    STDMETHOD_(UINT32, AddComp)		(THIS_
					const char*	pName) PURE;

    /************************************************************************
     *  Method:
     *      IRMAPNRegistry::AddInt
     *  Purpose:
     *      Add an INTEGER property with name in "pName" and value in 
     *  "iValue" to the registry. The return value is the id to
     *  the newly added Property or ZERO if there was an error.
     *
     *  pName - IN - name of the Property that is going to be added to 
     *               the registry
     *  nValue - IN - integer value of the Property that is going to be 
     *                added to the registry
     */
    STDMETHOD_(UINT32, AddInt)		(THIS_
					const char*	pName, 
					const INT32	nValue) PURE;

    /************************************************************************
     *  Method:
     *      IRMAPNRegistry::GetIntByName
     *  Purpose:
     *      Retreive an INTEGER value from the registry given its Property
     *  name "pName". If the Property is found, it will return PNR_OK, 
     *  otherwise it returns PNR_FAIL.
     *
     *  pName - IN - name of the Property whose value is to be retreived
     *  nValue - OUT - parameter into which the value of the Property is 
     *                 going to be returned
     */
    STDMETHOD(GetIntByName)		(THIS_
					const char*	pName,
					REF(INT32)	nValue) const PURE;

    /************************************************************************
     *  Method:
     *      IRMAPNRegistry::GetIntById
     *  Purpose:
     *      Retreive an INTEGER value from the registry given its id "ulId". 
     *  If the Property is found, it will return PNR_OK, otherwise it 
     *  returns PNR_FAIL.
     *
     *  ulId - IN - unique id of the Property whose value is to be retreived
     *  nValue - OUT - parameter into which the value of the Property is 
     *                 going to be returned
     */
    STDMETHOD(GetIntById)		(THIS_
					const UINT32	ulId,
					REF(INT32)	nValue) const PURE;

    /************************************************************************
     *  Method:
     *      IRMAPNRegistry::SetIntByName
     *  Purpose:
     *      Modify a Property's INTEGER value in the registry given the
     *  Property's name "pName". If the value was set, it will return PNR_OK, 
     *  otherwise it returns PNR_FAIL.
     *
     *  pName - IN - name of the Property whose value is to be set
     *  nValue - IN - the new value of the Property which is going to be set
     */
    STDMETHOD(SetIntByName)		(THIS_
					const char*	pName, 
					const INT32	nValue) PURE;

    /************************************************************************
     *  Method:
     *      IRMAPNRegistry::SetIntById
     *  Purpose:
     *      Modify a Property's INTEGER value in the registry given the
     *  its id "id". If the value was set, it will return PNR_OK, otherwise 
     *  it returns PNR_FAIL.
     *
     *  ulId - IN - unique id of the Property whose value is to be set
     *  nValue - IN - the new value of the Property which is going to be set
     */
    STDMETHOD(SetIntById)		(THIS_
					const UINT32	id,
					const INT32	nValue) PURE;

    /************************************************************************
     *  Method:
     *      IRMAPNRegistry::AddStr
     *  Purpose:
     *      Add an STRING property with name in "pName" and value in 
     *  "pValue" to the registry.
     *
     *  pName - IN - name of the Property that is going to be added to 
     *               the registry
     *  pValue - IN - buffer value of the Property that is going to be 
     *                added to the registry
     */
    STDMETHOD_(UINT32, AddStr)		(THIS_
					const char*	pName, 
					IRMABuffer*	pValue) PURE;

    /************************************************************************
     *  Method:
     *      IRMAPNRegistry::GetStrByName
     *  Purpose:
     *      Retreive an STRING value from the registry given its Property
     *  name "pName". If the Property is found, it will return PNR_OK, 
     *  otherwise it returns PNR_FAIL.
     *
     *  pName - IN - name of the Property whose value is to be retreived
     *  pValue - OUT - parameter into which the value of the Property is 
     *                 going to be returned
     */
    STDMETHOD(GetStrByName)		(THIS_
					const char*	 pName,
					REF(IRMABuffer*) pValue) const PURE;

    /************************************************************************
     *  Method:
     *      IRMAPNRegistry::GetStrById
     *  Purpose:
     *      Retreive an STRING value from the registry given its id "ulId". 
     *  If the Property is found, it will return PNR_OK, otherwise it 
     *  returns PNR_FAIL.
     *
     *  ulId - IN - unique id of the Property whose value is to be retreived
     *  pValue - OUT - parameter into which the value of the Property is 
     *                 going to be returned
     */
    STDMETHOD(GetStrById)		(THIS_
					const UINT32	 ulId,
					REF(IRMABuffer*) pValue) const PURE;

    /************************************************************************
     *  Method:
     *      IRMAPNRegistry::SetStrByName
     *  Purpose:
     *      Modify a Property's STRING value in the registry given the
     *  Property's name "pName". If the value was set, it will return 
     *  PNR_OK, otherwise it returns PNR_FAIL.
     *
     *  pName - IN - name of the Property whose value is to be set
     *  pValue - IN - the new value of the Property which is going to be set
     */
    STDMETHOD(SetStrByName)		(THIS_
					const char*	pName, 
					IRMABuffer*	pValue) PURE;

    /************************************************************************
     *  Method:
     *      IRMAPNRegistry::SetStrById
     *  Purpose:
     *      Modify a Property's STRING value in the registry given the
     *  its id "ulId". If the value was set, it will return PNR_OK, 
     *  otherwise it returns PNR_FAIL.
     *
     *  ulId - IN - unique id of the Property whose value is to be set
     *  pValue - IN - the new value of the Property which is going to be set
     */
    STDMETHOD(SetStrById)		(THIS_
					const UINT32	ulId,
					IRMABuffer*	pValue) PURE;

    /************************************************************************
     *  Method:
     *      IRMAPNRegistry::AddBuf
     *  Purpose:
     *      Add an BUFFER property with name in "pName" and value in 
     *  "pValue" to the registry.
     *
     *  pName - IN - name of the Property that is going to be added to 
     *               the registry
     *  pValue - IN - buffer value of the Property that is going to be 
     *                added to the registry
     */
    STDMETHOD_(UINT32, AddBuf)		(THIS_
					const char*	pName, 
					IRMABuffer*	pValue) PURE;

    /************************************************************************
     *  Method:
     *      IRMAPNRegistry::GetBufByName
     *  Purpose:
     *      Retreive the BUFFER from the registry given its Property name 
     *  "pName". If the Property is found, it will return PNR_OK, otherwise 
     *  it returns PNR_FAIL.
     *
     *  pName - IN - name of the Property whose value is to be retreived
     *  pValue - OUT - parameter into which the value of the Property is 
     *                 going to be returned
     */
    STDMETHOD(GetBufByName)		(THIS_
					const char*	 pName,
					REF(IRMABuffer*) pValue) const PURE;

    /************************************************************************
     *  Method:
     *      IRMAPNRegistry::GetBufById
     *  Purpose:
     *      Retreive the BUFFER from the registry given its id "ulId". If the 
     *  Property is found, it will return PNR_OK, otherwise it returns 
     *  PNR_FAIL.
     *
     *  ulId - IN - unique id of the Property whose value is to be retreived
     *  pValue - OUT - parameter into which the value of the Property is 
     *                 going to be returned
     */
    STDMETHOD(GetBufById)		(THIS_
					const UINT32	 ulId,
					REF(IRMABuffer*) pValue) const PURE;

    /************************************************************************
     *  Method:
     *      IRMAPNRegistry::SetBufByName
     *  Purpose:
     *      Modify a Property's BUFFER in the registry given the
     *  Property's name "pName". If the value was set, it will return 
     *  PNR_OK, otherwise it returns PNR_FAIL.
     *
     *  pName - IN - name of the Property whose value is to be set
     *  pValue - IN - the new value of the Property which is going to be set
     */
    STDMETHOD(SetBufByName)		(THIS_
					const char*	pName, 
					IRMABuffer*	pValue) PURE;

    /************************************************************************
     *  Method:
     *      IRMAPNRegistry::SetBufById
     *  Purpose:
     *      Modify a Property's BUFFER in the registry given its id "ulId". 
     *  If the value was set, it will return PNR_OK, otherwise it returns 
     *  PNR_FAIL.
     *
     *  ulId - IN - unique id of the Property whose value is to be set
     *  pValue - IN - the new value of the Property which is going to be set
     */
    STDMETHOD(SetBufById)		(THIS_
					const UINT32	ulId,
					IRMABuffer*	pValue) PURE;

    /************************************************************************
     *  Method:
     *      IRMAPNRegistry::AddIntRef
     *  Purpose:
     *      Add an INTEGER REFERENCE property with name in "pName" and 
     *  value in "iValue" to the registry. This property allows the user
     *  to modify its contents directly, without having to go through the
     *  registry.
     *
     *  pName - IN - name of the Property that is going to be added to 
     *               the registry
     *  pValue - IN - the pointer of the integer value is what gets stored
     *                in the registry as the Interger Reference Property's 
     *                value
     */
    STDMETHOD_(UINT32, AddIntRef)	(THIS_
					const char*	pName, 
					INT32*		pValue) PURE;

    /************************************************************************
     *  Method:
     *      IRMAPNRegistry::DeleteByName
     *  Purpose:
     *      Delete a Property from the registry using its name "pName".
     *
     *  pName - IN - name of the Property that is going to be deleted
     */
    STDMETHOD_(UINT32, DeleteByName)	(THIS_
					const char*	pName) PURE;

    /************************************************************************
     *  Method:
     *      IRMAPNRegistry::DeleteById
     *  Purpose:
     *      Delete a Property from the registry using its id "ulId".
     *
     *  ulId - IN - unique id of the Property that is going to be deleted
     */
    STDMETHOD_(UINT32, DeleteById)	(THIS_
					const UINT32	ulId) PURE;

    /************************************************************************
     *  Method:
     *      IRMAPNRegistry::GetTypeByName
     *  Purpose:
     *      Returns the datatype of the Property given its name "pName".
     *
     *  pName - IN - name of the Property whose type is to be retreived
     */
    STDMETHOD_(RMAPropType, GetTypeByName)	(THIS_
						const char* pName) const PURE;

    /************************************************************************
     *  Method:
     *      IRMAPNRegistry::GetTypeById
     *  Purpose:
     *      Returns the datatype of the Property given its its id "ulId".
     *
     *  ulId - IN - unique id of the Property whose type is to be retreived
     */
    STDMETHOD_(RMAPropType, GetTypeById)	(THIS_
						const UINT32 ulId) const PURE;

    /************************************************************************
     *  Method:
     *      IRMAPNRegistry::FindParentIdByName
     *  Purpose:
     *      Returns the id value of the parent node of the Property whose 
     *  name "pName" has been passed in. If it fails, a ZERO value is 
     *  returned.
     *
     *  pName - IN - name of the Property whose parent's unique id is to be
     *               retreived
     */
    STDMETHOD_(UINT32, FindParentIdByName)	(THIS_
						const char* pName) const PURE;

    /************************************************************************
     *  Method:
     *      IRMAPNRegistry::FindParentIdById
     *  Purpose:
     *      Returns the id value of the parent node of the Property whose 
     *  id "ulId" has been passed in. If it fails, a ZERO value is returned.
     *
     *  ulId - IN - unique id of the Property whose parent's id is to be
     *              retreived
     */
    STDMETHOD_(UINT32, FindParentIdById)	(THIS_
						const UINT32 ulId) const PURE;

    /************************************************************************
     *  Method:
     *      PNRegistry::GetPropName
     *  Purpose:
     *      Returns the Property name in the pName char buffer passed
     *  as a parameter, given the Property's id "ulId".
     *
     *  ulId - IN - unique id of the Property whose name is to be retreived
     *  pName - OUT - parameter into which the Property name is going to be
     *                returned
     */
    STDMETHOD(GetPropName)		(THIS_
					const UINT32 ulId,
					REF(IRMABuffer*) pName) const PURE;

    /************************************************************************
     *  Method:
     *      PNRegistry::GetId
     *  Purpose:
     *      Returns the Property's id given the Property name.
     *
     *  pName - IN - name of the Property whose unique id is to be 
     *               retreived
     */
    STDMETHOD_(UINT32, GetId)		(THIS_
					const char* pName) const PURE;

    /************************************************************************
     *  Method:
     *      IRMAPNRegistry::GetPropListOfRoot
     *  Purpose:
     *      Returns an array of a Properties under the root level of the 
     *  registry's hierarchy.
     *
     *  pValues - OUT - list of property name and unique id at the 
     *                  highest level (root) in the registry
     */
    STDMETHOD(GetPropListOfRoot) 	(THIS_
					REF(IRMAValues*) pValues) const PURE;

    /************************************************************************
     *  Method:
     *      IRMAPNRegistry::GetPropListByName
     *  Purpose:
     *      Returns an array of Properties immediately under the one whose
     *  name is passed in "pName".
     *
     *  pName - IN - name of the Property whose child property list is to be
     *               retreived
     *  pValues - OUT - list of property name and unique id under the 
     *                  Property whose name is in "pName"
     */
    STDMETHOD(GetPropListByName) 	(THIS_
					 const char* pName,
					 REF(IRMAValues*) pValues) const PURE;

    /************************************************************************
     *  Method:
     *      IRMAPNRegistry::GetPropListById
     *  Purpose:
     *      Returns an array of Properties immediately under the one whose
     *  id is passed in "ulId".
     *
     *  ulId - IN - unique id of the Property whose child property list is 
     *              to be retreived
     *  pValues - OUT - list of property name and unique id under the 
     *                  Property whose is is in "ulId"
     */
    STDMETHOD(GetPropListById) 	 	(THIS_
					 const UINT32 ulId,
					 REF(IRMAValues*) pValues) const PURE;

    /************************************************************************
     *  Method:
     *      IRMAPNRegistry::GetNumPropsAtRoot
     *  Purpose:
     *      Returns the number of Properties at the root of the registry. 
     */
    STDMETHOD_(INT32, GetNumPropsAtRoot)	(THIS) const PURE;

    /************************************************************************
     *  Method:
     *      IRMAPNRegistry::GetNumPropsByName
     *  Purpose:
     *      Returns the count of the number of Properties under the one 
     *  whose name is specified in "pName".
     *
     *  pName - IN - name of the Property whose number of children is to be
     *               retreived
     */
    STDMETHOD_(INT32, GetNumPropsByName)	(THIS_
						const char* pName) const PURE;

    /************************************************************************
     *  Method:
     *      IRMAPNRegistry::GetNumPropsById
     *  Purpose:
     *      Returns the count of the number of Properties under the one 
     *  whose unique id is specified in "ulId".
     *
     *  ulId - IN - unique id of the Property whose number of children is 
     *              to be retreived
     */
    STDMETHOD_(INT32, GetNumPropsById)		(THIS_
						const UINT32 ulId) const PURE;
};


/*
 * 
 *  Interface:
 *
 *	IRMAPropWatch
 *
 *  Purpose:
 *
 *      This interface allows the user to watch properties so that when
 *  changes happen to the properties the plugins receive notification via
 *  the IRMAPropWatchResponse API.
 *
 *  IID_IRMAPropWatch:
 *
 *	{00000601-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAPropWatch, 0x00000601, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAPropWatch

DECLARE_INTERFACE_(IRMAPropWatch, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /*
     *	IRMAPropWatch methods
     */

    /************************************************************************
     *  Method:
     *      IRMAPropWatch::Init
     *  Purpose:
     *      Initialize with the response object so that the Watch
     *  notifications can be sent back to the respective plugins.
     *
     *  pResponse - IN - pointer to the response object which gets used to
     *                   initialize the IRMAPropWatch object. the response
     *                   object gets AddRef'd in the Init method.
     */
    STDMETHOD(Init)		(THIS_
				IRMAPropWatchResponse*	pResponse) PURE;

    /************************************************************************
     *  Method:
     *      IRMAPropWatch::SetWatchOnRoot
     *  Purpose:
     *      The SetWatch method puts a watch at the highest level of
     *  the registry hierarchy. It notifies ONLY IF properties at THIS LEVEL
     *  get added/modified/deleted.
     */
    STDMETHOD_(UINT32, SetWatchOnRoot)	(THIS) PURE;

    /************************************************************************
     *  Method:
     *      IRMAPropWatch::SetWatchByName
     *  Purpose:
     *      Sets a watch-point on the Property whose name is passed in.
     *  In case the mentioned Property gets modified or deleted a
     *  notification of that will be sent to the object which set the
     *  watch-point.
     *
     *  pName - IN - name of Property on which a watch point is to be added
     */
    STDMETHOD_(UINT32, SetWatchByName)	(THIS_
					const char*	pName) PURE;

    /************************************************************************
     *  Method:
     *      IRMAPropWatch::SetWatchById
     *  Purpose:
     *      Sets a watch-point on the Property whose name is passed in.
     *  In case the mentioned Property gets modified or deleted a
     *  notification of that will be sent to the object which set the
     *  watch-point.
     *
     *  ulId - IN - unique id of Property on which a watch point is to be 
     *              added
     */
    STDMETHOD_(UINT32, SetWatchById)	(THIS_
					const UINT32	ulId) PURE;

    /************************************************************************
     *  Method:
     *      IRMAPropWatch::ClearWatchOnRoot
     *  Purpose:
     *      It clears the watch on the root of the registry.
     */
    STDMETHOD(ClearWatchOnRoot)		(THIS) PURE;

    /************************************************************************
     *  Method:
     *      IRMAPropWatch::ClearWatchByName
     *  Purpose:
     *      Clears a watch-point based on the Property's name.
     *
     *  pName - IN - name of Property whose watch point is to be cleared
     */
    STDMETHOD(ClearWatchByName)		(THIS_
					const char*	pName) PURE;

    /************************************************************************
     *  Method:
     *      IRMAPropWatch::ClearWatchById
     *  Purpose:
     *      Clears a watch-point based on the Property's id.
     *
     *  ulId - IN - unique id of Property whose watch point is to be cleared
     */
    STDMETHOD(ClearWatchById)		(THIS_
					const UINT32	ulId) PURE;
};


/*
 * 
 *  Interface:
 *
 *	IRMAPropWatchResponse
 *
 *  Purpose:
 *
 *	Interface for notification of additions/modifications/deletions of
 *  properties in the registry which are being watched.
 *
 *  IID_IRMAPropWatchResponse:
 *
 *	{00000602-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAPropWatchResponse, 0x00000602, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAPropWatchResponse

DECLARE_INTERFACE_(IRMAPropWatchResponse, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /*
     * IRMAPropWatchResponse methods
     */

    /************************************************************************
     *  Method:
     *      IRMAPropWatchResponse::AddedProp
     *  Purpose:
     *      Gets called when a new Property gets added under the Property 
     *  on which the Watch was set. It passes the id of the Property just 
     *  added, its datatype and the id of its immediate parent COMPOSITE 
     *  property.
     */
    STDMETHOD(AddedProp)	(THIS_
				const UINT32		ulId,
				const RMAPropType   	propType,
				const UINT32		ulParentID) PURE;

    /************************************************************************
     *  Method:
     *      IRMAPropWatchResponse::ModifiedProp
     *  Purpose:
     *      Gets called when a watched Property gets modified. It passes
     *  the id of the Property just modified, its datatype and the
     *  id of its immediate parent COMPOSITE property.
     */
    STDMETHOD(ModifiedProp)	(THIS_
				const UINT32		ulId,
				const RMAPropType   	propType,
				const UINT32		ulParentID) PURE;

    /************************************************************************
     *  Method:
     *      IRMAPropWatchResponse::DeletedProp
     *  Purpose:
     *      Gets called when a watched Property gets deleted. As can be
     *  seen, it returns the id of the Property just deleted and
     *  its immediate parent COMPOSITE property.
     */
    STDMETHOD(DeletedProp)	(THIS_
				const UINT32		ulId,
				const UINT32		ulParentID) PURE;
};

/*
 * 
 *  Interface:
 *
 *	IRMAActiveRegistry
 *
 *  Purpose:
 *
 *	Interface to get IRMAActiveUser responsible for a particular property 
 *  from the registry.
 *
 *  IID_IRMAActiveRegistry:
 *
 *	{00000603-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAActiveRegistry, 0x00000603, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAActiveRegistry

DECLARE_INTERFACE_(IRMAActiveRegistry, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /************************************************************************
    * IRMAActiveRegistry::SetAsActive
    *
    *     Method to set prop pName to active and register pUser as
    *   the active prop user.
    */
    STDMETHOD(SetAsActive)    (THIS_
				const char* pName,
				IRMAActivePropUser* pUser) PURE;

    /************************************************************************
    * IRMAActiveRegistry::SetAsInactive
    *
    *	Method to remove an IRMAActiveUser from Prop activation.
    */
    STDMETHOD(SetAsInactive)  (THIS_
				const char* pName,
				IRMAActivePropUser* pUser) PURE;

    /************************************************************************
    * IRMAActiveRegistry::IsActive
    *
    *     Tells if prop pName has an active user that must be queried to
    *   change the value, or if it can just be set.
    */
    STDMETHOD_(BOOL, IsActive)	(THIS_
				const char* pName) PURE;

    /************************************************************************
    * IRMAActiveRegistry::SetActiveInt
    *
    *    Async request to set int pName to ul.
    */
    STDMETHOD(SetActiveInt) (THIS_
			    const char* pName,
			    UINT32 ul,
			    IRMAActivePropUserResponse* pResponse) PURE;

    /************************************************************************
    * IRMAActiveRegistry::SetActiveStr
    *
    *    Async request to set string pName to string in pBuffer.
    */
    STDMETHOD(SetActiveStr) (THIS_
			    const char* pName,
			    IRMABuffer* pBuffer,
			    IRMAActivePropUserResponse* pResponse) PURE;

    /************************************************************************
    * IRMAActiveRegistry::SetActiveBuf
    *
    *    Async request to set buffer pName to buffer in pBuffer.
    */
    STDMETHOD(SetActiveBuf)	(THIS_
				const char* pName,
				IRMABuffer* pBuffer,
				IRMAActivePropUserResponse* pResponse) PURE;

    /************************************************************************
    * IRMAActiveRegistry::DeleteActiveProp
    *
    *	Async request to delete the active property.
    */
    STDMETHOD(DeleteActiveProp)	(THIS_
				const char* pName,
				IRMAActivePropUserResponse* pResponse) PURE;


};


/*
 * 
 *  Interface:
 *
 *	IRMAActivePropUser
 *
 *  Purpose:
 *
 *	An IRMAActivePropUser can be set as the active user of a property in 
 *  an IRMAActiveRegistry. This causes the IRMAActivePropUser to be consulted 
 *  everytime someone wants to change a property. The difference between this 
 *  and a prop watch is that this is async, and can call a done method with 
 *  failure to cause the prop to not be set, and this get called instead of 
 *  calling into the IRMAPNReg.
 *
 *  IID_IRMAActivePropUser:
 *
 *	{00000604-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAActivePropUser, 0x00000604, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAActivePropUser

DECLARE_INTERFACE_(IRMAActivePropUser, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /************************************************************************
    * IRMAActivePropUser::SetActiveInt
    *
    *    Async request to set int pName to ul.
    */
    STDMETHOD(SetActiveInt) (THIS_
			    const char* pName,
			    UINT32 ul,
			    IRMAActivePropUserResponse* pResponse) PURE;

    /************************************************************************
    * IRMAActivePropUser::SetActiveStr
    *
    *    Async request to set string pName to string in pBuffer.
    */
    STDMETHOD(SetActiveStr) (THIS_
			    const char* pName,
			    IRMABuffer* pBuffer,
			    IRMAActivePropUserResponse* pResponse) PURE;

    /************************************************************************
    * IRMAActivePropUser::SetActiveBuf
    *
    *    Async request to set buffer pName to buffer in pBuffer.
    */
    STDMETHOD(SetActiveBuf)	(THIS_
				const char* pName,
				IRMABuffer* pBuffer,
				IRMAActivePropUserResponse* pResponse) PURE;

    /************************************************************************
    * IRMAActivePropUser::DeleteActiveProp
    *
    *	Async request to delete the active property.
    */
    STDMETHOD(DeleteActiveProp)	(THIS_
				const char* pName,
				IRMAActivePropUserResponse* pResponse) PURE;

};

/*
 * 
 *  Interface:
 *
 *	IRMAActivePropUserResponse
 *
 *  Purpose:
 *
 *	Gets responses from IRMAActivePropUser for queries to set properties
 *  in the IRMAActiveRegistry.
 *
 *
 *  IID_IRMAActivePropUserResponse:
 *
 *	{00000605-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAActivePropUserResponse, 0x00000605, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAActivePropUserResponse

DECLARE_INTERFACE_(IRMAActivePropUserResponse, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /************************************************************************
    * Called with status result on completion of set request.
    */
    STDMETHOD(SetActiveIntDone)   (THIS_
				    PN_RESULT res,
				    const char* pName,
				    UINT32 ul,
				    IRMABuffer* pInfo[],
				    UINT32 ulNumInfo) PURE;

    STDMETHOD(SetActiveStrDone)	  (THIS_
				    PN_RESULT res,
				    const char* pName,
				    IRMABuffer* pBuffer,
				    IRMABuffer* pInfo[],
				    UINT32 ulNumInfo) PURE;

    STDMETHOD(SetActiveBufDone)	  (THIS_
				    PN_RESULT res,
				    const char* pName,
				    IRMABuffer* pBuffer,
				    IRMABuffer* pInfo[],
				    UINT32 ulNumInfo) PURE;

    STDMETHOD(DeleteActivePropDone) (THIS_
				    PN_RESULT res,
				    const char* pName,
				    IRMABuffer* pInfo[],
				    UINT32 ulNumInfo) PURE;

};

/*
 * 
 *  Interface:
 *
 *	IRMACopyRegistry
 *
 *  Purpose:
 *
 *	Allows copying from one registry key to another.
 *
 *
 *  IID_IRMACopyRegistry
 *
 *	{00000606-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMACopyRegistry, 0x00000606, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMACopyRegistry

DECLARE_INTERFACE_(IRMACopyRegistry, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /************************************************************************
    * IRMACopyRegistry::Copy
    *
    *   Here it is! The "Copy" method!
    */
    STDMETHOD (CopyByName)  (THIS_
			    const char* pFrom,
			    const char* pTo) PURE;
};


/*
 * 
 *  Interface:
 *
 *	IRMAPNRegistryAltStringHandling
 *
 *  Purpose:
 *
 *	Tells the registry about alternate handling of PT_STRING types.
 *
 *
 *  IID_IRMAPNRegistryAltStringHandling
 *
 *	{00000607-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAPNRegistryAltStringHandling, 0x00000607, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAPNRegistryAltStringHandling

DECLARE_INTERFACE_(IRMAPNRegistryAltStringHandling, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /************************************************************************
    * IRMAPNRegistryAltStringHandling::SetStringAccessAsBufferById
    *
    *   For those times when you added a property as a buffer, but wish it
    *   were a string (and of course, people now rely on the fact that it's
    *   a buffer)...  Create the property as a string and then pass this
    *   method it's ID.  The property will now be accessible/setable as a,
    *   but it will still be a string!
    */
    STDMETHOD (SetStringAccessAsBufferById)  (THIS_
					      UINT32 ulId) PURE;
};


#endif /* _RMAMON_H_ */
