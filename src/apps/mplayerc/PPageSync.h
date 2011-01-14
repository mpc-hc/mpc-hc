/*
 * (C) 2006-2007 see AUTHORS
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

#include "PPageBase.h"
#include "StaticLink.h"

class CPPageSync: public CPPageBase
{
	DECLARE_DYNAMIC(CPPageSync)

public:
	CPPageSync();
	virtual ~CPPageSync();

	enum {IDD = IDD_PPAGESYNC};
	BOOL m_bSynchronizeVideo;
	BOOL m_bSynchronizeDisplay;
	BOOL m_bSynchronizeNearest;

	int m_iLineDelta;
	int m_iColumnDelta;
	double m_fCycleDelta;

	double m_fTargetSyncOffset;
	double m_fControlLimit;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();
	virtual BOOL OnApply();

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedSyncVideo();
	afx_msg void OnBnClickedSyncDisplay();
	afx_msg void OnBnClickedSyncNearest();

private:
	void InitDialogPrivate();
};
