/*
 *===================================================================
 *  3GPP AMR Wideband Floating-point Speech Codec
 *===================================================================
 */
#include <stdlib.h>
#include <memory.h>
#include "typedef.h"
#include "dec_if.h"
#include "if_rom.h"
#include "dec.h"

#define L_FRAME16k   320            /* Frame size at 16kHz  */
#define MODE_7k       0             /* modes                */
#define MODE_9k       1
#define MODE_12k      2
#define MODE_14k      3
#define MODE_16k      4
#define MODE_18k      5
#define MODE_20k      6
#define MODE_23k      7
#define MODE_24k      8
#define MRDTX        9
#define NUM_OF_MODES 10
#define LOST_FRAME   14
#define MRNO_DATA    15
#define EHF_MASK     (Word16)0x0008 /* homing frame pattern */

typedef struct
{
   Word16 reset_flag_old;     /* previous was homing frame  */
   Word16 prev_ft;            /* previous frame type        */
   Word16 prev_mode;          /* previous mode              */
   void *decoder_state;       /* Points decoder state       */
} WB_dec_if_state;

Word16 nb_of_param_first[NUM_OF_SPMODES]=
{
	9,  14, 15,
	15, 15, 19,
	19, 19, 19
};

extern const Word16 mode_7k[];
extern const Word16 mode_9k[];
extern const Word16 mode_12k[];
extern const Word16 mode_14k[];
extern const Word16 mode_16k[];
extern const Word16 mode_18k[];
extern const Word16 mode_20k[];
extern const Word16 mode_23k[];
extern const Word16 mode_24k[];
extern const Word16 mode_DTX[];

extern const Word16 nb_of_param[];

extern const Word16 dfh_M7k[];
extern const Word16 dfh_M9k[];
extern const Word16 dfh_M12k[];
extern const Word16 dfh_M14k[];
extern const Word16 dfh_M16k[];
extern const Word16 dfh_M18k[];
extern const Word16 dfh_M20k[];
extern const Word16 dfh_M23k[];
extern const Word16 dfh_M24k[];

/* overall table with the parameters of the
   decoder homing frames for all modes */

extern const Word16 *dhf[10];

/*
 * Decoder_Interface_Homing_Frame_test
 *
 * Parameters:
 *    input_frame    I: input parameters
 *    mode           I: speech mode
 *
 * Function:
 *    Check parameters for matching homing frame
 *
 * Returns:
 *    If homing frame
 */
Word16 D_IF_homing_frame_test(Word16 input_frame[], Word16 mode)
{

   if (mode != MODE_24k)
   {
      /* perform test for COMPLETE parameter frame */
      return (Word16)!memcmp(input_frame, dhf[mode], nb_of_param[mode] * sizeof(Word16));
   }
   else
   {
      /* discard high-band energy */
      return (Word16)!(
         (memcmp(input_frame, dhf[MODE_24k], 19 * sizeof(Word16))) |
         (memcmp(input_frame + 20, dhf[MODE_24k] + 20, 11 * sizeof(Word16))) |
         (memcmp(input_frame + 32, dhf[MODE_24k] + 32, 11 * sizeof(Word16))) |
         (memcmp(input_frame + 44, dhf[MODE_24k] + 44, 11 * sizeof(Word16))) );

   }
}


Word16 D_IF_homing_frame_test_first(Word16 input_frame[], Word16 mode)
{
   /* perform test for FIRST SUBFRAME of parameter frame ONLY */
   return (Word16)!memcmp(input_frame, dhf[mode], nb_of_param_first[mode] * sizeof(Word16));
}

#ifdef IF2
/*
 * D_IF_conversion
 *
 *
 * Parameters:
 *    param             O: AMR parameters
 *    stream            I: input bitstream
 *    frame_type        O: frame type
 *    speech_mode       O: speech mode in DTX
 *    fqi               O: frame quality indicator
 *
 * Function:
 *    Unpacks IF2 octet stream
 *
 * Returns:
 *    mode              used mode
 */
