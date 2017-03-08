/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2014, 2016 see Authors.txt
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
#include "CCDecoder.h"
#include "SubtitleHelpers.h"

CCDecoder::CCDecoder(CString fn, CString rawfn) : m_fn(fn), m_rawfn(rawfn)
{
    m_sts.CreateDefaultStyle(ANSI_CHARSET);

    m_time = 0;
    m_fEndOfCaption = false;
    ZeroMemory(m_buff, sizeof(m_buff));
    ZeroMemory(m_disp, sizeof(m_disp));
    m_cursor = CPoint(0, 0);

    if (!m_rawfn.IsEmpty()) {
        _tremove(m_rawfn);
    }
}

CCDecoder::~CCDecoder()
{
    if (!m_sts.IsEmpty() && !m_fn.IsEmpty()) {
        m_sts.Sort();
        m_sts.SaveAs(m_fn, Subtitle::SRT, -1, 0, CTextFile::DEFAULT_ENCODING);
        m_sts.SaveAs(m_fn.Left(m_fn.ReverseFind('.') + 1) + _T("utf8.srt"), Subtitle::SRT, -1, 0, CTextFile::UTF8);
        m_sts.SaveAs(m_fn.Left(m_fn.ReverseFind('.') + 1) + _T("utf16le.srt"), Subtitle::SRT, -1, 0, CTextFile::LE16);
        m_sts.SaveAs(m_fn.Left(m_fn.ReverseFind('.') + 1) + _T("utf16be.srt"), Subtitle::SRT, -1, 0, CTextFile::BE16);
    }
}

void CCDecoder::MoveCursor(int x, int y)
{
    m_cursor = CPoint(x, y);
    if (m_cursor.x < 0) {
        m_cursor.x = 0;
    }
    if (m_cursor.y < 0) {
        m_cursor.y = 0;
    }
    if (m_cursor.x >= 32) {
        m_cursor.x = 0, m_cursor.y++;
    }
    if (m_cursor.y >= 16) {
        m_cursor.y = 0;
    }
}

void CCDecoder::OffsetCursor(int x, int y)
{
    MoveCursor(m_cursor.x + x, m_cursor.y + y);
}

void CCDecoder::PutChar(WCHAR c)
{
    m_buff[m_cursor.y][m_cursor.x] = c;
    OffsetCursor(1, 0);
}

void CCDecoder::SaveDisp(__int64 time)
{
    CStringW str;

    for (size_t row = 0; row < 16; row++) {
        bool fNonEmptyRow = false;

        for (size_t col = 0; col < 32; col++) {
            if (m_disp[row][col]) {
                CStringW str2(&m_disp[row][col]);
                if (fNonEmptyRow) {
                    str += L' ';
                }
                str += str2;
                col += str2.GetLength();
                fNonEmptyRow = true;
            }
        }

        if (fNonEmptyRow) {
            str += L'\n';
        }
    }

    if (str.IsEmpty()) {
        return;
    }

    m_sts.Add(str, true, (int)m_time, (int)time);
}

