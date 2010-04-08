#pragma once

#include <windows.h>
#include <winreg.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>

/* code definition */
#define PICTURE_START_CODE			0x100
#define SLICE_START_CODE_MIN		0x101
#define SLICE_START_CODE_MAX		0x1AF
#define USER_DATA_START_CODE		0x1B2
#define SEQUENCE_HEADER_CODE		0x1B3
#define EXTENSION_START_CODE		0x1B5
#define SEQUENCE_END_CODE			0x1B7
#define GROUP_START_CODE			0x1B8

#define SYSTEM_END_CODE				0x1B9
#define PACK_START_CODE				0x1BA
#define SYSTEM_START_CODE			0x1BB
#define PRIVATE_STREAM_1			0x1BD
#define VIDEO_ELEMENTARY_STREAM		0x1E0

/* extension start code IDs */
#define SEQUENCE_EXTENSION_ID					1
#define SEQUENCE_DISPLAY_EXTENSION_ID			2
#define QUANT_MATRIX_EXTENSION_ID				3
#define COPYRIGHT_EXTENSION_ID					4
#define PICTURE_DISPLAY_EXTENSION_ID			7
#define PICTURE_CODING_EXTENSION_ID				8

#define ZIG_ZAG									0
#define MB_WEIGHT								32
#define MB_CLASS4								64

#define I_TYPE			1
#define P_TYPE			2
#define B_TYPE			3

#define TOP_FIELD		1
#define BOTTOM_FIELD	2
#define FRAME_PICTURE	3

#define MACROBLOCK_INTRA				1
#define MACROBLOCK_PATTERN				2
#define MACROBLOCK_MOTION_BACKWARD		4
#define MACROBLOCK_MOTION_FORWARD		8
#define MACROBLOCK_QUANT				16

#define MC_FIELD		1
#define MC_FRAME		2
#define MC_16X8			2
#define MC_DMV			3

#define MV_FIELD		0
#define MV_FRAME		1

#define CHROMA420		1
#define CHROMA422		2
#define CHROMA444		3

#define BUFFER_SIZE			2048
#define MAX_FILE_NUMBER		256

#define IDCT_MMX		1
#define IDCT_SSEMMX		2
#define	IDCT_FPU		3
#define IDCT_REF		4

#define FO_NONE			0
#define FO_FILM			1
#define FO_SWAP			2


typedef void (WINAPI *PBufferOp)(unsigned char*, int, int);

#define MAX_FRAME_NUMBER	1000000
#define MAX_GOP_SIZE		1024


class CMPEG2Dec
{
protected:

    // getbit.cpp
    void Initialize_Buffer();
    void Fill_Buffer();
    void Next_Packet();
    void Flush_Buffer_All(unsigned int N);
    unsigned int Get_Bits_All(unsigned int N);
    void Next_File();

    unsigned int Show_Bits(unsigned int N);
    unsigned int Get_Bits(unsigned int N);
    void Flush_Buffer(unsigned int N);
    void Fill_Next();
    unsigned int Get_Byte();
    unsigned int Get_Short();
    void next_start_code();

    unsigned char Rdbfr[BUFFER_SIZE], *Rdptr, *Rdmax;
    unsigned int CurrentBfr, NextBfr, BitsLeft, Val, Read;