Word16 D_IF_conversion(Word16 *param, UWord8 *stream, UWord8 *frame_type,
                       Word16 *speech_mode, Word16 *fqi)
{
   Word32 mode;
   Word32 j;
   Word16 const *mask;

   memset(param, 0, PRMNO_24k << 1);
   mode = *stream >> 4;
   /* SID indication IF2 corresponds to mode 10 */
   if (mode == 9)
   {
      mode ++;
   }

   *fqi = (Word16)((*stream >> 3) & 0x1);
   *stream <<= (HEADER_SIZE - 1);

   switch (mode)
   {
   case MRDTX:
      mask = mode_DTX;

      for (j = HEADER_SIZE; j < T_NBBITS_SID; j++)
      {
         if (*stream & 0x80)
         {
            param[*mask] = (Word16)(param[*mask] + *(mask + 1));
         }

         mask += 2;

         if ( j % 8 )
         {
            *stream <<= 1;
         }
         else
         {
            stream++;
         }
      }

      /* get SID type bit */

      *frame_type = RX_SID_FIRST;

      if (*stream & 0x80)
      {
         *frame_type = RX_SID_UPDATE;
      }

      *stream <<= 1;

      /* speech mode indicator */
      *speech_mode = (Word16)(*stream >> 4);
      break;

   case MRNO_DATA:
      *frame_type = RX_NO_DATA;
      break;

   case LOST_FRAME:
      *frame_type = RX_SPEECH_LOST;
      break;

   case MODE_7k:
      mask = mode_7k;

      for (j = HEADER_SIZE; j < T_NBBITS_7k; j++)
      {
         if ( *stream & 0x80 )
         {
            param[*mask] = (Word16)(param[*mask] + *(mask + 1));
         }
         mask += 2;

         if (j % 8)
         {
            *stream <<= 1;
         }
         else
         {
            stream++;
         }
      }

      *frame_type = RX_SPEECH_GOOD;
      break;

   case MODE_9k:
      mask = mode_9k;

      for (j = HEADER_SIZE; j < T_NBBITS_9k; j++)
      {
         if (*stream & 0x80)
         {
            param[*mask] = (Word16)(param[*mask] + *(mask + 1));
         }
         mask += 2;

         if (j % 8)
         {
            *stream <<= 1;
         }
         else
         {
            stream++;
         }
      }

      *frame_type = RX_SPEECH_GOOD;
      break;

   case MODE_12k:
      mask = mode_12k;

      for (j = HEADER_SIZE; j < T_NBBITS_12k; j++)
      {
         if (*stream & 0x80)
         {
            param[*mask] = (Word16)(param[*mask] + *(mask + 1));
         }
         mask += 2;

         if ( j % 8 )
         {
            *stream <<= 1;
         }
         else
         {
            stream++;
         }
      }

      *frame_type = RX_SPEECH_GOOD;
      break;

   case MODE_14k:
      mask = mode_14k;

      for (j = HEADER_SIZE; j < T_NBBITS_14k; j++)
      {
         if (*stream & 0x80)
         {
            param[*mask] = (Word16)(param[*mask] + *(mask + 1));
         }

         mask += 2;

         if ( j % 8 )
         {
            *stream <<= 1;
         }
         else
         {
            stream++;
         }
      }

      *frame_type = RX_SPEECH_GOOD;
      break;

   case MODE_16k:
      mask = mode_16k;

      for (j = HEADER_SIZE; j < T_NBBITS_16k; j++)
      {
         if (*stream & 0x80)
         {
            param[*mask] = (Word16)(param[*mask] + *(mask + 1));
         }

         mask += 2;

         if (j % 8)
         {
            *stream <<= 1;
         }
         else
         {
            stream++;
         }
      }

      *frame_type = RX_SPEECH_GOOD;
      break;

   case MODE_18k:
      mask = mode_18k;

      for (j = HEADER_SIZE; j < T_NBBITS_18k; j++)
      {
         if (*stream & 0x80)
         {
            param[*mask] = (Word16)(param[*mask] + *(mask + 1));
         }

         mask += 2;

         if (j % 8)
         {
            *stream <<= 1;
         }
         else
         {
            stream++;
         }
      }

      *frame_type = RX_SPEECH_GOOD;
      break;

   case MODE_20k:
      mask = mode_20k;

      for (j = HEADER_SIZE; j < T_NBBITS_20k; j++)
      {
         if (*stream & 0x80)
         {
            param[*mask] = (Word16)(param[*mask] + *(mask + 1));
         }

         mask += 2;

         if (j % 8)
         {
            *stream <<= 1;
         }
         else
         {
            stream++;
         }
      }

      *frame_type = RX_SPEECH_GOOD;
      break;

   case MODE_23k:
      mask = mode_23k;

      for (j = HEADER_SIZE; j < T_NBBITS_23k; j++)
      {
         if (*stream & 0x80)
         {
            param[*mask] = (Word16)(param[*mask] + *(mask + 1));
         }

         mask += 2;

         if (j % 8)
         {
            *stream <<= 1;
         }
         else
         {
            stream++;
         }

      }

      *frame_type = RX_SPEECH_GOOD;
      break;

   case MODE_24k:
      mask = mode_24k;

      for (j = HEADER_SIZE; j < T_NBBITS_24k; j++)
      {
         if (*stream & 0x80)
         {
            param[*mask] = (Word16)(param[*mask] + *(mask + 1));
         }

         mask += 2;

         if (j % 8)
         {
            *stream <<= 1;
         }
         else
         {
            stream++;
         }

      }

      *frame_type = RX_SPEECH_GOOD;
      break;

   default:
      *frame_type = RX_SPEECH_LOST;
      *fqi = 0;
      break;

   }

   if (*fqi == 0)
   {
      if (*frame_type == RX_SPEECH_GOOD)
      {
         *frame_type = RX_SPEECH_BAD;
      }
      if ((*frame_type == RX_SID_FIRST) | (*frame_type == RX_SID_UPDATE))
      {
         *frame_type = RX_SID_BAD;
      }
   }

   return (Word16)mode;
}

