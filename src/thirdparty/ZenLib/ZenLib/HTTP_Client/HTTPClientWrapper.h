
#ifndef HTTP_CLIENT_WRAPPER
#define HTTP_CLIENT_WRAPPER

// Compilation mode
#define _HTTP_BUILD_WIN32            // Set Windows Build flag

///////////////////////////////////////////////////////////////////////////////
//
// Section      : Microsoft Windows Support
// Last updated : 01/09/2005
//
///////////////////////////////////////////////////////////////////////////////

#ifdef _HTTP_BUILD_WIN32

#if defined(_MSC_VER)
    #pragma warning (disable: 4996) // 'function': was declared deprecated (VS 2005)
#endif
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#if defined(_WIN32) || defined(WIN32)
    #include <winsock.h>
#endif

// Generic types
typedef unsigned int                 UINT32;
typedef int                          INT32;

// Sockets (Winsock wrapper)
#define                              HTTP_ECONNRESET     (WSAECONNRESET)
#define                              HTTP_EINPROGRESS    (WSAEINPROGRESS)
#define                              HTTP_EWOULDBLOCK    (WSAEWOULDBLOCK)
#endif


///////////////////////////////////////////////////////////////////////////////
//
// Section      : Functions that are not supported by the AMT stdc framework
//                So they had to be specificaly added.
// Last updated : 01/09/2005
//
///////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif

    // STDC Wrapper implimentation
    int                                 HTTPWrapperIsAscii              (int c);
    int                                 HTTPWrapperToUpper              (int c);
    int                                 HTTPWrapperToLower              (int c);
    int                                 HTTPWrapperIsAlpha              (int c);
    int                                 HTTPWrapperIsAlNum              (int c);
    char*                               HTTPWrapperItoa                 (char *buff,int i);
    void                                HTTPWrapperInitRandomeNumber    ();
    long                                HTTPWrapperGetUpTime            ();
    int                                 HTTPWrapperGetRandomeNumber     ();
    int                                 HTTPWrapperGetSocketError       (int s);
    unsigned long                       HTTPWrapperGetHostByName        (char *name,unsigned long *address);
    int                                 HTTPWrapperShutDown             (int s,int in);
    // SSL Wrapper prototypes
    int                                 HTTPWrapperSSLConnect           (int s,const struct sockaddr *name,int namelen,char *hostname);
    int                                 HTTPWrapperSSLNegotiate         (int s,const struct sockaddr *name,int namelen,char *hostname);
    int                                 HTTPWrapperSSLSend              (int s,char *buf, int len,int flags);
    int                                 HTTPWrapperSSLRecv              (int s,char *buf, int len,int flags);
    int                                 HTTPWrapperSSLClose             (int s);
    int                                 HTTPWrapperSSLRecvPending       (int s);
    // Global wrapper Functions
#define                             IToA                            HTTPWrapperItoa
#define                             GetUpTime                       HTTPWrapperGetUpTime
#define                             SocketGetErr                    HTTPWrapperGetSocketError
#define                             HostByName                      HTTPWrapperGetHostByName
#define                             InitRandomeNumber               HTTPWrapperInitRandomeNumber
#define                             GetRandomeNumber                HTTPWrapperGetRandomeNumber

#ifdef __cplusplus
}
#endif

///////////////////////////////////////////////////////////////////////////////
//
// Section      : Global type definitions
// Last updated : 01/09/2005
//
///////////////////////////////////////////////////////////////////////////////

#define VOID                         void
#ifndef NULL
#define NULL                         0
#endif
#define TRUE                         1
#define FALSE                        0
typedef char                         CHAR;
typedef unsigned short               UINT16;
typedef int                          BOOL;
typedef unsigned long                ULONG;

// Global socket structures and definitions
#define                              HTTP_INVALID_SOCKET (-1)
typedef struct sockaddr_in           HTTP_SOCKADDR_IN;
typedef struct timeval               HTTP_TIMEVAL;
typedef struct hostent               HTTP_HOSTNET;
typedef struct sockaddr              HTTP_SOCKADDR;
typedef struct in_addr               HTTP_INADDR;


#endif // HTTP_CLIENT_WRAPPER
