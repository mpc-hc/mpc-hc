#ifndef MYKDPRINT_H
#define MYKDPRINT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include "buffer.h"
#include "../Common/Interface.h"



/* maximum value our semaphore can reach without crashing the machine */
#define MAX_SEMAPHORE 10000

int  MyVsprintfLen(        const char * format,va_list marker);
void MyVsprintf(char *dst, const char * format,va_list marker);
void MySprintf (char *dst, const char * format, ...);

/* following functions are for general use */

void LogInit();
void LogDone();
void LogPrintf(const char * format, ...);
void LogBuffer(struct Buffer * b);

int SendToPipe (PIPE_COMMAND Cmd, long lSize, char* pBuff);

#ifdef __cplusplus
};
#endif

#endif
