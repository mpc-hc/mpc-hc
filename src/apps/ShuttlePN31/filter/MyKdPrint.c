#include "stddcls.h"
#include "MyKdPrint.h"
#include "Driver.h"


struct MyListEntry
{
	LIST_ENTRY		listEntry;
	PIPE_MSG		Msg;
	char*			str;
};

#pragma LOCKEDCODE
#pragma LOCKEDDATA


KSEMAPHORE mySemaphore;
KSPIN_LOCK mySpinLock;
LIST_ENTRY myListHead;

/*
  myEvent is used to synchronize the destruction of the thread. It is
  first create in a non signaled state. It's signalled when the thread
  is about to call PsTerminateSystemThread().
*/
KEVENT			myEvent;
PVOID			myThreadObjectPointer = NULL;

LARGE_INTEGER	myPerformanceCounter;
LARGE_INTEGER	performanceFrequency;

// convert the first 4 bit into a char like '0' ...'9', 'A' ... 'F'

long GetRelativeTime()
{
	LARGE_INTEGER	lRelTime;
		
	// Windows 2000 DDK: Callers of KeQueryPerformanceCounter can be running at any IRQL.
	KeQuerySystemTime(&lRelTime);
	return (LONG)((lRelTime.QuadPart - myPerformanceCounter.QuadPart)
								/ (performanceFrequency.QuadPart/1000));
}


const char HexaChar(unsigned int dw)
{
	dw = dw & 0xf;
	if (dw < 10)
		return '0' + dw;
	return 'a' + dw - 10;
}

/*
  MyVsprintfLen : return the length of the string produced by a
  printf().  The length does not include the null terminated
  character. It returns -1 if the format string is invalid (like %C)
*/

int MyVsprintfLen(const char * format, va_list marker)
{
	const char * src;
	unsigned int dw;
	int entier;
	const char * str;
	int diviseur, quotient, reste;

	int r = 0;

	for (src=format;*src!=0;src++)
	{
		if (*src != '%')
		{
			r ++;
			continue;
		}

		switch (* ++src)
		{
		case 'p':
		case 'x':
			dw = va_arg(marker,unsigned int);
			r += 8;
			break;
		case 'd':
			entier = va_arg(marker,int);

			// if 'entier' is negative, make it positive
			if (entier < 0)
			{
				entier = - entier;
				r ++;
			}

			diviseur = 1;
			while (10 * diviseur <= entier)
				diviseur = diviseur * 10;

			do {
				quotient = entier / diviseur;
				reste = entier - diviseur * quotient;
				r ++;

				diviseur = diviseur / 10;
				entier = reste;
			} while (diviseur > 0);

			break;
		case 's':
			str = va_arg(marker,const char *);
			while (*str++!=0)
				r ++;
			break;
		default:
			dw = va_arg(marker,unsigned int);			
			return -1;
			break;
		}
	}

	return r;
}

/*
	MyVsprintf : format a string into 'dst' acording to the format
	in 'format'. Warning : no checking is done on the length of 'dst'.
*/

void MyVsprintf(char *dst, const char * format,va_list marker)
{
	const char * src;
	unsigned int dw;
	int entier;
	const char * str;
	int diviseur, quotient, reste;

	for (src=format;*src!=0;src++)
	{
		if (*src != '%')
		{
			*dst ++ = *src;
			continue;
		}

		switch (* ++src)
		{
		case 'p':
		case 'x':
			dw = va_arg(marker,unsigned int);
			*dst ++ = HexaChar(dw>>28);
			*dst ++ = HexaChar(dw>>24);
			*dst ++ = HexaChar(dw>>20);
			*dst ++ = HexaChar(dw>>16);
			*dst ++ = HexaChar(dw>>12);
			*dst ++ = HexaChar(dw>> 8);
			*dst ++ = HexaChar(dw>> 4);
			*dst ++ = HexaChar(dw    );
			break;
		case 'd':
			entier = va_arg(marker,int);

			// if 'entier' is negative, make it positive
			if (entier < 0)
			{
				entier = - entier;
				*dst ++ = '-';
			}


			diviseur = 1;
			while (10 * diviseur <= entier)
				diviseur = diviseur * 10;

			do {
				quotient = entier / diviseur;
				reste = entier - diviseur * quotient;
				*dst ++ = '0' + quotient;

				diviseur = diviseur / 10;
				entier = reste;
			} while (diviseur > 0);

			break;
		case 's':
			str = va_arg(marker,const char *);
			while (*str!=0)
				*dst ++ = *str ++;
			break;
		default:
			dw = va_arg(marker,unsigned int);			
//			DbgPrint("Unhandle type %c\n",*src);
			return ;
			break;
		}

		// int i = va_arg(marker, thetype=int);
	}
	* dst = 0;
}

