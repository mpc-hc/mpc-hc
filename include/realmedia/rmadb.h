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
 *  RealMedia Architecture Interface.
 *
 */

#ifndef _RMADB_H_
#define _RMADB_H_

/*
 * Forward declarations of some interfaces defined or used here-in.
 */
typedef _INTERFACE	IRMABuffer				IRMABuffer;
typedef _INTERFACE	IRMAValues				IRMAValues;
typedef _INTERFACE	IRMADatabaseManager			IRMADatabaseManager;
typedef _INTERFACE	IRMAAuthenticationDBManager		IRMAAuthenticationDBManager;
typedef _INTERFACE	IRMAAuthenticationDBManagerResponse	IRMAAuthenticationDBManagerResponse;
typedef _INTERFACE	IRMAAsyncEnumAuthenticationDB		IRMAAsyncEnumAuthenticationDB;
typedef _INTERFACE	IRMAAsyncEnumAuthenticationDBResponse	IRMAAsyncEnumAuthenticationDBResponse;
typedef _INTERFACE	IRMAAuthenticationDBAccess		IRMAAuthenticationDBAccess;
typedef _INTERFACE	IRMAAuthenticationDBAccessResponse	IRMAAuthenticationDBAccessResponse;
typedef _INTERFACE	IRMAGUIDDBManager			IRMAGUIDDBManager;
typedef _INTERFACE	IRMAGUIDDBManagerResponse		IRMAGUIDDBManagerResponse;
typedef _INTERFACE	IRMAPPVDBManager			IRMAPPVDBManager;
typedef _INTERFACE	IRMAPPVDBManagerResponse		IRMAPPVDBManagerResponse;
typedef _INTERFACE	IRMARedirectDBManager			IRMARedirectDBManager;
typedef _INTERFACE	IRMARedirectDBManagerResponse		IRMARedirectDBManagerResponse;
typedef _INTERFACE	IRMARegistrationLogger			IRMARegistrationLogger;