#else

/*
 * D_IF_mms_conversion
 *
 *
 * Parameters:
 *    param             O: AMR parameters
 *    stream            I: input bitstream
 *    frame_type        O: frame type
 *    speech_mode       O: speech mode in DTX
 *    fqi               O: frame quality indicator
 *
 * Function:
 *    Unpacks MMS formatted octet stream (see RFC 3267, section 5.3)
 *
 * Returns:
 *    mode              used mode
 */
Word16 D_IF_mms_conversion(Word16 *param, UWord8 *stream, UWord8 *frame_type,
                           Word16 *speech_mode, Word16 *fqi)
{
   Word32 mode;
   Word32 j;
   Word16 const *mask;

   memset(param, 0, PRMNO_24k << 1);

   *fqi = (Word16)((*stream >> 2) & 0x01);
   mode = (Word32)((*stream >> 3) & 0x0F);

   /* SID indication IF2 corresponds to mode 10 */
   if (mode == 9)
   {
      mode ++;
   }

   stream++;

   switch (mode)
   {
   case MRDTX:
      mask = mode_DTX;

      for (j = 1; j <= NBBITS_SID; j++)
      {
         if (*stream & 0x80)
         {
            param[*mask] = (Word16)(param[*mask] + *(mask + 1));
         }

         mask += 2;

         if ( j % 8 )
         {
            *stream <<= 1;
         }
         else
         {
            stream++;
         }
      }

      /* get SID type bit */

      *frame_type = RX_SID_FIRST;

      if (*stream & 0x80)
      {
         *frame_type = RX_SID_UPDATE;
      }

      *stream <<= 1;

      /* speech mode indicator */
      *speech_mode = (Word16)(*stream >> 4);
      break;

   case MRNO_DATA:
      *frame_type = RX_NO_DATA;
      break;

   case LOST_FRAME:
      *frame_type = RX_SPEECH_LOST;
      break;

   case MODE_7k:
      mask = mode_7k;

      for (j = 1; j <= NBBITS_7k; j++)
      {
         if ( *stream & 0x80 )
         {
            param[*mask] = (Word16)(param[*mask] + *(mask + 1));
         }
         mask += 2;

         if (j % 8)
         {
            *stream <<= 1;
         }
         else
         {
            stream++;
         }
      }

      *frame_type = RX_SPEECH_GOOD;
      break;

   case MODE_9k:
      mask = mode_9k;

      for (j = 1; j <= NBBITS_9k; j++)
      {
         if (*stream & 0x80)
         {
            param[*mask] = (Word16)(param[*mask] + *(mask + 1));
         }
         mask += 2;

         if (j % 8)
         {
            *stream <<= 1;
         }
         else
         {
            stream++;
         }
      }

      *frame_type = RX_SPEECH_GOOD;
      break;

   case MODE_12k:
      mask = mode_12k;

      for (j = 1; j <= NBBITS_12k; j++)
      {
         if (*stream & 0x80)
         {
            param[*mask] = (Word16)(param[*mask] + *(mask + 1));
         }
         mask += 2;

         if ( j % 8 )
         {
            *stream <<= 1;
         }
         else
         {
            stream++;
         }
      }

      *frame_type = RX_SPEECH_GOOD;
      break;

   case MODE_14k:
      mask = mode_14k;

      for (j = 1; j <= NBBITS_14k; j++)
      {
         if (*stream & 0x80)
         {
            param[*mask] = (Word16)(param[*mask] + *(mask + 1));
         }

         mask += 2;

         if ( j % 8 )
         {
            *stream <<= 1;
         }
         else
         {
            stream++;
         }
      }

      *frame_type = RX_SPEECH_GOOD;
      break;

   case MODE_16k:
      mask = mode_16k;

      for (j = 1; j <= NBBITS_16k; j++)
      {
         if (*stream & 0x80)
         {
            param[*mask] = (Word16)(param[*mask] + *(mask + 1));
         }

         mask += 2;

         if (j % 8)
         {
            *stream <<= 1;
         }
         else
         {
            stream++;
         }
      }

      *frame_type = RX_SPEECH_GOOD;
      break;

   case MODE_18k:
      mask = mode_18k;

      for (j = 1; j <= NBBITS_18k; j++)
      {
         if (*stream & 0x80)
         {
            param[*mask] = (Word16)(param[*mask] + *(mask + 1));
         }

         mask += 2;

         if (j % 8)
         {
            *stream <<= 1;
         }
         else
         {
            stream++;
         }
      }

      *frame_type = RX_SPEECH_GOOD;
      break;

   case MODE_20k:
      mask = mode_20k;

      for (j = 1; j <= NBBITS_20k; j++)
      {
         if (*stream & 0x80)
         {
            param[*mask] = (Word16)(param[*mask] + *(mask + 1));
         }

         mask += 2;

         if (j % 8)
         {
            *stream <<= 1;
         }
         else
         {
            stream++;
         }
      }

      *frame_type = RX_SPEECH_GOOD;
      break;

   case MODE_23k:
      mask = mode_23k;

      for (j = 1; j <= NBBITS_23k; j++)
      {
         if (*stream & 0x80)
         {
            param[*mask] = (Word16)(param[*mask] + *(mask + 1));
         }

         mask += 2;

         if (j % 8)
         {
            *stream <<= 1;
         }
         else
         {
            stream++;
         }

      }

      *frame_type = RX_SPEECH_GOOD;
      break;

   case MODE_24k:
      mask = mode_24k;

      for (j = 1; j <= NBBITS_24k; j++)
      {
         if (*stream & 0x80)
         {
            param[*mask] = (Word16)(param[*mask] + *(mask + 1));
         }

         mask += 2;

         if (j % 8)
         {
            *stream <<= 1;
         }
         else
         {
            stream++;
         }

      }

      *frame_type = RX_SPEECH_GOOD;
      break;

   default:
      *frame_type = RX_SPEECH_LOST;
      *fqi = 0;
      break;

   }

   if (*fqi == 0)
   {
      if (*frame_type == RX_SPEECH_GOOD)
      {
         *frame_type = RX_SPEECH_BAD;
      }
      if ((*frame_type == RX_SID_FIRST) | (*frame_type == RX_SID_UPDATE))
      {
         *frame_type = RX_SID_BAD;
      }
   }

   return (Word16)mode;
}

