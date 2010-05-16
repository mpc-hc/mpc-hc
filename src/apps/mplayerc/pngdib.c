// pngdib.c
//
// PNGDIB - a mini PNG<->DIB (BMP) image conversion library for Win32
// By Jason Summers
// This software may be used without restriction.
//

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <malloc.h>
#include <math.h>

#include "../../libpng/png.h"

#define PNGDIB_INTERNALS
#include "pngdib.h"
#include <strsafe.h>

#define PNGDIB_SRC_VERSION              30002
#define PNGDIB_SRC_VERSION_STRING   _T("3.0.2")


#if PNGDIB_SRC_VERSION != PNGDIB_HEADER_VERSION
#error Wrong PNGDIB header file version
#endif

#if (PNG_LIBPNG_VER<10202) || \
    (PNG_LIBPNG_VER==10202 && PNG_LIBPNG_BUILD_TYPE<2) || \
    (PNG_LIBPNG_VER==10202 && PNG_LIBPNG_BUILD_TYPE==2 && PNG_LIBPNG_VER_BUILD<5)
#error libpng 1.2.2b5 or higher is recommended
/* You can comment out the previous line if you aren't using gamma
 * correction, or don't care about a few obscure gamma correction
 * problems that exist in earlier versions of libpng. */
#endif


// This is basically a Windows-only utility with a simple-as-possible
// interface, so I'm not too concerned about allowing a 
// user-configurable screen gamma.
//static const double screen_gamma = 2.2;

struct errstruct {
	jmp_buf *jbufp;
	TCHAR *errmsg;
};

static void pngd_get_error_message(int rv,TCHAR *e, int e_len)
{
	switch(rv) {
	case PNGD_E_ERROR:   StringCchCopy(e,e_len,_T("Unknown error")); break;
	case PNGD_E_VERSION: StringCchCopy(e,e_len,_T("Incompatible library version")); break;
	case PNGD_E_NOMEM:   StringCchCopy(e,e_len,_T("Unable to allocate memory")); break;
	case PNGD_E_UNSUPP:  StringCchCopy(e,e_len,_T("Invalid or unsupported image")); break;
	case PNGD_E_LIBPNG:  StringCchCopy(e,e_len,_T("libpng reported an error")); break;
	case PNGD_E_BADBMP:  StringCchCopy(e,e_len,_T("Invalid BMP image")); break;
	case PNGD_E_BADPNG:  StringCchCopy(e,e_len,_T("Invalid PNG image")); break;
	case PNGD_E_READ:    StringCchCopy(e,e_len,_T("Unable to read file")); break;
	case PNGD_E_WRITE:   StringCchCopy(e,e_len,_T("Unable to write file")); break;
	}
}

static unsigned char* uncompress_dib(LPBITMAPINFO lpbmi1, int infosize, void *lpbits1)
{
	LPBITMAPINFOHEADER lpdib2;
	unsigned char *lpbits2;
	void *whatever;
	int linesize, bitssize;
	HBITMAP hb;
	HDC hdc;
	HGDIOBJ rvgdi;
	int rvi;
	int width,height;
	LPBITMAPINFOHEADER lpdib1;

	lpdib1=(LPBITMAPINFOHEADER)lpbmi1;
	width=lpdib1->biWidth;
	height=lpdib1->biHeight;

	linesize= (((width * lpdib1->biBitCount)+31)/32)*4;
	bitssize= linesize*height;

	lpdib2= (LPBITMAPINFOHEADER)malloc(infosize);
	if(!lpdib2) return NULL;

	//  create a header for the new uncompressed DIB
	CopyMemory((void*)lpdib2,(void*)lpdib1,infosize);
	lpdib2->biCompression=BI_RGB;
	lpdib2->biSizeImage=0;

	lpbits2= (unsigned char*)malloc(bitssize);
	if(!lpbits2) { free((void*)lpdib2); return NULL; }


	// Windows bitmap handling functions are not exactly convenient,
	// especially when trying to deal with DIBs. Every function wants
	// to convert them into DDBs. We have to play stupid games and
	// convert back and forth. This probably uses too much memory,
	// and I'm not 100% sure it is exactly correct, but it seems to
	// work for me.

	hb=CreateDIBSection(NULL,(LPBITMAPINFO)lpdib2,DIB_RGB_COLORS,&whatever,NULL,0);

	hdc=CreateCompatibleDC(NULL);
	rvgdi=SelectObject(hdc,hb);
	//SetStretchBltMode(hdc,COLORONCOLOR);
	rvi=StretchDIBits(hdc,
		0,0,width,height,
		0,0,width,height,
		lpbits1, (LPBITMAPINFO)lpdib1,
		DIB_RGB_COLORS,SRCCOPY);
	rvi=GetDIBits(hdc,hb,0,height, (LPVOID)lpbits2,
		(LPBITMAPINFO)lpdib2,DIB_RGB_COLORS);

	DeleteDC(hdc);
	DeleteObject(hb);
	free((void*)lpdib2);

	return lpbits2;
}


static void my_png_error_fn(png_structp png_ptr, const char *err_msg)
{
	struct errstruct *errinfop;
	jmp_buf *j;

	errinfop = (struct errstruct *)png_get_error_ptr(png_ptr);
	j = errinfop->jbufp;

#ifdef _UNICODE
	StringCchPrintf(errinfop->errmsg,PNGDIB_ERRMSG_MAX,_T("[libpng] %S"),err_msg);
#else
	StringCchPrintf(errinfop->errmsg,PNGDIB_ERRMSG_MAX,"[libpng] %s",err_msg);
#endif

	longjmp(*j, -1);
}


static void my_png_warning_fn(png_structp png_ptr, const char *warn_msg)
{
	return;
}

// A callback function used when reading memory-mapped PNG files.
static void my_png_read_fn(png_structp png_ptr,
      png_bytep data, png_size_t length)
{
	struct p2d_struct *p2d;

	p2d = (struct p2d_struct*)png_get_io_ptr(png_ptr);

	if(p2d->input_memblk_size>0) {
		if((int)length > (p2d->input_memblk_size - p2d->input_memblk_curpos)) {
			png_error(png_ptr, "read error: unexpected end of file");
			return;
		}
	}

	CopyMemory((void*)data,(void*)&p2d->input_memblk[p2d->input_memblk_curpos],length);
	p2d->input_memblk_curpos+=length;
}


// This function should perform identically to libpng's gamma correction.
// I'd prefer to have libpng do all gamma correction itself,
// but I can't figure out how to do that efficiently.
static void gamma_correct(double screen_gamma,double file_gamma,
	 unsigned char *red, unsigned char *green, unsigned char *blue)
{
	double g;

#ifndef PNG_GAMMA_THRESHOLD
#  define PNG_GAMMA_THRESHOLD 0.05
#endif

	if(fabs(screen_gamma*file_gamma-1.0)<=PNG_GAMMA_THRESHOLD) return;

	if (screen_gamma>0.000001)
		g=1.0/(file_gamma*screen_gamma);
	else
		g=1.0;

	(*red)   = (unsigned char)(pow((double)(*red  )/255.0,g)*255.0+0.5);
	(*green) = (unsigned char)(pow((double)(*green)/255.0,g)*255.0+0.5);
	(*blue)  = (unsigned char)(pow((double)(*blue )/255.0,g)*255.0+0.5);
}


