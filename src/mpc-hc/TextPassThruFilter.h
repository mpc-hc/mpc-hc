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

#pragma once

class CTextPassThruInputPin;

class __declspec(uuid("E2BA9B7B-B65D-4804-ACB2-89C3E55511DB"))
    CTextPassThruFilter : public CBaseFilter, public CCritSec
{
    friend class CTextPassThruInputPin;
    friend class CTextPassThruOutputPin;

    CTextPassThruInputPin* m_pInput;
    CTextPassThruOutputPin* m_pOutput;

    CMainFrame* m_pMainFrame;

public:
    CTextPassThruFilter(CMainFrame* pMainFrame);
    virtual ~CTextPassThruFilter();

    DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    int GetPinCount();
    CBasePin* GetPin(int n);
};
