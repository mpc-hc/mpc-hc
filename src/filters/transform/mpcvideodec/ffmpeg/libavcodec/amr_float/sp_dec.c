/*
 * ===================================================================
 *  TS 26.104
 *  REL-5 V5.4.0 2004-03
 *  REL-6 V6.1.0 2004-03
 *  3GPP AMR Floating-point Speech Codec
 * ===================================================================
 *
 */

/*
 * sp_dec.c
 *
 *
 * Project:
 *    AMR Floating-Point Codec
 *
 * Contains:
 *    This module contains all the functions needed decoding AMR
 *    encoder parameters to 16-bit speech samples
 *
 */
/*
 * include files
 */
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include "sp_dec.h"
#include "rom_dec.h"

/*
 * Declare structure types
 */
enum DTXStateType
{
   SPEECH = 0, DTX, DTX_MUTE
};

/*
 * Decoder memory structure
 */
typedef struct
{
   /* history vector of past synthesis speech energy */
   Word32 frameEnergyHist[L_ENERGYHIST];


   /* state flags */
   Word16 bgHangover;   /* counter; number of frames after last speech frame */


}Bgn_scdState;
typedef struct
{
   Word32 hangCount;   /* counter; */
   /* history vector of past synthesis speech energy */
   Word32 cbGainHistory[L_CBGAINHIST];
   Word16 hangVar;   /* counter; */

}Cb_gain_averageState;
typedef struct
{
   Word32 lsp_meanSave[M];   /* Averaged LSPs saved for efficiency  */


}lsp_avgState;
typedef struct
{
   Word32 past_r_q[M];   /* Past quantized prediction error, Q15 */
   Word32 past_lsf_q[M];   /* Past dequantized lsfs, Q15 */


}D_plsfState;
typedef struct
{
   Word32 pbuf[5];
   Word32 past_gain_pit;
   Word32 prev_gp;


}ec_gain_pitchState;
typedef struct
{
   Word32 gbuf[5];
   Word32 past_gain_code;
   Word32 prev_gc;


}ec_gain_codeState;
typedef struct
{
   /*
    * normal MA predictor memory, Q10
    * (contains 20*log10(quaErr))
    */
   Word32 past_qua_en[4];


   /*
    * MA predictor memory for MR122 mode, Q10
    * (contains log2(quaErr))
    */
   Word32 past_qua_en_MR122[4];


}gc_predState;
typedef struct
{
   Word32 gainMem[PHDGAINMEMSIZE];
   Word32 prevCbGain;
   Word32 prevState;
   Word16 lockFull;
   Word16 onset;


}ph_dispState;
typedef struct
{
   enum DTXStateType dtxGlobalState;   /* contains previous state */

   Word32 log_en;
   Word32 old_log_en;
   Word32 pn_seed_rx;
   Word32 lsp[M];
   Word32 lsp_old[M];
   Word32 lsf_hist[M * DTX_HIST_SIZE];
   Word32 lsf_hist_mean[M * DTX_HIST_SIZE];
   Word32 log_en_hist[DTX_HIST_SIZE];
   Word32 true_sid_period_inv;
   Word16 since_last_sid;
   Word16 lsf_hist_ptr;
   Word16 log_pg_mean;
   Word16 log_en_hist_ptr;
   Word16 log_en_adjust;
   Word16 dtxHangoverCount;
   Word16 decAnaElapsedCount;
   Word16 sid_frame;
   Word16 valid_data;
   Word16 dtxHangoverAdded;


   /* updated in main decoder */
   Word16 data_updated;   /* marker to know if CNI data is ever renewed */


}dtx_decState;
typedef struct
{
   Word32 past_gain;


}agcState;
typedef struct
{
   /* Excitation vector */
   Word32 old_exc[L_SUBFR + PIT_MAX + L_INTERPOL];
   Word32 *exc;
   Word32 lsp_old[M];


   /* Filter's memory */
   Word32 mem_syn[M];


   /* pitch sharpening */
   Word32 sharp;
   Word32 old_T0;


   /* Variable holding received ltpLag, used in background noise and BFI */
   Word32 T0_lagBuff;


   /* Variables for the source characteristic detector (SCD) */
   Word32 inBackgroundNoise;
   Word32 voicedHangover;
   Word32 ltpGainHistory[9];


   /* Memories for bad frame handling */
   Word32 excEnergyHist[9];
   Word16 prev_bf;
   Word16 prev_pdf;
   Word16 state;
   Word16 nodataSeed;


   Bgn_scdState * background_state;
   Cb_gain_averageState * Cb_gain_averState;
   lsp_avgState * lsp_avg_st;
   D_plsfState * lsfState;
   ec_gain_pitchState * ec_gain_p_st;
   ec_gain_codeState * ec_gain_c_st;
   gc_predState * pred_state;
   ph_dispState * ph_disp_st;
   dtx_decState * dtxDecoderState;
}Decoder_amrState;
typedef struct
{
   Word32 res2[L_SUBFR];
   Word32 mem_syn_pst[M];
   Word32 synth_buf[M + L_FRAME];
   Word32 preemph_state_mem_pre;
   agcState * agc_state;
}Post_FilterState;
typedef struct
{
   Word32 y2_hi;
   Word32 y2_lo;
   Word32 y1_hi;
   Word32 y1_lo;
   Word32 x0;
   Word32 x1;


}Post_ProcessState;
typedef struct
{
   Decoder_amrState * decoder_amrState;
   Post_FilterState * post_state;
   Post_ProcessState * postHP_state;
}Speech_Decode_FrameState;


/*
 * CodAmrReset
 *
 *
 * Parameters:
 *    state             B: state structure
 *    mode              I: AMR mode
 *
 * Function:
 *    Resets state memory
 *
 * Returns:
 *    void
 */
static void Decoder_amr_reset( Decoder_amrState *state, enum Mode mode )
{
   Word32 i;

   /* Cb_gain_average_reset */
   memset(state->Cb_gain_averState->cbGainHistory, 0, L_CBGAINHIST << 2);
   state->Cb_gain_averState->hangVar = 0;
   state->Cb_gain_averState->hangCount= 0;

   /* Initialize static pointer */
   state->exc = state->old_exc + PIT_MAX + L_INTERPOL;

   /* Static vectors to zero */
   memset( state->old_exc, 0, ( PIT_MAX + L_INTERPOL )<<2 );

   if ( mode != MRDTX )
      memset( state->mem_syn, 0, M <<2 );

   /* initialize pitch sharpening */
   state->sharp = SHARPMIN;
   state->old_T0 = 40;

   /* Initialize state->lsp_old [] */
   if ( mode != MRDTX ) {
      state->lsp_old[0] = 30000;
      state->lsp_old[1] = 26000;
      state->lsp_old[2] = 21000;
      state->lsp_old[3] = 15000;
      state->lsp_old[4] = 8000;
      state->lsp_old[5] = 0;
      state->lsp_old[6] = -8000;
      state->lsp_old[7] = -15000;
      state->lsp_old[8] = -21000;
      state->lsp_old[9] = -26000;
   }

   /* Initialize memories of bad frame handling */
   state->prev_bf = 0;
   state->prev_pdf = 0;
   state->state = 0;
   state->T0_lagBuff = 40;
   state->inBackgroundNoise = 0;
   state->voicedHangover = 0;

   if ( mode != MRDTX )
      memset( state->excEnergyHist, 0, 9 <<2 );
   memset( state->ltpGainHistory, 0, 9 <<2 );

   if ( mode != MRDTX ) {
      state->lsp_avg_st->lsp_meanSave[0] = 1384;
      state->lsp_avg_st->lsp_meanSave[1] = 2077;
      state->lsp_avg_st->lsp_meanSave[2] = 3420;
      state->lsp_avg_st->lsp_meanSave[3] = 5108;
      state->lsp_avg_st->lsp_meanSave[4] = 6742;
      state->lsp_avg_st->lsp_meanSave[5] = 8122;
      state->lsp_avg_st->lsp_meanSave[6] = 9863;
      state->lsp_avg_st->lsp_meanSave[7] = 11092;
      state->lsp_avg_st->lsp_meanSave[8] = 12714;
      state->lsp_avg_st->lsp_meanSave[9] = 13701;
   }
   memset( state->lsfState->past_r_q, 0, M <<2 );

   /* Past dequantized lsfs */
   state->lsfState->past_lsf_q[0] = 1384;
   state->lsfState->past_lsf_q[1] = 2077;
   state->lsfState->past_lsf_q[2] = 3420;
   state->lsfState->past_lsf_q[3] = 5108;
   state->lsfState->past_lsf_q[4] = 6742;
   state->lsfState->past_lsf_q[5] = 8122;
   state->lsfState->past_lsf_q[6] = 9863;
   state->lsfState->past_lsf_q[7] = 11092;
   state->lsfState->past_lsf_q[8] = 12714;
   state->lsfState->past_lsf_q[9] = 13701;

   for ( i = 0; i < 5; i++ )
      state->ec_gain_p_st->pbuf[i] = 1640;
   state->ec_gain_p_st->past_gain_pit = 0;
   state->ec_gain_p_st->prev_gp = 16384;

   for ( i = 0; i < 5; i++ )
      state->ec_gain_c_st->gbuf[i] = 1;
   state->ec_gain_c_st->past_gain_code = 0;
   state->ec_gain_c_st->prev_gc = 1;

   if ( mode != MRDTX ) {
      for ( i = 0; i < NPRED; i++ ) {
         state->pred_state->past_qua_en[i] = MIN_ENERGY;
         state->pred_state->past_qua_en_MR122[i] = MIN_ENERGY_MR122;
      }
   }
   state->nodataSeed = 21845;

   /* Static vectors to zero */
   memset( state->background_state->frameEnergyHist, 0, L_ENERGYHIST <<2 );

   /* Initialize hangover handling */
   state->background_state->bgHangover = 0;

   /* phDispReset */
   memset( state->ph_disp_st->gainMem, 0, PHDGAINMEMSIZE <<2 );
   state->ph_disp_st->prevState = 0;
   state->ph_disp_st->prevCbGain = 0;
   state->ph_disp_st->lockFull = 0;
   state->ph_disp_st->onset = 0;   /* assume no onset in start */

   if ( mode != MRDTX ) {
      state->dtxDecoderState->since_last_sid = 0;
      state->dtxDecoderState->true_sid_period_inv = 8192;
      state->dtxDecoderState->log_en = 3500;
      state->dtxDecoderState->old_log_en = 3500;

      /* low level noise for better performance in  DTX handover cases*/
      state->dtxDecoderState->pn_seed_rx = PN_INITIAL_SEED;

      /* Initialize state->lsp [] */
      state->dtxDecoderState->lsp[0] = 30000;
      state->dtxDecoderState->lsp[1] = 26000;
      state->dtxDecoderState->lsp[2] = 21000;
      state->dtxDecoderState->lsp[3] = 15000;
      state->dtxDecoderState->lsp[4] = 8000;
      state->dtxDecoderState->lsp[5] = 0;
      state->dtxDecoderState->lsp[6] = -8000;
      state->dtxDecoderState->lsp[7] = -15000;
      state->dtxDecoderState->lsp[8] = -21000;
      state->dtxDecoderState->lsp[9] = -26000;

      /* Initialize state->lsp_old [] */
      state->dtxDecoderState->lsp_old[0] = 30000;
      state->dtxDecoderState->lsp_old[1] = 26000;
      state->dtxDecoderState->lsp_old[2] = 21000;
      state->dtxDecoderState->lsp_old[3] = 15000;
      state->dtxDecoderState->lsp_old[4] = 8000;
      state->dtxDecoderState->lsp_old[5] = 0;
      state->dtxDecoderState->lsp_old[6] = -8000;
      state->dtxDecoderState->lsp_old[7] = -15000;
      state->dtxDecoderState->lsp_old[8] = -21000;
      state->dtxDecoderState->lsp_old[9] = -26000;
      state->dtxDecoderState->lsf_hist_ptr = 0;
      state->dtxDecoderState->log_pg_mean = 0;
      state->dtxDecoderState->log_en_hist_ptr = 0;

      /* initialize decoder lsf history */
      state->dtxDecoderState->lsf_hist[0] = 1384;
      state->dtxDecoderState->lsf_hist[1] = 2077;
      state->dtxDecoderState->lsf_hist[2] = 3420;
      state->dtxDecoderState->lsf_hist[3] = 5108;
      state->dtxDecoderState->lsf_hist[4] = 6742;
      state->dtxDecoderState->lsf_hist[5] = 8122;
      state->dtxDecoderState->lsf_hist[6] = 9863;
      state->dtxDecoderState->lsf_hist[7] = 11092;
      state->dtxDecoderState->lsf_hist[8] = 12714;
      state->dtxDecoderState->lsf_hist[9] = 13701;

      for ( i = 1; i < DTX_HIST_SIZE; i++ ) {
         memcpy( &state->dtxDecoderState->lsf_hist[M * i], &state->
               dtxDecoderState->lsf_hist[0], M <<2 );
      }
      memset( state->dtxDecoderState->lsf_hist_mean, 0, M * DTX_HIST_SIZE <<2 );

      /* initialize decoder log frame energy */
      for ( i = 0; i < DTX_HIST_SIZE; i++ ) {
         state->dtxDecoderState->log_en_hist[i] = state->dtxDecoderState->log_en
         ;
      }
      state->dtxDecoderState->log_en_adjust = 0;
      state->dtxDecoderState->dtxHangoverCount = DTX_HANG_CONST;
      state->dtxDecoderState->decAnaElapsedCount = 31;
      state->dtxDecoderState->sid_frame = 0;
      state->dtxDecoderState->valid_data = 0;
      state->dtxDecoderState->dtxHangoverAdded = 0;
      state->dtxDecoderState->dtxGlobalState = DTX;
      state->dtxDecoderState->data_updated = 0;
   }
   return;
}


/*
 * rx_dtx_handler
 *
 *
 * Parameters:
 *    st->dtxGlobalState      I: DTX state
 *    st->since_last_sid      B: Frames after last SID frame
 *    st->data_updated        I: SID update flag
 *    st->decAnaElapsedCount  B: state machine that synch with the GSMEFR txDtx machine
 *    st->dtxHangoverAdded    B: DTX hangover
 *    st->sid_frame           O: SID frame indicator
 *    st->valid_data          O: Vaild data indicator
 *    frame_type              O: Frame type
 *
 * Function:
 *    Find the new DTX state
 *
 * Returns:
 *    DTXStateType            DTX, DTX_MUTE or SPEECH
 */
static enum DTXStateType rx_dtx_handler( dtx_decState *st, enum RXFrameType frame_type )
{
   enum DTXStateType newState;
   enum DTXStateType encState;

   /* DTX if SID frame or previously in DTX{_MUTE} and (NO_RX OR BAD_SPEECH) */
   if ( table_SID[frame_type] | ( ( st->dtxGlobalState != SPEECH ) &
         table_speech_bad[frame_type] ) ) {
      newState = DTX;

      /* stay in mute for these input types */
      if ( ( st->dtxGlobalState == DTX_MUTE ) & table_mute[frame_type] ) {
         newState = DTX_MUTE;
      }

      /*
       * evaluate if noise parameters are too old
       * since_last_sid is reset when CN parameters have been updated
       */
      st->since_last_sid += 1;

      /* no update of sid parameters in DTX for a long while */
      if ((frame_type != RX_SID_UPDATE) & ( st->since_last_sid > DTX_MAX_EMPTY_THRESH )) {
         newState = DTX_MUTE;
      }
   }
   else {
      newState = SPEECH;
      st->since_last_sid = 0;
   }

   /*
    * reset the decAnaElapsed Counter when receiving CNI data the first
    * time, to robustify counter missmatch after handover
    * this might delay the bwd CNI analysis in the new decoder slightly.
    */
   if ( ( st->data_updated == 0 ) & ( frame_type == RX_SID_UPDATE ) ) {
      st->decAnaElapsedCount = 0;
   }

   /*
    * update the SPE-SPD DTX hangover synchronization
    * to know when SPE has added dtx hangover
    */
   st->decAnaElapsedCount += 1;
   st->dtxHangoverAdded = 0;
   encState = SPEECH;

   if ( table_DTX[frame_type] ) {
      encState = DTX;
      if( ( frame_type == RX_NO_DATA ) & ( newState == SPEECH ) ) {
         encState = SPEECH;
      }
   }

   if ( encState == SPEECH ) {
      st->dtxHangoverCount = DTX_HANG_CONST;
   }
   else {
      if ( st->decAnaElapsedCount > DTX_ELAPSED_FRAMES_THRESH ) {
         st->dtxHangoverAdded = 1;
         st->decAnaElapsedCount = 0;
         st->dtxHangoverCount = 0;
      }
      else if ( st->dtxHangoverCount == 0 ) {
         st->decAnaElapsedCount = 0;
      }
      else {
         st->dtxHangoverCount -= 1;
      }
   }

   if ( newState != SPEECH ) {
      /*
       * DTX or DTX_MUTE
       * CN data is not in a first SID, first SIDs are marked as SID_BAD
       * but will do backwards analysis if a hangover period has been added
       * according to the state machine above
       */
      st->sid_frame = 0;
      st->valid_data = 0;

      if ( frame_type == RX_SID_FIRST ) {
         st->sid_frame = 1;
      }
      else if ( frame_type == RX_SID_UPDATE ) {
         st->sid_frame = 1;
         st->valid_data = 1;
      }
      else if ( frame_type == RX_SID_BAD ) {
         st->sid_frame = 1;

         /* use old data */
         st->dtxHangoverAdded = 0;
      }
   }

   /* newState is used by both SPEECH AND DTX synthesis routines */
   return newState;
}


/*
 * Lsf_lsp
 *
 *
 * Parameters:
 *    lsf               I: vector of LSFs
 *    lsp               O: vector of LSPs
 *
 * Function:
 *    Transformation lsf to lsp, order M
 *
 * Returns:
 *    void
 */
static void Lsf_lsp( Word32 lsf[], Word32 lsp[] )
{
   Word32 i, ind, offset, tmp;


   for ( i = 0; i < M; i++ ) {
      /* ind = b8-b15 of lsf[i] */
      ind = lsf[i] >> 8;

      /* offset = b0-b7  of lsf[i] */
      offset = lsf[i] & 0x00ff;

      /* lsp[i] = table[ind]+ ((table[ind+1]-table[ind])*offset) / 256 */
      tmp = ( ( cos_table[ind+1]-cos_table[ind] )*offset ) << 1;
      lsp[i] = cos_table[ind] + ( tmp >> 9 );
   }
   return;
}


/*
 * D_plsf_3
 *
 *
 * Parameters:
 *    st->past_lsf_q    I: Past dequantized LFSs
 *    st->past_r_q      B: past quantized residual
 *    mode              I: AMR mode
 *    bfi               B: bad frame indicator
 *    indice            I: quantization indices of 3 submatrices, Q0
 *    lsp1_q            O: quantized 1st LSP vector
 *
 * Function:
 *    Decodes the LSP parameters using the received quantization indices.
 *    1st order MA prediction and split by 3 vector quantization (split-VQ)
 *
 * Returns:
 *    void
 */
static void D_plsf_3( D_plsfState *st, enum Mode mode, Word16 bfi, Word16 *
      indice, Word32 *lsp1_q )
{
   Word32 lsf1_r[M], lsf1_q[M];
   Word32 i, index, temp;
   const Word32 *p_cb1, *p_cb2, *p_cb3, *p_dico;


   /* if bad frame */
   if ( bfi != 0 ) {
      /* use the past LSFs slightly shifted towards their mean */
      for ( i = 0; i < M; i++ ) {
         /* lsfi_q[i] = ALPHA*past_lsf_q[i] + ONE_ALPHA*meanLsf[i]; */
         lsf1_q[i] = ( ( st->past_lsf_q[i] * ALPHA ) >> 15 ) + ( ( mean_lsf_3[i]
               * ONE_ALPHA ) >> 15 );
      }

      /* estimate past quantized residual to be used in next frame */
      if ( mode != MRDTX ) {
         for ( i = 0; i < M; i++ ) {
            /* temp  = meanLsf[i] +  pastR2_q[i] * pred_fac; */
            temp = mean_lsf_3[i] + ( ( st->past_r_q[i] * pred_fac[i] ) >> 15 );
            st->past_r_q[i] = lsf1_q[i] - temp;
         }
      }
      else {
         for ( i = 0; i < M; i++ ) {
            /* temp  = meanLsf[i] +  pastR2_q[i]; */
            temp = mean_lsf_3[i] + st->past_r_q[i];
            st->past_r_q[i] = lsf1_q[i] - temp;
         }
      }
   }

   /* if good LSFs received */
   else {
      if ( ( mode == MR475 ) | ( mode == MR515 ) ) {
         /* MR475, MR515 */
         p_cb1 = dico1_lsf_3;
         p_cb2 = dico2_lsf_3;
         p_cb3 = mr515_3_lsf;
      }
      else if ( mode == MR795 ) {
         /* MR795 */
         p_cb1 = mr795_1_lsf;
         p_cb2 = dico2_lsf_3;
         p_cb3 = dico3_lsf_3;
      }
      else {
         /* MR59, MR67, MR74, MR102, MRDTX */
         p_cb1 = dico1_lsf_3;
         p_cb2 = dico2_lsf_3;
         p_cb3 = dico3_lsf_3;
      }

      /* decode prediction residuals from 3 received indices */
      index = *indice++;
      p_dico = &p_cb1[index + index + index];
      index = *indice++;
      lsf1_r[0] = *p_dico++;
      lsf1_r[1] = *p_dico++;
      lsf1_r[2] = *p_dico++;

      if ( ( mode == MR475 ) | ( mode == MR515 ) ) {
         /* MR475, MR515 only using every second entry */
         index = index << 1;
      }
      p_dico = &p_cb2[index + index + index];
      index = *indice++;
      lsf1_r[3] = *p_dico++;
      lsf1_r[4] = *p_dico++;
      lsf1_r[5] = *p_dico++;
      p_dico = &p_cb3[index << 2];
      lsf1_r[6] = *p_dico++;
      lsf1_r[7] = *p_dico++;
      lsf1_r[8] = *p_dico++;
      lsf1_r[9] = *p_dico++;

      /* Compute quantized LSFs and update the past quantized residual */
      if ( mode != MRDTX ) {
         for ( i = 0; i < M; i++ ) {
            lsf1_q[i] = lsf1_r[i] + ( mean_lsf_3[i] + ( ( st->past_r_q[i] *
                  pred_fac[i] ) >> 15 ) );
         }
         memcpy( st->past_r_q, lsf1_r, M <<2 );
      }
      else {
         for ( i = 0; i < M; i++ ) {
            lsf1_q[i] = lsf1_r[i] + ( mean_lsf_3[i] + st->past_r_q[i] );
         }
         memcpy( st->past_r_q, lsf1_r, M <<2 );
      }
   }

   /* verification that LSFs has minimum distance of LSF_GAP Hz */
   temp = LSF_GAP;

   for ( i = 0; i < M; i++ ) {
      if ( lsf1_q[i] < temp ) {
         lsf1_q[i] = temp;
      }
      temp = lsf1_q[i] + LSF_GAP;
   }
   memcpy( st->past_lsf_q, lsf1_q, M <<2 );

   /*  convert LSFs to the cosine domain */
   Lsf_lsp( lsf1_q, lsp1_q );
   return;
}


/*
 * pseudonoise
 *
 *
 * Parameters:
 *    shift_reg         B: Old CN generator shift register state
 *    no_bits           I: Number of bits
 *
 * Function:
 *    pseudonoise
 *
 * Returns:
 *    noise_bits
 */