int PNGDIB_DECL pngdib_p2d_run(PNGDIB *qq)
{
	struct p2d_struct *p2d;

	png_structp png_ptr;
	png_infop info_ptr;
	jmp_buf jbuf;
	struct errstruct errinfo;
	png_uint_32 width, height;
	int png_bit_depth, color_type, interlace_type;
	png_colorp png_palette;
	png_uint_32 res_x, res_y;
	int has_phys, has_gama;
	int res_unit_type;
	FILE *fp;
	int palette_entries;
	unsigned char **row_pointers;
	unsigned char *lpdib;
	unsigned char *dib_palette;
	unsigned char *dib_bits;
	unsigned char *tmprow;
	int dib_bpp, dib_bytesperrow;
	int i,j;
	int rv;
	png_color_16 bkgd; // used with png_set_background
	int has_trns, trns_color;
	int has_bkgd;  // ==1 if there a bkgd chunk, and USE_BKGD flag
	png_color_16p temp_colorp;
	png_color_16p bg_colorp;  // background color (if has_bkgd)
	png_bytep trns_trans;
	int manual_trns;
	int manual_gamma;
	struct PNGD_COLOR_struct bkgd_color;
	int is_grayscale,has_alpha_channel;
	double file_gamma;
	int dib_alpha32;
	int write_bitfields;

	p2d=(struct p2d_struct*)qq;

	dib_alpha32=0;
	write_bitfields=0;


	manual_trns=0;
	has_trns=has_bkgd=0;
	rv=PNGD_E_ERROR;
	png_ptr=NULL;
	info_ptr=NULL;
	fp=NULL;
	row_pointers=NULL;
	lpdib=NULL;

	StringCchCopy(p2d->common.errmsg,PNGDIB_ERRMSG_MAX,_T(""));

	if(p2d->use_custom_bg_flag) {
		bkgd_color.red=   p2d->bgcolor.red;
		bkgd_color.green= p2d->bgcolor.green;
		bkgd_color.blue=  p2d->bgcolor.blue;
	}
	else {
		bkgd_color.red=   255; // Should never get used. If the
		bkgd_color.green= 128; // background turns orange, it's a bug.
		bkgd_color.blue=  0;
	}

	// Set the user-defined pointer to point to our jmp_buf. This will
	// hopefully protect against potentially different sized jmp_buf's in
	// libpng, while still allowing this library to be threadsafe.
	errinfo.jbufp = &jbuf;
	errinfo.errmsg = p2d->common.errmsg;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,(void*)(&errinfo),
		my_png_error_fn, my_png_warning_fn);
	if(!png_ptr) { rv=PNGD_E_NOMEM; goto abort; }

	if(p2d->common.pngptrhook_function) {
		(*(p2d->common.pngptrhook_function))(p2d->common.userdata,(void*)png_ptr);
	}

	info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr) {
		//png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		rv=PNGD_E_NOMEM; goto abort;
	}

	if(setjmp(jbuf)) {
		// we'll get here if an error occurred in any of the following
		// png_ functions

		rv=PNGD_E_LIBPNG;
		goto abort;
	}

	if(p2d->input_method==0) {
		// reading from a filename
		if(!p2d->input_filename) {
			StringCchPrintf(p2d->common.errmsg,PNGDIB_ERRMSG_MAX,_T("Input filename not set"));
			rv=PNGD_E_ERROR; goto abort;
		}

		if((fp = _tfopen(p2d->input_filename,_T("rb"))) == NULL) {
			rv=PNGD_E_READ;
			goto abort;
		}
		png_init_io(png_ptr, fp);
	}
	else if(p2d->input_method==1) {
		// reading from a memory block
		p2d->input_memblk_curpos=0;
		png_set_read_fn(png_ptr, (void*)p2d, my_png_read_fn);
	}
	else { goto abort; }

	png_read_info(png_ptr, info_ptr);

	png_get_IHDR(png_ptr, info_ptr, &width, &height, &png_bit_depth, &color_type,
		&interlace_type, NULL, NULL);

	p2d->color_type=color_type;
	p2d->bits_per_sample=png_bit_depth;
	p2d->interlace=interlace_type;
	switch(color_type) {
	case PNG_COLOR_TYPE_RGB:        p2d->bits_per_pixel=png_bit_depth*3; break;
	case PNG_COLOR_TYPE_RGB_ALPHA:  p2d->bits_per_pixel=png_bit_depth*4; break;
	case PNG_COLOR_TYPE_GRAY_ALPHA: p2d->bits_per_pixel=png_bit_depth*2; break;
	default: p2d->bits_per_pixel=png_bit_depth;
	}

	is_grayscale = !(color_type&PNG_COLOR_MASK_COLOR);
	has_alpha_channel = (color_type&PNG_COLOR_MASK_ALPHA)?1:0;

	has_trns = png_get_valid(png_ptr,info_ptr,PNG_INFO_tRNS);

	if(p2d->common.dib_alpha32 && (has_trns || has_alpha_channel)) {
		// Fixme - if trns(for palette) has no transparent entries,
		// we could skip this.
		dib_alpha32=1;
		write_bitfields=1;

		if (!(color_type&PNG_COLOR_MASK_COLOR)) {
			png_set_gray_to_rgb(png_ptr);
		}
		else if(color_type==PNG_COLOR_TYPE_PALETTE) {
			png_set_palette_to_rgb(png_ptr);
		}

		if (has_trns) png_set_tRNS_to_alpha(png_ptr);

		goto notrans;
	}

	// look for bKGD chunk, and process if applicable
	if(p2d->use_file_bg_flag) {
		if(png_get_bKGD(png_ptr, info_ptr, &bg_colorp)) {
			// process the background, store 8-bit RGB in bkgd_color
			has_bkgd=1;

			if(is_grayscale && png_bit_depth<8) {
				bkgd_color.red  =
				bkgd_color.green=
				bkgd_color.blue =
					(unsigned char) ( (bg_colorp->gray*255)/( (1<<png_bit_depth)-1 ) );
			}
			else if(png_bit_depth<=8) {
				bkgd_color.red=(unsigned char)(bg_colorp->red);
				bkgd_color.green=(unsigned char)(bg_colorp->green);
				bkgd_color.blue =(unsigned char)(bg_colorp->blue);
			}
			else {
				bkgd_color.red=(unsigned char)(bg_colorp->red>>8);
				bkgd_color.green=(unsigned char)(bg_colorp->green>>8);
				bkgd_color.blue =(unsigned char)(bg_colorp->blue>>8);
			}
		}
	}

	if( !(color_type & PNG_COLOR_MASK_ALPHA) && !has_trns) {
		// If no transparency, we can skip this whole background-color mess.
		goto notrans;
	}

	if(has_bkgd && (png_bit_depth>8 || !is_grayscale || has_alpha_channel)) {
		png_set_background(png_ptr, bg_colorp,
			   PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
	}
	else if(is_grayscale && has_trns && png_bit_depth<=8
		&& (has_bkgd || (p2d->use_custom_bg_flag)) )
	{
		// grayscale binarytrans,<=8bpp: transparency is handle manually
		// by modifying a palette entry (later)
		png_get_tRNS(png_ptr,info_ptr,&trns_trans, &i, &temp_colorp);
		if(i>=1) {
			trns_color= temp_colorp->gray; // corresponds to a palette entry
			manual_trns=1;
		}
	}
	else if(!has_bkgd && (has_trns || has_alpha_channel) && 
		(p2d->use_custom_bg_flag) ) 
	{      // process most CUSTOM background colors
		bkgd.index = 0; // unused
		bkgd.red   = p2d->bgcolor.red;
		bkgd.green = p2d->bgcolor.green;
		bkgd.blue  = p2d->bgcolor.blue;

		// libpng may use bkgd.gray if bkgd.red==bkgd.green==bkgd.blue.
		// Not sure if that's a libpng bug or not.
		bkgd.gray  = p2d->bgcolor.red;

		if(png_bit_depth>8) {
			bkgd.red  = (bkgd.red  <<8)|bkgd.red; 
			bkgd.green= (bkgd.green<<8)|bkgd.green;
			bkgd.blue = (bkgd.blue <<8)|bkgd.blue;
			bkgd.gray = (bkgd.gray <<8)|bkgd.gray;
		}

		if(is_grayscale) {
			/* assert(png_bit_depth>8); */

			/* Need to expand to full RGB if unless background is pure gray */
			if(bkgd.red!=bkgd.green || bkgd.red!=bkgd.blue) {
				png_set_gray_to_rgb(png_ptr);

				// png_set_tRNS_to_alpha() is called here because otherwise
				// binary transparency for 16-bps grayscale images doesn't
				// work. Libpng will think black pixels are transparent.
				// I don't know exactly why it works. It does *not* add an
				// alpha channel, as you might think (adding an alpha
				// channnel makes no sense if you are using 
				// png_set_background).
				//
				// Here's an alternate hack that also seems to work, but
				// uses direct structure access:
				//
				// png_ptr->trans_values.red   =    				
				//  png_ptr->trans_values.green =
				//	png_ptr->trans_values.blue  = png_ptr->trans_values.gray;
				if(has_trns) 
					png_set_tRNS_to_alpha(png_ptr);

				png_set_background(png_ptr, &bkgd,
					  PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);

			}
			else {  // gray custom background
				png_set_background(png_ptr, &bkgd,
					  PNG_BACKGROUND_GAMMA_SCREEN, 1, 1.0);
			}

		}
		else {
			png_set_background(png_ptr, &bkgd,
				  PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);
		}
	}

notrans:

	// If we don't have any background color at all that we can use,
	// strip the alpha channel.
	if(!dib_alpha32 && has_alpha_channel && !has_bkgd && 
		!(p2d->use_custom_bg_flag) )
	{
		png_set_strip_alpha(png_ptr);
	}

	if(png_bit_depth>8)
		png_set_strip_16(png_ptr);

	if (png_get_sRGB(png_ptr, info_ptr, &i)) {
		has_gama=1;
		file_gamma = 0.45455;
	}
	else if(png_get_gAMA(png_ptr, info_ptr, &file_gamma)) {
		has_gama=1;
	}
	else {
		has_gama=0;
		file_gamma = 0.45455;
	}

	if(/*imginfo && */ has_gama) {
		p2d->file_gamma=file_gamma;
		p2d->gamma_returned=1;
	}

	manual_gamma=0;
	if(p2d->gamma_correction) {

		if(!is_grayscale || png_bit_depth>8 || has_alpha_channel) {
			png_set_gamma(png_ptr, p2d->screen_gamma, file_gamma);
			//png_ptr->transformations |= 0x2000; // hack for old libpng versions
		}
		else manual_gamma=1;

		if(has_bkgd) {
			// Gamma correct the background color (if we got it from the file)
			// before returning it to the app.
			gamma_correct(p2d->screen_gamma,file_gamma,&bkgd_color.red,&bkgd_color.green,&bkgd_color.blue);
		}
	}

	png_read_update_info(png_ptr, info_ptr);

	// color type may have changed, due to our transformations
	color_type = png_get_color_type(png_ptr,info_ptr);


	switch(color_type) {
	case PNG_COLOR_TYPE_RGB_ALPHA:
		assert(dib_alpha32);
		dib_bpp= 32;
		palette_entries=0;
		png_set_bgr(png_ptr);
		break;
	case PNG_COLOR_TYPE_RGB:
		dib_bpp= 24;
		palette_entries=0;
		png_set_bgr(png_ptr);
		break;
	case PNG_COLOR_TYPE_PALETTE:
		dib_bpp=png_bit_depth;
		png_get_PLTE(png_ptr,info_ptr,&png_palette,&palette_entries);
		break;
	case PNG_COLOR_TYPE_GRAY:
	case PNG_COLOR_TYPE_GRAY_ALPHA:
		dib_bpp=png_bit_depth;
		if(png_bit_depth>8) dib_bpp=8;
		palette_entries= 1<<dib_bpp;
		// we'll construct a grayscale palette later
		break;
	default:
		rv=PNGD_E_BADPNG;
		goto abort;
	}

	if(dib_bpp==2) dib_bpp=4;

	has_phys=png_get_valid(png_ptr,info_ptr,PNG_INFO_pHYs);
	if(has_phys) {
		png_get_pHYs(png_ptr,info_ptr,&res_x,&res_y,&res_unit_type);
		if(res_x>0 && res_y>0) {
			p2d->res_x=res_x;
			p2d->res_y=res_y;
			p2d->res_units=res_unit_type;
			p2d->res_valid=1;
		}
	}

	// DIB scanlines are padded to 4-byte boundaries.
	dib_bytesperrow= (((width * dib_bpp)+31)/32)*4;

	p2d->bitssize = height*dib_bytesperrow;

	p2d->dibsize=sizeof(BITMAPINFOHEADER) + 4*palette_entries +
		(write_bitfields?12:0) + p2d->bitssize;

	if(p2d->common.malloc_function) {
		lpdib = (unsigned char*)(*(p2d->common.malloc_function))(p2d->common.userdata,p2d->dibsize);
	}
	else {
		lpdib = (unsigned char*)calloc(p2d->dibsize,1);
	}


	if(!lpdib) { rv=PNGD_E_NOMEM; goto abort; }
	p2d->pdib = (LPBITMAPINFOHEADER)lpdib;

	row_pointers=(unsigned char**)malloc(height*sizeof(unsigned char*));
	if(!row_pointers) { rv=PNGD_E_NOMEM; goto abort; }

	// there is some redundancy here...
	p2d->palette_offs=sizeof(BITMAPINFOHEADER);
	p2d->bits_offs   =sizeof(BITMAPINFOHEADER) + 4*palette_entries + (write_bitfields?12:0);
	dib_palette= &lpdib[p2d->palette_offs];
	p2d->palette= (RGBQUAD*)dib_palette;
	dib_bits   = &lpdib[p2d->bits_offs];
	p2d->pbits = (VOID*)dib_bits;
	p2d->palette_colors = palette_entries;

	// set up the DIB palette, if needed
	switch(color_type) {
	case PNG_COLOR_TYPE_PALETTE:
		for(i=0;i<palette_entries;i++) {
			p2d->palette[i].rgbRed   = png_palette[i].red;
			p2d->palette[i].rgbGreen = png_palette[i].green;
			p2d->palette[i].rgbBlue  = png_palette[i].blue;
		}
		break;
	case PNG_COLOR_TYPE_GRAY:
	case PNG_COLOR_TYPE_GRAY_ALPHA:
		for(i=0;i<palette_entries;i++) {
			p2d->palette[i].rgbRed   =
			p2d->palette[i].rgbGreen =
			p2d->palette[i].rgbBlue  = (i*255)/(palette_entries-1);
			if(manual_gamma) {
				gamma_correct(p2d->screen_gamma,file_gamma,
					  &(p2d->palette[i].rgbRed),
					  &(p2d->palette[i].rgbGreen),
					  &(p2d->palette[i].rgbBlue));
			}
		}
		if(manual_trns) {
			p2d->palette[trns_color].rgbRed   = bkgd_color.red;
			p2d->palette[trns_color].rgbGreen = bkgd_color.green;
			p2d->palette[trns_color].rgbBlue  = bkgd_color.blue;
		}
		break;
	}

	// set up BITFIELDS, if needed
	if(dib_alpha32) {
		static const unsigned char bf_data[12] = {
			0x00,0x00,0xff,0x00,
			0x00,0xff,0x00,0x00,
			0xff,0x00,0x00,0x00
		};

		for(i=0;i<12;i++) {
			lpdib[p2d->palette_offs+i]=bf_data[i];
		}

	}

	for(j=0;j<(int)height;j++) {
		row_pointers[height-1-j]= &dib_bits[j*dib_bytesperrow];
	}

	png_read_image(png_ptr, row_pointers);

	// special handling for this bit depth, since it doesn't exist in DIBs
	// expand 2bpp to 4bpp
	if(png_bit_depth==2) {
		tmprow = (unsigned char*)malloc((width+3)/4 );
		if(!tmprow) { rv=PNGD_E_NOMEM; goto abort; }

		for(j=0;j<(int)height;j++) {
			CopyMemory(tmprow, row_pointers[j], (width+3)/4 );
			ZeroMemory(row_pointers[j], (width+1)/2 );

			for(i=0;i<(int)width;i++) {
				row_pointers[j][i/2] |= 
					( ((tmprow[i/4] >> (2*(3-i%4)) ) & 0x03)<< (4*(1-i%2)) );
			}
		}
		free((void*)tmprow);
	}

	free((void*)row_pointers);
	row_pointers=NULL;

	png_read_end(png_ptr, info_ptr);

	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
	png_ptr=NULL;

	if(p2d->input_method==0) {
		fclose(fp);
		fp=NULL;
	}

	// fill in the DIB header fields
	p2d->pdib->biSize=          sizeof(BITMAPINFOHEADER);
	p2d->pdib->biWidth=         width;
	p2d->pdib->biHeight=        height;
	p2d->pdib->biPlanes=        1;
	p2d->pdib->biBitCount=      dib_bpp;
	p2d->pdib->biCompression=   write_bitfields?BI_BITFIELDS:BI_RGB;
	// biSizeImage can also be 0 in uncompressed bitmaps
	p2d->pdib->biSizeImage=     height*dib_bytesperrow;

	if(has_phys) {
		if(res_unit_type==1) {
			p2d->pdib->biXPelsPerMeter= res_x;
			p2d->pdib->biYPelsPerMeter= res_y;
		}
	}
	p2d->pdib->biClrUsed=       palette_entries;
	p2d->pdib->biClrImportant=  0;

	if(has_bkgd || (p2d->use_custom_bg_flag)) {
		// return the background color if one was used
		p2d->bgcolor.red   = bkgd_color.red;
		p2d->bgcolor.green = bkgd_color.green;
		p2d->bgcolor.blue  = bkgd_color.blue;
		p2d->bgcolor_returned=1;
	}

	return PNGD_E_SUCCESS;

abort:

	if(png_ptr) png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
	if(p2d->input_method==0 && fp) fclose(fp);
	if(row_pointers) free((void*)row_pointers);
	if(lpdib) {
		pngdib_p2d_free_dib((PNGDIB*)p2d,NULL);
	}

	// If we don't have an error message yet, use a
	// default one based on the code
	if(!lstrlen(p2d->common.errmsg)) {
		pngd_get_error_message(rv,p2d->common.errmsg,PNGDIB_ERRMSG_MAX);
	}

	return rv;
}

