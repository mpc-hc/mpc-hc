// MediaInfo_Inform - Base for other files
// Copyright (C) 2002-2011 MediaArea.net SARL, Info@MediaArea.net
//
// This library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library. If not, see <http://www.gnu.org/licenses/>.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Inform part
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
// Pre-compilation
#include "MediaInfo/PreComp.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Setup.h"
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "ZenLib/Utils.h"
#include "MediaInfo/MediaInfo_Internal.h"
#include "MediaInfo/MediaInfo_Config.h"
#include "MediaInfo/Export/Export_Mpeg7.h"
#include "MediaInfo/Export/Export_reVTMD.h"
#include "MediaInfo/Export/Export_PBCore.h"
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//---------------------------------------------------------------------------
extern MediaInfo_Config Config;
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
Ztring MediaInfo_Internal::Inform()
{
    CS.Enter();
    if (Info && Info->Status[File__Analyze::IsUpdated])
        Info->Open_Buffer_Update();
    CS.Leave();

    #if MEDIAINFO_TRACE
        if (MediaInfoLib::Config.Trace_Level_Get() || MediaInfoLib::Config.Inform_Get()==_T("Details"))
        {
            if (!Details.empty())
            {
                Ztring Content=Details;

                return Details;
            }
            else if (Info)
                return Info->Details_Get();
        }
    #endif //MEDIAINFO_TRACE

    if (MediaInfoLib::Config.Inform_Get()==_T("MPEG-7"))
        return Export_Mpeg7().Transform(*this);
    if (MediaInfoLib::Config.Inform_Get()==_T("PBCore") || MediaInfoLib::Config.Inform_Get()==_T("PBCore_1.2"))
        return Export_PBCore().Transform(*this);
    if (MediaInfoLib::Config.Inform_Get()==_T("reVTMD"))
        return _T("reVTMD is disabled due to its non-free licensing."); //return Export_reVTMD().Transform(*this);

    if (!(
        MediaInfoLib::Config.Inform_Get(_T("General")).empty()
     && MediaInfoLib::Config.Inform_Get(_T("Video")).empty()
     && MediaInfoLib::Config.Inform_Get(_T("Audio")).empty()
     && MediaInfoLib::Config.Inform_Get(_T("Text")).empty()
     && MediaInfoLib::Config.Inform_Get(_T("Chapters")).empty()
     && MediaInfoLib::Config.Inform_Get(_T("Image")).empty()
     && MediaInfoLib::Config.Inform_Get(_T("Menu")).empty()
     ))
    {
        Ztring Retour;
        Retour+=MediaInfoLib::Config.Inform_Get(_T("File_Begin"));
        Retour+=MediaInfoLib::Config.Inform_Get(_T("General_Begin"));
        Retour+=Inform(Stream_General, 0, false);
        Retour+=MediaInfoLib::Config.Inform_Get(_T("General_End"));
        if (Count_Get(Stream_Video))
            Retour+=MediaInfoLib::Config.Inform_Get(_T("Video_Begin"));
        for (size_t I1=0; I1<Count_Get(Stream_Video); I1++)
        {
            Retour+=Inform(Stream_Video, I1, false);
            if (I1!=Count_Get(Stream_Video)-1)
                Retour+=MediaInfoLib::Config.Inform_Get(_T("Video_Middle"));
        }
        if (Count_Get(Stream_Video))
            Retour+=MediaInfoLib::Config.Inform_Get(_T("Video_End"));
        if (Count_Get(Stream_Audio))
            Retour+=MediaInfoLib::Config.Inform_Get(_T("Audio_Begin"));
        for (size_t I1=0; I1<Count_Get(Stream_Audio); I1++)
        {
            Retour+=Inform(Stream_Audio, I1, false);
            if (I1!=Count_Get(Stream_Audio)-1)
                Retour+=MediaInfoLib::Config.Inform_Get(_T("Audio_Middle"));
        }
        if (Count_Get(Stream_Audio))
            Retour+=MediaInfoLib::Config.Inform_Get(_T("Audio_End"));
        if (Count_Get(Stream_Text))
            Retour+=MediaInfoLib::Config.Inform_Get(_T("Text_Begin"));
        for (size_t I1=0; I1<Count_Get(Stream_Text); I1++)
        {
            Retour+=Inform(Stream_Text, I1, false);
            if (I1!=Count_Get(Stream_Text)-1)
                Retour+=MediaInfoLib::Config.Inform_Get(_T("Text_Middle"));
        }
        if (Count_Get(Stream_Text))
            Retour+=MediaInfoLib::Config.Inform_Get(_T("Text_End"));
        if (Count_Get(Stream_Chapters))
            Retour+=MediaInfoLib::Config.Inform_Get(_T("Chapters_Begin"));
        for (size_t I1=0; I1<Count_Get(Stream_Chapters); I1++)
        {
            Retour+=Inform(Stream_Chapters, I1, false);
            if (I1!=Count_Get(Stream_Chapters)-1)
                Retour+=MediaInfoLib::Config.Inform_Get(_T("Chapters_Middle"));
        }
        if (Count_Get(Stream_Chapters))
            Retour+=MediaInfoLib::Config.Inform_Get(_T("Chapters_End"));
        if (Count_Get(Stream_Image))
            Retour+=MediaInfoLib::Config.Inform_Get(_T("Image_Begin"));
        for (size_t I1=0; I1<Count_Get(Stream_Image); I1++)
        {
            Retour+=Inform(Stream_Image, I1, false);
            if (I1!=Count_Get(Stream_Image)-1)
                Retour+=MediaInfoLib::Config.Inform_Get(_T("Image_Middle"));
        }
        if (Count_Get(Stream_Image))
            Retour+=MediaInfoLib::Config.Inform_Get(_T("Image_End"));
        if (Count_Get(Stream_Menu))
            Retour+=MediaInfoLib::Config.Inform_Get(_T("Menu_Begin"));
        for (size_t I1=0; I1<Count_Get(Stream_Menu); I1++)
        {
            Retour+=Inform(Stream_Menu, I1, false);
            if (I1!=Count_Get(Stream_Menu)-1)
                Retour+=MediaInfoLib::Config.Inform_Get(_T("Menu_Middle"));
        }
        if (Count_Get(Stream_Menu))
            Retour+=MediaInfoLib::Config.Inform_Get(_T("Menu_End"));
        Retour+=MediaInfoLib::Config.Inform_Get(_T("File_End"));

        Retour.FindAndReplace(_T("\\r\\n"), _T("\n"), 0, Ztring_Recursive);
        Retour.FindAndReplace(_T("\\r"), _T("\n"), 0, Ztring_Recursive);
        Retour.FindAndReplace(_T("\\n"), _T("\n"), 0, Ztring_Recursive);
        Retour.FindAndReplace(_T("\r\n"), _T("\n"), 0, Ztring_Recursive);
        Retour.FindAndReplace(_T("\r"), _T("\n"), 0, Ztring_Recursive);
        Retour.FindAndReplace(_T("\n"), MediaInfoLib::Config.LineSeparator_Get(), 0, Ztring_Recursive);

        //Special characters
        Retour.FindAndReplace(_T("|SC1|"), _T("\\"), 0, Ztring_Recursive);
        Retour.FindAndReplace(_T("|SC2|"), _T("["), 0, Ztring_Recursive);
        Retour.FindAndReplace(_T("|SC3|"), _T("]"), 0, Ztring_Recursive);
        Retour.FindAndReplace(_T("|SC4|"), _T(","), 0, Ztring_Recursive);
        Retour.FindAndReplace(_T("|SC5|"), _T(";"), 0, Ztring_Recursive);
        Retour.FindAndReplace(_T("|SC6|"), _T("("), 0, Ztring_Recursive);
        Retour.FindAndReplace(_T("|SC7|"), _T(")"), 0, Ztring_Recursive);
        Retour.FindAndReplace(_T("|SC8|"), _T(")"), 0, Ztring_Recursive);
        Retour.FindAndReplace(_T("|SC9|"), _T("),"), 0, Ztring_Recursive);

        return Retour;
    }

    //Informations
    Ztring Retour;
    bool HTML=false;
    bool XML=false;
    bool CSV=false;
    if (MediaInfoLib::Config.Inform_Get()==_T("HTML"))
        HTML=true;
    if (MediaInfoLib::Config.Inform_Get()==_T("XML"))
        XML=true;
    if (MediaInfoLib::Config.Inform_Get()==_T("CSV"))
        CSV=true;

    if (HTML) Retour+=_T("<html>\n\n<head>\n<META http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" /></head>\n<body>\n");
    if (XML)  Retour+=_T("<File>\n");

    for (size_t StreamKind=(size_t)Stream_General; StreamKind<Stream_Max; StreamKind++)
    {
        //Pour chaque type de flux
        for (size_t StreamPos=0; StreamPos<(size_t)Count_Get((stream_t)StreamKind); StreamPos++)
        {
            //Pour chaque stream
            if (HTML) Retour+=_T("<table width=\"100%\" border=\"0\" cellpadding=\"1\" cellspacing=\"2\" style=\"border:1px solid Navy\">\n<tr>\n    <td width=\"150\"><h2>");
            if (XML) Retour+=_T("<track type=\"");
            Ztring A=Get((stream_t)StreamKind, StreamPos, _T("StreamKind/String"));
            Ztring B=Get((stream_t)StreamKind, StreamPos, _T("StreamKindPos"));
            if (!XML && !B.empty())
            {
                if (CSV)
                    A+=_T(",");
                else
                    A+=MediaInfoLib::Config.Language_Get(_T("  Config_Text_NumberTag"));
                A+=B;
            }
            Retour+=A;
            if (XML)
            {
                Retour+=_T("\"");
                if (!B.empty())
                {
                    Retour+=_T(" streamid=\"");
                    Retour+=B;
                    Retour+=_T("\"");
                }
            }
            if (HTML) Retour+=_T("</h2></td>\n  </tr>");
            if (XML) Retour+=_T(">");
            Retour+=MediaInfoLib::Config.LineSeparator_Get();
            Retour+=Inform((stream_t)StreamKind, StreamPos, false);
            Retour.FindAndReplace(_T("\\"), _T("|SC1|"), 0, Ztring_Recursive);
            if (HTML) Retour+=_T("</table>\n<br />");
            if (XML) Retour+=_T("</track>\n");
            Retour+=MediaInfoLib::Config.LineSeparator_Get();
        }
    }

    if (HTML) Retour+=_T("\n</body>\n</html>\n");
    if (XML)  Retour+=_T("</File>\n");

    Retour.FindAndReplace(_T("\\r\\n"), _T("\n"), 0, Ztring_Recursive);
    Retour.FindAndReplace(_T("\\r"), _T("\n"), 0, Ztring_Recursive);
    Retour.FindAndReplace(_T("\\n"), _T("\n"), 0, Ztring_Recursive);
    Retour.FindAndReplace(_T("\r\n"), _T("\n"), 0, Ztring_Recursive);
    Retour.FindAndReplace(_T("\r"), _T("\n"), 0, Ztring_Recursive);
    Retour.FindAndReplace(_T("\n"), MediaInfoLib::Config.LineSeparator_Get(), 0, Ztring_Recursive);

    //Special characters
    Retour.FindAndReplace(_T("|SC1|"), _T("\\"), 0, Ztring_Recursive);
    Retour.FindAndReplace(_T("|SC2|"), _T("["), 0, Ztring_Recursive);
    Retour.FindAndReplace(_T("|SC3|"), _T("]"), 0, Ztring_Recursive);
    Retour.FindAndReplace(_T("|SC4|"), _T(","), 0, Ztring_Recursive);
    Retour.FindAndReplace(_T("|SC5|"), _T(";"), 0, Ztring_Recursive);
    Retour.FindAndReplace(_T("|SC6|"), _T("("), 0, Ztring_Recursive);
    Retour.FindAndReplace(_T("|SC7|"), _T(")"), 0, Ztring_Recursive);
    Retour.FindAndReplace(_T("|SC8|"), _T(")"), 0, Ztring_Recursive);
    Retour.FindAndReplace(_T("|SC9|"), _T("),"), 0, Ztring_Recursive);

    return Retour;
}

//---------------------------------------------------------------------------
Ztring MediaInfo_Internal::Inform (stream_t StreamKind, size_t StreamPos, bool IsDirect)
{
    //Integrity
    if (StreamKind>=Stream_Max || StreamPos>=Stream[StreamKind].size())
        return Ztring();

    if (MediaInfoLib::Config.Inform_Get(_T("General")).empty()
     && MediaInfoLib::Config.Inform_Get(_T("Video")).empty()
     && MediaInfoLib::Config.Inform_Get(_T("Audio")).empty()
     && MediaInfoLib::Config.Inform_Get(_T("Text")).empty()
     && MediaInfoLib::Config.Inform_Get(_T("Chapters")).empty()
     && MediaInfoLib::Config.Inform_Get(_T("Image")).empty()
     && MediaInfoLib::Config.Inform_Get(_T("Menu")).empty())
    {
        Ztring Retour;
        bool HTML=false;
        bool XML=false;
        bool CSV=false;
        if (MediaInfoLib::Config.Inform_Get()==_T("HTML"))
            HTML=true;
        if (MediaInfoLib::Config.Inform_Get()==_T("XML"))
            XML=true;
        if (MediaInfoLib::Config.Inform_Get()==_T("CSV"))
            CSV=true;
        size_t Size=Count_Get(StreamKind, StreamPos);
        for (size_t Champ_Pos=0; Champ_Pos<Size; Champ_Pos++)
        {
            //Pour chaque champ
            //Ztring A=Get((stream_t)4, 2, 0, Info_Measure_Text); // TODO Bug sinon? voir Ztring
            Ztring A=Get((stream_t)StreamKind, StreamPos, Champ_Pos, Info_Measure_Text); // TODO Bug sinon? voir Ztring
            if ((MediaInfoLib::Config.Complete_Get() || Get((stream_t)StreamKind, StreamPos, Champ_Pos, Info_Options)[InfoOption_ShowInInform]==_T('Y')) && !Get((stream_t)StreamKind, StreamPos, Champ_Pos, Info_Text).empty())
            {
                Ztring Nom=Get((stream_t)StreamKind, StreamPos, Champ_Pos, Info_Name_Text);
                if (Nom.empty())
                    Nom=Get((stream_t)StreamKind, StreamPos, Champ_Pos, Info_Name); //Texte n'existe pas
                if (!HTML && !XML && !CSV)
                {
                     int8u Nom_Size=MediaInfoLib::Config.Language_Get(_T("  Config_Text_ColumnSize")).To_int8u();
                     if (Nom_Size==0)
                        Nom_Size=32; //Default
                     Nom.resize(Nom_Size, ' ');
                }
                Ztring Valeur=Get((stream_t)StreamKind, StreamPos, Champ_Pos, Info_Text);
                Valeur.FindAndReplace(_T("\\"), _T("|SC1|"), 0, Ztring_Recursive);
                if (HTML)
                {
                    Retour+=_T("  <tr>\n    <td><i>");
                    Retour+=Nom;
                    Retour+=_T(" :</i></td>\n    <td colspan=\"3\">");
                    Retour+=Valeur;
                    Retour+=_T("</td>\n  </tr>");
                }
                else if (XML)
                {
                    Nom=Xml_Name_Escape(Nom);
                    Xml_Content_Escape_Modifying(Valeur);

                    Retour+=_T("<");
                    Retour+=Nom;
                    Retour+=_T(">");
                    Retour+=Valeur;
                    Retour+=_T("</");
                    Retour+=Nom;
                    Retour+=_T(">");
                }
                else if (CSV)
                {
                    Retour+=Nom;
                    Retour+=_T(",");
                    Retour+=Valeur;
                }
                else
                    Retour+=Nom + MediaInfoLib::Config.Language_Get(_T("  Config_Text_Separator")) + Valeur;
                Retour+=MediaInfoLib::Config.LineSeparator_Get();
            }
        }

        Retour.FindAndReplace(_T("\\r\\n"), _T("\n"), 0, Ztring_Recursive);
        Retour.FindAndReplace(_T("\\r"), _T("\n"), 0, Ztring_Recursive);
        Retour.FindAndReplace(_T("\\n"), _T("\n"), 0, Ztring_Recursive);
        Retour.FindAndReplace(_T("\r\n"), _T("\n"), 0, Ztring_Recursive);
        Retour.FindAndReplace(_T("\r"), _T("\n"), 0, Ztring_Recursive);
        Retour.FindAndReplace(_T("\n"), MediaInfoLib::Config.LineSeparator_Get(), 0, Ztring_Recursive);
        Retour.FindAndReplace(_T("|SC1|"), _T("\\"), 0, Ztring_Recursive);
        return Retour;
    }

    Ztring Retour=MediaInfoLib::Config.Inform_Get(Get(StreamKind, 0, _T("StreamKind"), Info_Text));
    ZtringList Info;

    if (StreamKind>=Stream_Max)
        return Ztring();
    Info=Stream[StreamKind][StreamPos];

    //Special characters
    Retour.FindAndReplace(_T("\\\\"), _T("|SC1|"), 0, Ztring_Recursive);
    Retour.FindAndReplace(_T("\\["), _T("|SC2|"), 0, Ztring_Recursive);
    Retour.FindAndReplace(_T("\\]"), _T("|SC3|"), 0, Ztring_Recursive);
    Retour.FindAndReplace(_T("\\,"), _T("|SC4|"), 0, Ztring_Recursive);
    Retour.FindAndReplace(_T("\\;"), _T("|SC5|"), 0, Ztring_Recursive);
    Retour.FindAndReplace(_T("\\("), _T("|SC6|"), 0, Ztring_Recursive);
    Retour.FindAndReplace(_T("\\)"), _T("|SC7|"), 0, Ztring_Recursive);

    //Gestion $xx$
    size_t PosX=0;
    while (Retour.find(_T("$"), PosX)!=(size_t)-1)
    {
        PosX=Retour.find(_T("$"), PosX);
        if (Retour.size()>PosX+2 && !(Retour(PosX+1)==_T('i') && Retour(PosX+2)==_T('f') && Retour(PosX+3)==_T('('))) //To keep out "%" without any signification, or "$if(..."
        {
            Ztring ARemplacer=Ztring(_T("$")+Retour.SubString(_T("$"), _T("$"), PosX))+_T("$");
            Ztring RemplacerPar=MediaInfoLib::Config.Language_Get(Retour.SubString(_T("$"), _T("$"), PosX)); //TODO : case sensitive
            Retour.FindAndReplace(ARemplacer, RemplacerPar);
        }
        else
            PosX++;
    }

    //Gestion $if()
    size_t Position=Retour.find(_T("$if("));
    while (Position!=Error && Position>0)
    {
        ZtringList Elements;
        size_t Elements_Index;
        Elements.Separator_Set(0, _T(","));
        Elements.Write(Retour.SubString(_T("$if("), _T(")"), Position));
        Elements(0)=Elements(0).SubString(_T("%"), _T("%"));

        //Test if there is something to replace
        size_t Pos=MediaInfoLib::Config.Info_Get(StreamKind).Find(Elements(0));
        if (Pos!=std::string::npos)
        {
            if (Info(Pos).size()>0)
                Elements_Index=1;
            else
                Elements_Index=2;
        }
        else
            Elements_Index=2;

        //Replace
        while (Elements(Elements_Index).SubString(_T("%"), _T("%")).size()>0)
        {
            Ztring ToReplace=Elements(Elements_Index).SubString(_T("%"), _T("%"));
            Ztring ReplacedBy=Info(MediaInfoLib::Config.Info_Get(StreamKind).Find(ToReplace));
            ToReplace=Ztring(_T("%"))+ToReplace+Ztring(_T("%"));
            Elements(Elements_Index).FindAndReplace(ToReplace, ReplacedBy);
        }

        Ztring ToReplace=Ztring(_T("$if("))+Retour.SubString(_T("$if("), _T(")"), Position)+_T(")");
        Retour.FindAndReplace(ToReplace, Elements(Elements_Index));
        Position=Retour.find(_T("$if("));
    }

    //Gestion []
    while (!Retour.SubString(_T("["), _T("]")).empty())
    {
        Ztring Crochets=Retour.SubString(_T("["), _T("]"));
        Ztring ValueToFind=Crochets.SubString(_T("%"), _T("%"));
        size_t ValueToFind_Pos=MediaInfoLib::Config.Info_Get(StreamKind).Find(ValueToFind);
        Ztring ARemplacer=Ztring(_T("[")+Crochets+_T("]"));
        if (ValueToFind_Pos!=Error)
        {
            Ztring RemplacerPar=Info(ValueToFind_Pos);
            if (RemplacerPar.empty())
                Retour.FindAndReplace(ARemplacer, Ztring());
            else
            {
                //Formate l'interieur
                Ztring ATraiter=Crochets;
                Ztring Crochets_ARemplacer=Ztring(_T("%")+ATraiter.SubString(_T("%"), _T("%")))+_T("%");
                Ztring Crochets_RemplacerPar=Info(MediaInfoLib::Config.Info_Get(StreamKind).Find(ATraiter.SubString(_T("%"), _T("%"))));
                ATraiter.FindAndReplace(Crochets_ARemplacer, Crochets_RemplacerPar);
                Retour.FindAndReplace(ARemplacer, ATraiter);
            }
        }
        else
            Retour.FindAndReplace(ARemplacer, Ztring());
    }

    //Gestion %xxx%
    PosX=0;
    while (Retour.find(_T("%"), PosX)!=(size_t)-1)
    {
        PosX=Retour.find(_T("%"), PosX);
        if (Retour.size()>PosX+2 && Retour[PosX+1]>=_T('A') && Retour[PosX+1]<=_T('Z')) //To keep out "%" without any signification
        {
            Ztring ARemplacer=Ztring(_T("%")+Retour.SubString(_T("%"), _T("%"), PosX))+_T("%");
            Ztring RemplacerPar=Get(StreamKind, StreamPos, Retour.SubString(_T("%"), _T("%"), PosX));
            RemplacerPar.FindAndReplace(_T("\\"), _T("|SC1|"), 0, Ztring_Recursive);
            RemplacerPar.FindAndReplace(_T("),"), _T("|SC9|"), 0, Ztring_Recursive);
            RemplacerPar.FindAndReplace(_T(")"), _T("|SC8|"), 0, Ztring_Recursive);
            Retour.FindAndReplace(ARemplacer, RemplacerPar);
        }
        else
            PosX++;
    }

    //Retour=_T("<table width=\"100%\" border=\"0\" cellpadding=\"1\" cellspacing=\"2\" style=\"border:1px solid Navy\">\n<tr>\n    <td width=\"150\">Video #0</td>\n  </tr>\r\n  <tr>\n    <td><i>Codec :</i></td>\n    <td colspan=\"3\">WMV1</td>\n  </tr>\r\n  <tr>\n    <td><i>Codec/Info :</i></td>\n    <td colspan=\"3\">Windows Media Video 7</td>\n  </tr>\r\n  <tr>\n    <td><i>Width :</i></td>\n    <td colspan=\"3\">200 pixels</td>\n  </tr>\r\n  <tr>\n    <td><i>Height :</i></td>\n    <td colspan=\"3\">150 pixels</td>\n  </tr>\r\n  <tr>\n    <td><i>Aspect ratio :</i></td>\n    <td colspan=\"3\">4/3</td>\n  </tr>\r\n  <tr>\n    <td><i>Resolution :</i></td>\n    <td colspan=\"3\">24 bits</td>\n  </tr>\r\n</table>\n");
    Retour.FindAndReplace(_T("\\r\\n"), _T("\n"), 0, Ztring_Recursive);
    Retour.FindAndReplace(_T("\\r"), _T("\n"), 0, Ztring_Recursive);
    Retour.FindAndReplace(_T("\\n"), _T("\n"), 0, Ztring_Recursive);
    Retour.FindAndReplace(_T("\r\n"), _T("\n"), 0, Ztring_Recursive);
    Retour.FindAndReplace(_T("\r"), _T("\n"), 0, Ztring_Recursive);
    Retour.FindAndReplace(_T("\n"), MediaInfoLib::Config.LineSeparator_Get(), 0, Ztring_Recursive);

    //Special characters
    if (IsDirect)
    {
        Retour.FindAndReplace(_T("|SC1|"), _T("\\"), 0, Ztring_Recursive);
        Retour.FindAndReplace(_T("|SC2|"), _T("["), 0, Ztring_Recursive);
        Retour.FindAndReplace(_T("|SC3|"), _T("]"), 0, Ztring_Recursive);
        Retour.FindAndReplace(_T("|SC4|"), _T(","), 0, Ztring_Recursive);
        Retour.FindAndReplace(_T("|SC5|"), _T(";"), 0, Ztring_Recursive);
        Retour.FindAndReplace(_T("|SC6|"), _T("("), 0, Ztring_Recursive);
        Retour.FindAndReplace(_T("|SC7|"), _T(")"), 0, Ztring_Recursive);
        Retour.FindAndReplace(_T("|SC8|"), _T(")"), 0, Ztring_Recursive);
        Retour.FindAndReplace(_T("|SC9|"), _T("),"), 0, Ztring_Recursive);
    }

    return Retour;
}

//---------------------------------------------------------------------------
void MediaInfo_Internal::Traiter(Ztring &C)
{
    //$if(%a%,zezeze%a%,rrere)
    size_t Position=C.find(_T("$if("));
    while (Position>0)
    {
        //Recuperation de la chaine entiere
        Ztring Total;
        Ztring ARemplacer;
        ZtringList Total1;
        Total1.Separator_Set(0, _T("),"));
        Total=C.SubString(_T("$if("), _T(")"), Position);
        ARemplacer=Ztring(_T("$if(")+Total+_T(")"));
        Total1.Write(Total);
        if (Total1(0).empty()) //mettre champ2
            C.FindAndReplace(ARemplacer, Total1(2), Position);
        else
            C.FindAndReplace(ARemplacer, Total1(1), Position);
        Position=C.find(_T("$if("), Position);
    }

    //reformatage
    C.FindAndReplace(_T("|SC8|"), _T(")"), 0, Ztring_Recursive);
    C.FindAndReplace(_T("|SC9|"), _T("),"), 0, Ztring_Recursive);
    //C.FindAndReplace(_T("\\r\\n"), _T("\n"), 0, Ztring_Recursive);
}

//---------------------------------------------------------------------------
Ztring MediaInfo_Internal::Xml_Name_Escape (const Ztring &Name)
{
    Ztring ToReturn(Name);

    if (ToReturn.operator()(0)>='0' && ToReturn.operator()(0)<='9')
        ToReturn.insert(0, 1, _T('_'));
    ToReturn.FindAndReplace(_T(" "), _T("_"), 0, Ztring_Recursive);
    ToReturn.FindAndReplace(_T("/"), _T("_"), 0, Ztring_Recursive);
    ToReturn.FindAndReplace(_T("("), _T("_"), 0, Ztring_Recursive);
    ToReturn.FindAndReplace(_T(")"), _T("_"), 0, Ztring_Recursive);
    ToReturn.FindAndReplace(_T("*"), _T("_"), 0, Ztring_Recursive);
    ToReturn.FindAndReplace(_T(","), _T("_"), 0, Ztring_Recursive);
    ToReturn.FindAndReplace(_T(":"), _T("_"), 0, Ztring_Recursive);
    ToReturn.FindAndReplace(_T("@"), _T("_"), 0, Ztring_Recursive);
    size_t ToReturn_Pos=0;
    while (ToReturn_Pos<ToReturn.size())
    {
        if (!(ToReturn[ToReturn_Pos]>=_T('A') && ToReturn[ToReturn_Pos]<=_T('Z'))
         && !(ToReturn[ToReturn_Pos]>=_T('a') && ToReturn[ToReturn_Pos]<=_T('z'))
         && !(ToReturn[ToReturn_Pos]>=_T('0') && ToReturn[ToReturn_Pos]<=_T('9'))
         && !(ToReturn[ToReturn_Pos]==_T('_')))
            ToReturn.erase(ToReturn_Pos, 1);
        else
            ToReturn_Pos++;
    }
    if (ToReturn.empty())
        ToReturn="Unknown";

    return ToReturn;
}

//---------------------------------------------------------------------------
Ztring MediaInfo_Internal::Xml_Content_Escape (const Ztring &Content)
{
    Ztring ToReturn(Content);
    return Xml_Content_Escape_Modifying(ToReturn);
}

//---------------------------------------------------------------------------
size_t Xml_Content_Escape_MustEscape(const Ztring &Content)
{
    size_t Pos=0;
    size_t Size=Content.size();
    for (; Pos<Size; Pos++)
    {
        switch (Content[Pos])
        {
            case _T('\"'):
            case _T('&') : 
            case _T('\''):
            case _T('<') : 
            case _T('>') :
                            return Pos;
            default      : 
                            if (Content[Pos]<0x20)
                                return Pos;
        }
    }

    return Pos;
}
Ztring &MediaInfo_Internal::Xml_Content_Escape_Modifying (Ztring &Content)
{
    size_t Pos=Xml_Content_Escape_MustEscape(Content);
    if (Pos>=Content.size())
        return Content;
    
    for (; Pos<Content.size(); Pos++)
    {
        switch (Content[Pos])
        {
            case _T('\"'):
                            Content[Pos]=_T('&');
                            Content.insert(Pos+1, _T("quot;"));
                            Pos+=5;
                            break;
            case _T('&') : 
                            Content[Pos]=_T('&');
                            Content.insert(Pos+1, _T("amp;"));
                            Pos+=4;
                            break;
            case _T('\''):
                            Content[Pos]=_T('&');
                            Content.insert(Pos+1, _T("apos;"));
                            Pos+=5;
                            break;
            case _T('<') : 
                            Content[Pos]=_T('&');
                            Content.insert(Pos+1, _T("lt;"));
                            Pos+=3;
                            break;
            case _T('>') :
                            Content[Pos]=_T('&');
                            Content.insert(Pos+1, _T("gt;"));
                            Pos+=3;
                            break;
            default   : 
                        if (Content[Pos]<0x20)
                        {
                            Ztring Character=_T("#x")+Ztring::ToZtring(Content[Pos]/16, 16)+Ztring::ToZtring(Content[Pos]%16, 16)+_T(";");
                            Content[Pos]=_T('&');
                            Content.insert(Pos+1, Character);
                            Pos+=5;
                        } 
        }
    }

    return Content;
}

} //NameSpace

