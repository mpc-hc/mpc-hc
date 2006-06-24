/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: dirac_encoder.h 280 2005-01-30 05:11:46Z gabest $ $Name$
*
* Version: MPL 1.1/GPL 2.0/LGPL 2.1
*
* The contents of this file are subject to the Mozilla Public License
* Version 1.1 (the "License"); you may not use this file except in compliance
* with the License. You may obtain a copy of the License at
* http://www.mozilla.org/MPL/
*
* Software distributed under the License is distributed on an "AS IS" basis,
* WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for
* the specific language governing rights and limitations under the License.
*
* The Original Code is BBC Research and Development code.
*
* The Initial Developer of the Original Code is the British Broadcasting
* Corporation.
* Portions created by the Initial Developer are Copyright (C) 2004.
* All Rights Reserved.
*
* Contributor(s): Anuradha Suraparaju (Original Author)
*
* Alternatively, the contents of this file may be used under the terms of
* the GNU General Public License Version 2 (the "GPL"), or the GNU Lesser
* Public License Version 2.1 (the "LGPL"), in which case the provisions of
* the GPL or the LGPL are applicable instead of those above. If you wish to
* allow use of your version of this file only under the terms of the either
* the GPL or LGPL and not to allow others to use your version of this file
* under the MPL, indicate your decision by deleting the provisions above
* and replace them with the notice and other provisions required by the GPL
* or LGPL. If you do not delete the provisions above, a recipient may use
* your version of this file under the terms of any one of the MPL, the GPL
* or the LGPL.
* ***** END LICENSE BLOCK ***** */

#ifndef DIRAC_ENCODER_H
#define DIRAC_ENCODER_H

#include <libdirac_common/dirac_types.h>

/*! \file
\brief C interface to Dirac Encoder.
 
 A set of 'C' functions that define the public interface to the Dirac encoder.
 Refer to the the reference encoder source code, encoder/encmain.cpp for
 an example of how to use the "C" interface. The pseudocode below gives
 a brief description of the "C" interface usage.

\verbatim
 #include <libdirac_decoder/dirac_encoder.h>

 #define ENCBUF_SIZE 1024*1024;
 unsigned char *buffer, enc_buf[ENC_BUFSIZE];
 int buffer_size;
 dirac_encoder_t *encoder;
 dirac_encoder_context_t enc_ctx;

 // Initialse the encoder context with the presets for SD576 - Standard
 // Definition Digital
 dirac_encoder_context_init (&enc_ctx, SD576);

 // Override parameters if required
 // interlace : 1 - interlaced; 0 - progressive
 enc_ctx.seq_params.interlace = 0;
 enc_ctx.seq_params.topfieldfirst = 0;
 enc_ctx.enc_params.qf = 7.5;
 // disable instrumentation flag
 enc_ctx.instr_flag = 0;
 // return locally decoded output
 enc_ctx.decode_flag = 1;

 // Initialise the encoder with the encoder context. 
 // Setting verbose output to false
 encoder= dirac_encoder_init(&enc_ctx, false);

 // Set the buffer size. For SD576 4:2:0 chroma
 buffer_size = (720*576*3)/2;
 buffer = (unsigned char *)malloc (buffer_size);

 // Output buffer

 dirac_encoder_state_t state;

 while (read uncompressed frame data into buffer)
 {
    // load one frame of data into encoder
     if (dirac_encoder_load(encoder, buffer, buffer_size) == 0)
    {
        // Retrieve encoded frames from encoder
        do
        {
            encoder->enc_buf.buffer = enc_buf;
            encoder->enc_buf.size = ENCBUF_SIZE;
            state = dirac_encoder_output (encoder);
            switch (state)
            {
            case ENC_STATE_AVAIL:
                 // Encoded frame available in encoder->enc_buf
                 // Encoded frame params available in enccoder->enc_fparams
                 // Encoded frame stats available in enccoder->enc_fstats
                 break;
            case ENC_STATE_BUFFER:
                break;
            case ENC_STATE_INVALID:
            default:
                // Unrecoverable error encountered. Exit;
                exit (exit code);
            }
            if (encoder->decoded_frame_avail)
            {
                //locally decoded frame is available in 
                //encoder->dec_buf
                //locally decoded frame parameters available
                //in encoder->dec_fparams
            }
            if (encoder->instr_data_avail)
            {
                //Instrumentation data (motion vectors etc.)
                //available in encoder->instr
            }
        } while (state == ENC_STATE_AVAIL)
    }
 }
 // Retrieve end of sequence info
 encoder->enc_buf.buffer = video_buf;
 encoder->enc_buf.size = VIDEO_BUFFER_SIZE;
 dirac_encoder_end_sequence( encoder );
 // End of sequence info is availale in encoder->enc_buf
 // Sequence statistics available in encoder->enc_seqstats;

 // Free the encoder resources
 dirac_encoder_close(encoder)
 // Free the uncompressed data buffer
 free (buffer);

 \endverbatim
*/

