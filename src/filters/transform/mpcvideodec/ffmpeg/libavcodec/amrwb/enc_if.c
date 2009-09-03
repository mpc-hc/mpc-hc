/*
 *===================================================================
 *  3GPP AMR Wideband Floating-point Speech Codec
 *===================================================================
 */
#include <stdlib.h>
#include <memory.h>
#include "typedef.h"
#include "enc_if.h"
#include "if_rom.h"
#include "enc.h"

#define L_FRAME16k   320            /* Frame size at 16kHz     */
#define EHF_MASK     (Word32)0x0008 /* homing frame pattern    */
#define MODE_7k      0
#define MODE_9k      1
#define MODE_12k     2
#define MODE_14k     3
#define MODE_16k     4
#define MODE_18k     5
#define MODE_20k     6
#define MODE_23k     7
#define MODE_24k     8
#define MRDTX        9
#define MRNO_DATA    15

/* number of bits */
#define HEADER_SIZE  6     /* real size + 1  */
#define T_NBBITS_7k  (NBBITS_7k + HEADER_SIZE)
#define T_NBBITS_9k  (NBBITS_9k + HEADER_SIZE)
#define T_NBBITS_12k (NBBITS_12k + HEADER_SIZE)
#define T_NBBITS_14k (NBBITS_14k + HEADER_SIZE)
#define T_NBBITS_16k (NBBITS_16k + HEADER_SIZE)
#define T_NBBITS_18k (NBBITS_18k + HEADER_SIZE)
#define T_NBBITS_20k (NBBITS_20k + HEADER_SIZE)
#define T_NBBITS_23k (NBBITS_23k + HEADER_SIZE)
#define T_NBBITS_24k (NBBITS_24k + HEADER_SIZE)
#define T_NBBITS_SID (NBBITS_SID + HEADER_SIZE)


typedef struct
{
   Word16 sid_update_counter;   /* Number of frames since last SID */
   Word16 sid_handover_debt;    /* Number of extra SID_UPD frames to schedule */
   Word16 prev_ft;              /* Type of the previous frame */
   void *encoder_state;         /* Points encoder state structure */
} WB_enc_if_state;


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

extern const UWord8 block_size[];
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

const Word16 *dhf[10];


/*
 * E_IF_homing_frame_test
 *
 *
 * Parameters:
 *    input_frame I: input speech frame
 *
 * Function:
 *    Check 320 input samples for matching EHF_MASK
 *
 * Returns:
 *    If homing frame
 */
Word16 E_IF_homing_frame_test(Word16 input_frame[])
{
   Word32 i, j = 0;

   for (i = 0; i < L_FRAME16k; i++)
   {
      j = input_frame[i] ^ EHF_MASK;

      if (j)
      {
         break;
      }
   }

   return (Word16) (!j);
}

/*
 * E_IF_homing_coding
 *
 *
 * Parameters:
 *    parms  I/O: pointer to parameter vector
 *    mode     I: speech mode
 *
 * Function:
 *    Copy decoder homing frame from memory to parameter vector
 *
 * Returns:
 *    void
 */
void E_IF_homing_coding(Word16 *parms, Word16 mode)
{
   memcpy(parms, dhf[mode], nb_of_param[mode] * sizeof(Word16));
}


#ifdef IF2
/*
 * E_IF_if2_conversion
 *
 *
 * Parameters:
 *  mode        I: Mode
 *  param       I: encoder output
 *  stream      O: packed octets (TS26.201)
 *  frame_type  I: TX frame type
 *  dtx         I: speech mode for mode MRDTX
 *
 * Function:
 *  Packing one frame of encoded parameters to AMR-WB IF2
 *
 * Returns:
 *    number of octets
 */
