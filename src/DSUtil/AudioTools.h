#pragma once

#pragma warning(disable: 4005)
#include <stdint.h>
#pragma warning(default: 4005)

void gain_uint8 (const double factor, const size_t allsamples, uint8_t* pData);
void gain_int16 (const double factor, const size_t allsamples, int16_t* pData);
void gain_int24 (const double factor, const size_t allsamples, BYTE*    pData);
void gain_int32 (const double factor, const size_t allsamples, int32_t* pData);
void gain_float (const double factor, const size_t allsamples, float*   pData);
void gain_double(const double factor, const size_t allsamples, double*  pData);