void MySprintf(char *dst, const char * format, ...)
{
	va_list marker;

	va_start(marker,format);
	MyVsprintf(dst,format,marker);
	va_end(marker);
}

void MyThreadFunction(PVOID arg)
{
	HANDLE				hLog		= NULL;
	BOOLEAN				bContinue	= TRUE;
	OBJECT_ATTRIBUTES	objectAttributes;
	UNICODE_STRING		unicodeObjectName;
	ANSI_STRING			ansiObjectName;
	IO_STATUS_BLOCK		ioStatusBlock;
	NTSTATUS			status;
	struct MyListEntry*	pEntry;

	// set the same priority as the lower driver
	KeSetPriorityThread(KeGetCurrentThread(),12);

	RtlInitAnsiString(&ansiObjectName, DRV_PIPENAME);
	status = RtlAnsiStringToUnicodeString(&unicodeObjectName,&ansiObjectName,TRUE);
	InitializeObjectAttributes(&objectAttributes,&unicodeObjectName,0,NULL,NULL);

	while (bContinue)
	{
		// Wait next message to write into named pipe
		status = KeWaitForSingleObject(&mySemaphore,Executive,KernelMode,FALSE,NULL);
		pEntry = (struct MyListEntry *)ExInterlockedRemoveHeadList(&myListHead,&mySpinLock);
		if (pEntry == NULL)
			continue ;

		if (pEntry->str != NULL)
		{
			// Open the pipe (or reopen if broken)
			if (hLog == NULL)
			{
				status = ZwCreateFile(&hLog,FILE_GENERIC_WRITE|FILE_GENERIC_READ,
					&objectAttributes,&ioStatusBlock,NULL,FILE_ATTRIBUTE_NORMAL,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					FILE_OVERWRITE_IF,FILE_SYNCHRONOUS_IO_NONALERT,NULL,0);
				if (status != STATUS_SUCCESS) hLog = NULL;
			}

			// Write header then message
			if (hLog != NULL)
			{
				status = ZwWriteFile(hLog,NULL,NULL,NULL,&ioStatusBlock,
					(PVOID)&pEntry->Msg,sizeof(pEntry->Msg),NULL,NULL);
				if (status != STATUS_SUCCESS) hLog = NULL;
				else if (pEntry->Msg.lSize > 0)
				{
					status = ZwWriteFile(hLog,NULL,NULL,NULL,&ioStatusBlock,
						(PVOID)pEntry->str,pEntry->Msg.lSize,NULL,NULL);
				}
			}

			ExFreePool(pEntry->str);
		}
		else
		{
			// if we receive an entry with a NULL string, this is the signal to
			// stop this thread
			bContinue = FALSE;
		}

		ExFreePool(pEntry);
	}

	// close the file if necessary
	if (hLog != (HANDLE)NULL)
	{
		status = ZwClose(hLog);
		if (status != STATUS_SUCCESS)
			DbgPrint("ZwClose failed : status = 0x%x\n",status);
	}

	DbgPrint("Thread terminating ...\n");

	KeSetEvent(&myEvent,0,FALSE);

	DbgPrint("Thread is dying\n");
	PsTerminateSystemThread(STATUS_SUCCESS);
}


void MyThreadStart()
{
	HANDLE hThread;
	NTSTATUS status;

	DbgPrint("MyThreadStart()\n");

	KeInitializeSemaphore(&mySemaphore,0,MAX_SEMAPHORE);

	// initialize a double-linked list
	KeInitializeSpinLock(&mySpinLock);
	InitializeListHead(&myListHead);

	KeInitializeEvent(&myEvent, NotificationEvent, FALSE);

	myPerformanceCounter = KeQueryPerformanceCounter(&performanceFrequency);

	/*
		We create a kernel thread.
		From Windows 2000 DDK:
		Callers of this routine must be running at IRQL PASSIVE_LEVEL.
	*/

	status = PsCreateSystemThread(&hThread,THREAD_ALL_ACCESS,NULL,NULL,NULL,
		MyThreadFunction,NULL);
	if (status != STATUS_SUCCESS)
		DbgPrint("PsCreateSystemThread=0x%x\n",status);

	// the following code is compiled only in .sys file for
	// Windows 2000/XP platform because this code make the .sys
	// file unloadable under Windows 98

#ifdef WIN2K
	// we keep an "object pointer" on the thread, for waiting on it later
	ObReferenceObjectByHandle(hThread,THREAD_ALL_ACCESS,NULL,KernelMode,
		&myThreadObjectPointer,NULL);
#endif

//	status = ZwClose(hThread);
	if (status != STATUS_SUCCESS)
		DbgPrint("ZwClose=0x%x\n",status);

	DbgPrint("MyThreadStart() : OK\n");
}

