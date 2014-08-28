/*
* (C) 2006-2014 see Authors.txt
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

#include "stdafx.h"
#include "ShaderIncludeHandler.h"
#include <fstream>

CShaderIncludeHandler::CShaderIncludeHandler():
m_SourceDirectory(),
m_SystemDirectory()
{
}

CShaderIncludeHandler::CShaderIncludeHandler(LPCTSTR pSystemDirectory, LPCTSTR pSourceFilename) :
m_SourceDirectory(),
m_SystemDirectory(pSystemDirectory)
{
	CPath cp(pSourceFilename);
	cp.RemoveBackslash();
	cp.RemoveFileSpec();
	m_SourceDirectory = static_cast<CString>(cp);
}

CShaderIncludeHandler::~CShaderIncludeHandler()
{
}

HRESULT CShaderIncludeHandler::Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes)
{
	m_IncludedFiles.clear();

	CA2T lTFilename(pFileName);
	
	// Find which root directory to use
	CPath lIncludeFileName;
	if (IncludeType == D3D10_INCLUDE_LOCAL)
		lIncludeFileName = CPath(m_SourceDirectory);
	else
		lIncludeFileName = CPath(m_SystemDirectory);

	lIncludeFileName.Append(CPath(lTFilename));

	// Open file or generate warning message (pragma)
	CString lCIncludeFileName = lIncludeFileName;
	std::ifstream lFile(lCIncludeFileName, std::ios_base::in);
	if (lFile.is_open() == true)
	{
		// read full file
		lFile.seekg(0, std::ios_base::end);
		std::size_t lSize = lFile.tellg();
		lFile.seekg(0, std::ios_base::beg);

		*pBytes = (UINT)lSize;
		char* lContent = new char[*pBytes+1];
		lContent[*pBytes] = 0x00;
		*ppData = (LPCVOID*)lContent;

		lFile.read(lContent, *pBytes);

		// add the include path in the list.
		// the file change notifier needs to have backslashes.
		lCIncludeFileName.Replace('/', '\\');
		m_IncludedFiles.push_back(CPath(lCIncludeFileName));
	}
	else
	{
		// Création d'un warning
		CString lCIncludeFilename = lIncludeFileName;
		lCIncludeFilename.Replace(_T("\\"), _T("\\\\"));
		
		std::string lWarning = "#pragma message(\"/* warning: #include file not found '" + std::string(CT2A(lCIncludeFilename)) + "' */\")";

		UINT lSize = (UINT)lWarning.size();
		*pBytes = lSize;
		char *lContent = new char[lSize + 1];
		std::copy(lWarning.begin(), lWarning.end(), lContent);
		lContent[lSize] = 0x00;

		*ppData = (LPCVOID*)lContent;
	}

	return S_OK;
}

HRESULT CShaderIncludeHandler::Close(LPCVOID pData)
{
	delete[] (char*) pData;
	return S_OK;
}

const std::vector<CString>& CShaderIncludeHandler::getIncludedFiles() const
{
	return m_IncludedFiles;
}