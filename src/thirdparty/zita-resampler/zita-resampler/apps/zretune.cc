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


#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdio.h>
#include <math.h>
#include <zita-resampler/vresampler.h>
#include "audiofile.h"


enum { HELP, CAF, WAV, AMB, BIT16, BIT24, FLOAT, CENT, REC, TRI, LIPS, PAD };
enum { BUFFSIZE = 0x4000, FILTSIZE = 96 };


static unsigned int type = Audiofile::TYPE_WAV;
static unsigned int form = Audiofile::FORM_24BIT;
static double       cent = 0;
static unsigned int dith = Audiofile::DITHER_NONE;
static bool         zpad = false;


static void help (void)
{
    fprintf (stderr, "\nzretune %s\n", VERSION);
    fprintf (stderr, "(C) 2007-2012 Fons Adriaensen  <fons@linuxaudio.org>\n");
    fprintf (stderr, "Usage: zretune <options> <input file> <output file>.\n");
    fprintf (stderr, "Options:\n");
    fprintf (stderr, "  Display this text:     --help\n");
    fprintf (stderr, "  Output file type:      --caf, --wav, --amb\n");
    fprintf (stderr, "  Resampling ratio:      --cent <pitch change>\n");
    fprintf (stderr, "  Output sample format:  --16bit, --24bit, --float\n");
    fprintf (stderr, "  Dither type (16 bit):  --rec, --tri, --lips\n");
    fprintf (stderr, "  Add zero padding :     --pad\n");
    fprintf (stderr, "The default output file format is wav, 24-bit, no dithering.\n");
    fprintf (stderr, "Integer output formats are clipped, float output is not.\n\n");
    exit (1);
}


static struct option options [] = 
{
    { "help",  0, 0, HELP  },
    { "caf",   0, 0, CAF   },
    { "wav",   0, 0, WAV   },
    { "amb",   0, 0, AMB   },
    { "16bit", 0, 0, BIT16 },
    { "24bit", 0, 0, BIT24 },
    { "float", 0, 0, FLOAT },
    { "cent",  1, 0, CENT  },
    { "rec",   0, 0, REC   },
    { "tri",   0, 0, TRI   },
    { "lips",  0, 0, LIPS  },
    { "pad",   0, 0, PAD   },
    { 0, 0, 0, 0 }
};


static void procoptions (int ac, char *av [])
{
    int k;

    while ((k = getopt_long (ac, av, "", options, 0)) != -1)
    {
	switch (k)
	{
        case '?':
	case HELP:
	    help ();
	    break;
	case CAF:
	    type = Audiofile::TYPE_CAF;
	    break;
	case WAV:
	    type = Audiofile::TYPE_WAV;
	    break;
	case AMB:
	    type = Audiofile::TYPE_AMB;
	    break;
	case BIT16:
	    form = Audiofile::FORM_16BIT;
	    break;
	case BIT24:
	    form = Audiofile::FORM_24BIT;
	    break;
	case FLOAT:
	    form = Audiofile::FORM_FLOAT;
	    break;
	case CENT:
	    if (sscanf (optarg, "%lf", &cent) != 1)
	    {
		fprintf (stderr, "Illegal value for --rate option: '%s'.\n", optarg);
		exit (1);
	    }
	    break;
	case REC:
	    dith = Audiofile::DITHER_RECT;
	    break;
	case TRI:
	    dith = Audiofile::DITHER_TRIA;
	    break;
	case LIPS:
	    dith = Audiofile::DITHER_LIPS;
	    break;
	case PAD:
	    zpad = true;
	    break;
 	}
    }
}


int main (int ac, char *av [])
{
    Audiofile     Ainp;
    Audiofile     Aout;
    VResampler    R;
    unsigned int  k, chan, z1, z2;
    float         *inpb, *outb;
    bool          done;
    double        ratio;

    procoptions (ac, av);
    if (ac - optind < 2)
    {
        fprintf (stderr, "Missing arguments, try --help.\n");
	return 1;
    }
    if (ac - optind > 2 )
    {
        fprintf (stderr, "Too many arguments, try --help.\n");
	return 1;
    }

    if (Ainp.open_read (av [optind]))
    {
	fprintf (stderr, "Can't open input file '%s'.\n", av [optind]);
	return 1;
    }

    if ((cent < -1200) || (cent > 1200))
    {
        fprintf (stderr, "Pitch change %3.1lf is out of range.\n", cent);
        Ainp.close ();
        return 1;
    }
    ratio = pow (2.0, -cent / 1200.0);
    R.setup (ratio, Ainp.chan (), FILTSIZE);

    optind++;
    if (Aout.open_write (av [optind], type, form, Ainp.rate(), Ainp.chan ()))
    {
	fprintf (stderr, "Can't open output file '%s'.\n", av [optind]);
	Ainp.close ();
	return 1;
    }
    if (dith != Audiofile::DITHER_NONE) 
    {
	Aout.set_dither (dith);
    }

    if (zpad)
    {
	z1 = R.inpsize () - 1;
	z2 = R.inpsize () - 1;
    }
    else
    {
	z1 = R.inpsize () / 2 - 1;
	z2 = R.inpsize () / 2;
    }

    chan = Ainp.chan ();
    inpb = new float [chan * BUFFSIZE];
    if (cent != 0.0)
    {
        outb = new float [chan * BUFFSIZE];
	// Insert zero samples at start.
        R.inp_count = z1;
        R.inp_data = 0;
        R.out_count = BUFFSIZE;
        R.out_data = outb;
        done = false;
        while (true)
        {
            R.process ();
  	    if (R.inp_count == 0)
	    {
	        // Input buffer empty, read more samples, insert
	        // zeros at the end, or terminate.
	        if (done)
	        {
		    // We already inserted final zero samples.
		    // Write out any remaining output samples and terminate.
                    Aout.write (outb, BUFFSIZE - R.out_count);
		    break;
	        }
                k = Ainp.read (inpb, BUFFSIZE);
	        if (k)
	        {
		    // Process next 'k' input samples.
                    R.inp_count = k;
                    R.inp_data = inpb;
	        }
	        else
	        {
		    // At end of input, insert zero samples.
                    R.inp_count = z2;
                    R.inp_data = 0;
		    done = true;
	        }
	    }
	    if (R.out_count == 0)
	    {
	        // Output buffer full, write to file.
                Aout.write (outb, BUFFSIZE);
                R.out_count = BUFFSIZE;
                R.out_data = outb;
	    }
	}
	delete[] outb;
    }
    else
    {
	// No resampling, just copy.
	while (1)
	{
            k = Ainp.read (inpb, BUFFSIZE);
	    if (k) Aout.write (inpb, k);
	    else break;
	}
    }

    Ainp.close ();
    Aout.close ();
    delete[] inpb;

    return 0;
}
