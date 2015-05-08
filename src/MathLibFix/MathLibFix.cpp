/*
 * (C) 2014-2015 see Authors.txt
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

#include <math.h>
#include "MathLibFix.h"


void WorkAroundMathLibraryBug()
{
#if defined(_WIN64) && _MSC_VER == 1800
    // Temporary disable the use of FMA3 to work around a bug affecting the x64 math library
    // See http://connect.microsoft.com/VisualStudio/feedback/details/811093
    // The issue is fixed in Visual Studio 2015, the work-around is to be removed
    // when we stop supporting Visual Studio 2013
    _set_FMA3_enable(0);
#endif
}
