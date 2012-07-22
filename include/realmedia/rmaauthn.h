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

#ifndef _RMAAUTHN_H_
#define _RMAAUTHN_H_

/*
 * Forward declarations of some interfaces defined or used here-in.
 */
typedef _INTERFACE  IUnknown			    IUnknown;
typedef _INTERFACE  IRMACredRequest		    IRMACredRequest;
typedef _INTERFACE  IRMACredRequestResponse	    IRMACredRequestResponse;
typedef _INTERFACE  IRMAClientAuthConversation	    IRMAClientAuthConversation;
typedef _INTERFACE  IRMAClientAuthResponse	    IRMAClientAuthResponse;
typedef _INTERFACE  IRMAServerAuthConversation	    IRMAServerAuthConversation;
typedef _INTERFACE  IRMAServerAuthResponse	    IRMAServerAuthResponse;
typedef _INTERFACE  IRMAUserContext		    IRMAUserContext;
typedef _INTERFACE  IRMAUserProperties		    IRMAUserProperties;
typedef _INTERFACE  IRMAUserImpersonation	    IRMAUserImpersonation;
typedef _INTERFACE  IRMAChallenge		    IRMAChallenge;
typedef _INTERFACE  IRMAChallengeResponse	    IRMAChallengeResponse;
typedef _INTERFACE  IRMARequest			    IRMARequest;
typedef _INTERFACE  IRMABuffer			    IRMABuffer;
typedef _INTERFACE  IRMAValues			    IRMAValues;


/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMACredRequest
 *
 *  Purpose:
 *
 *	This is queried from the response interface passed into 
 *	IRMAClientAuthConversation::MakeResponse.  MakeResponse
 *	uses it to request the current user to enter their credentials. 
 *
 *  IRMACredRequest:
 *
 *	{00002801-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMACredRequest,   0x00002801, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMACredRequest

DECLARE_INTERFACE_(IRMACredRequest, IUnknown)
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
     *	Method:
     *	    IRMACredRequest::GetCredentials
     *	Purpose:
     *	    
     *	    Call this to request the credentials.  Usually presents UI to 
     *	    the user asking for username and password.
     *
     *	    While ignored at this time, pValuesCredentialRequest should
     *	    contain CString properties that describe the reason for the 
     *	    request. (like the URL, the Realm, the Auth protocol, and how 
     *	    secure it is, etc..)  In the future this data will be displayed
     *	    to the user.
     *
     */
    STDMETHOD(GetCredentials)
    (
	THIS_
	IRMACredRequestResponse* pCredRequestResponseRequester,
	IRMAValues* pValuesCredentialRequest
    ) PURE;

};

/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMACredRequestResponse
 *
 *  Purpose:
 *
 *	This is implemented by a client authenticator in order to receive
 *	the credentials requested in IRMACredRequest::GetCredentials
 *
 *  IRMACredRequestResponse:
 *
 *	{00002800-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMACredRequestResponse,   0x00002800, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMACredRequestResponse

DECLARE_INTERFACE_(IRMACredRequestResponse, IUnknown)
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
     *	Method:
     *	    IRMACredRequestResponse::CredentialsReady
     *	Purpose:
     *	    
     *	    Reports the success or failure of 
     *	    IRMACredRequest::GetCredentials
     *
     *	    If successful pValuesCredentials contains the requested 
     *	    credentials.  (usually CString:Username and CString:Password)
     *
     */
    STDMETHOD(CredentialsReady)
    (
	THIS_
	PN_RESULT	ResultStatus,
	IRMAValues* pValuesCredentials
    ) PURE;

};

