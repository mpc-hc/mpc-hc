/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

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
#include "MediaInfo/Export/Export_EbuCore.h"
#include "MediaInfo/Export/Export_Mpeg7.h"
#include "MediaInfo/Export/Export_reVTMD.h"
#include "MediaInfo/Export/Export_PBCore.h"
#include "MediaInfo/File__Analyze.h"
#include "base64.h"
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
        if (MediaInfoLib::Config.Trace_Level_Get() || MediaInfoLib::Config.Inform_Get()==__T("Details"))
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

    if (MediaInfoLib::Config.Inform_Get()==__T("EBUCore") || MediaInfoLib::Config.Inform_Get()==__T("EBUCore_1.4"))
        return Export_EbuCore().Transform(*this);
    if (MediaInfoLib::Config.Inform_Get()==__T("MPEG-7"))
        return Export_Mpeg7().Transform(*this);
    if (MediaInfoLib::Config.Inform_Get()==__T("PBCore") || MediaInfoLib::Config.Inform_Get()==__T("PBCore_1.2"))
        return Export_PBCore().Transform(*this);
    if (MediaInfoLib::Config.Inform_Get()==__T("reVTMD"))
        return __T("reVTMD is disabled due to its non-free licensing."); //return Export_reVTMD().Transform(*this);

    if (!(
        MediaInfoLib::Config.Inform_Get(__T("General")).empty()
     && MediaInfoLib::Config.Inform_Get(__T("Video")).empty()
     && MediaInfoLib::Config.Inform_Get(__T("Audio")).empty()
     && MediaInfoLib::Config.Inform_Get(__T("Text")).empty()
     && MediaInfoLib::Config.Inform_Get(__T("Chapters")).empty()
     && MediaInfoLib::Config.Inform_Get(__T("Image")).empty()
     && MediaInfoLib::Config.Inform_Get(__T("Menu")).empty()
     ))
    {
        Ztring Retour;
        Retour+=MediaInfoLib::Config.Inform_Get(__T("File_Begin"));
        Retour+=MediaInfoLib::Config.Inform_Get(__T("General_Begin"));
        Retour+=Inform(Stream_General, 0, false);
        Retour+=MediaInfoLib::Config.Inform_Get(__T("General_End"));
        if (Count_Get(Stream_Video))
            Retour+=MediaInfoLib::Config.Inform_Get(__T("Video_Begin"));
        for (size_t I1=0; I1<Count_Get(Stream_Video); I1++)
        {
            Retour+=Inform(Stream_Video, I1, false);
            if (I1!=Count_Get(Stream_Video)-1)
                Retour+=MediaInfoLib::Config.Inform_Get(__T("Video_Middle"));
        }
        if (Count_Get(Stream_Video))
            Retour+=MediaInfoLib::Config.Inform_Get(__T("Video_End"));
        if (Count_Get(Stream_Audio))
            Retour+=MediaInfoLib::Config.Inform_Get(__T("Audio_Begin"));
        for (size_t I1=0; I1<Count_Get(Stream_Audio); I1++)
        {
            Retour+=Inform(Stream_Audio, I1, false);
            if (I1!=Count_Get(Stream_Audio)-1)
                Retour+=MediaInfoLib::Config.Inform_Get(__T("Audio_Middle"));
        }
        if (Count_Get(Stream_Audio))
            Retour+=MediaInfoLib::Config.Inform_Get(__T("Audio_End"));
        if (Count_Get(Stream_Text))
            Retour+=MediaInfoLib::Config.Inform_Get(__T("Text_Begin"));
        for (size_t I1=0; I1<Count_Get(Stream_Text); I1++)
        {
            Retour+=Inform(Stream_Text, I1, false);
            if (I1!=Count_Get(Stream_Text)-1)
                Retour+=MediaInfoLib::Config.Inform_Get(__T("Text_Middle"));
        }
        if (Count_Get(Stream_Text))
            Retour+=MediaInfoLib::Config.Inform_Get(__T("Text_End"));
        if (Count_Get(Stream_Other))
            Retour+=MediaInfoLib::Config.Inform_Get(__T("Chapters_Begin"));
        for (size_t I1=0; I1<Count_Get(Stream_Other); I1++)
        {
            Retour+=Inform(Stream_Other, I1, false);
            if (I1!=Count_Get(Stream_Other)-1)
                Retour+=MediaInfoLib::Config.Inform_Get(__T("Chapters_Middle"));
        }
        if (Count_Get(Stream_Other))
            Retour+=MediaInfoLib::Config.Inform_Get(__T("Chapters_End"));
        if (Count_Get(Stream_Image))
            Retour+=MediaInfoLib::Config.Inform_Get(__T("Image_Begin"));
        for (size_t I1=0; I1<Count_Get(Stream_Image); I1++)
        {
            Retour+=Inform(Stream_Image, I1, false);
            if (I1!=Count_Get(Stream_Image)-1)
                Retour+=MediaInfoLib::Config.Inform_Get(__T("Image_Middle"));
        }
        if (Count_Get(Stream_Image))
            Retour+=MediaInfoLib::Config.Inform_Get(__T("Image_End"));
        if (Count_Get(Stream_Menu))
            Retour+=MediaInfoLib::Config.Inform_Get(__T("Menu_Begin"));
        for (size_t I1=0; I1<Count_Get(Stream_Menu); I1++)
        {
            Retour+=Inform(Stream_Menu, I1, false);
            if (I1!=Count_Get(Stream_Menu)-1)
                Retour+=MediaInfoLib::Config.Inform_Get(__T("Menu_Middle"));
        }
        if (Count_Get(Stream_Menu))
            Retour+=MediaInfoLib::Config.Inform_Get(__T("Menu_End"));
        Retour+=MediaInfoLib::Config.Inform_Get(__T("File_End"));

        Retour.FindAndReplace(__T("\\r\\n"), __T("\n"), 0, Ztring_Recursive);
        Retour.FindAndReplace(__T("\\r"), __T("\n"), 0, Ztring_Recursive);
        Retour.FindAndReplace(__T("\\n"), __T("\n"), 0, Ztring_Recursive);
        Retour.FindAndReplace(__T("\r\n"), __T("\n"), 0, Ztring_Recursive);
        Retour.FindAndReplace(__T("\r"), __T("\n"), 0, Ztring_Recursive);
        Retour.FindAndReplace(__T("\n"), MediaInfoLib::Config.LineSeparator_Get(), 0, Ztring_Recursive);

        //Special characters
        Retour.FindAndReplace(__T("|SC1|"), __T("\\"), 0, Ztring_Recursive);
        Retour.FindAndReplace(__T("|SC2|"), __T("["), 0, Ztring_Recursive);
        Retour.FindAndReplace(__T("|SC3|"), __T("]"), 0, Ztring_Recursive);
        Retour.FindAndReplace(__T("|SC4|"), __T(","), 0, Ztring_Recursive);
        Retour.FindAndReplace(__T("|SC5|"), __T(";"), 0, Ztring_Recursive);
        Retour.FindAndReplace(__T("|SC6|"), __T("("), 0, Ztring_Recursive);
        Retour.FindAndReplace(__T("|SC7|"), __T(")"), 0, Ztring_Recursive);
        Retour.FindAndReplace(__T("|SC8|"), __T(")"), 0, Ztring_Recursive);
        Retour.FindAndReplace(__T("|SC9|"), __T("),"), 0, Ztring_Recursive);

        return Retour;
    }

    //Informations
    Ztring Retour;
    bool HTML=false;
    bool XML=false;
    bool CSV=false;
    if (MediaInfoLib::Config.Inform_Get()==__T("HTML"))
        HTML=true;
    if (MediaInfoLib::Config.Inform_Get()==__T("XML"))
        XML=true;
    if (MediaInfoLib::Config.Inform_Get()==__T("CSV"))
        CSV=true;

    if (HTML) Retour+=__T("<html>\n\n<head>\n<META http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" /></head>\n<body>\n");
    if (XML)  Retour+=__T("<File>\n");

    for (size_t StreamKind=(size_t)Stream_General; StreamKind<Stream_Max; StreamKind++)
    {
        //Pour chaque type de flux
        for (size_t StreamPos=0; StreamPos<(size_t)Count_Get((stream_t)StreamKind); StreamPos++)
        {
            //Pour chaque stream
            if (HTML) Retour+=__T("<table width=\"100%\" border=\"0\" cellpadding=\"1\" cellspacing=\"2\" style=\"border:1px solid Navy\">\n<tr>\n    <td width=\"150\"><h2>");
            if (XML) Retour+=__T("<track type=\"");
            Ztring A=Get((stream_t)StreamKind, StreamPos, __T("StreamKind/String"));
            Ztring B=Get((stream_t)StreamKind, StreamPos, __T("StreamKindPos"));
            if (!XML && !B.empty())
            {
                if (CSV)
                    A+=__T(",");
                else
                    A+=MediaInfoLib::Config.Language_Get(__T("  Config_Text_NumberTag"));
                A+=B;
            }
            Retour+=A;
            if (XML)
            {
                Retour+=__T("\"");
                if (!B.empty())
                {
                    Retour+=__T(" streamid=\"");
                    Retour+=B;
                    Retour+=__T("\"");
                }
            }
            if (HTML) Retour+=__T("</h2></td>\n  </tr>");
            if (XML) Retour+=__T(">");
            Retour+=MediaInfoLib::Config.LineSeparator_Get();
            Retour+=Inform((stream_t)StreamKind, StreamPos, false);
            Retour.FindAndReplace(__T("\\"), __T("|SC1|"), 0, Ztring_Recursive);
            if (HTML) Retour+=__T("</table>\n<br />");
            if (XML) Retour+=__T("</track>\n");
            Retour+=MediaInfoLib::Config.LineSeparator_Get();
        }
    }

    if (HTML) Retour+=__T("\n</body>\n</html>\n");
    if (XML)  Retour+=__T("</File>\n");

    Retour.FindAndReplace(__T("\\r\\n"), __T("\n"), 0, Ztring_Recursive);
    Retour.FindAndReplace(__T("\\r"), __T("\n"), 0, Ztring_Recursive);
    Retour.FindAndReplace(__T("\\n"), __T("\n"), 0, Ztring_Recursive);
    Retour.FindAndReplace(__T("\r\n"), __T("\n"), 0, Ztring_Recursive);
    Retour.FindAndReplace(__T("\r"), __T("\n"), 0, Ztring_Recursive);
    Retour.FindAndReplace(__T("\n"), MediaInfoLib::Config.LineSeparator_Get(), 0, Ztring_Recursive);

    //Special characters
    Retour.FindAndReplace(__T("|SC1|"), __T("\\"), 0, Ztring_Recursive);
    Retour.FindAndReplace(__T("|SC2|"), __T("["), 0, Ztring_Recursive);
    Retour.FindAndReplace(__T("|SC3|"), __T("]"), 0, Ztring_Recursive);
    Retour.FindAndReplace(__T("|SC4|"), __T(","), 0, Ztring_Recursive);
    Retour.FindAndReplace(__T("|SC5|"), __T(";"), 0, Ztring_Recursive);
    Retour.FindAndReplace(__T("|SC6|"), __T("("), 0, Ztring_Recursive);
    Retour.FindAndReplace(__T("|SC7|"), __T(")"), 0, Ztring_Recursive);
    Retour.FindAndReplace(__T("|SC8|"), __T(")"), 0, Ztring_Recursive);
    Retour.FindAndReplace(__T("|SC9|"), __T("),"), 0, Ztring_Recursive);

    return Retour;
}