static Word32 pseudonoise( Word32 *shift_reg, Word32 no_bits )
{
   Word32 noise_bits, Sn, i;
   Word32 s_reg;


   s_reg = *shift_reg;
   noise_bits = 0;

   for ( i = 0; i < no_bits; i++ ) {
      /* State n == 31 */
      Sn = s_reg & 0x00000001L;

      /* State n == 3 */
      if ( s_reg & 0x10000000L ) {
         Sn = Sn ^ 0x1L;
      }
      else {
         Sn = Sn ^ 0x0L;
      }
      noise_bits = ( noise_bits << 1 ) | ( s_reg & 1 );
      s_reg = s_reg >> 1;

      if ( Sn & 1 ) {
         s_reg = s_reg | 0x40000000L;
      }
   }
   *shift_reg = s_reg;
   return noise_bits;
}


/*
 * Lsp_lsf
 *
 *
 * Parameters:
 *    lsp               I: LSP vector (range: -1<=val<1)
 *    lsf               O: LSF vector Old CN generator shift register state
 *
 * Function:
 *    Transformation lsp to lsf, LPC order M
 *    lsf[i] = arccos(lsp[i])/(2*pi)
 *
 * Returns:
 *    void
 */
static void Lsp_lsf( Word32 lsp[], Word32 lsf[] )
{
   Word32 i, ind = 63;   /* begin at end of table -1 */


   for ( i = M - 1; i >= 0; i-- ) {
      /* find value in table that is just greater than lsp[i] */
      while ( cos_table[ind] < lsp[i] ) {
         ind--;
      }
      lsf[i] = ( ( ( ( lsp[i] - cos_table[ind] ) * acos_slope[ind] ) + 0x800 )
            >> 12 ) + ( ind << 8 );
   }
   return;
}


/*
 * Reorder_lsf
 *
 *
 * Parameters:
 *    lsf            B: vector of LSFs (range: 0<=val<=0.5)
 *    min_dist       I: minimum required distance
 *
 * Function:
 *    Make sure that the LSFs are properly ordered and to keep a certain minimum
 *    distance between adjacent LSFs. LPC order = M.
 *
 * Returns:
 *    void
 */
static void Reorder_lsf( Word32 *lsf, Word32 min_dist )
{
   Word32 lsf_min, i;


   lsf_min = min_dist;

   for ( i = 0; i < M; i++ ) {
      if ( lsf[i] < lsf_min ) {
         lsf[i] = lsf_min;
      }
      lsf_min = lsf[i] + min_dist;
   }
}

/* VC5.0 Global optimization does not work with this function */
#if _MSC_VER == 1100
#pragma optimize( "g", off )
#endif
/*
 * Get_lsp_pol
 *
 *
 * Parameters:
 *    lsp               I: line spectral frequencies
 *    f                 O: polynomial F1(z) or F2(z)
 *
 * Function:
 *    Find the polynomial F1(z) or F2(z) from the LSPs.
 *
 *    F1(z) = product ( 1 - 2 lsp[i] z^-1 + z^-2 )
 *             i=0,2,4,6,8
 *    F2(z) = product   ( 1 - 2 lsp[i] z^-1 + z^-2 )
 *             i=1,3,5,7,9
 *
 *    where lsp[] is the LSP vector in the cosine domain.
 *
 *    The expansion is performed using the following recursion:
 *
 *    f[0] = 1
 *    b = -2.0 * lsp[0]
 *    f[1] = b
 *    for i=2 to 5 do
 *       b = -2.0 * lsp[2*i-2];
 *       f[i] = b*f[i-1] + 2.0*f[i-2];
 *       for j=i-1 down to 2 do
 *          f[j] = f[j] + b*f[j-1] + f[j-2];
 *       f[1] = f[1] + b;
 *
 * Returns:
 *    void
 */
static void Get_lsp_pol( Word32 *lsp, Word32 *f )
{
   volatile Word32 f0, f1, f2, f3, f4, f5;
   Word32 l1, l2, l3, l4;


   /* f[0] = 1.0; */
   f0 = 16777216L;

   /* f1 = *lsp * -1024; */
   f1 = -lsp[0] << 10;
   l1 = lsp[2];
   l2 = lsp[4];
   l3 = lsp[6];
   l4 = lsp[8];
   f2 = f0 << 1;
   f2 -= ( ( ( f1 >> 16 ) * l1 ) + ( ( ( f1 & 0xFFFE ) * l1 ) >> 16 ) ) << 2;
   f1 -= l1 << 10;
   f3 = f1 << 1;
   f3 -= ( ( ( f2 >> 16 ) * l2 ) + ( ( ( f2 & 0xFFFE ) * l2 ) >> 16 ) ) << 2;
   f2 += f0;
   f2 -= ( ( ( f1 >> 16 ) * l2 ) + ( ( ( f1 & 0xFFFE ) * l2 ) >> 16 ) ) << 2;
   f1 -= l2 << 10;
   f4 = f2 << 1;
   f4 -= ( ( ( f3 >> 16 ) * l3 ) + ( ( ( f3 & 0xFFFE ) * l3 ) >> 16 ) ) << 2;
   f3 += f1;
   f3 -= ( ( ( f2 >> 16 ) * l3 ) + ( ( ( f2 & 0xFFFE ) * l3 ) >> 16 ) ) << 2;
   f2 += f0;
   f2 -= ( ( ( f1 >> 16 ) * l3 ) + ( ( ( f1 & 0xFFFE ) * l3 ) >> 16 ) ) << 2;
   f1 -= l3 << 10;
   f5 = f3 << 1;
   f5 -= ( ( ( f4 >> 16 ) * l4 ) + ( ( ( f4 & 0xFFFE ) * l4 ) >> 16 ) ) << 2;
   f4 += f2;
   f4 -= ( ( ( f3 >> 16 ) * l4 ) + ( ( ( f3 & 0xFFFE ) * l4 ) >> 16 ) ) << 2;
   f3 += f1;
   f3 -= ( ( ( f2 >> 16 ) * l4 ) + ( ( ( f2 & 0xFFFE ) * l4 ) >> 16 ) ) << 2;
   f2 += f0;
   f2 -= ( ( ( f1 >> 16 ) * l4 ) + ( ( ( f1 & 0xFFFE ) * l4 ) >> 16 ) ) << 2;
   f1 -= l4 << 10;
   f[0] = f0;
   f[1] = f1;
   f[2] = f2;
   f[3] = f3;
   f[4] = f4;
   f[5] = f5;
   return;
}
#if _MSC_VER == 1100
#pragma optimize( "", on )
#endif


/*
 * Lsp_Az
 *
 *
 * Parameters:
 *    lsp                 I: Line spectral frequencies
 *    a                   O: Predictor coefficients
 *
 * Function:
 *    Converts from the line spectral pairs (LSP) to LP coefficients,
 *    for a 10th order filter.
 *
 *    Find the coefficients of F1(z) and F2(z)
 *    Multiply F1(z) by 1+z^{-1} and F2(z) by 1-z^{-1}
 *    A(z) = ( F1(z) + F2(z) ) / 2
 *
 * Returns:
 *    void
 */
static void Lsp_Az( Word32 lsp[], Word32 a[] )
{
   Word32 f1[6], f2[6];
   Word32 T0, i, j;


   Get_lsp_pol( &lsp[0], f1 );
   Get_lsp_pol( &lsp[1], f2 );

   for ( i = 5; i > 0; i-- ) {
      f1[i] += f1[i - 1];
      f2[i] -= f2[i - 1];
   }
   a[0] = 4096;

   for ( i = 1, j = 10; i <= 5; i++, j-- ) {
      T0 = f1[i] + f2[i];
      a[i] = (Word16)(T0 >> 13);  /* emulate fixed point bug */
      if ( ( T0 & 4096 ) != 0 ) {
         a[i]++;
      }
      T0 = f1[i] - f2[i];
      a[j] = (Word16)(T0 >> 13);   /* emulate fixed point bug */

      if ( ( T0 & 4096 ) != 0 ) {
         a[j]++;
      }
   }
   return;
}


/*
 * A_Refl
 *
 *
 * Parameters:
 *    a                 I: Directform coefficients
 *    refl              O: Reflection coefficients
 *
 * Function:
 *    Converts from the directform coefficients to reflection coefficients
 *
 * Returns:
 *    void
 */
static void A_Refl( Word32 a[], Word32 refl[] )
{
   /* local variables */
   int normShift;
   Word32 aState[M], bState[M];
   Word32 normProd, acc, temp, mult, scale, i, j;


   /* initialize states */
   memcpy( aState, a, M <<2 );

   /* backward Levinson recursion */
   for ( i = M - 1; i >= 0; i-- ) {
      if ( labs( aState[i] ) >= 4096 ) {
         goto ExitRefl;
      }
      refl[i] = aState[i] << 3;
      temp = ( refl[i] * refl[i] ) << 1;
      acc = ( MAX_32 - temp );
      normShift=0;
      if (acc != 0){
         temp = acc;
         while (!(temp & 0x40000000))
         {
            normShift++;
            temp = temp << 1;
         }
      }
      else{
         normShift = 0;
      }
      scale = 15 - normShift;
      acc = ( acc << normShift );
      temp = ( acc + ( Word32 )0x00008000L );

      if ( temp > 0 ) {
         normProd = temp >> 16;
         mult = 0x20000000L / normProd;
      }
      else
         mult = 16384;

      for ( j = 0; j < i; j++ ) {
         acc = aState[j] << 16;
         acc -= ( refl[i] * aState[i - j - 1] ) << 1;
         temp = ( acc + ( Word32 )0x00008000L ) >> 16;
         temp = ( mult * temp ) << 1;

         if ( scale > 0 ) {
            if ( ( temp & ( ( Word32 )1 << ( scale - 1 ) ) ) != 0 ) {
               temp = ( temp >> scale ) + 1;
            }
            else
               temp = ( temp >> scale );
         }
         else
            temp = ( temp >> scale );

         if ( labs( temp ) > 32767 ) {
            goto ExitRefl;
         }
         bState[j] = temp;
      }
      memcpy( aState, bState, i <<2 );
   }
   return;
ExitRefl:
   memset( refl, 0, M <<2 );
}


/*
 * Log2_norm
 *
 *
 * Parameters:
 *    x                 I: input value
 *    exp               I: exponent
 *    exponent          O: Integer part of Log2. (range: 0<=val<=30)
 *    fraction          O: Fractional part of Log2. (range: 0<=val<1)
 *
 * Function:
 *    Computes log2
 *
 *    Computes log2(L_x, exp),  where   L_x is positive and
 *    normalized, and exp is the normalisation exponent
 *    If L_x is negative or zero, the result is 0.
 *
 *    The function Log2(L_x) is approximated by a table and linear
 *    interpolation. The following steps are used to compute Log2(L_x)
 *
 *    exponent = 30-normExponent
 *    i = bit25-b31 of L_x;  32<=i<=63  (because of normalization).
 *    a = bit10-b24
 *    i -=32
 *    fraction = table[i]<<16 - (table[i] - table[i+1]) * a * 2
 *
 * Returns:
 *    void
 */
static void Log2_norm( Word32 x, Word32 exp, Word32 *exponent, Word32 *
      fraction )
{
   Word32 y, i, a;


   if ( x <= 0 ) {
      *exponent = 0;
      *fraction = 0;
      return;
   }

   /* Extract b25-b31 */
   i = x >> 25;
   i = i - 32;

   /* Extract b10-b24 of fraction */
   a = x >> 9;
   a = a & 0xFFFE;   /* 2a */

   /* fraction */
   y = ( log2_table[i] << 16 ) - a * ( log2_table[i] - log2_table[i + 1] );
   *fraction = y >> 16;
   *exponent = 30 - exp;
   return;
}


/*
 * Log2
 *
 *
 * Parameters:
 *    x                 I: input value
 *    exponent          O: Integer part of Log2. (range: 0<=val<=30)
 *    fraction          O: Fractional part of Log2. (range: 0<=val<1)
 *
 * Function:
 *    Computes log2(L_x)
 *    If x is negative or zero, the result is 0.
 *
 * Returns:
 *    void
 */
static void Log2( Word32 x, Word32 *exponent, Word32 *fraction )
{
   int tmp, exp=0;

   if (x != 0){
         tmp = x;
         while (!((tmp & 0x80000000) ^ ((tmp & 0x40000000) << 1)))
         {
            exp++;
            tmp = tmp << 1;
         }
      }
   Log2_norm( x <<exp, exp, exponent, fraction );
}


/*
 * Pow2
 *
 *
 * Parameters:
 *    exponent          I: Integer part. (range: 0<=val<=30)
 *    fraction          O: Fractional part. (range: 0.0<=val<1.0)
 *
 * Function:
 *    pow(2.0, exponent.fraction)
 *
 *    The function Pow2(L_x) is approximated by a table and linear interpolation.
 *
 *    i = bit10-b15 of fraction, 0 <= i <= 31
 *    a = biT0-b9   of fraction
 *    x = table[i]<<16 - (table[i] - table[i+1]) * a * 2
 *    x = L_x >> (30-exponent) (with rounding)
 *
 * Returns:
 *    result (range: 0<=val<=0x7fffffff)
 */
static Word32 Pow2( Word32 exponent, Word32 fraction )
{
   Word32 i, a, tmp, x, exp;


   /* Extract b10-b16 of fraction */
   i = fraction >> 10;

   /* Extract b0-b9 of fraction */
   a = ( fraction << 5 ) & 0x7fff;

   /* table[i] << 16 */
   x = pow2_table[i] << 16;

   /* table[i] - table[i+1] */
   tmp = pow2_table[i] - pow2_table[i + 1];

   /* L_x -= tmp*a*2 */
   x -= ( tmp * a ) << 1;

   if ( exponent >= -1 ) {
      exp = ( 30 - exponent );

      /* Rounding */
      if ( ( x & ( ( Word32 )1 << ( exp - 1 ) ) ) != 0 ) {
         x = ( x >> exp ) + 1;
      }
      else
         x = x >> exp;
   }
   else
      x = 0;
   return( x );
}


/*
 * Build_CN_code
 *
 *
 * Parameters:
 *    seed              B: Old CN generator shift register state
 *    cod               O: Generated CN fixed codebook vector
 *
 * Function:
 *    Generate CN fixed codebook vector
 *
 * Returns:
 *    void
 */
static void Build_CN_code( Word32 *seed, Word32 cod[] )
{
   Word32 i, j, k;


   memset( cod, 0, L_SUBFR <<2 );

   for ( k = 0; k < 10; k++ ) {
      i = pseudonoise( seed, 2 );   /* generate pulse position */
      i = ( i * 20 ) >> 1;
      i = ( i + k );
      j = pseudonoise( seed, 1 );   /* generate sign           */

      if ( j > 0 ) {
         cod[i] = 4096;
      }
      else {
         cod[i] = -4096;
      }
   }
   return;
}


/*
 * Build_CN_param
 *
 *
 * Parameters:
 *    seed              B: Old CN generator shift register state
 *    nParam            I: number of params
 *    paramSizeTable    I: size of params
 *    parm              O: CN Generated params
 *
 * Function:
 *    Generate parameters for comfort noise generation
 *
 * Returns:
 *    void
 */
static void Build_CN_param( Word16 *seed, enum Mode mode, Word16 parm[] )
{
   Word32 i;
   const Word32 *p;


   *seed = ( Word16 )( ( *seed * 31821 ) + 13849L );
   p = &window_200_40[ * seed & 0x7F];

   switch ( mode ) {
      case MR122:
         for ( i = 0; i < PRMNO_MR122; i++ ) {
            parm[i] = ( Word16 )( *p++ & ~( 0xFFFF << bitno_MR122[i] ) );
         }
         break;

      case MR102:
         for ( i = 0; i < PRMNO_MR102; i++ ) {
            parm[i] = ( Word16 )( *p++ & ~( 0xFFFF << bitno_MR102[i] ) );
         }
         break;

      case MR795:
         for ( i = 0; i < PRMNO_MR795; i++ ) {
            parm[i] = ( Word16 )( *p++ & ~( 0xFFFF << bitno_MR795[i] ) );
         }
         break;

      case MR74:
         for ( i = 0; i < PRMNO_MR74; i++ ) {
            parm[i] = ( Word16 )( *p++ & ~( 0xFFFF << bitno_MR74[i] ) );
         }
         break;

      case MR67:
         for ( i = 0; i < PRMNO_MR67; i++ ) {
            parm[i] = ( Word16 )( *p++ & ~( 0xFFFF << bitno_MR67[i] ) );
         }
         break;

      case MR59:
         for ( i = 0; i < PRMNO_MR59; i++ ) {
            parm[i] = ( Word16 )( *p++ & ~( 0xFFFF << bitno_MR59[i] ) );
         }
         break;

      case MR515:
         for ( i = 0; i < PRMNO_MR515; i++ ) {
            parm[i] = ( Word16 )( *p++ & ~( 0xFFFF << bitno_MR515[i] ) );
         }
         break;

      case MR475:
         for ( i = 0; i < PRMNO_MR475; i++ ) {
            parm[i] = ( Word16 )( *p++ & ~( 0xFFFF << bitno_MR475[i] ) );
         }
         break;
   }
}


/*
 * Syn_filt
 *
 *
 * Parameters:
 *    a                 I: prediction coefficients [M+1]
 *    x                 I: input signal
 *    y                 O: output signal
 *    lg                I: size of filtering
 *    mem               B: memory associated with this filtering
 *    update            I: 0=no update, 1=update of memory.
 *
 * Function:
 *    Perform synthesis filtering through 1/A(z).
 *
 * Returns:
 *    void
 */
static Word32 Syn_filt( Word32 a[], Word32 x[], Word32 y[], Word32 lg, Word32 mem[]
      , Word32 update )
{
   Word32 tmp[50];   /* malloc is slow */
   Word32 s, a0, overflow = 0;
   Word32 *yy, *yy_limit;


   /* Copy mem[] to yy[] */
   memcpy( tmp, mem, 40 );
   yy = tmp + M;
   yy_limit = yy + lg;
   a0 = a[0];

   /* Do the filtering. */
   while ( yy < yy_limit ) {

      s = *x++ * a0;
      s -= yy[-1] * a[1];
      s -= yy[-2] * a[2];
      s -= yy[-3] * a[3];
      s -= yy[-4] * a[4];
      s -= yy[-5] * a[5];
      s -= yy[-6] * a[6];
      s -= yy[-7] * a[7];
      s -= yy[-8] * a[8];
      s -= yy[-9] * a[9];
      s -= yy[-10] * a[10];
      if ( labs( s ) < 0x7ffffff )
         *yy = ( s + 0x800L ) >> 12;
      else if ( s > 0 ) {
         *yy = 32767;
         overflow = 1;
      }
      else {
         *yy = -32768;
         overflow = 1;
      }
      yy++;
   }
   memcpy( y, &tmp[M], lg <<2 );

   /* Update of memory if update==1 */
   if ( update ) {
      memcpy( mem, &y[lg - M], 40 );
   }
   return overflow;
}

/*
 * Syn_filt_overflow
 *
 *
 * Parameters:
 *    a                 I: prediction coefficients [M+1]
 *    x                 I: input signal
 *    y                 O: output signal
 *    lg                I: size of filtering
 *    mem               B: memory associated with this filtering
 *    update            I: 0=no update, 1=update of memory.
 *
 * Function:
 *    Perform synthesis filtering through 1/A(z).
 *    Saturate after every multiplication.
 * Returns:
 *    void
 */
static void Syn_filt_overflow( Word32 a[], Word32 x[], Word32 y[], Word32 lg, Word32 mem[]
      , Word32 update )
{
   Word32 tmp[50];   /* malloc is slow */
   Word32 i, j, s, a0;
   Word32 *yy;


   /* Copy mem[] to yy[] */
   memcpy( tmp, mem, 40 );
   yy = tmp + M;
   a0 = a[0];

   /* Do the filtering. */
   for ( i = 0; i < lg; i++ ) {
      s = x[i] * a0;

      for ( j = 1; j <= M; j++ ) {
         s -= a[j] * yy[ - j];
         if (s > 1073741823){
            s = 1073741823;
         }
         else if ( s < -1073741824) {
            s = -1073741824;
         }
      }

      if ( labs( s ) < 0x7FFE800 )
         *yy = ( s + 0x800L ) >> 12;
      else if ( s > 0 ) {
         *yy = 32767;
      }
      else {
         *yy = -32768;
      }
      yy++;
   }
   memcpy( y, &tmp[M], lg <<2 );

   /* Update of memory if update==1 */
   if ( update ) {
      memcpy( mem, &y[lg - M], 40 );
   }
   return;
}

/*
 * dtx_dec
 *
 *
 * Parameters:
 *    st                            B: DTX state struct
 *    mem_syn                       I: AMR decoder state
 *    lsfState                      B: LSF state struct
 *    pred_state->past_qua_en       O: table of past quantized energies
 *    pred_state->past_qua_en_MR122 O: table of past quantized energies MR122
 *    averState->hangVar            O:
 *    averState->hangCount          O: hangover variable
 *    new_state                     I: new DTX state
 *    mode                          I: AMR mode
 *    parm                          I: vector of synthesis parameters
 *    synth                         O: synthesised speech
 *    A_t                           O: decoded LP filter in 4 subframes
 *
 * Function:
 *    DTX
 *
 * Returns:
 *    void
 */
