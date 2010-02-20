/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2010 see AUTHORS
 *
 * This file is part of mplayerc.
 *
 * Mplayerc is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mplayerc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include "mplayerc.h"
#include "FileDropTarget.h"


// CFileDropTarget

//IMPLEMENT_DYNAMIC(CFileDropTarget, COleDropTarget)
CFileDropTarget::CFileDropTarget(CDropTarget* pDropTarget)
	: m_pDropTarget(pDropTarget)
{
	ASSERT(m_pDropTarget);
}

CFileDropTarget::~CFileDropTarget()
{
}

DROPEFFECT CFileDropTarget::OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	return m_pDropTarget ? m_pDropTarget->OnDragEnter(pDataObject, dwKeyState, point) : DROPEFFECT_NONE;
}

DROPEFFECT CFileDropTarget::OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	return m_pDropTarget ? m_pDropTarget->OnDragOver(pDataObject, dwKeyState, point) : DROPEFFECT_NONE;
}

BOOL CFileDropTarget::OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point)
{
	return m_pDropTarget ? m_pDropTarget->OnDrop(pDataObject, dropEffect, point) : DROPEFFECT_NONE;
}

DROPEFFECT CFileDropTarget::OnDropEx(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropDefault, DROPEFFECT dropList, CPoint point)
{
	return m_pDropTarget ? m_pDropTarget->OnDropEx(pDataObject, dropDefault, dropList, point) : DROPEFFECT_NONE;
}

void CFileDropTarget::OnDragLeave(CWnd* pWnd)
{
	if(m_pDropTarget) m_pDropTarget->OnDragLeave();
}

DROPEFFECT CFileDropTarget::OnDragScroll(CWnd* pWnd, DWORD dwKeyState, CPoint point)
{
	return m_pDropTarget ? m_pDropTarget->OnDragScroll(dwKeyState, point) : DROPEFFECT_NONE;
}

BEGIN_MESSAGE_MAP(CFileDropTarget, COleDropTarget)
END_MESSAGE_MAP()
