/*
 *===================================================================
 *  3GPP AMR Wideband Floating-point Speech Codec
 *===================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "typedef.h"
#include "enc_if.h"

#ifndef IF2
#define AMRWB_MAGIC_NUMBER "#!AMR-WB\n"
#endif

/*
 * ENCODER.C
 *
 *    Usage : encoder (-dtx) mode speech_file  bitstream_file
 *
 *    Format for speech_file:
 *      Speech is read from a binary file of 16 bits data.
 *
 *    Format for bitstream_file:
 *        Described in TS26.201
 *
 *    mode = 0..8 (bit rate = 6.60 to 23.85 k)
 *
 *    -dtx if DTX is ON
 */

int main(int argc, char *argv[])
{
   FILE *f_speech = NULL;                 /* File of speech data                   */
   FILE *f_serial = NULL;                 /* File of serial bits for transmission  */
   FILE *f_mode = NULL;                   /* File of modes for each frame          */

   Word32 serial_size, frame;
   Word16 signal[L_FRAME16k];             /* Buffer for speech @ 16kHz             */
   Word16 coding_mode = 0, allow_dtx, mode_file, mode = 0;
   UWord8 serial[NB_SERIAL_MAX];
   void *st;

   fprintf(stderr, "\n");
   fprintf(stderr, "===================================================================\n");
   fprintf(stderr, " 3GPP AMR-WB Floating-point Speech Coder, v6.0.0, Dec 14, 2004\n");
   fprintf(stderr, "===================================================================\n");
   fprintf(stderr, "\n");

   /*
    * Open speech file and result file (output serial bit stream)
    */

   if ((argc < 4) || (argc > 6))
   {
      fprintf(stderr, "Usage : encoder  (-dtx) mode speech_file  bitstream_file\n");
      fprintf(stderr, "\n");
      fprintf(stderr, "Format for speech_file:\n");
      fprintf(stderr, "  Speech is read form a binary file of 16 bits data.\n");
      fprintf(stderr, "\n");
      fprintf(stderr, "Format for bitstream_file:\n");
#ifdef IF2
      fprintf(stderr, "  Described in TS26.201.\n");
#else
      fprintf(stderr, "  Described in RFC 3267 (Sections 5.1 and 5.3).\n");
#endif
      fprintf(stderr, "\n");
      fprintf(stderr, "mode: 0 to 8 (9 bits rates) or\n");
      fprintf(stderr, "      -modefile filename\n");
      fprintf(stderr, " ===================================================================\n");
      fprintf(stderr, " mode   :  (0)  (1)   (2)   (3)   (4)   (5)   (6)   (7)   (8)     \n");
      fprintf(stderr, " bitrate: 6.60 8.85 12.65 14.25 15.85 18.25 19.85 23.05 23.85 kbit/s\n");
      fprintf(stderr, " ===================================================================\n");
      fprintf(stderr, "\n");
      fprintf(stderr, "-dtx if DTX is ON, default is OFF\n");
      fprintf(stderr, "\n");
      exit(0);
   }
   allow_dtx = 0;
   if (strcmp(argv[1], "-dtx") == 0)
   {
      allow_dtx = 1;
      argv++;
   }
   mode_file = 0;
   if (strcmp(argv[1], "-modefile") == 0)
   {
      mode_file = 1;
      argv++;
      if ((f_mode = fopen(argv[1], "r")) == NULL)
      {
         fprintf(stderr, "Error opening input file  %s !!\n", argv[1]);
         exit(0);
      }
      fprintf(stderr, "Mode file:  %s\n", argv[1]);
   }
   else
   {
      mode = (Word16) atoi(argv[1]);
      if ((mode < 0) || (mode > 8))
      {
         fprintf(stderr, " error in bit rate mode %d: use 0 to 8\n", mode);
         exit(0);
      }
   }

   if ((f_speech = fopen(argv[2], "rb")) == NULL)
   {
      fprintf(stderr, "Error opening input file  %s !!\n", argv[2]);
      exit(0);
   }
   fprintf(stderr, "Input speech file:  %s\n", argv[2]);

   if ((f_serial = fopen(argv[3], "wb")) == NULL)
   {
      fprintf(stderr, "Error opening output bitstream file %s !!\n", argv[3]);
      exit(0);
   }
   fprintf(stderr, "Output bitstream file:  %s\n", argv[3]);

   /*
    * Initialisation
    */

   st = E_IF_init();

#ifndef IF2

   /* If MMS output is selected, write the magic number at the beginning of the
    * bitstream file
	*/

	fwrite(AMRWB_MAGIC_NUMBER, sizeof(char), strlen(AMRWB_MAGIC_NUMBER), f_serial);

#endif

   /*
    * Loop for every analysis/transmission frame.
    *   -New L_FRAME data are read. (L_FRAME = number of speech data per frame)
    *   -Conversion of the speech data from 16 bit integer to real
    *   -Call coder to encode the speech.
    *   -The compressed serial output stream is written to a file.
    */

   fprintf(stderr, "\n --- Running ---\n");

   frame = 0;

   while (fread(signal, sizeof(Word16), L_FRAME16k, f_speech) == L_FRAME16k)
   {
      if (mode_file)
      {
         if (fscanf(f_mode, "%hd", &mode) == EOF)
         {
            mode = coding_mode;
            fprintf(stderr, "\n end of mode control file reached\n");
            fprintf(stderr, " From now on using mode: %hd.\n", mode);
            mode_file = 0;
         }

         if ((mode < 0) || (mode > 8))
         {
            fprintf(stderr, " error in bit rate mode %hd: use 0 to 8\n", mode);
            E_IF_exit(st);
            fclose(f_speech);
            fclose(f_serial);
            fclose(f_mode);
            exit(0);
         }
      }

      coding_mode = mode;

      frame++;
      fprintf(stderr, " Frames processed: %ld\r", frame);

      serial_size = E_IF_encode(st, coding_mode, signal, serial, allow_dtx);

      fwrite(serial, 1, serial_size, f_serial);

   }

   E_IF_exit(st);

   fclose(f_speech);
   fclose(f_serial);

   if (f_mode != NULL)
   {
      fclose(f_mode);
   }

   return 0;
}