void MyThreadStop()
{
	// allocate a new list entry
	struct MyListEntry * pEntry;
	NTSTATUS status;

	DbgPrint("MyThreadStop()\n");
	
	pEntry = (struct MyListEntry *)ExAllocatePool(NonPagedPool,
		sizeof(struct MyListEntry));
	if (pEntry == NULL)
	{
		DbgPrint("Failed to allocate pEntry. Can't stop the thread!\n");
		return ;
	}

	pEntry->str = NULL;

	// add the new entry to the tail of the list, this is used to tell
	// the thread to end its infinite loop

	ExInterlockedInsertTailList(&myListHead,&pEntry->listEntry,&mySpinLock);
	KeReleaseSemaphore(&mySemaphore,0,1,FALSE);

	// wait for the thread to be destroy
	status = KeWaitForSingleObject(&myEvent,Executive,KernelMode,FALSE,NULL);
	if (status != STATUS_SUCCESS)
		DbgPrint("KeWaitForSingleEvent=%x\n",status);

	DbgPrint("retour de KeWaitForSingleEvent\n");

	// windows xp: without this code, a BSOD occur when the driver is unloaded, because
	// of DRIVER_UNLOADER_WITH_PENDING_OPERATIONS (or similar)

#ifdef WIN2K
	status = KeWaitForSingleObject(myThreadObjectPointer,Executive,KernelMode,FALSE,NULL);
	if (status != STATUS_SUCCESS)
		DbgPrint("KeWaitForSingleThread=%x\n",status);

	DbgPrint("retour de KeWaitForSingleThread\n");

	ObDereferenceObject(myThreadObjectPointer);
#endif

	DbgPrint("MyThreadStop() : OK\n");
}


/* the LogXXX API implementation */

void LogInit()
{
	MyThreadStart();
}

void LogDone()
{
	MyThreadStop();
}

void LogPrintf(const char * format, ...)
{
	struct Buffer b;
	va_list marker;

	va_start(marker,format);

	BufferInit(&b);
	BufferVprintf(&b,format,marker);
	LogBuffer(&b);
	BufferDone(&b);

	va_end(marker);
}

void LogBuffer(struct Buffer * b)
{
	if (b->str == NULL)
		return;

	// windows98: we limit the number of character sent to the debugger
	// since this can cause BSOD.

#if 0
	DbgPrint("LogBuffer(str=%p, len=%d)\n",b->str,b->len);

	if (b->len > 100)
		b->len = 100;

	b->str[b->len] = 0;

	DbgPrint(b->str);
#endif

	if (SendToPipe(PC_LOG, strlen(b->str), b->str))
	{
		/*
			If InternalPrintStr succeds, that means the buffer if no longer
			our property. So we reset the field in 'b'
		*/

		b->str  = NULL;
		b->len  = 0;
		b->size = 0;
	}
}

/*
	InternalPrintStr : send a string to the log file (it is first sent to
	the thread, that will log to the file)

	Returned value is 1 (success, in this case the string will be freed later)
	or 0 (failure)
*/
int SendToPipe (PIPE_COMMAND Cmd, long lSize, char* pBuff)
{
	struct MyListEntry*	pEntry;
	LONG				semaphoreCount;
	KIRQL				currentIrql;

	// check the semaphore counter
	semaphoreCount = KeReadStateSemaphore(&mySemaphore);
	if (!(semaphoreCount < MAX_SEMAPHORE)) return 0;
		
	// allocate a new list entry
	pEntry = (struct MyListEntry *)ExAllocatePool(NonPagedPool,sizeof(struct MyListEntry));
	if (pEntry == NULL) return 0;

	// copy the string in the newly created entry
	pEntry->Msg.nCode	= Cmd;
	pEntry->Msg.lSize	= lSize;
	pEntry->Msg.lTime	= GetRelativeTime();

	pEntry->str			= ExAllocatePool(NonPagedPool, lSize);
	RtlMoveMemory (pEntry->str, pBuff, lSize);

	// add the new entry to the tail of the list
	ExInterlockedInsertTailList(&myListHead,&pEntry->listEntry,&mySpinLock);
	semaphoreCount = KeReleaseSemaphore(&mySemaphore,0,1,FALSE);

	return 1;
}