void CCDecoder::DecodeCC(const BYTE* buff, int len, __int64 time)
{
    if (!m_rawfn.IsEmpty()) {
        FILE* f = nullptr;
        if (!_tfopen_s(&f, m_rawfn, _T("at"))) {
            _ftprintf_s(f, _T("%02d:%02d:%02d.%03d\n"),
                        (int)(time / 1000 / 60 / 60),
                        (int)((time / 1000 / 60) % 60),
                        (int)((time / 1000) % 60),
                        (int)(time % 1000));

            for (ptrdiff_t i = 0; i < len; i++) {
                _ftprintf_s(f, _T("%02x"), buff[i]);
                if (i < len - 1) {
                    _ftprintf_s(f, _T(" "));
                }
                if (i > 0 && (i & 15) == 15) {
                    _ftprintf_s(f, _T("\n"));
                }
            }
            if (len > 0) {
                _ftprintf_s(f, _T("\n\n"));
            }
            fclose(f);
        }
    }

    for (ptrdiff_t i = 0; i < len; i++) {
        BYTE c = buff[i] & 0x7f;
        if (c >= 0x20) {
            static WCHAR charmap[0x60] = {
                ' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', 0xE1, '+', ',', '-', '.', '/',
                '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', 0x3F,
                '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
                'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', 0xE9, ']', 0xED, 0xF3,
                0xFA, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
                'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 0xE7, 0xF7, 'N', 'n', 0x3F
            };

            PutChar(charmap[c - 0x20]);
        } else if (i < len - 1 && buff[i] != 0x80) {
            // codes and special characters are supposed to be doubled
            if (i < len - 3 && buff[i] == buff[i + 2] && buff[i + 1] == buff[i + 3]) {
                i += 2;
            }

            c = buff[i + 1] & 0x7f;
            if (buff[i] == 0x91 && c >= 0x20 && c < 0x30) { // formating
                // TODO
            } else if (buff[i] == 0x91 && c == 0x39) { // transparent space
                OffsetCursor(1, 0);
            } else if (buff[i] == 0x91 && c >= 0x30 && c < 0x40) { // special characters
                static WCHAR charmap[0x10] = {
                    0x00ae, // (r)egistered
                    0x00b0, // degree
                    0x00bd, // 1/2
                    0x00bf, // inverted question mark
                    0x2122, // trade mark
                    0x00a2, // cent
                    0x00a3, // pound
                    0x266a, // music
                    0x00e0, // a`
                    0x00ff, // transparent space, handled above
                    0x00e8, // e`
                    0x00e2, // a^
                    0x00ea, // e^
                    0x00ee, // i^
                    0x00f4, // o^
                    0x00fb, // u^
                };

                PutChar(charmap[c - 0x30]);
            } else if (buff[i] == 0x92 && c >= 0x20 && c < 0x40) { // extended characters
                static WCHAR charmap[0x20] = {
                    0x00c0, // A'
                    0x00c9, // E'
                    0x00d3, // O'
                    0x00da, // U'
                    0x00dc, // U:
                    0x00fc, // u:
                    0x2018, // `
                    0x00a1, // inverted !
                    0x002a, // *
                    0x2019, // '
                    0x002d, // -
                    0x00a9, // (c)opyright
                    0x2120, // SM
                    0x00b7, // . (dot in the middle)
                    0x201c, // inverted "
                    0x201d, // "

                    0x00c1, // A`
                    0x00c2, // A^
                    0x00c7, // C,
                    0x00c8, // E`
                    0x00ca, // E^
                    0x00cb, // E:
                    0x00eb, // e:
                    0x00ce, // I^
                    0x00cf, // I:
                    0x00ef, // i:
                    0x00d4, // O^
                    0x00d9, // U`
                    0x00f9, // u`
                    0x00db, // U^
                    0x00ab, // <<
                    0x00bb, // >>
                };

                PutChar(charmap[c - 0x20]);
            } else if (buff[i] == 0x13 && c >= 0x20 && c < 0x40) { // more extended characters
                static WCHAR charmap[0x20] = {
                    0x00c3, // A~
                    0x00e3, // a~
                    0x00cd, // I'
                    0x00cc, // I`
                    0x00ec, // i`
                    0x00d2, // O`
                    0x00f2, // o`
                    0x00d5, // O~
                    0x00f5, // o~
                    0x007b, // {
                    0x007d, // }
                    0x005c, // /* \ */
                    0x005e, // ^
                    0x005f, // _
                    0x00a6, // |
                    0x007e, // ~

                    0x00c4, // A:
                    0x00e4, // a:
                    0x00d6, // O:
                    0x00f6, // o:
                    0x00df, // B (ss in german)
                    0x00a5, // Y=
                    0x00a4, // ox
                    0x007c, // |
                    0x00c5, // Ao
                    0x00e5, // ao
                    0x00d8, // O/
                    0x00f8, // o/
                    0x250c, // |-
                    0x2510, // -|
                    0x2514, // |_
                    0x2518, // _|
                };

                PutChar(charmap[c - 0x20]);
            } else if (buff[i] == 0x94 && buff[i + 1] == 0xae) { // Erase Non-displayed [buffer] Memory
                ZeroMemory(m_buff, sizeof(m_buff));
            } else if (buff[i] == 0x94 && buff[i + 1] == 0x20) { // Resume Caption Loading
                ZeroMemory(m_buff, sizeof(m_buff));
            } else if (buff[i] == 0x94 && buff[i + 1] == 0x2f) { // End Of Caption
                if (memcmp(m_disp, m_buff, sizeof(m_disp)) != 0) {
                    if (m_fEndOfCaption) {
                        SaveDisp(time + (i / 2) * 1000 / 30);
                    }

                    m_fEndOfCaption = true;
                    memcpy(m_disp, m_buff, sizeof(m_disp));
                    m_time = time + (i / 2) * 1000 / 30;
                }
            } else if (buff[i] == 0x94 && buff[i + 1] == 0x2c) { // Erase Displayed Memory
                if (m_fEndOfCaption) {
                    m_fEndOfCaption = false;
                    SaveDisp(time + (i / 2) * 1000 / 30);
                }

                ZeroMemory(m_disp, sizeof(m_disp));
            } else if (buff[i] == 0x97 && (buff[i + 1] == 0xa1 || buff[i + 1] == 0xa2 || buff[i + 1] == 0x23)) { // Tab Over
                OffsetCursor(buff[i + 1] & 3, 0);
            } else if (buff[i] == 0x91 || buff[i] == 0x92 || buff[i] == 0x15 || buff[i] == 0x16
                       || buff[i] == 0x97 || buff[i] == 0x10 || buff[i] == 0x13 || buff[i] == 0x94) { // curpos, color, underline
                int row = 0;
                switch (buff[i]) {
                    default:
                    case 0x91:
                        row = 0;
                        break;
                    case 0x92:
                        row = 2;
                        break;
                    case 0x15:
                        row = 4;
                        break;
                    case 0x16:
                        row = 6;
                        break;
                    case 0x97:
                        row = 8;
                        break;
                    case 0x10:
                        row = 10;
                        break;
                    case 0x13:
                        row = 12;
                        break;
                    case 0x94:
                        row = 14;
                        break;
                }
                if (buff[i + 1] & 0x20) {
                    row++;
                }

                int col = buff[i + 1] & 0xe;
                if (col == 0 || (col > 0 && !(buff[i + 1] & 0x10))) {
                    col = 0;
                } else {
                    col <<= 1;
                }

                MoveCursor(col, row);
            }

            i++;
        }
    }
}

