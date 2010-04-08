#ifndef _CHAR_T_H_
#define _CHAR_T_H_

#include <stdio.h>
#include <time.h>
#include <wchar.h>

#undef _l
#ifdef UNICODE
typedef wchar_t char_t;
#define tsprintf swprintf
#define tsnprintf _snwprintf
#define tfprintf fwprintf
#define tsscanf swscanf
#define __l(x) L ## x
#define _l(x) __l(x)
#else
typedef char char_t;
#define tsprintf sprintf
#define tsnprintf _snprintf
#define tfprintf fprintf
#define tsscanf sscanf
#define _l(x) x
#endif

#ifdef __cplusplus

#ifdef __GNUC__
#ifndef __forceinline
#define __forceinline __attribute__((__always_inline__)) inline
#endif
#endif

static __forceinline wchar_t* strcat(wchar_t *a, const wchar_t *b)
{
    return wcscat(a, b);
}
static __forceinline wchar_t* strncat(wchar_t *a, const wchar_t *b, size_t c)
{
    return wcsncat(a, b, c);
}
static __forceinline int strcmp(const wchar_t *a, const wchar_t *b)
{
    return wcscmp(a, b);
}
static __forceinline int strncmp(const wchar_t *a, const wchar_t *b, size_t c)
{
    return wcsncmp(a, b, c);
}
static __forceinline int strnicmp(const wchar_t *a, const wchar_t *b, size_t c)
{
    return _wcsnicmp(a, b, c);
}
static __forceinline wchar_t* strncpy(wchar_t *a, const wchar_t *b, size_t c)
{
    return wcsncpy(a, b, c);
}
static __forceinline long strtol(const wchar_t *a, wchar_t **b, int c)
{
    return wcstol(a, b, c);
}
static __forceinline wchar_t* strchr(const wchar_t *a, wchar_t b)
{
    return (wchar_t*)wcschr(a, b);
}
static __forceinline int _strnicmp(const wchar_t *a, const wchar_t *b, size_t c)
{
    return _wcsnicmp(a, b, c);
}
static __forceinline wchar_t* strstr(const wchar_t *a, const wchar_t *b)
{
    return (wchar_t*)wcsstr(a, b);
}
static __forceinline size_t strlen(const wchar_t *a)
{
    return wcslen(a);
}
static __forceinline wchar_t* strcpy(wchar_t *a, const wchar_t *b)
{
    return wcscpy(a, b);
}
static __forceinline int atoi(const wchar_t *a)
{
    return _wtoi(a);
}
static __forceinline wchar_t* _itoa(int a, wchar_t *b, int c)
{
    return _itow(a, b, c);
}
static __forceinline int stricmp(const wchar_t *a, const wchar_t *b)
{
    return  _wcsicmp(a, b);
}
static __forceinline unsigned long strtoul(const wchar_t *a, wchar_t **b, int c)
{
    return wcstoul(a, b, c);
}
static __forceinline double strtod(const wchar_t *a, wchar_t **b)
{
    return wcstod(a, b);
}
static __forceinline int vsprintf(wchar_t *a, const wchar_t *b, va_list c)
{
    return vswprintf(a, b, c);
}
static __forceinline int _vsnprintf(wchar_t *a, size_t b, const wchar_t *c, va_list d)
{
    return _vsnwprintf(a, b, c, d);
}
static __forceinline void _splitpath(const wchar_t *a, wchar_t *b, wchar_t *c, wchar_t *d, wchar_t *e)
{
    _wsplitpath(a, b, c, d, e);
}
static __forceinline void _makepath(wchar_t *a, const wchar_t *b, const wchar_t *c, const wchar_t *d, const wchar_t *e)
{
    _wmakepath(a, b, c, d, e);
}
static __forceinline FILE* fopen(const wchar_t *a, const wchar_t *b)
{
    return _wfopen(a, b);
}
static __forceinline int fputs(const wchar_t *a, FILE *b)
{
    return fputws(a, b);
}
static __forceinline int _stricoll(const wchar_t *a, const wchar_t *b)
{
    return _wcsicoll(a, b);
}
static __forceinline wchar_t* strrchr(const wchar_t *a, wchar_t b)
{
    return (wchar_t*)wcsrchr(a, b);
}
static __forceinline wchar_t* strupr(wchar_t *a)
{
    return _wcsupr(a);
}
static __forceinline wchar_t* strlwr(wchar_t *a)
{
    return _wcslwr(a);
}
static __forceinline wchar_t* _strtime(wchar_t *a)
{
    return _wstrtime(a);
}
static __forceinline wchar_t* memchr(wchar_t *a, wchar_t b, size_t c)
{
    return wmemchr(a, b, c);
}
static __forceinline const wchar_t* memchr(const wchar_t *a, wchar_t b, size_t c)
{
    return wmemchr(a, b, c);
}
static __forceinline wchar_t* strpbrk(const wchar_t *a, const wchar_t *b)
{
    return (wchar_t*)wcspbrk(a, b);
}