/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMADatabaseManager
 *
 *  Purpose:
 *
 *	This is implemented by the database manager in order to provide
 *	access to the databases it manages.
 *
 *  IRMADatabaseManager:
 *
 *	{00002A00-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMADatabaseManager,   0x00002A00, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#define CLSID_CRMADatabaseManager IID_IRMADatabaseManager

#undef  INTERFACE
#define INTERFACE   IRMADatabaseManager

DECLARE_INTERFACE_(IRMADatabaseManager, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)
    (
	THIS_
	REFIID			IIDOfInterfaceDesired,
	void**			ppVoidRequestedInterface
    ) PURE;

    STDMETHOD_(ULONG,AddRef)
    (
	THIS
    ) PURE;

    STDMETHOD_(ULONG,Release)
    (
	THIS
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMADatabaseManager::GetInstanceFromID
     *	Purpose:
     *	    
     *	    Returns a database object configured as defined for the specifed 
     *	    DatabaseID in the server config file.
     *
     */
    STDMETHOD(GetInstanceFromID)
    (
	THIS_
	IRMABuffer*		pBufferID, 
	REF(IUnknown*)		pUnknownDatabase
    ) PURE;

};

/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAAuthenticationDBManager
 *
 *  Purpose:
 *
 *	A database plugin will implement this when it desires to provide 
 *	storage for authentication data.
 *
 *  IRMAAuthenticationDBManager:
 *
 *	{00002A02-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAAuthenticationDBManager,   0x00002A02, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAAuthenticationDBManager

DECLARE_INTERFACE_(IRMAAuthenticationDBManager, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)
    (
	THIS_
	REFIID			IIDOfInterfaceDesired,
	void**			ppVoidRequestedInterface
    ) PURE;

    STDMETHOD_(ULONG,AddRef)
    (
	THIS
    ) PURE;

    STDMETHOD_(ULONG,Release)
    (
	THIS
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAAuthenticationDBManager::AddPrincipal
     *	Purpose:
     *	    
     *	    Adds the specified user to the database, if it is not already 
     *	    there.
     *
     */
    STDMETHOD(AddPrincipal)
    (
	THIS_ 
	IRMAAuthenticationDBManagerResponse* pAuthenticationDBManagerResponseNew,
	IRMABuffer*		pBufferPrincipalID
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAAuthenticationDBManager::RemovePrincipal
     *	Purpose:
     *	    
     *	    Removes the specified user from the database, if it is there.
     *	    
     *
     */
    STDMETHOD(RemovePrincipal)
    (
	THIS_ 
	IRMAAuthenticationDBManagerResponse* pAuthenticationDBManagerResponseNew,
	IRMABuffer*		pBufferPrincipalID
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAAuthenticationDBManager::SetCredentials
     *	Purpose:
     *	    
     *	    Replaces the credentials for the specified user.
     *	    Usually the credentials are a password.
     *	    It is not the databases job to protect this data
     *	    Authentication plugins will protect this data 
     *	    before storing it when neccesary.
     *
     */
    STDMETHOD(SetCredentials)
    (
	THIS_
	IRMAAuthenticationDBManagerResponse* pAuthenticationDBManagerResponseNew,
	IRMABuffer*		pBufferPrincipalID, 
	IRMABuffer*		pBufferCredentials
    ) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAAuthenticationDBManagerResponse
 *
 *  Purpose:
 *
 *	A database user will implement this when it desires to manage 
 *	storage for authentication data.
 *	This interface receives the results of IRMAAuthenticationDBManager
 *	methods
 *
 *  IRMAAuthenticationDBManagerResponse:
 *
 *	{00002A01-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAAuthenticationDBManagerResponse,   0x00002A01, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAAuthenticationDBManagerResponse

DECLARE_INTERFACE_(IRMAAuthenticationDBManagerResponse, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)
    (
	THIS_
	REFIID			IIDOfInterfaceDesired,
	void**			ppVoidRequestedInterface
    ) PURE;

    STDMETHOD_(ULONG,AddRef)
    (
	THIS
    ) PURE;

    STDMETHOD_(ULONG,Release)
    (
	THIS
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAAuthenticationDBManagerResponse::AddPrincipalDone
     *	Purpose:
     *	    
     *	    Reports the success or failure of 
     *	    IRMAAuthenticationDBManager::AddPrincipal
     *
     */
    STDMETHOD(AddPrincipalDone)
    (
	THIS_ 
	PN_RESULT		ResultStatus,
	IRMABuffer*		pBufferPrincipalID
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAAuthenticationDBManagerResponse::RemovePrincipalDone
     *	Purpose:
     *	    
     *	    Reports the success or failure of 
     *	    IRMAAuthenticationDBManager::RemovePrincipal
     *
     */
    STDMETHOD(RemovePrincipalDone)
    (
	THIS_ 
	PN_RESULT		ResultStatus,
	IRMABuffer*		pBufferPrincipalID
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAAuthenticationDBManagerResponse::SetCredentialsDone
     *	Purpose:
     *	    
     *	    Reports the success or failure of 
     *	    IRMAAuthenticationDBManager::SetCredentials
     *
     */
    STDMETHOD(SetCredentialsDone)
    (
	THIS_
	PN_RESULT		ResultStatus,
	IRMABuffer*		pBufferPrincipalID
    ) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAAsyncEnumAuthenticationDB
 *
 *  Purpose:
 *
 *	A database plugin will implement this when it desires to provide 
 *	enumeration of authentication data.
 *
 *  IRMAAsyncEnumAuthenticationDB:
 *
 *	{00002A04-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAAsyncEnumAuthenticationDB,   0x00002A04, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAAsyncEnumAuthenticationDB

DECLARE_INTERFACE_(IRMAAsyncEnumAuthenticationDB, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)
    (
	THIS_
	REFIID			IIDOfInterfaceDesired,
	void**			ppVoidRequestedInterface
    ) PURE;

    STDMETHOD_(ULONG,AddRef)
    (
	THIS
    ) PURE;

    STDMETHOD_(ULONG,Release)
    (
	THIS
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAAsyncEnumAuthenticationDB::Reset
     *	Purpose:
     *	    
     *	    Call this to reset this enumerator to the beginning of the 
     *	    collection.
     *
     */
    STDMETHOD(Reset)
    (
	THIS_
	IRMAAsyncEnumAuthenticationDBResponse* pAsyncEnumAuthenticationDBResponseNew
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAAsyncEnumAuthenticationDB::Next
     *	Purpose:
     *	    
     *	    Call this to retrieve the next item in the collection.
     *
     */
    STDMETHOD(Next)
    (
	THIS_
	IRMAAsyncEnumAuthenticationDBResponse* pAsyncEnumAuthenticationDBResponseNew
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAAsyncEnumAuthenticationDB::Skip
     *	Purpose:
     *	    
     *	    Call this to skip the next n items in the collection and 
     *	    retrieve the n+1 item.
     *
     */
    STDMETHOD(Skip)
    (
	THIS_
	IRMAAsyncEnumAuthenticationDBResponse* pAsyncEnumAuthenticationDBResponseNew,
	UINT32			ulNumToSkip
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAAsyncEnumAuthenticationDB::Clone
     *	Purpose:
     *	    
     *	    Call this to make a new enumerator of this collection.
     *
     */
    STDMETHOD(Clone)
    (
	THIS_
	REF(IRMAAsyncEnumAuthenticationDB*) pAsyncEnumAuthenticationDBNew
    ) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAAsyncEnumAuthenticationDBResponse
 *
 *  Purpose:
 *
 *	A database user will implement this when it desires to 
 *	enumerate authentication data.
 *	This interface receives the results of IRMAAsyncEnumAuthenticationDB
 *	methods
 *
 *  IRMAAsyncEnumAuthenticationDBResponse:
 *
 *	{00002A03-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAAsyncEnumAuthenticationDBResponse,   0x00002A03, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAAsyncEnumAuthenticationDBResponse

DECLARE_INTERFACE_(IRMAAsyncEnumAuthenticationDBResponse, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)
    (
	THIS_
	REFIID			IIDOfInterfaceDesired,
	void**			ppVoidRequestedInterface
    ) PURE;

    STDMETHOD_(ULONG,AddRef)
    (
	THIS
    ) PURE;

    STDMETHOD_(ULONG,Release)
    (
	THIS
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAAsyncEnumAuthenticationDBResponse::ResetDone
     *	Purpose:
     *	    
     *	    Reports the success or failure of 
     *	    IRMAAsyncEnumAuthenticationDB::Reset
     *
     */
    STDMETHOD(ResetDone)
    (
	THIS_
	PN_RESULT		ResultStatus
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAAsyncEnumAuthenticationDBResponse::NextDone
     *	Purpose:
     *	    
     *	    Reports the success or failure of 
     *	    IRMAAsyncEnumAuthenticationDB::Next
     *	    If successful the PrincipalID is valid.
     *
     */
    STDMETHOD(NextDone)
    (
	THIS_
	PN_RESULT		ResultStatus,
	IRMABuffer*		pBufferNextPrincipalID
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAAsyncEnumAuthenticationDBResponse::SkipDone
     *	Purpose:
     *	    
     *	    Reports the success or failure of 
     *	    IRMAAsyncEnumAuthenticationDB::Skip
     *	    If successful the PrincipalID is valid.
     *
     */
    STDMETHOD(SkipDone)
    (
	THIS_
	PN_RESULT		ResultStatus,
	IRMABuffer*		pBufferNextPrincipalID
    ) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAAuthenticationDBAccess
 *
 *  Purpose:
 *
 *	A database plugin will implement this when it desires to provide 
 *	access to authentication data.
 *
 *  IRMAAuthenticationDBAccess:
 *
 *	{00002A06-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAAuthenticationDBAccess,   0x00002A06, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAAuthenticationDBAccess

DECLARE_INTERFACE_(IRMAAuthenticationDBAccess, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)
    (
	THIS_
	REFIID			IIDOfInterfaceDesired,
	void**			ppVoidRequestedInterface
    ) PURE;

    STDMETHOD_(ULONG,AddRef)
    (
	THIS
    ) PURE;

    STDMETHOD_(ULONG,Release)
    (
	THIS
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAAuthenticationDBAccess::_NewEnum
     *	Purpose:
     *	    
     *	    Call this to make a new enumerator of this collection.
     *
     */
    STDMETHOD(_NewEnum)
    (
	THIS_
	REF(IRMAAsyncEnumAuthenticationDB*) pAsyncEnumAuthenticationDBNew
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAAuthenticationDBAccess::CheckExistence
     *	Purpose:
     *	    
     *	    Call this to verify the existance of a principal.
     *
     */
    STDMETHOD(CheckExistence)
    (
	THIS_
	IRMAAuthenticationDBAccessResponse* pAuthenticationDBAccessResponseNew,
	IRMABuffer*		pBufferPrincipalID
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAAuthenticationDBAccess::GetCredentials
     *	Purpose:
     *	    
     *	    Call this to access the credentials for the specified principal.
     *
     */
    STDMETHOD(GetCredentials)
    (
	THIS_
	IRMAAuthenticationDBAccessResponse* pAuthenticationDBAccessResponseNew,
	IRMABuffer*		pBufferPrincipalID
    ) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAAuthenticationDBAccessResponse
 *
 *  Purpose:
 *
 *	A database user will implement this when it desires to 
 *	access authentication data.
 *	This interface receives the results of IRMAAuthenticationDBAccess
 *	methods
 *
 *  IRMAAuthenticationDBAccessResponse:
 *
 *	{00002A05-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAAuthenticationDBAccessResponse,   0x00002A05, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAAuthenticationDBAccessResponse

DECLARE_INTERFACE_(IRMAAuthenticationDBAccessResponse, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)
    (
	THIS_
	REFIID			IIDOfInterfaceDesired,
	void**			ppVoidRequestedInterface
    ) PURE;

    STDMETHOD_(ULONG,AddRef)
    (
	THIS
    ) PURE;

    STDMETHOD_(ULONG,Release)
    (
	THIS
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAAuthenticationDBAccessResponse::ExistenceCheckDone
     *	Purpose:
     *	    
     *	    Reports the success or failure of 
     *	    IRMAAuthenticationDBAccess::ExistenceCheck
     *
     */
    STDMETHOD(ExistenceCheckDone)
    (
	THIS_
	PN_RESULT		ResultStatus,
	IRMABuffer*		pBufferPrincipalID
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAAuthenticationDBAccessResponse::GetCredentialsDone
     *	Purpose:
     *	    
     *	    Reports the success or failure of 
     *	    IRMAAuthenticationDBAccess::GetCredentials
     *	    If successful the Credentials var is valid.
     *
     */
    STDMETHOD(GetCredentialsDone)
    (
	THIS_
	PN_RESULT		ResultStatus,
	IRMABuffer*		pBufferPrincipalID,
	IRMABuffer*		pBufferCredentials
    ) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAGUIDDBManager
 *
 *  Purpose:
 *
 *	A database plugin will implement this when it desires to provide 
 *	storage of player GUID data (for Player Authentication).
 *
 *  IRMAGUIDDBManager:
 *
 *	{00002A08-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAGUIDDBManager,   0x00002A08, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAGUIDDBManager

DECLARE_INTERFACE_(IRMAGUIDDBManager, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)
    (
	THIS_
	REFIID			IIDOfInterfaceDesired,
	void**			ppVoidRequestedInterface
    ) PURE;

    STDMETHOD_(ULONG,AddRef)
    (
	THIS
    ) PURE;

    STDMETHOD_(ULONG,Release)
    (
	THIS
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAGUIDDBManager::SetGUIDForPrincipalID
     *	Purpose:
     *	    
     *	    Call this to associate a player GUID with a user.
     *
     */
    STDMETHOD(SetGUIDForPrincipalID)
    (
	THIS_
	IRMAGUIDDBManagerResponse* pGUIDDBManagerResponseNew,
	IRMABuffer*		pBufferPrincipalID,
	IRMABuffer*		pBufferGUID
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAGUIDDBManager::GetPrincipalIDFromGUID
     *	Purpose:
     *	    
     *	    Call this to get the associated player GUID from a user.
     *
     */
    STDMETHOD(GetPrincipalIDFromGUID)
    (
	THIS_
	IRMAGUIDDBManagerResponse* pGUIDDBManagerResponseNew,
	IRMABuffer*		pBufferGUID
    ) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAGUIDDBManagerResponse
 *
 *  Purpose:
 *
 *	A database user will implement this when it desires to 
 *	manage player GUID data.
 *	This interface receives the results of IRMAGUIDDBManager
 *	methods
 *
 *  IRMAGUIDDBManagerResponse:
 *
 *	{00002A07-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAGUIDDBManagerResponse,   0x00002A07, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAGUIDDBManagerResponse

DECLARE_INTERFACE_(IRMAGUIDDBManagerResponse, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)
    (
	THIS_
	REFIID			IIDOfInterfaceDesired,
	void**			ppVoidRequestedInterface
    ) PURE;

    STDMETHOD_(ULONG,AddRef)
    (
	THIS
    ) PURE;

    STDMETHOD_(ULONG,Release)
    (
	THIS
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAGUIDDBManagerResponse::SetGUIDForPrincipalIDDone
     *	Purpose:
     *	    
     *	    Reports the success or failure of 
     *	    IRMAGUIDDBManager::SetGUIDForPrincipalID
     *
     */
    STDMETHOD(SetGUIDForPrincipalIDDone)
    (
	THIS_
	PN_RESULT		ResultStatus,
	IRMABuffer*		pBufferPrincipalID
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAGUIDDBManagerResponse::GetPrincipalIDFromGUIDDone
     *	Purpose:
     *	    
     *	    Reports the success or failure of 
     *	    IRMAGUIDDBManager::GetGUIDForPrincipalID
     *
     */
    STDMETHOD(GetPrincipalIDFromGUIDDone)
    (
	THIS_
	PN_RESULT		ResultStatus,
	IRMABuffer*		pBufferGUID,
	IRMABuffer*		pBufferPrincipalID
    ) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAPPVDBManager
 *
 *  Purpose:
 *
 *	A database plugin will implement this when it desires to provide 
 *	storage of Pay-Per-View permission data.
 *
 *  IRMAPPVDBManager:
 *
 *	{00002A0A-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAPPVDBManager,   0x00002A0A, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAPPVDBManager

DECLARE_INTERFACE_(IRMAPPVDBManager, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)
    (
	THIS_
	REFIID			IIDOfInterfaceDesired,
	void**			ppVoidRequestedInterface
    ) PURE;

    STDMETHOD_(ULONG,AddRef)
    (
	THIS
    ) PURE;

    STDMETHOD_(ULONG,Release)
    (
	THIS
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAPPVDBManager::GetPermissions
     *	Purpose:
     *	    
     *	    Call this to find the PPV permissions for the specified URL 
     *	    and user.
     *
     */
    STDMETHOD(GetPermissions)
    (
	THIS_
	IRMAPPVDBManagerResponse* pPPVDBManagerResponseNew,
	IRMABuffer*		pBufferPrincipalID,
	IRMABuffer*		pBufferURL
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAPPVDBManager::SetPermissions
     *	Purpose:
     *	    
     *	    Call this to set the PPV permissions for the specified URL 
     *	    and user.
     *
     */
    STDMETHOD(SetPermissions)
    (
	THIS_
	IRMAPPVDBManagerResponse* pPPVDBManagerResponseNew,
	IRMABuffer*		pBufferPrincipalID,
	IRMABuffer*		pBufferURL,
	IRMAValues*		pValuesPermissions
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAPPVDBManager::RevokePermissions
     *	Purpose:
     *	    
     *	    Call this to remove the PPV permissions for the specified URL 
     *	    and user.
     *
     */
    STDMETHOD(RevokePermissions)
    (
	THIS_
	IRMAPPVDBManagerResponse* pPPVDBManagerResponseNew,
	IRMABuffer*		pBufferPrincipalID,
	IRMABuffer*		pBufferURL
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAPPVDBManager::RevokeAllPermissions
     *	Purpose:
     *	    
     *	    Call this to remove the PPV permissions for all URL's 
     *	    that this user has access too.
     *
     */
    STDMETHOD(RevokeAllPermissions)
    (
	THIS_
	IRMAPPVDBManagerResponse* pPPVDBManagerResponseNew,
	IRMABuffer*		pBufferPrincipalID
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAPPVDBManager::LogAccessAttempt
     *	Purpose:
     *	    
     *	    Call this to record the results of an attempt to access 
     *	    protected content.
     *
     */
    STDMETHOD(LogAccessAttempt)
    (
	THIS_
	IRMAValues*		pValuesAccess
    ) PURE;

};

/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAPPVDBManagerResponse
 *
 *  Purpose:
 *
 *	A database user will implement this when it desires to 
 *	manage Pay-Per-View permission data.
 *	This interface receives the results of IRMAPPVDBManager
 *	methods
 *
 *  IRMAPPVDBManagerResponse:
 *
 *	{00002A09-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAPPVDBManagerResponse,   0x00002A09, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAPPVDBManagerResponse

DECLARE_INTERFACE_(IRMAPPVDBManagerResponse, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)
    (
	THIS_
	REFIID			IIDOfInterfaceDesired,
	void**			ppVoidRequestedInterface
    ) PURE;

    STDMETHOD_(ULONG,AddRef)
    (
	THIS
    ) PURE;

    STDMETHOD_(ULONG,Release)
    (
	THIS
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAPPVDBManagerResponse::GetPermissionsDone
     *	Purpose:
     *	    
     *	    Reports the success or failure of 
     *	    IRMAPPVDBManager::GetPermissions
     *	    If successful then the Permissions are valid
     *
     */
    STDMETHOD(GetPermissionsDone)
    (
	THIS_
	PN_RESULT		ResultStatus,
	IRMABuffer*		pBufferPrincipalID,
	IRMAValues*		pValuesPermissions
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAPPVDBManagerResponse::SetPermissionsDone
     *	Purpose:
     *	    
     *	    Reports the success or failure of 
     *	    IRMAPPVDBManager::SetPermissions
     *
     */
    STDMETHOD(SetPermissionsDone)
    (
	THIS_
	PN_RESULT		ResultStatus,
	IRMABuffer*		pBufferPrincipalID
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAPPVDBManagerResponse::RevokePermissionsDone
     *	Purpose:
     *	    
     *	    Reports the success or failure of 
     *	    IRMAPPVDBManager::RevokePermissions
     *
     */
    STDMETHOD(RevokePermissionsDone)
    (
	THIS_
	PN_RESULT		ResultStatus,
	IRMABuffer*		pBufferPrincipalID
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAPPVDBManagerResponse::RevokeAllPermissionsDone
     *	Purpose:
     *	    
     *	    Reports the success or failure of 
     *	    IRMAPPVDBManager::RevokeAllPermissions
     *
     */
    STDMETHOD(RevokeAllPermissionsDone)
    (
	THIS_
	PN_RESULT		ResultStatus,
	IRMABuffer*		pBufferPrincipalID
    ) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMARedirectDBManager
 *
 *  Purpose:
 *
 *	A database plugin will implement this when it desires to provide 
 *	storage of URL's to redirect.
 *
 *  IRMARedirectDBManager:
 *
 *	{00002A0C-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMARedirectDBManager,   0x00002A0C, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMARedirectDBManager

DECLARE_INTERFACE_(IRMARedirectDBManager, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)
    (
	THIS_
	REFIID			IIDOfInterfaceDesired,
	void**			ppVoidRequestedInterface
    ) PURE;

    STDMETHOD_(ULONG,AddRef)
    (
	THIS
    ) PURE;

    STDMETHOD_(ULONG,Release)
    (
	THIS
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMARedirectDBManager::GetRedirect
     *	Purpose:
     *	    
     *	    Call this to retrieve the URL that the specified URL should
     *	    be redirected to.
     *
     */
    STDMETHOD(GetRedirect)
    (
	THIS_
	IRMARedirectDBManagerResponse* pRedirectDBManagerResponseNew,
	IRMABuffer*		pBufferURL
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMARedirectDBManager::AddRedirect
     *	Purpose:
     *	    
     *	    Call this to set the new URL that the specified URL should
     *	    be redirected to.
     *
     */
    STDMETHOD(AddRedirect)
    (
	THIS_
	IRMARedirectDBManagerResponse* pRedirectDBManagerResponseNew,
	IRMABuffer*		pBufferURL,
	IRMABuffer*		pBufferNewURL
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMARedirectDBManager::RemoveRedirect
     *	Purpose:
     *	    
     *	    Call this to stop redirecting the specified URL.
     *
     */
    STDMETHOD(RemoveRedirect)
    (
	THIS_
	IRMARedirectDBManagerResponse* pRedirectDBManagerResponseNew,
	IRMABuffer*		pBufferURL
    ) PURE;

};

/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMARedirectDBManagerResponse
 *
 *  Purpose:
 *
 *	A database user will implement this when it desires to 
 *	manage the URL's to redirect.
 *	This interface receives the results of IRMARedirectDBManager
 *	methods
 *
 *  IRMARedirectDBManagerResponse:
 *
 *	{00002A0B-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMARedirectDBManagerResponse,   0x00002A0B, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMARedirectDBManagerResponse

DECLARE_INTERFACE_(IRMARedirectDBManagerResponse, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)
    (
	THIS_
	REFIID			IIDOfInterfaceDesired,
	void**			ppVoidRequestedInterface
    ) PURE;

    STDMETHOD_(ULONG,AddRef)
    (
	THIS
    ) PURE;

    STDMETHOD_(ULONG,Release)
    (
	THIS
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMARedirectDBManagerResponse::GetRedirectDone
     *	Purpose:
     *	    
     *	    Reports the success or failure of 
     *	    IRMARedirectDBManager::GetRedirect
     *	    If successful then the new URL is valid
     *
     */
    STDMETHOD(GetRedirectDone)
    (
	THIS_
	PN_RESULT		ResultStatus,
	IRMABuffer*		pBufferURL,
	IRMABuffer*		pBufferNewURL
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMARedirectDBManagerResponse::AddRedirectDone
     *	Purpose:
     *	    
     *	    Reports the success or failure of 
     *	    IRMARedirectDBManager::AddRedirect
     *
     */
    STDMETHOD(AddRedirectDone)
    (
	THIS_
	PN_RESULT		ResultStatus,
	IRMABuffer*		pBufferURL
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMARedirectDBManagerResponse::RemoveRedirectDone
     *	Purpose:
     *	    
     *	    Reports the success or failure of 
     *	    IRMARedirectDBManager::RemoveRedirect
     *
     */
    STDMETHOD(RemoveRedirectDone)
    (
	THIS_
	PN_RESULT		ResultStatus,
	IRMABuffer*		pBufferURL
    ) PURE;

};

/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMARegistrationLogger
 *
 *  Purpose:
 *
 *	A database plugin will implement this when it desires to provide 
 *	storage of player registration attempts.
 *
 *  IRMARegistrationLogger:
 *
 *	{00002A0E-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMARegistrationLogger,   0x00002A0E, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMARegistrationLogger

DECLARE_INTERFACE_(IRMARegistrationLogger, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)
    (
	THIS_
	REFIID			IIDOfInterfaceDesired,
	void**			ppVoidRequestedInterface
    ) PURE;

    STDMETHOD_(ULONG,AddRef)
    (
	THIS
    ) PURE;

    STDMETHOD_(ULONG,Release)
    (
	THIS
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMARegistrationLogger::LogRegistrationAttempt
     *	Purpose:
     *	    
     *	    Call this to record the results of an attempt to register
     *	    a player's GUID.
     *
     */
    STDMETHOD(LogRegistrationAttempt)
    (
	THIS_
	IRMAValues*		pValuesRegistration
    ) PURE;
};


#endif /* !_RMADB_H_ */