static int E_IF_if2_conversion(Word16 mode, Word16 *param, UWord8 *stream,
                               Word16 frame_type, Word16 speech_mode)
{
   Word32 j = 0;
   Word16 const *mask;

   memset(stream, 0, block_size[mode]);

   switch(mode)
   {
   case MRNO_DATA:
      *stream = 0xF8;
      j = 8;
      break;

   case MODE_7k:
      mask = mode_7k;
      *stream = 0x2;    /* frame_type = 0, fqi = 1  */

      for (j = HEADER_SIZE; j < T_NBBITS_7k; j++)
      {
         if (param[*mask] & *(mask + 1))
         {
            *stream += 0x1;
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

      while (j % 8)
      {
         *stream <<= 1;
         j++;
      }

      break;

   case MODE_9k:
      mask = mode_9k;
      *stream = 0x6;    /* frame_type = 1, fqi = 1  */

      for (j = HEADER_SIZE; j < T_NBBITS_9k; j++)
      {
         if (param[*mask] & *(mask + 1))
         {
            *stream += 0x1;
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

      while (j % 8)
      {
         *stream <<= 1;
         j++;
      }

      break;

   case MODE_12k:
      mask = mode_12k;
      *stream = 0xA;    /* frame_type = 2, fqi = 1  */

      for (j = HEADER_SIZE; j < T_NBBITS_12k; j++)
      {
         if (param[*mask] & *(mask + 1))
         {
            *stream += 0x1;
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

      while (j % 8)
      {
         *stream <<= 1;
         j++;
      }
      break;

   case MODE_14k:
      mask = mode_14k;
      *stream = 0xE;    /* frame_type = 3, fqi = 1  */

      for ( j = HEADER_SIZE; j < T_NBBITS_14k; j++ )
      {
         if (param[*mask] & *(mask + 1))
         {
            *stream += 0x1;
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

      while (j % 8)
      {
         *stream <<= 1;
         j++;
      }

      break;

   case MODE_16k:
      mask = mode_16k;
      *stream = 0x12;    /* frame_type = 4, fqi = 1  */

      for (j = HEADER_SIZE; j < T_NBBITS_16k; j++)
      {
         if (param[*mask] & *(mask + 1))
         {
            *stream += 0x1;
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

      while (j % 8)
      {
         *stream <<= 1;
         j++;
      }

      break;

   case MODE_18k:
      mask = mode_18k;
      *stream = 0x16;    /* frame_type = 5, fqi = 1  */

      for (j = HEADER_SIZE; j < T_NBBITS_18k; j++)
      {
         if (param[*mask] & *(mask + 1))
         {
            *stream += 0x1;
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

      while (j % 8)
      {
         *stream <<= 1;
         j++;
      }

      break;

   case MODE_20k:
      mask = mode_20k;
      *stream = 0x1A;    /* frame_type = 6, fqi = 1  */

      for (j = HEADER_SIZE; j < T_NBBITS_20k; j++)
      {
         if (param[*mask] & *( mask + 1))
         {
            *stream += 0x1;
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

      while (j % 8)
      {
         *stream <<= 1;
         j++;
      }

      break;

   case MODE_23k:
      mask = mode_23k;
      *stream = 0x1E;    /* frame_type = 7, fqi = 1  */

      for (j = HEADER_SIZE; j < T_NBBITS_23k; j++)
      {
         if (param[*mask] & *( mask + 1))
         {
            *stream += 0x1;
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

      while (j % 8)
      {
         *stream <<= 1;
         j++;
      }

      break;

   case MODE_24k:
      mask = mode_24k;
      *stream = 0x22;    /* frame_type = 8, fqi = 1  */

      for (j = HEADER_SIZE; j < T_NBBITS_24k; j++)
      {
         if (param[*mask] & *( mask + 1))
         {
            *stream += 0x1;
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

      while (j % 8)
      {
         *stream <<= 1;
         j++;
      }

      break;

   case MRDTX:
      mask = mode_DTX;
      *stream = 0x26;    /* frame_type = 9, fqi = 1  */

      for ( j = HEADER_SIZE; j < T_NBBITS_SID; j++ )
      {
         if (param[*mask] & *(mask + 1))
         {
            *stream += 0x1;
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

      /* sid type */
      if (frame_type == TX_SID_UPDATE)
      {
         /* sid update */
         *stream += 0x1;
      }

      /* speech mode indicator */
      *stream <<= 4;
      *stream = (UWord8)(*stream + speech_mode);
      /* bit stuffing */
      *stream <<= 3;
      j = 48;

      break;

   default:
      break;

   }

   return j/8;
}

#else

/*
 * E_IF_mms_conversion
 *
 *
 * Parameters:
 *  mode        I: Mode
 *  param       I: encoder output
 *  stream      O: packed octets (RFC 3267, section 5.3)
 *  frame_type  I: TX frame type
 *  dtx         I: speech mode for mode MRDTX
 *
 * Function:
 *  Packing one frame of encoded parameters to AMR-WB MMS format
 *
 * Returns:
 *    number of octets
 */
static int E_IF_mms_conversion(Word16 mode, Word16 *param, UWord8 *stream,
                               Word16 frame_type, Word16 speech_mode)
{
   Word32 j = 0;
   Word16 const *mask;

   memset(stream, 0, block_size[mode]);

   switch(mode)
   {
   case MRNO_DATA:
      *stream = 0x7C;
      j = 0;
      break;

   case MODE_7k:
      mask = mode_7k;
      *stream = 0x04;    /* frame_type = 0, fqi = 1  */
	  stream++;

      for (j = 1; j <= NBBITS_7k; j++)
      {
         if (param[*mask] & *(mask + 1))
         {
            *stream += 0x1;
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

      while (j % 8)
      {
         *stream <<= 1;
         j++;
      }

      break;

   case MODE_9k:
      mask = mode_9k;
      *stream = 0x0C;    /* frame_type = 1, fqi = 1  */
	  stream++;

      for (j = 1; j <= NBBITS_9k; j++)
      {
         if (param[*mask] & *(mask + 1))
         {
            *stream += 0x1;
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

      while (j % 8)
      {
         *stream <<= 1;
         j++;
      }

      break;

   case MODE_12k:
      mask = mode_12k;
      *stream = 0x14;    /* frame_type = 2, fqi = 1  */
	  stream++;

      for (j = 1; j <= NBBITS_12k; j++)
      {
         if (param[*mask] & *(mask + 1))
         {
            *stream += 0x1;
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

      while (j % 8)
      {
         *stream <<= 1;
         j++;
      }
      break;

   case MODE_14k:
      mask = mode_14k;
      *stream = 0x1C;    /* frame_type = 3, fqi = 1  */
	  stream++;

      for ( j = 1; j <= NBBITS_14k; j++ )
      {
         if (param[*mask] & *(mask + 1))
         {
            *stream += 0x1;
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

      while (j % 8)
      {
         *stream <<= 1;
         j++;
      }

      break;

   case MODE_16k:
      mask = mode_16k;
      *stream = 0x24;    /* frame_type = 4, fqi = 1  */
	  stream++;

      for (j = 1; j <= NBBITS_16k; j++)
      {
         if (param[*mask] & *(mask + 1))
         {
            *stream += 0x1;
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

      while (j % 8)
      {
         *stream <<= 1;
         j++;
      }

      break;

   case MODE_18k:
      mask = mode_18k;
      *stream = 0x2C;    /* frame_type = 5, fqi = 1  */
	  stream++;

      for (j = 1; j <= NBBITS_18k; j++)
      {
         if (param[*mask] & *(mask + 1))
         {
            *stream += 0x1;
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

      while (j % 8)
      {
         *stream <<= 1;
         j++;
      }

      break;

   case MODE_20k:
      mask = mode_20k;
      *stream = 0x34;    /* frame_type = 6, fqi = 1  */
	  stream++;

      for (j = 1; j <= NBBITS_20k; j++)
      {
         if (param[*mask] & *( mask + 1))
         {
            *stream += 0x1;
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

      while (j % 8)
      {
         *stream <<= 1;
         j++;
      }

      break;

   case MODE_23k:
      mask = mode_23k;
      *stream = 0x3C;    /* frame_type = 7, fqi = 1  */
	  stream++;

      for (j = 1; j <= NBBITS_23k; j++)
      {
         if (param[*mask] & *( mask + 1))
         {
            *stream += 0x1;
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

      while (j % 8)
      {
         *stream <<= 1;
         j++;
      }

      break;

   case MODE_24k:
      mask = mode_24k;
      *stream = 0x44;    /* frame_type = 8, fqi = 1  */
	  stream++;

      for (j = 1; j <= NBBITS_24k; j++)
      {
         if (param[*mask] & *( mask + 1))
         {
            *stream += 0x1;
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

      while (j % 8)
      {
         *stream <<= 1;
         j++;
      }

      break;

   case MRDTX:
      mask = mode_DTX;
      *stream = 0x4C;    /* frame_type = 9, fqi = 1  */
	  stream++;

      for ( j = 1; j <= NBBITS_SID; j++ )
      {
         if (param[*mask] & *(mask + 1))
         {
            *stream += 0x1;
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

      /* sid type */
      if (frame_type == TX_SID_UPDATE)
      {
         /* sid update */
         *stream += 0x1;
      }

      /* speech mode indicator */
      *stream <<= 4;
      *stream = (UWord8)(*stream + speech_mode);
      j = 40;

      break;

   default:
      break;

   }

   return j/8 + 1;
}

#endif

/*
 * E_IF_sid_sync_reset
 *
 * Parameters:
 *    st                O: state structure
 *
 * Function:
 *    Initializes state memory
 *
 * Returns:
 *    void
 */
static void E_IF_sid_sync_reset(WB_enc_if_state *st)
{
   st->sid_update_counter = 3;
   st->sid_handover_debt = 0;
   st->prev_ft = TX_SPEECH;
}

/*
 * E_IF_encode
 *
 *
 * Parameters:
 *    st                I: pointer to state structure
 *    mode              I: Speech Mode
 *    speech            I: Input speech
 *    serial            O: Output octet structure IF2 or 16-bit serial stream
 *    dtx               I: use DTX
 *
 * Function:
 *    Encoding and packing one frame of speech
 *
 * Returns:
 *    number of octets
 */
int E_IF_encode(void *st, Word16 req_mode, Word16 *speech, UWord8 *serial,
                Word16 dtx)
{
   Word16 prms[NB_PARM_MAX];
   Word32 i;
   Word16 frame_type, mode, reset_flag;
   WB_enc_if_state *s;

   s = (WB_enc_if_state *)st;
   mode = req_mode;

   /* check for homing frame */
   reset_flag = E_IF_homing_frame_test(speech);

   if (!reset_flag)
   {
      for (i = 0; i < L_FRAME16k; i++)   /* Delete the 2 LSBs (14-bit input) */
      {
         speech[i] = (Word16) (speech[i] & 0xfffC);
      }

      E_MAIN_encode(&mode, speech, prms, s->encoder_state, dtx);

      if (mode == MRDTX)
      {
         s->sid_update_counter--;

         if (s->prev_ft == TX_SPEECH)
         {
            frame_type = TX_SID_FIRST;
            s->sid_update_counter = 3;
         }
         else
         {
            if ((s->sid_handover_debt > 0) && (s->sid_update_counter > 2))
            {
               /*
                * ensure extra updates are
                * properly delayed after a possible SID_FIRST
                */
               frame_type = TX_SID_UPDATE;
               s->sid_handover_debt--;
            }
            else
            {
               if (s->sid_update_counter == 0)
               {
                  frame_type = TX_SID_UPDATE;
                  s->sid_update_counter = 8;
               }
               else
               {
                  frame_type = TX_NO_DATA;
                  mode = MRNO_DATA;
               }
            }
         }
      }
      else
      {
         s->sid_update_counter = 8;
         frame_type = TX_SPEECH;
      }

      s->prev_ft = frame_type;

   }
   /* perform homing if homing frame was detected at encoder input */
   else
   {
      E_MAIN_reset(s->encoder_state, 1);
      E_IF_sid_sync_reset(s);
      E_IF_homing_coding(prms, mode);
      frame_type = TX_SPEECH;
   }

#ifdef IF2
   return E_IF_if2_conversion(mode, prms, serial, frame_type, req_mode);
#else
   return E_IF_mms_conversion(mode, prms, serial, frame_type, req_mode);
#endif
}

/*
 * E_IF_init
 *
 * Parameters:
 *    none
 *
 * Function:
 *    Allocates state memory and initializes state memory
 *
 * Returns:
 *    pointer to encoder interface structure
 */
void *E_IF_init(void)
{
   WB_enc_if_state * s;

   /* allocate memory */
   if ((s = (WB_enc_if_state *)malloc(sizeof(WB_enc_if_state))) == NULL)
   {
      return NULL;
   }

   E_MAIN_init(&(s->encoder_state));
   if (s->encoder_state == NULL)
   {
      free(s);
      return NULL;
   }

   E_IF_sid_sync_reset(s);

   return s;
}

/*
 * E_IF_exit
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
void E_IF_exit(void *state)
{
   WB_enc_if_state *s;
   s = (WB_enc_if_state *)state;

   /* free memory */
   E_MAIN_close(&s->encoder_state);
   free(s);
   state = NULL;
}
