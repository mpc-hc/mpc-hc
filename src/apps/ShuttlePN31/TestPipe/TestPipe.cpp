// TestPipe.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <atlbase.h>
#include <atltrace.h>
#include <windows.h> 
#include <stdio.h> 
#include <tchar.h>
#include <conio.h>

#include "ShuttlePN31Client.h"

#define BUFSIZE 4096
 
VOID InstanceThread(LPVOID); 
VOID GetAnswerToRequest(BYTE*, DWORD); 


 
int _tmain(VOID) 
{
	CShuttlePN31Client		PN31;

//	PN31.Install();
	PN31.Connect();
	printf ("PN31 is ready...\n");
	getch();
	PN31.Disconnect();
/*
   BOOL fConnected; 
   DWORD dwThreadId; 
   HANDLE hPipe, hThread; 
   LPTSTR lpszPipename = TEXT(APP_PIPENAME); 
 
// The main loop creates an instance of the named pipe and 
// then waits for a client to connect to it. When the client 
// connects, a thread is created to handle communications 
// with that client, and the loop is repeated. 
 
   for (;;) 
   { 
      hPipe = CreateNamedPipe( 
          lpszPipename,             // pipe name 
          PIPE_ACCESS_DUPLEX,       // read/write access 
          PIPE_TYPE_MESSAGE |       // message type pipe 
          PIPE_READMODE_MESSAGE |   // message-read mode 
          PIPE_WAIT,                // blocking mode 
          PIPE_UNLIMITED_INSTANCES, // max. instances  
          BUFSIZE,                  // output buffer size 
          BUFSIZE,                  // input buffer size 
          NMPWAIT_USE_DEFAULT_WAIT, // client time-out 
          NULL);                    // default security attribute 

      if (hPipe == INVALID_HANDLE_VALUE) 
      {
          printf("CreatePipe failed"); 
          return 0;
      }
 
      // Wait for the client to connect; if it succeeds, 
      // the function returns a nonzero value. If the function returns 
      // zero, GetLastError returns ERROR_PIPE_CONNECTED. 
 
      fConnected = ConnectNamedPipe(hPipe, NULL) ? 
         TRUE : (GetLastError() == ERROR_PIPE_CONNECTED); 
 
      if (fConnected) 
      { 
      // Create a thread for this client. 
         hThread = CreateThread( 
            NULL,              // no security attribute 
            0,                 // default stack size 
            (LPTHREAD_START_ROUTINE) InstanceThread, 
            (LPVOID) hPipe,    // thread parameter 
            0,                 // not suspended 
            &dwThreadId);      // returns thread ID 

         if (hThread == NULL) 
         {
            printf("CreateThread failed"); 
            return 0;
         }
         else CloseHandle(hThread); 
       } 
      else 
        // The client could not connect, so close the pipe. 
         CloseHandle(hPipe); 
   } */
   return 1; 
} 
/* 
VOID InstanceThread(LPVOID lpvParam) 
{ 
   BYTE		chRequest[BUFSIZE]; 
   TCHAR	chReply[BUFSIZE]; 
   DWORD	cbBytesRead, cbReplyBytes, cbWritten; 
   BOOL		fSuccess; 
   HANDLE	hPipe;
   PIPE_MSG	Msg;
 
// The thread's parameter is a handle to a pipe instance. 
 
   hPipe = (HANDLE) lpvParam; 
 
   while (1) 
   { 
   // Read client requests from the pipe. 
      fSuccess = ReadFile(hPipe, &Msg, sizeof(Msg), &cbBytesRead, NULL);
	  if (!fSuccess) break;

	  if (Msg.lSize > 0)
		fSuccess = ReadFile(hPipe, &chRequest, min(Msg.lSize, sizeof(chRequest)), &cbBytesRead, NULL);

	  switch (Msg.nCode)
	  {
		case PC_LOG :
			chRequest[cbBytesRead] = 0;
			printf ("[L] %s", chRequest);
			break;
		case PC_BULK_DOWN :
			PN31_KEYCODE* pKey = FindPN31Key(chRequest);
			if (pKey != NULL) printf ("%s\n", pKey->strName);
			for (int i=1; i<8; i++)
			{
				if (chRequest[i] != 0)
				{
					GetAnswerToRequest (chRequest, cbBytesRead);
					break;
				}
			}
			break;
	  }

//      if (! fSuccess || cbBytesRead == 0) 
  //       break; 
//      GetAnswerToRequest(chRequest, cbBytesRead); 
//	  printf ((char*)chRequest);

  } 
 
// Flush the pipe to allow the client to read the pipe's contents 
// before disconnecting. Then disconnect the pipe, and close the 
// handle to this pipe instance. 
 
   FlushFileBuffers(hPipe); 
   DisconnectNamedPipe(hPipe); 
   CloseHandle(hPipe); 
}

VOID GetAnswerToRequest(BYTE* chRequest, DWORD cbBytesRead)
{
	for (int i=0; i<cbBytesRead; i++)
	{
		printf ("%02x ", chRequest[i]);
		ATLTRACE ("%02x ", chRequest[i]);
	}
   printf("\n");
   ATLTRACE ("\n");
}

*/