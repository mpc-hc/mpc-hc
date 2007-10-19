#ifndef _SKL_DCT_H_
#define _SKL_DCT_H_

void Skl_IDct16_Put_C(uint8_t *dest/*align 8*/, int line_size, DCTELEM *block/*align 16*/);
void Skl_IDct16_Add_C(uint8_t *dest/*align 8*/, int line_size, DCTELEM *block/*align 16*/);
void Skl_IDct16_C(DCTELEM *block);

void Skl_IDct16_Put_SSE2(uint8_t *dest/*align 8*/, int line_size, DCTELEM *block/*align 16*/);
void Skl_IDct16_Put_SSE (uint8_t *dest/*align 8*/, int line_size, DCTELEM *block/*align 16*/);
void Skl_IDct16_Put_MMX (uint8_t *dest/*align 8*/, int line_size, DCTELEM *block/*align 16*/);
void Skl_IDct16_Add_SSE2(uint8_t *dest/*align 8*/, int line_size, DCTELEM *block/*align 16*/);
void Skl_IDct16_Add_SSE (uint8_t *dest/*align 8*/, int line_size, DCTELEM *block/*align 16*/);
void Skl_IDct16_Add_MMX (uint8_t *dest/*align 8*/, int line_size, DCTELEM *block/*align 16*/);
void Skl_IDct16_SSE2(DCTELEM *block);
void Skl_IDct16_SSE (DCTELEM *block);
void Skl_IDct16_MMX (DCTELEM *block);

#endif
