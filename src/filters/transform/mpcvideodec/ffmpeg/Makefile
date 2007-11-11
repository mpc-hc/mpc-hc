LAVC_DIR=libavcodec
LAVU_DIR=libavutil
ZLIB_DIR=../../../../zlib
PNG_DIR =../png

CFLAGS+= -I. -I.. -I$(LAVC_DIR) -I$(LAVU_DIR) -I../codecs -I../imgFilters -I../../../../zlib \
         -DHAVE_AV_CONFIG_H

ifeq ($(CC),icl)
CFLAGS+=-Qc99
else
ifneq ($(CC),cl)
CFLAGS+=-std=gnu99
endif
endif

#CFLAGS+=-fverbose-asm -save-temps -fno-stack-check
       
#SRCS_C=DllEntry.c \			=> No entry in MPC!
SRCS_C=$(LAVC_DIR)/8bps.c \
      $(LAVC_DIR)/aasc.c \
      $(LAVC_DIR)/ac3.c \
      $(LAVC_DIR)/ac3enc.c \
      $(LAVC_DIR)/ac3tab.c \
      $(LAVC_DIR)/adpcm.c \
      $(LAVC_DIR)/allcodecs.c \
      $(LAVC_DIR)/amr.c \
      $(LAVC_DIR)/amr_float/interf_dec.c \
      $(LAVC_DIR)/amr_float/sp_dec.c \
      $(LAVC_DIR)/atrac3.c \
      $(LAVC_DIR)/asv1.c \
      $(LAVC_DIR)/avs.c \
      $(LAVC_DIR)/bitstream.c \
      $(LAVC_DIR)/cabac.c \
      $(LAVC_DIR)/cavs.c \
      $(LAVC_DIR)/cavsdec.c \
      $(LAVC_DIR)/cavsdsp.c \
      $(LAVC_DIR)/cinepak.c \
      $(LAVC_DIR)/cook.c \
      $(LAVC_DIR)/corepng.c \
      $(LAVC_DIR)/cscd.c \
      $(LAVC_DIR)/cyuv.c \
      $(LAVC_DIR)/dsputil.c \
      $(LAVC_DIR)/dv.c \
      $(LAVC_DIR)/error_resilience.c \
      $(LAVC_DIR)/eval.c \
      $(LAVC_DIR)/faandct.c \
      $(LAVC_DIR)/fft.c \
      $(LAVC_DIR)/ffv1.c \
      $(LAVC_DIR)/flac.c \
      $(LAVC_DIR)/fraps.c \
      $(LAVC_DIR)/g726.c \
      $(LAVC_DIR)/golomb.c \
      $(LAVC_DIR)/gsm.c \
      $(LAVC_DIR)/h261.c \
      $(LAVC_DIR)/h261dec.c \
      $(LAVC_DIR)/h261enc.c \
      $(LAVC_DIR)/h263.c \
      $(LAVC_DIR)/h263_parser.c \
      $(LAVC_DIR)/h263dec.c \
      $(LAVC_DIR)/h264.c \
      $(LAVC_DIR)/h264_parser.c \
      $(LAVC_DIR)/h264idct.c \
      $(LAVC_DIR)/h264pred.c \
      $(LAVC_DIR)/huffman.c \
      $(LAVC_DIR)/huffyuv.c \
      $(LAVC_DIR)/imc.c \
      $(LAVC_DIR)/imgconvert.c \
      $(LAVC_DIR)/indeo2.c \
      $(LAVC_DIR)/indeo3.c \
      $(LAVC_DIR)/jfdctfst.c \
      $(LAVC_DIR)/jfdctint.c \
      $(LAVC_DIR)/jpegls.c \
      $(LAVC_DIR)/jpeglsdec.c \
      $(LAVC_DIR)/jrevdct.c \
      $(LAVC_DIR)/lcldec.c \
      $(LAVC_DIR)/ljpegenc.c \
      $(LAVC_DIR)/loco.c \
      $(LAVC_DIR)/mace.c \
      $(LAVC_DIR)/mdct.c \
      $(LAVC_DIR)/mjpeg.c \
      $(LAVC_DIR)/mjpegbdec.c \
      $(LAVC_DIR)/mjpegdec.c \
      $(LAVC_DIR)/mjpegenc.c \
      $(LAVC_DIR)/mjpeg_parser.c \
      $(LAVC_DIR)/motion_est.c \
      $(LAVC_DIR)/mpeg12.c \
      $(LAVC_DIR)/mpeg12data.c \
      $(LAVC_DIR)/mpeg12enc.c \
      $(LAVC_DIR)/mpegaudiodata.c \
      $(LAVC_DIR)/mpegaudiodec.c \
      $(LAVC_DIR)/mpegvideo.c \
      $(LAVC_DIR)/mpegvideo_enc.c \
      $(LAVC_DIR)/msmpeg4.c \
      $(LAVC_DIR)/msmpeg4data.c \
      $(LAVC_DIR)/msrle.c \
      $(LAVC_DIR)/msvideo1.c \
      $(LAVC_DIR)/parser.c \
      $(LAVC_DIR)/pcm.c \
      $(LAVC_DIR)/png.c \
      $(LAVC_DIR)/pngdec.c \
      $(LAVC_DIR)/pngenc.c \
      $(LAVC_DIR)/qdm2.c \
      $(LAVC_DIR)/qpeg.c \
      $(LAVC_DIR)/qtrle.c \
      $(LAVC_DIR)/ra144.c \
      $(LAVC_DIR)/ra288.c \
      $(LAVC_DIR)/rangecoder.c \
      $(LAVC_DIR)/ratecontrol.c \
      $(LAVC_DIR)/rpza.c \
      $(LAVC_DIR)/rtjpeg.c \
      $(LAVC_DIR)/rv10.c \
      $(LAVC_DIR)/simple_idct.c \
      $(LAVC_DIR)/skl_dct_c.c \
      $(LAVC_DIR)/snow.c \
      $(LAVC_DIR)/sp5xdec.c \
      $(LAVC_DIR)/svq1.c \
      $(LAVC_DIR)/svq1dec.c \
      $(LAVC_DIR)/truemotion1.c \
      $(LAVC_DIR)/truemotion2.c \
      $(LAVC_DIR)/truespeech.c \
      $(LAVC_DIR)/tscc.c \
      $(LAVC_DIR)/tta.c \
      $(LAVC_DIR)/ulti.c \
      $(LAVC_DIR)/utils.c \
      $(LAVC_DIR)/vc1.c \
      $(LAVC_DIR)/vc1data.c \
      $(LAVC_DIR)/vc1dsp.c \
      $(LAVC_DIR)/vcr1.c \
      $(LAVC_DIR)/vorbis.c \
      $(LAVC_DIR)/vp3.c \
      $(LAVC_DIR)/vp3dsp.c \
      $(LAVC_DIR)/vp5.c \
      $(LAVC_DIR)/vp56.c \
      $(LAVC_DIR)/vp56data.c \
      $(LAVC_DIR)/vp6.c \
      $(LAVC_DIR)/w32thread.c \
      $(LAVC_DIR)/wma.c \
      $(LAVC_DIR)/wmadec.c \
      $(LAVC_DIR)/wmv2.c \
      $(LAVC_DIR)/wnv1.c \
      $(LAVC_DIR)/xl.c \
      $(LAVC_DIR)/zmbv.c \
      $(LAVC_DIR)/xiph.c \
	\
      $(LAVU_DIR)/crc.c \
      $(LAVU_DIR)/log.c \
      $(LAVU_DIR)/lzo.c \
      $(LAVU_DIR)/mathematics.c \
      $(LAVU_DIR)/mem.c \
      $(LAVU_DIR)/random.c \
      $(LAVU_DIR)/rational.c \
