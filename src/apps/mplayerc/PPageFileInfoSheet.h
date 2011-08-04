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

#include "PPageFileInfoClip.h"
#include "PPageFileInfoDetails.h"
#include "PPageFileInfoRes.h"
#include "PPageFileMediaInfo.h"


class CMainFrame;

// CPPageFileInfoSheet

class CPPageFileInfoSheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CPPageFileInfoSheet)

private:
	CPPageFileInfoClip m_clip;
	CPPageFileInfoDetails m_details;
	CPPageFileInfoRes m_res;
	CPPageFileMediaInfo m_mi;

	CButton m_Button_MI;

public:
	CPPageFileInfoSheet(CString fn, CMainFrame* pMainFrame, CWnd* pParentWnd);
	virtual ~CPPageFileInfoSheet();

	afx_msg void OnSaveAs();

	CString m_fn;
protected:
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
};