void PNGDIB_DECL pngdib_p2d_free_dib(PNGDIB *qq, BITMAPINFOHEADER* pdib)
{
	struct p2d_struct *p2d;

	if(!qq) {
		if(pdib) free((void*)pdib);
		return;
	}

	p2d=(struct p2d_struct*)qq;
	if(!pdib) {
		// DIB not explicitly given; use the one from the PNGDIB object.
		// (this is the normal case)
		pdib = p2d->pdib;
		p2d->pdib = NULL;
	}
	if(pdib) {
		if(p2d->common.free_function) {
			(*(p2d->common.free_function))(p2d->common.userdata,(void*)pdib);
		}
		else {
			free((void*)pdib);
		}
	}
}

int PNGDIB_DECL pngdib_d2p_run(PNGDIB *qq)
{
	struct d2p_struct *d2p;
	png_structp png_ptr;
	png_infop info_ptr;
	jmp_buf jbuf;
	struct errstruct errinfo;
	png_text text_ptr[1];
	unsigned char *bits;
	unsigned char *newimage;
	RGBQUAD* dib_palette;
	int headersize, dib_bpp;
	int png_color_type;
	png_uint_32 res_x, res_y;
	png_color_8 pngc8;
	int height,width;
	int palette_entries;
	int topdown;
	int dib_bytesperrow;
	int compression;
	FILE *fp;
	png_color png_palette[256];
	unsigned char **row_pointers;
	int i,x,y,size;
	DWORD *bitfields;
	unsigned int v;
	int bf_format;   // bitfields format identifier
	int iscompressed;
	int bfsize; // bytes in "bitfields" section
	int palsize;  // bytes in palette

	int palentsize;  // bytes in a palette entry: 3 or 4;
	BITMAPCOREHEADER *lpolddib;
	int rv;  // return code
	int dib_alpha32;

	d2p=(struct d2p_struct*)qq;

	rv=PNGD_E_ERROR;  // this should always get changed before returning
	png_ptr=NULL;
	info_ptr=NULL;
	fp=NULL;
	row_pointers=NULL;
	newimage=NULL;
	dib_alpha32=0;

	StringCchCopy(d2p->common.errmsg,PNGDIB_ERRMSG_MAX,_T(""));

	if(!d2p->output_filename) {
		StringCchCopy(d2p->common.errmsg,PNGDIB_ERRMSG_MAX,_T("Output filename not set"));
		rv=PNGD_E_ERROR; goto abort;
	}

	if(!d2p->pdib) {
		StringCchCopy(d2p->common.errmsg,PNGDIB_ERRMSG_MAX,_T("Input DIB not set"));
		rv=PNGD_E_ERROR; goto abort;
	}


	headersize= d2p->pdib->biSize;

	if(headersize<40 && headersize!=12) {
		StringCchPrintf(d2p->common.errmsg,PNGDIB_ERRMSG_MAX,_T("Unexpected BMP header size (%d)"),headersize);
		rv=PNGD_E_BADBMP; goto abort;
	}

	if(headersize==12) {
		// This is to support an old kind of DIBs (OS/2) that aren't really
		// used anymore.
		palentsize= 3;
		lpolddib= (BITMAPCOREHEADER*) d2p->pdib;
		width= lpolddib->bcWidth;
		height= lpolddib->bcHeight;
		dib_bpp= lpolddib->bcBitCount;
		compression = BI_RGB;
		res_x = res_y = 0;

		// This will get adjusted later if there is a palette.
		// Not sure it's right, though. Do old DIBs always have a
		// full-sized palette?
		palette_entries=0;
	}
	else {
		palentsize=4;
		width            = d2p->pdib->biWidth;
		height           = d2p->pdib->biHeight;
		dib_bpp          = d2p->pdib->biBitCount;
		compression      = d2p->pdib->biCompression;
		res_x            = d2p->pdib->biXPelsPerMeter;
		res_y            = d2p->pdib->biYPelsPerMeter;
		palette_entries  = d2p->pdib->biClrUsed;
	}

	// supposedly, if the height is negative, the top scanline is stored first
	topdown=0;
	if(height<0) {
		height= -height;
		topdown=1;
	}

	// sanity check
	if(height<1 || height>1000000 || width<1 || width>1000000) {
		StringCchPrintf(d2p->common.errmsg,PNGDIB_ERRMSG_MAX,_T("Unreasonable image dimensions (%dx%d)"),width,height);
		rv=PNGD_E_BADBMP; goto abort;
	}

	if(dib_bpp==32 && (d2p->common.dib_alpha32)) {
		dib_alpha32=1;
	}

	// only certain combinations of compression and bpp are allowed
	switch(compression) {
	case BI_RGB: 
		if(dib_bpp!=1 && dib_bpp!=4 && dib_bpp!=8 && dib_bpp!=16
			&& dib_bpp!=24 && dib_bpp!=32)
		{
			StringCchPrintf(d2p->common.errmsg,PNGDIB_ERRMSG_MAX,_T("Unsupported bit depth (%d)"),dib_bpp);
			rv=PNGD_E_UNSUPP; goto abort;
		}
		break;
	case BI_RLE4:
		if(dib_bpp!=4) {
			rv=PNGD_E_UNSUPP; goto abort;
		}
		break;
	case BI_RLE8:
		if(dib_bpp!=8) {
			rv=PNGD_E_UNSUPP; goto abort;
		}
		break;
	case BI_BITFIELDS:
		if(dib_bpp!=16 && dib_bpp!=32) {
			rv=PNGD_E_UNSUPP; goto abort;
		}
		break;
	default:
		StringCchCopy(d2p->common.errmsg,PNGDIB_ERRMSG_MAX,_T("Unsupported compression scheme"));
		return PNGD_E_UNSUPP;
	}

	iscompressed= (compression==BI_RLE4 || compression==BI_RLE8);

	// uncompressed dibs are padded to 4-byte bondaries
	dib_bytesperrow= (((width * dib_bpp)+31)/32)*4;

	if(dib_bpp<16) {
		if(palette_entries==0) palette_entries = 1<<dib_bpp;
	}

	bfsize = (compression==BI_BITFIELDS)?12:0;

	palsize=palentsize*palette_entries;

	// bounds check
	size= headersize + bfsize + palsize;
	if(d2p->dibsize) {
		if(size>d2p->dibsize) {
			rv=PNGD_E_BADBMP; goto abort;
		}
	}

	if(d2p->pbits) {
		if(d2p->bitssize && !iscompressed) {   // bounds check
			size=dib_bytesperrow*height;
			if(size>d2p->bitssize) { rv=PNGD_E_BADBMP; goto abort; }
		}

		bits=(unsigned char*)d2p->pbits;
	}
	else {
		// If not provided by user, assume the bits immediately
		// follow the palette.

		if(d2p->dibsize && !iscompressed) {  // bounds check
			size= headersize+bfsize+palsize+dib_bytesperrow*height;
			if(size>d2p->dibsize) { rv=PNGD_E_BADBMP; goto abort; }
		}

		bits= &((unsigned char*)(d2p->pdib))[headersize+bfsize+palsize];
	}

	bitfields   =  (DWORD*)  ( &((unsigned char*)(d2p->pdib))[headersize] );
	dib_palette = (RGBQUAD*) ( &((unsigned char*)(d2p->pdib))[headersize+bfsize] );

	bf_format=0;
	if(compression==BI_BITFIELDS) {
		if(dib_bpp==16) {
			if     (bitfields[0]==0x00007c00 && bitfields[1]==0x000003e0 && 
			        bitfields[2]==0x0000001f) bf_format=11;  // 555
			else if(bitfields[0]==0x0000f800 && bitfields[1]==0x000007e0 &&
			        bitfields[2]==0x0000001f) bf_format=12;  // 565
			else { rv=PNGD_E_UNSUPP; goto abort; }
		}
		if(dib_bpp==32) {
			if     (bitfields[0]==0x00ff0000 && bitfields[1]==0x0000ff00 &&
			        bitfields[2]==0x000000ff) bf_format=21;
			else { rv=PNGD_E_UNSUPP; goto abort; }
		}
	}

	if(bf_format==0 && dib_bpp==16) bf_format=10;
	if(bf_format==0 && dib_bpp==32) bf_format=20;


	// Done analyzing the DIB, now time to convert it to PNG

	// FIXME: this probably isn't quite right
	// jbuf: see comments in read_png_to_dib()
	errinfo.jbufp = &jbuf;
	errinfo.errmsg = d2p->common.errmsg;
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, (void*)(&errinfo),
	    my_png_error_fn, my_png_warning_fn);
	if (!png_ptr) { rv=PNGD_E_NOMEM; goto abort; }

	if(d2p->common.pngptrhook_function) {
		(*(d2p->common.pngptrhook_function))(d2p->common.userdata,(void*)png_ptr);
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		//png_destroy_write_struct(&png_ptr,(png_infopp)NULL);
		rv=PNGD_E_NOMEM; goto abort;
	}

	if(setjmp(jbuf)) {
		// we'll get here if an error occurred in any of the following
		// png_ functions
		rv=PNGD_E_LIBPNG;
		goto abort;
	}

	fp= _tfopen(d2p->output_filename,_T("wb"));
	if(!fp) {
		rv=PNGD_E_WRITE;
		goto abort;
	}

	png_init_io(png_ptr, fp);

	if(dib_alpha32) {
		png_color_type = PNG_COLOR_TYPE_RGB_ALPHA;
	}
	else {
		png_color_type = (dib_bpp>8)?PNG_COLOR_TYPE_RGB:PNG_COLOR_TYPE_PALETTE;
	}

	png_set_IHDR(png_ptr, info_ptr, width, height, (dib_bpp>8)?8:dib_bpp,
		png_color_type,
		(d2p->interlaced)?PNG_INTERLACE_ADAM7:PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	// write sRGB and gAMA chunks
	if(d2p->file_gamma_valid) {
		if(d2p->file_gamma>0.454539 && d2p->file_gamma<0.454551) {
			png_set_sRGB(png_ptr, info_ptr, PNG_sRGB_INTENT_RELATIVE);
		}
		png_set_gAMA(png_ptr, info_ptr, d2p->file_gamma);
	}

	// For 16-bit DIBs, we get to write an sBIT chunk.
	if(bf_format==10 || bf_format==11) {
		pngc8.red= 5;  pngc8.green= 5;  pngc8.blue= 5;
		png_set_sBIT(png_ptr, info_ptr, &pngc8);
	}
	if(bf_format==12) {
		pngc8.red= 5;  pngc8.green= 6;  pngc8.blue= 5;
		png_set_sBIT(png_ptr, info_ptr, &pngc8);
	}

	// pHYs
	if(res_x>0 && res_y>0)
		png_set_pHYs(png_ptr, info_ptr, res_x, res_y, 1);


	if(palette_entries>0) {
		for(i=0;i<palette_entries;i++) {
			if(palentsize==3) {
				png_palette[i].red   = ((RGBTRIPLE*)dib_palette)[i].rgbtRed;
				png_palette[i].green = ((RGBTRIPLE*)dib_palette)[i].rgbtGreen;
				png_palette[i].blue  = ((RGBTRIPLE*)dib_palette)[i].rgbtBlue;
			}
			else {
				png_palette[i].red   = dib_palette[i].rgbRed;
				png_palette[i].green = dib_palette[i].rgbGreen;
				png_palette[i].blue  = dib_palette[i].rgbBlue;
			}
		}
		png_set_PLTE(png_ptr, info_ptr, png_palette, palette_entries);
	}

	if(dib_bpp>8)
		png_set_bgr(png_ptr);

	png_write_info(png_ptr, info_ptr);

	row_pointers=(unsigned char**)malloc(height*sizeof(unsigned char*));
	if(!row_pointers) { rv=PNGD_E_NOMEM; goto abort; }


	if(dib_bpp==16 || (dib_bpp==32 && !dib_alpha32)) {

		// Special handling for these bit depths.
		// This uses a lot of memory, and could be improved by processing
		// one line at a time (but that makes it tricky to write interlaced
		// images).

		newimage=(unsigned char*)malloc(height*width*3);
		if(!newimage) { rv=PNGD_E_NOMEM; goto abort; }

		for(y=0;y<height;y++) {
			for(x=0;x<width;x++) {
				switch(bf_format) {
				case 10:  case 11:  // 16-bit, format 555 (xRRRRRGG GGGBBBBB)
					v= bits[y*dib_bytesperrow+x*2+0] | (bits[y*dib_bytesperrow+x*2+1]<<8);
					newimage[(y*width+x)*3+0]= (v & 0x0000001f)<<3 | (v & 0x0000001f)>>2;  // blue
					newimage[(y*width+x)*3+1]= (v & 0x000003e0)>>2 | (v & 0x000003e0)>>7;  // green
					newimage[(y*width+x)*3+2]= (v & 0x00007c00)>>7 | (v & 0x00007c00)>>12; // red
					break;
				case 12:            // 16-bit, format 565 (RRRRRGGG GGGBBBBB)
					v= bits[y*dib_bytesperrow+x*2+0] | (bits[y*dib_bytesperrow+x*2+1]<<8);
					newimage[(y*width+x)*3+0]= (v & 0x0000001f)<<3 | (v & 0x0000001f)>>2;  // blue
					newimage[(y*width+x)*3+1]= (v & 0x000007e0)>>3 | (v & 0x000007e0)>>9;  // green
					newimage[(y*width+x)*3+2]= (v & 0x0000f800)>>8 | (v & 0x0000f800)>>13; // red
					break;
				case 20:  case 21:  // 32-bit, every 4th byte wasted (b g r x)
					newimage[(y*width+x)*3+0]= bits[y*dib_bytesperrow+x*4+0]; // blue
					newimage[(y*width+x)*3+1]= bits[y*dib_bytesperrow+x*4+1]; // green
					newimage[(y*width+x)*3+2]= bits[y*dib_bytesperrow+x*4+2]; // red
					break;
				}
			}
		}

		for(i=0;i<height;i++) {
			if(topdown) 
				row_pointers[i]= &newimage[i*width*3];
			else
				row_pointers[height-1-i]= &newimage[i*width*3];
		}
		png_write_image(png_ptr, row_pointers);

		free(newimage);
		newimage=NULL;
	}
	else if(iscompressed) {
		newimage= uncompress_dib((LPBITMAPINFO)d2p->pdib, headersize+bfsize+palsize, bits);
		if(!newimage) { rv=PNGD_E_NOMEM; goto abort; }
		for(i=0;i<height;i++) {
			if(topdown) 
				row_pointers[i]= &newimage[i*dib_bytesperrow];
			else
				row_pointers[height-1-i]= &newimage[i*dib_bytesperrow];
		}
		png_write_image(png_ptr, row_pointers);

		free(newimage);
		newimage=NULL;
	}
	else {
		for(i=0;i<height;i++) {
			if(topdown) 
				row_pointers[i]= &bits[i*dib_bytesperrow];
			else
				row_pointers[height-1-i]= &bits[i*dib_bytesperrow];
		}
		png_write_image(png_ptr, row_pointers);
	}

	free((VOID*)row_pointers);
	row_pointers=NULL;

	if(d2p->software_id_string) {
		text_ptr[0].key = "Software";
		text_ptr[0].text = d2p->software_id_string;
		text_ptr[0].compression = PNG_TEXT_COMPRESSION_NONE;
		png_set_text(png_ptr, info_ptr, text_ptr, 1);
	}

	png_write_end(png_ptr, info_ptr);

	rv=PNGD_E_SUCCESS;

abort:
	if(png_ptr) png_destroy_write_struct(&png_ptr, &info_ptr);
	if(fp) fclose(fp);
	if(row_pointers) free(row_pointers);
	if(newimage) free(newimage);

	// If we don't have an error message yet, use a
	// default one based on the code
	if(!lstrlen(d2p->common.errmsg)) {
		pngd_get_error_message(rv,d2p->common.errmsg,PNGDIB_ERRMSG_MAX);
	}
	return rv;
}

