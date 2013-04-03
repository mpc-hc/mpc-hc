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

#define clock (p0[3])
#define PI acos(-1)

float4 main(float2 tex : TEXCOORD0) : COLOR
{
	// - this is a very simple raytracer, one sphere only
	// - no reflection or refraction, yet (my ati 9800 has a 64 + 32 instruction limit...)

	float3 pl = float3(3, -3, -4); // light pos
	float4 cl = 0.4; // light color

	float3 pc = float3(0, 0, -1);  // cam pos
	float3 ps = float3(0, 0, 0.5); // sphere pos
	float r = 0.65; // sphere radius

	float3 pd = normalize(float3(tex.x - 0.5, tex.y - 0.5, 0) - pc);

	float A = 1;
	float B = 2 * dot(pd, pc - ps);
	float C = dot(pc - ps, pc - ps) - r * r;
	float D = B * B - 4 * A * C;

	float4 c0 = 0;

	if (D >= 0) {
		// t2 is the smaller, obviously...
		// float t1 = (-B + sqrt(D)) / (2 * A);
		// float t2 = (-B - sqrt(D)) / (2 * A);
		// float t = min(t1, t2);

		float t = (-B - sqrt(D)) / (2 * A);

		// intersection data
		float3 p = pc + pd * t;
		float3 n = normalize(p  - ps);
		float3 l = normalize(pl - p);

		// mapping the image onto the sphere
		tex = acos(-n) / PI;

		// rotate it
		tex.x = frac(tex.x + frac(clock / 10));

		// diffuse + specular
		c0 = tex2D(s0, tex) * dot(n, l) + cl * pow(max(dot(l, reflect(pd, n)), 0), 50);
	}

	return c0;
}
