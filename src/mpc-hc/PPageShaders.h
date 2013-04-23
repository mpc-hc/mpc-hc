/*
 * (C) 2013 see Authors.txt
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

#include "PPageBase.h"

class CShaderListBox : public CListBox
{
public:
    CShaderListBox();
    int AddShader(const CAppSettings::Shader& shader);
    int InsertShader(int nIndex, const CAppSettings::Shader& shader);
    int UpShader(UINT nIndex);
    int DownShader(UINT nIndex);
    int DeleteShader(UINT nIndex);
    virtual void ResetContent();
    void SetList(const CAppSettings::ShaderList& list);
    const CAppSettings::ShaderList& GetList();

    bool UpCurrentShader();
    bool DownCurrentShader();
    bool DeleteCurrentShader();

private:
    // These methods should not be used
    virtual int AddString(LPCTSTR lpszItem) { ASSERT(FALSE); return LB_ERR; };
    virtual int DeleteString(UINT nIndex) { ASSERT(FALSE); return LB_ERR; };
    virtual int InsertString(int nIndex, LPCTSTR lpszItem) { ASSERT(FALSE); return LB_ERR; };
    virtual int Dir(UINT attr, LPCTSTR lpszWildCard) { ASSERT(FALSE); return LB_ERR; };

protected:
    CAppSettings::ShaderList m_List;
    CString GetTitle(const CAppSettings::Shader& shader);
    virtual void PreSubclassWindow();

    virtual INT_PTR OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
    BOOL OnToolTipNotify(UINT id, NMHDR* pNMHDR, LRESULT* pResult);

    DECLARE_MESSAGE_MAP();
};

class CPPageShaders : public CPPageBase
{
public:
    CPPageShaders();

protected:
    CShaderListBox m_Shaders, m_PreResize, m_PostResize;
    CComboBox m_Presets;

    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    virtual BOOL OnApply();

    void UpdateState();

    void OnLoadShaderPreset();
    void OnSaveShaderPreset();
    void OnDeleteShaderPreset();
    void OnChangeShaderPresetText();

    void OnAddToPreResize();
    void OnAddToPostResize();
    void OnUpPreResize();
    void OnDownPreResize();
    void OnRemovePreResize();
    void OnUpPostResize();
    void OnDownPostResize();
    void OnRemovePostResize();
    void OnAddShaderFile();
    void OnRemoveShader();

    DECLARE_MESSAGE_MAP()
};
