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

#include "ShaderEditorDlg.h"
#include <sizecbar/scbarg.h>

#ifndef baseCPlayerShaderEditorBar
#define baseCPlayerShaderEditorBar CSizingControlBarG
#endif


// CPlayerShaderEditorBar

class CPlayerShaderEditorBar : public baseCPlayerShaderEditorBar
{
	DECLARE_DYNAMIC(CPlayerShaderEditorBar)

public:
	CPlayerShaderEditorBar();
	virtual ~CPlayerShaderEditorBar();

	BOOL Create(CWnd* pParentWnd);

public:
	CShaderEditorDlg m_dlg;

protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