static void dtx_dec( dtx_decState *st, Word32 *mem_syn, D_plsfState *lsfState,
      gc_predState *pred_state, Cb_gain_averageState *averState, enum
      DTXStateType new_state, enum Mode mode, Word16 parm[], Word32 synth[],
      Word32 A_t[] )
{
   Word32 ex[L_SUBFR], acoeff[11], acoeff_variab[M + 1], lsp_int[M];
   Word32 refl[M], lsf[M], lsf_int[M], lsf_int_variab[M], lsp_int_variab[M];
   Word32 i, j, int_fac, log_en_int, pred_err, log_pg_e, log_pg_m, log_pg;
   Word32 negative, lsf_mean, lsf_variab_index, lsf_variab_factor, ptr;
   Word16 log_en_index, log_en_int_e, log_en_int_m, level, ma_pred_init,
         tmp_int_length;


   if ( ( st->dtxHangoverAdded != 0 ) & ( st->sid_frame != 0 ) ) {
      /*
       * sidFirst after dtx hangover period
       * or sidUpd after dtxhangover
       */
      /* set log_en_adjust to correct value */
      st->log_en_adjust = dtx_log_en_adjust[mode];
      ptr = st->lsf_hist_ptr + M;

      if ( ptr == 80 ) {
         ptr = 0;
      }
      memcpy( &st->lsf_hist[ptr], &st->lsf_hist[st->lsf_hist_ptr], M <<2 );
      ptr = st->log_en_hist_ptr + 1;

      if ( ptr == DTX_HIST_SIZE ) {
         ptr = 0;
      }
      st->log_en_hist[ptr] = st->log_en_hist[st->log_en_hist_ptr];   /* Q11 */

      /*
       * compute mean log energy and lsp
       * from decoded signal (SID_FIRST)
       */
      st->log_en = 0;
      memset( lsf, 0, M <<2 );

      /* average energy and lsp */
      for ( i = 0; i < DTX_HIST_SIZE; i++ ) {
         st->log_en = st->log_en + ( st->log_en_hist[i] >> 3 );

         for ( j = 0; j < M; j++ ) {
            lsf[j] += st->lsf_hist[i * M + j];
         }
      }

      for ( j = 0; j < M; j++ ) {
         lsf[j] = lsf[j] >> 3;   /* divide by 8 */
      }
      Lsf_lsp( lsf, st->lsp );

      /*
       * make log_en speech coder mode independent
       * added again later before synthesis
       */
      st->log_en = st->log_en - st->log_en_adjust;

      /* compute lsf variability vector */
      memcpy( st->lsf_hist_mean, st->lsf_hist, 80 <<2 );

      for ( i = 0; i < M; i++ ) {
         lsf_mean = 0;

         /* compute mean lsf */
         for ( j = 0; j < 8; j++ ) {
            lsf_mean += st->lsf_hist_mean[i + j * M];
         }
         lsf_mean = lsf_mean >> 3;

         /*
          * subtract mean and limit to within reasonable limits
          * moreover the upper lsf's are attenuated
          */
         for ( j = 0; j < 8; j++ ) {
            /* subtract mean */
            st->lsf_hist_mean[i + j * M] = st->lsf_hist_mean[i + j * M] -
                  lsf_mean;

            /* attenuate deviation from mean, especially for upper lsf's */
            st->lsf_hist_mean[i + j * M] = ( st->lsf_hist_mean[i + j * M] *
                  lsf_hist_mean_scale[i] ) >> 15;

            /* limit the deviation */
            if ( st->lsf_hist_mean[i + j * M] < 0 ) {
               negative = 1;
            }
            else {
               negative = 0;
            }
            st->lsf_hist_mean[i + j * M] = labs( st->lsf_hist_mean[i + j * M] );

            /* apply soft limit */
            if ( st->lsf_hist_mean[i + j * M] > 655 ) {
               st->lsf_hist_mean[i + j * M] = 655 + ( ( st->lsf_hist_mean[i + j
                     * M] - 655 ) >> 2 );
            }

            /* apply hard limit */
            if ( st->lsf_hist_mean[i + j * M] > 1310 ) {
               st->lsf_hist_mean[i + j * M] = 1310;
            }

            if ( negative != 0 ) {
               st->lsf_hist_mean[i + j * M] = -st->lsf_hist_mean[i + j * M];
            }
         }
      }
   }

   if ( st->sid_frame != 0 ) {
      /*
       * Set old SID parameters, always shift
       * even if there is no new valid_data
       */
      memcpy( st->lsp_old, st->lsp, M <<2 );
      st->old_log_en = st->log_en;

      if ( st->valid_data != 0 ) /* new data available (no CRC) */ {
      /* Compute interpolation factor, since the division only works
       * for values of since_last_sid < 32 we have to limit the
       * interpolation to 32 frames
       */
         tmp_int_length = st->since_last_sid;
         st->since_last_sid = 0;

         if ( tmp_int_length > 32 ) {
            tmp_int_length = 32;
         }

         if ( tmp_int_length >= 2 ) {
            st->true_sid_period_inv = 0x2000000 / ( tmp_int_length
                  << 10 );
         }
         else {
            st->true_sid_period_inv = 16384;   /* 0.5 it Q15 */
         }
         memcpy( lsfState->past_r_q, &past_rq_init[parm[0] * M], M <<2 );
         D_plsf_3( lsfState, MRDTX, 0, &parm[1], st->lsp );

         /* reset for next speech frame */
         memset( lsfState->past_r_q, 0, M <<2 );
         log_en_index = parm[4];

         /* Q11 and divide by 4 */
         st->log_en = ( Word16 )( log_en_index << 9 );

         /* Subtract 2.5 in Q11 */
         st->log_en = ( Word16 )( st->log_en - 5120 );

         /* Index 0 is reserved for silence */
         if ( log_en_index == 0 ) {
            st->log_en = MIN_16;
         }

         /*
          * no interpolation at startup after coder reset
          * or when SID_UPD has been received right after SPEECH
          */
         if ( ( st->data_updated == 0 ) || ( st->dtxGlobalState == SPEECH ) ) {
            memcpy( st->lsp_old, st->lsp, M <<2 );
            st->old_log_en = st->log_en;
         }
      }   /* endif valid_data */

      /* initialize gain predictor memory of other modes */
      ma_pred_init = ( Word16 )( ( st->log_en >> 1 ) - 9000 );

      if ( ma_pred_init > 0 ) {
         ma_pred_init = 0;
      }

      if ( ma_pred_init < - 14436 ) {
         ma_pred_init = -14436;
      }
      pred_state->past_qua_en[0] = ma_pred_init;
      pred_state->past_qua_en[1] = ma_pred_init;
      pred_state->past_qua_en[2] = ma_pred_init;
      pred_state->past_qua_en[3] = ma_pred_init;

      /* past_qua_en for other modes than MR122 */
      ma_pred_init = ( Word16 )( ( 5443*ma_pred_init ) >> 15 );

      /* scale down by factor 20*log10(2) in Q15 */
      pred_state->past_qua_en_MR122[0] = ma_pred_init;
      pred_state->past_qua_en_MR122[1] = ma_pred_init;
      pred_state->past_qua_en_MR122[2] = ma_pred_init;
      pred_state->past_qua_en_MR122[3] = ma_pred_init;
   }   /* endif sid_frame */

   /*
    * CN generation
    * recompute level adjustment factor Q11
    * st->log_en_adjust = 0.9*st->log_en_adjust +
    *                     0.1*dtx_log_en_adjust[mode]);
    */
   st->log_en_adjust = ( Word16 )( ( ( st->log_en_adjust * 29491 ) >> 15 ) + ( (
         ( dtx_log_en_adjust[mode] << 5 ) * 3277 ) >> 20 ) );

   /* Interpolate SID info */
   /* Q10 */
   if ( st->since_last_sid > 30 )
      int_fac = 32767;
   else
      int_fac = ( Word16 )( (st->since_last_sid + 1) << 10 );

   /* Q10 * Q15 -> Q10 */
   int_fac = ( int_fac * st->true_sid_period_inv ) >> 15;

   /* Maximize to 1.0 in Q10 */
   if ( int_fac > 1024 ) {
      int_fac = 1024;
   }

   /* Q10 -> Q14 */
   int_fac = ( Word16 )( int_fac << 4 );

   /* Q14 * Q11->Q26 */
   log_en_int = ( int_fac * st->log_en ) << 1;

   for ( i = 0; i < M; i++ ) {
      /* Q14 * Q15 -> Q14 */
      lsp_int[i] = ( int_fac * st->lsp[i] ) >> 15;
   }

   /* 1-k in Q14 */
   int_fac = 16384 - int_fac;

   /* (Q14 * Q11 -> Q26) + Q26 -> Q26 */
   log_en_int += ( int_fac * st->old_log_en ) << 1;

   for ( i = 0; i < M; i++ ) {
      /* Q14 + (Q14 * Q15 -> Q14) -> Q14 */
      lsp_int[i] = lsp_int[i] + ( ( int_fac * st->lsp_old[i] ) >> 15 );

      /* Q14 -> Q15 */
      lsp_int[i] = lsp_int[i] << 1;
   }

   /* compute the amount of lsf variability */
   /* -0.6 in Q12 */
   lsf_variab_factor = st->log_pg_mean - 2457;

   /* *0.3 Q12*Q15 -> Q12 */
   lsf_variab_factor = 4096 - ( ( lsf_variab_factor * 9830 ) >> 15 );

   /* limit to values between 0..1 in Q12 */
   if ( lsf_variab_factor >= 4096 ) {
      lsf_variab_factor = 32767;
   }
   else if ( lsf_variab_factor < 0 ) {
      lsf_variab_factor = 0;
   }
   else
      lsf_variab_factor = lsf_variab_factor << 3;   /* -> Q15 */

   /* get index of vector to do variability with */
   lsf_variab_index = pseudonoise( &st->pn_seed_rx, 3 );

   /* convert to lsf */
   Lsp_lsf( lsp_int, lsf_int );

   /* apply lsf variability */
   memcpy( lsf_int_variab, lsf_int, M <<2 );

   for ( i = 0; i < M; i++ ) {
      lsf_int_variab[i] = lsf_int_variab[i] + ( ( lsf_variab_factor * st->
            lsf_hist_mean[i + lsf_variab_index * M] ) >> 15 );
   }

   /* make sure that LSP's are ordered */
   Reorder_lsf( lsf_int, LSF_GAP );
   Reorder_lsf( lsf_int_variab, LSF_GAP );

   /* copy lsf to speech decoders lsf state */
   memcpy( lsfState->past_lsf_q, lsf_int, M <<2 );

   /* convert to lsp */
   Lsf_lsp( lsf_int, lsp_int );
   Lsf_lsp( lsf_int_variab, lsp_int_variab );

     /* Compute acoeffs Q12 acoeff is used for level
      * normalization and Post_Filter, acoeff_variab is
      * used for synthesis filter
      * by doing this we make sure that the level
      * in high frequenncies does not jump up and down
      */
   Lsp_Az( lsp_int, acoeff );
   Lsp_Az( lsp_int_variab, acoeff_variab );

   /* For use in Post_Filter */
   memcpy( &A_t[0], acoeff, MP1 <<2 );
   memcpy( &A_t[MP1], acoeff, MP1 <<2 );
   memcpy( &A_t[MP1 <<1], acoeff, MP1 <<2 );
   memcpy( &A_t[MP1 + MP1 + MP1], acoeff, MP1 <<2 );

   /* Compute reflection coefficients Q15 */
   A_Refl( &acoeff[1], refl );

   /* Compute prediction error in Q15 */
   /* 0.99997 in Q15 */
   pred_err = MAX_16;

   for ( i = 0; i < M; i++ ) {
      pred_err = ( pred_err * ( MAX_16 - ( ( refl[i] * refl[i] ) >> 15 ) ) ) >>
            15;
   }

   /* compute logarithm of prediction gain */
   Log2( pred_err, &log_pg_e, &log_pg_m );

   /* convert exponent and mantissa to Word16 Q12 */
   /* Q12 */
   log_pg = ( log_pg_e - 15 ) << 12;
   /* saturate */
   if (log_pg < -32768) {
      log_pg = -32768;
   }
   log_pg = ( -( log_pg + ( log_pg_m >> 3 ) ) ) >> 1;
   st->log_pg_mean = ( Word16 )( ( ( 29491*st->log_pg_mean ) >> 15 ) + ( ( 3277
         * log_pg ) >> 15 ) );

   /* Compute interpolated log energy */
   /* Q26 -> Q16 */
   log_en_int = log_en_int >> 10;

   /* Add 4 in Q16 */
   log_en_int += 262144L;

   /* subtract prediction gain */
   log_en_int = log_en_int - ( log_pg << 4 );

   /* adjust level to speech coder mode */
   log_en_int += st->log_en_adjust << 5;
   log_en_int_e = ( Word16 )( log_en_int >> 16 );
   log_en_int_m = ( Word16 )( ( log_en_int - ( log_en_int_e << 16 ) ) >> 1 );

   /* Q4 */
   level = ( Word16 )( Pow2( log_en_int_e, log_en_int_m ) );

   for ( i = 0; i < 4; i++ ) {
      /* Compute innovation vector */
      Build_CN_code( &st->pn_seed_rx, ex );

      for ( j = 0; j < L_SUBFR; j++ ) {
         ex[j] = ( level * ex[j] ) >> 15;
      }

      /* Synthesize */
      Syn_filt( acoeff_variab, ex, &synth[i * L_SUBFR], L_SUBFR, mem_syn, 1 );
   }   /* next i */

   /* reset codebook averaging variables */
   averState->hangVar = 20;
   averState->hangCount = 0;

   if ( new_state == DTX_MUTE ) {
        /*
         * mute comfort noise as it has been quite a long time since
         * last SID update  was performed
         */
      Word32 num, denom;


      tmp_int_length = st->since_last_sid;

      if ( tmp_int_length > 32 ) {
         tmp_int_length = 32;
      }

      if ( tmp_int_length == 1 ) {
         st->true_sid_period_inv = MAX_16;
      }
      else {
         num = 1024;
         denom = ( tmp_int_length << 10 );
         st->true_sid_period_inv = 0;

         for ( i = 0; i < 15; i++ ) {
            st->true_sid_period_inv <<= 1;
            num <<= 1;

            if ( num >= denom ) {
               num = num - denom;
               st->true_sid_period_inv += 1;
            }
         }
      }
      st->since_last_sid = 0;
      memcpy( st->lsp_old, st->lsp, M << 2 );
      st->old_log_en = st->log_en;

      /* subtract 1/8 in Q11 i.e -6/8 dB */
      st->log_en = st->log_en - 256;
      if (st->log_en < -32768) st->log_en = -32768;
   }

     /*
      * reset interpolation length timer
      * if data has been updated.
      */
   if ( ( st->sid_frame != 0 ) & ( ( st->valid_data != 0 ) || ( ( st->valid_data
         == 0 ) & ( st->dtxHangoverAdded != 0 ) ) ) ) {
      st->since_last_sid = 0;
      st->data_updated = 1;
   }
   return;
}


/*
 * lsp_avg
 *
 *
 * Parameters:
 *    st->lsp_meanSave  B: LSP averages
 *    lsp               I: LSPs
 *
 * Function:
 *    Calculate the LSP averages
 *
 * Returns:
 *    void
 */
static void lsp_avg( lsp_avgState *st, Word32 *lsp )
{
   Word32 i, tmp;


   for ( i = 0; i < M; i++ ) {
      /* mean = 0.84*mean */
      tmp = ( st->lsp_meanSave[i] << 16 );
      tmp -= ( EXPCONST * st->lsp_meanSave[i] ) << 1;

      /* Add 0.16 of newest LSPs to mean */
      tmp += ( EXPCONST * lsp[i] ) << 1;

      /* Save means */
      tmp += 0x00008000L;
      st->lsp_meanSave[i] = tmp >> 16;
   }
   return;
}


/*
 * Int_lpc_1and3
 *
 *
 * Parameters:
 *    lsp_old        I: LSP vector at the 4th subfr. of past frame      [M]
 *    lsp_mid        I: LSP vector at the 2nd subframe of present frame [M]
 *    lsp_new        I: LSP vector at the 4th subframe of present frame [M]
 *    Az             O: interpolated LP parameters in subframes 1 and 3
 *                                                                   [AZ_SIZE]
 *
 * Function:
 *    Interpolates the LSPs and converts to LPC parameters
 *    to get a different LP filter in each subframe.
 *
 *    The 20 ms speech frame is divided into 4 subframes.
 *    The LSPs are quantized and transmitted at the 2nd and
 *    4th subframes (twice per frame) and interpolated at the
 *    1st and 3rd subframe.
 *
 * Returns:
 *    void
 */
static void Int_lpc_1and3( Word32 lsp_old[], Word32 lsp_mid[], Word32 lsp_new[],
      Word32 Az[] )
{
   Word32 lsp[M];
   Word32 i;


   /* lsp[i] = lsp_mid[i] * 0.5 + lsp_old[i] * 0.5 */
   for ( i = 0; i < 10; i++ ) {
      lsp[i] = ( lsp_mid[i] >> 1 ) + ( lsp_old[i] >> 1 );
   }

   /* Subframe 1 */
   Lsp_Az( lsp, Az );
   Az += MP1;

   /* Subframe 2 */
   Lsp_Az( lsp_mid, Az );
   Az += MP1;

   for ( i = 0; i < 10; i++ ) {
      lsp[i] = ( lsp_mid[i] >> 1 ) + ( lsp_new[i] >> 1 );
   }

   /* Subframe 3 */
   Lsp_Az( lsp, Az );
   Az += MP1;

   /* Subframe 4 */
   Lsp_Az( lsp_new, Az );
   return;
}


/*
 * Int_lpc_1to3
 *
 *
 * Parameters:
 *    lsp_old           I: LSP vector at the 4th subframe of past frame    [M]
 *    lsp_new           I: LSP vector at the 4th subframe of present frame [M]
 *    Az                O: interpolated LP parameters in all subframes
 *                                                                   [AZ_SIZE]
 *
 * Function:
 *    Interpolates the LSPs and converts to LPC parameters to get a different
 *    LP filter in each subframe.
 *
 *    The 20 ms speech frame is divided into 4 subframes.
 *    The LSPs are quantized and transmitted at the 4th
 *    subframes (once per frame) and interpolated at the
 *    1st, 2nd and 3rd subframe.
 *
 * Returns:
 *    void
 */
static void Int_lpc_1to3( Word32 lsp_old[], Word32 lsp_new[], Word32 Az[] )
{
   Word32 lsp[M];
   Word32 i;


   for ( i = 0; i < 10; i++ ) {
      lsp[i] = ( lsp_new[i] >> 2 ) + ( lsp_old[i] - ( lsp_old[i] >> 2 ) );
   }

   /* Subframe 1 */
   Lsp_Az( lsp, Az );
   Az += MP1;

   for ( i = 0; i < 10; i++ ) {
      lsp[i] = ( lsp_old[i] >> 1 ) + ( lsp_new[i] >> 1 );
   }

   /* Subframe 2 */
   Lsp_Az( lsp, Az );
   Az += MP1;

   for ( i = 0; i < 10; i++ ) {
      lsp[i] = ( lsp_old[i] >> 2 ) + ( lsp_new[i] - ( lsp_new[i] >> 2 ) );
   }

   /* Subframe 3 */
   Lsp_Az( lsp, Az );
   Az += MP1;

   /* Subframe 4 */
   Lsp_Az( lsp_new, Az );
   return;
}


/*
 * D_plsf_5
 *
 *
 * Parameters:
 *    st->past_lsf_q I: Past dequantized LFSs
 *    st->past_r_q      B: past quantized residual
 *    bfi               B: bad frame indicator
 *    indice            I: quantization indices of 3 submatrices, Q0
 *    lsp1_q            O: quantized 1st LSP vector
 *    lsp2_q            O: quantized 2nd LSP vector
 *
 * Function:
 *    Decodes the 2 sets of LSP parameters in a frame
 *    using the received quantization indices.
 *
 * Returns:
 *    void
 */
static void D_plsf_5( D_plsfState *st, Word16 bfi, Word16 *indice, Word32 *lsp1_q
      , Word32 *lsp2_q )
{
   Word32 lsf1_r[M], lsf2_r[M], lsf1_q[M], lsf2_q[M];
   Word32 i, temp1, temp2, sign;
   const Word32 *p_dico;


   /* if bad frame */
   if ( bfi != 0 ) {
      /* use the past LSFs slightly shifted towards their mean */
      for ( i = 0; i < M; i += 2 ) {
         /* lsfi_q[i] = ALPHA*st->past_lsf_q[i] + ONE_ALPHA*meanLsf[i]; */
         lsf1_q[i] = ( ( st->past_lsf_q[i] * ALPHA_122 ) >> 15 ) + ( ( mean_lsf_5[i]
               * ONE_ALPHA_122 ) >> 15 );
         lsf1_q[i + 1] = ( ( st->past_lsf_q[i + 1] * ALPHA_122 ) >> 15 ) + ( (
               mean_lsf_5[i + 1] * ONE_ALPHA_122 ) >> 15 );
      }
      memcpy( lsf2_q, lsf1_q, M <<2 );

      /* estimate past quantized residual to be used in next frame */
      for ( i = 0; i < M; i += 2 ) {
         /* temp  = meanLsf[i] +  st->past_r_q[i] * LSPPpred_facMR122; */
         temp1 = mean_lsf_5[i] + ( ( st->past_r_q[i] * LSP_PRED_FAC_MR122 ) >>
               15 );
         temp2 = mean_lsf_5[i + 1] +( ( st->past_r_q[i + 1] *LSP_PRED_FAC_MR122
               ) >> 15 );
         st->past_r_q[i] = lsf2_q[i] - temp1;
         st->past_r_q[i + 1] = lsf2_q[i + 1] -temp2;
      }
   }

   /* if good LSFs received */
   else {
      /* decode prediction residuals from 5 received indices */
      p_dico = &dico1_lsf_5[indice[0] << 2];
      lsf1_r[0] = *p_dico++;
      lsf1_r[1] = *p_dico++;
      lsf2_r[0] = *p_dico++;
      lsf2_r[1] = *p_dico++;
      p_dico = &dico2_lsf_5[indice[1] << 2];
      lsf1_r[2] = *p_dico++;
      lsf1_r[3] = *p_dico++;
      lsf2_r[2] = *p_dico++;
      lsf2_r[3] = *p_dico++;
      sign = ( Word16 )( indice[2] & 1 );
      i = indice[2] >> 1;
      p_dico = &dico3_lsf_5[i << 2];

      if ( sign == 0 ) {
         lsf1_r[4] = *p_dico++;
         lsf1_r[5] = *p_dico++;
         lsf2_r[4] = *p_dico++;
         lsf2_r[5] = *p_dico++;
      }
      else {
         lsf1_r[4] = ( Word16 )( -( *p_dico++ ) );
         lsf1_r[5] = ( Word16 )( -( *p_dico++ ) );
         lsf2_r[4] = ( Word16 )( -( *p_dico++ ) );
         lsf2_r[5] = ( Word16 )( -( *p_dico++ ) );
      }
      p_dico = &dico4_lsf_5[( indice[3]<<2 )];
      lsf1_r[6] = *p_dico++;
      lsf1_r[7] = *p_dico++;
      lsf2_r[6] = *p_dico++;
      lsf2_r[7] = *p_dico++;
      p_dico = &dico5_lsf_5[( indice[4]<<2 )];
      lsf1_r[8] = *p_dico++;
      lsf1_r[9] = *p_dico++;
      lsf2_r[8] = *p_dico++;
      lsf2_r[9] = *p_dico++;

      /* Compute quantized LSFs and update the past quantized residual */
      for ( i = 0; i < M; i++ ) {
         temp1 = mean_lsf_5[i] + ( ( st->past_r_q[i] * LSP_PRED_FAC_MR122 ) >>
               15 );
         lsf1_q[i] = lsf1_r[i] + temp1;
         lsf2_q[i] = lsf2_r[i] + temp1;
         st->past_r_q[i] = lsf2_r[i];
      }
   }

   /* verification that LSFs have minimum distance of LSF_GAP Hz */
   Reorder_lsf( lsf1_q, LSF_GAP );
   Reorder_lsf( lsf2_q, LSF_GAP );
   memcpy( st->past_lsf_q, lsf2_q, M <<2 );

   /*  convert LSFs to the cosine domain */
   Lsf_lsp( lsf1_q, lsp1_q );
   Lsf_lsp( lsf2_q, lsp2_q );
   return;
}


/*
 * Dec_lag3
 *
 *
 * Parameters:
 *    index             I: received pitch index
 *    t0_min            I: minimum of search range
 *    t0_max            I: maximum of search range
 *    i_subfr           I: subframe flag
 *    T0_prev           I: integer pitch delay of last subframe used
 *                         in 2nd and 4th subframes
 *    T0                O: integer part of pitch lag
 *    T0_frac           O : fractional part of pitch lag
 *    flag4             I : flag for encoding with 4 bits
 * Function:
 *    Decoding of fractional pitch lag with 1/3 resolution.
 *    Extract the integer and fraction parts of the pitch lag from
 *    the received adaptive codebook index.
 *
 *    The fractional lag in 1st and 3rd subframes is encoded with 8 bits
 *    while that in 2nd and 4th subframes is relatively encoded with 4, 5
 *    and 6 bits depending on the mode.
 *
 * Returns:
 *    void
 */