#endif

/*
 * D_IF_decode
 *
 *
 * Parameters:
 *    st       B: pointer to state structure
 *    bits     I: bitstream form the encoder
 *    synth    O: decoder output
 *    lfi      I: lost frame indicator
 *                _good_frame, _bad_frame, _lost_frame, _no_frame
 *
 * Function:
 *    Decoding one frame of speech. Lost frame indicator can be used
 *    to inform encoder about the problems in the received frame.
 *    _good_frame:good speech or sid frame is received.
 *    _bad_frame: frame with possible bit errors
 *    _lost_frame:speech of sid frame is lost in transmission
 *    _no_frame:  indicates non-received frames in dtx-operation
 * Returns:
 *
 */
void D_IF_decode( void *st, UWord8 *bits, Word16 *synth, Word32 lfi)
{
   Word32 i;
   Word16 mode = 0;                 /* AMR mode                */
   Word16 speech_mode = MODE_7k;    /* speech mode             */
   Word16 fqi;                      /* frame quality indicator */

   Word16 prm[PRMNO_24k];           /* AMR parameters          */

   UWord8 frame_type;               /* frame type              */
   Word16 reset_flag = 0;           /* reset flag              */
   WB_dec_if_state * s;             /* pointer to structure    */

   s = (WB_dec_if_state*)st;

   /* bits -> param, if needed */
   if ((lfi == _good_frame) | (lfi == _bad_frame))
   {
      /* add fqi data */
#ifdef IF2
      *bits = (UWord8)((Word32)*bits & ~(lfi << 3));
#else
      *bits = (UWord8)((Word32)*bits & ~(lfi << 2));
#endif
      /*
       * extract mode information and frame_type,
       * octets to parameters
       */
#ifdef IF2
      mode = D_IF_conversion( prm, bits, &frame_type, &speech_mode, &fqi);
#else
      mode = D_IF_mms_conversion( prm, bits, &frame_type, &speech_mode, &fqi);
#endif

   }
   else if (lfi == _no_frame)
   {
      frame_type = RX_NO_DATA;
   }
   else
   {
      frame_type = RX_SPEECH_LOST;
   }

   /*
    * if no mode information
    * guess one from the previous frame
    */
   if ((frame_type == RX_SPEECH_LOST) | (frame_type == RX_NO_DATA))
   {
      mode = s->prev_mode;
   }

   if (mode == MRDTX)
   {
      mode = speech_mode;
   }

   /* if homed: check if this frame is another homing frame */
   if (s->reset_flag_old == 1)
   {
      /* only check until end of first subframe */
      reset_flag = D_IF_homing_frame_test_first(prm, mode);
   }

   /* produce encoder homing frame if homed & input=decoder homing frame */
   if ((reset_flag != 0) && (s->reset_flag_old != 0))
   {
      for (i = 0; i < L_FRAME16k; i++)
      {
         synth[i] = EHF_MASK;
      }
   }
   else
   {
      D_MAIN_decode(mode, prm, synth, s->decoder_state, frame_type);
   }

   for (i = 0; i < L_FRAME16k; i++)   /* Delete the 2 LSBs (14-bit input) */
   {
      synth[i] = (Word16) (synth[i] & 0xfffC);
   }

   /* if not homed: check whether current frame is a homing frame */
   if ((s->reset_flag_old == 0) & (mode < 9))
   {
      /* check whole frame */
      reset_flag = D_IF_homing_frame_test(prm, mode);
   }
   /* reset decoder if current frame is a homing frame */
   if (reset_flag != 0)
   {
      D_MAIN_reset(s->decoder_state, 1);
   }
   s->reset_flag_old = reset_flag;

   s->prev_ft = frame_type;
   s->prev_mode = mode;
}