//---------------------------------------------------------------------------
Ztring MediaInfo_Internal::Inform (stream_t StreamKind, size_t StreamPos, bool IsDirect)
{
    //Integrity
    if (StreamKind>=Stream_Max || StreamPos>=Stream[StreamKind].size())
        return Ztring();

    if (MediaInfoLib::Config.Inform_Get(__T("General")).empty()
     && MediaInfoLib::Config.Inform_Get(__T("Video")).empty()
     && MediaInfoLib::Config.Inform_Get(__T("Audio")).empty()
     && MediaInfoLib::Config.Inform_Get(__T("Text")).empty()
     && MediaInfoLib::Config.Inform_Get(__T("Chapters")).empty()
     && MediaInfoLib::Config.Inform_Get(__T("Image")).empty()
     && MediaInfoLib::Config.Inform_Get(__T("Menu")).empty())
    {
        Ztring Retour;
        bool HTML=false;
        bool XML=false;
        bool CSV=false;
        if (MediaInfoLib::Config.Inform_Get()==__T("HTML"))
            HTML=true;
        if (MediaInfoLib::Config.Inform_Get()==__T("XML"))
            XML=true;
        if (MediaInfoLib::Config.Inform_Get()==__T("CSV"))
            CSV=true;
        size_t Size=Count_Get(StreamKind, StreamPos);
        for (size_t Champ_Pos=0; Champ_Pos<Size; Champ_Pos++)
        {
            //Pour chaque champ
            //Ztring A=Get((stream_t)4, 2, 0, Info_Measure_Text); // TODO Bug sinon? voir Ztring
            Ztring A=Get((stream_t)StreamKind, StreamPos, Champ_Pos, Info_Measure_Text); // TODO Bug sinon? voir Ztring
            if ((MediaInfoLib::Config.Complete_Get() || Get((stream_t)StreamKind, StreamPos, Champ_Pos, Info_Options)[InfoOption_ShowInInform]==__T('Y')) && !Get((stream_t)StreamKind, StreamPos, Champ_Pos, Info_Text).empty())
            {
                Ztring Nom=Get((stream_t)StreamKind, StreamPos, Champ_Pos, Info_Name_Text);
                if (Nom.empty())
                    Nom=Get((stream_t)StreamKind, StreamPos, Champ_Pos, Info_Name); //Texte n'existe pas
                if (!HTML && !XML && !CSV)
                {
                     int8u Nom_Size=MediaInfoLib::Config.Language_Get(__T("  Config_Text_ColumnSize")).To_int8u();
                     if (Nom_Size==0)
                        Nom_Size=32; //Default
                     Nom.resize(Nom_Size, ' ');
                }
                Ztring Valeur=Get((stream_t)StreamKind, StreamPos, Champ_Pos, Info_Text);
                Valeur.FindAndReplace(__T("\\"), __T("|SC1|"), 0, Ztring_Recursive);
                if (HTML)
                {
                    Retour+=__T("  <tr>\n    <td><i>");
                    Retour+=Nom;
                    Retour+=__T(" :</i></td>\n    <td colspan=\"3\">");
                    Retour+=Valeur;
                    Retour+=__T("</td>\n  </tr>");
                }
                else if (XML)
                {
                    Nom=Xml_Name_Escape(Nom);
                    size_t Modified;
                    Xml_Content_Escape_Modifying(Valeur, Modified);

                    Retour+=__T("<");
                    Retour+=Nom;
                    if (Modified==1 && !MediaInfoLib::Config.SkipBinaryData_Get()) //Base64
                        Retour+=__T(" dt=\"binary.base64\"");
                    Retour+=__T(">");
                    if (Modified==1 && MediaInfoLib::Config.SkipBinaryData_Get())
                        Retour+=__T("(Binary data)");
                    else
                        Retour+=Valeur;
                    Retour+=__T("</");
                    Retour+=Nom;
                    Retour+=__T(">");
                }
                else if (CSV)
                {
                    Retour+=Nom;
                    Retour+=__T(",");
                    Retour+=Valeur;
                }
                else
                    Retour+=Nom + MediaInfoLib::Config.Language_Get(__T("  Config_Text_Separator")) + Valeur;
                Retour+=MediaInfoLib::Config.LineSeparator_Get();
            }
        }

        Retour.FindAndReplace(__T("\\r\\n"), __T("\n"), 0, Ztring_Recursive);
        Retour.FindAndReplace(__T("\\r"), __T("\n"), 0, Ztring_Recursive);
        Retour.FindAndReplace(__T("\\n"), __T("\n"), 0, Ztring_Recursive);
        Retour.FindAndReplace(__T("\r\n"), __T("\n"), 0, Ztring_Recursive);
        Retour.FindAndReplace(__T("\r"), __T("\n"), 0, Ztring_Recursive);
        Retour.FindAndReplace(__T("\n"), MediaInfoLib::Config.LineSeparator_Get(), 0, Ztring_Recursive);
        Retour.FindAndReplace(__T("|SC1|"), __T("\\"), 0, Ztring_Recursive);
        return Retour;
    }

    Ztring Retour=MediaInfoLib::Config.Inform_Get(Get(StreamKind, 0, __T("StreamKind"), Info_Text));
    ZtringList Info;

    if (StreamKind>=Stream_Max)
        return Ztring();
    Info=Stream[StreamKind][StreamPos];

    //Special characters
    Retour.FindAndReplace(__T("\\\\"), __T("|SC1|"), 0, Ztring_Recursive);
    Retour.FindAndReplace(__T("\\["), __T("|SC2|"), 0, Ztring_Recursive);
    Retour.FindAndReplace(__T("\\]"), __T("|SC3|"), 0, Ztring_Recursive);
    Retour.FindAndReplace(__T("\\,"), __T("|SC4|"), 0, Ztring_Recursive);
    Retour.FindAndReplace(__T("\\;"), __T("|SC5|"), 0, Ztring_Recursive);
    Retour.FindAndReplace(__T("\\("), __T("|SC6|"), 0, Ztring_Recursive);
    Retour.FindAndReplace(__T("\\)"), __T("|SC7|"), 0, Ztring_Recursive);

    //Gestion $xx$
    size_t PosX=0;
    while (Retour.find(__T("$"), PosX)!=(size_t)-1)
    {
        PosX=Retour.find(__T("$"), PosX);
        if (Retour.size()>PosX+2 && !(Retour(PosX+1)==__T('i') && Retour(PosX+2)==__T('f') && Retour(PosX+3)==__T('('))) //To keep out "%" without any signification, or "$if(..."
        {
            Ztring ARemplacer=Ztring(__T("$")+Retour.SubString(__T("$"), __T("$"), PosX))+__T("$");
            Ztring RemplacerPar=MediaInfoLib::Config.Language_Get(Retour.SubString(__T("$"), __T("$"), PosX)); //TODO : case sensitive
            Retour.FindAndReplace(ARemplacer, RemplacerPar);
        }
        else
            PosX++;
    }

    //Gestion $if()
    size_t Position=Retour.find(__T("$if("));
    while (Position!=Error && Position>0)
    {
        ZtringList Elements;
        size_t Elements_Index;
        Elements.Separator_Set(0, __T(","));
        Elements.Write(Retour.SubString(__T("$if("), __T(")"), Position));
        Elements(0)=Elements(0).SubString(__T("%"), __T("%"));

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
        while (Elements(Elements_Index).SubString(__T("%"), __T("%")).size()>0)
        {
            Ztring ToReplace=Elements(Elements_Index).SubString(__T("%"), __T("%"));
            Ztring ReplacedBy=Info(MediaInfoLib::Config.Info_Get(StreamKind).Find(ToReplace));
            ToReplace=Ztring(__T("%"))+ToReplace+Ztring(__T("%"));
            Elements(Elements_Index).FindAndReplace(ToReplace, ReplacedBy);
        }

        Ztring ToReplace=Ztring(__T("$if("))+Retour.SubString(__T("$if("), __T(")"), Position)+__T(")");
        Retour.FindAndReplace(ToReplace, Elements(Elements_Index));
        Position=Retour.find(__T("$if("));
    }

    //Gestion []
    while (!Retour.SubString(__T("["), __T("]")).empty())
    {
        Ztring Crochets=Retour.SubString(__T("["), __T("]"));
        Ztring ValueToFind=Crochets.SubString(__T("%"), __T("%"));
        size_t ValueToFind_Pos=MediaInfoLib::Config.Info_Get(StreamKind).Find(ValueToFind);
        Ztring ARemplacer=Ztring(__T("[")+Crochets+__T("]"));
        if (ValueToFind_Pos!=Error)
        {
            Ztring RemplacerPar=Info(ValueToFind_Pos);
            if (RemplacerPar.empty())
                Retour.FindAndReplace(ARemplacer, Ztring());
            else
            {
                //Formate l'interieur
                Ztring ATraiter=Crochets;
                Ztring Crochets_ARemplacer=Ztring(__T("%")+ATraiter.SubString(__T("%"), __T("%")))+__T("%");
                Ztring Crochets_RemplacerPar=Info(MediaInfoLib::Config.Info_Get(StreamKind).Find(ATraiter.SubString(__T("%"), __T("%"))));
                ATraiter.FindAndReplace(Crochets_ARemplacer, Crochets_RemplacerPar);
                Retour.FindAndReplace(ARemplacer, ATraiter);
            }
        }
        else
            Retour.FindAndReplace(ARemplacer, Ztring());
    }

    //Gestion %xxx%
    PosX=0;
    while (Retour.find(__T("%"), PosX)!=(size_t)-1)
    {
        PosX=Retour.find(__T("%"), PosX);
        if (Retour.size()>PosX+2 && Retour[PosX+1]>=__T('A') && Retour[PosX+1]<=__T('Z')) //To keep out "%" without any signification
        {
            Ztring ARemplacer=Ztring(__T("%")+Retour.SubString(__T("%"), __T("%"), PosX))+__T("%");
            Ztring RemplacerPar=Get(StreamKind, StreamPos, Retour.SubString(__T("%"), __T("%"), PosX));
            RemplacerPar.FindAndReplace(__T("\\"), __T("|SC1|"), 0, Ztring_Recursive);
            RemplacerPar.FindAndReplace(__T("),"), __T("|SC9|"), 0, Ztring_Recursive);
            RemplacerPar.FindAndReplace(__T(")"), __T("|SC8|"), 0, Ztring_Recursive);
            Retour.FindAndReplace(ARemplacer, RemplacerPar);
        }
        else
            PosX++;
    }

    //Retour=__T("<table width=\"100%\" border=\"0\" cellpadding=\"1\" cellspacing=\"2\" style=\"border:1px solid Navy\">\n<tr>\n    <td width=\"150\">Video #0</td>\n  </tr>\r\n  <tr>\n    <td><i>Codec :</i></td>\n    <td colspan=\"3\">WMV1</td>\n  </tr>\r\n  <tr>\n    <td><i>Codec/Info :</i></td>\n    <td colspan=\"3\">Windows Media Video 7</td>\n  </tr>\r\n  <tr>\n    <td><i>Width :</i></td>\n    <td colspan=\"3\">200 pixels</td>\n  </tr>\r\n  <tr>\n    <td><i>Height :</i></td>\n    <td colspan=\"3\">150 pixels</td>\n  </tr>\r\n  <tr>\n    <td><i>Aspect ratio :</i></td>\n    <td colspan=\"3\">4/3</td>\n  </tr>\r\n  <tr>\n    <td><i>Resolution :</i></td>\n    <td colspan=\"3\">24 bits</td>\n  </tr>\r\n</table>\n");
    Retour.FindAndReplace(__T("\\r\\n"), __T("\n"), 0, Ztring_Recursive);
    Retour.FindAndReplace(__T("\\r"), __T("\n"), 0, Ztring_Recursive);
    Retour.FindAndReplace(__T("\\n"), __T("\n"), 0, Ztring_Recursive);
    Retour.FindAndReplace(__T("\r\n"), __T("\n"), 0, Ztring_Recursive);
    Retour.FindAndReplace(__T("\r"), __T("\n"), 0, Ztring_Recursive);
    Retour.FindAndReplace(__T("\n"), MediaInfoLib::Config.LineSeparator_Get(), 0, Ztring_Recursive);

    //Special characters
    if (IsDirect)
    {
        Retour.FindAndReplace(__T("|SC1|"), __T("\\"), 0, Ztring_Recursive);
        Retour.FindAndReplace(__T("|SC2|"), __T("["), 0, Ztring_Recursive);
        Retour.FindAndReplace(__T("|SC3|"), __T("]"), 0, Ztring_Recursive);
        Retour.FindAndReplace(__T("|SC4|"), __T(","), 0, Ztring_Recursive);
        Retour.FindAndReplace(__T("|SC5|"), __T(";"), 0, Ztring_Recursive);
        Retour.FindAndReplace(__T("|SC6|"), __T("("), 0, Ztring_Recursive);
        Retour.FindAndReplace(__T("|SC7|"), __T(")"), 0, Ztring_Recursive);
        Retour.FindAndReplace(__T("|SC8|"), __T(")"), 0, Ztring_Recursive);
        Retour.FindAndReplace(__T("|SC9|"), __T("),"), 0, Ztring_Recursive);
    }

    return Retour;
}

