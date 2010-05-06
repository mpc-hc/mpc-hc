// pngdib.h

#ifndef PNGDIB_H_INCLUDED
#define PNGDIB_H_INCLUDED

#include <tchar.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _UNICODE
// the V2 compatibility layer isn't UNICODE compatible
#define PNGDIB_V2COMPATIBLE  0
#endif

// If you don't need to be compatible with PNGDIB v1.x and v2.x,
// you can define this to 0 and save a few bytes.
#ifndef PNGDIB_V2COMPATIBLE
#define PNGDIB_V2COMPATIBLE  1
#endif

#define PNGDIB_HEADER_VERSION     30002


#define PNGDIB_DEFAULT_SCREEN_GAMMA   2.20000
#define PNGDIB_DEFAULT_FILE_GAMMA     0.45455


#define PNGDIB_DECL  __stdcall

#define PNGDIB_EXT


// error codes returned by pngdib_*_run()

#define PNGD_E_SUCCESS   0
#define PNGD_E_ERROR     1   // unspecified error 
#define PNGD_E_VERSION   2   // struct size problem
#define PNGD_E_NOMEM     3   // could not alloc memory
#define PNGD_E_UNSUPP    4   // unsupported image type
#define PNGD_E_LIBPNG    5   // libpng error (corrupt PNG?)
#define PNGD_E_BADBMP    6   // corrupt or unsupported DIB
#define PNGD_E_BADPNG    7   // corrupt or unsupported PNG
#define PNGD_E_READ      8   // couldn't read PNG file
#define PNGD_E_WRITE     9   // couldn't write PNG file

#if (PNGDIB_V2COMPATIBLE) || defined(PNGDIB_INTERNALS)

struct PNGD_COLOR_struct {
	unsigned char red, green, blue, reserved;
};

#endif


#if PNGDIB_V2COMPATIBLE

typedef struct PNGD_D2PINFO_struct {
	DWORD           structsize;      // sizeof(PNGD_D2PINFO)
	DWORD           flags;
#define PNGD_INTERLACED        0x00000001
#define PNGD_NO_GAMMA_LABEL    0x00000002
#define PNGD_DIB_ALPHA32       0x00000010

	char*           pngfn;          // PNG filename to write

	LPBITMAPINFOHEADER    lpdib;
	int             dibsize;        // can be 0

	VOID*           lpbits;         // can be NULL
	int             bitssize;       // can be 0

	char*           software;       // (NULL==don't include)
// added in v2.0
	char*           errmsg;          // user can set to null or 100-char buffer
} PNGD_D2PINFO;


typedef struct PNGD_IMAGEINFO_struct {
	DWORD           structsize;    // sizeof(PNGD_IMAGEINFO)
	DWORD           flags;

} PNGD_IMAGEINFO;

typedef struct PNGD_P2DINFO_struct {
	DWORD           structsize;      // sizeof(PNGD_P2DINFO)

	DWORD           flags;           // combination of below:
#define PNGD_USE_BKGD          0x00000001
#define PNGD_USE_CUSTOM_BG     0x00000002
#define PNGD_GAMMA_CORRECTION  0x00000004
#define PNGD_USE_HEAPALLOC     0x00000008
//#define PNGD_DIB_ALPHA32     0x00000010  already defined above

#define PNGD_BG_RETURNED     0x00010000 // return value only
#define PNGD_RES_RETURNED    0x00020000 // set if xres,yres,res_units are valid
#define PNGD_GAMMA_RETURNED  0x00040000 // set if file_gamma is valid

	char*           pngfn;           // PNG filename to read

	LPBITMAPINFOHEADER    lpdib;     // return value only
	int             dibsize;         // return value only
	int             palette_offs;    // return value only
	int             bits_offs;       // return value only
	RGBQUAD*        palette;         // return value only
	int             palette_colors;  // return value only
	VOID*           lpbits;          // return value only
// added in v2.0  (size=48)
	struct PNGD_COLOR_struct bgcolor; // IN OUT
	char*           errmsg;          // user can set to null or 100-char buffer
// added in v2.1  (size=88)
	int             color_type;
	int             bits_per_sample;
	int             bits_per_pixel;
	int             interlace;
	int             res_x,res_y;
	int             res_units;
	int             reserved1;
	double          file_gamma;
// added in v2.2   (size=96)
	HANDLE          heap;
	int             reserved2;

} PNGD_P2DINFO;