/*
 * D_IF_reset
 *
 * Parameters:
 *    st                O: state struct
 *
 * Function:
 *    Reset homing frame counter
 *
 * Returns:
 *    void
 */
void D_IF_reset(WB_dec_if_state *st)
{
   st->reset_flag_old = 1;
   st->prev_ft = RX_SPEECH_GOOD;
   st->prev_mode = MODE_7k;   /* minimum bitrate */
}

/*
 * D_IF_init
 *
 * Parameters:
 *
 * Function:
 *    Allocates state memory and initializes state memory
 *
 * Returns:
 *    pointer to encoder interface structure
 */
void *D_IF_init( void)
{
   WB_dec_if_state *s = NULL;

   /* allocate memory */
   if ((s = (WB_dec_if_state*) malloc(sizeof(WB_dec_if_state))) == NULL)
   {
      return NULL;
   }

   D_MAIN_init(&(s->decoder_state));
   if (s->decoder_state == NULL)
   {
      free(s);
      return NULL;
   }

   D_IF_reset(s);

   return s;
}

/*
 * D_IF_exit
 *
 * Parameters:
 *    state             I: state structure
 *
 * Function:
 *    The memory used for state memory is freed
 *
 * Returns:
 *    Void
 */
void D_IF_exit(void *state)
{
   WB_dec_if_state *s;

   s = (WB_dec_if_state *)state;

   /* free memory */
   D_MAIN_close(&s->decoder_state);
   free(s);
   state = NULL;
}
