/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a zlib-style license that can
 *  be found in the License.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#include "ZenLib/PreComp.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "ZenLib/Conf_Internal.h"
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "ZenLib/Thread.h"
#include <ZenLib/Ztring.h>
#include <ZenLib/CriticalSection.h>
//---------------------------------------------------------------------------

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ZENLIB_USEWX
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef ZENLIB_USEWX
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#include <wx/thread.h>
//---------------------------------------------------------------------------

namespace ZenLib
{

class ThreadEntry : public wxThread
{
public :
    ThreadEntry(Thread* Th_) : wxThread(wxTHREAD_JOINABLE)
        {Th=Th_;};
    void* Entry() {Th->Entry(); return NULL;}
private :
    Thread* Th;
};

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
Thread::Thread()
{
    ThreadPointer=(void*)new ThreadEntry(this);
    ((ThreadEntry*)ThreadPointer)->Create();
}

//---------------------------------------------------------------------------
Thread::~Thread()
{
    delete (ThreadEntry*)ThreadPointer;
}

//***************************************************************************
// Main Entry
//***************************************************************************

void Thread::Entry()
{
}

//***************************************************************************
// Control
//***************************************************************************

void Thread::Run()
{
    ((ThreadEntry*)ThreadPointer)->Resume();
}

void Thread::Pause()
{
    ((ThreadEntry*)ThreadPointer)->Pause();
}

void Thread::Stop()
{
    ((ThreadEntry*)ThreadPointer)->Delete();
}

bool Thread::IsRunning()
{
    return ((ThreadEntry*)ThreadPointer)->IsRunning();
}

//***************************************************************************
// Communicating
//***************************************************************************

void Thread::Sleep(size_t Millisecond)
{
    ((ThreadEntry*)ThreadPointer)->Sleep((unsigned long)Millisecond);
}

void Thread::Yield()
{
    ((ThreadEntry*)ThreadPointer)->Yield();
}

} //Namespace

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// WINDOWS
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#else //ZENLIB_USEWX
#ifdef WINDOWS
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#undef __TEXT
#include <windows.h>
#undef Yield
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//Config

#ifndef _MT
    #define _MT //Must have this symbol defined to get _beginthread/_endthread declarations
#endif

#ifdef __BORLANDC__
    #if !defined(__MT__)
        #define __MT__ //-tWM in the IDE is not always set
    #endif

    #if !defined(__MFC_COMPAT__)
        #define __MFC_COMPAT__ // Needed to know about _beginthreadex etc..
    #endif
#endif //__BORLANDC__


#if  defined(_MSC_VER) || \
    (defined(__GNUG__) && defined(__MSVCRT__)) || \
     defined(__WATCOMC__) || \
     defined(__MWERKS__)
    //(defined(__BORLANDC__) && (__BORLANDC__ >= 0x500))

    #ifndef __WXWINCE__
        #define USING_BEGINTHREAD //Using _beginthreadex() instead of CreateThread() if possible (better, because of Win32 API has problems with memory leaks in C library)
    #endif
#endif

#ifdef USING_BEGINTHREAD
    #include <process.h>
    typedef unsigned THREAD_RETVAL;     //The return type        of the thread function entry point
    #define THREAD_CALLCONV __stdcall   //The calling convention of the thread function entry point
#else
    // the settings for CreateThread()
    typedef DWORD THREAD_RETVAL;
    #define THREAD_CALLCONV WINAPI
#endif
//---------------------------------------------------------------------------