//---------------------------------------------------------------------------
void MediaInfo_Internal::Traiter(Ztring &C)
{
    //$if(%a%,zezeze%a%,rrere)
    size_t Position=C.find(__T("$if("));
    while (Position>0)
    {
        //Recuperation de la chaine entiere
        Ztring Total;
        Ztring ARemplacer;
        ZtringList Total1;
        Total1.Separator_Set(0, __T("),"));
        Total=C.SubString(__T("$if("), __T(")"), Position);
        ARemplacer=Ztring(__T("$if(")+Total+__T(")"));
        Total1.Write(Total);
        if (Total1(0).empty()) //mettre champ2
            C.FindAndReplace(ARemplacer, Total1(2), Position);
        else
            C.FindAndReplace(ARemplacer, Total1(1), Position);
        Position=C.find(__T("$if("), Position);
    }

    //reformatage
    C.FindAndReplace(__T("|SC8|"), __T(")"), 0, Ztring_Recursive);
    C.FindAndReplace(__T("|SC9|"), __T("),"), 0, Ztring_Recursive);
    //C.FindAndReplace(__T("\\r\\n"), __T("\n"), 0, Ztring_Recursive);
}

//---------------------------------------------------------------------------
Ztring MediaInfo_Internal::Xml_Name_Escape (const Ztring &Name)
{
    Ztring ToReturn(Name);

    if (ToReturn.operator()(0)>='0' && ToReturn.operator()(0)<='9')
        ToReturn.insert(0, 1, __T('_'));
    ToReturn.FindAndReplace(__T(" "), __T("_"), 0, Ztring_Recursive);
    ToReturn.FindAndReplace(__T("/"), __T("_"), 0, Ztring_Recursive);
    ToReturn.FindAndReplace(__T("("), __T("_"), 0, Ztring_Recursive);
    ToReturn.FindAndReplace(__T(")"), __T("_"), 0, Ztring_Recursive);
    ToReturn.FindAndReplace(__T("*"), __T("_"), 0, Ztring_Recursive);
    ToReturn.FindAndReplace(__T(","), __T("_"), 0, Ztring_Recursive);
    ToReturn.FindAndReplace(__T(":"), __T("_"), 0, Ztring_Recursive);
    ToReturn.FindAndReplace(__T("@"), __T("_"), 0, Ztring_Recursive);
    size_t ToReturn_Pos=0;
    while (ToReturn_Pos<ToReturn.size())
    {
        if (!(ToReturn[ToReturn_Pos]>=__T('A') && ToReturn[ToReturn_Pos]<=__T('Z'))
         && !(ToReturn[ToReturn_Pos]>=__T('a') && ToReturn[ToReturn_Pos]<=__T('z'))
         && !(ToReturn[ToReturn_Pos]>=__T('0') && ToReturn[ToReturn_Pos]<=__T('9'))
         && !(ToReturn[ToReturn_Pos]==__T('_')))
            ToReturn.erase(ToReturn_Pos, 1);
        else
            ToReturn_Pos++;
    }
    if (ToReturn.empty())
        ToReturn="Unknown";

    return ToReturn;
}

