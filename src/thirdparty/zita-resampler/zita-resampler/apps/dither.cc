// ----------------------------------------------------------------------------
//
//  Copyright (C) 2006-2011 Fons Adriaensen <fons@linuxaudio.org>
//    
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// ----------------------------------------------------------------------------


#include <string.h>
#include <math.h>
#include "dither.h"


float Dither::_div = 0;

#define SCALE 32768.0f
#define LIMIT 32767



Dither::Dither (void)
{
    reset ();
    _div = ldexpf (1.0f, 32);
}


void Dither::reset (void)
{
    memset (_err, 0, (SIZE + 4) * sizeof(float));
    _ind = SIZE - 1;
    _ran = 1234567;
}


void Dither::proc_rectangular (const float *srce, int16_t *dest, int step, int nsam)
{
    float    v, r;
    int32_t  k;

    while (nsam--)
    {
	r = genrand () - 0.5f;
        v = *srce * SCALE + r;
	k = lrintf (v);
	if      (k < -LIMIT) k = -LIMIT;
	else if (k >  LIMIT) k =  LIMIT;
        *dest = k;
        srce += step;
        dest += step;
    }
}


void Dither::proc_triangular (const float *srce, int16_t *dest, int step, int nsam)
{
    float    v, r0, r1;
    int32_t  k;

    r1 = *_err;
    while (nsam--)
    {
        r0 = genrand ();
        v = *srce * SCALE + r0 - r1;
	r1 = r0;
	k = lrintf (v);
	if      (k < -LIMIT) k = -LIMIT;
	else if (k >  LIMIT) k =  LIMIT;
        *dest = k;
        srce += step;
        dest += step;
    }
    *_err = r1;
}


void Dither::proc_lipschitz (const float *srce, int16_t *dest, int step, int nsam)
{
    float    e, u, v, *p;
    int      i;
    int32_t  k;

    i = _ind;
    while (nsam--)
    {
	p = _err + i;
        u = *srce * SCALE
	    - 2.033f * p [0]
	    + 2.165f * p [1]
	    - 1.959f * p [2]
	    + 1.590f * p [3]
	    - 0.615f * p [4];
	v = u + genrand () - genrand ();
	k = lrintf (v);
	e = k - u;
	if      (k < -LIMIT) k = -LIMIT;
	else if (k >  LIMIT) k =  LIMIT;
        *dest = k;
	if (--i < 0)
	{
	    _err [SIZE + 0] = _err [0];
	    _err [SIZE + 1] = _err [1];
	    _err [SIZE + 2] = _err [2];
	    _err [SIZE + 3] = _err [3];
	    i += SIZE;
	}
	_err [i] = e;
        srce += step;
        dest += step;
    }
    _ind = i;
}


