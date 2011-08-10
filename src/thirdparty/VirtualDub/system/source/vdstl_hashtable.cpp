//	VirtualDub - Video processing and capture application
//	System library component
//	Copyright (C) 1998-2010 Avery Lee, All Rights Reserved.
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

#include <stdafx.h>
#include <vd2/system/vdstl_hashtable.h>

vdhashtable_base_node *const vdhashtable_base::sEmptyBucket = {NULL};

vdhashtable_base::vdhashtable_base()
	: mBucketCount(0)
	, mElementCount(0)
	, mpBucketStart(const_cast<vdhashtable_base_node **>(&sEmptyBucket))
	, mpBucketEnd(const_cast<vdhashtable_base_node **>(&sEmptyBucket))
{
}

vdhashtable_base::size_type vdhashtable_base::bucket_count() const {
	return mpBucketEnd - mpBucketStart;
}

vdhashtable_base::size_type vdhashtable_base::max_bucket_count() const {
	return (size_type)-1 >> 1;
}

vdhashtable_base::size_type vdhashtable_base::bucket_size(size_type n) const {
	VDASSERT(n < (size_type)(mpBucketEnd - mpBucketStart));

	size_type len = 0;
	for(vdhashtable_base_node *p = mpBucketStart[n]; p; p = p->mpHashNext)
		++len;

	return len;
}

vdhashtable_base::size_type vdhashtable_base::compute_bucket_count(size_type n) {
	static const size_t kBucketSizes[]={
		11,
		17, 37, 67, 131,
		257, 521, 1031, 2049,
		4099, 8209, 16411, 32771,
		65537, 131101, 262147, 524309,
		1048583, 2097169, 4194319, 8388617,
		16777259, 33554467, 67108879, 134217757,
		268435459, 536870923, 1073741827
	};

	int i = 0;
	size_type buckets;

	while(i < sizeof(kBucketSizes)/sizeof(kBucketSizes[0])) {
		buckets = kBucketSizes[i];

		if (n <= buckets)
			break;

		++i;
	}

	return buckets;
}