//---------------------------------------------------------------------------
Ztring MediaInfo_Internal::Xml_Content_Escape (const Ztring &Content, size_t &Modified)
{
    Ztring ToReturn(Content);
    return Xml_Content_Escape_Modifying(ToReturn, Modified);
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
            case __T('\"'):
            case __T('&') :
            case __T('\''):
            case __T('<') :
            case __T('>') :
                            return Pos;
            default      :
                            if (Content[Pos]<0x20)
                                return Pos;
        }
    }

    return Pos;
}
Ztring &MediaInfo_Internal::Xml_Content_Escape_Modifying (Ztring &Content, size_t &Modified)
{
    size_t Pos=Xml_Content_Escape_MustEscape(Content);
    Ztring Content_Save=Content;
    Modified=0;
    if (Pos>=Content.size())
        return Content;

    for (; Pos<Content.size(); Pos++)
    {
        switch (Content[Pos])
        {
            case __T('\"'):
                            Content[Pos]=__T('&');
                            Content.insert(Pos+1, __T("quot;"));
                            Pos+=5;
                            break;
            case __T('&'):
                            Content[Pos]=__T('&');
                            Content.insert(Pos+1, __T("amp;"));
                            Pos+=4;
                            break;
            case __T('\''):
                            Content[Pos]=__T('&');
                            Content.insert(Pos+1, __T("apos;"));
                            Pos+=5;
                            break;
            case __T('<'):
                            Content[Pos]=__T('&');
                            Content.insert(Pos+1, __T("lt;"));
                            Pos+=3;
                            break;
            case __T('>'):
                            Content[Pos]=__T('&');
                            Content.insert(Pos+1, __T("gt;"));
                            Pos+=3;
                            break;
            default:
                        if (Content[Pos]<0x20)
                        {
                            /* Is still invalid XML
                            Ztring Character=__T("#x")+Ztring::ToZtring(Content[Pos]/16, 16)+Ztring::ToZtring(Content[Pos]%16, 16)+__T(";");
                            Content[Pos]=__T('&');
                            Content.insert(Pos+1, Character);
                            Pos+=5;
                            */
                            string Content_Utf8=Content_Save.To_UTF8(); //TODO: shouldn't we never convert to Unicode?
                            string Content_Base64=Base64::encode(Content_Utf8);
                            Content.From_UTF8(Content_Base64);
                            Modified=1; //Base64
                            Pos=Content.size(); //End
                        }
        }
    }

    return Content;
}

} //NameSpace
