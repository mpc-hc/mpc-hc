/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2014, 2016-2017 see Authors.txt
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
#include <atlutil.h>
#include "text.h"

DWORD CharSetToCodePage(DWORD dwCharSet)
{
    if (dwCharSet == CP_UTF8) {
        return CP_UTF8;
    }
    if (dwCharSet == CP_UTF7) {
        return CP_UTF7;
    }
    CHARSETINFO cs;
    ZeroMemory(&cs, sizeof(CHARSETINFO));
    ::TranslateCharsetInfo((DWORD*)(DWORD_PTR)dwCharSet, &cs, TCI_SRCCHARSET);
    return cs.ciACP;
}

CStringA ConvertMBCS(CStringA str, DWORD SrcCharSet, DWORD DstCharSet)
{
    WCHAR* utf16 = DEBUG_NEW WCHAR[str.GetLength() + 1];
    ZeroMemory(utf16, (str.GetLength() + 1)*sizeof(WCHAR));

    CHAR* mbcs = DEBUG_NEW CHAR[str.GetLength() * 6 + 1];
    ZeroMemory(mbcs, str.GetLength() * 6 + 1);

    int len = MultiByteToWideChar(
                  CharSetToCodePage(SrcCharSet),
                  0,
                  str,
                  -1, // null terminated string
                  utf16,
                  str.GetLength() + 1);

    len = WideCharToMultiByte(
              CharSetToCodePage(DstCharSet),
              0,
              utf16,
              len,
              mbcs,
              str.GetLength() * 6,
              nullptr,
              nullptr);

    str = mbcs;

    delete [] utf16;
    delete [] mbcs;

    return str;
}

CStringA UrlEncode(const CStringA& strIn)
{
    CStringA strOut;
    DWORD dwStrLen = 0, dwMaxLength = 0;
    // Request the buffer size needed to encode the URL
    AtlEscapeUrl(strIn, strOut.GetBuffer(), &dwStrLen, dwMaxLength, ATL_URL_ENCODE_PERCENT);
    dwMaxLength = dwStrLen;
    // Encode the URL
    if (dwMaxLength > 0) {
        if (AtlEscapeUrl(strIn, strOut.GetBuffer(int(dwMaxLength)), &dwStrLen, dwMaxLength, ATL_URL_ENCODE_PERCENT)) {
            dwStrLen--;
        } else {
            dwStrLen = 0;
        }
        strOut.ReleaseBuffer(dwStrLen);
    }

    return strOut;
}

CStringA EscapeJSONString(const CStringA& str)
{
    CStringA escapedString = str;
    // replace all of JSON's reserved characters with their escaped
    // equivalents.
    escapedString.Replace("\"", "\\\"");
    escapedString.Replace("\\", "\\\\");
    escapedString.Replace("/", "\\/");
    escapedString.Replace("\b", "\\b");
    escapedString.Replace("\f", "\\f");
    escapedString.Replace("\n", "\\n");
    escapedString.Replace("\r", "\\r");
    escapedString.Replace("\t", "\\t");
    return escapedString;
}

CStringA UrlDecode(const CStringA& strIn)
{
    CStringA strOut;
    DWORD dwStrLen = 0, dwMaxLength = strIn.GetLength() + 1;

    if (AtlUnescapeUrl(strIn, strOut.GetBuffer(int(dwMaxLength)), &dwStrLen, dwMaxLength)) {
        dwStrLen--;
    } else {
        dwStrLen = 0;
    }
    strOut.ReleaseBuffer(dwStrLen);

    return strOut;
}

CString ExtractTag(CString tag, CMapStringToString& attribs, bool& fClosing)
{
    tag.Trim();
    attribs.RemoveAll();

    fClosing = !tag.IsEmpty() ? tag[0] == '/' : false;
    tag.TrimLeft('/');

    int i = tag.Find(' ');
    if (i < 0) {
        i = tag.GetLength();
    }
    CString type = tag.Left(i).MakeLower();
    tag = tag.Mid(i).Trim();

    while ((i = tag.Find('=')) > 0) {
        CString attrib = tag.Left(i).Trim().MakeLower();
        tag = tag.Mid(i + 1);
        for (i = 0; i < tag.GetLength() && _istspace(tag[i]); i++) {
            ;
        }
        if (i < tag.GetLength()) {
            tag = tag.Mid(i);
        } else {
            tag.Empty();
        }
        if (!tag.IsEmpty() && tag[0] == '\"') {
            tag = tag.Mid(1);
            i = tag.Find('\"');
        } else {
            i = tag.Find(' ');
        }
        if (i < 0) {
            i = tag.GetLength();
        }
        CString param = tag.Left(i).Trim();
        if (!param.IsEmpty()) {
            attribs[attrib] = param;
        }
        if (i + 1 < tag.GetLength()) {
            tag = tag.Mid(i + 1);
        } else {
            tag.Empty();
        }
    }

    return type;
}

CStringA HtmlSpecialChars(CStringA str, bool bQuotes /*= false*/)
{
    str.Replace("&", "&amp;");
    str.Replace("\"", "&quot;");
    if (bQuotes) {
        str.Replace("\'", "&#039;");
    }
    str.Replace("<", "&lt;");
    str.Replace(">", "&gt;");

    return str;
}

CStringA HtmlSpecialCharsDecode(CStringA str)
{
    str.Replace("&amp;", "&");
    str.Replace("&quot;", "\"");
    str.Replace("&#039;", "\'");
    str.Replace("&lt;", "<");
    str.Replace("&gt;", ">");
    str.Replace("&rsquo;", "'");

    return str;
}

CAtlList<CString>& MakeLower(CAtlList<CString>& sl)
{
    POSITION pos = sl.GetHeadPosition();
    while (pos) {
        sl.GetNext(pos).MakeLower();
    }
    return sl;
}

CAtlList<CString>& MakeUpper(CAtlList<CString>& sl)
{
    POSITION pos = sl.GetHeadPosition();
    while (pos) {
        sl.GetNext(pos).MakeUpper();
    }
    return sl;
}

CString FormatNumber(CString szNumber, bool bNoFractionalDigits /*= true*/)
{
    CString ret;

    int nChars = GetNumberFormat(LOCALE_USER_DEFAULT, 0, szNumber, nullptr, nullptr, 0);
    GetNumberFormat(LOCALE_USER_DEFAULT, 0, szNumber, nullptr, ret.GetBuffer(nChars), nChars);
    ret.ReleaseBuffer();

    if (bNoFractionalDigits) {
        TCHAR szNumberFractionalDigits[2] = {0};
        GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IDIGITS, szNumberFractionalDigits, _countof(szNumberFractionalDigits));
        int nNumberFractionalDigits = _tcstol(szNumberFractionalDigits, nullptr, 10);
        if (nNumberFractionalDigits) {
            ret.Truncate(ret.GetLength() - nNumberFractionalDigits - 1);
        }
    }

    return ret;
}