template<class Tout> struct text
// ANSI <--> UNICODE conversion.
// in     : input  string in char* or wchar_t*.
// inlen  : count of characters, not byte size!! -1 for null terminated string.
// out    : output string in char* or wchar_t*.
// outlen : count of characters, not byte size!!
{
private:
    Tout *buf;
    bool own;
public:
    template<class Tin> inline text(const Tin *in);
    template<class Tin> inline text(const Tin *in, Tout *Ibuf);
    template<class Tin> inline text(const Tin *in, int inlen, Tout *Ibuf, size_t outlen);
    ~text()
    {
        if(own) delete []buf;
    }
    operator const Tout*() const
    {
        return buf;
    }
};


template<> template<> inline text<char>::text(const char *in): buf(const_cast<char*>(in)), own(false) {}
template<> template<> inline text<char>::text(const char *in, char *Ibuf): buf(strcpy(Ibuf, in)), own(false) {}
template<> template<> inline text<char>::text(const char *in, int inlen, char *Ibuf, size_t outlen): own(false)
{
    if(inlen != -1)
        buf = strncpy(Ibuf, in, ((size_t)inlen < outlen ? inlen : outlen));
    else
    {
        size_t inlen1 = strlen(in) + 1;
        buf = strncpy(Ibuf, in, inlen1 < outlen ? inlen1 : outlen);
    }
}


template<> template<> inline text<wchar_t>::text(const wchar_t *in): buf(const_cast<wchar_t*>(in)), own(false) {}
template<> template<> inline text<wchar_t>::text(const wchar_t *in, wchar_t *Ibuf): buf(strcpy(Ibuf, in)), own(false) {}
template<> template<> inline text<wchar_t>::text(const wchar_t *in, int inlen, wchar_t *Ibuf, size_t outlen): own(false)
{
    if(inlen != -1)
        buf = strncpy(Ibuf, in, ((size_t)inlen < outlen ? inlen : outlen));
    else
    {
        size_t inlen1 = strlen(in) + 1;
        buf = strncpy(Ibuf, in, inlen1 < outlen ? inlen1 : outlen);
    }
}


template<> template<> inline text<wchar_t>::text(const char *in): own(in ? true : false)
{
    if(in)
    {
        size_t l = strlen(in);
        buf = new wchar_t[l+1];
        MultiByteToWideChar(CP_ACP, 0, in, int(l + 1), buf, int(l + 1));
    }
    else
        buf = NULL;
}
template<> template<> inline text<wchar_t>::text(const char *in, wchar_t *Ibuf): own(false), buf(Ibuf)
{
    size_t l = strlen(in);
    MultiByteToWideChar(CP_ACP, 0, in, int(l + 1), buf, int(l + 1));
}
template<> template<> inline text<wchar_t>::text(const char *in, int inlen, wchar_t *Ibuf, size_t outlen): own(false), buf(Ibuf)
{
    MultiByteToWideChar(CP_ACP, 0, in, int(inlen), buf, int(outlen));
}


template<> template<> inline text<char>::text(const wchar_t *in): own(in ? true : false)
{
    if(in)
    {
        size_t l = strlen(in);
        int length = WideCharToMultiByte(CP_ACP, 0, in, int(l + 1), buf, 0, NULL, NULL);
        buf = new char[length+1];
        WideCharToMultiByte(CP_ACP, 0, in, int(l + 1), buf, int(length + 1), NULL, NULL);
    }
    else
        buf = NULL;
}
template<> template<> inline text<char>::text(const wchar_t *in, char *Ibuf): own(false), buf(Ibuf)
{
    size_t l = strlen(in);
    int length = WideCharToMultiByte(CP_ACP, 0, in, int(l + 1), buf, 0, NULL, NULL);
    WideCharToMultiByte(CP_ACP, 0, in, int(l + 1), buf, length, NULL, NULL);
}
template<> template<> inline text<char>::text(const wchar_t *in, int inlen, char *Ibuf, size_t outlen): own(false), buf(Ibuf)
{
    WideCharToMultiByte(CP_ACP, 0, in, int(inlen), buf, int(outlen), NULL, NULL);
}

#endif

#endif
