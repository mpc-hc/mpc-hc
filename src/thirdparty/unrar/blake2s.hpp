// Based on public domain code written in 2012 by Samuel Neves
#ifndef _RAR_BLAKE2_
#define _RAR_BLAKE2_

#if defined(_MSC_VER)
#define BLAKE_ALIGN(x) __declspec(align(x))
#else
#define BLAKE_ALIGN(x) __attribute__((aligned(x)))
#endif

#define BLAKE2_DIGEST_SIZE 32

enum blake2s_constant
{
  BLAKE2S_BLOCKBYTES = 64,
  BLAKE2S_OUTBYTES   = 32
};


// Alignment improves performance of non-SSE version a little
// and is required for SSE version.
BLAKE_ALIGN( 64 )
typedef struct __blake2s_state
{
  uint32 h[8];
  uint32 t[2];
  uint32 f[2];
  byte  buf[2 * BLAKE2S_BLOCKBYTES];
  size_t   buflen;
  byte  last_node;
} blake2s_state ;


#ifdef RAR_SMP
class ThreadPool;
#endif

typedef struct __blake2sp_state
{
  blake2s_state S[8][1];
  blake2s_state R[1];
  byte buf[8 * BLAKE2S_BLOCKBYTES];
  size_t buflen;

#ifdef RAR_SMP
  ThreadPool *ThPool;
  uint MaxThreads;
#endif

} blake2sp_state;

void blake2sp_init( blake2sp_state *S );
void blake2sp_update( blake2sp_state *S, const byte *in, size_t inlen );
void blake2sp_final( blake2sp_state *S, byte *digest );

#endif

