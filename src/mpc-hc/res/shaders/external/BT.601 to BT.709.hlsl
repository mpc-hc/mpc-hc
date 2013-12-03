/*
 * (C) 2011 Jan-Willem Krans (janwillem32 <at> hotmail.com)
 * (C) 2011-2013 see Authors.txt
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

// Correct video colorspace BT.601 [SD] to BT.709 [HD] for HD video input
// Use this shader only if BT.709 [HD] encoded video is incorrectly matrixed to full range RGB with the BT.601 [SD] colorspace.

sampler s0;
float2  c0;

float4 main(float2 tex : TEXCOORD0) : COLOR
{
	float4 si = tex2D(s0, tex); // original pixel
	if (c0.x < 1120 && c0.y < 630) {
		return si; // this shader does not alter SD video
	}
	float3 s1 = si.rgb;
	s1 = s1.rrr * float3(0.299, -0.1495 / 0.886, 0.5) + s1.ggg * float3(0.587, -0.2935 / 0.886, -0.2935 / 0.701) + s1.bbb * float3(0.114, 0.5, -0.057 / 0.701); // RGB to Y'CbCr, BT.601 [SD] colorspace
	return (s1.rrr + float3(0, -0.1674679 / 0.894, 1.8556) * s1.ggg + float3(1.5748, -0.4185031 / 0.894, 0) * s1.bbb).rgbb; // Y'CbCr to RGB output, BT.709 [HD] colorspace
}
