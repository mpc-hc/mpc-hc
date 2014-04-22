/*
* (C) 2014 see Authors.txt
*
* This file is part of MPC-HC.
*
* MPC-HC is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3 of the License, or
* (at your option) any later version.
*
* MPC-HC is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#include "stdafx.h"
#include "Translations.h"
#include "WinAPIUtils.h"

static const std::vector<const Translations::LanguageResource> languageResources = {
    { ID_LANGUAGE_ARMENIAN,             1067,   _T("Armenian"),                 _T("Lang\\mpcresources.hy.dll") },
    { ID_LANGUAGE_BASQUE,               1069,   _T("Basque"),                   _T("Lang\\mpcresources.eu.dll") },
    { ID_LANGUAGE_BELARUSIAN,           1059,   _T("Belarusian"),               _T("Lang\\mpcresources.be.dll") },
    { ID_LANGUAGE_BENGALI,              1093,   _T("Bengali"),                  _T("Lang\\mpcresources.bn.dll") },
    { ID_LANGUAGE_CATALAN,              1027,   _T("Catalan"),                  _T("Lang\\mpcresources.ca.dll") },
    { ID_LANGUAGE_CHINESE_SIMPLIFIED,   2052,   _T("Chinese (Simplified)"),     _T("Lang\\mpcresources.zh_CN.dll") },
    { ID_LANGUAGE_CHINESE_TRADITIONAL,  1028,   _T("Chinese (Traditional)"),    _T("Lang\\mpcresources.zh_TW.dll") },
    { ID_LANGUAGE_CROATIAN,             1050,   _T("Croatian"),                 _T("Lang\\mpcresources.hr.dll") },
    { ID_LANGUAGE_CZECH,                1029,   _T("Czech"),                    _T("Lang\\mpcresources.cs.dll") },
    { ID_LANGUAGE_DUTCH,                1043,   _T("Dutch"),                    _T("Lang\\mpcresources.nl.dll") },
    { ID_LANGUAGE_ENGLISH,              0,      _T("English"),                  nullptr },
    { ID_LANGUAGE_ENGLISH_GB,           2057,   _T("English (British)"),        _T("Lang\\mpcresources.en_GB.dll") },
    { ID_LANGUAGE_FRENCH,               1036,   _T("French"),                   _T("Lang\\mpcresources.fr.dll") },
    { ID_LANGUAGE_GALICIAN,             1110,   _T("Galician"),                 _T("Lang\\mpcresources.gl.dll") },
    { ID_LANGUAGE_GERMAN,               1031,   _T("German"),                   _T("Lang\\mpcresources.de.dll") },
    { ID_LANGUAGE_GREEK,                1032,   _T("Greek"),                    _T("Lang\\mpcresources.el.dll") },
    { ID_LANGUAGE_HEBREW,               1037,   _T("Hebrew"),                   _T("Lang\\mpcresources.he.dll") },
    { ID_LANGUAGE_HUNGARIAN,            1038,   _T("Hungarian"),                _T("Lang\\mpcresources.hu.dll") },
    { ID_LANGUAGE_ITALIAN,              1040,   _T("Italian"),                  _T("Lang\\mpcresources.it.dll") },
    { ID_LANGUAGE_JAPANESE,             1041,   _T("Japanese"),                 _T("Lang\\mpcresources.ja.dll") },
    { ID_LANGUAGE_KOREAN,               1042,   _T("Korean"),                   _T("Lang\\mpcresources.ko.dll") },
    { ID_LANGUAGE_MALAY,                1086,   _T("Malay"),                    _T("Lang\\mpcresources.ms_MY.dll") },
    { ID_LANGUAGE_POLISH,               1045,   _T("Polish"),                   _T("Lang\\mpcresources.pl.dll") },
    { ID_LANGUAGE_PORTUGUESE_BR,        1046,   _T("Portuguese (Brazil)"),      _T("Lang\\mpcresources.pt_BR.dll") },
    { ID_LANGUAGE_ROMANIAN,             1048,   _T("Romanian"),                 _T("Lang\\mpcresources.ro.dll") },
    { ID_LANGUAGE_RUSSIAN,              1049,   _T("Russian"),                  _T("Lang\\mpcresources.ru.dll") },
    { ID_LANGUAGE_SLOVAK,               1051,   _T("Slovak"),                   _T("Lang\\mpcresources.sk.dll") },
    { ID_LANGUAGE_SLOVENIAN,            1060,   _T("Slovenian"),                _T("Lang\\mpcresources.sl.dll") },
    { ID_LANGUAGE_SWEDISH,              1053,   _T("Swedish"),                  _T("Lang\\mpcresources.sv.dll") },
    { ID_LANGUAGE_SPANISH,              3082,   _T("Spanish"),                  _T("Lang\\mpcresources.es.dll") },
    { ID_LANGUAGE_TATAR,                1092,   _T("Tatar"),                    _T("Lang\\mpcresources.tt.dll") },
    { ID_LANGUAGE_TURKISH,              1055,   _T("Turkish"),                  _T("Lang\\mpcresources.tr.dll") },
    { ID_LANGUAGE_UKRAINIAN,            1058,   _T("Ukrainian"),                _T("Lang\\mpcresources.uk.dll") },
    { ID_LANGUAGE_VIETNAMESE,           1066,   _T("Vietnamese"),               _T("Lang\\mpcresources.vi.dll") }
};

Translations::LanguageResource Translations::GetLanguageResourceByResourceID(UINT resourceID)
{
    Translations::LanguageResource defaultResource;

    for (auto& lr : languageResources) {
        if (resourceID == lr.resourceID) {
            return lr;
        } else if (ID_LANGUAGE_ENGLISH == lr.resourceID) {
            defaultResource = lr;
        }
    }

    return defaultResource;
}

Translations::LanguageResource Translations::GetLanguageResourceByLocaleID(LANGID localeID)
{
    Translations::LanguageResource defaultResource;

    for (auto& lr : languageResources) {
        if (localeID == lr.localeID) {
            return lr;
        } else if (0 == lr.localeID) {
            defaultResource = lr;
        }
    }

    return defaultResource;
}

std::list<const Translations::LanguageResource> Translations::GetAvailableLanguageResources()
{
    std::list<const Translations::LanguageResource> availableResources;

    CString appPath = GetProgramPath();

    for (auto& lr : languageResources) {
        if (0 == lr.localeID || FileExists(appPath + lr.dllPath)) {
            availableResources.emplace_back(lr);
        }
    }

    return availableResources;
}
