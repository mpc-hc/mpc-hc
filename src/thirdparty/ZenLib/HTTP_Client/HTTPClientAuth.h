
#ifndef HTTP_CLIENT_AUTH_H
#define HTTP_CLIENT_AUTH_H

#include "HTTPClientWrapper.h"  // Cross platform support


#define HASHLEN         16
#define HASHHEXLEN      32
#define IN
#define OUT

typedef char HASH[HASHLEN];
typedef char HASHHEX[HASHHEXLEN+1];
typedef unsigned long uint32;

// Base 64 Related 
#define DECODE64(c)  (isascii(c) ? base64val[c] : BAD)
#define BAD     -1

static const char base64digits[] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const char base64val[] = {
BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD,
BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD,
BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD, 62, BAD,BAD,BAD, 63,
52, 53, 54, 55,  56, 57, 58, 59,  60, 61,BAD,BAD, BAD,BAD,BAD,BAD,
BAD,  0,  1,  2,   3,  4,  5,  6,   7,  8,  9, 10,  11, 12, 13, 14,
15, 16, 17, 18,  19, 20, 21, 22,  23, 24, 25,BAD, BAD,BAD,BAD,BAD,
BAD, 26, 27, 28,  29, 30, 31, 32,  33, 34, 35, 36,  37, 38, 39, 40,
41, 42, 43, 44,  45, 46, 47, 48,  49, 50, 51,BAD, BAD,BAD,BAD,BAD
};

void HTTPBase64Encoder(unsigned char *out, const unsigned char *in, int inlen);
int  HTTPBase64Decoder(char *out, const char *in);


// Digest Related
// Generates a 32 byte random hexadecimal string such as "4f6ba982..."
void HTTPDigestGenerateCNonce(char *outbuff);

// Calculate H(A1) as per HTTP Digest spec 
void HTTPDigestCalcHA1(
                   IN int    nAlg,     /* 0 = MD5, 1 = MD5-Sess */
                   IN char * pszUserName,
                   IN char * pszRealm,
                   IN int    nRealmLength,
                   IN char * pszPassword,
                   IN char * pszNonce,
                   IN int    nNonceLength,
                   IN char * pszCNonce,
                   OUT HASHHEX SessionKey
                   );

// Calculate request-digest/response-digest as per HTTP Digest spec 
void HTTPDigestCalcResponse(
                        IN HASHHEX HA1,             // H(A1) 
                        IN char * pszNonce,         // nonce from server 
                        IN int    nNonceLength,     // Length of nonce
                        IN char * pszNonceCount,    // 8 hex digits 
                        IN char * pszCNonce,        // client nonce 
                        IN char * pszQop,           // qop-value: "", "auth", "auth-int" 
                        IN int    nQopLength,       // qop param length
                        IN char * pszMethod,        // method from the request 
                        IN char * pszDigestUri,     // requested URL 
                        IN int    nDigestUriLebgth, // Uri Length
                        IN HASHHEX HEntity,         // H(entity body) if qop="auth-int"
                        OUT HASHHEX Response        // request-digest or response-digest 
                        );

// MD5 structures and functions 
struct MD5Context 
{
    uint32 buf[4];
    uint32 bits[2];
    unsigned char in[64];
};

void HTTPMD5Init        (struct MD5Context *context);
void HTTPMD5Update      (struct MD5Context *context, unsigned char const *buf,unsigned len);
void HTTPMD5Final       (unsigned char digest[16], struct MD5Context *context);
void HTTPMD5Transform   (uint32 buf[4], uint32 const in[16]);


// This is needed to make RSAREF happy on some MS-DOS compilers.
typedef struct MD5Context MD5_CTX;

#endif 

