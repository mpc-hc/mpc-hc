#pragma once

extern void CSSdisckey(unsigned char *dkey,const unsigned char *pkey);
extern void CSStitlekey(unsigned char *tkey,const unsigned char *dkey);
extern void CSSdescramble(unsigned char *sector,const unsigned char *tkey);

extern const unsigned char g_PlayerKeys[][6];
extern const int g_nPlayerKeys;
