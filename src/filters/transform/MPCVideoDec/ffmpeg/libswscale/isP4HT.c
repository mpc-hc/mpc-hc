// Included from swscale.c, if HAVE_THREADS is defined.

//#include <string.h>
//#include "config.h"

// int IsCPUID() call to see if CPUID is supported
int IsCPUID(void)
{
#ifdef WIN64
    return 0;
#else
    long result;
    result = 1;
#if ARCH_X86 // GCC
    asm volatile(
        "pushf                  \n\t"
        "pop    %%edx           \n\t"
        "mov    %%edx,%%ecx     \n\t"
        "xor    $0x200000,%%edx \n\t"
        "push   %%edx           \n\t"
        "popf                   \n\t"
        "pushf                  \n\t"
        "pop    %%edx           \n\t"
        "cmp    %%ecx,%%edx     \n\t"
        "jnz    1f              \n\t"
        "mov    $0,%0           \n\t"
        "1:                     \n\t"

        : "+a"(result)
        :: "%ecx", "%edx"
    );
#endif
    return result;
#endif
}

int isP4HT_I(int family, int model, const char *v_name) // return true if P4HT, P4EE or P4
{
    if(!strncmp("GenuineIntel", v_name, 12) && family == 0x0f && model <= 3)
        return 1;
    return 0;
}

int isP4HT(void)
{
#ifdef WIN64
    return 0;
#else
    long dwStandard = 0;
    long dwBrandIndex = 0;
    long dwFeature = 0;
    long dwMax = 0;
    long dwExt = 0;
    union
    {
        char cBuf[12+1];
        struct
        {
            long dw0;
            long dw1;
            long dw2;
        } s;
    } Ident;
    char v_name[13];                    // vendor name
    int family;                         // family of the processor
    // e.g. 6 = Pentium-Pro architecture
    int model;                          // model of processor
    // e.g. 1 = Pentium-Pro for family = 6
    int stepping;                       // processor revision number

    if(!IsCPUID())
        return 0;

#if ARCH_X86 // GCC
    asm volatile(
        // get the vendor string
        "xor    %%eax, %%eax \n\t"
        "cpuid               \n\t"
        "mov    %%eax, %0    \n\t"
        "mov    %%ebx, %1    \n\t"
        "mov    %%edx, %2    \n\t"
        "mov    %%ecx, %3    \n\t"

        // get the Standard bits
        "mov    $1, %%eax    \n\t"
        "cpuid               \n\t"
        "mov    %%eax, %4    \n\t"
        "mov    %%ebx, %5    \n\t"
        "mov    %%edx, %6    \n\t"

        : "=m"(dwMax), "=m"(Ident.s.dw0), "=m"(Ident.s.dw1), "=m"(Ident.s.dw2), "=m"(dwStandard), "=m"(dwBrandIndex), "=m"(dwFeature)
        :: "%eax", "%ebx", "%ecx", "%edx"
    );
#ifndef _WIN64
#elif defined(_WIN32)
    _asm
    {
        push ebx
        push ecx
        push edx

        // get the vendor string
        xor eax, eax
        cpuid
        mov dwMax, eax
        mov Ident.s.dw0, ebx
        mov Ident.s.dw1, edx
        mov Ident.s.dw2, ecx

        // get the Standard bits
        mov eax, 1
        cpuid
        mov dwStandard, eax
        mov dwBrandIndex, ebx
        mov dwFeature, edx

        pop ecx
        pop ebx
        pop edx
    }
#else
#endif
    return 0;
#endif

    family = (dwStandard >> 8) & 0xF; // retrieve family
    model = (dwStandard >> 4) & 0xF;  // retrieve model
    stepping = (dwStandard) & 0xF;    // retrieve stepping
    Ident.cBuf[12] = 0;
    strcpy(v_name, Ident.cBuf);
    if((dwFeature & 0x8000000) == 0)
        return 0;

    if(((dwBrandIndex & 0xff0000) >> 16) == 1)
        return 0;

    return isP4HT_I(family, model, v_name);
#endif
}
