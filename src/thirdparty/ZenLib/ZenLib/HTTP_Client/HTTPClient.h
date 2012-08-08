
#ifndef _HTTP_CLIENT
#define _HTTP_CLIENT

#include "HTTPClientWrapper.h"
#include "HTTPClientCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Section      : HTTP API global definitions
    // Last updated : 01/09/2005
    //
    ///////////////////////////////////////////////////////////////////////////////
#ifdef _HTTP_BUILD_AMT
#define CMSI_HTTPCLIENT_PROTOCOL_GUID {0x471b2c0e, 0x6137, 0x4d55, 0x92, 0x36, 0xdd, 0x0f, 0xdb, 0xc2, 0x52, 0xfb}
#endif

    // Debug hook
    // #define  _HTTP_DEBUGGING_                    // Simply dumps more debugging data to the console

    // API Version
#define HTTP_CLIENT_VERSION_MINOR           0
#define HTTP_CLIENT_VERSION_MAJOR           1

    // Global default sizes
#define HTTP_CLIENT_MAX_SEND_RECV_HEADERS   1024        // Maximum Send and receive buffers size
#define HTTP_CLIENT_INIT_SEND_RECV_HEADERS  2048        // If we can resize the buffers this would be the initial size

#define HTTP_CLIENT_MAX_USERNAME_LENGTH     16          // Maximum length the user name (host and proxy authentication)
#define HTTP_CLIENT_MAX_PASSWORD_LENGTH     16          // Maximum length  for the password
    // Maximum length for the base 64 encoded credentials (twice the size of the user name and password max parameters)
#define HTTP_CLIENT_MAX_64_ENCODED_CRED     ((HTTP_CLIENT_MAX_USERNAME_LENGTH + HTTP_CLIENT_MAX_PASSWORD_LENGTH) * 2) + 4
#define HTTP_CLIENT_MAX_CHUNK_HEADER        64          // Maximum length for the received chunk header (hex - string) size
#define HTTP_CLIENT_MAX_PROXY_HOST_LENGTH   64          // Maximum length for the proxy host name
#define HTTP_CLIENT_MAX_TOKEN_LENGTH        512         // Maximum length for an HTTP token data (authentication header elements)
#define HTTP_CLIENT_MAX_TOKEN_NAME_LENGTH   32          // Maximum length for an HTTP authorization token name ("qop")
#define HTTP_CLIENT_MAX_HEADER_SEARCH_CLUE  1024          // Maximum length for a search clue string (Headers searching)
#define HTTP_CLIENT_ALLOW_HEAD_VERB         0           // Can we use the HTTP HEAD verb in our outgoing requests?

#define HTTP_CLIENT_MEMORY_RESIZABLE        FALSE       // Permission to dynamically resize the headers buffer
#define HTTP_CLIENT_MEMORY_RESIZE_FACTOR    16          // Factor for memory resizing operation

#define HTTP_CLIENT_DEFAULT_PORT            80          // Default HTTP port
#define HTTP_CLIENT_DEFAULT_SSL_PORT        443         // Default HTTPS port
#define HTTP_CLIENT_DEFAULT_VERB            0           // GET
#define HTTP_CLIENT_DEFAULT_VER             "HTTP/1.1"  // We will send this in the outgoing header
#define HTTP_CLIENT_DEFAULT_PROXY_VER       "HTTP/1.0"  // We will send this in the outgoing header (proxy)
#define HTTP_CLIENT_DEFAULT_AGENT           "Mozilla/4.0 (compatible; MSIE 5.5; Windows NT 5.0)"
#define HTTP_CLIENT_DEFAULT_TIMEOUT         30          // Default timeout in seconds
#define HTTP_CLIENT_DEFAULT_KEEP_ALIVE      30          // Default Keep-alive value in seconds
#define HTTP_CLIENT_DEFAULT_DIGEST_AUTH     "MD5"       // This is for bypassing a known bug in AMT05..
#define HTTP_CLIENT_DEFAULT_PROXY_AUTH      1           // Basic

#define HTTP_CLIENT_CRLF                    "\r\n"      // End of line macro
#define HTTP_CLIENT_CRLFX2                  "\r\n\r\n"  // Double End of line macro

    // HTTP Session internal API flags
    // Note: Not intended to be set the by the API user
#define HTTP_CLIENT_FLAG_SECURE             0x00000010 // The session is secured using TLS
#define HTTP_CLIENT_FLAG_URLANDPORT         0x00000020 // Url has a port within
#define HTTP_CLIENT_FLAG_URLHTTPS           0x00000040 // Url has a https prefix
#define HTTP_CLIENT_FLAG_USINGPROXY         0x00000080 // Operation will be performed using a proxy server
#define HTTP_CLIENT_FLAG_CHUNKED            0x00000100 // The incoming data is chunked

    // HTTP Status codes