TCHAR* PNGDIB_DECL pngdib_get_version_string(void)
{
	return PNGDIB_SRC_VERSION_STRING;
}

int PNGDIB_DECL pngdib_get_version(void)
{
	return PNGDIB_SRC_VERSION;
}


PNGDIB* PNGDIB_DECL _pngdib_init(int structtype, int caller_header_vers)
{
	PNGDIB *qq = NULL;

	if(structtype==PNGD_ST_D2P) {
		struct d2p_struct *d2p;

		d2p = (struct d2p_struct *)calloc(sizeof(struct d2p_struct),1);
		if(d2p) {
			d2p->common.structtype = PNGD_ST_D2P;
			d2p->file_gamma_valid = 1;
			d2p->file_gamma = PNGDIB_DEFAULT_FILE_GAMMA;
		}
		qq=(PNGDIB*)d2p;

	}
	else if(structtype==PNGD_ST_P2D) {
		struct p2d_struct *p2d;

		p2d = (struct p2d_struct *)calloc(sizeof(struct p2d_struct),1);
		if(p2d) {
			p2d->common.structtype = PNGD_ST_P2D;
		}
		qq=(PNGDIB*)p2d;
	}

	// initialize common fields:
	if(qq) {
		qq->errmsg = calloc(PNGDIB_ERRMSG_MAX,sizeof(TCHAR));
	}

	return qq;
}


