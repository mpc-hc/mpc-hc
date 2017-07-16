// ----------------------------------------------------------------------------
//
//  Copyright (C) 2006-2011 Fons Adriaensen <fons@linuxaudio.org>
//    
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// ----------------------------------------------------------------------------


#ifndef __AUDIOFILE_H
#define __AUDIOFILE_H


#include <stdio.h>
#include <stdint.h>
#include <sndfile.h>
#include "dither.h"


class Audiofile
{
public:

    enum
    {
        MODE_NONE,
	MODE_READ,
	MODE_WRITE
    };

    enum
    {
	TYPE_OTHER,
	TYPE_CAF,
        TYPE_WAV,
        TYPE_AMB
    };

    enum
    {
	FORM_OTHER,
        FORM_16BIT,
	FORM_24BIT,
	FORM_32BIT,
        FORM_FLOAT,
    };

    enum
    {
	DITHER_NONE,
        DITHER_RECT,
	DITHER_TRIA,
	DITHER_LIPS,
    };

    enum
    {
        ERR_NONE    = 0,
	ERR_MODE    = -1,
	ERR_TYPE    = -2,
	ERR_FORM    = -3,
	ERR_OPEN    = -4,
	ERR_SEEK    = -5,
	ERR_DATA    = -6,
	ERR_READ    = -7,
	ERR_WRITE   = -8
    };

    Audiofile (void);
    ~Audiofile (void);

    int mode (void) const { return _mode; }
    int type (void) const { return _type; }
    int form (void) const { return _form; }
    int rate (void) const { return _rate; }
    int chan (void) const { return _chan; }
    uint32_t size (void) const { return _size; }

    int open_read (const char *name);
    int open_write (const char *name, int type, int form, int rate, int chan);
    int set_dither (int type);
    int close (void);

    int seek (uint32_t posit);
    int read  (float *data, uint32_t frames);
    int write (float *data, uint32_t frames);

private:

    enum { BUFFSIZE = 1024 };

    void reset (void);

    SNDFILE  *_sndfile;
    int       _mode;
    int       _type;
    int       _form;
    int       _rate;
    int       _chan;
    uint32_t  _size;
    int       _dith_type;
    Dither   *_dith_proc;
    int16_t  *_dith_buff;
};


#endif

