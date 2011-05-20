/*
 * $Id$
 *
 * (C) 2003-2005 Gabest
 * (C) 2006-2011 see AUTHORS
 *
 * This file is part of asf2mkv.
 *
 * Asf2mkv is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Asf2mkv is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include "../../../include/stdafx_common.h"
#include "../../../include/stdafx_common_afx2.h"
#include "../../../include/stdafx_common_dshow.h"

#include "../../CmdUI/CmdUI.h"
#include "../../thirdparty/ui/ResizableLib/ResizableDialog.h"
#include "../../thirdparty/ui/ResizableLib/ResizablePage.h"
#include "../../thirdparty/ui/ResizableLib/ResizableSheet.h"
#include "../../thirdparty/ui/sizecbar/sizecbar.h"
#include "../../thirdparty/ui/sizecbar/scbarcf.h"
#include "../../thirdparty/ui/sizecbar/scbarg.h"
#include "../../DSUtil/DSUtil.h"
#include "../../DSUtil/SharedInclude.h"
#include "../../filters/Filters.h"

#define ResStr(id) CString(MAKEINTRESOURCE(id))