int PNGDIB_DECL pngdib_d2p_set_dib(PNGDIB *qq,
      const BITMAPINFOHEADER* lpdib, int dibsize,
	  const void* lpbits,             int bitssize)
{
	struct d2p_struct *d2p;
	if(qq->structtype!=PNGD_ST_D2P) return 0;
	d2p=(struct d2p_struct*)qq;

	d2p->pdib = lpdib;
	d2p->dibsize = dibsize;
	d2p->pbits = lpbits;
	d2p->bitssize = bitssize;
	return 1;
}

void PNGDIB_DECL pngdib_d2p_set_interlace(PNGDIB *qq, int interlaced)
{
	struct d2p_struct *d2p;
	d2p=(struct d2p_struct*)qq;
	d2p->interlaced = interlaced;
}

int PNGDIB_DECL pngdib_d2p_set_png_filename(PNGDIB *qq, const TCHAR *fn)
{
	struct d2p_struct *d2p;
	d2p=(struct d2p_struct*)qq;
	d2p->output_filename = _tcsdup(fn);
	return (d2p->output_filename)?1:0;
}

int PNGDIB_DECL pngdib_d2p_set_software_id(PNGDIB *qq, const TCHAR *s)
{
	struct d2p_struct *d2p;
	int len;

	d2p=(struct d2p_struct*)qq;
	len= lstrlen(s);

	// The software id is never stored as UNICODE.
#ifdef _UNICODE
	d2p->software_id_string = calloc(len+10,1);
	StringCchPrintfA(d2p->software_id_string,len+10,"%S",s);
#else
	d2p->software_id_string = _strdup(s);
#endif

	return (d2p->output_filename)?1:0;
}


