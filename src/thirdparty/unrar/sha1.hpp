#ifndef _RAR_SHA1_
#define _RAR_SHA1_

#define HW 5

typedef struct {
    uint32 state[5];
    uint32 count[2];
    unsigned char buffer[64];

    unsigned char workspace[64]; // Temporary buffer.
} sha1_context;

void sha1_init( sha1_context * c );
void sha1_process( sha1_context * c, const byte *data, size_t len,
                   bool handsoff);
void sha1_done( sha1_context * c, uint32[HW], bool handsoff);

#endif
