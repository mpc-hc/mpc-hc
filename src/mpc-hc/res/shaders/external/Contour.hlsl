/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013 see Authors.txt
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

sampler s0 : register(s0);
float4 p0 :  register(c0);

#define width  (p0[0])
#define height (p0[1])

float4 main(float2 tex : TEXCOORD0) : COLOR
{
	float dx = 4 / width;
	float dy = 4 / height;

	float4 c2 = tex2D(s0, tex + float2(  0, -dy));
	float4 c4 = tex2D(s0, tex + float2(-dx,   0));
	float4 c5 = tex2D(s0, tex + float2(  0,   0));
	float4 c6 = tex2D(s0, tex + float2( dx,   0));
	float4 c8 = tex2D(s0, tex + float2(  0,  dy));

	float4 c0 = (-c2 - c4 + c5 * 4 - c6 - c8);
	if (length(c0) < 1.0) {
		c0 = float4(0, 0, 0, 0);
	} else {
		c0 = float4(1, 1, 1, 0);
	}

	return c0;
}
