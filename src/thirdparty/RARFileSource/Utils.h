/*
 * Copyright (C) 2008-2012, OctaneSnail <os@v12pwr.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UTILS_H
#define UTILS_H

#include <windows.h>

void ErrorMsg (DWORD errorCode, const wchar_t *format, ...);

#ifdef _DEBUG
#include "RAR.h"

void LogHeader (rar_header_t *rh);
#define LOG_HEADER(rh) LogHeader(rh)
#else
#define LOG_HEADER(rh)
#endif

#endif // UTILS_H
