/*
 * $Id$
 *
 * (C) 2003-2005 Gabest
 *
 * This file is part of subresync.
 *
 * Subresync is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Subresync is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#ifndef __AFXWIN_H__
#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols


// CsubresyncApp:
// See subresync.cpp for the implementation of this class
//

class CSubresyncApp : public CWinApp
{
public:
    CSubresyncApp();

// Overrides
public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();

// Implementation

    DECLARE_MESSAGE_MAP()
};

extern CSubresyncApp theApp;