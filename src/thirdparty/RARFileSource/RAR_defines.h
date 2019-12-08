
#ifndef RAR_DEFINES_H
#define RAR_DEFINES_H

/*
  The defines below where taken from the UnRAR source distribution
  which came with the following license:

      The source code of UnRAR utility is freeware. This means:

   1. All copyrights to RAR and the utility UnRAR are exclusively
      owned by the author - Alexander Roshal.

   2. The UnRAR sources may be used in any software to handle RAR
      archives without limitations free of charge, but cannot be used
      to re-create the RAR compression algorithm, which is proprietary.
      Distribution of modified UnRAR sources in separate form or as a
      part of other software is permitted, provided that it is clearly
      stated in the documentation and source comments that the code may
      not be used to develop a RAR (WinRAR) compatible archiver.

   3. The UnRAR utility may be freely distributed. It is allowed
      to distribute UnRAR inside of other software packages.

   4. THE RAR ARCHIVER AND THE UnRAR UTILITY ARE DISTRIBUTED "AS IS".
      NO WARRANTY OF ANY KIND IS EXPRESSED OR IMPLIED.  YOU USE AT 
      YOUR OWN RISK. THE AUTHOR WILL NOT BE LIABLE FOR DATA LOSS, 
      DAMAGES, LOSS OF PROFITS OR ANY OTHER KIND OF LOSS WHILE USING
      OR MISUSING THIS SOFTWARE.

   5. Installing and using the UnRAR utility signifies acceptance of
      these terms and conditions of the license.

   6. If you don't agree with terms of the license you must remove
      UnRAR files from your storage devices and cease to use the
      utility.

      Thank you for your interest in RAR and UnRAR.


                                            Alexander L. Roshal
*/

#define  MHD_VOLUME         0x0001
#define  MHD_COMMENT        0x0002
#define  MHD_LOCK           0x0004
#define  MHD_SOLID          0x0008
#define  MHD_PACK_COMMENT   0x0010
#define  MHD_NEWNUMBERING   0x0010
#define  MHD_AV             0x0020
#define  MHD_PROTECT        0x0040
#define  MHD_PASSWORD       0x0080
#define  MHD_FIRSTVOLUME    0x0100
#define  MHD_ENCRYPTVER     0x0200

#define  LHD_SPLIT_BEFORE   0x0001
#define  LHD_SPLIT_AFTER    0x0002
#define  LHD_PASSWORD       0x0004
#define  LHD_COMMENT        0x0008
#define  LHD_SOLID          0x0010

#define  LHD_WINDOWMASK     0x00e0
#define  LHD_WINDOW64       0x0000
#define  LHD_WINDOW128      0x0020
#define  LHD_WINDOW256      0x0040
#define  LHD_WINDOW512      0x0060
#define  LHD_WINDOW1024     0x0080
#define  LHD_WINDOW2048     0x00a0
#define  LHD_WINDOW4096     0x00c0
#define  LHD_DIRECTORY      0x00e0

#define  LHD_LARGE          0x0100
#define  LHD_UNICODE        0x0200
#define  LHD_SALT           0x0400
#define  LHD_VERSION        0x0800
#define  LHD_EXTTIME        0x1000
#define  LHD_EXTFLAGS       0x2000

#define  SKIP_IF_UNKNOWN    0x4000
#define  LONG_BLOCK         0x8000

#define  EARC_NEXT_VOLUME   0x0001
#define  EARC_DATACRC       0x0002
#define  EARC_REVSPACE      0x0004
#define  EARC_VOLNUMBER     0x0008

#endif RAR_DEFINES_H