    // gethdr.cpp
    int Get_Hdr();
    void sequence_header();
    int slice_header();
private:
    void group_of_pictures_header();
    void picture_header();
    void sequence_extension();
    void sequence_display_extension();
    void quant_matrix_extension();
    void picture_display_extension();
    void picture_coding_extension();
    void copyright_extension();
    int  extra_bit_information();
    void extension_and_user_data();

protected:
    // getpic.cpp
    void Decode_Picture(int ref, unsigned char *dst, int pitch);
private:
    void Update_Picture_Buffers();
    void picture_data();
    int slice(int MBAmax);
    void macroblock_modes(int *pmacroblock_type, int *pmotion_type,
                          int *pmotion_vector_count, int *pmv_format, int *pdmv, int *pmvscale, int *pdct_type);
    void Clear_Block(int count);
    void Add_Block(int count, int bx, int by, int dct_type, int addflag);
    void motion_compensation(int MBA, int macroblock_type, int motion_type,
                             int PMV[2][2][2], int motion_vertical_field_select[2][2], int dmvector[2], int dct_type);
    void skipped_macroblock(int dc_dct_pred[3], int PMV[2][2][2],
                            int *motion_type, int motion_vertical_field_select[2][2], int *macroblock_type);
    int start_of_slice(int *MBA, int *MBAinc, int dc_dct_pred[3], int PMV[2][2][2]);
    int decode_macroblock(int *macroblock_type, int *motion_type, int *dct_type,
                          int PMV[2][2][2], int dc_dct_pred[3], int motion_vertical_field_select[2][2], int dmvector[2]);
    void Decode_MPEG2_Intra_Block(int comp, int dc_dct_pred[]);
    void Decode_MPEG2_Non_Intra_Block(int comp);

    int Get_macroblock_type();
    int Get_I_macroblock_type();
    int Get_P_macroblock_type();
    int Get_B_macroblock_type();
    int Get_D_macroblock_type();
    int Get_coded_block_pattern();
    int Get_macroblock_address_increment();
    int Get_Luma_DC_dct_diff();
    int Get_Chroma_DC_dct_diff();

    void form_predictions(int bx, int by, int macroblock_type, int motion_type,
                          int PMV[2][2][2], int motion_vertical_field_select[2][2], int dmvector[2]);
    void form_prediction(unsigned char *src[], int sfield, unsigned char *dst[], int dfield,
                         int lx, int lx2, int w, int h, int x, int y, int dx, int dy, int average_flag);
    void form_component_prediction(unsigned char *src, unsigned char *dst,
                                   int lx, int lx2, int w, int h, int x, int y, int dx, int dy, int average_flag);

    // motion.cpp
    void motion_vectors(int PMV[2][2][2], int dmvector[2], int motion_vertical_field_select[2][2],
                        int s, int motion_vector_count, int mv_format,
                        int h_r_size, int v_r_size, int dmv, int mvscale);
    void Dual_Prime_Arithmetic(int DMV[][2], int *dmvector, int mvx, int mvy);
private:
    void motion_vector(int *PMV, int *dmvector, int h_r_size, int v_r_size,
                       int dmv, int mvscale, int full_pel_vector);
    void decode_motion_vector(int *pred, int r_size, int motion_code,
                              int motion_residualesidual, int full_pel_vector);
    int Get_motion_code();
    int Get_dmvector();

protected:
    // store.cpp
    void assembleFrame(unsigned char *src[], int pf, unsigned char *dst, int pitch);
private:
    void Luminance_Filter(unsigned char *src, unsigned char *dst);
    void conv420to422(unsigned char *src, unsigned char *dst, int frame_type);
    void conv422to444(unsigned char *src, unsigned char *dst);
    void conv444toRGB24(unsigned char *py, unsigned char *pu, unsigned char *pv, unsigned char *dst, int pitch);
    void conv422toYUY2(unsigned char *py, unsigned char *pu, unsigned char *pv, unsigned char *dst, int pitch);

protected:
    // decoder operation control flags
    int Fault_Flag;
    int File_Flag;
    int File_Limit;
    int FO_Flag;
    int IDCT_Flag;
    int SystemStream_Flag;

    int Luminance_Flag;
    int Resize_Flag;

    int KeyOp_Flag;
    int lfsr0, lfsr1;
    PBufferOp BufferOp;

    int Infile[MAX_FILE_NUMBER];
    char *Infilename[MAX_FILE_NUMBER];

    int intra_quantizer_matrix[64];
    int non_intra_quantizer_matrix[64];
    int chroma_intra_quantizer_matrix[64];
    int chroma_non_intra_quantizer_matrix[64];