#define HTTP_STATUS_OK                              200 // The request has succeeded
#define HTTP_STATUS_UNAUTHORIZED                    401 // The request requires user authentic
#define HTTP_STATUS_PROXY_AUTHENTICATION_REQUIRED   407 // The client must first authenticate itself with the proxy

    // Redirection (Note: there are more 30x codes, those are the most popular)
#define HTTP_STATUS_OBJECT_MOVED                    302 // Page redirection notification
#define HTTP_STATUS_OBJECT_MOVED_PERMANENTLY        301 // Page redirection notification
#define HTTP_STATUS_CONTINUE                        100 // Page continue message


    // MIN AMX macro
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
    // HTTP timeout macro for selecting the default value if the caller passed 0 (no timeout) to the function
#define HTTP_TIMEOUT(nTimeout) (((nTimeout) > (0)) ? (nTimeout) : (HTTP_CLIENT_DEFAULT_TIMEOUT))

    // 32 bit alignment macro
#define ALIGN(size) ((size & 0xfffffffc) + ((size & 3) ? 4 : 0))


#ifdef _HTTP_DEBUGGING_
    typedef VOID _stdcall E_HTTPDebug(const char *,const char*,UINT32,char *,...); // HTTPDebug hook function
#endif


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Section      : HTTP API internals structures
    // Last updated : 01/09/2005
    //
    ///////////////////////////////////////////////////////////////////////////////

    // Generic parameter structure contains a pointer to the buffer and its length

    typedef struct _HTTP_PARAM
    {

        CHAR                *pParam;
        UINT32              nLength;

    } HTTP_PARAM;

    // HTTP socket events
    typedef struct _HTTP_CONNECTION
    {

        fd_set              FDRead;             // socket read event
        fd_set              FDWrite;            // socket write event
        fd_set              FDError;            // socket error event
        INT32               HttpSocket;         // The underling socket
        UINT32              HttpStartTime;      // Time stamp for the session
        UINT32              HttpClientPort;     // For client side binding
        BOOL                TlsNego;            // TLS negotiation flag

    } HTTP_CONNECTION;

    // Request URL
    typedef struct _HTTP_URL
    {

        HTTP_PARAM           UrlBsee;           // a pointer and length to the "http" section of the URL
        HTTP_PARAM           UrlHost;           // a pointer and length to the host section of the URL
        HTTP_PARAM           UrlPort;           // a pointer and length to the PORT (if was specified section)
        HTTP_PARAM           UrlRequest;        // a pointer and length of the request section of the URL
        UINT16               nPort;             // the PORT that we should use (could be default or the one found within the URL)
        CHAR                 Url[HTTP_CLIENT_MAX_URL_LENGTH];   // a buffer for the URL

    }HTTP_URL;
    // HTTP headers (incoming and outgoing)
    typedef struct _HTTP_HEADERS
    {

        HTTP_PARAM           HeadersBuffer;     // a pointer and length of the complete Headers (in\out) buffer
        HTTP_PARAM           HeadersOut;        // a pointer and length of the outgoing HTTP headers
        HTTP_PARAM           HeadersIn;         // a pointer and length of the incoming headers
        HTTP_PARAM           HeaderSearch;      // Index and pointer for the header search functions
        HTTP_VERB            HttpVerb;          // the HTTP verb that was used in the session
        HTTP_VERB            HttpLastVerb;      // the HTTP verb that was last transmited to the server
        CHAR                 SearchClue[HTTP_CLIENT_MAX_HEADER_SEARCH_CLUE];
        CHAR                 Verb[16];          // the actual string buffer of the HTTP verb


    }HTTP_HEADERS;

    // HTTP headers (parsed headers information)
    typedef struct _HTTP_HEADERS_INFO
    {
        HTTP_PARAM           HttpRedirectURL;       // Stores the redirection URL if we got a 301 or 303 return code
        UINT32               nHTTPStatus;           // the HTTP status code (200 401 407 act')
        UINT32               nHTTPContentLength;    // the Content length if specified of the returned data
        UINT32               nHTTPPostContentLength;// the Content-Length of the POSTed data (if known)
        BOOL                 Connection;            // True = Keep alive or undefined, False = Closed
        BOOL                 ValidHeaders;          // a flag that indicates if the incoming header ware parsed OK and found to be valid
        BOOL                 HaveCredentials;       // a flag that indicates if we have credentials for the session
        CHAR                 HTTPVersion[16];       // HTTP version string buffer (for example: "HTTP 1.1")

    }HTTP_HEADERS_INFO;

    // Authentication parameters that ware extracted from the incoming headers
    typedef struct _HTTP_AUTH_HEADER
    {

        HTTP_PARAM          AuthHeader;             // the pointer and length of the authentication header
        UINT32              HTTP_AUTH_SCHEMA;       // Its schema (could be any of the supported)

    }HTTP_AUTH_HEADER;
    // Proxy related data
    typedef struct _HTTP_PROXY
    {
        CHAR                ProxyHost[HTTP_CLIENT_MAX_PROXY_HOST_LENGTH];
        CHAR                ProxtUser[HTTP_CLIENT_MAX_USERNAME_LENGTH];
        CHAR                ProxyPassword[HTTP_CLIENT_MAX_PASSWORD_LENGTH];
        UINT16              nProxyPort;
        CHAR                AuthSchemaName[16];  // The authentication schema name (for string comperission)
        HTTP_AUTH_SCHEMA    ProxyAuthSchema;

    }HTTP_PROXY;

    // HTTP User credentials
    typedef struct _HTTP_CREDENTIALS
    {

        CHAR                CredUser[HTTP_CLIENT_MAX_USERNAME_LENGTH];
        CHAR                CredPassword[HTTP_CLIENT_MAX_PASSWORD_LENGTH];
        CHAR                AuthSchemaName[16];  // The authentication schema name (for string comperission)
        HTTP_AUTH_SCHEMA    CredAuthSchema;      // The schema that calle has selected for the session
        BOOL                Authentication;      // a flag that indicates that this session has requested a user authentication

    }HTTP_CREDENTIALS;
    // HTTP Counters
    typedef struct _HTTP_COUNTERS
    {

        UINT32              nRecivedHeaderLength;   // Bytes count of the incoming header
        UINT32              nRecivedBodyLength;     // Bytes count of the incoming body length
        UINT32              nRecivedChunkLength;    // The next chunk length in bytes
        UINT32              nBytesToNextChunk;      // How many bytes we have to read until we can expect the next chunk
        UINT32              nActionStartTime;       // Operation start time
        UINT32              nActionTimeout;         // Timeout for the session
        UINT32              nSentChunks;            // Count of sent chunks
        UINT32              nSentBodyBytes;         // Count of body bytes that ware sent
        UINT32              nSentHeaderBytes;       // Count of header bytes thhat ware sent

    }HTTP_COUNTERS;

    // HTTP Client Session data
    typedef struct _HTTP_REQUEST
    {

        HTTP_URL            HttpUrl;
        HTTP_HEADERS        HttpHeaders;
        HTTP_HEADERS_INFO   HttpHeadersInfo;
        HTTP_AUTH_HEADER    HttpAuthHeader;
        HTTP_PROXY          HttpProxy;
        HTTP_CREDENTIALS    HttpCredentials;
        HTTP_CONNECTION     HttpConnection;
        HTTP_COUNTERS       HttpCounters;
        UINT32              HttpState;
        UINT32              HttpFlags;
#ifdef _HTTP_DEBUGGING_
        E_HTTPDebug         *pDebug;
#endif
    } HTTP_SESSION, *P_HTTP_SESSION;


    // HTTP Type Definitions
    typedef UINT32          HTTP_SESSION_HANDLE;
    typedef UINT32          HTTP_CLIENT_SESSION_FLAGS;


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Section      : HTTP API public interface
    // Last updated : 01/09/2005
    //
    ///////////////////////////////////////////////////////////////////////////////


    HTTP_SESSION_HANDLE     HTTPClientOpenRequest         (HTTP_CLIENT_SESSION_FLAGS Flags);
    UINT32                  HTTPClientCloseRequest        (HTTP_SESSION_HANDLE *pSession);
    UINT32                  HTTPClientSetLocalConnection  (HTTP_SESSION_HANDLE pSession, UINT32 nPort);
    UINT32                  HTTPClientSetAuth             (HTTP_SESSION_HANDLE pSession, HTTP_AUTH_SCHEMA AuthSchema, void *pReserved);
    UINT32                  HTTPClientSetCredentials      (HTTP_SESSION_HANDLE pSession, CHAR *pUserName, CHAR *pPassword);
    UINT32                  HTTPClientSetProxy            (HTTP_SESSION_HANDLE pSession, CHAR *pProxyName, UINT16 nPort, CHAR *pUserName, CHAR *pPassword);
    UINT32                  HTTPClientSetVerb             (HTTP_SESSION_HANDLE pSession, HTTP_VERB HttpVerb);
    UINT32                  HTTPClientAddRequestHeaders   (HTTP_SESSION_HANDLE pSession, CHAR *pHeaderName, CHAR *pHeaderData, BOOL nInsert);
    UINT32                  HTTPClientSendRequest         (HTTP_SESSION_HANDLE pSession, CHAR *pUrl, VOID *pData, UINT32 nDataLength, BOOL TotalLength, UINT32 nTimeout,UINT32 nClientPort);
    UINT32                  HTTPClientWriteData           (HTTP_SESSION_HANDLE pSession, VOID *pBuffer, UINT32 nBufferLength, UINT32 nTimeout);
    UINT32                  HTTPClientRecvResponse        (HTTP_SESSION_HANDLE pSession, UINT32 nTimeout);
    UINT32                  HTTPClientReadData            (HTTP_SESSION_HANDLE pSession, VOID *pBuffer, UINT32 nBytesToRead, UINT32 nTimeout, UINT32 *nBytesRecived);
    UINT32                  HTTPClientGetInfo             (HTTP_SESSION_HANDLE pSession, HTTP_CLIENT *HTTPClient);

    UINT32                  HTTPClientFindFirstHeader     (HTTP_SESSION_HANDLE pSession, CHAR *pSearchClue,CHAR *pHeaderBuffer, UINT32 *nLength);
    UINT32                  HTTPClientGetNextHeader       (HTTP_SESSION_HANDLE pSession, CHAR *pHeaderBuffer, UINT32 *nLength);
    UINT32                  HTTPClientFindCloseHeader     (HTTP_SESSION_HANDLE pSession);


