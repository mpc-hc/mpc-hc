// ZenLib::ZtringListListF - ZtringListList with file load/save
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
#include "ZenLib/PreComp.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "ZenLib/Conf_Internal.h"
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "ZenLib/ZtringListListF.h"
#include "ZenLib/File.h"
//---------------------------------------------------------------------------

namespace ZenLib
{

//---------------------------------------------------------------------------
#define READ_SIZE 512*1024
//---------------------------------------------------------------------------

//***************************************************************************
// Constructors/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
// Constructors
void ZtringListListF::ZtringListListF_Common ()
{
    Backup_Nb_Max=0;
    Backup_Nb=0;
    Sauvegarde=true;
    #ifdef _UNICODE
        Local=false;
    #else
        Local=true;
    #endif
}

ZtringListListF::ZtringListListF ()
:ZtringListList ()
{
    ZtringListListF_Common();
}

ZtringListListF::ZtringListListF (const ZtringListList &Source)
:ZtringListList (Source)
{
    ZtringListListF_Common();
}

ZtringListListF::ZtringListListF (const Ztring &Source)
:ZtringListList (Source)
{
    ZtringListListF_Common();
}

ZtringListListF::ZtringListListF (const Char *Source)
:ZtringListList (Source)
{
    ZtringListListF_Common();
}

#ifdef _UNICODE
ZtringListListF::ZtringListListF (const char* Source)
:ZtringListList (Source)
{
    ZtringListListF_Common();
}
#endif

//***************************************************************************
// File management
//***************************************************************************

//---------------------------------------------------------------------------
// Load
bool ZtringListListF::Load (const Ztring &NewFileName)
{
    clear();
    if (!NewFileName.empty())
        Name=NewFileName;

    size_t I1=Error;

    if (Name.find(_T(".csv"))!=Error)
        I1=CSV_Charger();
    if (Name.find(_T(".cfg"))!=Error)
        I1=CFG_Charger();
    if (I1!=Error)
    {
        Backup_Nb=0; //mettre ici le code pour trouver le nb de backup
        return true;
    }
    else
        return false;
}

//---------------------------------------------------------------------------
// Load CSV
bool ZtringListListF::CSV_Charger ()
{
    //Read file
    File F;
    if (!F.Open(Name))
        return false;

    int8u* Buffer=new int8u[(size_t)F.Size_Get()+1];
    size_t BytesCount=F.Read(Buffer, (size_t)F.Size_Get());
    F.Close();
    if (BytesCount==Error)
    {
        delete[] Buffer; //Buffer=NULL;
        return false;
    }
    Buffer[(int32u)BytesCount]=(int8u)'\0';

    //Convert file in UTF-8 or Local
    Ztring File;
    if (!Local)
    {
        //UTF-8
        File.From_UTF8((char*)Buffer, 0, BytesCount);
        #ifdef _DEBUG
        if (File.size()==0)
             File.From_Local((char*)Buffer, 0, BytesCount);
        #endif //_DEBUG
    }
    if (File.size()==0)
        //Local of UTF-8 failed
        File.From_Local((char*)Buffer, 0, BytesCount);

    //Separators
    if (Separator[0]==_T("(Default)"))
            Separator[0]=EOL;
    Ztring SeparatorT=Separator[1];
    Separator[1]=_T(";");

    //Writing
    Write(File);

    //Separators
    Separator[1]=SeparatorT;

    delete[] Buffer; //Buffer=NULL;
    return true;
}

//---------------------------------------------------------------------------
// Chargement CFG
bool ZtringListListF::CFG_Charger ()
{
    //Read file
    File F(Name);
    int8u* Buffer=new int8u[(size_t)F.Size_Get()+1];
    size_t BytesCount=F.Read(Buffer, (size_t)F.Size_Get());
    F.Close();
    if (BytesCount==Error)
    {
        delete[] Buffer; //Buffer=NULL;
        return false;
    }
    Buffer[(int32u)BytesCount]=(int8u)'\0';

    //Convert File --> ZtringList
    ZtringList List;
    List.Separator_Set(0, EOL);
    Ztring Z1;
    Z1.From_UTF8((char*)Buffer, 0, BytesCount);
    List.Write(Z1);

    Ztring SeparatorT=Separator[1];
    Separator[1]=_T(";");

    Ztring Propriete, Valeur, Commentaire;

    for (size_t Pos=0; Pos<List.size(); Pos++)
    {
        Ztring &Lu=List(Pos);
        if (Lu.find(_T("="))>0)
        {
            //Obtention du Name
            Propriete=Lu.SubString(Ztring(), _T("="));
            NettoyerEspaces(Propriete);
            //Obtention de la valeur
            Valeur=Lu.SubString(_T("="), _T(";"), 0, Ztring_AddLastItem);
            NettoyerEspaces(Valeur);
        }
        //Obtention du commentaire
        Commentaire=Lu.SubString(_T(";"), Ztring(), 0, Ztring_AddLastItem);
        NettoyerEspaces(Commentaire);
        //Ecriture
        push_back((Propriete+_T(";")+Valeur+_T(";")+Commentaire).c_str()); //Visual C++ 6 is old...
    }
    Separator[1]=SeparatorT;

    delete[] Buffer; //Buffer=NULL;
    return true;
}

//---------------------------------------------------------------------------
// Sauvegarde globale
bool ZtringListListF::Save (const Ztring &FileName)
{
    //Gestion de l'annulation de la sauvegarde
    if (!Sauvegarde)
        return true;

    if (FileName!=Ztring())
        Name=FileName;

    //Gestion des backups
    Backup_Nb=0;
    int8u I2;
    Separator[0]=EOL;
    if (Backup_Nb_Max>0)
    {
        //TODO : not tested
        for (int8u I1=Backup_Nb_Max-1; I1>0; I1--)
        {
            Ztring Z1=Name+_T(".sav"); Z1+=Ztring::ToZtring(I1);
            Ztring Z2=Name+_T(".sav"); Z2+=Ztring::ToZtring(I1+1);
            File::Delete(Z2.c_str());
            I2=File::Move(Z1.c_str(), Z2.c_str());
            if (I2 && !Backup_Nb)
                Backup_Nb=I2;
        }
        Ztring Z1=Name+_T(".sav0");
        File::Delete(Z1.c_str());
        File::Move(Name.c_str(), Z1.c_str());
        Backup_Nb++;
    }

    I2=0;
    if (Name.find(_T(".csv"))!=Error)
        I2=CSV_Sauvegarder();
    if (Name.find(_T(".cfg"))!=Error)
        I2=CFG_Sauvegarder();

    if (I2>0)
    {
        return true;
    }
    else
        return false;
}

//---------------------------------------------------------------------------
// Sauvegarde CSV
bool ZtringListListF::CSV_Sauvegarder ()
{
    //Sauvegarde
    File F;
    if (!F.Create(Name, true))
        return Error;

    if (Separator[0]==_T("(Default)"))
        Separator[0]=EOL;

    F.Write(Read());

    return true;
}

//---------------------------------------------------------------------------
// Sauvegarde CFG
bool ZtringListListF::CFG_Sauvegarder ()
{
    File F;
    if (!F.Create(Name, true))
        return Error;

    Ztring ToWrite;
    Ztring Propriete, Valeur, Commentaire;

    ;
    for (size_t Pos=0; Pos<size(); Pos++)
    {
        Propriete=Read(Pos, 0);
        Valeur=Read(Pos, 1);
        Commentaire=Read(Pos, 2);
        if (Propriete!=Ztring())
        {
            ToWrite+=Propriete+_T(" = ");
            if (Valeur!=Ztring())
                ToWrite+=Valeur+_T(" ");
        }
        if (Commentaire!=Ztring())
            ToWrite+=_T("; ")+Commentaire;
        ToWrite+=EOL;
    }
    F.Write(ToWrite);

    return true;
}

//---------------------------------------------------------------------------
// Annulation
bool ZtringListListF::Cancel ()
{
    Ztring Z1=Name+_T(".sav0"); //Visual C++ 6 patch
    File::Delete(Name.c_str());
    File::Move(Z1.c_str(), Name.c_str());
    for (int8u I1=1; I1<=Backup_Nb; I1++)
    {
        Ztring Z2=Name+_T(".sav"); Z2+=Ztring::ToZtring(I1); //Visual C++ 6 patch
        Ztring Z3=Name+_T(".sav"); Z3+=Ztring::ToZtring(I1-1); //Visual C++ 6 patch
        File::Delete(Z3.c_str());
        File::Move(Z2.c_str(), Z3.c_str());
    }
    Write(Ztring());
    return CSV_Charger();
}

//***************************************************************************
// Divers
//***************************************************************************

//---------------------------------------------------------------------------
// Nettoyage
bool ZtringListListF::NettoyerEspaces (Ztring &ANettoyer)
{
    size_t Debut=0;
    while (Debut<ANettoyer.size() && ANettoyer[Debut]==_T(' '))
        Debut++;
    size_t Fin=ANettoyer.size()-1;
    while (Fin!=(size_t)-2 && ANettoyer[Fin]==_T(' '))
        Fin--;
    if (Fin>=Debut)
        ANettoyer=ANettoyer.substr(Debut, Fin-Debut+1);
    else
        ANettoyer=Ztring();
    return true;
}

//---------------------------------------------------------------------------
// Backup
void ZtringListListF::Backup_Set (bool NewSave)
{
    Sauvegarde=NewSave;
    Save();
}

void ZtringListListF::Backup_Count_Set (int8u NewCount)
{
    Backup_Nb_Max=NewCount;
}

//---------------------------------------------------------------------------
// Local
void ZtringListListF::Local_Set (bool NewLocal)
{
    Local=NewLocal;
}

} //Namespace

