/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013, 2017 see Authors.txt
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

interface __declspec(uuid("EE6F2741-7DB4-4AAD-A3CB-545208EE4C0A"))
    IBaseMuxerRelatedPin :
    public IUnknown
{
    STDMETHOD(SetRelatedPin)(CBasePin* pPin) PURE;
    STDMETHOD_(CBasePin*, GetRelatedPin)() PURE;
};

class CBaseMuxerRelatedPin : public IBaseMuxerRelatedPin
{
    CBasePin* m_pRelatedPin; // should not hold a reference because it would be circular

public:
    CBaseMuxerRelatedPin();
    virtual ~CBaseMuxerRelatedPin();

    // IBaseMuxerRelatedPin

    STDMETHODIMP SetRelatedPin(CBasePin* pPin);
    STDMETHODIMP_(CBasePin*) GetRelatedPin();
};
