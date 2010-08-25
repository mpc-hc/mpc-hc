// ZenLib::MemoryDebug - To debug memory leaks
// Copyright (C) 2002-2010 MediaArea.net SARL, Info@MediaArea.net
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

//---------------------------------------------------------------------------
#if defined(ZENLIB_DEBUG)
//---------------------------------------------------------------------------
#include <iomanip>
#include <sstream>
#include "ZenLib/MemoryDebug.h"
#include "ZenLib/Ztring.h"
#ifdef WINDOWS
    #include <io.h>
#else
    #include <cstdio>
#endif
#include <fcntl.h>
#include <sys/stat.h>
using namespace std;
//---------------------------------------------------------------------------

namespace ZenLib
{

//***************************************************************************
// Constructors/destructor
//***************************************************************************

MemoryDebug::MemoryDebug()
{
}

MemoryDebug::~MemoryDebug()
{
    if (!m_Blocks.empty())
        ReportLeaks();
}

//***************************************************************************
// Instance
//***************************************************************************

MemoryDebug& MemoryDebug::Instance()
{
    static MemoryDebug Inst;
    return Inst;
}

//***************************************************************************
// Reports
//***************************************************************************

void MemoryDebug::ReportLeaks()
{
    Ztring m_File;
    //std::ofstream      m_File ("Debug_MemoryLeak.txt");        // Fichier de sortie

    // Détail des fuites
    std::size_t TotalSize = 0;
    for (TBlockMap::iterator i = m_Blocks.begin(); i != m_Blocks.end(); ++i)
    {
        // Ajout de la taille du bloc au cumul
        TotalSize += i->second.Size;

        // Inscription dans le fichier des informations sur le bloc courant
        /*
        m_File << "-> 0x" << std::hex << i->first << std::dec
               << " | "   << std::setw(7) << std::setfill(' ') << static_cast<int>(i->second.Size) << " bytes"
               << " | "   << i->second.File.c_str() << " (" << i->second.Line << ")" << std::endl;
        */
        m_File.append(_T("-> 0x"));
        m_File.append(Ztring::ToZtring((size_t)i->first, 16));
        m_File.append(_T(" | "));
        Ztring Temp;
        Temp.From_Number(static_cast<int>(i->second.Size));
        while(Temp.size()<7)
            Temp=_T(" ")+Temp;
        m_File.append(Temp);
        m_File.append(_T(" bytes"));
        m_File.append(_T(" | "));
        m_File.append(Ztring().From_Local(i->second.File.c_str()));
        m_File.append(_T(" ("));
        m_File.append(Ztring::ToZtring(i->second.Line));
        m_File.append(_T(")"));
        m_File.append(EOL);
    }

    // Affichage du cumul des fuites
    /*
    m_File << std::endl << std::endl << "-- "
           << static_cast<int>(m_Blocks.size()) << " non-released blocs, "
           << static_cast<int>(TotalSize)       << " bytes --"
           << std::endl;
    */
    m_File.append(EOL);
    m_File.append(EOL);
    m_File.append(_T("-- "));
    m_File.append(Ztring::ToZtring(static_cast<int>(m_Blocks.size())));
    m_File.append(_T(" non-released blocs, "));
    m_File.append(Ztring::ToZtring(static_cast<int>(TotalSize)));
    m_File.append(_T(" bytes --"));
    m_File.append(EOL);

    std::string ToWrite=m_File.To_Local().c_str();
    int m_File_sav=open("Debug_MemoryLeak.txt", O_BINARY|O_RDWR  |O_CREAT);        // Fichier de sortie
    write(m_File_sav, (int8u*)ToWrite.c_str(), ToWrite.size());
    close(m_File_sav);
}

//***************************************************************************
// Memory management
//***************************************************************************

void* MemoryDebug::Allocate(std::size_t Size, const char* File, int Line, bool Array)
{
    // Allocation de la mémoire
    void* Ptr = malloc(Size);

    // Ajout du bloc à la liste des blocs alloués
    TBlock NewBlock;
    NewBlock.Size  = Size;
    NewBlock.File  = File;
    NewBlock.Line  = Line;
    NewBlock.Array = Array;
    m_Blocks[Ptr]  = NewBlock;
    return Ptr;
}

void MemoryDebug::Free(void* Ptr, bool Array)
{
    // Recherche de l'adresse dans les blocs alloués
    TBlockMap::iterator It = m_Blocks.find(Ptr);

    // Si le bloc n'a pas été alloué, on génère une erreur
    if (It == m_Blocks.end())
    {
        // En fait ça arrive souvent, du fait que le delete surcharge est pris en compte meme la ou on n'inclue pas DebugNew.h,
        // mais pas la macro pour le new
        // Dans ce cas on détruit le bloc et on quitte immédiatement
        free(Ptr);
        return;
    }

    // Si le type d'allocation ne correspond pas, on génère une erreur
    if (It->second.Array != Array)
    {
        //throw CBadDelete(Ptr, It->second.File.c_str(), It->second.Line, !Array);
    }

    // Finalement, si tout va bien, on supprime le bloc et on loggiz tout ça
    m_Blocks.erase(It);
    m_DeleteStack.pop();

    // Libération de la mémoire
    free(Ptr);
}

void MemoryDebug::NextDelete(const char* File, int Line)
{
    TBlock Delete;
    Delete.File = File;
    Delete.Line = Line;

    m_DeleteStack.push(Delete);
}

//***************************************************************************
//
//***************************************************************************

} //NameSpace

#endif // defined(ZENLIB_DEBUG)
