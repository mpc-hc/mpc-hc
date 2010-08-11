/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2010 see AUTHORS
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "stdafx.h"
#include "DirectVobSubFilter.h"
#include "TextInputPin.h"
#include "../../../DSUtil/DSUtil.h"

CTextInputPin::CTextInputPin(CDirectVobSubFilter* pFilter, CCritSec* pLock, CCritSec* pSubLock, HRESULT* phr)
	: CSubtitleInputPin(pFilter, pLock, pSubLock, phr)
	, m_pDVS(pFilter)
{
}

void CTextInputPin::AddSubStream(ISubStream* pSubStream)
{
	m_pDVS->AddSubStream(pSubStream);
}

void CTextInputPin::RemoveSubStream(ISubStream* pSubStream)
{
	m_pDVS->RemoveSubStream(pSubStream);
}

void CTextInputPin::InvalidateSubtitle(REFERENCE_TIME rtStart, ISubStream* pSubStream)
{
	m_pDVS->InvalidateSubtitle(rtStart, (DWORD_PTR)(ISubStream*)pSubStream);
}
