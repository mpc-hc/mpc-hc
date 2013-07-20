/*
 * (C) 2008-2013 see Authors.txt
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

#include <_mingw.h>

#define str(s)  xstr(s)
#define xstr(s) #s

#ifdef __MINGW64_VERSION_MAJOR
#define MINGW "MinGW-w64"
#else
#define MINGW "MinGW"
#endif

char g_Gcc_Compiler[] = MINGW " GCC " str(__GNUC__) "." str(__GNUC_MINOR__) "." str(__GNUC_PATCHLEVEL__);
