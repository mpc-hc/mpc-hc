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
	float dx = 1 / width;
	float dy = 1 / height;

	float4 c1 = tex2D(s0, tex + float2(-dx, -dy));
	float4 c2 = tex2D(s0, tex + float2(  0, -dy));
	float4 c4 = tex2D(s0, tex + float2(-dx,   0));
	float4 c6 = tex2D(s0, tex + float2( dx,   0));
	float4 c8 = tex2D(s0, tex + float2(  0,  dy));
	float4 c9 = tex2D(s0, tex + float2( dx,  dy));

	float4 c0 = (-c1 - c2 - c4 + c6 + c8 + c9);
	c0 = (c0.r + c0.g + c0.b) / 3 + 0.5;

	return c0;
}