#	=== Modify for MPC-HC to create library instead of DLL : ZLib is linked already	  
#	\
#      $(ZLIB_DIR)/adler32.c \
#      $(ZLIB_DIR)/compress.c \
#      $(ZLIB_DIR)/crc32.c \
#      $(ZLIB_DIR)/deflate.c \
#      $(ZLIB_DIR)/infback.c \
#      $(ZLIB_DIR)/inffast.c \
#      $(ZLIB_DIR)/inflate.c \
#      $(ZLIB_DIR)/inftrees.c \
#      $(ZLIB_DIR)/trees.c \
#      $(ZLIB_DIR)/uncompr.c \
#      $(ZLIB_DIR)/zutil.c \

ifneq ($(CC),icl)
ifneq ($(CC),cl)
SRCS_C+=$(LAVC_DIR)/i386/cputest.c \
      $(LAVC_DIR)/i386/cavsdsp_mmx.c \
      $(LAVC_DIR)/i386/dsputil_mmx.c \
      $(LAVC_DIR)/i386/fdct_mmx.c \
      $(LAVC_DIR)/i386/fft_3dn.c \
      $(LAVC_DIR)/i386/fft_3dn2.c \
      $(LAVC_DIR)/i386/fft_sse.c \
      $(LAVC_DIR)/i386/idct_mmx.c \
      $(LAVC_DIR)/i386/idct_mmx_xvid.c \
      $(LAVC_DIR)/i386/motion_est_mmx.c \
      $(LAVC_DIR)/i386/mpegvideo_mmx.c \
      $(LAVC_DIR)/i386/simple_idct_mmx.c \
      $(LAVC_DIR)/i386/snowdsp_mmx.c \
      $(LAVC_DIR)/i386/vp3dsp_mmx.c \
      $(LAVC_DIR)/i386/vp3dsp_sse2.c
