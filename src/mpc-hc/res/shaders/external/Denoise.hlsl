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

// Fail early with ps_2_0 and ps_2_b shader profiles
#ifdef MPC_HC_SHADER_PROFILE
#	if MPC_HC_SHADER_PROFILE < 2
#		error Usupported shader profile
#	endif
#endif

sampler s0 : register(s0);
float4 p0 :  register(c0);

#define width  (p0[0])
#define height (p0[1])
#define val0 (1.0)
#define val1 (0.125)
#define effect_width (0.1)

float4 main(float2 tex : TEXCOORD0) : COLOR
{
	float dx = 0.0f;
	float dy = 0.0f;
	float fTap = effect_width;

	float4 cAccum = tex2D(s0, tex) * val0;

	for (int iDx = 0; iDx < 16; ++iDx) {
		dx = fTap / width;
		dy = fTap / height;

		cAccum += tex2D(s0, tex + float2(-dx, -dy)) * val1;
		cAccum += tex2D(s0, tex + float2(  0, -dy)) * val1;
		cAccum += tex2D(s0, tex + float2(-dx,   0)) * val1;
		cAccum += tex2D(s0, tex + float2( dx,   0)) * val1;
		cAccum += tex2D(s0, tex + float2(  0,  dy)) * val1;
		cAccum += tex2D(s0, tex + float2( dx,  dy)) * val1;
		cAccum += tex2D(s0, tex + float2(-dx, +dy)) * val1;
		cAccum += tex2D(s0, tex + float2(+dx, -dy)) * val1;

		fTap  += 0.1f;
	}

	return (cAccum / 16.0f);
}