#ifdef __cplusplus
extern "C" {
#endif

/*! Enumerated type that defines encoder state */
typedef enum
{ 
    ENC_STATE_INVALID = -1, 
    ENC_STATE_BUFFER, 
    ENC_STATE_AVAIL
} dirac_encoder_state_t ;

/*! Enumerated type that defines encoder presets that set the encoder and
    sequence paramters.  More presets may be added in future*/
typedef enum
{
    CIF, 
    SD576, 
    HD720, 
    HD1080
} dirac_encoder_presets_t;

/*! Structure that holds the encoder specific parameters */
typedef struct 
{
    /*! Quality factor 0.0 to 10.0 */
    float qf;
    /*! The separation between L1 frames */
    int L1_sep;
    /*! The number of L1 frames before the next intra frame. Together
        with L1_sep determines the GOP structure.
    */
    int num_L1;
    /*! Normalised viewing distance parameter, in cycles per degree */
    float cpd;
    /*! The width of blocks used for motion compensation */
    int xblen;
    /*! The height of blocks used for motion compensation */
    int yblen;
    /*! The horizontal separation between blocks. Always <xblen */
    int xbsep;
    /*! The vertical separation between blocks. Always <yblen */
    int ybsep;
} dirac_encparams_t;

/*! Structure that holds the parameters that set up the encoder context */
typedef struct
{
    /*! Sequence parameters */
    dirac_seqparams_t seq_params;
    /*! Encoder parameters */
    dirac_encparams_t enc_params;
    /*! Return diagnostics info 1-return mv data, 0-no diagnostics returned */
    int instr_flag;
    /*! Return locally decoded frames  1-return locally decoded frames, 
                                       0-no decoded frames returned */
    int decode_flag;
} dirac_encoder_context_t;

/*! Function that creates an encoder context based on a preset value. The
    values can then be overridden by the user by setting each field separately
    \param   enc_ctx    pointer to Encoder context tp be initialised.
    \param   preset     Preset to be used to initialise the encoder context
    \verbatim
     The sequence parameters and encoder parameters are initialised as follows

     Sequence Parameters:
     Preset           Field          Value
     CIF              width          352
                      height         288
                      chroma         Planar YUV 4:2:0
                      chroma_width   calculated from width and chroma
                      chroma_height  calculated from height and chroma
                      frame_rate     13/1
                      interlace      0 (progressive)
                      topfieldfirst  0 
     SD576            width          720
                      height         576
                      chroma         Planar YUV 4:2:0
                      chroma_width   calculated from width and chroma
                      chroma_height  calculated from height and chroma
                      frame_rate     25/1
                      interlace      1 (interlaced)
                      topfieldfirst  1 
     HD720            width          1280
                      height         720
                      chroma         Planar YUV 4:2:0
                      chroma_width   calculated from width and chroma
                      chroma_height  calculated from height and chroma
                      frame_rate     50/1
                      interlace      0 (progressive)
                      topfieldfirst  0 
     HD1080           width          1920
                      height         1080
                      chroma         Planar YUV 4:2:0
                      chroma_width   calculated from width and chroma
                      chroma_height  calculated from height and chroma
                      frame_rate     25/1
                      interlace      1 (interlaced)
                      topfieldfirst  1 

     Encoder params:
     Preset           Field          Value
     CIF              qf             7
                      L1_sep         3
                      num_L1         11
                      cpd            20.0
                      xblen          12
                      yblen          12
                      xbsep          8
                      ybsep          8

     SD576            qf             7
                      L1_sep         3
                      num_L1         3
                      cpd            32.0
                      xblen          12
                      yblen          12
                      xbsep          8
                      ybsep          8

     HD720            qf             7
                      L1_sep         3
                      num_L1         7
                      cpd            20.0
                      xblen          16
                      yblen          16
                      xbsep          10
                      ybsep          12

     HD1080           qf             7
                      L1_sep         3
                      num_L1         3
                      cpd            32.0
                      xblen          20
                      yblen          20
                      xbsep          16
                      ybsep          16

    \endverbatim
*/
extern DllExport void dirac_encoder_context_init (dirac_encoder_context_t *enc_ctx, dirac_encoder_presets_t preset);


/*! Structure that holds the encoded data*/
typedef struct
{
    /*! Buffer to hold encoded.  Allocated and managed by library user. */
    unsigned char *buffer;
    /*! Buffer size */
    int size;
} dirac_enc_data_t;

/*! Structure that holds the statistics about the encoded frame */
typedef struct
{
    /*! Number of motion vector bits */
    unsigned int mv_bits;
    /*! Number of motion vector header bits */
    unsigned int mv_hdr_bits;
    /*! Number of  used to encode y component */
    unsigned int ycomp_bits;
    /*! y component header bits*/
    unsigned int ycomp_hdr_bits;
    /*! Number of  used to encode u component */
    unsigned int ucomp_bits;
    /*! v component header bits*/
    unsigned int ucomp_hdr_bits;
    /*! Number of  used to encode v component */
    unsigned int vcomp_bits;
    /*! v component header bits*/
    unsigned int vcomp_hdr_bits;
    /*! Total number of bits used to encode frame */
    unsigned int frame_bits;
    /*! Number of frame header bits */
    unsigned int frame_hdr_bits;
} dirac_enc_framestats_t;

/*! Structure that holds the statistics about the encoded sequence */
typedef struct
{
    /*! Number of motion vector bits */
    unsigned int mv_bits;
    /*! Total number of bits used to encode sequence */
    unsigned int seq_bits;
    /*! Number of sequence header bits */
    unsigned int seq_hdr_bits;
    /*! Number of  used to encode y component */
    unsigned int ycomp_bits;
    /*! Number of  used to encode u component */
    unsigned int ucomp_bits;
    /*! Number of  used to encode v component */
    unsigned int vcomp_bits;
    /*! Average bit rate for the sequence */
    unsigned int bit_rate;
} dirac_enc_seqstats_t;

/*! Structure that holds the motion vector information */
typedef struct
{
    /*! X component */
    int x;
    /*! Y component */
    int y;
} dirac_mv_t;

/*! Structure that holds the motion vector cost information*/
typedef struct
{
    /*! The Sum of Absolute Differences */
    float SAD;
    /*! The (Lagrangian-weighted) motion vector cost */
    float mvcost;
} dirac_mv_cost_t;

/*! Structure that diagnostics data returned by the encoder */
typedef struct
{
    /*! Frame type */
    dirac_frame_type_t ftype;
    /*! Frame number */
    int fnum;
    /*! Number of reference frames */
    int num_refs;
    /*! Array of Reference frame numbers */
    int refs[2];
    /*! Block separation in X direction */
    int xbsep;
    /*! Block separation in Y direction */
    int ybsep;
    /*! MacroBlock length in X direction */
    int mb_xlen;
    /*! MacroBlock length in Y direction */
    int mb_ylen;
    /*! Motion Vector array length in X direction */
    int mv_xlen;
    /*! Motion Vector array length in Y direction */
    int mv_ylen;
    /*! Macro-block split mode array - mb_ylen*mb_xlen*/
    int *mb_split_mode;
    /*! Macro-block common mode array - mb_ylen*mb_xlen*/
    int *mb_common_mode;
    /*! Macro-block costs array - mb_ylen*mb_xlen*/
    float *mb_costs;
    /*! Block prediction mode - mv_xlen*mv_ylen */
    int *pred_mode;
    /*! Block intrac costs - mv_xlen*mv_ylen */
    float *intra_costs;
    /*! Bi prediction costs - mv_xlen*mv_ylen*2 */
    dirac_mv_cost_t *bipred_costs;
    /*! DC values of y_comp */
    short *dc_ycomp;
    /*! DC values of u_comp */
    short *dc_ucomp;
    /*! DC values of v_comp */
    short *dc_vcomp;
    /*! Motion vectors for Reference frames mv_ylen*mv_xlen */
    dirac_mv_t *mv[2];
    /*! Predictions costs for Reference frames mv_ylen*mv_xlen */
    dirac_mv_cost_t *pred_costs[2];
} dirac_instr_t;

/*! Structure that holds the information returned by the encoder */
typedef struct
{
    /*! Encoder context */
    dirac_encoder_context_t enc_ctx;

    /*! encoded frame avail flag */
    int encoded_frame_avail;

    /*! 
        encoded output. This buffer must be initialised by the user of the
        library
    */
    dirac_enc_data_t enc_buf;

    /*! encoded frame params */
    dirac_frameparams_t enc_fparams;

    /*! encoded frame stats */
    dirac_enc_framestats_t enc_fstats;

    /*! encoded frame stats */
    dirac_enc_seqstats_t enc_seqstats;

    /*! end of sequence */
    int end_of_sequence;

    /* locally decoded frame available flag. 
       1 - locally decoded frame available in dec_buf. 
       0 - locally decoded frame not available.
    */
    int decoded_frame_avail;

    /*! 
       locally decoded output buffer. This buffer is allocated and managed by 
       the encoder library
    */
    dirac_framebuf_t dec_buf;

    /*! locally decoded frame params */
    dirac_frameparams_t dec_fparams;

    /*! 
       instrumentation data buffer. This buffer is allocated and managed by 
       the encoder library. */
    dirac_instr_t instr;

    /*! instrumentation data available flag
       1 - instrumentation data available in instr
       0 - linstrumentation data not available.
    */
    int instr_data_avail;

    /*! void pointer to internal sequence compressor */
    const void *compressor;
} dirac_encoder_t;

/*!
    Initialise encoder. Makes a copy of the enc_ctx passed to it.
    \param   enc_ctx    Parameters to initialise encoder context
    \param   verbose    boolean flag to set verbose output
    \return  encoder    Handle to encoder if successful or NULL on failure
*/
extern DllExport dirac_encoder_t *dirac_encoder_init (const dirac_encoder_context_t *enc_ctx, int verbose);

/*!
    Load uncompressed data into the encoder. Expects one full frame of data
    \param   encoder         Encoder Handle
    \param   uncdata         Uncompressed data buffer
    \param   uncdata_size    boolean flag to set verbose output
    \return                  return status. >0 - successful; -1 failed
                             Failure may be due to input data size not matching
                             the required frame size.
*/
extern DllExport int dirac_encoder_load (dirac_encoder_t *encoder, unsigned char *uncdata, int uncdata_size);

/*!
    Retrieve an encoded frame from the encoder. Returns the state of the
    encoder. The encoder buffer enc_buf in the encodermust be
    set up with the buffer and buffer_size that will hold the encoded frame
    \param   encoder         Encoder Handle
    \return                  ENC_STATE_INVALID - unrecoverable error
                             ENC_STATE_BUFFER - load data into encoder
                             ENC_STATE_AVAIL - Encoded frame available
*/
extern DllExport dirac_encoder_state_t dirac_encoder_output (dirac_encoder_t *encoder);

/*!
    Retrieve end of sequence information from the encoder. The encoder buffer,
    enc_buf, in the encodermust be set up with the buffer and 
    buffer_size that will hold the end of sequence information.
    \param   encoder         Encoder Handle
    \return                  return status. >=0 - successful; -1 failed
*/
extern DllExport int dirac_encoder_end_sequence (dirac_encoder_t *encoder);

/*!
    Free resources held by encoder
    \param   encoder         Encoder Handle
*/
extern DllExport void dirac_encoder_close (dirac_encoder_t *encoder);

#endif
#ifdef __cplusplus
}
#endif
