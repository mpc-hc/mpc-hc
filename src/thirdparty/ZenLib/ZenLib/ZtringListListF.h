// ZenLib::ZtringListListF - ZtringListList with files
// Copyright (C) 2002-2011 MediaArea.net SARL, Info@MediaArea.net
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
//
// ZtringListList with file load/save
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef ZtringListListFH
#define ZtringListListFH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "ZenLib/ZtringListList.h"
//---------------------------------------------------------------------------

namespace ZenLib
{

//***************************************************************************
/// @brief ZtringListList with file management
//***************************************************************************

class ZtringListListF : public ZtringListList
{
public :
    //Constructors/Destructor
    ZtringListListF ();
    ZtringListListF (const ZtringListList &Source);
    ZtringListListF (const Ztring &Source);
    ZtringListListF (const Char *Source);
    #ifdef _UNICODE
    ZtringListListF (const char *Source); //convert a UTF-8 string into Unicode
    #endif

    //File management
    bool   Load   (const Ztring &FileName=Ztring());
    bool   Save   (const Ztring &FileName=Ztring());
    bool   Cancel ();

    //Configuration
    // @brief enable or disable backup creation
    void Backup_Set       (bool Save);
    // @brief Set the count of versions to save
    void Backup_Count_Set (int8u Count);
    // @brief Set if the content of file is a localized (=not UTF8) file
    void Local_Set        (bool Local);

protected :
    Ztring Name; //Nom du fichier
    bool   Sauvegarde; //Indicateur si on a le droit de sauvegarder (par defaut)
    int8u  Backup_Nb_Max; //Nombre maxi de sauvegardes
    int8u  Backup_Nb; //Nombre actuel de backups pour cette session
    bool   Local; //if true, this is a local charset, else this is a UTF8 charset

    //File management
    bool   CSV_Charger ();
    bool   CFG_Charger ();
    bool   CSV_Sauvegarder ();
    bool   CFG_Sauvegarder ();
    bool   File_Load ();

    //Divers
    bool   NettoyerEspaces (Ztring &ANettoyer); //Enlever les espaces avant et apres

private:
    void ZtringListListF_Common();
};

} //Namespace

#endif