static void Dec_lag3( Word32 index, Word32 t0_min, Word32 t0_max, Word32 i_subfr
      , Word32 T0_prev, Word32 *T0, Word32 *T0_frac, Word32 flag4 )
{
   Word32 i, tmp_lag;


   /* if 1st or 3rd subframe */
   if ( i_subfr == 0 ) {
      if ( index < 197 ) {
         *T0 = ( ( ( index + 2 ) * 10923 ) >> 15 ) + 19;
         i = *T0 + *T0 + *T0;
         *T0_frac = ( index - i ) + 58;
      }
      else {
         *T0 = index - 112;
         *T0_frac = 0;
      }
   }

   /* 2nd or 4th subframe */
   else {
      if ( flag4 == 0 ) {
         /* 'normal' decoding: either with 5 or 6 bit resolution */
         i = ( ( ( index + 2 ) * 10923 ) >> 15 ) - 1;
         *T0 = i + t0_min;
         i = i + i + i;
         *T0_frac = ( index - 2 ) - i;
      }
      else {
         /* decoding with 4 bit resolution */
         tmp_lag = T0_prev;

         if ( ( tmp_lag - t0_min ) > 5 )
            tmp_lag = t0_min + 5;

         if ( ( t0_max - tmp_lag ) > 4 )
            tmp_lag = t0_max - 4;

         if ( index < 4 ) {
            i = ( tmp_lag - 5 );
            *T0 = i + index;
            *T0_frac = 0;
         }
         else {
            if ( index < 12 ) {
               i = ( ( ( index - 5 ) * 10923 ) >> 15 ) - 1;
               *T0 = i + tmp_lag;
               i = i + i + i;
               *T0_frac = ( index - 9 ) - i;
            }
            else {
               i = ( index - 12 ) + tmp_lag;
               *T0 = i + 1;
               *T0_frac = 0;
            }
         }
      }   /* end if (decoding with 4 bit resolution) */
   }
   return;
}


/*
 * Pred_lt_3or6_40
 *
 *
 * Parameters:
 *    exc               B: excitation buffer
 *    T0                I: integer pitch lag
 *    frac              I: fraction of lag
 *    flag3             I: if set, upsampling rate = 3 (6 otherwise)
 *
 * Function:
 *    Compute the result of long term prediction with fractional
 *    interpolation of resolution 1/3 or 1/6. (Interpolated past excitation).
 *
 *    Once the fractional pitch lag is determined,
 *    the adaptive codebook vector v(n) is computed by interpolating
 *    the past excitation signal u(n) at the given integer delay k
 *    and phase (fraction)  :
 *
 *          9                       9
 *    v(n) = SUM[ u(n-k-i) * b60(t+i*6) ] + SUM[ u(n-k+1+i) * b60(6-t+i*6) ],
 *          i=0                       i=0
 *    n = 0, ...,39, t = 0, ...,5.
 *
 *    The interpolation filter b60 is based on a Hamming windowed sin(x)/x
 *    function truncated at ± 59 and padded with zeros at ± 60 (b60(60)=0)).
 *    The filter has a cut-off frequency (-3 dB) at 3 600 Hz in
 *    the over-sampled domain.
 *
 * Returns:
 *    void
 */
static void Pred_lt_3or6_40( Word32 exc[], Word32 T0, Word32 frac, Word32 flag3 )
{
   Word32 s, i;
   Word32 *x0, *x1, *x2;
   const Word32 *c1, *c2;


   x0 = &exc[ - T0];
   frac = -frac;

   if ( flag3 != 0 ) {
      frac <<= 1;   /* inter_3l[k] = inter6[2*k] -> k' = 2*k */
   }

   if ( frac < 0 ) {
      frac += 6;
      x0--;
   }
   c1 = &inter6[frac];
   c2 = &inter6[6 - frac];

   for ( i = 0; i < 40; i++ ) {
      x1 = x0++;
      x2 = x0;
      s = x1[0] * c1[0];
      s += x1[ - 1] * c1[6];
      s += x1[ - 2] * c1[12];
      s += x1[ - 3] * c1[18];
      s += x1[ - 4] * c1[24];
      s += x1[ - 5] * c1[30];
      s += x1[ - 6] * c1[36];
      s += x1[ - 7] * c1[42];
      s += x1[ - 8] * c1[48];
      s += x1[ - 9] * c1[54];
      s += x2[0] * c2[0];
      s += x2[1] * c2[6];
      s += x2[2] * c2[12];
      s += x2[3] * c2[18];
      s += x2[4] * c2[24];
      s += x2[5] * c2[30];
      s += x2[6] * c2[36];
      s += x2[7] * c2[42];
      s += x2[8] * c2[48];
      s += x2[9] * c2[54];
      exc[i] = ( s + 0x4000 ) >> 15;

   }
}


/*
 * Dec_lag6
 *
 *
 * Parameters:
 *    index             I: received pitch index
 *    pit_min           I: minimum pitch lag
 *    pit_max           I: maximum pitch lag
 *    i_subfr           I: subframe flag
 *    T0                B: integer part of pitch lag
 *    T0_frac           O : fractional part of pitch lag
 *
 * Function:
 *    Decoding of fractional pitch lag with 1/6 resolution.
 *    Extract the integer and fraction parts of the pitch lag from
 *    the received adaptive codebook index.
 *
 *    The fractional lag in 1st and 3rd subframes is encoded with 9 bits
 *    while that in 2nd and 4th subframes is relatively encoded with 6 bits.
 *    Note that in relative encoding only 61 values are used. If the
 *    decoder receives 61, 62, or 63 as the relative pitch index, it means
 *    that a transmission error occurred. In this case, the pitch lag from
 *    previous subframe (actually from previous frame) is used.
 *
 * Returns:
 *    void
 */
static void Dec_lag6( Word32 index, Word32 pit_min, Word32 pit_max, Word32
      i_subfr, Word32 *T0, Word32 *T0_frac )
{
   Word32 t0_min, t0_max, i;


   /* if 1st or 3rd subframe */
   if ( i_subfr == 0 ) {
      if ( index < 463 ) {
         /* T0 = (index+5)/6 + 17 */
         *T0 = ( index + 5 ) / 6 + 17;
         i = *T0 + *T0 + *T0;

         /* *T0_frac = index - T0*6 + 105 */
         *T0_frac = ( index - ( i + i ) ) + 105;
      }
      else {
         *T0 = index - 368;
         *T0_frac = 0;
      }
   }

   /* second or fourth subframe */
   else {
      /* find t0_min and t0_max for 2nd (or 4th) subframe */
      t0_min = *T0 - 5;

      if ( t0_min < pit_min ) {
         t0_min = pit_min;
      }
      t0_max = t0_min + 9;

      if ( t0_max > pit_max ) {
         t0_max = pit_max;
         t0_min = t0_max - 9;
      }

      /* i = (index+5)/6 - 1 */
      i = ( index + 5 ) / 6 - 1;
      *T0 = i + t0_min;
      i = i + i + i;
      *T0_frac = ( index - 3 ) - ( i + i );
   }
}


/*
 * decompress10
 *
 *
 * Parameters:
 *    MSBs              I: MSB part of the index
 *    LSBs              I: LSB part of the index
 *    index1            I: index for first pos in posIndex
 *    index2            I: index for second pos in posIndex
 *    index3            I: index for third pos in posIndex
 *    pos_indx          O: position of 3 pulses (decompressed)
 * Function:
 *    Decompression of the linear codeword
 *
 * Returns:
 *    void
 */
static void decompress10( Word32 MSBs, Word32 LSBs, Word32 index1, Word32 index2
      , Word32 index3, Word32 pos_indx[] )
{
   Word32 divMSB;

   if (MSBs > 124)
   {
      MSBs = 124;
   }
   /*
    * pos_indx[index1] = ((MSBs-25*(MSBs/25))%5)*2 + (LSBs-4*(LSBs/4))%2;
    * pos_indx[index2] = ((MSBs-25*(MSBs/25))/5)*2 + (LSBs-4*(LSBs/4))/2;
    * pos_indx[index3] = (MSBs/25)*2 + LSBs/4;
    */
   divMSB = MSBs / 25;
   pos_indx[index1] = ( ( ( MSBs - 25 * ( divMSB ) ) % 5 ) << 1 ) + ( LSBs & 0x1
         );
   pos_indx[index2] = ( ( ( MSBs - 25 * ( divMSB ) ) / 5 ) << 1 ) + ( ( LSBs &
         0x2 ) >> 1 );
   pos_indx[index3] = ( divMSB << 1 ) + ( LSBs >> 2 );
   return;
}


/*
 * decompress_codewords
 *
 *
 * Parameters:
 *    indx              I: position of 8 pulses (compressed)
 *    pos_indx          O: position index of 8 pulses (position only)
 *
 * Function:
 *    Decompression of the linear codewords to 4+three indeces
 *    one bit from each pulse is made robust to errors by
 *    minimizing the phase shift of a bit error.
 *
 *    i0,i4,i1 => one index (7+3) bits, 3   LSBs more robust
 *    i2,i6,i5 => one index (7+3) bits, 3   LSBs more robust
 *    i3,i7    => one index (5+2) bits, 2-3 LSbs more robust
 *
 * Returns:
 *    void
 */
static void decompress_codewords( Word16 indx[], Word32 pos_indx[] )
{
   Word32 ia, ib, MSBs, LSBs, MSBs0_24, tmp;


    /*
     * First index: 10x10x10 -> 2x5x2x5x2x5-> 125x2x2x2 -> 7+1x3 bits
     * MSBs = indx[NB_TRACK]/8;
     * LSBs = indx[NB_TRACK]%8;
     */
   MSBs = *indx >> 3;
   LSBs = *indx & 0x7;
   decompress10( MSBs, LSBs, 0, 4, 1, pos_indx );

    /*
     * Second index: 10x10x10 -> 2x5x2x5x2x5-> 125x2x2x2 -> 7+1x3 bits
     * MSBs = indx[NB_TRACK+1]/8;
     * LSBs = indx[NB_TRACK+1]%8;
     */
   MSBs = indx[1] >> 3;
   LSBs = indx[1] & 0x7;
   decompress10( MSBs, LSBs, 2, 6, 5, pos_indx );

    /*
     * Third index: 10x10 -> 2x5x2x5-> 25x2x2 -> 5+1x2 bits
     * MSBs = indx[NB_TRACK+2]/4;
     * LSBs = indx[NB_TRACK+2]%4;
     * MSBs0_24 = (MSBs*25+12)/32;
     * if ((MSBs0_24/5)%2==1)
     *    pos_indx[3] = (4-(MSBs0_24%5))*2 + LSBs%2;
     * else
     *    pos_indx[3] = (MSBs0_24%5)*2 + LSBs%2;
     * pos_indx[7] = (MSBs0_24/5)*2 + LSBs/2;
     */
   MSBs = indx[2] >> 2;
   LSBs = indx[2] & 0x3;
   MSBs0_24 = ( ( ( MSBs * 25 ) + 12 ) >> 5 );
   tmp = ( MSBs0_24 * 6554 ) >> 15;
   ia = tmp & 0x1;
   ib = ( MSBs0_24 - ( tmp * 5 ) );

   if ( ia == 1 ) {
      ib = 4 - ib;
   }
   pos_indx[3] = ( ib << 1 ) + ( LSBs & 0x1 );
   pos_indx[7] = ( tmp << 1 ) + ( LSBs >> 1 );
}


/*
 * decode_2i40_9bits
 *
 *
 * Parameters:
 *    subNr             I: subframe number
 *    sign              I: signs of 2 pulses
 *    index             I: Positions of the 2 pulses
 *    cod               O: algebraic (fixed) codebook excitation
 *
 * Function:
 *    Algebraic codebook decoder
 *
 * Returns:
 *    void
 */
static void decode_2i40_9bits( Word32 subNr, Word32 sign, Word32 index, Word32
      cod[] )
{
   Word32 pos[2];
   Word32 i, j, k;


   /* Decode the positions */
   /* table bit  is the MSB */
   j = ( index & 64 ) >> 6;
   i = index & 7;

   /* pos0 =i*5+startPos[j*8+subNr*2] */
   i = ( i + ( i << 2 ) );
   k = startPos[( j <<3 )+( subNr << 1 )];
   pos[0] = i + k;
   index = index >> 3;
   i = index & 7;

   /* pos1 =i*5+startPos[j*8+subNr*2+1] */
   i = ( i + ( i << 2 ) );
   k = startPos[( ( j <<3 )+ ( subNr <<1 ) ) + 1];
   pos[1] = ( Word16 )( i + k );

   /* decode the signs  and build the codeword */
   memset( cod, 0, L_SUBFR <<2 );

   for ( j = 0; j < 2; j++ ) {
      i = sign & 1;
      sign = sign >> 1;

      if ( i != 0 ) {
         cod[pos[j]] = 8191;   /* +1.0 */
      }
      else {
         cod[pos[j]] = -8192;   /* -1.0 */
      }
   }
   return;
}


/*
 * decode_2i40_11bits
 *
 *
 * Parameters:
 *    sign              I: signs of 2 pulses
 *    index             I: Positions of the 2 pulses
 *    cod               O: algebraic (fixed) codebook excitation
 *
 * Function:
 *    Algebraic codebook decoder
 *
 * Returns:
 *    void
 */
static void decode_2i40_11bits( Word32 sign, Word32 index, Word32 cod[] )
{
   Word32 pos[2];
   Word32 i, j;


   /* Decode the positions */
   j = index & 1;
   index = index >> 1;
   i = index & 7;

   /* pos0 =i*5+1+j*2 */
   i = ( i + ( i << 2 ) );
   i = ( i + 1 );
   j = ( j << 1 );
   pos[0] = i + j;
   index = index >> 3;
   j = index & 3;
   index = index >> 2;
   i = index & 7;

   if ( j == 3 ) {
      /* pos1 =i*5+4 */
      i = ( i + ( i << 2 ) );
      pos[1] = i + 4;
   }
   else {
      /* pos1 =i*5+j */
      i = ( i + ( i << 2 ) );
      pos[1] = i + j;
   }

   /* decode the signs  and build the codeword */
   memset( cod, 0, L_SUBFR <<2 );

   for ( j = 0; j < 2; j++ ) {
      i = sign & 1;
      sign = sign >> 1;

      if ( i != 0 ) {
         cod[pos[j]] = 8191;   /* +1.0 */
      }
      else {
         cod[pos[j]] = -8192;   /* -1.0 */
      }
   }
   return;
}


/*
 * decode_3i40_14bits
 *
 *
 * Parameters:
 *    sign              I: signs of 3 pulses
 *    index             I: Positions of the 3 pulses
 *    cod               O: algebraic (fixed) codebook excitation
 *
 * Function:
 *    Algebraic codebook decoder
 *
 * Returns:
 *    void
 */
static void decode_3i40_14bits( Word32 sign, Word32 index, Word32 cod[] )
{
   Word32 pos[3];
   Word32 i, j;


   /* Decode the positions */
   i = index & 7;

   /* pos0 =i*5 */
   pos[0] = i + ( i << 2 );
   index = index >> 3;
   j = index & 1;
   index = index >> 1;
   i = index & 7;

   /* pos1 =i*5+1+j*2 */
   i = ( i + ( i << 2 ) );
   i = ( i + 1 );
   j = ( j << 1 );
   pos[1] = i + j;
   index = index >> 3;
   j = index & 1;
   index = index >> 1;
   i = index & 7;

   /* pos2 =i*5+2+j*2 */
   i = ( i + ( i << 2 ) );
   i = ( i + 2 );
   j = ( j << 1 );
   pos[2] = i + j;

   /* decode the signs  and build the codeword */
   memset( cod, 0, L_SUBFR <<2 );

   for ( j = 0; j < 3; j++ ) {
      i = sign & 1;
      sign = sign >> 1;

      if ( i > 0 ) {
         cod[pos[j]] = 8191;   /* +1.0 */
      }
      else {
         cod[pos[j]] = -8192;   /* -1.0 */
      }
   }
   return;
}


/*
 * decode_3i40_14bits
 *
 *
 * Parameters:
 *    sign              I: signs of 4 pulses
 *    index             I: Positions of the 4 pulses
 *    cod               O: algebraic (fixed) codebook excitation
 *
 * Function:
 *    Algebraic codebook decoder
 *
 * Returns:
 *    void
 */
static void decode_4i40_17bits( Word32 sign, Word32 index, Word32 cod[] )
{
   Word32 pos[4];
   Word32 i, j;


   /* Decode the positions */
   i = index & 7;
   i = dgray[i];

   /* pos0 =i*5 */
   pos[0] = i + ( i << 2 );
   index = index >> 3;
   i = index & 7;
   i = dgray[i];

   /* pos1 =i*5+1 */
   i = ( i + ( i << 2 ) );
   pos[1] = i + 1;
   index = index >> 3;
   i = index & 7;
   i = dgray[i];

   /* pos2 =i*5+1 */
   i = ( i + ( i << 2 ) );
   pos[2] = i + 2;
   index = index >> 3;
   j = index & 1;
   index = index >> 1;
   i = index & 7;
   i = dgray[i];

   /* pos3 =i*5+3+j */
   i = ( i + ( i << 2 ) );
   i = ( i + 3 );
   pos[3] = i + j;

   /* decode the signs  and build the codeword */
   memset( cod, 0, L_SUBFR <<2 );

   for ( j = 0; j < 4; j++ ) {
      i = sign & 1;
      sign = sign >> 1;

      if ( i != 0 ) {
         cod[pos[j]] = 8191;
      }
      else {
         cod[pos[j]] = -8192;
      }
   }
   return;
}


/*
 * decode_8i40_31bits
 *
 *
 * Parameters:
 *    index             I: index of 8 pulses (sign+position)
 *    cod               O: algebraic (fixed) codebook excitation
 *
 * Function:
 *    Algebraic codebook decoder
 *
 * Returns:
 *    void
 */
static void decode_8i40_31bits( Word16 index[], Word32 cod[] )
{
   Word32 linear_codewords[8];
   Word32 i, j, pos1, pos2, sign;


   memset( cod, 0, L_CODE <<2 );
   decompress_codewords( &index[NB_TRACK_MR102], linear_codewords );

   /* decode the positions and signs of pulses and build the codeword */
   for ( j = 0; j < NB_TRACK_MR102; j++ ) {
      /* compute index i */
      i = linear_codewords[j];
      i <<= 2;

      /* position of pulse "j" */
      pos1 = i + j;

      if ( index[j] == 0 ) {
         sign = POS_CODE;   /* +1.0 */
      }
      else {
         sign = -NEG_CODE;   /* -1.0 */
      }

      /* compute index i */
      i = linear_codewords[j + 4];
      i = i << 2;

      /* position of pulse "j+4" */
      pos2 = i + j;
      cod[pos1] = sign;

      if ( pos2 < pos1 ) {
         sign = -( sign );
      }
      cod[pos2] = cod[pos2] + sign;
   }
   return;
}


/*
 * decode_10i40_35bits
 *
 *
 * Parameters:
 *    index             I: index of 10 pulses (sign+position)
 *    cod               O: algebraic (fixed) codebook excitation
 *
 * Function:
 *    Algebraic codebook decoder
 *
 * Returns:
 *    void
 */
static void decode_10i40_35bits( Word16 index[], Word32 cod[] )
{
   Word32 i, j, pos1, pos2, sign, tmp;


   memset( cod, 0, L_CODE <<2 );

   /* decode the positions and signs of pulses and build the codeword */
   for ( j = 0; j < 5; j++ ) {
      /* compute index i */
      tmp = index[j];
      i = tmp & 7;
      i = dgray[i];
      i = ( i * 5 );

      /* position of pulse "j" */
      pos1 = ( i + j );
      i = ( tmp >> 3 ) & 1;

      if ( i == 0 ) {
         sign = 4096;   /* +1.0 */
      }
      else {
         sign = -4096;   /* -1.0 */
      }

      /* compute index i */
      i = index[j + 5] & 7;
      i = dgray[i];
      i = i * 5;

      /* position of pulse "j+5" */
      pos2 = ( i + j );
      cod[pos1] = sign;

      if ( pos2 < pos1 ) {
         sign = -( sign );
      }
      cod[pos2] = cod[pos2] + sign;
   }
   return;
}


/*
 * gmed_n
 *
 *
 * Parameters:
 *    ind               I: values
 *    n                 I: The number of gains (odd)
 *
 * Function:
 *    Calculates N-point median.
 *
 * Returns:
 *    index of the median value
 */
static Word32 gmed_n( Word32 ind[], Word32 n )
{
   Word32 tmp[NMAX], tmp2[NMAX];
   Word32 max, medianIndex, i, j, ix = 0;


   for ( i = 0; i < n; i++ ) {
      tmp2[i] = ind[i];
   }

   for ( i = 0; i < n; i++ ) {
      max = -32767;

      for ( j = 0; j < n; j++ ) {
         if ( tmp2[j] >= max ) {
            max = tmp2[j];
            ix = j;
         }
      }
      tmp2[ix] = -32768;
      tmp[i] = ix;
   }
   medianIndex = tmp[( n >>1 )];
   return( ind[medianIndex] );
}


/*
 * ec_gain_pitch
 *
 *
 * Parameters:
 *    st->pbuf          I: last five gains
 *    st->past_gain_pit I: past gain
 *    state             I: state of the state machine
 *    gain_pitch        O: pitch gain
 *
 * Function:
 *    Calculates pitch from previous values.
 *
 * Returns:
 *    void
 */
static void ec_gain_pitch( ec_gain_pitchState *st, Word16 state, Word32 *
      gain_pitch )
{
   Word32 tmp;


   /* calculate median of last five gains */
   tmp = gmed_n( st->pbuf, 5 );

   /* new gain = minimum(median, past_gain) * pdown[state] */
   if ( tmp > st->past_gain_pit ) {
      tmp = st->past_gain_pit;
   }
   *gain_pitch = ( tmp * pdown[state] ) >> 15;
}


/*
 * d_gain_pitch
 *
 *
 * Parameters:
 *    mode              I: AMR mode
 *    index             I: index of quantization
 *
 * Function:
 *    Decodes the pitch gain using the received index
 *
 * Returns:
 *    gain
 */
static Word32 d_gain_pitch( enum Mode mode, Word32 index )
{
   Word32 gain;


   if ( mode == MR122 ) {
      /* clear 2 LSBits */
      gain = ( qua_gain_pitch[index] >> 2 ) << 2;
   }
   else {
      gain = qua_gain_pitch[index];
   }
   return gain;
}


/*
 * ec_gain_pitch_update
 *
 *
 * Parameters:
 *    st->prev_gp       B: previous pitch gain
 *    st->past_gain_pit O: past gain
 *    st->pbuf          B: past gain buffer
 *    bfi               I: bad frame indicator
 *    prev_bf           I: previous frame was bad
 *    gain_pitch        B: pitch gain
 *
 * Function:
 *    Update the pitch gain concealment state
 *    Limit gain_pitch if the previous frame was bad
 *
 * Returns:
 *    gain
 */
static void ec_gain_pitch_update( ec_gain_pitchState *st, Word32 bfi,
      Word32 prev_bf, Word32 *gain_pitch )
{
   if ( bfi == 0 ) {
      if ( prev_bf != 0 ) {
         if ( *gain_pitch > st->prev_gp ) {
            *gain_pitch = st->prev_gp;
         }
      }
      st->prev_gp = *gain_pitch;
   }
   st->past_gain_pit = *gain_pitch;

   /* if (st->past_gain_pit > 1.0) */
   if ( st->past_gain_pit > 16384 ) {
      st->past_gain_pit = 16384;
   }
   st->pbuf[0] = st->pbuf[1];
   st->pbuf[1] = st->pbuf[2];
   st->pbuf[2] = st->pbuf[3];
   st->pbuf[3] = st->pbuf[4];
   st->pbuf[4] = st->past_gain_pit;
}


/*
 * gc_pred (366)
 *
 *
 * Parameters:
 *    st->past_qua_en         I: MA predictor
 *    st->past_qua_en_MR122   I: MA predictor MR122
 *    mode                    I: AMR mode
 *    code                    I: innovative codebook vector
 *    exp_gcode0              O: predicted gain factor (exponent)
 *    frac_gcode0             O: predicted gain factor (fraction)
 *    exp_en                  I: innovation energy (MR795) (exponent)
 *    frac_en                 I: innovation energy (MR795) (fraction)
 *
 * Function:
 *    MA prediction of the innovation energy
 *
 *    Mean removed innovation energy (dB) in subframe n
 *                          N-1
 *    E(n) = 10*log(gc*gc * SUM[(code(i) * code(i)]/N) - EMean
 *                          i=0
 *    N=40
 *
 *    Mean innovation energy (dB)
 *                   N-1
 *    Ei(n) = 10*log(SUM[(code(i) * code(i)]/N)
 *                   i=0
 *
 *    Predicted energy
 *              4
 *    Ep(n) = SUM[b(i) * R(n-i)]
 *            i=1
 *    b = [0.68 0.58 0.34 0.19]
 *    R(k) is quantified prediction error at subframe k
 *
 *    E_Mean = 36 dB (MR122)
 *
 *    Predicted gain gc is found by
 *
 *    gc = POW[10, 0.05 * (Ep(n) + EMean - Ei)]
 *
 * Returns:
 *    void
 */
