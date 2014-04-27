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

#pragma once

#include <Windows.h>
#include <list>

namespace Translations
{
    struct LanguageResource {
        LanguageResource()
            : LanguageResource(WORD_ERROR, nullptr, nullptr)
        {};

        LanguageResource(LANGID localeID, LPCTSTR name, LPCTSTR dllPath)
            : localeID(localeID)
            , name(name)
            , dllPath(dllPath)
        {};

        LANGID localeID; // Check http://msdn.microsoft.com/en-us/goglobal/bb964664
        LPCTSTR name;
        LPCTSTR dllPath;
    };

    LanguageResource GetLanguageResourceByLocaleID(LANGID localeID);

    std::list<const LanguageResource> GetAvailableLanguageResources();

    LANGID SetDefaultLanguage();
    bool SetLanguage(LanguageResource languageResource, bool showErrorMsg = true);
};
