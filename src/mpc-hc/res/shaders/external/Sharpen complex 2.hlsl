/*
 * (C) 2009-2013 see Authors.txt
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
float4 p1 :  register(c1);

#define width  (p0[0])
#define height (p0[1])

// pixel "width"
#define px (p1[0])
#define py (p1[1])

// Parameters

// for the blur filter
#define mean 0.6
#define dx (mean * px)
#define dy (mean * py)

#define CoefBlur 2
#define CoefOrig (1 + CoefBlur)

// for the sharpen filter
#define SharpenEdge  0.2
#define Sharpen_val0 2
#define Sharpen_val1 ((Sharpen_val0 - 1) / 8.0)

float4 main(float2 tex : TEXCOORD0) : COLOR
{
	// get original pixel
	float4 orig = tex2D(s0, tex);

	// compute blurred image (gaussian filter)
	float4 c1 = tex2D(s0, tex + float2(-dx, -dy));
	float4 c2 = tex2D(s0, tex + float2(  0, -dy));
	float4 c3 = tex2D(s0, tex + float2( dx, -dy));
	float4 c4 = tex2D(s0, tex + float2(-dx,   0));
	float4 c5 = tex2D(s0, tex + float2( dx,   0));
	float4 c6 = tex2D(s0, tex + float2(-dx,  dy));
	float4 c7 = tex2D(s0, tex + float2(  0,  dy));
	float4 c8 = tex2D(s0, tex + float2( dx,  dy));

	// gaussian filter
	// [ 1, 2, 1 ]
	// [ 2, 4, 2 ]
	// [ 1, 2, 1 ]
	// to normalize the values, we need to divide by the coeff sum
	// 1 / (1+2+1+2+4+2+1+2+1) = 1 / 16 = 0.0625
	float4 flou = (c1 + c3 + c6 + c8 + 2 * (c2 + c4 + c5 + c7) + 4 * orig) * 0.0625;

	// substract blurred image from original image
	float4 corrected = CoefOrig * orig - CoefBlur * flou;

	// edge detection
	// Get neighbor points
	// [ c1,   c2, c3 ]
	// [ c4, orig, c5 ]
	// [ c6,   c7, c8 ]
	c1 = tex2D(s0, tex + float2(-px, -py));
	c2 = tex2D(s0, tex + float2(  0, -py));
	c3 = tex2D(s0, tex + float2( px, -py));
	c4 = tex2D(s0, tex + float2(-px,   0));
	c5 = tex2D(s0, tex + float2( px,   0));
	c6 = tex2D(s0, tex + float2(-px,  py));
	c7 = tex2D(s0, tex + float2(  0,  py));
	c8 = tex2D(s0, tex + float2( px,  py));

	// using Sobel filter
	// horizontal gradient
	// [ -1, 0, 1 ]
	// [ -2, 0, 2 ]
	// [ -1, 0, 1 ]
	float delta1 = (c3 + 2 * c5 + c8) - (c1 + 2 * c4 + c6);

	// vertical gradient
	// [ -1, - 2, -1 ]
	// [  0,   0,  0 ]
	// [  1,   2,  1 ]
	c1 += c3;
	c6 += c8;
	float delta2 = (c6 + 2 * c7) - (c1 + 2 * c2);

	// computation
	if (sqrt(mul(delta1, delta1) + mul(delta2, delta2)) > SharpenEdge) {
		// if we have an edge, use sharpen
		return orig * Sharpen_val0 - (c1 + c2 + c4 + c5 + c6 + c7) * Sharpen_val1;
	} else {
		// else return corrected image
		return corrected;
	}
}
