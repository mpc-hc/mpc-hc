#ifndef BUFFER_H
#define BUFFER_H

#include <stdarg.h> /* for va_list */

struct Buffer
{
	char * str;

	int len;  /* current length of the string str (without the null terminator) */
	int size; /* number of bytes allocated for str. size > len */
};

void BufferInit(struct Buffer *b);
void BufferVprintf(struct Buffer *b, const char * format, va_list marker);
void BufferPrintf (struct Buffer *b, const char * format, ...);
void BufferDone(struct Buffer *b);

#endif
