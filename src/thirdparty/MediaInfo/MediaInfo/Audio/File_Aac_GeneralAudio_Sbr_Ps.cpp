// File_Aac - Info for AAC (Raw) files
// Copyright (C) 2008-2012 MediaArea.net SARL, Info@MediaArea.net
//
// This library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Library General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with this library. If not, see <http://www.gnu.org/licenses/>.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
#if defined(MEDIAINFO_AAC_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_Aac.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//---------------------------------------------------------------------------
extern const char* Aac_audioObjectType(int8u audioObjectType);

//---------------------------------------------------------------------------
void File_Aac::ps_data(size_t End)
{
    FILLING_BEGIN();
        if (Infos["Format_Settings_PS"].empty())
        {
            Infos["Format_Profile"]=__T("HE-AACv2");
            Ztring Channels=Infos["Channel(s)"];
            Ztring ChannelPositions=Infos["ChannelPositions"];
            Ztring SamplingRate=Infos["SamplingRate"];
            Infos["Channel(s)"]=__T("2");
            Infos["ChannelPositions"]=__T("Front: L R");
            if (MediaInfoLib::Config.LegacyStreamDisplay_Get())
            {
                Infos["Format_Profile"]+=__T(" / HE-AAC / LC");
                Infos["Channel(s)"]+=__T(" / ")+Channels+__T(" / ")+Channels;
                Infos["ChannelPositions"]+=__T(" / ")+ChannelPositions+__T(" / ")+ChannelPositions;
                Infos["SamplingRate"]=Ztring().From_Number((extension_sampling_frequency_index==(int8u)-1)?(sampling_frequency*2):extension_sampling_frequency, 10)+__T(" / ")+SamplingRate;
            }
            Infos["Format_Settings_PS"]=__T("Yes (Implicit)");
            Ztring Codec=Retrieve(Stream_Audio, StreamPos_Last, Audio_Codec);
            Infos["Codec"]=Ztring().From_Local(Aac_audioObjectType(audioObjectType))+__T("-SBR-PS");
        }
    FILLING_END();

    //Parsing
    Element_Begin1("ps_data");
    bool enable_ps_header;
    Get_SB(enable_ps_header,                                    "enable_ps_header");
    if (enable_ps_header)
    {
        //Init
        delete ps; ps=new ps_handler();

        Get_SB(ps->enable_iid,                                  "enable_iid");
        if (ps->enable_iid)
        {
            Get_S1 (3, ps->iid_mode,                            "iid_mode");
        }
        Get_SB(ps->enable_icc,                                  "enable_icc");
        if (ps->enable_icc)
        {
            Get_S1 (3, ps->icc_mode,                            "icc_mode");
        }
        Get_SB(ps->enable_ext,                                  "enable_ext");
    }

    if (ps==NULL)
    {
        if (Data_BS_Remain()>End)
            Skip_BS(Data_BS_Remain()-End,                       "(Waiting for header)");
        Element_End0();
        return;
    }

    //PS not yet parsed
    if (Data_BS_Remain()>End)
        Skip_BS(Data_BS_Remain()-End,                           "Data");
    Element_End0();
}

} //NameSpace

#endif //MEDIAINFO_AAC_YES

