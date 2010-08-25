
///////////////////////////////////////////////////////////////////////////////
//
// Module Name:                                                                
//   CmsiHTTPClientCommon.h                                                    
//                                                                             
// Abstract: Coomon structs and types for the HTTP protocol API                
// Author:	Eitan Michaelso                                                     
// Version:  1.0                                                               
//                                                                             
///////////////////////////////////////////////////////////////////////////////

#ifndef _HTTPCLIENT_PROTOCOL_H_
#define _HTTPCLIENT_PROTOCOL_H_

#ifdef __cplusplus 
extern "C" { 
#endif

    // Global default sizes
#define HTTP_CLIENT_MAX_URL_LENGTH          512         // Maximum length for an HTTP Url parameter

    // HTTP Session flags (Public flags)
#define HTTP_CLIENT_FLAG_KEEP_ALIVE         0x00000001 // Set the keep alive header
#define HTTP_CLIENT_FLAG_SEND_CHUNKED       0x00000002 // The outgoing should chunked
#define HTTP_CLIENT_FLAG_NO_CACHE           0x00000004 // Set the no cache header
#define HTTP_CLIENT_FLAG_ASYNC              0x00000008 // Currently not implemented 

    // HTTP status internal flags
#define HTTP_CLIENT_STATE_PRE_INIT          0x00000000  // Starting stage
#define HTTP_CLIENT_STATE_INIT              0x00000001  // API was initialized (memory was allocated)
#define HTTP_CLIENT_STATE_URL_PARSED        0x00000002  // Url was parsed
#define HTTP_CLIENT_STATE_HOST_CONNECTED    0x00000004  // HEAD verb was sent
#define HTTP_CLIENT_STATE_HEAD_SENT         0x00000008  // Post verb was sent
#define HTTP_CLIENT_STATE_POST_SENT         0x00000010  // HTTP requet was sent
#define HTTP_CLIENT_STATE_REQUEST_SENT      0x00000020  // HTTP request was sent
#define HTTP_CLIENT_STATE_HEADERS_RECIVED   0x00000040  // Headers ware recived from the server
#define HTTP_CLIENT_STATE_HEADERS_PARSED    0x00000080  // HTTP headers ware parsed
#define HTTP_CLIENT_STATE_HEADERS_OK        0x00000100  // Headers  status was OK

    // HTTP Return codes
#define HTTP_CLIENT_SUCCESS                 0 // HTTP Success status

#define HTTP_CLIENT_UNKNOWN_ERROR           1 // Unknown error
#define HTTP_CLIENT_ERROR_INVALID_HANDLE    2  // an Invalid handle or possible bad pointer was passed to a function
#define HTTP_CLIENT_ERROR_NO_MEMORY         3  // Buffer too small or a failure while in memory allocation
#define HTTP_CLIENT_ERROR_SOCKET_INVALID    4  // an attempt to use an invalid socket handle was made
#define HTTP_CLIENT_ERROR_SOCKET_CANT_SET   5  // Can't send socket parameters
#define HTTP_CLIENT_ERROR_SOCKET_RESOLVE    6  // Error while resolving host name
#define HTTP_CLIENT_ERROR_SOCKET_CONNECT    7  // Error while connecting to the remote server
#define HTTP_CLIENT_ERROR_SOCKET_TIME_OUT   8  // socket time out error
#define HTTP_CLIENT_ERROR_SOCKET_RECV       9  // Error while receiving data
#define HTTP_CLIENT_ERROR_SOCKET_SEND       10 // Error while sending data
#define HTTP_CLIENT_ERROR_HEADER_RECV       11 // Error while receiving the remote HTTP headers
#define HTTP_CLIENT_ERROR_HEADER_NOT_FOUND  12 // Could not find element within header
#define HTTP_CLIENT_ERROR_HEADER_BIG_CLUE   13 // The headers search clue was too large for the internal API buffer
#define HTTP_CLIENT_ERROR_HEADER_NO_LENGTH  14 // No content length was specified for the outgoing data. the caller should specify chunking mode in the session creation
#define HTTP_CLIENT_ERROR_CHUNK_TOO_BIG     15 // The HTTP chunk token that was received from the server was too big and possibly wrong
#define HTTP_CLIENT_ERROR_AUTH_HOST         16 // Could not authenticate with the remote host
#define HTTP_CLIENT_ERROR_AUTH_PROXY        17 // Could not authenticate with the remote proxy
#define HTTP_CLIENT_ERROR_BAD_VERB          18 // Bad or not supported HTTP verb was passed to a function
#define HTTP_CLIENT_ERROR_LONG_INPUT        19 // a function received a parameter that was too large
#define HTTP_CLIENT_ERROR_BAD_STATE         20 // The session state prevents the current function from proceeding
#define HTTP_CLIENT_ERROR_CHUNK             21 // Could not parse the chunk length while in chunked transfer
#define HTTP_CLIENT_ERROR_BAD_URL           22 // Could not parse curtail elements from the URL (such as the host name, HTTP prefix act')
#define HTTP_CLIENT_ERROR_BAD_HEADER        23 // Could not detect key elements in the received headers
#define HTTP_CLIENT_ERROR_BUFFER_RSIZE      24 // Error while attempting to resize a buffer
#define HTTP_CLIENT_ERROR_BAD_AUTH          25 // Authentication schema is not supported
#define HTTP_CLIENT_ERROR_AUTH_MISMATCH     26 // The selected authentication schema does not match the server response
#define HTTP_CLIENT_ERROR_NO_DIGEST_TOKEN   27 // an element was missing while parsing the digest authentication challenge
#define HTTP_CLIENT_ERROR_NO_DIGEST_ALG     28 // Digest algorithem could be MD5 or MD5-sess other types are not supported
#define HTTP_CLIENT_ERROR_SOCKET_BIND		  29 // Binding error
#define HTTP_CLIENT_ERROR_TLS_NEGO			  30 // Tls negotiation error
#define HTTP_CLIENT_ERROR_NOT_IMPLEMENTED   64 // Feature is not (yet) implemented
#define HTTP_CLIENT_EOS                     1000        // HTTP end of stream message

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Section      : HTTP API structures
    // Last updated : 01/09/2005
    //
    ///////////////////////////////////////////////////////////////////////////////

    // HTTP Supported authentication methods 
    typedef enum _HTTP_AUTH_SCHEMA
    {
        AuthSchemaNone      = 0,
        AuthSchemaBasic,
        AuthSchemaDigest,
        AuthSchemaKerberos,
        AuthNotSupported

    } HTTP_AUTH_SCHEMA;

    // HTTP supported verbs
    typedef enum _HTTP_VERB
    {
        VerbGet             = 0,
        VerbHead,
        VerbPost,
        VerbNotSupported
        // Note: others verb such as connect and put are currently not supported

    } HTTP_VERB;

    // Data structure that the caller can request at any time that will include some information regarding the session
    typedef struct _HTTP_CLIENT
    {
        UINT32        HTTPStatusCode;                 // HTTP Status code (200 OK)
        UINT32		    RequestBodyLengthSent;          // Total bytes sent (body only)
        UINT32		    ResponseBodyLengthReceived;     // Total bytes received (body only)
        UINT32		    TotalResponseBodyLength;        // as extracted from the “content-length" header
        UINT32        HttpState;
    } HTTP_CLIENT;

#ifdef __cplusplus
}
#endif
  
#endif // _HTTPCLIENT_PROTOCOL_H_
