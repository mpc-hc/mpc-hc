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

#pragma once

#include <afxole.h>


class CDropTarget
{
public:
	CDropTarget() {}

	virtual DROPEFFECT OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
	{
		return DROPEFFECT_NONE;
	}
	virtual DROPEFFECT OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
	{
		return DROPEFFECT_NONE;
	}
	virtual BOOL OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point)
	{
		return FALSE;
	}
	virtual DROPEFFECT OnDropEx(COleDataObject* pDataObject, DROPEFFECT dropDefault, DROPEFFECT dropList, CPoint point)
	{
		return (DROPEFFECT)-1;
	}
	virtual void OnDragLeave() {}
	virtual DROPEFFECT OnDragScroll(DWORD dwKeyState, CPoint point)
	{
		return DROPEFFECT_NONE;
	}
};

// CFileDropTarget command target

class CFileDropTarget : public COleDropTarget
{
//	DECLARE_DYNAMIC(CFileDropTarget)

private:
	CDropTarget* m_pDropTarget;

public:
	CFileDropTarget(CDropTarget* pDropTarget);
	virtual ~CFileDropTarget();

protected:
	DECLARE_MESSAGE_MAP()

	DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	DROPEFFECT OnDropEx(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropDefault, DROPEFFECT dropList, CPoint point);
	void OnDragLeave(CWnd* pWnd);
	DROPEFFECT OnDragScroll(CWnd* pWnd, DWORD dwKeyState, CPoint point);
};
