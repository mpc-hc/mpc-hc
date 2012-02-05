/*
 * $Id$
 *
 * (C) 2006-2011 see AUTHORS
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

#if defined(_DEBUG) && defined(DXVA_LOGFILE_B)

#define LOG_FILE				_T("dxva.log")

void LOG(LPCTSTR fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	if (TCHAR* buff = new TCHAR[_vsctprintf(fmt, args) + 1]) {
		_vstprintf(buff, fmt, args);
		if (FILE* f = _tfopen(LOG_FILE, _T("at"))) {
			fseek(f, 0, 2);
			_ftprintf(f, _T("%s\n"), buff);
			fclose(f);
		}
		delete [] buff;
	}
	va_end(args);
}

#endif
