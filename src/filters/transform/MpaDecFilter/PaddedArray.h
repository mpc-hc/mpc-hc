/*
 * (C) 2012 see Authors.txt
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

#include <atlcoll.h>

class CPaddedArray : public CAtlArray<BYTE>
{
protected:
    size_t m_padsize;

public:
    CPaddedArray(size_t padsize)
        : m_padsize(padsize)
    {
    }

    size_t GetCount()
    {
        size_t count = __super::GetCount();
        return (count > m_padsize) ? count - m_padsize : 0;
    }

    bool SetCount(size_t nNewSize, int nGrowBy = - 1)
    {
        if (__super::SetCount(nNewSize + m_padsize, nGrowBy)) {
            memset(GetData() + nNewSize, 0, m_padsize);
            return true;
        }
        return false;
    }
};
