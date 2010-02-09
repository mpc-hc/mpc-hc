#ifndef _POSTPROCFILTERS_H_
#define _POSTPROCFILTERS_H_

#define V_DEBLOCK       0x01
#define H_DEBLOCK       0x02
#define DERING          0x04
#define LEVEL_FIX	0x08 ///< Brightness & Contrast

#define LUM_V_DEBLOCK   V_DEBLOCK               //   1
#define LUM_H_DEBLOCK   H_DEBLOCK               //   2
#define CHROM_V_DEBLOCK (V_DEBLOCK<<4)          //  16
#define CHROM_H_DEBLOCK (H_DEBLOCK<<4)          //  32
#define LUM_DERING      DERING                  //   4
#define CHROM_DERING    (DERING<<4)             //  64
#define LUM_LEVEL_FIX	LEVEL_FIX		//   8
#define CHROM_LEVEL_FIX	(LEVEL_FIX<<4)		// 128 (not implemented yet)

// Experimental vertical filters
#define V_X1_FILTER	0x0
#define V_A_DEBLOCK	0x2000000

// Experimental horizontal filters
#define H_X1_FILTER	0x0
#define H_A_DEBLOCK	0x4000000

/// select between full y range (255-0) or standart one (234-16)
#define FULL_Y_RANGE	0x8000			// 32768

//Deinterlacing Filters
#define	LINEAR_IPOL_DEINT_FILTER	 0x10000	// 65536
#define	LINEAR_BLEND_DEINT_FILTER	 0x20000	// 131072
#define	CUBIC_BLEND_DEINT_FILTER	  0x8000	// (not implemented yet)
#define	CUBIC_IPOL_DEINT_FILTER		 0x40000	// 262144
#define	MEDIAN_DEINT_FILTER		     0x80000	// 524288
#define	FFMPEG_DEINT_FILTER		    0x400000
#define	LOWPASS5_DEINT_FILTER		0x800000

#define TEMP_NOISE_FILTER		0x100000
#define FORCE_QUANT			0x200000

#endif
