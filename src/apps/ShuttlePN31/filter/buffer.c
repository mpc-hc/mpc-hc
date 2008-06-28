#include "buffer.h"
#include "mykdprint.h"

#include <stdio.h> /* for NULL */
#include <wdm.h>   /* for PAGE_SIZE */

void BufferInit(struct Buffer *b)
{
	b->str  = NULL;
	b->len  = 0;
	b->size = 0;
}

/*
	BufferPrintf : like printf() but the output string goes to a buffer
	that is reallocated dynamically. So there is no compile-time limit
	to the length of the output string.

	Implementation details : to avoid reallocation for only few bytes. 'size'
	keeps the size of the allocated buffer. Its size grows by a multiple of
	PAGE_SIZE.
*/

void BufferVprintf(struct Buffer *b, const char *format,va_list marker)
{
	int nlen, nsize ;
	char * nstr;

	nlen = MyVsprintfLen(format,marker);
	if (nlen < 0)
	{
		/* The format string is invalid. So we print nothing!	*/
		return ;
	}

	if (b->len + nlen + 1 > b->size)
	{
		nsize = ((b->len + nlen + 1 + PAGE_SIZE -1) / PAGE_SIZE) * PAGE_SIZE;
		nstr = ExAllocatePool(NonPagedPool, nsize);
		if (nstr == NULL) return ;

		/* copy and free the previous memory block if there was one */
		if (b->str != NULL)
		{
			RtlMoveMemory(nstr, b->str,b->len+1);
			ExFreePool(b->str);
		}

		b->str = nstr;
		b->size = nsize;
	}


	MyVsprintf(b->str + b->len,format,marker);
	b->len += nlen;
}

void BufferPrintf(struct Buffer *b, const char *format, ...)
{
	va_list marker;

	va_start(marker,format);
	BufferVprintf(b,format,marker);
	va_end(marker);
}

/*
	Implementation detail: since we reinitilize the str, len and size field,
	this function can be called multiple time
*/

void BufferDone(struct Buffer *b)
{
	if (b->str != NULL && b->size != 0)
	{
		ExFreePool(b->str);

		b->str  = NULL;
		b->len  = 0;
		b->size = 0;
	}
}
