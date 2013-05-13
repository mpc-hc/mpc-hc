#ifndef MD5_H
#define MD5_H

#if defined(_MSC_VER) && _MSC_VER < 1600
    typedef unsigned __int32  uint32_t;
#else
    #include <stdint.h>
#endif

struct MD5Context {
	uint32_t buf[4];
	uint32_t bits[2];
	unsigned char in[64];
};

void MD5Init(struct MD5Context *context);
void MD5Update(struct MD5Context *context, unsigned char const *buf, unsigned len);
void MD5Final(unsigned char digest[16], struct MD5Context *context);
void MD5Transform(uint32_t buf[4], uint32_t const in[16]);

#endif /* !MD5_H */