    int load_intra_quantizer_matrix;
    int load_non_intra_quantizer_matrix;
    int load_chroma_intra_quantizer_matrix;
    int load_chroma_non_intra_quantizer_matrix;

    int q_scale_type;
    int alternate_scan;
    int quantizer_scale;

    void *fTempArray, *p_fTempArray;
    short *block[8], *p_block[8];
    int pf_backward, pf_forward, pf_current;

    // global values
    unsigned char *backward_reference_frame[3], *forward_reference_frame[3];
    unsigned char *auxframe[3], *current_frame[3];
    unsigned char *u422, *v422, *u444, *v444, /* *rgb24,*/ *lum;
    unsigned char *dstFrame;	// replaces rgb24
    __int64 RGB_Scale, RGB_Offset, RGB_CRV, RGB_CBU, RGB_CGX, LumOffsetMask, LumGainMask;

    int HALF_WIDTH, PROGRESSIVE_HEIGHT, INTERLACED_HEIGHT, DOUBLE_WIDTH;
    int /*TWIDTH, SWIDTH,*/ HALF_WIDTH_D8, LUM_AREA, CLIP_AREA, HALF_CLIP_AREA, CLIP_STEP;
    int DSTBYTES, DSTBYTES2;	// these replace TWIDTH and SWIDTH
public:
    int Clip_Width, Clip_Height, Resize_Width, Resize_Height;
protected:

    int Coded_Picture_Width, Coded_Picture_Height, Chroma_Width, Chroma_Height;
    int block_count, Second_Field;
    int horizontal_size, vertical_size, mb_width, mb_height;

    /* ISO/IEC 13818-2 section 6.2.2.3:  sequence_extension() */
    int progressive_sequence;
    int chroma_format;

    /* ISO/IEC 13818-2 section 6.2.3: picture_header() */
    int picture_coding_type;
    int temporal_reference;

    /* ISO/IEC 13818-2 section 6.2.3.1: picture_coding_extension() header */
    int f_code[2][2];
    int picture_structure;
    int frame_pred_frame_dct;
    int progressive_frame;
    int concealment_motion_vectors;
    int intra_dc_precision;
    int top_field_first;
    int repeat_first_field;
    int intra_vlc_format;

    // interface
    typedef struct
    {
        DWORD		number;
        int			file;
        __int64		position;
    }	GOPLIST;
    GOPLIST *GOPList[MAX_FRAME_NUMBER];

    typedef struct
    {
        DWORD			top;
        DWORD			bottom;
        char			forward;
        char			backward;
    }	FRAMELIST;
    FRAMELIST *FrameList[MAX_FRAME_NUMBER];

    unsigned char *GOPBuffer[MAX_GOP_SIZE];
public:
    BOOL Field_Order, Full_Frame;
protected:
    HINSTANCE hLibrary;

    void Copyodd(unsigned char *src, unsigned char *dst, int pitch, int forward);
    void Copyeven(unsigned char *src, unsigned char *dst, int pitch, int forward);
public:
    FILE		*VF_File;
    int		VF_FrameRate;
    DWORD		VF_FrameLimit;
    DWORD		VF_FrameBound;
    DWORD		VF_GOPLimit;
    DWORD		VF_GOPNow;
    DWORD		VF_GOPSize;
    int		VF_FrameSize;
    DWORD		VF_OldFrame;
    DWORD		VF_OldRef;

    enum DstFormat
    {
        RGB24, YUY2
    };
    DstFormat m_dstFormat;

    CMPEG2Dec();
    ~CMPEG2Dec()
    {
        Close();
    }
    int Open(LPCTSTR path, DstFormat);
    void Close();
    void Decode(unsigned char *dst, DWORD frame, int pitch);
    bool dstRGB24() const
    {
        return m_dstFormat == RGB24;
    }
    bool dstYUY2() const
    {
        return m_dstFormat == YUY2;
    }
};