#ifdef _HTTP_DEBUGGING_
    UINT32                  HTTPClientSetDebugHook        (HTTP_SESSION_HANDLE pSession,E_HTTPDebug *pDebug);
#endif


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Section      : HTTP API private function
    // Last updated : 01/09/2005
    //
    ///////////////////////////////////////////////////////////////////////////////

    UINT32                  HTTPIntrnResizeBuffer         (P_HTTP_SESSION pHTTPSession, UINT32 nNewSize);
    UINT32                  HTTPIntrnSetURL               (P_HTTP_SESSION pHTTPSession, CHAR *pUrl,UINT32 nUrlLength);
    UINT32                  HTTPIntrnConnectionClose      (P_HTTP_SESSION pHTTPSession);
    UINT32                  HTTPIntrnConnectionOpen       (P_HTTP_SESSION pHTTPSession);
    UINT32                  HTTPIntrnGetRemoteHeaders     (P_HTTP_SESSION pHTTPSession);
    UINT32                  HTTPIntrnGetRemoteChunkLength (P_HTTP_SESSION pHTTPSession);
    UINT32                  HTTPIntrnSend                 (P_HTTP_SESSION pHTTPSession, CHAR *pData,UINT32 *nLength);
    UINT32                  HTTPIntrnRecv                 (P_HTTP_SESSION pHTTPSession, CHAR *pData,UINT32 *nLength,BOOL PeekOnly);
    UINT32                  HTTPIntrnParseAuthHeader      (P_HTTP_SESSION pHTTPSession);
    UINT32                  HTTPIntrnAuthHandler          (P_HTTP_SESSION pHTTPSession);
    UINT32                  HTTPIntrnAuthSendDigest       (P_HTTP_SESSION pHTTPSession);
    UINT32                  HTTPIntrnAuthSendBasic        (P_HTTP_SESSION pHTTPSession);
    UINT32                  HTTPIntrnAuthenticate         (P_HTTP_SESSION pHTTPSession);
    UINT32                  HTTPIntrnHeadersAdd           (P_HTTP_SESSION pHTTPSession, CHAR *pHeaderName, UINT32 nNameLength, CHAR *pHeaderData, UINT32 nDataLength);
    UINT32                  HTTPIntrnHeadersRemove        (P_HTTP_SESSION pHTTPSession, CHAR *pHeaderName);
    UINT32                  HTTPIntrnHeadersReceive       (P_HTTP_SESSION pHTTPSession, UINT32 nTimeout);
    UINT32                  HTTPIntrnHeadersSend          (P_HTTP_SESSION pHTTPSession, HTTP_VERB HttpVerb);
    UINT32                  HTTPIntrnHeadersParse         (P_HTTP_SESSION pHTTPSession);
    UINT32                  HTTPIntrnHeadersFind          (P_HTTP_SESSION pHTTPSession, CHAR *pHeaderName, HTTP_PARAM *pParam,BOOL IncommingHeaders,UINT32 nOffset);
    UINT32                  HTTPIntrnSessionReset         (P_HTTP_SESSION pHTTPSession, BOOL EntireSession);
    UINT32                  HTTPIntrnSessionGetUpTime     (VOID);
    BOOL                    HTTPIntrnSessionEvalTimeout   (P_HTTP_SESSION pHTTPSession);

#ifdef __cplusplus
}
#endif

#endif //_HTTP_CLIENT