int read_png_to_dib(PNGD_P2DINFO *p2dinfo);
int write_dib_to_png(PNGD_D2PINFO *d2pinfo);

#endif  // PNGDIB_V2COMPATIBLE


// public definitions

typedef	void* (PNGDIB_DECL *pngdib_malloc_cb_type)(void *userdata, int memblksize);
typedef	void  (PNGDIB_DECL *pngdib_free_cb_type)(void *userdata, void *memblk);
typedef	void* (PNGDIB_DECL *pngdib_realloc_cb_type)(void *userdata, void *memblk, int memblksize);
typedef	void  (PNGDIB_DECL *pngdib_pngptrhook_cb_type)(void *userdata, void *pngptr);

struct pngdib_common_struct;
typedef struct pngdib_common_struct PNGDIB;


#define PNGD_ST_D2P 1
#define PNGD_ST_P2D 2


#ifdef PNGDIB_INTERNALS

// definitions for internal library use only

#define PNGDIB_ERRMSG_MAX 200


struct pngdib_common_struct {
	int structtype;
	void *userdata;
	TCHAR *errmsg;
	pngdib_malloc_cb_type   malloc_function;
	pngdib_free_cb_type     free_function;
	pngdib_realloc_cb_type  realloc_function;
	pngdib_pngptrhook_cb_type pngptrhook_function;
	int dib_alpha32;
};

struct d2p_struct {
	struct pngdib_common_struct common;

	const BITMAPINFOHEADER* pdib;
	int dibsize;
	const void* pbits;
	int bitssize;
	int interlaced;
	TCHAR* output_filename;
	char* software_id_string;
	int file_gamma_valid;
	double file_gamma;
};

struct p2d_struct {
	struct pngdib_common_struct common;

	int input_method; // 0=filename, 1=memory
	TCHAR* input_filename;
	unsigned char* input_memblk;
	int input_memblk_size;
	int input_memblk_curpos;

	int use_file_bg_flag;
	int use_custom_bg_flag;
	struct PNGD_COLOR_struct bgcolor;
	int gamma_correction; // should we gamma correct (using screen_gamma)?
	double screen_gamma;

	BITMAPINFOHEADER*   pdib;
	int        dibsize;
	int        palette_offs;
	int        bits_offs;
	int        bitssize;
	RGBQUAD*   palette;
	int        palette_colors;
	void*      pbits;
	int        color_type;
	int        bits_per_sample;
	int        bits_per_pixel;
	int        interlace;
	int        res_x,res_y;
	int        res_units;
	int        res_valid;  // are res_x, res_y, res_units valid?
	double file_gamma;
	int gamma_returned;  // set if we know the file gamma
	int bgcolor_returned;
};

#endif // PNGDIB_INTERNALS

///////////// d2p functions

#define pngdib_d2p_init()  _pngdib_init(PNGD_ST_D2P,PNGDIB_HEADER_VERSION)

PNGDIB_EXT int PNGDIB_DECL pngdib_d2p_set_dib(PNGDIB *d2p,
        const BITMAPINFOHEADER* pdib, int dibsize,
	    const void* pbits, int bitssize);

PNGDIB_EXT void PNGDIB_DECL pngdib_d2p_set_interlace(PNGDIB *d2p, int interlaced);
PNGDIB_EXT int  PNGDIB_DECL pngdib_d2p_set_png_filename(PNGDIB *d2p, const TCHAR *fn);
PNGDIB_EXT int  PNGDIB_DECL pngdib_d2p_set_software_id(PNGDIB *d2p, const TCHAR *s);
PNGDIB_EXT void PNGDIB_DECL pngdib_d2p_set_gamma_label(PNGDIB *d2p, int flag, double file_gamma);

PNGDIB_EXT int  PNGDIB_DECL pngdib_d2p_run(PNGDIB *d2p);

////////////

//////////// p2d functions

#define pngdib_p2d_init()  _pngdib_init(PNGD_ST_P2D,PNGDIB_HEADER_VERSION)

