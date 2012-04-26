#ifndef CPUDETECT_H
#define CPUDETECT_H

#ifdef __cplusplus
extern "C" {
#endif 

#define CPUTYPE_I386    3
#define CPUTYPE_I486    4
#define CPUTYPE_I586    5
#define CPUTYPE_I686    6

typedef struct cpucaps_s {
    int hasMMX;
    int hasMMX2;
    int has3DNow;
    int has3DNowExt;
    int hasSSE;
    int hasSSE2;
    int hasSSSE3;
    int isP4HT;      // -1: not initialized, 0: non P4Ht, 1: p4HT
} CpuCaps;

extern CpuCaps gCpuCaps;

#ifdef __cplusplus
}
#endif 

#endif /* !CPUDETECT_H */