static void gc_pred( gc_predState *st, enum Mode mode, Word32 *code, Word32 *
      exp_gcode0, Word32 *frac_gcode0, Word32 *exp_en, Word32 *frac_en )
{
   Word32 exp, frac, ener_code = 0, i = 0;


    /* energy of code:
     * ener_code = sum(code[i]^2)
     */
   while ( i < L_SUBFR ) {
      ener_code += code[i] * code[i];
      i++;
   }

   if ( ( 0x3fffffff <= ener_code ) | ( ener_code < 0 ) )
      ener_code = MAX_32;
   else
      ener_code <<= 1;

   if ( mode == MR122 ) {
      Word32 ener;


      /* ener_code = ener_code / lcode; lcode = 40; 1/40 = 26214 Q20       */
      ener_code = ( ( ener_code + 0x00008000L ) >> 16 ) * 52428;

      /* Q9  * Q20 -> Q30 */
        /* energy of code:
         * ener_code(Q17) = 10 * Log10(energy) / constant
         *                = 1/2 * Log2(energy)
         * constant = 20*Log10(2)
         */
      /* ener_code = 1/2 * Log2(ener_code); Note: Log2=log2+30 */
      Log2( ener_code, &exp, &frac );
      ener_code = ( ( exp - 30 ) << 16 ) + ( frac << 1 );

      /* Q16 for log(), ->Q17 for 1/2 log() */
         /*
          * predicted energy:
          * ener(Q24) = (Emean + sum{pred[i]*pastEn[i]})/constant
          *           = MEAN_ENER + sum(pred[i]*past_qua_en[i])
          * constant = 20*Log10(2)
          */
      ener = 0;
      i = 0;

      while ( i < 4 ) {
         ener += st->past_qua_en_MR122[i] * pred_MR122[i];
         i++;
      }
      ener <<= 1;
      ener += MEAN_ENER_MR122;

        /*
         * predicted codebook gain

         * gc0 = Pow10( (ener*constant - ener_code*constant) / 20 )
         *     = Pow2(ener-ener_code)
         *     = Pow2(int(d)+frac(d))
         */
      ener = ( ener - ener_code ) >> 1;   /* Q16 */
      *exp_gcode0 = ener >> 16;
      *frac_gcode0 = ( ener >> 1 ) - ( *exp_gcode0 << 15 );
   }

   /* all modes except 12.2 */
   else {
      Word32 tmp, gcode0;
      int exp_code;


        /*
         * Compute: meansEner - 10log10(ener_code/ LSufr)
         */
      exp_code=0;
      if (ener_code != 0){
         while (!(ener_code & 0x40000000))
         {
            exp_code++;
            ener_code = ener_code << 1;
         }
      }

      /* Log2 = log2 + 27 */
      Log2_norm( ener_code, exp_code, &exp, &frac );

      /* fact = 10/log2(10) = 3.01 = 24660 Q13 */
      /* Q0.Q15 * Q13 -> Q14 */
      tmp = ( exp * ( -49320 ) ) + ( ( ( frac * ( -24660 ) ) >> 15 ) << 1 );

      /*
       * tmp = meansEner - 10log10(ener_code/L_SUBFR)
       *       = meansEner - 10log10(ener_code) + 10log10(L_SUBFR)
       *       = K - fact * Log2(ener_code)
       *     = K - fact * log2(ener_code) - fact*27
       *
       *   ==> K = meansEner + fact*27 + 10log10(L_SUBFR)
       *
       *   meansEner =       33    =  540672    Q14  (MR475, MR515, MR59)
       *   meansEner =       28.75 =  471040    Q14  (MR67)
       *   meansEner =       30    =  491520    Q14  (MR74)
       *   meansEner =       36    =  589824    Q14  (MR795)
       *   meansEner =       33    =  540672    Q14  (MR102)
       *   10log10(L_SUBFR) = 16.02 =  262481.51 Q14
       *   fact * 27                = 1331640    Q14
       *   -----------------------------------------
       *   (MR475, MR515, MR59)   K = 2134793.51 Q14 ~= 16678 * 64 * 2
       *   (MR67)                 K = 2065161.51 Q14 ~= 32268 * 32 * 2
       *   (MR74)                 K = 2085641.51 Q14 ~= 32588 * 32 * 2
       *   (MR795)                K = 2183945.51 Q14 ~= 17062 * 64 * 2
       *   (MR102)                K = 2134793.51 Q14 ~= 16678 * 64 * 2
       */
      if ( mode == MR102 ) {
         /* mean = 33 dB */
         tmp += 2134784;   /* Q14 */
      }
      else if ( mode == MR795 ) {
         /* mean = 36 dB */
         tmp += 2183936;   /* Q14 */

         /*
          * ener_code  = <xn xn> * 2^27*2^exp_code
          * frac_en    = ener_code / 2^16
          *            = <xn xn> * 2^11*2^exp_code
          * <xn xn>    = <xn xn>*2^11*2^exp * 2^exp_en
          *           := frac_en            * 2^exp_en
          *
          * ==> exp_en = -11-exp_code;
          */
         *frac_en = ener_code >> 16;
         *exp_en = -11 - exp_code;
      }
      else if ( mode == MR74 ) {
         /* mean = 30 dB */
         tmp += 2085632;   /* Q14 */
      }
      else if ( mode == MR67 ) {
         /* mean = 28.75 dB */
         tmp += 2065152;   /* Q14 */
      }
      else /* MR59, MR515, MR475 */ {
         /* mean = 33 dB */
         tmp += 2134784;   /* Q14 */
      }

        /*
         * Compute gcode0
         * = Sum(i=0,3) pred[i]*past_qua_en[i] - ener_code + meanEner
         */
      tmp = tmp << 9;   /* Q23 */

      /* Q13 * Q10 -> Q23 */
      i = 0;

      while ( i < 4 ) {
         tmp += pred[i] * st->past_qua_en[i];
         i++;
      }
      gcode0 = tmp >> 15;   /* Q8  */

        /*
         * gcode0 = pow(10.0, gcode0/20)
         *        = pow(2, 3.3219*gcode0/20)
         *        = pow(2, 0.166*gcode0)
         */
      /* 5439 Q15 = 0.165985                                        */
      /* (correct: 1/(20*log10(2)) 0.166096 = 5443 Q15)             */
      /* For IS641 bitexactness */
      if ( mode == MR74 ) {
         /* Q8 * Q15 -> Q24 */
         tmp = gcode0 * 10878;
      }
      else {
         /* Q8 * Q15 -> Q24 */
         tmp = gcode0 * 10886;
      }
      tmp = tmp >> 9;   /* -> Q15 */

      /* -> Q0.Q15 */
      *exp_gcode0 = tmp >> 15;
      *frac_gcode0 = tmp - ( *exp_gcode0 * 32768 );
   }
}


/*
 * gc_pred_update
 *
 *
 * Parameters:
 *    st->past_qua_en         B: MA predictor
 *    st->past_qua_en_MR122   B: MA predictor MR122
 *    qua_ener_MR122          I: quantized energy for update (log2(quaErr))
 *    qua_ener                I: quantized energy for update (20*log10(quaErr))
 *
 * Function:
 *    Update MA predictor with last quantized energy
 *
 * Returns:
 *    void
 */
static void gc_pred_update( gc_predState *st, Word32 qua_ener_MR122,
      Word32 qua_ener )
{
   Word32 i;


   for ( i = 3; i > 0; i-- ) {
      st->past_qua_en[i] = st->past_qua_en[i - 1];
      st->past_qua_en_MR122[i] = st->past_qua_en_MR122[i - 1];
   }
   st->past_qua_en_MR122[0] = qua_ener_MR122;   /* log2 (quaErr), Q10 */
   st->past_qua_en[0] = qua_ener;   /* 20*log10(quaErr), Q10 */
}


/*
 * Dec_gain
 *
 *
 * Parameters:
 *    pred_state->past_qua_en       B: MA predictor
 *    pred_state->past_qua_en_MR122 B: MA predictor MR122
 *    mode                          I: AMR mode
 *    index                         I: index of quantization
 *    code                          I: Innovative vector
 *    evenSubfr                     I: Flag for even subframes
 *    gain_pit                      O: Pitch gain
 *    gain_cod                      O: Code gain
 *
 * Function:
 *    Decode the pitch and codebook gains
 *
 * Returns:
 *    void
 */
static void Dec_gain( gc_predState *pred_state, enum Mode mode, Word32 index,
      Word32 code[], Word32 evenSubfr, Word32 *gain_pit, Word32 *gain_cod )
{
   Word32 frac, gcode0, exp, qua_ener, qua_ener_MR122, g_code, tmp;
   const Word32 *p;


   /* Read the quantized gains (table depends on mode) */
   index = index << 2;

   if ( ( mode == MR102 ) || ( mode == MR74 ) || ( mode == MR67 ) ) {
      p = &table_gain_highrates[index];
      *gain_pit = *p++;
      g_code = *p++;
      qua_ener_MR122 = *p++;
      qua_ener = *p;
   }
   else {
      if ( mode == MR475 ) {
         index = index + ( ( 1 - evenSubfr ) << 1 );
         p = &table_gain_MR475[index];
         *gain_pit = *p++;
         g_code = *p++;

            /*
             * calculate predictor update values (not stored in 4.75
             * quantizer table to save space):
             *   qua_ener       = log2(g)
             *   qua_ener_MR122 = 20*log10(g)
             */
         /* Log2(x Q12) = log2(x) + 12 */
         Log2( g_code, &exp, &frac );
         exp = exp - 12;
         tmp = frac >> 5;

         if ( ( frac & ( ( Word16 )1 << 4 ) ) != 0 ) {
            tmp++;
         }
         qua_ener_MR122 = tmp + ( exp << 10 );

         /* 24660 Q12 ~= 6.0206 = 20*log10(2) */
         tmp = exp * 49320;
         tmp += ( ( ( frac * 24660 ) >> 15 ) << 1 );

         /* Q12 * Q0 = Q13 -> Q10 */
         qua_ener = ( ( tmp << 13 ) + 0x00008000L ) >> 16;
      }
      else {
         p = &table_gain_lowrates[index];
         *gain_pit = *p++;
         g_code = *p++;
         qua_ener_MR122 = *p++;
         qua_ener = *p;
      }
   }

    /*
     * predict codebook gain
     * gc0 = Pow2(int(d)+frac(d))
     *     = 2^exp + 2^frac
     * gcode0 (Q14) = 2^14*2^frac = gc0 * 2^(14-exp)
     */
   gc_pred( pred_state, mode, code, &exp, &frac, NULL, NULL );
   gcode0 = Pow2( 14, frac );

    /*
     * read quantized gains, update table of past quantized energies
     * st->past_qua_en(Q10) = 20 * Log10(gFac) / constant
     *                      = Log2(gFac)
     *                      = qua_ener
     * constant = 20*Log10(2)
     */
   if ( exp < 11 ) {
      *gain_cod = ( g_code * gcode0 ) >> ( 25 - exp );
   }
   else {
      tmp = ( ( g_code * gcode0 ) << ( exp - 9 ) );

      if ( ( tmp >> ( exp - 9 ) ) != ( g_code * gcode0 ) ) {
         *gain_cod = 0x7FFF;
      }
      else {
         *gain_cod = tmp >> 16;
      }
   }

   /* update table of past quantized energies */
   gc_pred_update( pred_state, qua_ener_MR122, qua_ener );
   return;
}


/*
 * gc_pred_average_limited
 *
 *
 * Parameters:
 *    st->past_qua_en         I: MA predictor
 *    st->past_qua_en_MR122   I: MA predictor MR122
 *    ener_avg_MR122          O: everaged quantized energy (log2(quaErr))
 *    ener_avg                O: averaged quantized energy (20*log10(quaErr))
 *
 * Function:
 *    Compute average limited quantized energy
 * Returns:
 *    void
 */
static void gc_pred_average_limited( gc_predState *st, Word32 *ener_avg_MR122,
      Word32 *ener_avg )
{
   Word32 av_pred_en, i;


   /* do average in MR122 mode (log2() domain) */
   av_pred_en = 0;

   for ( i = 0; i < NPRED; i++ ) {
      av_pred_en = ( av_pred_en + st->past_qua_en_MR122[i] );
   }

   /* av_pred_en = 0.25*av_pred_en */
   av_pred_en = ( av_pred_en * 8192 ) >> 15;

   /* if (av_pred_en < -14/(20Log10(2))) av_pred_en = .. */
   if ( av_pred_en < MIN_ENERGY_MR122 ) {
      av_pred_en = MIN_ENERGY_MR122;
   }
   *ener_avg_MR122 = ( Word16 )av_pred_en;

   /* do average for other modes (20*log10() domain) */
   av_pred_en = 0;

   for ( i = 0; i < NPRED; i++ ) {
      av_pred_en = ( av_pred_en + st->past_qua_en[i] );
      if (av_pred_en < -32768)
         av_pred_en = -32768;
      else if (av_pred_en > 32767)
         av_pred_en = 32767;
   }

   /* av_pred_en = 0.25*av_pred_en */
   av_pred_en = ( av_pred_en * 8192 ) >> 15;

   *ener_avg = av_pred_en;
}


/*
 * ec_gain_code
 *
 *
 * Parameters:
 *    st->gbuf             I: last five gains
 *    st->past_gain_code   I: past gain
 *    pred_state           B: MA predictor state
 *    state                I: state of the state machine
 *    gain_code            O: decoded innovation gain
 *
 * Function:
 *    Conceal the codebook gain
 *
 * Returns:
 *    void
 */
static void ec_gain_code( ec_gain_codeState *st, gc_predState *pred_state,
      Word16 state, Word32 *gain_code )
{
   Word32 tmp, qua_ener_MR122, qua_ener;


   /* calculate median of last five gain values */
   tmp = gmed_n( st->gbuf, 5 );

   /* new gain = minimum(median, past_gain) * cdown[state] */
   if ( tmp > st->past_gain_code ) {
      tmp = st->past_gain_code;
   }
   tmp = ( tmp * cdown[state] ) >> 15;
   *gain_code = tmp;

      /*
       * update table of past quantized energies with average of
       * current values
       */
   gc_pred_average_limited( pred_state, &qua_ener_MR122, &qua_ener );
   gc_pred_update( pred_state, qua_ener_MR122, qua_ener );
}


/*
 * ec_gain_code_update
 *
 *
 * Parameters:
 *    st->gbuf             B: last five gains
 *    st->past_gain_code   O: past gain
 *    st->prev_gc          B  previous gain
 *    bfi                  I: bad indicator
 *    prev_bf              I: previous frame bad indicator
 *    gain_code            O: decoded innovation gain
 *
 * Function:
 *    Update the codebook gain concealment state
 *
 * Returns:
 *    void
 */
static void ec_gain_code_update( ec_gain_codeState *st, Word16 bfi,
      Word16 prev_bf, Word32 *gain_code )
{
   /* limit gain_code by previous good gain if previous frame was bad */
   if ( bfi == 0 ) {
      if ( prev_bf != 0 ) {
         if ( *gain_code > st->prev_gc ) {
            *gain_code = st->prev_gc;
         }
      }
      st->prev_gc = *gain_code;
   }

   /* update EC states: previous gain, gain buffer */
   st->past_gain_code = *gain_code;
   st->gbuf[0] = st->gbuf[1];
   st->gbuf[1] = st->gbuf[2];
   st->gbuf[2] = st->gbuf[3];
   st->gbuf[3] = st->gbuf[4];
   st->gbuf[4] = *gain_code;
   return;
}


/*
 * d_gain_code
 *
 *
 * Parameters:
 *    pred_state        B: MA predictor state
 *    mode              I: AMR mode (MR795 or MR122)
 *    index             I: received quantization index
 *    code              I: innovation codevector
 *    gain_code         O: decoded innovation gain
 *
 * Function:
 *    Decode the fixed codebook gain using the received index
 *
 * Returns:
 *    void
 */
static void d_gain_code( gc_predState *pred_state, enum Mode mode, Word32 index,
                        Word32 code[], Word32 *gain_code )
{
   Word32 g_code0, exp, frac, qua_ener_MR122, qua_ener;
   Word32 exp_inn_en, frac_inn_en, tmp, tmp2, i;
   const Word32 *p;


   /*
    * Decode codebook gain
    */
   gc_pred( pred_state, mode, code, &exp, &frac, &exp_inn_en, &frac_inn_en );
   p = &qua_gain_code[( ( index + index )+ index )];

   /* Different scalings between MR122 and the other modes */
   if ( mode == MR122 ) {
      /* predicted gain */
      g_code0 = Pow2( exp, frac );

      if ( g_code0 <= 2047 )
         g_code0 = g_code0 << 4;
      else
         g_code0 = 32767;
      *gain_code = ( ( g_code0 * *p++ ) >> 15 ) << 1;
      if (*gain_code & 0xFFFF8000)
         *gain_code = 32767;

   }
   else {
      g_code0 = Pow2( 14, frac );
      tmp = ( *p++ * g_code0 ) << 1;
      exp = 9 - exp;

      if ( exp > 0 ) {
         tmp = tmp >> exp;
      }
      else {
         for (i = exp; i < 0; i++) {
            tmp2 = tmp << 1;
            if ((tmp ^ tmp2) & 0x80000000) {
               tmp = (tmp & 0x80000000) ? 0x80000000 : 0x7FFFFFFF;
               break;
            }
            else {
               tmp = tmp2;
            }
         }
      }
      *gain_code = tmp >> 16;
      if (*gain_code & 0xFFFF8000)
         *gain_code = 32767;
   }

   /*
    * update table of past quantized energies
    */
   qua_ener_MR122 = *p++;
   qua_ener = *p++;
   gc_pred_update( pred_state, qua_ener_MR122, qua_ener );
   return;
}


/*
 * Int_lsf
 *
 *
 * Parameters:
 *    lsf_old           I: LSF vector at the 4th subframe of past frame
 *    lsf_new           I: LSF vector at the 4th subframe of present frame
 *    i_subfr           I: current subframe
 *    lsf_out           O: interpolated LSF parameters for current subframe
 *
 * Function:
 *    Interpolates the LSFs for selected subframe
 *
 *    The LSFs are interpolated at the 1st, 2nd and 3rd
 *    ubframe and only forwarded at the 4th subframe.
 *
 *    sf1:  3/4 F0 + 1/4 F1
 *    sf2:  1/2 F0 + 1/2 F1
 *    sf3:  1/4 F0 + 3/4 F1
 *    sf4:  F1
 *
 * Returns:
 *    void
 */
static void Int_lsf( Word32 lsf_old[], Word32 lsf_new[], int i_subfr, Word32
      lsf_out[] )
{
   Word32 i;


   switch ( i_subfr ) {
      case 0:
         for ( i = 0; i < 10; i++ ) {
            lsf_out[i] = lsf_old[i] - ( lsf_old[i] >> 2 ) + ( lsf_new[i] >> 2 );
         }
         break;

      case 40:
         for ( i = 0; i < 10; i++ ) {
            lsf_out[i] = ( lsf_old[i] >> 1 ) + ( lsf_new[i] >> 1 );
         }
         break;

      case 80:
         for ( i = 0; i < 10; i++ ) {
            lsf_out[i] = ( lsf_old[i] >> 2 ) - ( lsf_new[i] >> 2 ) +
                  lsf_new[i];
         }
         break;

      case 120:
         memcpy( lsf_out, lsf_new, M <<2 );
         break;
   }
}


/*
 * Cb_gain_average
 *
 *
 * Parameters:
 *    st->cbGainHistory B: codebook gain history
 *    st->hangCount     B: hangover counter
 *    mode              I: AMR mode
 *    gain_code         I: codebook gain
 *    lsp               I: The LSP for the current frame
 *    lspAver           I: The average of LSP for 8 frames
 *    bfi               I: bad frame indication
 *    prev_bf           I: previous bad frame indication
 *    pdfi              I: potential degraded bad frame indication
 *    prev_pdf          I: previous potential degraded bad frame indication
 *    inBackgroundNoise I: background noise decision
 *    voicedHangover    I: number of frames after last voiced frame
 *
 * Function:
 *    The mixed codebook gain, used to make codebook gain more smooth in background
 *
 *
 * Returns:
 *    void
 */
static Word32 Cb_gain_average( Cb_gain_averageState *st, enum Mode mode, Word32
      gain_code, Word32 lsp[], Word32 lspAver[], Word16 bfi, Word16 prev_bf,
      Word16 pdfi, Word16 prev_pdf, Word32 inBackgroundNoise, Word32
      voicedHangover )
{
   Word32 tmp[M];
   Word32 i, cbGainMix, tmp_diff, bgMix, cbGainMean, sum, diff, tmp1, tmp2;
   int shift1, shift2, shift;


   /* set correct cbGainMix for MR74, MR795, MR122 */
   cbGainMix = gain_code;

   /*
    * Store list of CB gain needed in the CB gain averaging                                           *
    */
   st->cbGainHistory[0] = st->cbGainHistory[1];
   st->cbGainHistory[1] = st->cbGainHistory[2];
   st->cbGainHistory[2] = st->cbGainHistory[3];
   st->cbGainHistory[3] = st->cbGainHistory[4];
   st->cbGainHistory[4] = st->cbGainHistory[5];
   st->cbGainHistory[5] = st->cbGainHistory[6];
   st->cbGainHistory[6] = gain_code;

   /* compute lsp difference */
   for ( i = 0; i < M; i++ ) {
      tmp1 = labs( lspAver[i]- lsp[i] );
      shift1 = 0;
      if (tmp1 != 0){
         while (!(tmp1 & 0x2000))
         {
            shift1++;
            tmp1 = tmp1 << 1;
         }
      }
      tmp2 = lspAver[i];
      shift2 = 0;
      if (tmp2 != 0){
         while (!(tmp2 & 0x4000))
         {
            shift2++;
            tmp2 = tmp2 << 1;
         }
      }
      tmp[i] = ( tmp1 << 15 ) / tmp2;
      shift = 2 + shift1 - shift2;

      if ( shift >= 0 ) {
         tmp[i] = tmp[i] >> shift;
      }
      else {
         tmp[i] = tmp[i] << -( shift );
      }
   }
   diff = *tmp + tmp[1] + tmp[2] + tmp[3] + tmp[4] + tmp[5] + tmp[6] + tmp[7] +
         tmp[8] + tmp[9];

   /* saturate */
   if ( diff > 32767 ) {
      diff = 32767;
   }

   /* Compute hangover */
   st->hangVar += 1;

   if ( diff <= 5325 ) {
      st->hangVar = 0;
   }

   if ( st->hangVar > 10 ) {
      /* Speech period, reset hangover variable */
      st->hangCount = 0;
   }

   /* Compute mix constant (bgMix) */
   bgMix = 8192;

   /* MR475, MR515, MR59, MR67, MR102 */
   if ( ( mode <= MR67 ) | ( mode == MR102 ) ) {
      /* disable mix if too short time since */
      if ( ( st->hangCount >= 40 ) & ( diff <= 5325 ) ) /* 0.65 in Q13 */ {
         /* if errors and presumed noise make smoothing probability stronger */
         if ( ( ( ( ( pdfi != 0 ) & ( prev_pdf != 0 ) ) | ( bfi != 0 ) | (
               prev_bf != 0 ) ) & ( ( voicedHangover > 1 ) ) & (
               inBackgroundNoise != 0 ) & ( mode < MR67 ) ) ) {
            /* bgMix = min(0.25, max(0.0, diff-0.55)) / 0.25; */
            tmp_diff = diff - 4506;   /* 0.55 in Q13 */

            /* max(0.0, diff-0.55) */
            tmp1 = 0;

            if ( tmp_diff > 0 ) {
               tmp1 = tmp_diff;
            }

            /* min(0.25, tmp1) */
            if ( 2048 >= tmp1 ) {
               bgMix = tmp1 << 2;
            }
         }
         else {
            /* bgMix = min(0.25, max(0.0, diff-0.40)) / 0.25; */
            tmp_diff = diff - 3277;   /* 0.4 in Q13 */

            /* max(0.0, diff-0.40) */
            tmp1 = 0;

            if ( tmp_diff > 0 ) {
               tmp1 = tmp_diff;
            }

            /* min(0.25, tmp1) */
            if ( 2048 >= tmp1 ) {
               bgMix = tmp1 << 2;
            }
         }
      }

      /*
       * Smoothen the cb gain trajectory
       * smoothing depends on mix constant bgMix
       */
      sum = st->cbGainHistory[2] + st->cbGainHistory[3] + st->cbGainHistory[4] +
            st->cbGainHistory[5] + st->cbGainHistory[6];

      if ( sum > 163822 ) {
         cbGainMean = 32767;
      }
      else {
         cbGainMean = ( 3277 * sum + 0x00002000L ) >> 14;   /* Q1 */
      }

      /* more smoothing in error and bg noise (NB no DFI used  here) */
      if ( ( ( bfi != 0 ) | ( prev_bf != 0 ) ) & ( inBackgroundNoise != 0 ) & (
            mode < MR67 ) ) {
         sum = 9362 * ( st->cbGainHistory[0] + st->cbGainHistory[1] + st->
               cbGainHistory[2] + st->cbGainHistory[3] + st->cbGainHistory[4] +
               st->cbGainHistory[5] + st->cbGainHistory[6] );
         cbGainMean = ( sum + 0x00008000L ) >> 16;   /* Q1 */
      }

      /* cbGainMix = bgMix*cbGainMix + (1-bgMix)*cbGainMean; */
      sum = bgMix * cbGainMix;   /* sum in Q14 */
      sum += cbGainMean << 13;
      sum -= bgMix * cbGainMean;
      cbGainMix = ( sum + 0x00001000L ) >> 13;

      /* Q1 */
   }
   st->hangCount += 1;
   if (st->hangCount & 0x80000000)
      st->hangCount = 40;
   return cbGainMix;
}