void CCDecoder::ExtractCC(BYTE* buff, int len, __int64 time)
{
    for (ptrdiff_t i = 0; i < len - 9; i++) {
        if (*(DWORD*)&buff[i] == 0xb2010000 && *(DWORD*)&buff[i + 4] == 0xf8014343) {
            i += 8;
            int nBytes = buff[i++] & 0x3f;
            if (nBytes > 0) {
                nBytes = (nBytes + 1) & ~1;

                BYTE* pData1 = DEBUG_NEW BYTE[nBytes];
                BYTE* pData2 = DEBUG_NEW BYTE[nBytes];

                if (pData1 && pData2) {
                    int nBytes1 = 0, nBytes2 = 0;

                    for (ptrdiff_t j = 0; j < nBytes && i < 0x800;) {
                        if (buff[i++] == 0xff) {
                            pData1[nBytes1++] = buff[i++];
                            pData1[nBytes1++] = buff[i++];
                        } else {
                            i += 2;
                        }

                        j++;

                        if (j >= nBytes) {
                            break;
                        }

                        if (buff[i++] == 0xff) {
                            pData2[nBytes2++] = buff[i++];
                            pData2[nBytes2++] = buff[i++];
                        } else {
                            i += 2;
                        }

                        j++;
                    }

                    if (nBytes1 > 0) {
                        DecodeCC(pData1, nBytes1, time);
                    }

                    if (nBytes2 > 0) {
                        DecodeCC(pData2, nBytes2, time);
                    }
                }

                if (pData1) {
                    delete [] pData1;
                }
                if (pData2) {
                    delete [] pData2;
                }
            }

            break;
        }
    }
}