namespace ZenLib
{

//---------------------------------------------------------------------------
THREAD_RETVAL THREAD_CALLCONV Thread_Start(void *param)
{
    ((Thread*)param)->Entry();

    ((Thread*)param)->Internal_Exit();

    return 1;
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
Thread::Thread()
{
    C.Enter();

    State=State_New;
    ThreadPointer=NULL;

    C.Leave();
}

//---------------------------------------------------------------------------
Thread::~Thread()
{
    C.Enter();

    if (ThreadPointer!=NULL)
        CloseHandle((HANDLE)ThreadPointer); //ThreadPointer=NULL

    C.Leave();
}

//***************************************************************************
// Control
//***************************************************************************

//---------------------------------------------------------------------------
Thread::returnvalue Thread::Run()
{
    C.Enter();

    //Coherency
    if (State!=State_New || ThreadPointer!=NULL)
    {
        C.Leave();
        return Incoherent;
    }

    //Creating
    #ifdef USING_BEGINTHREAD
        #ifdef __WATCOMC__
            const unsigned stksize=10240; //Watcom is reported to not like 0 stack size (which means "use default")
        #else
            const unsigned stksize=0; //Default
        #endif //__WATCOMC__

        ThreadPointer=(void*)_beginthreadex (NULL, stksize, Thread_Start, this, CREATE_SUSPENDED, NULL);
    #else
        ThreadPointer=(void*)CreateThread   (NULL,       0, Thread_Start, this, CREATE_SUSPENDED, NULL);
    #endif //USING_BEGINTHREAD
    if (ThreadPointer==NULL)
    {
        C.Leave();
        return Resource;
    }

    //Running
    ResumeThread((HANDLE)ThreadPointer);

    //Configuring
    State=State_Running;

    C.Leave();
    return Ok;
}

//---------------------------------------------------------------------------
Thread::returnvalue Thread::RunAgain()
{
    //Coherency
    C.Enter();

    //Coherency
    if (State!=State_New
     && State!=State_Terminated)
    {
        C.Leave();
        return Incoherent;
    }

    //Configuring
    if (State==State_Terminated)
        State=State_New;

    C.Leave();

    return Run();
}

//---------------------------------------------------------------------------
Thread::returnvalue Thread::Pause()
{
    C.Enter();

    //Pausing
    SuspendThread((HANDLE)ThreadPointer);

    //Configuring
    State=State_Paused;

    C.Leave();
    return Ok;
}

//---------------------------------------------------------------------------
Thread::returnvalue Thread::RequestTerminate()
{
    C.Enter();

    //Coherency
    if (State!=State_Running)
    {
        C.Leave();
        return IsNotRunning;
    }

    //Configuring
    State=State_Terminating;

    C.Leave();
    return Ok;
}

//---------------------------------------------------------------------------
Thread::returnvalue Thread::ForceTerminate()
{
    C.Enter();

    //Terminating (not clean)
    TerminateThread((HANDLE)ThreadPointer, 1); ThreadPointer=NULL;

    //Configuring
    State=State_Terminated;

    C.Leave();
    return Ok;
}

//***************************************************************************
// Status
//***************************************************************************

bool Thread::IsRunning()
{
    C.Enter();
    bool ToReturn=State==State_Running;
    C.Leave();
    return ToReturn;
}

//---------------------------------------------------------------------------
bool Thread::IsTerminating()
{
    C.Enter();
    bool ToReturn=State==State_Terminating;
    C.Leave();
    return ToReturn;
}

//---------------------------------------------------------------------------
bool Thread::IsExited()
{
    C.Enter();
    bool ToReturn=State==State_New || State==State_Terminating;
    C.Leave();
    return ToReturn;
}

//***************************************************************************
// Main Entry
//***************************************************************************

void Thread::Entry()
{
}

//***************************************************************************
// Communicating
//***************************************************************************

//---------------------------------------------------------------------------
void Thread::Sleep(size_t Millisecond)
{
    ::Sleep((DWORD)Millisecond);
}

//---------------------------------------------------------------------------
void Thread::Yield()
{
    ::Sleep(0);
}

//***************************************************************************
// Internal
//***************************************************************************

//---------------------------------------------------------------------------
Thread::returnvalue Thread::Internal_Exit()
{
    C.Enter();

    //Coherency
    if (State!=State_Running
     && State!=State_Terminating)
    {
        C.Leave();
        return IsNotRunning;
    }

    //Closing old handle
    CloseHandle((HANDLE)ThreadPointer); ThreadPointer=NULL;

    //Configuring
    State=State_Terminated;

    C.Leave();
    return Ok;
}

} //Namespace

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// UNIX
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#else //WINDOWS
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
//Source: http://www.yolinux.com/TUTORIALS/LinuxTutorialPosixThreads.html
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include <pthread.h>
#include <unistd.h>
#ifdef _POSIX_PRIORITY_SCHEDULING
    #include <sched.h>
#endif //_POSIX_PRIORITY_SCHEDULING
//---------------------------------------------------------------------------

namespace ZenLib
{

//---------------------------------------------------------------------------
void *Thread_Start(void *param)
{
    ((Thread*)param)->Entry();
    ((Thread*)param)->Internal_Exit();

    return NULL;
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
Thread::Thread()
{
    C.Enter();

    State=State_New;
    ThreadPointer=NULL;

    C.Leave();

}

//---------------------------------------------------------------------------
Thread::~Thread()
{
}

//***************************************************************************
// Main Entry
//***************************************************************************

void Thread::Entry()
{
}

//***************************************************************************
// Control
//***************************************************************************

Thread::returnvalue Thread::Run()
{
    C.Enter();

    //Coherency
    if (State!=State_New || ThreadPointer!=NULL)
    {
        C.Leave();
        return Incoherent;
    }

    //Creating
    pthread_attr_t Attr;
    pthread_attr_init(&Attr);
    pthread_attr_setdetachstate(&Attr, PTHREAD_CREATE_DETACHED);

    //Running
    pthread_create((pthread_t*)&ThreadPointer, &Attr, Thread_Start, (void*)this);

    //Configuring
    State=State_Running;

    C.Leave();
    return Ok;
}

Thread::returnvalue Thread::RunAgain()
{
    //Coherency
    C.Enter();

    //Coherency
    if (State!=State_New
     && State!=State_Terminated)
    {
        C.Leave();
        return Incoherent;
    }

    //Configuring
    if (State==State_Terminated)
        State=State_New;

    C.Leave();

    return Run();
}

Thread::returnvalue Thread::Pause()
{
    //pthread_cond_wait
    return Ok;
}

Thread::returnvalue Thread::RequestTerminate()
{
    C.Enter();

    if (State!=State_Running)
    {
        C.Leave();
        return IsNotRunning;
    }

    State=State_Terminating;

    C.Leave();
    return Ok;
}

Thread::returnvalue Thread::ForceTerminate()
{
    //Terminating (not clean)

    //Configuring
    State=State_Terminated;

    return Ok;
}

//***************************************************************************
// Status
//***************************************************************************

//---------------------------------------------------------------------------
bool Thread::IsRunning()
{
    C.Enter();
    bool ToReturn=State==State_Running;
    C.Leave();
    return ToReturn;
}

//---------------------------------------------------------------------------
bool Thread::IsTerminating()
{
    C.Enter();
    bool ToReturn=State==State_Terminating;
    C.Leave();
    return ToReturn;
}

//---------------------------------------------------------------------------
bool Thread::IsExited()
{
    C.Enter();
    bool ToReturn=State==State_New || State==State_Terminating;
    C.Leave();
    return ToReturn;
}

//***************************************************************************
// Communicating
//***************************************************************************

void Thread::Sleep(size_t)
{
}

void Thread::Yield()
{
    #ifdef _POSIX_PRIORITY_SCHEDULING
        sched_yield();
    #endif //_POSIX_PRIORITY_SCHEDULING
}

//***************************************************************************
// Internal
//***************************************************************************

Thread::returnvalue Thread::Internal_Exit()
{
    C.Enter();

    //Coherency
    if (State!=State_Running
     && State!=State_Terminating)
    {
        C.Leave();
        return IsNotRunning;
    }

    //Closing old handle
    ; ThreadPointer=NULL;

    //Configuring
    State=State_Terminated;

    C.Leave();
    return Ok;
}

} //Namespace

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#endif //WINDOWS
#endif //ZENLIB_USEWX
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