void PNGDIB_DECL pngdib_d2p_set_gamma_label(PNGDIB *qq, int flag, double file_gamma)
{
	struct d2p_struct *d2p;
	d2p=(struct d2p_struct*)qq;
	d2p->file_gamma_valid = flag;
	d2p->file_gamma = file_gamma;
}

int PNGDIB_DECL pngdib_done(PNGDIB *qq)
{
	struct d2p_struct *d2p;
	struct p2d_struct *p2d;

	if(!qq) return 0;

	if(qq->errmsg) free(qq->errmsg);

	switch(qq->structtype) {
	case PNGD_ST_D2P:
		d2p=(struct d2p_struct*)qq;
		if(d2p->output_filename) free(d2p->output_filename);
		if(d2p->software_id_string) free(d2p->software_id_string);
		free(d2p);
		return 1;
	case PNGD_ST_P2D:
		p2d=(struct p2d_struct*)qq;
		if(p2d->input_filename) free(p2d->input_filename);
		free(p2d);
		return 1;
	}
	return 0;
}

void PNGDIB_DECL pngdib_setcallback_malloc(PNGDIB *qq,
   pngdib_malloc_cb_type   mallocfunc,
   pngdib_free_cb_type     freefunc,
   pngdib_realloc_cb_type  reallocfunc)
{
	qq->malloc_function = mallocfunc;
	qq->free_function = freefunc;
	qq->realloc_function = reallocfunc;
}

