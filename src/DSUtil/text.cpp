/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2012 see Authors.txt
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
#include "text.h"

DWORD CharSetToCodePage(DWORD dwCharSet)
{
    if (dwCharSet == CP_UTF8) {
        return CP_UTF8;
    }
    if (dwCharSet == CP_UTF7) {
        return CP_UTF7;
    }
    CHARSETINFO cs = {0};
    ::TranslateCharsetInfo((DWORD*)dwCharSet, &cs, TCI_SRCCHARSET);
    return cs.ciACP;
}

CStringA ConvertMBCS(CStringA str, DWORD SrcCharSet, DWORD DstCharSet)
{
    WCHAR* utf16 = DEBUG_NEW WCHAR[str.GetLength() + 1];
    memset(utf16, 0, (str.GetLength() + 1)*sizeof(WCHAR));

    CHAR* mbcs = DEBUG_NEW CHAR[str.GetLength() * 6 + 1];
    memset(mbcs, 0, str.GetLength() * 6 + 1);

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
              NULL,
              NULL);

    str = mbcs;

    delete [] utf16;
    delete [] mbcs;

    return str;
}

CStringA UrlEncode(CStringA str_in, bool fArg)
{
    CStringA str_out;

    for (int i = 0; i < str_in.GetLength(); i++) {
        char c = str_in[i];
        if (fArg && (c == '#' || c == '?' || c == '%' || c == '&' || c == '=')) {
            str_out.AppendFormat("%%%02x", (BYTE)c);
        } else if (c > 0x20 && c < 0x7f) {
            str_out += c;
        } else {
            str_out.AppendFormat("%%%02x", (BYTE)c);
        }
    }

    return str_out;
}

CStringA UrlDecode(CStringA str_in)
{
    CStringA str_out;

    for (int i = 0, len = str_in.GetLength(); i < len; i++) {
        if (str_in[i] == '%' && i + 2 < len) {
            bool b = true;
            char c1 = str_in[i + 1];
            if (c1 >= '0' && c1 <= '9') {
                c1 -= '0';
            } else if (c1 >= 'A' && c1 <= 'F') {
                c1 -= 'A' - 10;
            } else if (c1 >= 'a' && c1 <= 'f') {
                c1 -= 'a' - 10;
            } else {
                b = false;
            }
            if (b) {
                char c2 = str_in[i + 2];
                if (c2 >= '0' && c2 <= '9') {
                    c2 -= '0';
                } else if (c2 >= 'A' && c2 <= 'F') {
                    c2 -= 'A' - 10;
                } else if (c2 >= 'a' && c2 <= 'f') {
                    c2 -= 'a' - 10;
                } else {
                    b = false;
                }
                if (b) {
                    str_out += (char)((c1 << 4) | c2);
                    i += 2;
                    continue;
                }
            }
        }
        str_out += str_in[i];
    }

    return str_out;
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
        tag = i < tag.GetLength() ? tag.Mid(i) : _T("");
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
        tag = i + 1 < tag.GetLength() ? tag.Mid(i + 1) : _T("");
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

    int nChars = GetNumberFormat(LOCALE_USER_DEFAULT, 0, szNumber, NULL, NULL, 0);
    GetNumberFormat(LOCALE_USER_DEFAULT, 0, szNumber, NULL, ret.GetBuffer(nChars), nChars);
    ret.ReleaseBuffer();

    if (bNoFractionalDigits) {
        TCHAR szNumberFractionalDigits[2] = {0};
        GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IDIGITS, szNumberFractionalDigits, _countof(szNumberFractionalDigits));
        int nNumberFractionalDigits = _tcstol(szNumberFractionalDigits, NULL, 10);
        if (nNumberFractionalDigits) {
            ret.Truncate(ret.GetLength() - nNumberFractionalDigits - 1);
        }
    }

    return ret;
}
