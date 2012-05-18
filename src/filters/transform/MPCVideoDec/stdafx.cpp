/*
 * $Id$
 *
 * (C) 2006-2012 see Authors.txt
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

//#define DXVA_LOGFILE_B

#if defined(_DEBUG) && defined(DXVA_LOGFILE_B)

#define LOG_FILE _T("dxva.log")

void LOG(LPCTSTR fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	if (TCHAR* buff = DNew TCHAR[_vsctprintf(fmt, args) + 1]) {
		_vstprintf(buff, fmt, args);
		if (FILE* f = _tfopen(LOG_FILE, _T("at"))) {
			fseek(f, 0, 2);
			_ftprintf_s(f, _T("%s\n"), buff);
			fclose(f);
		}
		delete [] buff;
	}
	va_end(args);
}

#endif