/*
 * ph_disp
 *
 *
 * Parameters:
 *    state->gainMem    B: LTP gain memory
 *    state->prevCbGain B: Codebook gain memory
 *    mode              I: AMR mode
 *    x                 B: LTP excitation signal -> total excitation signal
 *    cbGain            I: Codebook gain
 *    ltpGain           I: LTP gain
 *    inno              B: Innovation vector
 *    pitch_fac         I: pitch factor used to scale the LTP excitation
 *    tmp_shift         I: shift factor applied to sum of scaled LTP ex & innov.
 *                         before rounding
 *
 * Function:
 *    Adaptive phase dispersion; forming of total excitation
 *
 *
 * Returns:
 *    void
 */
static void ph_disp( ph_dispState *state, enum Mode mode, Word32 x[],
                    Word32 cbGain, Word32 ltpGain, Word32 inno[],
                    Word32 pitch_fac, Word32 tmp_shift)
{
   Word32 inno_sav[L_SUBFR], ps_poss[L_SUBFR];
   Word32 i, i1, impNr, temp1, temp2, j, nze, nPulse, ppos;
   const Word32 *ph_imp;   /* Pointer to phase dispersion filter */


   /* Update LTP gain memory */
   state->gainMem[4] = state->gainMem[3];
   state->gainMem[3] = state->gainMem[2];
   state->gainMem[2] = state->gainMem[1];
   state->gainMem[1] = state->gainMem[0];
   state->gainMem[0] = ltpGain;

   /* basic adaption of phase dispersion */
   /* no dispersion */
   impNr = 2;

   /* if (ltpGain < 0.9) */
   if ( ltpGain < PHDTHR2LTP ) {
      /* maximum dispersion */
      impNr = 0;

      /* if (ltpGain > 0.6 */
      if ( ltpGain > PHDTHR1LTP ) {
         /* medium dispersion */
         impNr = 1;
      }
   }

   /* onset indicator */
   /* onset = (cbGain  > onFact * cbGainMem[0]) */
   temp1 = ( ( state->prevCbGain * ONFACTPLUS1 ) + 0x1000 ) >> 13;

   if ( cbGain > temp1 ) {
      state->onset = ONLENGTH;
   }
   else {
      if ( state->onset > 0 ) {
         state->onset--;
      }
   }

   /*
    * if not onset, check ltpGain buffer and use max phase dispersion if
    * half or more of the ltpGain-parameters say so
    */
   if ( state->onset == 0 ) {
      /* Check LTP gain memory and set filter accordingly */
      i1 = 0;

      for ( i = 0; i < PHDGAINMEMSIZE; i++ ) {
         if ( state->gainMem[i] < PHDTHR1LTP ) {
            i1++;
         }
      }

      if ( i1 > 2 ) {
         impNr = 0;
      }
   }

   /* Restrict decrease in phase dispersion to one step if not onset */
   if ( ( impNr > ( state->prevState + 1 ) ) & ( state->onset == 0 ) ) {
      impNr--;
   }

   /* if onset, use one step less phase dispersion */
   if ( ( impNr<2 )&( state->onset>0 ) ) {
      impNr++;
   }

   /* disable for very low levels */
   if ( cbGain < 10 ) {
      impNr = 2;
   }

   if ( state->lockFull == 1 ) {
      impNr = 0;
   }

   /* update static memory */
   state->prevState = impNr;
   state->prevCbGain = cbGain;

   /*
    * do phase dispersion for all modes but 12.2 and 7.4;
    * don't modify the innovation if impNr >=2 (= no phase disp)
    */
   if ( ( mode != MR122 ) & ( mode != MR102 ) & ( mode != MR74 ) & ( impNr < 2 )
      ) {
      /*
       * track pulse positions, save innovation,
       * and initialize new innovation
       */
      nze = 0;

      for ( i = 0; i < L_SUBFR; i++ ) {
         if ( inno[i] != 0 ) {
            ps_poss[nze] = i;
            nze++;
         }
      }
      memcpy( inno_sav, inno, L_SUBFR <<2 );
      memset( inno, 0, L_SUBFR <<2 );

      /* Choose filter corresponding to codec mode and dispersion criterium */
      ph_imp = ph_imp_mid;

      if ( impNr == 0 ) {
         ph_imp = ph_imp_low;
      }

      if ( mode == MR795 ) {
         ph_imp = ph_imp_mid_MR795;

         if ( impNr == 0 ) {
            ph_imp = ph_imp_low_MR795;
         }
      }

      /* Do phase dispersion of innovation */
      for ( nPulse = 0; nPulse < nze; nPulse++ ) {
         ppos = ps_poss[nPulse];

         /* circular convolution with impulse response */
         j = 0;

         for ( i = ppos; i < L_SUBFR; i++ ) {
            /* inno[i1] += inno_sav[ppos] * ph_imp[i1-ppos] */
            temp1 = ( inno_sav[ppos] * ph_imp[j++] ) >> 15;
            inno[i] = inno[i] + temp1;
         }

         for ( i = 0; i < ppos; i++ ) {
            /* inno[i] += inno_sav[ppos] * ph_imp[L_SUBFR-ppos+i] */
            temp1 = ( inno_sav[ppos] * ph_imp[j++] ) >> 15;
            inno[i] = inno[i] + temp1;
         }
      }
   }

   /*
    * compute total excitation for synthesis part of decoder
    * (using modified innovation if phase dispersion is active)
    */
   for ( i = 0; i < L_SUBFR; i++ ) {
      /* x[i] = gain_pit*x[i] + cbGain*code[i]; */
      temp1 = x[i] * pitch_fac + inno[i] * cbGain;
      temp2 = temp1 << tmp_shift;
      x[i] = ( temp2 + 0x4000 ) >> 15;
      if (labs(x[i]) > 32767)
      {
         if ((temp1 ^ temp2) & 0x80000000) {
            x[i] = (temp1 & 0x80000000) ? -32768: 32767;
         }
         else {
            x[i] = (temp2 & 0x80000000) ? -32768: 32767;
         }
      }
   }
   return;
}


/*
 * sqrt_l_exp
 *
 *
 * Parameters:
 *    x                 I: input value
 *    exp               O: right shift to be applied to result
 *
 * Function:
 *    Sqrt with exponent value.
 *
 *    y = sqrt(x)
 *    x = f * 2^-e,   0.5 <= f < 1   (normalization)
 *    y = sqrt(f) * 2^(-e/2)
 *
 *    a) e = 2k   --> y = sqrt(f)   * 2^-k
 *       (k = e div 2, 0.707 <= sqrt(f) < 1)
 *    b) e = 2k+1 --> y = sqrt(f/2) * 2^-k
 *       (k = e div 2, 0.5 <= sqrt(f/2) < 0.707)
 *
 *
 * Returns:
 *    y                 output value
 */
static Word32 sqrt_l_exp( Word32 x, Word32 *exp )
{
   Word32 y, a, i, tmp;
   int e;


   if ( x <= ( Word32 )0 ) {
      *exp = 0;
      return( Word32 )0;
   }
   e=0;
   if (x != 0){
      tmp = x;
      while (!(tmp & 0x40000000))
      {
         e++;
         tmp = tmp << 1;
      }
   }
   e = e & 0xFFFE;
   x = ( x << e );
   *exp = ( Word16 )e;
   x = ( x >> 9 );
   i = ( Word16 )( x >> 16 );
   x = ( x >> 1 );
   a = x & ( Word16 )0x7fff;
   i = ( i - 16 );
   y = ( sqrt_table[i] << 16 );
   tmp = ( sqrt_table[i] - sqrt_table[i + 1] );
   y -= ( tmp * a ) << 1;
   return( y );
}


/*
 * Ex_ctrl
 *
 *
 * Parameters:
 *    excitation        B: Current subframe excitation
 *    excEnergy         I: Exc. Energy, sqrt(totEx*totEx)
 *    exEnergyHist      I: History of subframe energies
 *    voicedHangover    I: number of frames after last voiced frame
 *    prevBFI           I: Set i previous bad frame indicators
 *    carefulFlag       I: Restrict dymamic in scaling
 *
 * Function:
 *    Charaterice synthesis speech and detect background noise
 *
 * Returns:
 *    background noise decision; 0 = no bgn, 1 = bgn
 */
static Word16 Ex_ctrl( Word32 excitation[], Word32 excEnergy, Word32
      exEnergyHist[], Word32 voicedHangover, Word16 prevBFI, Word16 carefulFlag
      )
{
   Word32 i, testEnergy, scaleFactor, avgEnergy, prevEnergy, T0;
   int exp;


   /* get target level */
   avgEnergy = gmed_n( exEnergyHist, 9 );
   prevEnergy = ( exEnergyHist[7] + exEnergyHist[8] ) >> 1;

   if ( exEnergyHist[8] < prevEnergy ) {
      prevEnergy = exEnergyHist[8];
   }

   /* upscaling to avoid too rapid energy rises  for some cases */
   if ( ( excEnergy<avgEnergy )&( excEnergy>5 ) ) {
      /* testEnergy = 4*prevEnergy; */
      testEnergy = prevEnergy << 2;

      if ( ( voicedHangover < 7 ) || prevBFI != 0 ) {
         /* testEnergy = 3*prevEnergy */
         testEnergy = testEnergy - prevEnergy;
      }

      if ( avgEnergy > testEnergy ) {
         avgEnergy = testEnergy;
      }

      /* scaleFactor=avgEnergy/excEnergy in Q0 */
      exp=0;
      if (excEnergy != 0){
         while (!(excEnergy & 0x4000))
         {
            exp++;
            excEnergy = excEnergy << 1;
         }
      }
      excEnergy = 536838144 / excEnergy;
      T0 = ( avgEnergy * excEnergy ) << 1;
      T0 = ( T0 >> ( 20 - exp ) );

      if ( T0 > 32767 ) {
         /* saturate  */
         T0 = 32767;
      }
      scaleFactor = T0;

      /* test if scaleFactor > 3.0 */
      if ( ( carefulFlag != 0 ) & ( scaleFactor > 3072 ) ) {
         scaleFactor = 3072;
      }

      /* scale the excitation by scaleFactor */
      for ( i = 0; i < L_SUBFR; i++ ) {
         T0 = ( scaleFactor * excitation[i] ) << 1;
         T0 = ( T0 >> 11 );
         excitation[i] = T0;
      }
   }
   return 0;
}


/*
 * Inv_sqrt
 *
 *
 * Parameters:
 *    x                 I: input value
 *
 * Function:
 *    1/sqrt(x)
 *
 * Returns:
 *    y                 1/sqrt(x)
 */
static Word32 Inv_sqrt( Word32 x )
{
   int i, a, tmp, exp;
   Word32 y;


   if ( x <= ( Word32 )0 )
      return( ( Word32 )0x3fffffffL );
   exp=0;
   while (!(x & 0x40000000))
   {
      exp++;
      x = x << 1;
   }

   /* x is normalized */
   exp = ( 30 - exp );

   /* If exponent even -> shift right */
   if ( ( exp & 1 ) == 0 ) {
      x = ( x >> 1 );
   }
   exp = ( exp >> 1 );
   exp = ( exp + 1 );
   x = ( x >> 9 );

   /* Extract b25-b31 */
   i = ( Word16 )( x >> 16 );

   /* Extract b10-b24 */
   x = ( x >> 1 );
   a = x & ( Word16 )0x7fff;
   i = ( i - 16 );

   /* table[i] << 16 */
   y = inv_sqrt_table[i] << 16;

   /* table[i] - table[i+1]) */
   tmp = ( inv_sqrt_table[i] - inv_sqrt_table[i + 1] );

   /* y -= tmp*a*2 */
   y -= ( tmp * a ) << 1;

   /* denormalization */
   y = ( y >> exp );
   return( y );
}


/*
 * energy_old
 *
 *
 * Parameters:
 *    in                I: input value
 *
 * Function:
 *    Energy of signal
 *
 * Returns:
 *    Energy
 */
static Word32 energy_old( Word32 in[] )
{
   Word32 temp, i, sum = 0;


   for ( i = 0; i < L_SUBFR; i += 8 ) {
      temp = in[i] >> 2;
      sum += temp * temp;
      temp = in[i + 1] >> 2;
      sum += temp * temp;
      temp = in[i + 2] >> 2;
      sum += temp * temp;
      temp = in[i + 3] >> 2;
      sum += temp * temp;
      temp = in[i + 4] >> 2;
      sum += temp * temp;
      temp = in[i + 5] >> 2;
      sum += temp * temp;
      temp = in[i + 6] >> 2;
      sum += temp * temp;
      temp = in[i + 7] >> 2;
      sum += temp * temp;
   }

   if ( sum & 0xC0000000 ) {
      return 0x7FFFFFFF;
   }
   return( sum << 1 );
}


/*
 * energy_new
 *
 *
 * Parameters:
 *    in                I: input value
 *
 * Function:
 *    Energy of signal
 *
 * Returns:
 *    Energy
 */
static Word32 energy_new( Word32 in[] )
{
   Word32 i, s = 0, overflow = 0;

   s += in[0] * in[0];
   for ( i = 1; i < L_SUBFR; i += 3 ) {
      s += in[i] * in[i];
      s += in[i + 1] *in[i + 1];
      s += in[i + 2] * in[i + 2];


      if ( s & 0xC0000000 ) {
         overflow = 1;
         break;
      }
   }

   /* check for overflow */
   if ( overflow ) {
      s = energy_old( in );
   }
   else {
      s = ( s >> 3 );
   }
   return s;
}


/*
 * agc2
 *
 *
 * Parameters:
 *    sig_in            I: Post_Filter input signal
 *    sig_out           B: Post_Filter output signal
 *
 * Function:
 *    Scales the excitation on a subframe basis
 *
 * Returns:
 *    Energy
 */
static void agc2( Word32 *sig_in, Word32 *sig_out )
{
   Word32 s;
   int i, exp;
   Word16 gain_in, gain_out, g0;


   /* calculate gain_out with exponent */
   s = energy_new( sig_out );

   if ( s == 0 ) {
      return;
   }
   exp=0;
   while (!(s & 0x20000000))
   {
      exp++;
      s = s << 1;
   }

   gain_out = ( Word16 )( ( s + 0x00008000L ) >> 16 );

   /* calculate gain_in with exponent */
   s = energy_new( sig_in );

   if ( s == 0 ) {
      g0 = 0;
   }
   else {
      i = 0;
      while (!(s & 0x40000000))
      {
         i++;
         s = s << 1;
      }

      if ( s < 0x7fff7fff )
         gain_in = ( Word16 )( ( s + 0x00008000L ) >> 16 );
      else
         gain_in = 32767;
      exp = ( exp - i );

        /*
         * g0 = sqrt(gain_in/gain_out);
         */
      /* s = gain_out / gain_in */
      s = ( gain_out << 15 ) / gain_in;
      s = ( s << 7 );

      if ( exp > 0 )
         s = ( s >> exp );
      else
         s = ( s << ( -exp ) );
      s = Inv_sqrt( s );
      g0 = ( Word16 )( ( ( s << 9 ) + 0x00008000L ) >> 16 );
   }

   /* sig_out(n) = gain(n) * sig_out(n) */
   for ( i = 0; i < L_SUBFR; i++ ) {
      sig_out[i] = ( sig_out[i] * g0 ) >> 12;
   }
   return;
}


/*
 * Bgn_scd
 *
 *
 * Parameters:
 *    st->frameEnergyHist  B: Frame Energy memory
 *    st->bgHangover       B: Background hangover counter
 *    ltpGainHist          I: LTP gain history
 *    speech               I: synthesis speech frame
 *    voicedHangover       O: number of frames after last voiced frame
 *
 * Function:
 *    Charaterice synthesis speech and detect background noise
 *
 * Returns:
 *    inbgNoise         background noise decision; 0 = no bgn, 1 = bgn
 */
static Word16 Bgn_scd( Bgn_scdState *st, Word32 ltpGainHist[], Word32 speech[],
      Word32 *voicedHangover )
{
   Word32 temp, ltpLimit, frame_energyMin, currEnergy, noiseFloor, maxEnergy,
         maxEnergyLastPart, s, i;
   Word16 prevVoiced, inbgNoise;


   /*
    * Update the inBackgroundNoise flag (valid for use in next frame if BFI)
    * it now works as a energy detector floating on top
    * not as good as a VAD.
    */
   s = 0;

   for ( i = 0; i < L_FRAME; i++ ) {
      s += speech[i] * speech[i];
   }

   if ( (s < 0xFFFFFFF) & (s >= 0) )
      currEnergy = s >> 13;
   else
      currEnergy = 32767;
   frame_energyMin = 32767;

   for ( i = 0; i < L_ENERGYHIST; i++ ) {
      if ( st->frameEnergyHist[i] < frame_energyMin )
         frame_energyMin = st->frameEnergyHist[i];
   }

   /* Frame Energy Margin of 16 */
   noiseFloor = frame_energyMin << 4;
   maxEnergy = st->frameEnergyHist[0];

   for ( i = 1; i < L_ENERGYHIST - 4; i++ ) {
      if ( maxEnergy < st->frameEnergyHist[i] ) {
         maxEnergy = st->frameEnergyHist[i];
      }
   }
   maxEnergyLastPart = st->frameEnergyHist[2 * L_ENERGYHIST / 3];

   for ( i = 2 * L_ENERGYHIST / 3 + 1; i < L_ENERGYHIST; i++ ) {
      if ( maxEnergyLastPart < st->frameEnergyHist[i] ) {
         maxEnergyLastPart = st->frameEnergyHist[i];
      }
   }

   /* false */
   inbgNoise = 0;

   /*
    * Do not consider silence as noise
    * Do not consider continuous high volume as noise
    * Or if the current noise level is very low
    * Mark as noise if under current noise limit
    * OR if the maximum energy is below the upper limit
    */
   if ( ( maxEnergy> LOWERNOISELIMIT )&( currEnergy<FRAMEENERGYLIMIT )&(
         currEnergy>LOWERNOISELIMIT ) & ( ( currEnergy < noiseFloor ) || (
         maxEnergyLastPart < UPPERNOISELIMIT ) ) ) {
      if ( ( st->bgHangover + 1 ) > 30 ) {
         st->bgHangover = 30;
      }
      else {
         st->bgHangover += 1;
      }
   }
   else {
      st->bgHangover = 0;
   }

   /* make final decision about frame state, act somewhat cautiosly */
   if ( st->bgHangover > 1 )
      inbgNoise = 1;   /* true  */

   for ( i = 0; i < L_ENERGYHIST - 1; i++ ) {
      st->frameEnergyHist[i] = st->frameEnergyHist[i + 1];
   }
   st->frameEnergyHist[L_ENERGYHIST - 1] = currEnergy;

   /*
    * prepare for voicing decision;
    * tighten the threshold after some time in noise
    */
   ltpLimit = 13926;   /* 0.85  Q14 */

   if ( st->bgHangover > 8 ) {
      ltpLimit = 15565;   /* 0.95  Q14 */
   }

   if ( st->bgHangover > 15 ) {
      ltpLimit = 16383;   /* 1.00  Q14 */
   }

   /* weak sort of voicing indication. */
   prevVoiced = 0;   /* false */

   if ( gmed_n( &ltpGainHist[4], 5 ) > ltpLimit ) {
      prevVoiced = 1;   /* true  */
   }

   if ( st->bgHangover > 20 ) {
      if ( gmed_n( ltpGainHist, 9 ) > ltpLimit ) {
         prevVoiced = 1;   /* true  */
      }
      else {
         prevVoiced = 0;   /* false  */
      }
   }

   if ( prevVoiced ) {
      *voicedHangover = 0;
   }
   else {
      temp = *voicedHangover + 1;

      if ( temp > 10 ) {
         *voicedHangover = 10;
      }
      else {
         *voicedHangover = temp;
      }
   }
   return inbgNoise;
}


/*
 * dtx_dec_activity_update
 *
 *
 * Parameters:
 *    st->lsf_hist_ptr  B: LSF history pointer
 *    st->lsf_hist      B: LSF history
 *    lsf               I: lsf
 *    frame             I: noise frame
 *
 * Function:
 *    Update lsp history and compute log energy.
 *
 * Returns:
 *    void
 */
static void dtx_dec_activity_update( dtx_decState *st, Word32 lsf[], Word32
      frame[] )
{
   Word32 frame_en;
   Word32 log_en_e, log_en_m, log_en, i;


   /* update lsp history */
   st->lsf_hist_ptr += M;

   if ( st->lsf_hist_ptr == 80 ) {
      st->lsf_hist_ptr = 0;
   }
   memcpy( &st->lsf_hist[st->lsf_hist_ptr], lsf, M <<2 );

   /* compute log energy based on frame energy */
   frame_en = 0;   /* Q0 */

   for ( i = 0; (i < L_FRAME); i ++ ) {
      frame_en += frame[i] * frame[i];
      if (frame_en & 0x80000000)
         break;
   }

   log_en = (frame_en & 0xC0000000) ? 0x7FFFFFFE: (Word32)frame_en << 1;

   Log2( log_en , &log_en_e, &log_en_m );

   /* convert exponent and mantissa to Word16 Q10 */
   log_en = log_en_e << 10;   /* Q10 */
   log_en = log_en + ( log_en_m >> 5 );

   /* divide with L_FRAME i.e subtract with log2(L_FRAME) = 7.32193 */
   log_en = log_en - 8521;

   /*
    * insert into log energy buffer, no division by two as
    * log_en in decoder is Q11
    */
   st->log_en_hist_ptr += 1;

   if ( st->log_en_hist_ptr == DTX_HIST_SIZE ) {
      st->log_en_hist_ptr = 0;
   }
   st->log_en_hist[st->log_en_hist_ptr] = log_en;   /* Q11 */
}


