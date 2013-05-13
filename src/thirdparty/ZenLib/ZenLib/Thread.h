/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a zlib-style license that can
 *  be found in the License.txt file in the root of the source tree.
 */

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
        Resource,
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
    void    Sleep(std::size_t Millisecond);
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
