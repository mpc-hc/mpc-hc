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
float4 p1 :  register(c1);

#define width   (p0[0])
#define height  (p0[1])
#define counter (p0[2])
#define clock   (p0[3])
#define one_over_width  (p1[0])
#define one_over_height (p1[1])

#define PI acos(-1)

static float4x4 r2y = {
	 0.299,  0.587,  0.114, 0.000,
	-0.147, -0.289,  0.437, 0.000,
	 0.615, -0.515, -0.100, 0.000,
	 0.000,  0.000,  0.000, 0.000
};

static float4x4 y2r = {
	1.000,  0.000,  1.140, 0.000,
	1.000, -0.394, -0.581, 0.000,
	1.000,  2.028,  0.000, 0.000,
	0.000,  0.000,  0.000, 0.000
};

#define ymin ( 16.0 / 255)
#define ymax (235.0 / 255)

// Brightness: -1.0 to 1.0, default 0.0
// Contrast: 0.0 to 10.0, default 1.0
// Hue: -180.0 to +180.0, default 0.0
// Saturation: 0.0 to 10.0, default 1.0

#define Brightness 0.0
#define Contrast   1.0
#define Hue        0.0
#define Saturation 1.0

// tv -> pc scale
// #define Brightness (-ymin)
// #define Contrast   (1.0 / (ymax - ymin))

static float2x2 HueMatrix = {
	 cos(Hue * PI / 180), sin(Hue * PI / 180),
	-sin(Hue * PI / 180), cos(Hue * PI / 180)
};

float4 main(float2 tex : TEXCOORD0) : COLOR
{
	float4 c0 = tex2D(s0, tex);
	c0 = mul(r2y, c0);
	c0.r = Contrast * (c0.r - ymin) + ymin + Brightness;
	c0.gb = mul(HueMatrix, c0.gb) * Saturation;
	c0 = mul(y2r, c0);
	return c0;
}