/*
 * Decoder_amr
 *
 *
 * Parameters:
 *    st                B: State variables
 *    mode              I: AMR mode
 *    parm              I: vector of synthesis parameters
 *    frame_type        I: received frame type
 *    synth             O: synthesis speech
 *    A_t               O: decoded LP filter in 4 subframes
 *
 * Function:
 *    Speech decoder routine
 *
 * Returns:
 *    void
 */
static void Decoder_amr( Decoder_amrState *st, enum Mode mode, Word16 parm[],
      enum RXFrameType frame_type, Word32 synth[], Word32 A_t[] )
{
   /* LSPs */
   Word32 lsp_new[M];
   Word32 lsp_mid[M];


   /* LSFs */
   Word32 prev_lsf[M];
   Word32 lsf_i[M];


   /* Algebraic codevector */
   Word32 code[L_SUBFR];


   /* excitation */
   Word32 excp[L_SUBFR];
   Word32 exc_enhanced[L_SUBFR];


   /* Scalars */
   Word32 i, i_subfr, overflow, T0_frac, index, temp, temp2, subfrNr, excEnergy;
   Word32 gain_code, gain_code_mix, pit_sharp, pit_flag, pitch_fac, t0_min, t0_max;
   Word32 gain_pit = 0, evenSubfr = 0, T0 = 0, index_mr475 = 0;
   Word32 *Az;   /* Pointer on A_t */
   Word16 flag4, carefulFlag;
   Word16 delta_frc_low, delta_frc_range, tmp_shift;
   Word16 bfi = 0, pdfi = 0;
   /* bad frame indication flag, potential degraded bad frame flag */


   enum DTXStateType newDTXState;   /* SPEECH , DTX, DTX_MUTE */

   /* find the new  DTX state  SPEECH OR DTX */
   newDTXState = rx_dtx_handler( st->dtxDecoderState, frame_type );

   /* DTX actions */
   if ( newDTXState != SPEECH ) {
      Decoder_amr_reset( st, MRDTX );
      dtx_dec( st->dtxDecoderState, st->mem_syn, st->lsfState, st->pred_state,
            st->Cb_gain_averState, newDTXState, mode, parm, synth, A_t );

      /* update average lsp */
      Lsf_lsp( st->lsfState->past_lsf_q, st->lsp_old );
      lsp_avg( st->lsp_avg_st, st->lsfState->past_lsf_q );
      goto theEnd;
   }

   /* SPEECH action state machine  */
   if ( table_speech_bad[frame_type] ) {
      bfi = 1;

      if ( frame_type != RX_SPEECH_BAD ) {
         Build_CN_param( &st->nodataSeed, mode, parm );
      }
   }
   else if ( frame_type == RX_SPEECH_DEGRADED ) {
      pdfi = 1;
   }

   if ( bfi != 0 ) {
      st->state += 1;
   }
   else if ( st->state == 6 ) {
      st->state = 5;
   }
   else {
      st->state = 0;
   }

   if ( st->state > 6 ) {
      st->state = 6;
   }

    /*
     * If this frame is the first speech frame after CNI period,
     * set the BFH state machine to an appropriate state depending
     * on whether there was DTX muting before start of speech or not
     * If there was DTX muting, the first speech frame is muted.
     * If there was no DTX muting, the first speech frame is not
     * muted. The BFH state machine starts from state 5, however, to
     * keep the audible noise resulting from a SID frame which is
     * erroneously interpreted as a good speech frame as small as
     * possible (the decoder output in this case is quickly muted)
     */
   if ( st->dtxDecoderState->dtxGlobalState == DTX ) {
      st->state = 5;
      st->prev_bf = 0;
   }
   else if ( st->dtxDecoderState->dtxGlobalState == DTX_MUTE ) {
      st->state = 5;
      st->prev_bf = 1;
   }

   /* save old LSFs for CB gain smoothing */
   memcpy( prev_lsf, st->lsfState->past_lsf_q, M <<2 );

    /*
     * decode LSF parameters and generate interpolated lpc coefficients
     * for the 4 subframes
     */
   if ( mode != MR122 ) {
      D_plsf_3( st->lsfState, mode, bfi, parm, lsp_new );

      /* Advance synthesis parameters pointer */
      parm += 3;
      Int_lpc_1to3( st->lsp_old, lsp_new, A_t );
   }
   else {
      D_plsf_5( st->lsfState, bfi, parm, lsp_mid, lsp_new );

      /* Advance synthesis parameters pointer */
      parm += 5;
      Int_lpc_1and3( st->lsp_old, lsp_mid, lsp_new, A_t );
   }

   /* update the LSPs for the next frame */
   memcpy( st->lsp_old, lsp_new, M <<2 );

   /*
    * Loop for every subframe in the analysis frame
    *
    * The subframe size is L_SUBFR and the loop is repeated
    * L_FRAME/L_SUBFR times                                                                 *
    *  - decode the pitch delay
    *  - decode algebraic code
    *  - decode pitch and codebook gains
    *  - find the excitation and compute synthesis speech
    */
   /* pointer to interpolated LPC parameters */
   Az = A_t;
   evenSubfr = 0;
   subfrNr = -1;

   for ( i_subfr = 0; i_subfr < L_FRAME; i_subfr += L_SUBFR ) {
      subfrNr += 1;
      evenSubfr = 1 - evenSubfr;

      /* flag for first and 3th subframe */
      pit_flag = i_subfr;

      if ( i_subfr == L_FRAME_BY2 ) {
         if ( ( mode != MR475 ) & ( mode != MR515 ) ) {
            pit_flag = 0;
         }
      }

      /* pitch index */
      index = *parm++;

       /*
        * decode pitch lag and find adaptive codebook vector.
        */
      if ( mode != MR122 ) {
          /*
           * flag4 indicates encoding with 4 bit resolution;
           * this is needed for mode MR475, MR515, MR59 and MR67
           */
         flag4 = 0;

         if ( ( mode == MR475 ) || ( mode == MR515 ) || ( mode == MR59 ) || (
               mode == MR67 ) ) {
            flag4 = 1;
         }

          /*
           * get ranges for the t0_min and t0_max
           * only needed in delta decoding
           */
         delta_frc_low = 5;
         delta_frc_range = 9;

         if ( mode == MR795 ) {
            delta_frc_low = 10;
            delta_frc_range = 19;
         }
         t0_min = st->old_T0 - delta_frc_low;

         if ( t0_min < PIT_MIN ) {
            t0_min = PIT_MIN;
         }
         t0_max = t0_min + delta_frc_range;

         if ( t0_max > PIT_MAX ) {
            t0_max = PIT_MAX;
            t0_min = t0_max - delta_frc_range;
         }
         Dec_lag3( index, t0_min, t0_max, pit_flag, st->old_T0, &T0, &T0_frac,
               flag4 );
         st->T0_lagBuff = T0;

         if ( bfi != 0 ) {
            if ( st->old_T0 < PIT_MAX ) {
               /* Graceful pitch degradation */
               st->old_T0 += 1;
            }
            T0 = st->old_T0;
            T0_frac = 0;

            if ( ( st->inBackgroundNoise != 0 ) & ( st->voicedHangover > 4 ) & (
                  ( mode == MR475 ) || ( mode == MR515 ) || ( mode == MR59 ) ) )
            {
               T0 = st->T0_lagBuff;
            }
         }
         Pred_lt_3or6_40( st->exc, T0, T0_frac, 1 );
      }
      else {
         Dec_lag6( index, PIT_MIN_MR122, PIT_MAX, pit_flag, &T0, &T0_frac );

         if ( ( bfi != 0 ) || ( ( pit_flag != 0 ) & ( index > 60 ) ) ) {
            st->T0_lagBuff = T0;
            T0 = st->old_T0;
            T0_frac = 0;
         }
         Pred_lt_3or6_40( st->exc, T0, T0_frac, 0 );
      }

       /*
        * (MR122 only: Decode pitch gain.)
        * Decode innovative codebook.
        * set pitch sharpening factor
        */
      /* MR475, MR515 */
      if ( ( mode == MR475 ) || ( mode == MR515 ) ) {
         /* index of position */
         index = *parm++;

         /* signs */
         i = *parm++;
         decode_2i40_9bits( subfrNr, i, index, code );
         pit_sharp = st->sharp << 1;
      }

      /* MR59 */
      else if ( mode == MR59 ) {
         /* index of position */
         index = *parm++;

         /* signs */
         i = *parm++;
         decode_2i40_11bits( i, index, code );
         pit_sharp = st->sharp << 1;
      }

      /* MR67 */
      else if ( mode == MR67 ) {
         /* index of position */
         index = *parm++;

         /* signs */
         i = *parm++;
         decode_3i40_14bits( i, index, code );
         pit_sharp = st->sharp << 1;
      }

      /* MR74, MR795 */
      else if ( mode <= MR795 ) {
         /* index of position */
         index = *parm++;

         /* signs */
         i = *parm++;
         decode_4i40_17bits( i, index, code );
         pit_sharp = st->sharp << 1;
      }

      /* MR102 */
      else if ( mode == MR102 ) {
         decode_8i40_31bits( parm, code );
         parm += 7;
         pit_sharp = st->sharp << 1;
      }

      /* MR122 */
      else {
         index = *parm++;

         if ( bfi != 0 ) {
            ec_gain_pitch( st->ec_gain_p_st, st->state, &gain_pit );
         }
         else {
            gain_pit = d_gain_pitch( mode, index );
         }
         ec_gain_pitch_update( st->ec_gain_p_st, bfi, st->prev_bf, &gain_pit );
         decode_10i40_35bits( parm, code );
         parm += 10;

           /*
            * pit_sharp = gain_pit;
            * if (pit_sharp > 1.0) pit_sharp = 1.0;
            */
         pit_sharp = gain_pit;

         if ( pit_sharp > 16383 )
            pit_sharp = 32767;
         else
            pit_sharp *= 2;
      }

        /*
         * Add the pitch contribution to code[].
         */
      for ( i = T0; i < L_SUBFR; i++ ) {
         temp = ( code[i - T0] * pit_sharp ) >> 15;
         code[i] = code[i] + temp;
      }

        /*
         * Decode codebook gain (MR122) or both pitch
         * gain and codebook gain (all others)
         * Update pitch sharpening "sharp" with quantized gain_pit
         */
      if ( mode == MR475 ) {
         /* read and decode pitch and code gain */
         if ( evenSubfr != 0 ) {
            /* index of gain(s) */
            index_mr475 = *parm++;
         }

         if ( bfi == 0 ) {
            Dec_gain( st->pred_state, mode, index_mr475, code, evenSubfr, &
                  gain_pit, &gain_code );
         }
         else {
            ec_gain_pitch( st->ec_gain_p_st, st->state, &gain_pit );
            ec_gain_code( st->ec_gain_c_st, st->pred_state, st->state, &
                  gain_code );
         }
         ec_gain_pitch_update( st->ec_gain_p_st, bfi, st->prev_bf, &gain_pit );
         ec_gain_code_update( st->ec_gain_c_st, bfi, st->prev_bf, &gain_code );
         pit_sharp = gain_pit;

         if ( pit_sharp > SHARPMAX ) {
            pit_sharp = SHARPMAX;
         }
      }
      else if ( ( mode <= MR74 ) || ( mode == MR102 ) ) {
         /* read and decode pitch and code gain */
         /* index of gain(s) */
         index = *parm++;

         if ( bfi == 0 ) {
            Dec_gain( st->pred_state, mode, index, code, evenSubfr, &gain_pit, &
                  gain_code );
         }
         else {
            ec_gain_pitch( st->ec_gain_p_st, st->state, &gain_pit );
            ec_gain_code( st->ec_gain_c_st, st->pred_state, st->state, &
                  gain_code );
         }
         ec_gain_pitch_update( st->ec_gain_p_st, bfi, st->prev_bf, &gain_pit );
         ec_gain_code_update( st->ec_gain_c_st, bfi, st->prev_bf, &gain_code );
         pit_sharp = gain_pit;

         if ( pit_sharp > SHARPMAX ) {
            pit_sharp = SHARPMAX;
         }

         if ( mode == MR102 ) {
            if ( st->old_T0 > ( L_SUBFR + 5 ) ) {
               pit_sharp = pit_sharp >> 2;
            }
         }
      }
      else {
         /* read and decode pitch gain */
         /* index of gain(s) */
         index = *parm++;

         if ( mode == MR795 ) {
            /* decode pitch gain */
            if ( bfi != 0 ) {
               ec_gain_pitch( st->ec_gain_p_st, st->state, &gain_pit );
            }
            else {
               gain_pit = d_gain_pitch( mode, index );
            }
            ec_gain_pitch_update( st->ec_gain_p_st, bfi, st->prev_bf, &gain_pit
                  );

            /* read and decode code gain */
            index = *parm++;

            if ( bfi == 0 ) {
               d_gain_code( st->pred_state, mode, index, code, &gain_code );
            }
            else {
               ec_gain_code( st->ec_gain_c_st, st->pred_state, st->state, &
                     gain_code );
            }
            ec_gain_code_update( st->ec_gain_c_st, bfi, st->prev_bf, &gain_code
                  );
            pit_sharp = gain_pit;

            if ( pit_sharp > SHARPMAX ) {
               pit_sharp = SHARPMAX;
            }
         }
         else {   /* MR122 */

            if ( bfi == 0 ) {
               d_gain_code( st->pred_state, mode, index, code, &gain_code );
            }
            else {
               ec_gain_code( st->ec_gain_c_st, st->pred_state, st->state, &
                     gain_code );
            }
            ec_gain_code_update( st->ec_gain_c_st, bfi, st->prev_bf, &gain_code
                  );
            pit_sharp = gain_pit;
         }
      }

        /*
         * store pitch sharpening for next subframe
         * (for modes which use the previous pitch gain for
         *  pitch sharpening in the search phase)
         * do not update sharpening in even subframes for MR475
         */
      if ( ( mode != MR475 ) || evenSubfr == 0 ) {
         st->sharp = gain_pit;

         if ( st->sharp > SHARPMAX ) {
            st->sharp = SHARPMAX;
         }
      }

      if ( pit_sharp > 16383 )
         pit_sharp = 32767;
      else
         pit_sharp *= 2;

      if ( pit_sharp > 16384 ) {
         for ( i = 0; i < L_SUBFR; i++ ) {
            temp = ( st->exc[i] * pit_sharp ) >> 15;
            temp2 = ( temp * gain_pit ) << 1;

            if ( mode == MR122 ) {
               temp2 = ( temp2 >> 1 );
            }
            excp[i] = ( temp2 + 0x00008000L ) >> 16;
         }
      }

        /*
         * Store list of LTP gains needed in the source
         * characteristic detector (SCD)
         */
      if ( bfi == 0 ) {
         for (i = 0; i < 8; i++){
            st->ltpGainHistory[i] = st->ltpGainHistory[i+1];
         }
         st->ltpGainHistory[8] = gain_pit;
      }


        /*
         * Limit gain_pit if in background noise and BFI
         * for MR475, MR515, MR59
         */
      if ( ( st->prev_bf != 0 || bfi != 0 ) & ( st->inBackgroundNoise != 0 ) & (
            ( mode == MR475 ) || ( mode == MR515 ) || ( mode == MR59 ) ) ) {
         /* if (gain_pit > 0.75) in Q14*/
         if ( gain_pit > 12288 )
            /* gain_pit = (gain_pit-0.75)/2.0 + 0.75; */
            gain_pit = ( ( gain_pit - 12288 ) >> 1 ) + 12288;

         /* if (gain_pit > 0.90) in Q14*/
         if ( gain_pit > 14745 ) {
            gain_pit = 14745;
         }
      }

        /*
         * Calculate CB mixed gain
         */
      Int_lsf( prev_lsf, st->lsfState->past_lsf_q, i_subfr, lsf_i );
      gain_code_mix = Cb_gain_average( st->Cb_gain_averState, mode, gain_code,
            lsf_i, st->lsp_avg_st->lsp_meanSave, bfi, st->prev_bf, pdfi, st->
            prev_pdf, st->inBackgroundNoise, st->voicedHangover );

      /* make sure that MR74, MR795, MR122 have original codeGain*/
      /* MR74, MR795, MR122 */
      if ( ( mode > MR67 ) & ( mode != MR102 ) ) {
         gain_code_mix = gain_code;
      }

        /*
         * Find the total excitation.
         * Find synthesis speech corresponding to st->exc[].
         */
      /* MR475, MR515, MR59, MR67, MR74, MR795, MR102*/
      if ( mode <= MR102 ) {
         pitch_fac = gain_pit;
         tmp_shift = 1;
      }

      /* MR122 */
      else {
         pitch_fac = gain_pit >> 1;
         tmp_shift = 2;
      }

        /*
         * copy unscaled LTP excitation to exc_enhanced (used in phase
         * dispersion below) and compute total excitation for LTP feedback
         */
      memcpy( exc_enhanced, st->exc, L_SUBFR <<2 );

      for ( i = 0; i < L_SUBFR; i++ ) {
         /* st->exc[i] = gain_pit*st->exc[i] + gain_code*code[i]; */
         temp = ( st->exc[i] * pitch_fac ) + ( code[i] * gain_code );
         temp2 = ( temp << tmp_shift );
         if (((temp2 >> 1) ^ temp2) & 0x40000000) {
            if ((temp ^ temp2) & 0x80000000) {
               temp2 = (temp & 0x80000000) ? (-1073741824L) : 1073725439;
            }
            else {
               temp2 = (temp2 & 0x80000000) ? (-1073741824L) : 1073725439;
            }
         }
         st->exc[i] = ( temp2 + 0x00004000L ) >> 15;
      }
      /*
       * Adaptive phase dispersion
       */

      /* free phase dispersion adaption */
      st->ph_disp_st->lockFull = 0;

      if ( ( ( mode == MR475 ) || ( mode == MR515 ) || ( mode == MR59 ) ) & ( st
            ->voicedHangover > 3 ) & ( st->inBackgroundNoise != 0 ) & ( bfi != 0
            ) ) {
           /*
            * Always Use full Phase Disp.
            * if error in bg noise
            */
         st->ph_disp_st->lockFull = 1;
      }

        /*
         * apply phase dispersion to innovation (if enabled) and
         * compute total excitation for synthesis part
         */
      ph_disp( st->ph_disp_st, mode, exc_enhanced, gain_code_mix, gain_pit, code
            , pitch_fac, tmp_shift );

        /*
         * The Excitation control module are active during BFI.
         * Conceal drops in signal energy if in bg noise.
         */
      temp2 = 0;

      for ( i = 0; i < L_SUBFR; i++ ) {
         temp2 += ( exc_enhanced[i] * exc_enhanced[i] );
      }

      if ( temp2 > 0x3FFFFFFF ) {
         excEnergy = 11584;
      }
      else {
         temp2 = sqrt_l_exp( temp2, &temp );
         temp2 = ( temp2 >> ( ( temp >> 1 ) + 15 ) );
         excEnergy = temp2 >> 2;
      }

      if ( ( ( mode == MR475 ) || ( mode == MR515 ) || ( mode == MR59 ) ) & ( st
            ->voicedHangover > 5 ) & ( st->inBackgroundNoise != 0 ) & ( st->
            state < 4 ) & ( ( ( pdfi != 0 ) & ( st->prev_pdf != 0 ) ) || bfi !=
            0 || st->prev_bf != 0 ) ) {
         carefulFlag = 0;

         if ( ( pdfi != 0 ) & ( bfi == 0 ) ) {
            carefulFlag = 1;
         }
         Ex_ctrl( exc_enhanced, excEnergy, st->excEnergyHist, st->voicedHangover
               , st->prev_bf, carefulFlag );
      }

      if ( ( st->inBackgroundNoise != 0 ) & ( bfi != 0 || st->prev_bf != 0 ) & (
            st->state < 4 ) ) {;   /* do nothing! */
      }
      else {
         /* Update energy history for all modes */
         for (i = 0; i < 8; i++){
            st->excEnergyHist[i] = st->excEnergyHist[i+1];
         }
         st->excEnergyHist[8] = excEnergy;
      }

        /*
         * Excitation control module end.
         */
      if ( pit_sharp > 16384 ) {
         for ( i = 0; i < L_SUBFR; i++ ) {
            excp[i] = excp[i] + exc_enhanced[i];
            if (labs(excp[i]) > 32767)
               excp[i] = (excp[i] & 0x80000000) ? -32768 : 32767;
         }
         agc2( exc_enhanced, excp );
         overflow = Syn_filt( Az, excp, &synth[i_subfr], L_SUBFR, st->mem_syn, 0
            );
      }
      else {
         overflow = Syn_filt( Az, exc_enhanced, &synth[i_subfr], L_SUBFR, st->
            mem_syn, 0 );
      }

      if ( overflow ) {
         for ( i = 0; i < PIT_MAX + L_INTERPOL + L_SUBFR; i++ ) {
            st->old_exc[i] = st->old_exc[i] >> 2;
         }

         for ( i = 0; i < L_SUBFR; i++ ) {
            exc_enhanced[i] = exc_enhanced[i] >> 2;
         }
         Syn_filt_overflow( Az, exc_enhanced, &synth[i_subfr], L_SUBFR, st->mem_syn, 1 );
      }
      else {
         memcpy( st->mem_syn, &synth[i_subfr + 30], 40 );
      }

        /*
         * Update signal for next frame.
         * -> shift to the left by L_SUBFR  st->exc[]
         */
      memcpy( &st->old_exc[0], &st->old_exc[L_SUBFR], ( PIT_MAX + L_INTERPOL )<<
            2 );

      /* interpolated LPC parameters for next subframe */
      Az += MP1;

      /* store T0 for next subframe */
      st->old_T0 = T0;
   }

    /*
     * Call the Source Characteristic Detector which updates
     * st->inBackgroundNoise and st->voicedHangover.
     */
   st->inBackgroundNoise = Bgn_scd( st->background_state, &( st->ltpGainHistory[
         0] ), &( synth[0] ), &( st->voicedHangover ) );
   dtx_dec_activity_update( st->dtxDecoderState, st->lsfState->past_lsf_q, synth
         );

   /* store bfi for next subframe */
   st->prev_bf = bfi;
   st->prev_pdf = pdfi;

    /*
     * Calculate the LSF averages on the eight
     * previous frames
     */
   lsp_avg( st->lsp_avg_st, st->lsfState->past_lsf_q );
theEnd:
   st->dtxDecoderState->dtxGlobalState = newDTXState;
   return;
}


/*
 * Residu40
 *
 *
 * Parameters:
 *    a                 I: prediction coefficients
 *    x                 I: speech signal
 *    y                 O: residual signal
 *
 * Function:
 *    The LP residual is computed by filtering the input
 *    speech through the LP inverse filter a(z)
 *
 * Returns:
 *    void
 */