PNGDIB_EXT int  PNGDIB_DECL pngdib_p2d_set_png_filename(PNGDIB *p2d, const TCHAR *fn);
PNGDIB_EXT void PNGDIB_DECL pngdib_p2d_set_png_memblk(PNGDIB *p2d, const void *mem, int memsize);

PNGDIB_EXT void PNGDIB_DECL pngdib_p2d_set_use_file_bg(PNGDIB *p2d, int flag);
PNGDIB_EXT void PNGDIB_DECL pngdib_p2d_set_custom_bg(PNGDIB *p2d, unsigned char r,
								  unsigned char g, unsigned char b);
PNGDIB_EXT void PNGDIB_DECL pngdib_p2d_set_gamma_correction(PNGDIB *p2d, int flag, double screen_gamma);

PNGDIB_EXT int  PNGDIB_DECL pngdib_p2d_run(PNGDIB *p2d);

PNGDIB_EXT int  PNGDIB_DECL pngdib_p2d_get_dib(PNGDIB *p2d, BITMAPINFOHEADER **ppdib, int *pdibsize);
PNGDIB_EXT int  PNGDIB_DECL pngdib_p2d_get_dibbits(PNGDIB *p2d, void **ppbits, int *pbitsoffset, int *pbitssize);
PNGDIB_EXT int  PNGDIB_DECL pngdib_p2d_get_palette(PNGDIB *p2d, RGBQUAD **ppal, int *ppaloffset, int *ppalnumcolors);   

PNGDIB_EXT int  PNGDIB_DECL pngdib_p2d_get_colortype(PNGDIB *p2d);
PNGDIB_EXT int  PNGDIB_DECL pngdib_p2d_get_bitspersample(PNGDIB *p2d);
PNGDIB_EXT int  PNGDIB_DECL pngdib_p2d_get_bitsperpixel(PNGDIB *p2d);
PNGDIB_EXT int  PNGDIB_DECL pngdib_p2d_get_samplesperpixel(PNGDIB *p2d);
PNGDIB_EXT int  PNGDIB_DECL pngdib_p2d_get_interlace(PNGDIB *p2d);
PNGDIB_EXT int  PNGDIB_DECL pngdib_p2d_get_density(PNGDIB *p2d, int *pres_x, int *pres_y, int *pres_units);
PNGDIB_EXT int  PNGDIB_DECL pngdib_p2d_get_file_gamma(PNGDIB *p2d, double *pgamma);
PNGDIB_EXT int  PNGDIB_DECL pngdib_p2d_get_bgcolor(PNGDIB *p2d, unsigned char *pr, unsigned char *pg, unsigned char *pb);

PNGDIB_EXT void PNGDIB_DECL pngdib_p2d_free_dib(PNGDIB *p2d, BITMAPINFOHEADER *pdib);

//////////// common functions

PNGDIB_EXT PNGDIB* PNGDIB_DECL _pngdib_init(int structtype, int caller_header_vers);

PNGDIB_EXT int   PNGDIB_DECL pngdib_done(PNGDIB *xx);

PNGDIB_EXT void  PNGDIB_DECL pngdib_setcallback_malloc(PNGDIB *xx,
        pngdib_malloc_cb_type   mallocfunc,
        pngdib_free_cb_type     freefunc,
        pngdib_realloc_cb_type  reallocfunc);

PNGDIB_EXT void PNGDIB_DECL pngdib_setcallback_pngptrhook(PNGDIB *xx,
        pngdib_pngptrhook_cb_type pngptrhookfn);

PNGDIB_EXT void   PNGDIB_DECL pngdib_set_userdata(PNGDIB *xx, void *userdata);
PNGDIB_EXT void*  PNGDIB_DECL pngdib_get_userdata(PNGDIB *xx);
PNGDIB_EXT TCHAR* PNGDIB_DECL pngdib_get_error_msg(PNGDIB *xx);

PNGDIB_EXT TCHAR* PNGDIB_DECL pngdib_get_version_string(void);
PNGDIB_EXT int    PNGDIB_DECL pngdib_get_version(void);

PNGDIB_EXT void   PNGDIB_DECL pngdib_set_dibalpha32(PNGDIB *xx, int flag);

////////////


#ifdef __cplusplus
}
#endif

#endif // PNGDIB_H_INCLUDED
