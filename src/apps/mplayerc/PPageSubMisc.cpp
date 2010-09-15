/*
 * $Id$
 *
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
#include "PPageSubMisc.h"


// CPPageSubMisc dialog

IMPLEMENT_DYNAMIC(CPPageSubMisc, CPPageBase)

CPPageSubMisc::CPPageSubMisc()
	: CPPageBase(CPPageSubMisc::IDD, CPPageSubMisc::IDD)
	, m_fPrioritizeExternalSubtitles(FALSE)
	, m_fDisableInternalSubtitles(FALSE)
	, m_szAutoloadPaths("")
{

}

CPPageSubMisc::~CPPageSubMisc()
{
}

void CPPageSubMisc::DoDataExchange(CDataExchange* pDX)
{
	CPPageBase::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK1, m_fPrioritizeExternalSubtitles);
	DDX_Check(pDX, IDC_CHECK2, m_fDisableInternalSubtitles);
	DDX_Text(pDX, IDC_EDIT1, m_szAutoloadPaths);
}

BOOL CPPageSubMisc::OnInitDialog()
{
	__super::OnInitDialog();

	AppSettings& s = AfxGetAppSettings();

	m_fPrioritizeExternalSubtitles = s.fPrioritizeExternalSubtitles;
	m_fDisableInternalSubtitles = s.fDisableInternalSubtitles;
	m_szAutoloadPaths = s.szSubtitlePaths;

	UpdateData(FALSE);

	return TRUE;
}

BOOL CPPageSubMisc::OnApply()
{
	UpdateData();

	AppSettings& s = AfxGetAppSettings();

	s.fPrioritizeExternalSubtitles = m_fPrioritizeExternalSubtitles;
	s.fDisableInternalSubtitles = m_fDisableInternalSubtitles;
	s.szSubtitlePaths = m_szAutoloadPaths;

	return __super::OnApply();
}


BEGIN_MESSAGE_MAP(CPPageSubMisc, CPPageBase)
END_MESSAGE_MAP()
