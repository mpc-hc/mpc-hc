
///////////////////////////////////////////////////////////////////////////////
// Module Name:
// HTTPClientAuth.c
//
// Abstract: Handle Digest, MD5 and 64 Bit Encoding
//
// Platform: Any that supports standard C calls
///////////////////////////////////////////////////////////////////////////////

#include "HTTPClientAuth.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function     : HTTPBase64Encoder
// Purpose      : Converts a given string into a base64 encoded buffer.
// Last updated : 01/09/200515/05/2005
// Author Name  : Eitan Michaelson
// Notes        :
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HTTPBase64Encoder(unsigned char *out, const unsigned char *in, int inlen)
// [OUT] out  A pointer to a char to hold the converted string
// [IN]  in  String to convert
// [IN]  inlen  Length of the string to be converted

{
    for (; inlen >= 3; inlen -= 3)
    {
        *out++ = base64digits[in[0] >> 2];
        *out++ = base64digits[((in[0] << 4) & 0x30) | (in[1] >> 4)];
        *out++ = base64digits[((in[1] << 2) & 0x3c) | (in[2] >> 6)];
        *out++ = base64digits[in[2] & 0x3f];
        in += 3;
    }

    if (inlen > 0)
    {
        unsigned char fragment;

        *out++ = base64digits[in[0] >> 2];
        fragment = (in[0] << 4) & 0x30;

        if (inlen > 1)
            fragment |= in[1] >> 4;

        *out++ = base64digits[fragment];
        *out++ = (inlen < 2) ? '=' : base64digits[(in[1] << 2) & 0x3c];
        *out++ = '=';
    }

    *out = '\0';
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function     : HTTPBase64Decoder
// Purpose      : Converts a given base64 string into a bytes buffer.
// Last updated : 01/09/200515/05/2005
// Author Name  : Eitan Michaelson
// Notes        :
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int HTTPBase64Decoder(char *out, const char *in)
{                           // [OUT]  out  Where to save the converted string
    // [IN]   in  String to convert

    int len = 0;
    register unsigned char digit1, digit2, digit3, digit4;

    if (in[0] == '+' && in[1] == ' ')
        in += 2;
    if (*in == '\r')
        return(0);

    do {

        digit1 = in[0];
        if (DECODE64(digit1) == BAD)
            return(-1);
        digit2 = in[1];
        if (DECODE64(digit2) == BAD)
            return(-1);
        digit3 = in[2];
        if (digit3 != '=' && DECODE64(digit3) == BAD)
            return(-1);
        digit4 = in[3];
        if (digit4 != '=' && DECODE64(digit4) == BAD)
            return(-1);
        in += 4;
        *out++ = (DECODE64(digit1) << 2) | (DECODE64(digit2) >> 4);
        ++len;
        if (digit3 != '=')
        {
            *out++ = ((DECODE64(digit2) << 4) & 0xf0) | (DECODE64(digit3) >> 2);
            ++len;
            if (digit4 != '=')
            {
                *out++ = ((DECODE64(digit3) << 6) & 0xc0) | DECODE64(digit4);
                ++len;
            }
        }
    } while (*in && *in != '\r' && digit4 != '=');

    return (len);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Purpose      : The following code implements the calculations of H(A1), H(A2),
//                request-digest and response-digest
// Last updated : 01/09/200515/05/2005
// Author Name  : Public Domain\RFC2617
// Notes        : Digest Access Authentication
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function     : GenerateCNonce
// Purpose      : Generates a 32 byte random hexadecimal string such as "4f6ba982..."
// Last updated : 15/05/2005
// Author Name  : Eitan Michaelson
// Notes        :
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HTTPDigestGenerateCNonce(char *outbuff)
{
    int i,num;
    InitRandomeNumber();
    for(i = 0; i < 32; i++) {
        num = GetRandomeNumber();
        switch(num) {
        case 0: case 1: case 2: case 3: case 4: case 5:
        case 6: case 7: case 8: case 9:
            outbuff[i] = '0' + num;
            break;
        case 10: case 11: case 12: case 13: case 14: case 15:
            outbuff[i] = 'a' + (num-10);
            break;
        default:
            outbuff[i] = 'f';
        }
    }
    outbuff[32] = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function     : DigestCvtHex
// Purpose      : CConvert to HEX
// Last updated : 15/05/2005
// Author Name  : Public Domain\RFC2617
// Notes        :
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void HTTPDigestCvtHex(IN HASH Bin,OUT HASHHEX Hex)
{
    unsigned short i;
    unsigned char j;

    for (i = 0; i < HASHLEN; i++) {
        j = (Bin[i] >> 4) & 0xf;
        if (j <= 9)
            Hex[i*2] = (j + '0');
        else
            Hex[i*2] = (j + 'a' - 10);
        j = Bin[i] & 0xf;
        if (j <= 9)
            Hex[i*2+1] = (j + '0');
        else
            Hex[i*2+1] = (j + 'a' - 10);
    };
    Hex[HASHHEXLEN] = '\0';
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function     : DigestCalcHA1
// Purpose      : Calculate H(A1) as per spec
// Last updated : 15/05/2005
// Author Name  : Public Domain\RFC2617
// Notes        :
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
                       )
{
    MD5_CTX Md5Ctx;
    HASH    HA1;
    HASHHEX HASess;
    HASH    HAll;

    HTTPMD5Init(&Md5Ctx);
    HTTPMD5Update(&Md5Ctx, (const unsigned char *)pszUserName, strlen(pszUserName)); //Daniel casting
    HTTPMD5Update(&Md5Ctx, (const unsigned char *)":", 1);
    HTTPMD5Update(&Md5Ctx, (const unsigned char *)pszRealm, nRealmLength); //Daniel
    HTTPMD5Update(&Md5Ctx, (const unsigned char *)":", 1);
    HTTPMD5Update(&Md5Ctx, (const unsigned char *)pszPassword, strlen(pszPassword)); //Daniel
    HTTPMD5Final((unsigned char *)HA1, &Md5Ctx);

    if (nAlg == 1)  /* MD5-Sess */
    {
        HTTPDigestCvtHex(HA1, HASess);
        HTTPMD5Init(&Md5Ctx);
        HTTPMD5Update(&Md5Ctx, (const unsigned char *)HASess, HASHHEXLEN);
        HTTPMD5Update(&Md5Ctx, (const unsigned char *)":", 1);
        HTTPMD5Update(&Md5Ctx, (const unsigned char *)pszNonce, nNonceLength);
        HTTPMD5Update(&Md5Ctx, (const unsigned char *)":", 1);
        HTTPMD5Update(&Md5Ctx, (const unsigned char *)pszCNonce, strlen(pszCNonce));
        HTTPMD5Final((unsigned char *)HAll, &Md5Ctx);
        HTTPDigestCvtHex(HAll, SessionKey);
        return;

    }

    HTTPDigestCvtHex(HA1, SessionKey);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function     : DigestCalcResponse
// Purpose      : Calculate request-digest/response-digest as per HTTP Digest spec
// Last updated : 15/05/2005
// Author Name  : Public Domain\RFC2617
// Notes        :
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HTTPDigestCalcResponse(
                            IN HASHHEX HA1,             // H(A1)
                            IN char * pszNonce,         // nonce from server
                            IN int    nNonceLength,     // Length of nonce
                            IN char * pszNonceCount,    // 8 hex digits
                            IN char * pszCNonce,        // client nonce */
                            IN char * pszQop,           // qop-value: "", "auth", "auth-int"
                            IN int    nQopLength,       // qop param length
                            IN char * pszMethod,        // method from the request
                            IN char * pszDigestUri,     // requested URL
                            IN int    nDigestUriLebgth, // Uri Length
                            IN HASHHEX HEntity,         // H(entity body) if qop="auth-int"
                            OUT HASHHEX Response        // request-digest or response-digest
                            )
{
    MD5_CTX Md5Ctx;
    HASH HA2;
    HASH RespHash;
    HASHHEX HA2Hex;

    // Calculate H(A2)
    HTTPMD5Init(&Md5Ctx);
    HTTPMD5Update(&Md5Ctx, (const unsigned char *)pszMethod, strlen(pszMethod));
    HTTPMD5Update(&Md5Ctx, (const unsigned char *)":", 1);
    HTTPMD5Update(&Md5Ctx, (const unsigned char *)pszDigestUri, nDigestUriLebgth);
    if (stricmp(pszQop, "auth-int") == 0)
    {

        HTTPMD5Update(&Md5Ctx, (const unsigned char *)":", 1);
        HTTPMD5Update(&Md5Ctx, (const unsigned char *)HEntity, HASHHEXLEN);
    };
    HTTPMD5Final((unsigned char *)HA2, &Md5Ctx);
    HTTPDigestCvtHex(HA2, HA2Hex);

    // Calculate response
    HTTPMD5Init(&Md5Ctx);
    HTTPMD5Update(&Md5Ctx, (const unsigned char *)HA1, HASHHEXLEN);
    HTTPMD5Update(&Md5Ctx, (const unsigned char *)":", 1);
    HTTPMD5Update(&Md5Ctx, (const unsigned char *)pszNonce, nNonceLength);
    HTTPMD5Update(&Md5Ctx, (const unsigned char *)":", 1);
    if (*pszQop)
    {

        HTTPMD5Update(&Md5Ctx, (const unsigned char *)pszNonceCount, strlen(pszNonceCount));
        HTTPMD5Update(&Md5Ctx, (const unsigned char *)":", 1);
        HTTPMD5Update(&Md5Ctx, (const unsigned char *)pszCNonce, strlen(pszCNonce));
        HTTPMD5Update(&Md5Ctx, (const unsigned char *)":", 1);
        HTTPMD5Update(&Md5Ctx, (const unsigned char *)pszQop, nQopLength);
        HTTPMD5Update(&Md5Ctx, (const unsigned char *)":", 1);
    };
    HTTPMD5Update(&Md5Ctx, (const unsigned char *)HA2Hex, HASHHEXLEN);
    HTTPMD5Final((unsigned char *)RespHash, &Md5Ctx);
    HTTPDigestCvtHex(RespHash, Response);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Purpose      : This code implements the MD5 message-digest algorithm.
//                The algorithm is due to Ron Rivest.  This code was
//                written by Colin Plumb in 1993, no copyright is claimed.
//                This code is in the public domain; do with it what you wish.
//                Equivalent code is available from RSA Data Security, Inc.
//                This code has been tested against that, and is equivalent,
//                except that you don't need to include two pages of legalese
//                with every copy.
// Usage        : To compute the message digest of a chunk of bytes, declare an
//                MD5Context structure, pass it to MD5Init, call MD5Update as
//                needed on buffers full of bytes, and then call MD5Final, which
//                will fill a supplied 16-byte array with the digest.
// Last updated : 15/05/2005
// Author Name  : Public Domain
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef HIGHFIRST
#define HTTPMD5ByteReverse(buf, len)    /* Nothing */
#else
void HTTPMD5ByteReverse(unsigned char *buf, unsigned longs);
#ifndef ASM_MD5

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function     : ByteReverse
// Purpose      : Little\Big Endian support
// Gets         :
// Returns      :
// Last updated : 15/05/2005
// Author Name  : Public Domain
// Notes        : this code is harmless on little-endian machines.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HTTPMD5ByteReverse(unsigned char *buf, unsigned longs)
{
    uint32 t;
    do {
        t = (uint32) ((unsigned) buf[3] << 8 | buf[2]) << 16 |
            ((unsigned) buf[1] << 8 | buf[0]);
        *(uint32 *) buf = t;
        buf += 4;
    } while (--longs);
}
#endif
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function     : MD5Init
// Purpose      : Initialize the MD5Context structure
// Gets         : MD5Context structure
// Returns      :
// Last updated : 15/05/2005
// Author Name  : Public Domain
// Notes        : Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious
//                initialization constants.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void HTTPMD5Init(struct MD5Context *ctx)
{
    ctx->buf[0] = 0x67452301;
    ctx->buf[1] = 0xefcdab89;
    ctx->buf[2] = 0x98badcfe;
    ctx->buf[3] = 0x10325476;

    ctx->bits[0] = 0;
    ctx->bits[1] = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function     : MD5Update
// Purpose      : Update the MD5Context structure with the target byte array
// Gets         : MD5Context structure, buffer and length
// Returns      :
// Last updated : 15/05/2005
// Author Name  : Public Domain
// Notes        : Update context to reflect the concatenation of another buffer full of bytes.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HTTPMD5Update(struct MD5Context *ctx, unsigned char const *buf, unsigned len)
{
    uint32 t;

    // Update bitcount

    t = ctx->bits[0];
    if ((ctx->bits[0] = t + ((uint32) len << 3)) < t)
        ctx->bits[1]++;     // Carry from low to high
    ctx->bits[1] += len >> 29;

    t = (t >> 3) & 0x3f;    // Bytes already in shsInfo->data

    // Handle any leading odd-sized chunks

    if (t) {
        unsigned char *p = (unsigned char *) ctx->in + t;

        t = 64 - t;
        if (len < t) {
            memcpy(p, buf, len);
            return;
        }
        memcpy(p, buf, t);
        HTTPMD5ByteReverse(ctx->in, 16);
        HTTPMD5Transform(ctx->buf, (uint32 *) ctx->in);
        buf += t;
        len -= t;
    }
    // Process data in 64-byte chunks

    while (len >= 64) {
        memcpy(ctx->in, buf, 64);
        HTTPMD5ByteReverse(ctx->in, 16);
        HTTPMD5Transform(ctx->buf, (uint32 *) ctx->in);
        buf += 64;
        len -= 64;
    }

    // Handle any remaining bytes of data.

    memcpy(ctx->in, buf, len);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function     : MD5Final
// Purpose      : Finalize.
// Gets         : Output digest structure, MD5Context structure
// Returns      :
// Last updated : 15/05/2005
// Author Name  : Public Domain
// Notes        : Final wrapup - pad to 64-byte boundary with the bit pattern
//                1 0* (64-bit count of bits processed, MSB-first).
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/


void HTTPMD5Final(unsigned char digest[16], struct MD5Context *ctx)
{
    unsigned count;
    unsigned char *p;

    // Compute number of bytes mod 64
    count = (ctx->bits[0] >> 3) & 0x3F;

    // Set the first char of padding to 0x80.  This is safe since there is
    // always at least one byte free
    p = ctx->in + count;
    *p++ = 0x80;

    // Bytes of padding needed to make 64 bytes
    count = 64 - 1 - count;

    // Pad out to 56 mod 64 */
    if (count < 8) {
        // Two lots of padding:  Pad the first block to 64 bytes
        memset(p, 0, count);
        HTTPMD5ByteReverse(ctx->in, 16);
        HTTPMD5Transform(ctx->buf, (uint32 *) ctx->in);

        // Now fill the next block with 56 bytes
        memset(ctx->in, 0, 56);
    } else {
        // Pad block to 56 bytes
        memset(p, 0, count - 8);
    }
    HTTPMD5ByteReverse(ctx->in, 14);

    // Append length in bits and transform
    ((uint32 *) ctx->in)[14] = ctx->bits[0];
    ((uint32 *) ctx->in)[15] = ctx->bits[1];

    HTTPMD5Transform(ctx->buf, (uint32 *) ctx->in);
    HTTPMD5ByteReverse((unsigned char *) ctx->buf, 4);
    memcpy(digest, ctx->buf, 16);
    memset(ctx, 0, sizeof(*ctx));    // In case it's sensitive
}

#ifndef ASM_MD5

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function     :
// Purpose      : The four core functions - F1 is optimized somewhat
// Last updated : 15/05/2005
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

// This is the central step in the MD5 algorithm.
#ifdef __PUREC__
#define MD5STEP(f, w, x, y, z, data, s) \
    ( w += f /*(x, y, z)*/ + data,  w = w<<s | w>>(32-s),  w += x )
#else
#define MD5STEP(f, w, x, y, z, data, s) \
    ( w += f(x, y, z) + data,  w = w<<s | w>>(32-s),  w += x )
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function     : MD5Transform
// Purpose      : The core of the MD5 algorithm, this alters an existing MD5 hash to
//                reflect the addition of 16 longwords of new data.  MD5Update blocks
//                the data and converts bytes into longwords for this routine.
// Last updated : 15/05/2005
// Author Name  : Public Domain
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void HTTPMD5Transform(uint32 buf[4], uint32 const in[16])
{
    register uint32 a, b, c, d;

    a = buf[0];
    b = buf[1];
    c = buf[2];
    d = buf[3];

#ifdef __PUREC__    // PureC Weirdness... (GG)
    MD5STEP(F1(b,c,d), a, b, c, d, in[0] + 0xd76aa478L, 7);
    MD5STEP(F1(a,b,c), d, a, b, c, in[1] + 0xe8c7b756L, 12);
    MD5STEP(F1(d,a,b), c, d, a, b, in[2] + 0x242070dbL, 17);
    MD5STEP(F1(c,d,a), b, c, d, a, in[3] + 0xc1bdceeeL, 22);
    MD5STEP(F1(b,c,d), a, b, c, d, in[4] + 0xf57c0fafL, 7);
    MD5STEP(F1(a,b,c), d, a, b, c, in[5] + 0x4787c62aL, 12);
    MD5STEP(F1(d,a,b), c, d, a, b, in[6] + 0xa8304613L, 17);
    MD5STEP(F1(c,d,a), b, c, d, a, in[7] + 0xfd469501L, 22);
    MD5STEP(F1(b,c,d), a, b, c, d, in[8] + 0x698098d8L, 7);
    MD5STEP(F1(a,b,c), d, a, b, c, in[9] + 0x8b44f7afL, 12);
    MD5STEP(F1(d,a,b), c, d, a, b, in[10] + 0xffff5bb1L, 17);
    MD5STEP(F1(c,d,a), b, c, d, a, in[11] + 0x895cd7beL, 22);
    MD5STEP(F1(b,c,d), a, b, c, d, in[12] + 0x6b901122L, 7);
    MD5STEP(F1(a,b,c), d, a, b, c, in[13] + 0xfd987193L, 12);
    MD5STEP(F1(d,a,b), c, d, a, b, in[14] + 0xa679438eL, 17);
    MD5STEP(F1(c,d,a), b, c, d, a, in[15] + 0x49b40821L, 22);

    MD5STEP(F2(b,c,d), a, b, c, d, in[1] + 0xf61e2562L, 5);
    MD5STEP(F2(a,b,c), d, a, b, c, in[6] + 0xc040b340L, 9);
    MD5STEP(F2(d,a,b), c, d, a, b, in[11] + 0x265e5a51L, 14);
    MD5STEP(F2(c,d,a), b, c, d, a, in[0] + 0xe9b6c7aaL, 20);
    MD5STEP(F2(b,c,d), a, b, c, d, in[5] + 0xd62f105dL, 5);
    MD5STEP(F2(a,b,c), d, a, b, c, in[10] + 0x02441453L, 9);
    MD5STEP(F2(d,a,b), c, d, a, b, in[15] + 0xd8a1e681L, 14);
    MD5STEP(F2(c,d,a), b, c, d, a, in[4] + 0xe7d3fbc8L, 20);
    MD5STEP(F2(b,c,d), a, b, c, d, in[9] + 0x21e1cde6L, 5);
    MD5STEP(F2(a,b,c), d, a, b, c, in[14] + 0xc33707d6L, 9);
    MD5STEP(F2(d,a,b), c, d, a, b, in[3] + 0xf4d50d87L, 14);
    MD5STEP(F2(c,d,a), b, c, d, a, in[8] + 0x455a14edL, 20);
    MD5STEP(F2(b,c,d), a, b, c, d, in[13] + 0xa9e3e905L, 5);
    MD5STEP(F2(a,b,c), d, a, b, c, in[2] + 0xfcefa3f8L, 9);
    MD5STEP(F2(d,a,b), c, d, a, b, in[7] + 0x676f02d9L, 14);
    MD5STEP(F2(c,d,a), b, c, d, a, in[12] + 0x8d2a4c8aL, 20);

    MD5STEP(F3(b,c,d), a, b, c, d, in[5] + 0xfffa3942L, 4);
    MD5STEP(F3(a,b,c), d, a, b, c, in[8] + 0x8771f681L, 11);
    MD5STEP(F3(d,a,b), c, d, a, b, in[11] + 0x6d9d6122L, 16);
    MD5STEP(F3(c,d,a), b, c, d, a, in[14] + 0xfde5380cL, 23);
    MD5STEP(F3(b,c,d), a, b, c, d, in[1] + 0xa4beea44L, 4);
    MD5STEP(F3(a,b,c), d, a, b, c, in[4] + 0x4bdecfa9L, 11);
    MD5STEP(F3(d,a,b), c, d, a, b, in[7] + 0xf6bb4b60L, 16);
    MD5STEP(F3(c,d,a), b, c, d, a, in[10] + 0xbebfbc70L, 23);
    MD5STEP(F3(b,c,d), a, b, c, d, in[13] + 0x289b7ec6L, 4);
    MD5STEP(F3(a,b,c), d, a, b, c, in[0] + 0xeaa127faL, 11);
    MD5STEP(F3(d,a,b), c, d, a, b, in[3] + 0xd4ef3085L, 16);
    MD5STEP(F3(c,d,a), b, c, d, a, in[6] + 0x04881d05L, 23);
    MD5STEP(F3(b,c,d), a, b, c, d, in[9] + 0xd9d4d039L, 4);
    MD5STEP(F3(a,b,c), d, a, b, c, in[12] + 0xe6db99e5L, 11);
    MD5STEP(F3(d,a,b), c, d, a, b, in[15] + 0x1fa27cf8L, 16);
    MD5STEP(F3(c,d,a), b, c, d, a, in[2] + 0xc4ac5665L, 23);

    MD5STEP(F4(b,c,d), a, b, c, d, in[0] + 0xf4292244L, 6);
    MD5STEP(F4(a,b,c), d, a, b, c, in[7] + 0x432aff97L, 10);
    MD5STEP(F4(d,a,b), c, d, a, b, in[14] + 0xab9423a7L, 15);
    MD5STEP(F4(c,d,a), b, c, d, a, in[5] + 0xfc93a039L, 21);
    MD5STEP(F4(b,c,d), a, b, c, d, in[12] + 0x655b59c3L, 6);
    MD5STEP(F4(a,b,c), d, a, b, c, in[3] + 0x8f0ccc92L, 10);
    MD5STEP(F4(d,a,b), c, d, a, b, in[10] + 0xffeff47dL, 15);
    MD5STEP(F4(c,d,a), b, c, d, a, in[1] + 0x85845dd1L, 21);
    MD5STEP(F4(b,c,d), a, b, c, d, in[8] + 0x6fa87e4fL, 6);
    MD5STEP(F4(a,b,c), d, a, b, c, in[15] + 0xfe2ce6e0L, 10);
    MD5STEP(F4(d,a,b), c, d, a, b, in[6] + 0xa3014314L, 15);
    MD5STEP(F4(c,d,a), b, c, d, a, in[13] + 0x4e0811a1L, 21);
    MD5STEP(F4(b,c,d), a, b, c, d, in[4] + 0xf7537e82L, 6);
    MD5STEP(F4(a,b,c), d, a, b, c, in[11] + 0xbd3af235L, 10);
    MD5STEP(F4(d,a,b), c, d, a, b, in[2] + 0x2ad7d2bbL, 15);
    MD5STEP(F4(c,d,a), b, c, d, a, in[9] + 0xeb86d391L, 21);
#else
    MD5STEP(F1, a, b, c, d, in[0] + 0xd76aa478, 7);
    MD5STEP(F1, d, a, b, c, in[1] + 0xe8c7b756, 12);
    MD5STEP(F1, c, d, a, b, in[2] + 0x242070db, 17);
    MD5STEP(F1, b, c, d, a, in[3] + 0xc1bdceee, 22);
    MD5STEP(F1, a, b, c, d, in[4] + 0xf57c0faf, 7);
    MD5STEP(F1, d, a, b, c, in[5] + 0x4787c62a, 12);
    MD5STEP(F1, c, d, a, b, in[6] + 0xa8304613, 17);
    MD5STEP(F1, b, c, d, a, in[7] + 0xfd469501, 22);
    MD5STEP(F1, a, b, c, d, in[8] + 0x698098d8, 7);
    MD5STEP(F1, d, a, b, c, in[9] + 0x8b44f7af, 12);
    MD5STEP(F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
    MD5STEP(F1, b, c, d, a, in[11] + 0x895cd7be, 22);
    MD5STEP(F1, a, b, c, d, in[12] + 0x6b901122, 7);
    MD5STEP(F1, d, a, b, c, in[13] + 0xfd987193, 12);
    MD5STEP(F1, c, d, a, b, in[14] + 0xa679438e, 17);
    MD5STEP(F1, b, c, d, a, in[15] + 0x49b40821, 22);

    MD5STEP(F2, a, b, c, d, in[1] + 0xf61e2562, 5);
    MD5STEP(F2, d, a, b, c, in[6] + 0xc040b340, 9);
    MD5STEP(F2, c, d, a, b, in[11] + 0x265e5a51, 14);
    MD5STEP(F2, b, c, d, a, in[0] + 0xe9b6c7aa, 20);
    MD5STEP(F2, a, b, c, d, in[5] + 0xd62f105d, 5);
    MD5STEP(F2, d, a, b, c, in[10] + 0x02441453, 9);
    MD5STEP(F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
    MD5STEP(F2, b, c, d, a, in[4] + 0xe7d3fbc8, 20);
    MD5STEP(F2, a, b, c, d, in[9] + 0x21e1cde6, 5);
    MD5STEP(F2, d, a, b, c, in[14] + 0xc33707d6, 9);
    MD5STEP(F2, c, d, a, b, in[3] + 0xf4d50d87, 14);
    MD5STEP(F2, b, c, d, a, in[8] + 0x455a14ed, 20);
    MD5STEP(F2, a, b, c, d, in[13] + 0xa9e3e905, 5);
    MD5STEP(F2, d, a, b, c, in[2] + 0xfcefa3f8, 9);
    MD5STEP(F2, c, d, a, b, in[7] + 0x676f02d9, 14);
    MD5STEP(F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);

    MD5STEP(F3, a, b, c, d, in[5] + 0xfffa3942, 4);
    MD5STEP(F3, d, a, b, c, in[8] + 0x8771f681, 11);
    MD5STEP(F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
    MD5STEP(F3, b, c, d, a, in[14] + 0xfde5380c, 23);
    MD5STEP(F3, a, b, c, d, in[1] + 0xa4beea44, 4);
    MD5STEP(F3, d, a, b, c, in[4] + 0x4bdecfa9, 11);
    MD5STEP(F3, c, d, a, b, in[7] + 0xf6bb4b60, 16);
    MD5STEP(F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
    MD5STEP(F3, a, b, c, d, in[13] + 0x289b7ec6, 4);
    MD5STEP(F3, d, a, b, c, in[0] + 0xeaa127fa, 11);
    MD5STEP(F3, c, d, a, b, in[3] + 0xd4ef3085, 16);
    MD5STEP(F3, b, c, d, a, in[6] + 0x04881d05, 23);
    MD5STEP(F3, a, b, c, d, in[9] + 0xd9d4d039, 4);
    MD5STEP(F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
    MD5STEP(F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
    MD5STEP(F3, b, c, d, a, in[2] + 0xc4ac5665, 23);

    MD5STEP(F4, a, b, c, d, in[0] + 0xf4292244, 6);
    MD5STEP(F4, d, a, b, c, in[7] + 0x432aff97, 10);
    MD5STEP(F4, c, d, a, b, in[14] + 0xab9423a7, 15);
    MD5STEP(F4, b, c, d, a, in[5] + 0xfc93a039, 21);
    MD5STEP(F4, a, b, c, d, in[12] + 0x655b59c3, 6);
    MD5STEP(F4, d, a, b, c, in[3] + 0x8f0ccc92, 10);
    MD5STEP(F4, c, d, a, b, in[10] + 0xffeff47d, 15);
    MD5STEP(F4, b, c, d, a, in[1] + 0x85845dd1, 21);
    MD5STEP(F4, a, b, c, d, in[8] + 0x6fa87e4f, 6);
    MD5STEP(F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
    MD5STEP(F4, c, d, a, b, in[6] + 0xa3014314, 15);
    MD5STEP(F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
    MD5STEP(F4, a, b, c, d, in[4] + 0xf7537e82, 6);
    MD5STEP(F4, d, a, b, c, in[11] + 0xbd3af235, 10);
    MD5STEP(F4, c, d, a, b, in[2] + 0x2ad7d2bb, 15);
    MD5STEP(F4, b, c, d, a, in[9] + 0xeb86d391, 21);
#endif

    buf[0] += a;
    buf[1] += b;
    buf[2] += c;
    buf[3] += d;
}

#endif
