/* 
 *	Copyright (C) 2003-2006 Gabest
 *	http://www.gabest.org
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

#pragma once
#include "PPageBase.h"
#include "floatedit.h"
#include "mplayerc.h"

// CPPageExternalFilters dialog

class CPPageExternalFilters : public CPPageBase
{
	DECLARE_DYNAMIC(CPPageExternalFilters)

private:
	void StepUp(CCheckListBox& list);
	void StepDown(CCheckListBox& list);

	CAutoPtrList<FilterOverride> m_pFilters;
	FilterOverride* m_pLastSelFilter;
	FilterOverride* GetCurFilter();

	void SetupMajorTypes(CAtlArray<GUID>& guids);
	void SetupSubTypes(CAtlArray<GUID>& guids);

public:
	CPPageExternalFilters();
	virtual ~CPPageExternalFilters();

// Dialog Data
	enum { IDD = IDD_PPAGEEXTERNALFILTERS };

	CCheckListBox m_filters;
	int m_iLoadType;
	CHexEdit m_dwMerit;
	CTreeCtrl m_tree;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnUpdateFilter(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFilterUp(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFilterDown(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFilterMerit(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSubType(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDeleteType(CCmdUI* pCmdUI);
	afx_msg void OnAddRegistered();
	afx_msg void OnRemoveFilter();
	afx_msg void OnMoveFilterUp();
	afx_msg void OnMoveFilterDown();
	afx_msg void OnLbnDblclkFilter();
	afx_msg void OnAddMajorType();
	afx_msg void OnAddSubType();
	afx_msg void OnDeleteType();
	afx_msg void OnResetTypes();
	afx_msg void OnLbnSelchangeList1();
	afx_msg void OnBnClickedRadio();
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnNMDblclkTree2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDropFiles(HDROP hDropInfo);
};
