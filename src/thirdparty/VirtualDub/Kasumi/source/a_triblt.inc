		struc	span
.u		resd	1
.v		resd	1
		endstruc

		struc	mipspan
.u		resd	1
.v		resd	1
.lambda	resd	1
		endstruc

		struc	mipmap
.bits	resd	1
.pitch	resd	1
.uvmul	resd	1
		resd	1
		endstruc

		struc	texinfo
.mips	resd	16*4
.dst	resd	1
.src	resd	1
.w		resd	1
		endstruc
