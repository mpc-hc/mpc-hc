/*
 *===================================================================
 *  3GPP AMR Wideband Floating-point Speech Codec
 *===================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "typedef.h"
#include "dec_if.h"

#ifndef IF2
#include <string.h>
#define AMRWB_MAGIC_NUMBER "#!AMR-WB\n"
#endif

/*
 * DECODER.C
 *
 * Main program of the AMR WB ACELP wideband decoder.
 *
 *    Usage : decoder bitstream_file synth_file
 *
 *    Format for bitstream_file:
 *        Described in TS26.201
 *
 *    Format for synth_file:
 *      Synthesis is written to a binary file of 16 bits data.
 *
 */

extern const UWord8 block_size[];

int main(int argc, char *argv[])
{
    FILE *f_serial;                        /* File of serial bits for transmission  */
    FILE *f_synth;                         /* File of speech data                   */

    Word16 synth[L_FRAME16k];              /* Buffer for speech @ 16kHz             */
    UWord8 serial[NB_SERIAL_MAX];
    Word16 mode;
    Word32 frame;

#ifndef IF2
	char magic[16];
#endif
    void *st;

    fprintf(stderr, "\n");
	   fprintf(stderr, "===================================================================\n");
	   fprintf(stderr, " 3GPP AMR-WB Floating-point Speech Decoder, v6.0.0, Dec 14, 2004\n");
	   fprintf(stderr, "===================================================================\n");
   fprintf(stderr, "\n");

    /*
     * Read passed arguments and open in/out files
     */
    if (argc != 3)
    {
        fprintf(stderr, "Usage : decoder  bitstream_file  synth_file\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Format for bitstream_file:\n");
#ifdef IF2
		fprintf(stderr, "  Described in TS26.201.\n");
#else
		fprintf(stderr, "  Described in RFC 3267 (Sections 5.1 and 5.3).\n");
#endif
        fprintf(stderr, "\n");
        fprintf(stderr, "Format for synth_file:\n");
        fprintf(stderr, "  Synthesis is written to a binary file of 16 bits data.\n");
        fprintf(stderr, "\n");
        exit(0);
    }

    /* Open file for synthesis and packed serial stream */
    if ((f_serial = fopen(argv[1], "rb")) == NULL)
    {
        fprintf(stderr, "Input file '%s' does not exist !!\n", argv[1]);
        exit(0);
    }
    else
    {
        fprintf(stderr, "Input bitstream file:   %s\n", argv[1]);
    }

    if ((f_synth = fopen(argv[2], "wb")) == NULL)
    {
        fprintf(stderr, "Cannot open file '%s' !!\n", argv[2]);
        exit(0);
    }
    else
    {
        fprintf(stderr, "Synthesis speech file:   %s\n", argv[2]);
    }

    /*
     * Initialization of decoder
     */
    st = D_IF_init();

#ifndef IF2
   /* read magic number */
   fread(magic, sizeof(char), strlen(AMRWB_MAGIC_NUMBER), f_serial);

   /* verify magic number */
   if (strncmp(magic, AMRWB_MAGIC_NUMBER, strlen(AMRWB_MAGIC_NUMBER)))
   {
	   fprintf(stderr, "%s%s\n", "Invalid magic number: ", magic);
	   fclose(f_serial);
	   fclose(f_synth);
	   exit(0);
   }
#endif

    /*
     * Loop for each "L_FRAME" speech data
     */
    fprintf(stderr, "\n --- Running ---\n");

    frame = 0;
    while (fread(serial, sizeof (UWord8), 1, f_serial ) > 0)
    {
#ifdef IF2
       mode = (Word16)(serial[0] >> 4);
#else
	   mode = (Word16)((serial[0] >> 3) & 0x0F);
#endif
	   fread(&serial[1], sizeof (UWord8), block_size[mode] - 1, f_serial );

	   frame++;

	   fprintf(stderr, " Decoding frame: %ld\r", frame);

	   D_IF_decode( st, serial, synth, _good_frame);

	   fwrite(synth, sizeof(Word16), L_FRAME16k, f_synth);
	   fflush(f_synth);
    }

    D_IF_exit(st);

    fclose(f_serial);
    fclose(f_synth);

    return 0;
}