SRCS_NASM=$(LAVC_DIR)/i386/skl_dct_sse.asm \
          $(LAVC_DIR)/i386/skl_dct_sse2.asm

NASMFLAGS=-DUSE32 -Dalloc -Dexec
endif
endif

# For 64-bit build. Dunno if it will compile properly.
ifeq ($(64BIT),yes)
	 CFLAGS+=-DARCH_X86_64
	 SLIB=../../../../../lib64/libavcodec_gcc.lib
else
	 SLIB=../../../../../lib/libavcodec_gcc.lib
endif

#ifeq ($(DECODERS_ONLY),yes)
# CFLAGS+=-DDECODERS_ONLY
# SLIB= ../../bin/libavcodec_dec.dll
# DEF=libavcodec_dec.def
#else
# SLIB= ../../bin/libavcodec.dll
# DEF=libavcodec.def
#endif

NOINTRIN=1

#GCCDLLENTRY=--entry=_DllEntryPoint@12
#ifeq ($(CC),icl)
#CLDLLENTRY=/entry:"DllEntryPoint"
#else
#CLDLLENTRY=/entry:"DllEntryPoint@12"
#endif

include makefile_c.inc

.all: $(SLIB)

clean: 
	rm -f *.o *.d *~ $(LAVC_DIR)/*.o $(LAVC_DIR)/*.d $(LAVC_DIR)/i386/*.o $(LAVC_DIR)/amr_float/*.o $(LAVF_DIR)/*.o $(LAVF_DIR)/*.d $(LAVU_DIR)/*.o $(LAVU_DIR)/*.d $(SLIB) $(ZLIB_DIR)/*.o $(PNG_DIR)/*.o $(LAVC_DIR)/i386/*.d $(LAVC_DIR)/amr_float/*.d $(ZLIB_DIR)/*.d

#
# include dependency files if they exist
#
-include $(SRCS:.cpp=.d) $(SRCS_C:.c=.d)
