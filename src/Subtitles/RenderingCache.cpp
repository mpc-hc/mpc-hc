/*
 * (C) 2013 see Authors.txt
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
#include "RenderingCache.h"
#include "RTS.h"

template<typename T>
bool NEARLY_EQ(T a, T b, T tol)
{
    return (abs(a - b) < tol);
}

COutlineKey::COutlineKey(const CWord* word, CPoint org)
    : m_str(word->m_str)
    , m_scalex(word->m_scalex)
    , m_scaley(word->m_scaley)
    , m_style(DEBUG_NEW STSStyle(word->m_style))
    , m_org(org)
{
    UpdateHash();
};

COutlineKey::COutlineKey(const COutlineKey& outLineKey)
    : m_str(outLineKey.m_str)
    , m_scalex(outLineKey.m_scalex)
    , m_scaley(outLineKey.m_scaley)
    , m_style(DEBUG_NEW STSStyle(*outLineKey.m_style))
    , m_org(outLineKey.m_org)
    , m_hash(outLineKey.m_hash)
{
};


void COutlineKey::UpdateHash()
{
    // CreatePath
    m_hash  = CStringElementTraits<CString>::Hash(m_str);
    m_hash += m_hash << 5;
    m_hash += int(m_scalex);
    m_hash += m_hash << 5;
    m_hash += m_style->charSet;
    m_hash += m_hash << 5;
    m_hash += CStringElementTraits<CString>::Hash(m_style->fontName);
    m_hash += m_hash << 5;
    m_hash += int(m_style->fontSize);
    m_hash += m_hash << 5;
    m_hash += int(m_style->fontSpacing);
    m_hash += m_hash << 5;
    m_hash += m_style->fontWeight;
    m_hash += m_hash << 5;
    m_hash += m_style->fItalic;
    m_hash += m_hash << 5;
    m_hash += m_style->fUnderline;
    m_hash += m_hash << 5;
    m_hash += m_style->fStrikeOut;
    m_hash += m_hash << 5;
    // Transform
    m_hash += int(m_style->fontScaleX);
    m_hash += m_hash << 5;
    m_hash += int(m_style->fontScaleY);
    m_hash += m_hash << 5;
    m_hash += int(m_style->fontAngleX);
    m_hash += m_hash << 5;
    m_hash += int(m_style->fontAngleY);
    m_hash += m_hash << 5;
    m_hash += int(m_style->fontAngleZ);
    m_hash += m_hash << 5;
    m_hash += int(m_style->fontShiftX);
    m_hash += m_hash << 5;
    m_hash += int(m_style->fontShiftY);
    m_hash += m_hash << 5;
    m_hash += m_org.x + (m_org.y << 16);
    m_hash += m_hash << 5;
    // CreateWidenedRegion
    m_hash += m_style->borderStyle;
    m_hash += m_hash << 5;
    m_hash += int(m_style->outlineWidthX);
    m_hash += m_hash << 5;
    m_hash += int(m_style->outlineWidthY);
}

bool COutlineKey::operator==(const COutlineKey& outLineKey) const
{
    return m_str == outLineKey.m_str // CreatePath
            && NEARLY_EQ(m_scalex, outLineKey.m_scalex, 1e-6)
            && m_style->charSet == outLineKey.m_style->charSet
            && m_style->fontName == outLineKey.m_style->fontName
            && NEARLY_EQ(m_style->fontSize, outLineKey.m_style->fontSize, 1e-6)
            && NEARLY_EQ(m_style->fontSpacing, outLineKey.m_style->fontSpacing, 1e-6)
            && m_style->fontWeight == outLineKey.m_style->fontWeight
            && m_style->fItalic == outLineKey.m_style->fItalic
            && m_style->fUnderline == outLineKey.m_style->fUnderline
            && m_style->fStrikeOut == outLineKey.m_style->fStrikeOut
            // Transform
            && NEARLY_EQ(m_style->fontScaleX, outLineKey.m_style->fontScaleX, 1e-6)
            && NEARLY_EQ(m_style->fontScaleY, outLineKey.m_style->fontScaleY, 1e-6)
            && NEARLY_EQ(m_style->fontAngleX, outLineKey.m_style->fontAngleX, 1e-6)
            && NEARLY_EQ(m_style->fontAngleY, outLineKey.m_style->fontAngleY, 1e-6)
            && NEARLY_EQ(m_style->fontAngleZ, outLineKey.m_style->fontAngleZ, 1e-6)
            && NEARLY_EQ(m_style->fontShiftX, outLineKey.m_style->fontShiftX, 1e-6)
            && NEARLY_EQ(m_style->fontShiftY, outLineKey.m_style->fontShiftY, 1e-6)
            && m_org == outLineKey.m_org
            // CreateWidenedRegion
            && m_style->borderStyle == outLineKey.m_style->borderStyle
            && NEARLY_EQ(m_style->outlineWidthX, outLineKey.m_style->outlineWidthX, 1e-6)
            && NEARLY_EQ(m_style->outlineWidthY, outLineKey.m_style->outlineWidthY, 1e-6);
}
