//	VirtualDub - Video processing and capture application
//	System library component
//	Copyright (C) 1998-2004 Avery Lee, All Rights Reserved.
//
//	Beginning with 1.6.0, the VirtualDub system library is licensed
//	differently than the remainder of VirtualDub.  This particular file is
//	thus licensed as follows (the "zlib" license):
//
//	This software is provided 'as-is', without any express or implied
//	warranty.  In no event will the authors be held liable for any
//	damages arising from the use of this software.
//
//	Permission is granted to anyone to use this software for any purpose,
//	including commercial applications, and to alter it and redistribute it
//	freely, subject to the following restrictions:
//
//	1.	The origin of this software must not be misrepresented; you must
//		not claim that you wrote the original software. If you use this
//		software in a product, an acknowledgment in the product
//		documentation would be appreciated but is not required.
//	2.	Altered source versions must be plainly marked as such, and must
//		not be misrepresented as being the original software.
//	3.	This notice may not be removed or altered from any source
//		distribution.

#include "stdafx.h"
#include <vd2/system/vdstl.h>
#include <vd2/system/vectors.h>

bool VDSolveLinearEquation(double *src, int n, ptrdiff_t stride_elements, double *b, double tolerance) {
	vdfastvector<double *> array(n);
	double **m = &array[0];
	int i, j, k;

	for(i=0; i<n; ++i) {
		m[i] = src;
		src += stride_elements;
	}

	// factor U
	for(i=0; i<n; ++i) {
		int best = i;

		for(j=i+1; j<n; ++j) {
			if (fabs(m[best][i]) < fabs(m[j][i]))
				best = j;
		}

		std::swap(m[i], m[best]);
		std::swap(b[i], b[best]);

		if (fabs(m[i][i]) < tolerance)
			return false;

		double f = 1.0 / m[i][i];

		m[i][i] = 1.0;

		for(j=i+1; j<n; ++j)
			m[i][j] *= f;

		b[i] *= f;

		for(j=i+1; j<n; ++j) {
			b[j] -= b[i] * m[j][i];
			for(k=n-1; k>=i; --k)
				m[j][k] -= m[i][k] * m[j][i];
		}
	}

	// factor L
	for(i=n-1; i>=0; --i)
		for(j=i-1; j>=0; --j)
			b[j] -= b[i] * m[j][i];

	return true;
}

template<>
bool vdrect32::contains(const vdpoint32& pt) const {
	return ((uint32)pt.x - (uint32)left) < (uint32)right - (uint32)left
		&& ((uint32)pt.y - (uint32)top) < (uint32)bottom - (uint32)top;
}
