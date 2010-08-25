// ZenLib::Thread - Thread functions
// Copyright (C) 2007-2010 MediaArea.net SARL, Info@MediaArea.net
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Thread functions
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef ZenLib_ThreadH
#define ZenLib_ThreadH
//---------------------------------------------------------------------------
#include "ZenLib/Conf.h"
#include "ZenLib/CriticalSection.h"
#ifdef _WINDOWS
    #undef Yield
#endif
//---------------------------------------------------------------------------

namespace ZenLib
{

//***************************************************************************
/// @brief Thread manipulation
//***************************************************************************

class Thread
{
public :
    //Constructor/Destructor
    Thread  ();
    virtual ~Thread ();

    //Control
    enum returnvalue
    {
        Ok,
        IsNotRunning,
        Incoherent,
        Ressource,
    };
    returnvalue Run();
    returnvalue RunAgain();
    returnvalue Pause();
    returnvalue RequestTerminate();
    returnvalue ForceTerminate();

    //Status
    bool        IsRunning();
    bool        IsTerminating();
    bool        IsExited();

    //Configuration
    void        Priority_Set(int8s Priority); //-100 to +100

    //Main Entry
    virtual void Entry();

    //Internal
    returnvalue Internal_Exit(); //Do not use it

protected :

    //Communicating
    void    Sleep(size_t Millisecond);
    void    Yield();

private :
    //Internal
    void*   ThreadPointer;

    //The possible states of the thread ("-->" shows all possible transitions from this state)
    enum state
    {
        State_New,              // didn't start execution yet (--> Running)
        State_Running,          // thread is running (--> Paused, Terminating)
        State_Paused,           // thread is temporarily suspended (--> Running)
        State_Terminating,      // thread should terminate a.s.a.p. (--> Terminated)
        State_Terminated,       // thread is terminated
    };
    state State;
    CriticalSection C;
};

} //NameSpace

#endif