static void Residu40( Word32 a[], Word32 x[], Word32 y[] )
{
   Word32 s, i, j;


   for ( i = 0; i < 40; i++ ) {
      s = a[0] * x[i] + a[1] * x[i - 1] + a[2] * x[i - 2] + a[3] * x[i - 3];
      s += a[4] * x[i - 4] + a[5] * x[i - 5] + a[6] * x[i - 6] + a[7] * x[i - 7]
         ;
      s += a[8] * x[i - 8] + a[9] * x[i - 9] + a[10] * x[i - 10];
      y[i] = ( s + 0x800 ) >> 12;
      if (abs(y[i]) > 32767){
         /* go to safe mode */
         for (i = 0; i < 40; i++) {
            s = a[0] * x[i];
            for (j = 1; j <= 10; j++) {
               s += a[j] * x[i - j];
               if (s > 1073741823){
                  s = 1073741823;
               }
               else if ( s < -1073741824) {
                  s = -1073741824;
               }
            }
            y[i] = ( s + 0x800 ) >> 12;
            if (abs(y[i]) > 32767)
               y[i] = (y[i] & 0x80000000) ? -32768 : 32767;
         }
         return;
      }

   }
   return;
}


/*
 * agc
 *
 *
 * Parameters:
 *    st->past_gain     B: gain memory
 *    sig_in            I: Post_Filter input signal
 *    sig_out           B: Post_Filter output signal
 *    agc_fac           I: AGC factor
 *
 * Function:
 *    Scales the Post_Filter output on a subframe basis
 *
 * Returns:
 *    void
 */
static void agc( agcState *st, Word32 *sig_in, Word32 *sig_out, Word16 agc_fac )
{
   Word32 s, gain_in, gain_out, g0, gain;
   int exp, i;


   /* calculate gain_out with exponent */
   s = energy_new( sig_out );

   if ( s == 0 ) {
      st->past_gain = 0;
      return;
   }
   exp=0;
   i = s;
   while (!(i & 0x40000000))
   {
      exp++;
      i = i << 1;
   }
   exp -=1;
   if (exp & 0x80000000) {
      s >>= 1;
   }
   else {
      s <<= exp;
   }
   gain_out = ( s + 0x00008000L ) >> 16;

   /* calculate gain_in with exponent */
   s = energy_new( sig_in );

   if ( s == 0 ) {
      g0 = 0;
   }
   else {
      i=0;
   while (!(s & 0x40000000))
   {
      i++;
      s = s << 1;
   }
      s = s + 0x00008000L;

      if ( s >= 0 )
         gain_in = s >> 16;
      else
         gain_in = 32767;
      exp = ( exp - i );

      /*
       * g0 = (1-agc_fac) * sqrt(gain_in/gain_out);
       */
      /* s = gain_out / gain_in */
      s = ( gain_out << 15 ) / gain_in;
      exp = 7 - exp;

      if ( exp > 0 ){
         if (exp > 31)
         {
            if(s){
               s = 2147483647;
            }
         }
         else {
            s = s << exp ;
         }
      }
      else
         s = ( s >> ( -exp ) );
      if (s < 0)
         s = 2147483647;
      s = Inv_sqrt( s );
      i = ( ( s << 9 ) + 0x00008000L ) >> 16;
      if (i & 0xFFFF8000)
         i = 32767;

      /* g0 = i * (1-agc_fac) */
      g0 = ( i * ( 32767 - agc_fac ) ) >> 15;
   }

   /*
    * compute gain[n] = agc_fac * gain[n-1] + (1-agc_fac) * sqrt(gain_in/gain_out)
    * sig_out[n] = gain[n] * sig_out[n]
    */
   gain = st->past_gain;

   for ( i = 0; i < L_SUBFR; i++ ) {
      gain = ( gain * agc_fac ) >> 15;
      gain = gain + g0;
      sig_out[i] = ( sig_out[i] * gain ) >> 12;
      if (labs(sig_out[i]) > 32767)
         sig_out[i] = (sig_out[i] & 0x8000000) ? -32768 : 32767;
   }
   st->past_gain = gain;
   return;
}


/*
 * Post_Filter
 *
 *
 * Parameters:
 *    st                B: post filter states
 *    mode              I: AMR mode
 *    syn               B: synthesis speech
 *    Az_4              I: interpolated LPC parameters in all subfr.
 *
 * Function:
 *    Post_Filtering of synthesis speech.
 *
 *    inverse filtering of syn[] through A(z/0.7) to get res2[]
 *    tilt compensation filtering; 1 - MU*k*z^-1
 *    synthesis filtering through 1/A(z/0.75)
 *    adaptive gain control
 *
 * Returns:
 *    void
 */
static void Post_Filter( Post_FilterState *st, enum Mode mode, Word32 *syn,
      Word32 *Az_4 )
{
   Word32 h[22], Ap3[MP1], Ap4[MP1];   /* bandwidth expanded LP parameters */
   Word32 tmp, i_subfr, i, temp1, temp2, overflow = 0;
   Word32 *Az, *p1, *p2, *syn_work = &st->synth_buf[M];
   const Word32 *pgamma3 = &gamma3[0];
   const Word32 *pgamma4 = &gamma4_gamma3_MR122[0];


   /*
    * Post filtering
    */
   memcpy( syn_work, syn, L_FRAME <<2 );
   Az = Az_4;

   if ( ( mode == MR122 ) || ( mode == MR102 ) ) {
      pgamma3 = &gamma4_gamma3_MR122[0];
      pgamma4 = &gamma4_MR122[0];
   }

   for ( i_subfr = 0; i_subfr < L_FRAME; i_subfr += L_SUBFR ) {
      /* Find weighted filter coefficients Ap3[] and Ap[4] */
      Ap3[0] = Az[0];
      Ap4[0] = Az[0];

      for ( i = 1; i <= 10; i++ ) {
         Ap3[i] = ( Az[i] * pgamma3[i - 1] +0x4000 ) >> 15;
         Ap4[i] = ( Az[i] * pgamma4[i - 1] +0x4000 ) >> 15;
      }

      /* filtering of synthesis speech by A(z/0.7) to find res2[] */
      Residu40( Ap3, &syn_work[i_subfr], st->res2 );

      /* tilt compensation filter */
      /* impulse response of A(z/0.7)/A(z/0.75) */
      memcpy( h, Ap3, MP1 <<2 );
      memset( &h[M +1], 0, ( 22 - M - 1 )<<2 );
      Syn_filt( Ap4, h, h, 22, &h[M +1], 0 );

      /* 1st correlation of h[] */
      tmp = 16777216 + h[1] * h[1];

      for ( i = 2; i < 22; i++ ) {
         tmp += h[i] * h[i];
         if (tmp > 0x3FFF8000)
            break;
      }
      temp1 = tmp >> 15;
      if (temp1 & 0xFFFF8000)
         temp1 = 32767;

      tmp = h[0] * h[1];

      for ( i = 1; i < 21; i++ ) {
         tmp += h[i] * h[i + 1];
         if (abs(tmp) > 1073741823)
            tmp = 1073741823;
      }
      temp2 = tmp >> 15;

      if ( temp2 <= 0 ) {
         temp2 = 0;
      }
      else {
         tmp = temp2 * 26214;
         temp2 = ( tmp & 0xffff8000 ) / temp1;
      }

      /* preemphasis */
      p1 = st->res2 + 39;
      p2 = p1 - 1;
      tmp = *p1;

      do {
         *p1 = *p1 - ( ( temp2 * *p2-- ) >> 15 );
         if (abs(*p1) > 32767) {
            *p1 = (*p1 & 0x80000000) ? -32768 : 32767;
         }
         p1--;
         *p1 = *p1 - ( ( temp2 * *p2-- ) >> 15 );
         if (abs(*p1) > 32767) {
            *p1 = (*p1 & 0x80000000) ? -32768 : 32767;
         }
         p1--;
         *p1 = *p1 - ( ( temp2 * *p2-- ) >> 15 );
         if (abs(*p1) > 32767) {
            *p1 = (*p1 & 0x80000000) ? -32768 : 32767;
         }
         p1--;
      } while( p1 > st->res2 );
      *p1 = *p1 - ( ( temp2 * st->preemph_state_mem_pre ) >> 15 );
      if (abs(*p1) > 32767) {
         *p1 = (*p1 & 0x80000000) ? -32768 : 32767;
      }
      st->preemph_state_mem_pre = tmp;

      /* filtering through  1/A(z/0.75) */
      overflow = Syn_filt( Ap4, st->res2, &syn[i_subfr], L_SUBFR, st->mem_syn_pst, 0 );
      if (overflow){
         Syn_filt_overflow( Ap4, st->res2, &syn[i_subfr], L_SUBFR, st->mem_syn_pst, 1 );
         overflow = 0;
      }
      else {
         memcpy(st->mem_syn_pst, &syn[i_subfr + 30], 40);
      }

      /* scale output to input */
      agc( st->agc_state, &syn_work[i_subfr], &syn[i_subfr], AGC_FAC );
      Az += MP1;
   }

   /* update syn_work[] buffer */
   memcpy( &syn_work[- M], &syn_work[L_FRAME - M], M <<2 );
   return;
}


/*
 * Post_Process
 *
 *
 * Parameters:
 *    st                B: post filter states
 *    signal            B: signal
 *
 * Function:
 *    Postprocessing of input speech.
 *
 *    2nd order high pass filtering with cut off frequency at 60 Hz.
 *    Multiplication of output by two.
 *
 *
 * Returns:
 *    void
 */
 static void Post_Process( Post_ProcessState *st, Word32 signal[] )
 {
    Word32 x2, tmp, i = 0;
    Word32 mask = 0x40000000;

    do {
       x2 = st->x1;
       st->x1 = st->x0;
       st->x0 = signal[i];

       /*
       * y[i] = b[0]*x[i]*2 + b[1]*x[i-1]*2 + b140[2]*x[i-2]/2
       *                    + a[1]*y[i-1] + a[2] * y[i-2];
       */
       tmp = ( st->y1_hi * 15836) + ( ( ( st->y1_lo * 15836 ) & ( Word32 )0xffff8000 ) >> 15);
       tmp += (st->y2_hi * -7667) + ( ( ( st->y2_lo * ( -7667 ) ) & ( Word32 )0xffff8000 ) >> 15);
       tmp += st->x0 * 7699;
       tmp += st->x1 * -15398;
       if ( ( (tmp >> 1) ^ tmp ) & mask)
          tmp = (tmp & 0x80000000) ? -1073741824 : 1073741823;

       tmp += x2 * 7699;
       if ( ( (tmp >> 1) ^ tmp ) & mask)
          tmp = (tmp & 0x80000000) ? -1073741824 : 1073741823;

       tmp = tmp << 1;
       if ( ( (tmp >> 1) ^ tmp ) & mask)
          tmp = (tmp & 0x80000000) ? -1073741824 : 1073741823;

       tmp = tmp << 1;
       if ( ( (tmp >> 1) ^ tmp ) & mask)
          tmp = (tmp & 0x80000000) ? -1073741824 : 1073741823;

       if ( labs( tmp ) < 536862720 ) {
          signal[i++] = ( tmp + 0x00002000L ) >> 14;
       }
       else if ( tmp > 0 ) {
          signal[i++] = 32767;
       }
       else {
          signal[i++] = -32768;
       }
       st->y2_hi = st->y1_hi;
       st->y2_lo = st->y1_lo;
       st->y1_hi = tmp >> 15;
       st->y1_lo = ( ( tmp << 1 ) - ( st->y1_hi << 16 ) ) >> 1;
    } while( i < 160 );
    return;
}


/*
 * Speech_Decode_Frame
 *
 *
 * Parameters:
 *    st                B: decoder memory
 *    mode              I: AMR mode
 *    parm              I: speech parameters
 *    frame_type        I: Frame type
 *    synth             O: synthesis speech

 * Function:
 *    Decode one frame
 *
 * Returns:
 *    void
 */
void Speech_Decode_Frame( void *st, enum Mode mode, Word16 *parm, enum
      RXFrameType frame_type, Word16 *synth )
{
   Word32 Az_dec[AZ_SIZE];   /* Decoded Az for post-filter in 4 subframes*/
   Word32 synth_speech[L_FRAME];
   Word32 i;

   /* Synthesis */
   Decoder_amr( ( ( Speech_Decode_FrameState * ) st )->decoder_amrState, mode,
         parm, frame_type, synth_speech, Az_dec );
   Post_Filter( ( ( Speech_Decode_FrameState * ) st )->post_state, mode,
         synth_speech, Az_dec );

   /* post HP filter, and 15->16 bits */
   Post_Process( ( ( Speech_Decode_FrameState * ) st )->postHP_state,
         synth_speech );

for ( i = 0; i < L_FRAME; i++ ) {
#ifndef NO13BIT
      /* Truncate to 13 bits */
      synth[i] = ( Word16 )( synth_speech[i] & 0xfff8 );
#else
      synth[i] = ( Word16 )( synth_speech[i]);
#endif
   }


   return;
}


/*
 * Decoder_amr_exit
 *
 *
 * Parameters:
 *    state                I: state structure
 *
 * Function:
 *    The memory used for state memory is freed
 *
 * Returns:
 *    Void
 */
static void Decoder_amr_exit( Decoder_amrState **state )
{
   if ( state == NULL || *state == NULL )
      return;
   free( ( *state )->lsfState );
   free( ( *state )->ec_gain_p_st );
   free( ( *state )->ec_gain_c_st );
   free( ( *state )->pred_state );
   free( ( *state )->background_state );
   free( ( *state )->ph_disp_st );
   free( ( *state )->Cb_gain_averState );
   free( ( *state )->lsp_avg_st );
   free( ( *state )->dtxDecoderState );

   /* deallocate memory */
   free( *state );
   *state = NULL;
   return;
}


/*
 * Post_Filter_exit
 *
 *
 * Parameters:
 *    state                I: state structure
 *
 * Function:
 *    The memory used for state memory is freed
 *
 * Returns:
 *    Void
 */
static void Post_Filter_exit( Post_FilterState **state )
{
   if ( state == NULL || *state == NULL )
      return;
   free( ( *state )->agc_state );

   /* deallocate memory */
   free( *state );
   *state = NULL;
   return;
}


/*
 * Post_Process_reset
 *
 *
 * Parameters:
 *    state             B: state structure
 *
 * Function:
 *    Resets state memory
 *
 * Returns:
 *    -1 failure
 */
static int Post_Process_reset( Post_ProcessState *state )
{
   if ( ( Post_ProcessState * )state == NULL ) {
      fprintf( stderr, "Post_Process_reset: invalid parameter\n" );
      return-1;
   }
   state->y2_hi = 0;
   state->y2_lo = 0;
   state->y1_hi = 0;
   state->y1_lo = 0;
   state->x0 = 0;
   state->x1 = 0;
   return 0;
}


/*
 * Post_Process_exit
 *
 *
 * Parameters:
 *    state                I: state structure
 *
 * Function:
 *    The memory used for state memory is freed
 *
 * Returns:
 *    Void
 */
static void Post_Process_exit( Post_ProcessState **state )
{
   if ( state == NULL || *state == NULL )
      return;

   /* deallocate memory */
   free( *state );
   *state = NULL;
   return;
}


/*
 * Decoder_amr_init
 *
 *
 * Parameters:
 *    state             O: state structure
 *
 * Function:
 *    Allocates state memory and initializes state memory
 *
 * Returns:
 *    success = 0
 */
static int Decoder_amr_init( Decoder_amrState **state )
{
   Decoder_amrState * s;

   if ( ( Decoder_amrState * )state == NULL ) {
      fprintf( stderr, "Decoder_amr_init: invalid parameter\n" );
      return-1;
   }
   *state = NULL;

   /* allocate memory */
   if ( ( s = ( Decoder_amrState * ) malloc( sizeof( Decoder_amrState ) ) ) ==
         NULL ) {
      fprintf( stderr, "Decoder_amr_init: can not malloc state structure\n" );
      return-1;
   }

   /* DPlsf_init */
   /* allocate memory */
   if ( ( s->lsfState = ( D_plsfState * ) malloc( sizeof( D_plsfState ) ) ) ==
         NULL ) {
      fprintf( stderr, "DPlsf_init: can not malloc state structure\n" );
      return-1;
   }

   /* ecGainPitchInit */
   /* allocate memory */
   if ( ( s->ec_gain_p_st = ( ec_gain_pitchState * ) malloc( sizeof(
         ec_gain_pitchState ) ) ) == NULL ) {
      fprintf( stderr, "ecGainPitchInit: can not malloc state structure\n" );
      return-1;
   }

   /* ecGainCodeInit */
   /* allocate memory */
   if ( ( s->ec_gain_c_st = ( ec_gain_codeState * ) malloc( sizeof(
         ec_gain_codeState ) ) ) == NULL ) {
      fprintf( stderr, "ecGainCodeInit: can not malloc state structure\n" );
      return-1;
   }

   /* gcPredInit */
   /* allocate memory */
   if ( ( s->pred_state = ( gc_predState * ) malloc( sizeof( gc_predState ) ) )
         == NULL ) {
      fprintf( stderr, "gcPredInit: can not malloc state structure\n" );
      return-1;
   }

   /* Cb_gain_averageInit */
   /* allocate memory */
   if ( ( s->Cb_gain_averState = ( Cb_gain_averageState * ) malloc( sizeof(
         Cb_gain_averageState ) ) ) == NULL ) {
      fprintf( stderr, "Cb_gain_averageInit: can not malloc state structure\n" )
      ;
      return-1;
   }
   memset( s->Cb_gain_averState->cbGainHistory, 0, L_CBGAINHIST <<2 );

   /* Initialize hangover handling */
   s->Cb_gain_averState->hangVar = 0;
   s->Cb_gain_averState->hangCount = 0;

   /* lsp_avgInit */
   /* allocate memory */
   if ( ( s->lsp_avg_st = ( lsp_avgState * ) malloc( sizeof( lsp_avgState ) ) )
         == NULL ) {
      fprintf( stderr, "lsp_avgInit: can not malloc state structure\n" );
      return-1;
   }

   /* Bgn_scdInit */
   /* allocate memory */
   if ( ( s->background_state = ( Bgn_scdState * ) malloc( sizeof( Bgn_scdState
         ) ) ) == NULL ) {
      fprintf( stderr, "Bgn_scdInit: can not malloc state structure\n" );
      return-1;
   }

   /* phDispInit */
   /* allocate memory */
   if ( ( s->ph_disp_st = ( ph_dispState * ) malloc( sizeof( ph_dispState ) ) )
         == NULL ) {
      fprintf( stderr, "phDispInit: can not malloc state structure\n" );
      return-1;
   }

   /* dtxDecInit */
   /* allocate memory */
   if ( ( s->dtxDecoderState = ( dtx_decState * ) malloc( sizeof( dtx_decState )
         ) ) == NULL ) {
      fprintf( stderr, "dtxDecInit: can not malloc state structure\n" );
      return-1;
   }
   Decoder_amr_reset( s, 0 );
   *state = s;
   return 0;
}


/*
 * Post_Filter_reset
 *
 *
 * Parameters:
 *    state             B: state structure
 *
 * Function:
 *    Resets state memory
 *
 * Returns:
 *    -1 failure
 */
static int Post_Filter_reset( Post_FilterState *state )
{
   if ( ( Post_FilterState * )state == NULL ) {
      fprintf( stderr, "Post_Filter_reset: invalid parameter\n" );
      return-1;
   }
   state->preemph_state_mem_pre = 0;
   state->agc_state->past_gain = 4096;
   memset( state->mem_syn_pst, 0, M <<2 );
   memset( state->res2, 0, L_SUBFR <<2 );
   memset( state->synth_buf, 0, ( L_FRAME + M )<<2 );
   return 0;
}


/*
 * Post_Filter_init
 *
 *
 * Parameters:
 *    state             O: state structure
 *
 * Function:
 *    Allocates state memory and initializes state memory
 *
 * Returns:
 *    success = 0
 */
static int Post_Filter_init( Post_FilterState **state )
{
   Post_FilterState * s;

   if ( ( Post_FilterState * )state == NULL ) {
      fprintf( stderr, "F057:invalid parameter\n" );
      return-1;
   }
   *state = NULL;

   /* allocate memory */
   if ( ( s = ( Post_FilterState * ) malloc( sizeof( Post_FilterState ) ) ) ==
         NULL ) {
      fprintf( stderr, "F057:can not malloc filter structure\n" );
      return-1;
   }
   s->agc_state = NULL;

   /* allocate memory */
   if ( ( s->agc_state = ( agcState * ) malloc( sizeof( agcState ) ) ) == NULL )
   {
      fprintf( stderr, "agcInit: can not malloc state structure\n" );
      return-1;
   }
   Post_Filter_reset( s );
   *state = s;
   return 0;
}


/*
 * Post_Process_init
 *
 *
 * Parameters:
 *    state             O: state structure
 *
 * Function:
 *    Allocates state memory and initializes state memory
 *
 * Returns:
 *    success = 0
 */
static int Post_Process_init( Post_ProcessState **state )
{
   Post_ProcessState * s;

   if ( ( Post_ProcessState * )state == NULL ) {
      fprintf( stderr, "Post_Process_init: invalid parameter\n" );
      return-1;
   }
   *state = NULL;

   /* allocate memory */
   if ( ( s = ( Post_ProcessState * ) malloc( sizeof( Post_ProcessState ) ) ) ==
         NULL ) {
      fprintf( stderr, "Post_Process_init: can not malloc state structure\n" );
      return-1;
   }
   Post_Process_reset( s );
   *state = s;
   return 0;
}


/*
 * Speech_Decode_Frame_exit
 *
 *
 * Parameters:
 *    state                I: state structure
 *
 * Function:
 *    The memory used for state memory is freed
 *
 * Returns:
 *    Void
 */
void Speech_Decode_Frame_exit( void **st )
{
   if ( (( Speech_Decode_FrameState * )( st )) == NULL )
      return;
   Decoder_amr_exit( &( ( ( Speech_Decode_FrameState * ) st )->decoder_amrState
         ) );
   Post_Filter_exit( &( ( ( Speech_Decode_FrameState * ) st )->post_state ) );
   Post_Process_exit( &( ( ( Speech_Decode_FrameState * ) st )->postHP_state ) )
   ;

   /* deallocate memory */
   free( (( Speech_Decode_FrameState * )st) );
   return;
}


/*
 * Speech_Decode_Frame_reset
 *
 *
 * Parameters:
 *    state             B: state structure
 *
 * Function:
 *    Resets state memory
 *
 * Returns:
 *    -1 = failure
 */
int Speech_Decode_Frame_reset( void **st )
{
   Speech_Decode_FrameState * state;

   if ( st == NULL || *st == NULL )
      return (-1);
   state = ( Speech_Decode_FrameState * )st;
   Decoder_amr_reset( state->decoder_amrState, ( enum Mode ) 0 );
   Post_Filter_reset( state->post_state );
   Post_Process_reset( state->postHP_state );
   return 0;
}


/*
 * Speech_Decode_Frame_init
 *
 *
 * Parameters:
 *    state             O: state structure
 *
 * Function:
 *    Allocates state memory and initializes state memory
 *
 * Returns:
 *    success = 0
 */
void * Speech_Decode_Frame_init( )
{
   Speech_Decode_FrameState * s;

   /* allocate memory */
   if ( ( s = ( Speech_Decode_FrameState * ) malloc( sizeof(
         Speech_Decode_FrameState ) ) ) == NULL ) {
      fprintf( stderr, "Speech_Decode_Frame_init: can not malloc state "
            "structure\n" );
      return NULL;
   }
   s->decoder_amrState = NULL;
   s->post_state = NULL;
   s->postHP_state = NULL;

   if ( Decoder_amr_init( &s->decoder_amrState ) || Post_Filter_init( &s->
         post_state ) || Post_Process_init( &s->postHP_state ) ) {
      Speech_Decode_Frame_exit( ( void ** )( &s ) );
      return NULL;
   }
   return s;
}
