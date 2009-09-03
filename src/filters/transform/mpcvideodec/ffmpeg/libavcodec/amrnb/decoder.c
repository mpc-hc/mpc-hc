/*
 * ===================================================================
 *  TS 26.104
 *  REL-5 V5.4.0 2004-03
 *  REL-6 V6.1.0 2004-03
 *  3GPP AMR Floating-point Speech Codec
 * ===================================================================
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "interf_dec.h"
#include "sp_dec.h"
#include "typedef.h"

#ifndef ETSI
#ifndef IF2
#include <string.h>
#define AMR_MAGIC_NUMBER "#!AMR\n"
#endif
#endif

void Copyright(void){
fprintf (stderr,
"===================================================================\n"
" TS 26.104                                                         \n"
" REL-5 V5.4.0 2004-03                                              \n"
" REL-6 V6.1.0 2004-03                                              \n"
" 3GPP AMR Floating-point Speech Decoder                            \n"
"===================================================================\n"
);
}
/*
 * main
 *
 *
 * Function:
 *    Speech decoder main program
 *
 *    Usage: decoder bitstream_file synthesis_file
 *
 *    Format for ETSI bitstream file:
 *       1 word (2-byte) for the TX frame type
 *       244 words (2-byte) containing 244 bits.
 *          Bit 0 = 0x0000 and Bit 1 = 0x0001
 *       1 word (2-byte) for the mode indication
 *       4 words for future use, currently written as zero
 *
 *    Format for 3GPP bitstream file:
 *       Holds mode information and bits packed to octets.
 *       Size is from 1 byte to 31 bytes.
 *
 *    Format for synthesis_file:
 *       Speech is written to a 16 bit 8kHz file.
 *
 *    ETSI bitstream file format is defined using ETSI as preprocessor
 *    definition
 * Returns:
 *    0
 */
int main (int argc, char * argv[]){

   FILE * file_speech, *file_analysis;

   short synth[160];
   int frames = 0;
   int * destate;
   int read_size;
#ifndef ETSI
   unsigned char analysis[32];
   enum Mode dec_mode;
#ifdef IF2
   short block_size[16]={ 12, 13, 15, 17, 18, 20, 25, 30, 5, 0, 0, 0, 0, 0, 0, 0 };
#else
   char magic[8];
   short block_size[16]={ 12, 13, 15, 17, 19, 20, 26, 31, 5, 0, 0, 0, 0, 0, 0, 0 };
#endif
#else
   short analysis[250];
#endif

   /* Process command line options */
   if (argc == 3){

      file_speech = fopen(argv[2], "wb");
      if (file_speech == NULL){
         fprintf ( stderr, "%s%s%s\n","Use: ",argv[0], " input.file output.file " );
         return 1;
      }

      file_analysis = fopen(argv[1], "rb");
      if (file_analysis == NULL){
         fprintf ( stderr, "%s%s%s\n","Use: ",argv[0], " input.file output.file " );
         fclose(file_speech);
         return 1;
      }

   }
   else {
      fprintf ( stderr, "%s%s%s\n","Use: ",argv[0], " input.file output.file " );
      return 1;
   }
   Copyright();
   /* init decoder */
   destate = Decoder_Interface_init();

#ifndef ETSI
#ifndef IF2
   /* read and verify magic number */
   fread( magic, sizeof( char ), strlen( AMR_MAGIC_NUMBER ), file_analysis );
   if ( strncmp( magic, AMR_MAGIC_NUMBER, strlen( AMR_MAGIC_NUMBER ) ) ) {
	   fprintf( stderr, "%s%s\n", "Invalid magic number: ", magic );
	   fclose( file_speech );
	   fclose( file_analysis );
	   return 1;
   }
#endif
#endif

#ifndef ETSI

   /* find mode, read file */
   while (fread(analysis, sizeof (unsigned char), 1, file_analysis ) > 0)
   {
#ifdef IF2
      dec_mode = analysis[0] & 0x000F;
#else
      dec_mode = (analysis[0] >> 3) & 0x000F;
#endif
	  read_size = block_size[dec_mode];

      fread(&analysis[1], sizeof (char), read_size, file_analysis );
#else

   read_size = 250;
   /* read file */
   while (fread(analysis, sizeof (short), read_size, file_analysis ) > 0)
   {
#endif

      frames ++;

      /* call decoder */
      Decoder_Interface_Decode(destate, analysis, synth, 0);

      fwrite( synth, sizeof (short), 160, file_speech );
   }

   Decoder_Interface_exit(destate);

   fclose(file_speech);
   fclose(file_analysis);
   fprintf ( stderr, "\n%s%i%s\n","Decoded ", frames, " frames.");

   return 0;
}