void PNGDIB_DECL pngdib_setcallback_pngptrhook(PNGDIB *qq,
        pngdib_pngptrhook_cb_type pngptrhookfn)
{
	qq->pngptrhook_function = pngptrhookfn;
}

TCHAR* PNGDIB_DECL pngdib_get_error_msg(PNGDIB *qq)
{
	return qq->errmsg;
}

void PNGDIB_DECL pngdib_set_userdata(PNGDIB* qq, void* userdata)
{
	qq->userdata = userdata;
}

void* PNGDIB_DECL pngdib_get_userdata(PNGDIB* qq)
{
	return qq->userdata;
}


int PNGDIB_DECL pngdib_p2d_set_png_filename(PNGDIB *qq, const TCHAR *fn)
{
	struct p2d_struct *p2d;
	p2d=(struct p2d_struct*)qq;
	p2d->input_filename = _tcsdup(fn);
	p2d->input_method = 0;
	return (p2d->input_filename)?1:0;
}

void PNGDIB_DECL pngdib_p2d_set_png_memblk(PNGDIB *qq, const void *mem, int memsize)
{
	struct p2d_struct *p2d;
	p2d=(struct p2d_struct*)qq;
	p2d->input_memblk = (unsigned char*)mem;
	p2d->input_method = 1;
	if(memsize>=0)
		p2d->input_memblk_size = memsize;
}

void PNGDIB_DECL pngdib_p2d_set_use_file_bg(PNGDIB *qq, int flag)
{
	struct p2d_struct *p2d;
	p2d=(struct p2d_struct*)qq;
	p2d->use_file_bg_flag = flag;
}


void PNGDIB_DECL pngdib_p2d_set_custom_bg(PNGDIB *qq, unsigned char r,
								  unsigned char g, unsigned char b)
{
	struct p2d_struct *p2d;
	p2d=(struct p2d_struct*)qq;
	p2d->bgcolor.red = r;
	p2d->bgcolor.green = g;
	p2d->bgcolor.blue = b;
	p2d->use_custom_bg_flag = 1;
}

void PNGDIB_DECL pngdib_p2d_set_gamma_correction(PNGDIB *qq, int flag, double screen_gamma)
{
	struct p2d_struct *p2d;
	p2d=(struct p2d_struct*)qq;
	p2d->screen_gamma = screen_gamma;
	p2d->gamma_correction = flag;
}


int PNGDIB_DECL pngdib_p2d_get_dib(PNGDIB *qq,
   BITMAPINFOHEADER **ppdib, int *pdibsize)
{
	struct p2d_struct *p2d;
	p2d=(struct p2d_struct*)qq;
	*ppdib = p2d->pdib;
	if(pdibsize) *pdibsize = p2d->dibsize;
	return 1;
}	

int PNGDIB_DECL pngdib_p2d_get_dibbits(PNGDIB *qq, void **ppbits, int *pbitsoffset, int *pbitssize)
{
	struct p2d_struct *p2d;
	p2d=(struct p2d_struct*)qq;
	*ppbits = p2d->pbits;
	if(pbitsoffset) *pbitsoffset = p2d->bits_offs;
	if(pbitssize) *pbitssize = p2d->bitssize;
	return 1;
}

int PNGDIB_DECL pngdib_p2d_get_palette(PNGDIB *qq, RGBQUAD **ppal, int *ppaloffset, int *ppalnumcolors)
{
	struct p2d_struct *p2d;
	p2d=(struct p2d_struct*)qq;
	if(ppal) *ppal = p2d->palette;
	if(ppaloffset) *ppaloffset = p2d->palette_offs;
	if(ppalnumcolors) *ppalnumcolors = p2d->palette_colors;
	return 1;
}

int PNGDIB_DECL pngdib_p2d_get_colortype(PNGDIB *qq)
{
	struct p2d_struct *p2d;
	p2d=(struct p2d_struct*)qq;
	return p2d->color_type;
}

int PNGDIB_DECL pngdib_p2d_get_bitspersample(PNGDIB *qq)
{
	struct p2d_struct *p2d;
	p2d=(struct p2d_struct*)qq;
	return p2d->bits_per_sample;
}

int PNGDIB_DECL pngdib_p2d_get_bitsperpixel(PNGDIB *qq)
{
	struct p2d_struct *p2d;
	p2d=(struct p2d_struct*)qq;
	return p2d->bits_per_pixel;
}

int PNGDIB_DECL pngdib_p2d_get_samplesperpixel(PNGDIB *qq)
{
	struct p2d_struct *p2d;
	p2d=(struct p2d_struct*)qq;
	return p2d->bits_per_pixel/p2d->bits_per_sample;
}

int PNGDIB_DECL pngdib_p2d_get_interlace(PNGDIB *qq)
{
	struct p2d_struct *p2d;
	p2d=(struct p2d_struct*)qq;
	return p2d->interlace;
}

