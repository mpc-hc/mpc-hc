/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a zlib-style license that can
 *  be found in the License.txt file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// MemoryDebug
//
// Provide "new" and "delete" overloadings to be able to detect memory leaks
// Based on http://loulou.developpez.com/tutoriels/moteur3d/partie1/ 2.2.1
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef ZenMemoryDebugH
#define ZenMemoryDebugH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if defined(ZENLIB_DEBUG)
//---------------------------------------------------------------------------
#include "ZenLib/Conf.h"
#include <fstream>
#include <map>
#include <stack>
#include <string>
//---------------------------------------------------------------------------

namespace ZenLib
{

//***************************************************************************
// Class
//***************************************************************************

class MemoryDebug
{
public :
    ~MemoryDebug();
    static MemoryDebug& Instance();

    void* Allocate(std::size_t Size, const char* File, int Line, bool Array);
    void  Free(void* Ptr, bool Array);
    void  NextDelete(const char*, int Line); //Sauvegarde les infos sur la désallocation courante

    void ReportLeaks();

private :
    MemoryDebug();
    struct TBlock
    {
        std::size_t Size;  // Taille allouée
        std::string File;  // Fichier contenant l'allocation
        int         Line;  // Ligne de l'allocation
        bool        Array; // Est-ce un objet ou un tableau ?
    };
    typedef std::map<void*, TBlock> TBlockMap;

    TBlockMap          m_Blocks;      // Blocs de mémoire alloués
    std::stack<TBlock> m_DeleteStack; // Pile dont le sommet contient la ligne et le fichier de la prochaine désallocation
};

} //NameSpace

//***************************************************************************
// operator overloadings
//***************************************************************************

inline void* operator new(std::size_t Size, const char* File, int Line)
{
    return ZenLib::MemoryDebug::Instance().Allocate(Size, File, Line, false);
}
inline void* operator new[](std::size_t Size, const char* File, int Line)
{
    return ZenLib::MemoryDebug::Instance().Allocate(Size, File, Line, true);
}

inline void operator delete(void* Ptr)
{
    ZenLib::MemoryDebug::Instance().Free(Ptr, false);
}

inline void operator delete[](void* Ptr)
{
    ZenLib::MemoryDebug::Instance().Free(Ptr, true);
}

#if !defined(__BORLANDC__) // Borland does not support overloaded delete
inline void operator delete(void* Ptr, const char* File, int Line)
{
    ZenLib::MemoryDebug::Instance().NextDelete(File, Line);
    ZenLib::MemoryDebug::Instance().Free(Ptr, false);
}

inline void operator delete[](void* Ptr, const char* File, int Line)
{
    ZenLib::MemoryDebug::Instance().NextDelete(File, Line);
    ZenLib::MemoryDebug::Instance().Free(Ptr, true);
}
#endif

#if !defined(__MINGW32__) //TODO: Does not work on MinGW, don't know why
#ifndef new
    #define new         new(__FILE__, __LINE__)
#endif
#ifndef delete
    #define delete      ZenLib::MemoryDebug::Instance().NextDelete(__FILE__, __LINE__), delete
#endif
#endif // __MINGW32__

#endif // defined(ZENLIB_DEBUG)

#endif // ZenMemoryDebugH