/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAClientAuthConversation
 *
 *  Purpose:
 *
 *	This is implemented by a client authenticator in order to perform 
 *	the client side of an authentication protocol.	
 *
 *  IRMAClientAuthConversation:
 *
 *	{00002803-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAClientAuthConversation,   0x00002803, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

/*
 *  The IRMACommonClassFactory supports creating an instance
 *  of this object.
 */
#define CLSID_CRMAClientAuthenticator IID_IRMAClientAuthConversation

#undef  INTERFACE
#define INTERFACE   IRMAClientAuthConversation

DECLARE_INTERFACE_(IRMAClientAuthConversation, IUnknown)
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
     *	Method:
     *	    IRMAClientAuthConversation::MakeResponse
     *	Purpose:
     *	    
     *	    Call this when a challenge is received from the server.
     *	    
     *	    pRequestChallengeHeaders should contain the server challenge.
     *
     */
    STDMETHOD(MakeResponse)
    (
	THIS_
	IRMAClientAuthResponse* pClientAuthResponseRequester,
	IRMARequest*	pRequestChallengeHeaders
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAClientAuthConversation::IsDone
     *	Purpose:
     *	    
     *	    Call this to determine whether the conversation is complete.
     *	    (some protocols have more then one message exchange.)
     *
     */
    STDMETHOD_(BOOL,IsDone)(THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAClientAuthConversation::Authenticated
     *	Purpose:
     *	    
     *	    Call this to signal the authenticator that the conversation 
     *	    just completed succeeded or failed.
     *
     */
    STDMETHOD(Authenticated)(THIS_ BOOL bAuthenticated) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAClientAuthResponse
 *
 *  Purpose:
 *
 *	This is implemented by the client core in order to receive the 
 *	response generated by IRMAClientAuthConversation::MakeResponse
 *
 *  IRMAClientAuthResponse:
 *
 *	{00002802-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAClientAuthResponse,   0x00002802, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAClientAuthResponse

DECLARE_INTERFACE_(IRMAClientAuthResponse, IUnknown)
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
     *	Method:
     *	    IRMAClientAuthResponse::ResponseReady
     *	Purpose:
     *	    
     *	    Reports the success or failure of 
     *	    IRMAClientAuthConversation::MakeResponse
     *
     *	    pRequestResponseHeaders should be the same Request object 
     *	    that was passed into MakeResponse, it should contain
     *	    CString values for each MimeHeader it wishes to send to 
     *	    the Server.
     *
     */
    STDMETHOD(ResponseReady)
    (
	THIS_
	PN_RESULT	ResultStatus,
	IRMARequest*	pRequestResponseHeaders
    ) PURE;

};


/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAServerAuthConversation
 *
 *  Purpose:
 *
 *	This is implemented by a server authenticator in order to perform 
 *	the server side of an authentication protocol.	
 *
 *  IRMAServerAuthConversation:
 *
 *	{00002805-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAServerAuthConversation,   0x00002805, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

/*
 *  The IRMACommonClassFactory supports creating an instance
 *  of this object.
 */
#define CLSID_CRMAServerAuthenticator IID_IRMAServerAuthResponse

#undef  INTERFACE
#define INTERFACE   IRMAServerAuthConversation

DECLARE_INTERFACE_(IRMAServerAuthConversation, IUnknown)
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
     *	Method:
     *	    IRMAServerAuthConversation::MakeChallenge
     *	Purpose:
     *	    
     *	    Call this to create a challenge for a client.  If the request 
     *	    passed in does not contain a respose from the client, then it 
     *	    will generate the initial challenge.
     *
     *	    pRequestResponseHeaders is the request for a secured URL.  If
     *	    this is the initial request for the URL it probably does not
     *	    have any credentials from the client.
     *
     */
    STDMETHOD(MakeChallenge)
    (
	THIS_
	IRMAServerAuthResponse*	pServerAuthResponseRequester,
	IRMARequest*		pRequestResponseHeaders
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAServerAuthConversation::IsAuthenticated
     *	Purpose:
     *	    
     *	    Call this to determine whether the last response from the 
     *	    client completed the authentication successfully.
     *
     */
    STDMETHOD_(BOOL,IsAuthenticated)(THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAServerAuthConversation::GetUserContext
     *	Purpose:
     *	    
     *	    Call this to retrieve the Context of the user that completed
     *	    authentication successfully.
     *
     *	    If successful pUnknownUser is a valid context
     *
     */
    STDMETHOD(GetUserContext)(THIS_ REF(IUnknown*) pUnknownUser) PURE;

};

/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAServerAuthResponse
 *
 *  Purpose:
 *
 *	This is implemented by various server plugins in order to receive the 
 *	challenge generated by IRMAServerAuthConversation::MakeChallenge
 *
 *  IRMAServerAuthResponse:
 *
 *	{00002804-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAServerAuthResponse,   0x00002804, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAServerAuthResponse

DECLARE_INTERFACE_(IRMAServerAuthResponse, IUnknown)
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
     *	Method:
     *	    IRMAServerAuthResponse::ChallengeReady
     *	Purpose:
     *	    
     *	    Reports the success or failure of 
     *	    IRMAServerAuthConversation::MakeChallenge
     *
     *	    pRequestChallengeHeaders should be the same Request object 
     *	    that was passed into MakeChallenge, it should contain
     *	    CString values for each MimeHeader it wishes to send to 
     *	    the client.
     *
     */
    STDMETHOD(ChallengeReady)
    (
	THIS_
	PN_RESULT	ResultStatus,
	IRMARequest*	pRequestChallengeHeaders
    ) PURE;

};


/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAUserContext
 *
 *  Purpose:
 *
 *	This is implemented by a user context in order to provide 
 *	access to information about the currently authenticated user.
 *
 *  IRMAUserContext:
 *
 *	{00002806-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAUserContext,   0x00002806, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAUserContext

DECLARE_INTERFACE_(IRMAUserContext, IUnknown)
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
     *	Method:
     *	    IRMAUserContext::IsMemberOf
     *	Purpose:
     *	    
     *	    Call this to determine whether the authenticated user
     *	    is a member of the specified group.
     *
     */
    STDMETHOD(IsMemberOf)(THIS_ IRMABuffer* pBufferGroupID) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAUserProperties
 *
 *  Purpose:
 *
 *	This is implemented by a user context in order to provide 
 *	access to properties of the currently authenticated user.
 *
 *  IRMAUserProperties:
 *
 *	{00002807-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAUserProperties,   0x00002807, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAUserProperties

DECLARE_INTERFACE_(IRMAUserProperties, IUnknown)
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
     *	Method:
     *	    IRMAUserProperties::GetPrincipalID
     *	Purpose:
     *	    
     *	    Call this to determine the principalID of the authenticated user.
     *
     */
    STDMETHOD(GetPrincipalID)(THIS_ REF(IRMABuffer*) pBufferPrincipalID) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAUserProperties::GetAuthorityName
     *	Purpose:
     *	    
     *	    Call this to determine the authority name that authorized the 
     *	    authenticated user. (realm or domain name)
     *
     */
    STDMETHOD(GetAuthorityName)(THIS_ REF(IRMABuffer*) pBufferAuthorityName) PURE;

};


/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAUserImpersonation
 *
 *  Purpose:
 *
 *	This can be implemented by a user context in order to provide 
 *	the ability to have the server impersonate the currently authenticated
 *	user.
 *
 *  IRMAUserImpersonation:
 *
 *	{00002808-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAUserImpersonation,   0x00002808, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAUserImpersonation

DECLARE_INTERFACE_(IRMAUserImpersonation, IUnknown)
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
     *	Method:
     *	    IRMAUserImpersonation::Start
     *	Purpose:
     *	    
     *	    Call this to impersonate the authenticated user.
     *
     */
    STDMETHOD(Start)(THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAUserImpersonation::Stop
     *	Purpose:
     *	    
     *	    Call this to stop impersonating the authenticated user.
     *
     */
    STDMETHOD(Stop)(THIS) PURE;

};

/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAChallenge
 *
 *  Purpose:
 *
 *	This is implemented by the server core in order to allow 
 *	additional exchanges of information with the client without
 *	creating a new request. (It is stored in the IRMARequest object
 *	and can be retrieved by calling IRMARequestContext::GetRequester()
 *	if it is absent then the protocol that this request was made on 
 *	does not support multi-message authentication (PNA doesn't) )
 *
 *  IRMAChallenge:
 *
 *	{0000280A-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAChallenge,   0x0000280A, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAChallenge

DECLARE_INTERFACE_(IRMAChallenge, IUnknown)
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
     *	Method:
     *	    IRMAChallenge::SendChallenge
     *	Purpose:
     *	    
     *	    Call this to request additional information from the client.
     *
     *	    pRequestChallenge should be the same Request object 
     *	    that was passed into MakeChallenge, it should contain
     *	    CString values for each MimeHeader it wishes to send to 
     *	    the client.
     *
     */
    STDMETHOD(SendChallenge)
    (
	THIS_
	IRMAChallengeResponse* pChallengeResponseSender,
	IRMARequest* pRequestChallenge
    ) PURE;

};

/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAChallengeResponse
 *
 *  Purpose:
 *
 *	This is implemented by a server authenticator in order to 
 *	receive the Response returned by the client in response to 
 *	IRMAChallenge::SendChallenge.
 *
 *  IRMAChallengeResponse:
 *
 *	{00002809-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAChallengeResponse,   0x00002809, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAChallengeResponse

DECLARE_INTERFACE_(IRMAChallengeResponse, IUnknown)
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
     *	Method:
     *	    IRMAChallengeResponse::ResponseReady
     *	Purpose:
     *	    
     *	    Called this to return the additional information requested 
     *	    from IRMAChallenge::SendChallenge.
     *
     *	    pRequestResponse should be the same Request object 
     *	    that was passed into MakeChallenge and SendChallenge.
     *
     */
    STDMETHOD(ResponseReady)
    (
	THIS_
	IRMARequest* pRequestResponse
    ) PURE;

};

#endif //!_RMAAUTHN_H_