int PNGDIB_DECL pngdib_p2d_get_density(PNGDIB *qq, int *pres_x, int *pres_y, int *pres_units)
{
	struct p2d_struct *p2d;
	p2d=(struct p2d_struct*)qq;
	if(p2d->res_valid) {
		*pres_x = p2d->res_x;
		*pres_y = p2d->res_y;
		*pres_units = p2d->res_units;
		return 1;
	}
	*pres_x = 1;
	*pres_y = 1;
	*pres_units = 0;
	return 0;
}

int PNGDIB_DECL pngdib_p2d_get_file_gamma(PNGDIB *qq, double *pgamma)
{
	struct p2d_struct *p2d;
	p2d=(struct p2d_struct*)qq;
	if(p2d->gamma_returned) {
		*pgamma = p2d->file_gamma;
		return 1;
	}
	return 0;
}

int PNGDIB_DECL pngdib_p2d_get_bgcolor(PNGDIB *qq, unsigned char *pr, unsigned char *pg, unsigned char *pb)
{
	struct p2d_struct *p2d;
	p2d=(struct p2d_struct*)qq;

	if(p2d->bgcolor_returned) {
		*pr = p2d->bgcolor.red;
		*pg = p2d->bgcolor.green;
		*pb = p2d->bgcolor.blue;
		return 1;
	}
	return 0;
}

void PNGDIB_DECL pngdib_set_dibalpha32(PNGDIB *qq, int flag)
{
	qq->dib_alpha32 = flag;
}


////////////////////////////////////
////////////////////////////////////

#if PNGDIB_V2COMPATIBLE

static void* PNGDIB_DECL globalalloc_callback(void *userdata, int memblksize)
{
	return (void*)GlobalAlloc(GPTR,memblksize);
}

static void PNGDIB_DECL globalfree_callback(void *userdata, void *memblk)
{
	GlobalFree((HGLOBAL)memblk);
}


static void* PNGDIB_DECL heapalloc_callback(void *userdata, int memblksize)
{
	return HeapAlloc((HANDLE)userdata,HEAP_ZERO_MEMORY,memblksize);
}

static void PNGDIB_DECL heapfree_callback(void *userdata, void *memblk)
{
	HeapFree((HANDLE)userdata,0,memblk);
}


int read_png_to_dib(PNGD_P2DINFO *p2dp)
{
	PNGDIB *qq;
	int errcode;
	char *msg;
	HANDLE heap;
	int use_heapalloc;
	int imginfo;

	imginfo=0;
	use_heapalloc=0;


	qq=pngdib_p2d_init();
	if(!qq) return PNGD_E_NOMEM;


	// fields through errmsg must exist
	if(p2dp->structsize<48) return PNGD_E_VERSION;

	// try to be somewhat backward-compatible
	if(p2dp->structsize>=88) {
		imginfo=1;
	}

	if(p2dp->structsize>=96) {
		if(p2dp->flags & PNGD_USE_HEAPALLOC) {
			use_heapalloc=1;
			heap = p2dp->heap;
			if(!heap) heap = GetProcessHeap();
		}
	}

	if(use_heapalloc) {
		pngdib_set_userdata(qq,(void*)heap);
		pngdib_setcallback_malloc(qq,heapalloc_callback,heapfree_callback,NULL);
	}
	else {
		pngdib_setcallback_malloc(qq,globalalloc_callback,globalfree_callback,NULL);
	}

	pngdib_p2d_set_png_filename(qq,p2dp->pngfn);

	pngdib_p2d_set_gamma_correction(qq,(p2dp->flags&PNGD_GAMMA_CORRECTION)?1:0,PNGDIB_DEFAULT_SCREEN_GAMMA);

	if(p2dp->flags&PNGD_USE_BKGD)
		pngdib_p2d_set_use_file_bg(qq,1);


	if(p2dp->flags&PNGD_USE_CUSTOM_BG) {
		pngdib_p2d_set_custom_bg(qq,p2dp->bgcolor.red,
			p2dp->bgcolor.green,p2dp->bgcolor.blue);
	}

	if(p2dp->flags&PNGD_DIB_ALPHA32)
		pngdib_set_dibalpha32(qq,1);

	errcode=pngdib_p2d_run(qq);

	if(!errcode) {

		pngdib_p2d_get_dib(qq,&p2dp->lpdib, &p2dp->dibsize);
		pngdib_p2d_get_dibbits(qq,&p2dp->lpbits, &p2dp->bits_offs, NULL);
		pngdib_p2d_get_palette(qq,&p2dp->palette, &p2dp->palette_offs, &p2dp->palette_colors);

		if(imginfo) {
			p2dp->color_type = pngdib_p2d_get_colortype(qq);
			p2dp->bits_per_sample = pngdib_p2d_get_bitspersample(qq);
			p2dp->interlace = pngdib_p2d_get_interlace(qq);
			p2dp->bits_per_pixel = pngdib_p2d_get_bitsperpixel(qq);
			if(pngdib_p2d_get_file_gamma(qq, &p2dp->file_gamma)) {
				p2dp->flags |= PNGD_GAMMA_RETURNED;
			}
			if(pngdib_p2d_get_density(qq, &p2dp->res_x, &p2dp->res_y, &p2dp->res_units)) {
				p2dp->flags |= PNGD_RES_RETURNED;
			}
			if(pngdib_p2d_get_bgcolor(qq, &p2dp->bgcolor.red, &p2dp->bgcolor.green,
				&p2dp->bgcolor.blue))
			{
				p2dp->flags |= PNGD_BG_RETURNED;
			}
		}
	}

	if(p2dp->errmsg) {
		msg=pngdib_get_error_msg(qq);
		lstrcpyn(p2dp->errmsg,msg,PNGDIB_ERRMSG_MAX);
	}

	pngdib_done(qq);

	return errcode;
}

int write_dib_to_png(PNGD_D2PINFO *d2pp)
{
	PNGDIB *qq;
	int errcode;
	char *msg;

	if(d2pp->structsize != sizeof(PNGD_D2PINFO)) return PNGD_E_VERSION;

	qq=pngdib_d2p_init();
	if(!qq) return PNGD_E_NOMEM;
	pngdib_d2p_set_dib(qq,d2pp->lpdib,d2pp->dibsize,
		d2pp->lpbits,d2pp->bitssize);
	if(d2pp->flags&PNGD_INTERLACED)
		pngdib_d2p_set_interlace(qq,1);

	if(d2pp->flags&PNGD_NO_GAMMA_LABEL)
		pngdib_d2p_set_gamma_label(qq,0,0.0);
	else
		pngdib_d2p_set_gamma_label(qq,1,PNGDIB_DEFAULT_FILE_GAMMA);

	pngdib_d2p_set_png_filename(qq,d2pp->pngfn);

	if(d2pp->software)
		pngdib_d2p_set_software_id(qq, d2pp->software);

	if(d2pp->flags&PNGD_DIB_ALPHA32)
		pngdib_set_dibalpha32(qq,1);

	errcode=pngdib_d2p_run(qq);

	if(d2pp->errmsg) {
		msg=pngdib_get_error_msg(qq);
		lstrcpyn(d2pp->errmsg,msg,PNGDIB_ERRMSG_MAX);
	}

	pngdib_done(qq);
	return errcode;
}

#endif // PNGDIB_V2COMPATIBLE
