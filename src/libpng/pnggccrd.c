
/* pnggccrd.c - mixed C/assembler version of utilities to read a PNG file
 *
 * For Intel/AMD x86 or x86-64 CPU (Pentium-MMX or later) and GNU C compiler.
 *
 * Last changed in libpng 1.2.19 August 18, 2007
 * For conditions of distribution and use, see copyright notice in png.h
 * Copyright (c) 1998 Intel Corporation
 * Copyright (c) 1999-2002,2007 Greg Roelofs
 * Copyright (c) 1998-2007 Glenn Randers-Pehrson
 *
 * Based on MSVC code contributed by Nirav Chhatrapati, Intel Corp., 1998.
 * Interface to libpng contributed by Gilles Vollant, 1999.
 * GNU C port by Greg Roelofs, 1999-2001.
 *
 * References:
 *
 *     http://www.intel.com/drg/pentiumII/appnotes/916/916.htm
 *     http://www.intel.com/drg/pentiumII/appnotes/923/923.htm
 *       [Intel's performance analysis of the MMX vs. non-MMX code;
 *        moved/deleted as of 2006, but text and some graphs still
 *        available via WayBack Machine at archive.org]
 *
 *     http://www.ibiblio.org/gferg/ldp/GCC-Inline-Assembly-HOWTO.html
 *     http://sam.zoy.org/blog/2007-04-13-shlib-with-non-pic-code-have-inline-assembly-and-pic-mix-well
 *     http://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html
 *     http://gcc.gnu.org/onlinedocs/gcc/Variable-Attributes.html
 *     http://gcc.gnu.org/onlinedocs/gcc/Function-Attributes.html
 *     AMD64 Architecture Programmer's Manual, volumes 1 and 5
 *       [http://www.amd.com/us-en/Processors/TechnicalResources/0,,30_182_739_7044,00.html]
 *     Intel 64 and IA-32 Software Developer's Manuals
 *       [http://developer.intel.com/products/processor/manuals/]
 *
 * png_read_filter_row_mmx_*() were converted in place with intel2gas 1.3.1:
 *
 *     intel2gas -mdI pnggccrd.c.partially-msvc -o pnggccrd.c
 *
 * and then cleaned up by hand.  See http://hermes.terminal.at/intel2gas/ .
 *
 * NOTE:  A sufficiently recent version of GNU as (or as.exe under DOS/Windows)
 * is required to assemble the newer asm instructions such as movq.  (Version
 * 2.5.2l.15 is definitely too old.)  See ftp://ftp.gnu.org/pub/gnu/binutils/ .
 */

/*
 * PORTING NOTES AND CHANGELOG (mostly by Greg Roelofs)
 * ===========================
 *
 * 19991006:
 *  - fixed sign error in post-MMX cleanup code (16- & 32-bit cases)
 *
 * 19991007:
 *  - additional optimizations (possible or definite):
 *     x [DONE] write MMX code for 64-bit case (pixel_bytes == 8) [not tested]
 *     - write MMX code for 48-bit case (pixel_bytes == 6)
 *     - figure out what's up with 24-bit case (pixel_bytes == 3):
 *        why subtract 8 from width_mmx in the pass 4/5 case?
 *        (only width_mmx case) (near line 2335)
 *     x [DONE] replace pixel_bytes within each block with the true
 *        constant value (or are compilers smart enough to do that?)
 *     - rewrite all MMX interlacing code so it's aligned with
 *        the *beginning* of the row buffer, not the end.  This
 *        would not only allow one to eliminate half of the memory
 *        writes for odd passes (that is, pass == odd), it may also
 *        eliminate some unaligned-data-access exceptions (assuming
 *        there's a penalty for not aligning 64-bit accesses on
 *        64-bit boundaries).  The only catch is that the "leftover"
 *        pixel(s) at the end of the row would have to be saved,
 *        but there are enough unused MMX registers in every case,
 *        so this is not a problem.  A further benefit is that the
 *        post-MMX cleanup code (C code) in at least some of the
 *        cases could be done within the assembler block.
 *  x [DONE] the "v3 v2 v1 v0 v7 v6 v5 v4" comments are confusing,
 *     inconsistent, and don't match the MMX Programmer's Reference
 *     Manual conventions anyway.  They should be changed to
 *     "b7 b6 b5 b4 b3 b2 b1 b0," where b0 indicates the byte that
 *     was lowest in memory (i.e., corresponding to a left pixel)
 *     and b7 is the byte that was highest (i.e., a right pixel).
 *
 * 19991016:
 *  - Brennan's Guide notwithstanding, gcc under Linux does *not*
 *     want globals prefixed by underscores when referencing them--
 *     i.e., if the variable is const4, then refer to it as const4,
 *     not _const4.  This seems to be a djgpp-specific requirement.
 *     Also, such variables apparently *must* be declared outside
 *     of functions; neither static nor automatic variables work if
 *     defined within the scope of a single function, but both
 *     static and truly global (multi-module) variables work fine.
 *
 * 19991017:
 *  - replaced pixel_bytes in each png_memcpy() call with constant value for
 *     inlining (png_do_read_interlace() "non-MMX/modified C code" block)
 *
 * 19991023:
 *  - fixed png_combine_row() non-MMX replication bug (odd passes only?)
 *  - switched from string-concatenation-with-macros to cleaner method of
 *     renaming global variables for djgpp--i.e., always use prefixes in
 *     inlined assembler code (== strings) and conditionally rename the
 *     variables, not the other way around.  Hence _const4, _mask8_0, etc.
 *
 * 19991024:
 *  - fixed mmxsupport()/png_do_read_interlace() first-row bug
 *     This one was severely weird:  even though mmxsupport() doesn't touch
 *     ebx (where "row" pointer was stored), it nevertheless managed to zero
 *     the register (even in static/non-fPIC code--see below), which in turn
 *     caused png_do_read_interlace() to return prematurely on the first row of
 *     interlaced images (i.e., without expanding the interlaced pixels).
 *     Inspection of the generated assembly code didn't turn up any clues,
 *     although it did point at a minor optimization (i.e., get rid of
 *     mmx_supported_local variable and just use eax).  Possibly the CPUID
 *     instruction is more destructive than it looks?  (Not yet checked.)
 *  - "info gcc" was next to useless, so compared fPIC and non-fPIC assembly
 *     listings...  Apparently register spillage has to do with ebx, since
 *     it's used to index the global offset table.  Commenting it out of the
 *     input-reg lists in png_combine_row() eliminated compiler barfage, so
 *     ifdef'd with __PIC__ macro:  if defined, use a global for unmask
 *
 * 19991107:
 *  - verified CPUID clobberage:  12-char string constant ("GenuineIntel",
 *     "AuthenticAMD", etc.) placed in ebx:ecx:edx.  Still need to polish.
 *
 * 19991120:
 *  - made "diff" variable (now "_dif") global to simplify conversion of
 *     filtering routines (running out of regs, sigh).  "diff" is still used
 *     in interlacing routines, however.
 *  - fixed up both versions of mmxsupport() (ORIG_THAT_USED_TO_CLOBBER_EBX
 *     macro determines which is used); original not yet tested.
 *
 * 20000213:
 *  - when compiling with gcc, be sure to use  -fomit-frame-pointer
 *
 * 20000319:
 *  - fixed a register-name typo in png_do_read_interlace(), default (MMX) case,
 *     pass == 4 or 5, that caused visible corruption of interlaced images
 *
 * 20000623:
 *  - Various problems were reported with gcc 2.95.2 in the Cygwin environment,
 *     many of the form "forbidden register 0 (ax) was spilled for class AREG."
 *     This is explained at http://gcc.gnu.org/fom_serv/cache/23.html, and
 *     Chuck Wilson supplied a patch involving dummy output registers.  See
 *     http://sourceforge.net/bugs/?func=detailbug&bug_id=108741&group_id=5624
 *     for the original (anonymous) SourceForge bug report.
 *
 * 20000706:
 *  - Chuck Wilson passed along these remaining gcc 2.95.2 errors:
 *       pnggccrd.c: In function `png_combine_row':
 *       pnggccrd.c:525: more than 10 operands in `asm'
 *       pnggccrd.c:669: more than 10 operands in `asm'
 *       pnggccrd.c:828: more than 10 operands in `asm'
 *       pnggccrd.c:994: more than 10 operands in `asm'
 *       pnggccrd.c:1177: more than 10 operands in `asm'
 *     They are all the same problem and can be worked around by using the
 *     global _unmask variable unconditionally, not just in the -fPIC case.
 *     Reportedly earlier versions of gcc also have the problem with more than
 *     10 operands; they just don't report it.  Much strangeness ensues, etc.
 *
 * 20000729:
 *  - enabled png_read_filter_row_mmx_up() (shortest remaining unconverted
 *     MMX routine); began converting png_read_filter_row_mmx_sub()
 *  - to finish remaining sections:
 *     - clean up indentation and comments
 *     - preload local variables
 *     - add output and input regs (order of former determines numerical
 *        mapping of latter)
 *     - avoid all usage of ebx (including bx, bh, bl) register [20000823]
 *     - remove "$" from addressing of Shift and Mask variables [20000823]
 *
 * 20000731:
 *  - global union vars causing segfaults in png_read_filter_row_mmx_sub()?
 *
 * 20000822:
 *  - ARGH, stupid png_read_filter_row_mmx_sub() segfault only happens with
 *     shared-library (-fPIC) version!  Code works just fine as part of static
 *     library.  Should have tested that sooner.
 *     ebx is getting clobbered again (explicitly this time); need to save it
 *     on stack or rewrite asm code to avoid using it altogether.  Blargh!
 *
 * 20000823:
 *  - first section was trickiest; all remaining sections have ebx -> edx now.
 *     (-fPIC works again.)  Also added missing underscores to various Shift*
 *     and *Mask* globals and got rid of leading "$" signs.
 *
 * 20000826:
 *  - added visual separators to help navigate microscopic printed copies
 *     (http://pobox.com/~newt/code/gpr-latest.zip, mode 10); started working
 *     on png_read_filter_row_mmx_avg()
 *
 * 20000828:
 *  - finished png_read_filter_row_mmx_avg():  only Paeth left! (930 lines...)
 *     What the hell, did png_read_filter_row_mmx_paeth(), too.  Comments not
 *     cleaned up/shortened in either routine, but functionality is complete
 *     and seems to be working fine.
 *
 * 20000829:
 *  - ahhh, figured out last(?) bit of gcc/gas asm-fu:  if register is listed
 *     as an input reg (with dummy output variables, etc.), then it *cannot*
 *     also appear in the clobber list or gcc 2.95.2 will barf.  The solution
 *     is simple enough...
 *
 * 20000914:
 *  - bug in png_read_filter_row_mmx_avg():  16-bit grayscale not handled
 *     correctly (but 48-bit RGB just fine)
 *
 * 20000916:
 *  - fixed bug in png_read_filter_row_mmx_avg(), bpp == 2 case; three errors:
 *     - "_ShiftBpp.use = 24;"      should have been   "_ShiftBpp.use = 16;"
 *     - "_ShiftRem.use = 40;"      should have been   "_ShiftRem.use = 48;"
 *     - "psllq _ShiftRem, %%mm2"   should have been   "psrlq _ShiftRem, %%mm2"
 *
 * 20010101:
 *  - added new png_init_mmx_flags() function (here only because it needs to
 *     call mmxsupport(), which should probably become global png_mmxsupport());
 *     modified other MMX routines to run conditionally (png_ptr->asm_flags)
 *
 * 20010103:
 *  - renamed mmxsupport() to png_mmx_support(), with auto-set of mmx_supported,
 *     and made it public; moved png_init_mmx_flags() to png.c as internal func
 *
 * 20010104:
 *  - removed dependency on png_read_filter_row_c() (C code already duplicated
 *     within MMX version of png_read_filter_row()) so no longer necessary to
 *     compile it into pngrutil.o
 *
 * 20010310:
 *  - fixed buffer-overrun bug in png_combine_row() C code (non-MMX)
 *
 * 20010808:
 *  - added PNG_THREAD_UNSAFE_OK around code using global variables [GR-P]
 *
 * 20011124:
 *  - fixed missing save of Eflag in png_mmx_support() [Maxim Sobolev]
 *
 * 20020304:
 *  - eliminated incorrect use of width_mmx in pixel_bytes == 8 case
 *
 * 20020407:
 *  - fixed insufficient preservation of ebx register [Sami Farin]
 *
 * 20040724:
 *  - more tinkering with clobber list at lines 4529 and 5033 to get it to
 *     compile with gcc 3.4 [GR-P]
 *
 * 20040809:
 *  - added "rim" definitions for CONST4 and CONST6 [GR-P]
 *
 * 20060303:
 *  - added "OS2" to list of systems that don't need leading underscores [GR-P]
 *
 * 20060320:
 *  - made PIC-compliant [Christian Aichinger]
 *
 * 20070313:
 *  - finally applied Giuseppe Ghibò's 64-bit patch of 20060803 (completely
 *     overlooked Dylan Alex Simon's similar patch of 20060414, oops...)
 *
 * 20070524:
 *  - fixed link failure caused by asm-only variables being optimized out
 *     (identified by Dimitri of Trolltech) with __attribute__((used)), which
 *     also gets rid of warnings => nuked ugly png_squelch_warnings() hack
 *  - dropped redundant ifdef
 *  - moved png_mmx_support() back up where originally intended (as in
 *     pngvcrd.c), using __attribute__((noinline)) in extra prototype
 *
 * 20070527:
 *  - revised png_combine_row() to reuse mask in lieu of external _unmask
 *  - moved 32-bit (RGBA) case to top of png_combine_row():  most common
 *  - just about ready to give up on x86-64 -fPIC mode; can't even access 16
 *     _mask*_* constants without triggering link error on shared library:
 *       /usr/bin/ld: pnggccrd.pic.o: relocation R_X86_64_32S against `a local
 *         symbol' can not be used when making a shared object; recompile with
 *         -fPIC
 *       pnggccrd.pic.o: could not read symbols: Bad value
 *       ("objdump -x pnggccrd.pic.o | grep rodata" to verify)
 *     [might be able to work around by doing within assembly code whatever
 *     -fPIC does, but given problems to date, seems like long shot...]
 *     [relevant ifdefs:  __x86_64__ && __PIC__ => C code only]
 *  - changed #if 0 to #ifdef PNG_CLOBBER_MMX_REGS_SUPPORTED in case gcc ever
 *     supports MMX regs (%mm0, etc.) in clobber list (not supported by gcc
 *     2.7.2.3, 2.91.66 (egcs 1.1.2), 3.x, or 4.1.2)
 *
 * 20070603:
 *  - revised png_combine_row() to use @GOTPCREL(%%rip) addressing on _c64
 *     struct of _mask*_* constants for x86-64 -fPIC; see sam.zoy.org link
 *     above for details
 *  - moved _const4 and _const6 into _c64 struct, renamed to _amask5_3_0 and
 *     _amask7_1_0, respectively
 *  - can't figure out how to use _c64._mask*_* vars within asm code, so still
 *     need single variables for non-x86-64/-fPIC half :-(
 *  - replaced various __PIC__ ifdefs with *_GOT_ebx macros
 *  - moved _LBCarryMask and _HBClearMask into _c64 struct
 *  - conditionally replaced _p*temp variables with %r11d-%r13d (via p*_TEMP
 *     and CLOBBER_r1*d macros)
 *
 * 20070604:
 *  - replaced all _ActiveMask and _ActiveMaskEnd with new _amask*_*_* consts
 *     (_amask naming convention:  numbers of 00-bytes, ff-bytes, 00-bytes)
 *    - _ActiveMask     // (10) // avg/paeth/sub; read-only; consts; movq/pand
 *       0x0000000000ffffffLL (bpp 3, avg)      _amask5_3_0
 *       0xffffffffffffffffLL (bpp 4, 6, avg)   _amask0_8_0
 *       0x000000000000ffffLL (bpp 2, avg)      _amask6_2_0
 *       0x0000000000ffffffLL (bpp 3, paeth)    _amask5_3_0
 *       0x00000000ffffffffLL (bpp 6, paeth)    _amask4_4_0
 *       0x00000000ffffffffLL (bpp 4, paeth)    _amask4_4_0
 *       0x00000000ffffffffLL (bpp 8, paeth)    _amask4_4_0
 *       0x0000ffffff000000LL (bpp 3, sub)      _amask2_3_3
 *       0x00000000ffff0000LL (bpp 2, sub)      _amask4_2_2
 *    - _ActiveMaskEnd  // (1)  // paeth only; read-only; const; pand
 *       0xffff000000000000LL (bpp 3, paeth)    _amask0_2_6
 *  - changed all "#if defined(__x86_64__) // later // && defined(__PIC__)"
 *     lines to "#ifdef PNG_x86_64_USE_GOTPCREL" for easier/safer testing
 *
 * 20070605:
 *  - merged PNG_x86_64_USE_GOTPCREL, non-PNG_x86_64_USE_GOTPCREL code via
 *     *MASK* and LOAD/RESTORE macros
 *
 * 20070607:
 *  - replaced all constant instances of _ShiftBpp, _ShiftRem with immediates
 *     (still have two shared cases in avg, sub routines)
 *
 * 20070609:
 *  - replaced remaining instances of _ShiftBpp, _ShiftRem with immediates
 *     (split sub and avg 4/6-bpp cases into separate blocks)
 *  - fixed paeth bug due to clobbered r11/r12/r13 regs
 *
 * 20070610:
 *  - made global "_dif" variable (avg/paeth/sub routines) local again (now
 *     "diff"--see 19991120 entry above), using register constraints
 *  - note that %ebp in clobber list doesn't actually work, at least for 32-bit
 *     version and gcc 4.1.2; must save and restore manually.  (Seems to work
 *     OK for 64-bit version and gcc 3.4.3, but gcc may not be using ebp/rbp
 *     in that case.)
 *  - started replacing direct _MMXLength accesses with register constraints
 *
 * 20070612:
 *  - continued replacing direct _MMXLength accesses with register constraints
 *
 * 20070613:
 *  - finished replacing direct _MMXLength accesses with register constraints;
 *     switched to local variable (and renamed back to MMXLength)
 *
 * 20070614:
 *  - fixed sub bpp = 1 bug
 *  - started replacing direct _FullLength accesses with register constraints
 *
 * 20070615:
 *  - fixed 64-bit paeth bpp 3 crash bug (misplaced LOAD_GOT_rbp)
 *  - fixed 64-bit paeth bpp 1/2 and cleanup-block crash bugs (misplaced
 *     RESTORE_r11_r12_r13)
 *  - slightly optimized avg/paeth cleanup blocks and paeth bpp 1/2 block
 *     (save/restore ebx only if needed)
 *  - continued replacing direct _FullLength accesses with register constraints
 *
 * 20070616:
 *  - finished replacing direct _FullLength accesses with register constraints
 *     (*ugly* conditional clobber-separator macros for avg and paeth, sigh)
 *
 * 20070618:
 *  - fixed misplaced PNG_THREAD_UNSAFE_OK endif (was missing LOAD_GOT_rbp/
 *     RESTORE_rbp in 32-bit thread-safe case)
 *  - changed all "ifdef *" to "if defined(*)" [GR-P]
 *
 * 20070619:
 *  - rearranged most bitdepth-related case statements to put most frequent
 *     cases at top (24-bit, 32-bit, 8-bit, rest)
 *
 * 20070623:
 *  - cleaned up png_debug() warnings/formatting
 *  - removed PNG_MMX_CODE_SUPPORTED ifdefs and added outer __GNUC__ ifdef
 *     (module no longer used by non-x86/non-GCC builds as of libpng 1.2.19)
 *  - removed single libpng-1.2.x PNG_DEBUG dependency on 1.0.x png_struct
 *     member (row_buf_size)
 *  - rearranged pass-related if-blocks in png_do_read_interlace() to put most
 *     frequent cases (4, 5) at top [GR-P suggestion]
 *
 * 20070624-29:
 *  - fixed 64-bit crash bug:  pointers -> rsi/rdi, not esi/edi (switched to
 *     %0/%1/%2/%3/%4 notation; eliminated size suffixes from relevant add/
 *     inc/sub/mov instructions; changed dummy vars to pointers)
 *     - png_combine_row()
 *     - png_do_read_interlace()
 *     - png_read_filter_row_mmx_avg()
 *     - png_read_filter_row_mmx_paeth()
 *     - png_read_filter_row_mmx_sub()
 *     - png_read_filter_row_mmx_up()
 *  - NOTE:  this fix makes use of the fact that modifying a 32-bit reg (e.g.,
 *     %%ebx) clears the top half of its corresponding 64-bit reg (%%rbx), so
 *     it's safe to mix 32-bit operations with 64-bit base/index addressing
 *     (see new PSI/PAX/PBX/PDX/PBP/etc. "pointer-register" macros); applies
 *     also to clobber lists
 *
 * 20070630:
 *  - cleaned up formatting, macros, minor png_read_filter_row_mmx_sub() 8-bpp
 *     register-usage inefficiency
 *  - fixed 32-bit png_do_read_interlace() bug (was using pointer size for
 *     64-bit dummy values)
 *
 * 20070703:
 *  - added check for (manual) PIC macro to fix OpenBSD crash bug
 *
 * 20070717:
 *  - fixed 48-bit png_combine_row() bug (was acting like 32-bit):  copy 6
 *     bytes per pixel, not 4, and use stride of 6, not 4, in the second loop
 *     of interlace processing of 48-bit pixels [GR-P]
 *
 * 20070722:
 *  - fixed 64-bit png_uint_32 bug with MMXLength/FullLength temp vars
 *
 * [still broken:  tops of all row-filter blocks (input/output constraints);
 *  shows up on 64-bit dynamic (-fPIC) version with -O2, especially if debug-
 *  printfs enabled, but at right edge of odd-width images even if disabled]
 *
 *
 * STILL TO DO:
 *  - fix final thread-unsafe code using stack vars and pointer? (paeth top,
 *     default, bottom only:  default, bottom already 5 reg constraints; could
 *     replace bpp with pointer and group bpp/patemp/pbtemp/pctemp in array)
 *  - fix ebp/no-reg-constraint inefficiency (avg/paeth/sub top)
 *  - test png_do_read_interlace() 64-bit case (pixel_bytes == 8)
 *  - write MMX code for 48-bit case (pixel_bytes == 6)
 *  - figure out what's up with 24-bit case (pixel_bytes == 3):
 *     why subtract 8 from width_mmx in the pass 4/5 case?  due to
 *     odd number of bytes? (only width_mmx case) (near line 2335)
 *  - rewrite all MMX interlacing code so it's aligned with beginning
 *     of the row buffer, not the end (see 19991007 for details)
 *  - add error messages to any remaining bogus default cases
 *  - enable pixel_depth == 8 cases in png_read_filter_row()? (test speed)
 *  - try =r, etc., as reg constraints?  (would gcc use 64-bit ones on x86-64?)
 *  - need full, non-graphical, CRC-based test suite...  maybe autogenerate
 *     random data of various height/width/depth, compute CRCs, write (C
 *     funcs), read (asm/MMX), recompute CRCs, and compare?
 *  - write true x86-64 version using 128-bit "media instructions", %xmm0-15,
 *     and extra general-purpose registers
 */

#if defined(__GNUC__)

#define PNG_INTERNAL
#include "png.h"


/* for some inexplicable reason, gcc 3.3.5 on OpenBSD (and elsewhere?) does
 * *not* define __PIC__ when the -fPIC option is used, so we have to rely on
 * makefiles and whatnot to define the PIC macro explicitly */
#if defined(PIC) && !defined(__PIC__)   // (this can/should move to pngconf.h)
#  define __PIC__
#endif

#if defined(PNG_ASSEMBLER_CODE_SUPPORTED) && defined(PNG_USE_PNGGCCRD)

/* if you want/need full thread-safety on x86-64 even when linking statically,
 * comment out the "&& defined(__PIC__)" part here: */
#if defined(__x86_64__) && defined(__PIC__)
#  define PNG_x86_64_USE_GOTPCREL            // GOTPCREL => full thread-safety
#  define PNG_CLOBBER_x86_64_REGS_SUPPORTED  // works as of gcc 3.4.3 ...
#endif

int PNGAPI png_mmx_support(void);

#if defined(PNG_USE_LOCAL_ARRAYS)
static PNG_CONST int FARDATA png_pass_start[7] = {0, 4, 0, 2, 0, 1, 0};
static PNG_CONST int FARDATA png_pass_inc[7]   = {8, 8, 4, 4, 2, 2, 1};
static PNG_CONST int FARDATA png_pass_width[7] = {8, 4, 4, 2, 2, 1, 1};
#endif

/* djgpp, Win32, Cygwin, and OS2 add their own underscores to global variables,
 * so define them without: */
#if defined(__DJGPP__) || defined(WIN32) || defined(__CYGWIN__) || \
    defined(__OS2__)
#  define _mmx_supported  mmx_supported
#  define _mask8_0        mask8_0
#  define _mask16_1       mask16_1
#  define _mask16_0       mask16_0
#  define _mask24_2       mask24_2
#  define _mask24_1       mask24_1
#  define _mask24_0       mask24_0
#  define _mask32_3       mask32_3
#  define _mask32_2       mask32_2
#  define _mask32_1       mask32_1
#  define _mask32_0       mask32_0
#  define _mask48_5       mask48_5
#  define _mask48_4       mask48_4
#  define _mask48_3       mask48_3
#  define _mask48_2       mask48_2
#  define _mask48_1       mask48_1
#  define _mask48_0       mask48_0
#  define _amask5_3_0     amask5_3_0
#  define _amask7_1_0     amask7_1_0
#  define _LBCarryMask    LBCarryMask
#  define _HBClearMask    HBClearMask
#  define _amask0_8_0     amask0_8_0
#  define _amask6_2_0     amask6_2_0
#  define _amask4_4_0     amask4_4_0
#  define _amask0_2_6     amask0_2_6
#  define _amask2_3_3     amask2_3_3
#  define _amask4_2_2     amask4_2_2
#  if defined(PNG_THREAD_UNSAFE_OK)
#    define _patemp       patemp
#    define _pbtemp       pbtemp
#    define _pctemp       pctemp
#  endif
#endif // djgpp, Win32, Cygwin, OS2


/* These constants are used in the inlined MMX assembly code. */

typedef unsigned long long  ull;

#if defined(PNG_x86_64_USE_GOTPCREL)
static PNG_CONST struct {
    //ull _mask_array[26];

    // png_combine_row() constants:
    ull _mask8_0;
    ull _mask16_0, _mask16_1;
    ull _mask24_0, _mask24_1, _mask24_2;
    ull _mask32_0, _mask32_1, _mask32_2, _mask32_3;
    ull _mask48_0, _mask48_1, _mask48_2, _mask48_3, _mask48_4, _mask48_5;

    // png_do_read_interlace() constants:
    ull _amask5_3_0, _amask7_1_0;  // was _const4 and _const6, respectively

    // png_read_filter_row_mmx_avg() constants (also uses _amask5_3_0):
    ull _LBCarryMask, _HBClearMask;
    ull _amask0_8_0, _amask6_2_0;  // was ActiveMask for bpp 4/6 and 2 cases

    // png_read_filter_row_mmx_paeth() constants (also uses _amask5_3_0):
    ull _amask4_4_0, _amask0_2_6;  // was ActiveMask{,End} for bpp 6/4/8 and 3

    // png_read_filter_row_mmx_sub() constants:
    ull _amask2_3_3, _amask4_2_2;  // was ActiveMask for bpp 3 and 2 cases

} _c64 __attribute__((used, aligned(8))) = {

    // png_combine_row() constants:
    0x0102040810204080LL, // _mask8_0      offset 0

    0x1010202040408080LL, // _mask16_0     offset 8
    0x0101020204040808LL, // _mask16_1     offset 16

    0x2020404040808080LL, // _mask24_0     offset 24
    0x0408080810101020LL, // _mask24_1     offset 32
    0x0101010202020404LL, // _mask24_2     offset 40

    0x4040404080808080LL, // _mask32_0     offset 48
    0x1010101020202020LL, // _mask32_1     offset 56
    0x0404040408080808LL, // _mask32_2     offset 64
    0x0101010102020202LL, // _mask32_3     offset 72

    0x4040808080808080LL, // _mask48_0     offset 80
    0x2020202040404040LL, // _mask48_1     offset 88
    0x1010101010102020LL, // _mask48_2     offset 96
    0x0404080808080808LL, // _mask48_3     offset 104
    0x0202020204040404LL, // _mask48_4     offset 112
    0x0101010101010202LL, // _mask48_5     offset 120

    // png_do_read_interlace() constants:
    0x0000000000FFFFFFLL, // _amask5_3_0   offset 128  (bpp 3, avg/paeth) const4
    0x00000000000000FFLL, // _amask7_1_0   offset 136                     const6

    // png_read_filter_row_mmx_avg() constants:
    0x0101010101010101LL, // _LBCarryMask  offset 144
    0x7F7F7F7F7F7F7F7FLL, // _HBClearMask  offset 152
    0xFFFFFFFFFFFFFFFFLL, // _amask0_8_0   offset 160  (bpp 4/6, avg)
    0x000000000000FFFFLL, // _amask6_2_0   offset 168  (bpp 2,   avg)

    // png_read_filter_row_mmx_paeth() constants:
    0x00000000FFFFFFFFLL, // _amask4_4_0   offset 176  (bpp 6/4/8, paeth)
    0xFFFF000000000000LL, // _amask0_2_6   offset 184  (bpp 3, paeth)   A.M.End

    // png_read_filter_row_mmx_sub() constants:
    0x0000FFFFFF000000LL, // _amask2_3_3   offset 192  (bpp 3, sub)
    0x00000000FFFF0000LL, // _amask4_2_2   offset 200  (bpp 2, sub)

};

#define MASK8_0        "(%%rbp)"
#define MASK16_0       "8(%%rbp)"
#define MASK16_1       "16(%%rbp)"
#define MASK24_0       "24(%%rbp)"
#define MASK24_1       "32(%%rbp)"
#define MASK24_2       "40(%%rbp)"
#define MASK32_0       "48(%%rbp)"
#define MASK32_1       "56(%%rbp)"
#define MASK32_2       "64(%%rbp)"
#define MASK32_3       "72(%%rbp)"
#define MASK48_0       "80(%%rbp)"
#define MASK48_1       "88(%%rbp)"
#define MASK48_2       "96(%%rbp)"
#define MASK48_3       "104(%%rbp)"
#define MASK48_4       "112(%%rbp)"
#define MASK48_5       "120(%%rbp)"
#define AMASK5_3_0     "128(%%rbp)"
#define AMASK7_1_0     "136(%%rbp)"
#define LB_CARRY_MASK  "144(%%rbp)"
#define HB_CLEAR_MASK  "152(%%rbp)"
#define AMASK0_8_0     "160(%%rbp)"
#define AMASK6_2_0     "168(%%rbp)"
#define AMASK4_4_0     "176(%%rbp)"
#define AMASK0_2_6     "184(%%rbp)"
#define AMASK2_3_3     "192(%%rbp)"
#define AMASK4_2_2     "200(%%rbp)"

#else // !PNG_x86_64_USE_GOTPCREL

static PNG_CONST ull _mask8_0  __attribute__((used, aligned(8))) = 0x0102040810204080LL;

static PNG_CONST ull _mask16_1 __attribute__((used, aligned(8))) = 0x0101020204040808LL;
static PNG_CONST ull _mask16_0 __attribute__((used, aligned(8))) = 0x1010202040408080LL;

static PNG_CONST ull _mask24_2 __attribute__((used, aligned(8))) = 0x0101010202020404LL;
static PNG_CONST ull _mask24_1 __attribute__((used, aligned(8))) = 0x0408080810101020LL;
static PNG_CONST ull _mask24_0 __attribute__((used, aligned(8))) = 0x2020404040808080LL;

static PNG_CONST ull _mask32_3 __attribute__((used, aligned(8))) = 0x0101010102020202LL;
static PNG_CONST ull _mask32_2 __attribute__((used, aligned(8))) = 0x0404040408080808LL;
static PNG_CONST ull _mask32_1 __attribute__((used, aligned(8))) = 0x1010101020202020LL;
static PNG_CONST ull _mask32_0 __attribute__((used, aligned(8))) = 0x4040404080808080LL;

static PNG_CONST ull _mask48_5 __attribute__((used, aligned(8))) = 0x0101010101010202LL;
static PNG_CONST ull _mask48_4 __attribute__((used, aligned(8))) = 0x0202020204040404LL;
static PNG_CONST ull _mask48_3 __attribute__((used, aligned(8))) = 0x0404080808080808LL;
static PNG_CONST ull _mask48_2 __attribute__((used, aligned(8))) = 0x1010101010102020LL;
static PNG_CONST ull _mask48_1 __attribute__((used, aligned(8))) = 0x2020202040404040LL;
static PNG_CONST ull _mask48_0 __attribute__((used, aligned(8))) = 0x4040808080808080LL;

// png_do_read_interlace() constants:
static PNG_CONST ull _amask5_3_0  __attribute__((aligned(8))) = 0x0000000000FFFFFFLL;  // was _const4
static PNG_CONST ull _amask7_1_0  __attribute__((aligned(8))) = 0x00000000000000FFLL;  // was _const6

// png_read_filter_row_mmx_avg() constants:
static PNG_CONST ull _LBCarryMask __attribute__((used, aligned(8))) = 0x0101010101010101LL;
static PNG_CONST ull _HBClearMask __attribute__((used, aligned(8))) = 0x7f7f7f7f7f7f7f7fLL;
static PNG_CONST ull _amask0_8_0  __attribute__((used, aligned(8))) = 0xFFFFFFFFFFFFFFFFLL;
static PNG_CONST ull _amask6_2_0  __attribute__((used, aligned(8))) = 0x000000000000FFFFLL;

// png_read_filter_row_mmx_paeth() constants:
static PNG_CONST ull _amask4_4_0  __attribute__((used, aligned(8))) = 0x00000000FFFFFFFFLL;
static PNG_CONST ull _amask0_2_6  __attribute__((used, aligned(8))) = 0xFFFF000000000000LL;

// png_read_filter_row_mmx_sub() constants:
static PNG_CONST ull _amask2_3_3  __attribute__((used, aligned(8))) = 0x0000FFFFFF000000LL;
static PNG_CONST ull _amask4_2_2  __attribute__((used, aligned(8))) = 0x00000000FFFF0000LL;

#define MASK8_0        "_mask8_0"
#define MASK16_0       "_mask16_0"
#define MASK16_1       "_mask16_1"
#define MASK24_0       "_mask24_0"
#define MASK24_1       "_mask24_1"
#define MASK24_2       "_mask24_2"
#define MASK32_0       "_mask32_0"
#define MASK32_1       "_mask32_1"
#define MASK32_2       "_mask32_2"
#define MASK32_3       "_mask32_3"
#define MASK48_0       "_mask48_0"
#define MASK48_1       "_mask48_1"
#define MASK48_2       "_mask48_2"
#define MASK48_3       "_mask48_3"
#define MASK48_4       "_mask48_4"
#define MASK48_5       "_mask48_5"
#define AMASK5_3_0     "_amask5_3_0"
#define AMASK7_1_0     "_amask7_1_0"
#define LB_CARRY_MASK  "_LBCarryMask"
#define HB_CLEAR_MASK  "_HBClearMask"
#define AMASK0_8_0     "_amask0_8_0"
#define AMASK6_2_0     "_amask6_2_0"
#define AMASK4_4_0     "_amask4_4_0"
#define AMASK0_2_6     "_amask0_2_6"
#define AMASK2_3_3     "_amask2_3_3"
#define AMASK4_2_2     "_amask4_2_2"

#endif // ?PNG_x86_64_USE_GOTPCREL


#if defined(PNG_HAVE_MMX_READ_FILTER_ROW) || defined(PNG_HAVE_MMX_COMBINE_ROW)

// this block is specific to png_read_filter_row_mmx_paeth() except for
// LOAD_GOT_rbp and RESTORE_rbp, which are also used in png_combine_row()
#if defined(PNG_x86_64_USE_GOTPCREL)
#  define pa_TEMP                "%%r11d"
#  define pb_TEMP                "%%r12d"
#  define pc_TEMP                "%%r13d"
#  if defined(PNG_CLOBBER_x86_64_REGS_SUPPORTED)  // works as of gcc 3.4.3 ...
#    define SAVE_r11_r12_r13
#    define RESTORE_r11_r12_r13
#    define _CLOBBER_r11_r12_r13 ,"%r11", "%r12", "%r13"
#    define CLOBBER_r11_r12_r13  "%r11", "%r12", "%r13"
#  else // !PNG_CLOBBER_x86_64_REGS_SUPPORTED
#    define SAVE_r11_r12_r13     "pushq %%r11  \n\t" \
                                 "pushq %%r12  \n\t" \
                                 "pushq %%r13  \n\t"  // "normally 0-extended"
#    define RESTORE_r11_r12_r13  "popq  %%r13  \n\t" \
                                 "popq  %%r12  \n\t" \
                                 "popq  %%r11  \n\t"
#    define _CLOBBER_r11_r12_r13
#    define CLOBBER_r11_r12_r13
#  endif
#  define LOAD_GOT_rbp           "pushq %%rbp                        \n\t" \
                                 "movq  _c64@GOTPCREL(%%rip), %%rbp  \n\t"
#  define RESTORE_rbp            "popq  %%rbp                        \n\t"
#else // 32-bit and/or non-PIC
#  if defined(PNG_THREAD_UNSAFE_OK)
     // These variables are used in png_read_filter_row_mmx_paeth() and would be
     //   local variables if not for gcc-inline-assembly addressing limitations
     //   (some apparently related to ELF format, others to CPU type).
     //
     // WARNING: Their presence defeats the thread-safety of libpng.
     static int                     _patemp  __attribute__((used));
     static int                     _pbtemp  __attribute__((used));
     static int                     _pctemp  __attribute__((used));
#    define pa_TEMP                "_patemp"
#    define pb_TEMP                "_pbtemp"  // temp variables for
#    define pc_TEMP                "_pctemp"  //  Paeth routine
#    define SAVE_r11_r12_r13
#    define RESTORE_r11_r12_r13
#    define _CLOBBER_r11_r12_r13   // not using regs => not clobbering
#    define CLOBBER_r11_r12_r13
#  endif // PNG_THREAD_UNSAFE_OK
#  define LOAD_GOT_rbp
#  define RESTORE_rbp
#endif

#if defined(__x86_64__)
#  define SAVE_ebp
#  define RESTORE_ebp
#  define _CLOBBER_ebp         ,"%ebp"
#  define CLOBBER_ebp          "%ebp"
#  define SAVE_FullLength      "movl %%eax, %%r15d  \n\t"
#  define RESTORE_FullLength   "movl %%r15d, "     // may go into eax or ecx
#  if defined(PNG_CLOBBER_x86_64_REGS_SUPPORTED)   // works as of gcc 3.4.3 ...
#    define SAVE_r15
#    define RESTORE_r15
#    define _CLOBBER_r15       ,"%r15"
#  else
#    define SAVE_r15           "pushq %%r15  \n\t"
#    define RESTORE_r15        "popq  %%r15  \n\t"
#    define _CLOBBER_r15
#  endif
#  define PBP                  "%%rbp"             // regs used for 64-bit
#  define PAX                  "%%rax"             //  pointers or in
#  define PBX                  "%%rbx"             //  combination with
#  define PCX                  "%%rcx"             //  64-bit pointer-regs
#  define PDX                  "%%rdx"             //  (base/index pairs,
#  define PSI                  "%%rsi"             //  add/sub/mov pairs)
#  define CLEAR_BOTTOM_3_BITS  "and  $0xfffffffffffffff8, "
#else
#  define SAVE_ebp             "pushl %%ebp \n\t"  // clobber list doesn't work
#  define RESTORE_ebp          "popl  %%ebp \n\t"  //  for %ebp on 32-bit; not
#  define _CLOBBER_ebp                             //  clear why not
#  define CLOBBER_ebp
#  define SAVE_FullLength      "pushl %%eax \n\t"
#  define RESTORE_FullLength   "popl "             // eax (avg) or ecx (paeth)
#  define SAVE_r15
#  define RESTORE_r15
#  define _CLOBBER_r15
#  define PBP                  "%%ebp"             // regs used for or in
#  define PAX                  "%%eax"             //  combination with
#  define PBX                  "%%ebx"             //  "normal," 32-bit
#  define PCX                  "%%ecx"             //  pointers
#  define PDX                  "%%edx"
#  define PSI                  "%%esi"
#  define CLEAR_BOTTOM_3_BITS  "and  $0xfffffff8, "
#endif

// CLOB_COMMA_ebx_ebp:  need comma ONLY if both CLOBBER_ebp and CLOBBER_GOT_ebx
//                      have values, i.e., only if __x86_64__ AND !__PIC__
#if defined(__x86_64__) && !defined(__PIC__)
#  define CLOB_COMMA_ebx_ebp    , // clobbering both ebp and ebx => need comma
#else
#  define CLOB_COMMA_ebx_ebp
#endif

// CLOB_COMMA_ebX_r1X:  need comma UNLESS both CLOBBER_ebp and CLOBBER_GOT_ebx
//                   are empty OR CLOBBER_r11_r12_r13 is empty--i.e., NO comma
//                   if (!__x86_64__ AND __PIC__) OR !(PNG_x86_64_USE_GOTPCREL
//                   AND PNG_CLOBBER_x86_64_REGS_SUPPORTED)   (double sigh...)
#if (!defined(__x86_64__) && defined(__PIC__)) || \
    !defined(PNG_x86_64_USE_GOTPCREL) || \
    !defined(PNG_CLOBBER_x86_64_REGS_SUPPORTED)
#  define CLOB_COMMA_ebX_r1X
#else
#  define CLOB_COMMA_ebX_r1X    , // clobbering (ebp OR ebx) AND r11_r12_r13
#endif

// CLOB_COLON_ebx_ebp:  need colon unless CLOBBER_ebp and CLOBBER_GOT_ebx are
//                      BOTH empty--i.e., NO colon if (!__x86_64__ AND __PIC__)
// CLOB_COLON_ebx_ebp_r1X:  if, in addition, CLOBBER_r11_r12_r13 is empty, then
//                          no colon for Paeth blocks, either--i.e., NO colon
//                          if !(PNG_x86_64_USE_GOTPCREL AND
//                               PNG_CLOBBER_x86_64_REGS_SUPPORTED)
#if (!defined(__x86_64__) && defined(__PIC__))
#  define CLOB_COLON_ebx_ebp
#  if !(defined(PNG_x86_64_USE_GOTPCREL) && \
        defined(PNG_CLOBBER_x86_64_REGS_SUPPORTED))
#    define CLOB_COLON_ebx_ebp_r1X
#  else
#    define CLOB_COLON_ebx_ebp_r1X  : // clobbering ebp OR ebx OR r11_r12_r13
#  endif
#else
#  define CLOB_COLON_ebx_ebp        : // clobbering ebp OR ebx
#  define CLOB_COLON_ebx_ebp_r1X    : // clobbering ebp OR ebx OR r11_r12_r13
#endif

#endif // PNG_HAVE_MMX_READ_FILTER_ROW

#if defined(__PIC__)  // macros to save, restore index to Global Offset Table
#  if defined(__x86_64__)
#    define SAVE_GOT_ebx     "pushq %%rbx \n\t"
#    define RESTORE_GOT_ebx  "popq  %%rbx \n\t"
#  else
#    define SAVE_GOT_ebx     "pushl %%ebx \n\t"
#    define RESTORE_GOT_ebx  "popl  %%ebx \n\t"
#  endif
#  define _CLOBBER_GOT_ebx   // explicitly saved, restored => not clobbered
#  define CLOBBER_GOT_ebx
#else
#  define SAVE_GOT_ebx
#  define RESTORE_GOT_ebx
#  define _CLOBBER_GOT_ebx   ,"%ebx"
#  define CLOBBER_GOT_ebx    "%ebx"
#endif

#if defined(PNG_HAVE_MMX_COMBINE_ROW) || defined(PNG_HAVE_MMX_READ_INTERLACE)
#  define BPP2  2
#  define BPP3  3  // bytes per pixel (a.k.a. pixel_bytes)
#  define BPP4  4  // (defined only to help avoid cut-and-paste errors)
#  define BPP6  6
#  define BPP8  8
#endif



static int _mmx_supported = 2; // 0: no MMX; 1: MMX supported; 2: not tested

/*===========================================================================*/
/*                                                                           */
/*                      P N G _ M M X _ S U P P O R T                        */
/*                                                                           */
/*===========================================================================*/

// GRR NOTES:  (1) the following code assumes 386 or better (pushfl/popfl)
//             (2) all instructions compile with gcc 2.7.2.3 and later
//           x (3) the function is moved down here to prevent gcc from
//           x      inlining it in multiple places and then barfing be-
//           x      cause the ".NOT_SUPPORTED" label is multiply defined
//                  [need to retest with gcc 2.7.2.3]

// GRR 20070524:  This declaration apparently is compatible with but supersedes
//   the one in png.h; in any case, the generated object file is slightly
//   smaller.  It is unnecessary with gcc 4.1.2, but gcc 2.x apparently
//   replicated the ".NOT_SUPPORTED" label in each location the function was
//   inlined, leading to compilation errors due to the "multiply defined"
//   label.  Old workaround was to leave the function at the end of this
//   file; new one (still testing) is to use a gcc-specific function attribute
//   to prevent local inlining.
int PNGAPI
png_mmx_support(void) __attribute__((noinline));

int PNGAPI
png_mmx_support(void)
{
#if defined(PNG_MMX_CODE_SUPPORTED)  // superfluous, but what the heck
    int result;
    __asm__ __volatile__ (
#if defined(__x86_64__)
        "pushq %%rbx          \n\t"  // rbx gets clobbered by CPUID instruction
        "pushq %%rcx          \n\t"  // so does rcx...
        "pushq %%rdx          \n\t"  // ...and rdx (but rcx & rdx safe on Linux)
        "pushfq               \n\t"  // save Eflag to stack
        "popq %%rax           \n\t"  // get Eflag from stack into rax
        "movq %%rax, %%rcx    \n\t"  // make another copy of Eflag in rcx
        "xorl $0x200000, %%eax \n\t" // toggle ID bit in Eflag (i.e., bit 21)
        "pushq %%rax          \n\t"  // save modified Eflag back to stack
        "popfq                \n\t"  // restore modified value to Eflag reg
        "pushfq               \n\t"  // save Eflag to stack
        "popq %%rax           \n\t"  // get Eflag from stack
        "pushq %%rcx          \n\t"  // save original Eflag to stack
        "popfq                \n\t"  // restore original Eflag
#else
        "pushl %%ebx          \n\t"  // ebx gets clobbered by CPUID instruction
        "pushl %%ecx          \n\t"  // so does ecx...
        "pushl %%edx          \n\t"  // ...and edx (but ecx & edx safe on Linux)
        "pushfl               \n\t"  // save Eflag to stack
        "popl %%eax           \n\t"  // get Eflag from stack into eax
        "movl %%eax, %%ecx    \n\t"  // make another copy of Eflag in ecx
        "xorl $0x200000, %%eax \n\t" // toggle ID bit in Eflag (i.e., bit 21)
        "pushl %%eax          \n\t"  // save modified Eflag back to stack
        "popfl                \n\t"  // restore modified value to Eflag reg
        "pushfl               \n\t"  // save Eflag to stack
        "popl %%eax           \n\t"  // get Eflag from stack
        "pushl %%ecx          \n\t"  // save original Eflag to stack
        "popfl                \n\t"  // restore original Eflag
#endif
        "xorl %%ecx, %%eax    \n\t"  // compare new Eflag with original Eflag
        "jz 0f                \n\t"  // if same, CPUID instr. is not supported

        "xorl %%eax, %%eax    \n\t"  // set eax to zero
//      ".byte  0x0f, 0xa2    \n\t"  // CPUID instruction (two-byte opcode)
        "cpuid                \n\t"  // get the CPU identification info
        "cmpl $1, %%eax       \n\t"  // make sure eax return non-zero value
        "jl 0f                \n\t"  // if eax is zero, MMX is not supported

        "xorl %%eax, %%eax    \n\t"  // set eax to zero and...
        "incl %%eax           \n\t"  // ...increment eax to 1.  This pair is
                                     // faster than the instruction "mov eax, 1"
        "cpuid                \n\t"  // get the CPU identification info again
        "andl $0x800000, %%edx \n\t" // mask out all bits but MMX bit (23)
        "cmpl $0, %%edx       \n\t"  // 0 = MMX not supported
        "jz 0f                \n\t"  // non-zero = yes, MMX IS supported

        "movl $1, %%eax       \n\t"  // set return value to 1
        "jmp  1f              \n\t"  // DONE:  have MMX support

    "0:                       \n\t"  // .NOT_SUPPORTED: target label for jump instructions
        "movl $0, %%eax       \n\t"  // set return value to 0
    "1:                       \n\t"  // .RETURN: target label for jump instructions
#if defined(__x86_64__)
        "popq %%rdx           \n\t"  // restore rdx
        "popq %%rcx           \n\t"  // restore rcx
        "popq %%rbx           \n\t"  // restore rbx
#else
        "popl %%edx           \n\t"  // restore edx
        "popl %%ecx           \n\t"  // restore ecx
        "popl %%ebx           \n\t"  // restore ebx
#endif

//      "ret                  \n\t"  // DONE:  no MMX support
                                     // (fall through to standard C "ret")

        : "=a" (result)              // output list

        :                            // any variables used on input (none)

                                     // no clobber list
//      , "%ebx", "%ecx", "%edx"     // GRR:  we handle these manually
//      , "memory"   // if write to a variable gcc thought was in a reg
//      , "cc"       // "condition codes" (flag bits)
    );
    _mmx_supported = result;
#else
    _mmx_supported = 0;
#endif /* PNG_MMX_CODE_SUPPORTED */

    return _mmx_supported;
}


/*===========================================================================*/
/*                                                                           */
/*                       P N G _ C O M B I N E _ R O W                       */
/*                                                                           */
/*===========================================================================*/

#if defined(PNG_HAVE_MMX_COMBINE_ROW)

/* Combines the row recently read in with the previous row.
   This routine takes care of alpha and transparency if requested.
   This routine also handles the two methods of progressive display
   of interlaced images, depending on the mask value.
   The mask value describes which pixels are to be combined with
   the row.  The pattern always repeats every 8 pixels, so just 8
   bits are needed.  A one indicates the pixel is to be combined; a
   zero indicates the pixel is to be skipped.  This is in addition
   to any alpha or transparency value associated with the pixel.
   If you want all pixels to be combined, pass 0xff (255) in mask. */

/* Use this routine for the x86 platform - it uses a faster MMX routine
   if the machine supports MMX. */

void /* PRIVATE */
png_combine_row(png_structp png_ptr, png_bytep row, int mask)
{
   int dummy_value_a;    // fix 'forbidden register spilled' error
   int dummy_value_c;
   int dummy_value_d;
   png_bytep dummy_value_S;
   png_bytep dummy_value_D;

   png_debug(1, "in png_combine_row (pnggccrd.c)\n");

   if (_mmx_supported == 2) {
#if !defined(PNG_1_0_X)
       /* this should have happened in png_init_mmx_flags() already */
       png_warning(png_ptr, "asm_flags may not have been initialized");
#endif
       png_mmx_support();
   }

   if (mask == 0xff)
   {
      png_debug(2,"mask == 0xff:  doing single png_memcpy()\n");
      png_memcpy(row, png_ptr->row_buf + 1,
       (png_size_t)PNG_ROWBYTES(png_ptr->row_info.pixel_depth,png_ptr->width));
   }
   else   /* (png_combine_row() is never called with mask == 0) */
   {
      switch (png_ptr->row_info.pixel_depth)
      {
         case 24:       /* png_ptr->row_info.pixel_depth */
         {
            png_bytep srcptr;
            png_bytep dstptr;

#if !defined(PNG_1_0_X)
            if (png_ptr->asm_flags & PNG_ASM_FLAG_MMX_READ_COMBINE_ROW)
#else
            if (_mmx_supported)
#endif
            {
               png_uint_32 len;
               int diff;

               srcptr = png_ptr->row_buf + 1;
               dstptr = row;
               len  = png_ptr->width & ~7;          // reduce to multiple of 8
               diff = (int) (png_ptr->width & 7);   // amount lost

               __asm__ __volatile__ (
                  "not       %%edx            \n\t" // mask => unmask
                  "movd      %%edx, %%mm7     \n\t" // load bit pattern
                  "not       %%edx            \n\t" // unmask => mask for later
                  "psubb     %%mm6, %%mm6     \n\t" // zero mm6
                  "punpcklbw %%mm7, %%mm7     \n\t"
                  "punpcklwd %%mm7, %%mm7     \n\t"
                  "punpckldq %%mm7, %%mm7     \n\t" // fill reg with 8 masks

                  LOAD_GOT_rbp
                  "movq   " MASK24_0 ", %%mm0 \n\t" // _mask24_0 -> mm0
                  "movq   " MASK24_1 ", %%mm1 \n\t" // _mask24_1 -> mm1
                  "movq   " MASK24_2 ", %%mm2 \n\t" // _mask24_2 -> mm2
                  RESTORE_rbp

                  "pand      %%mm7, %%mm0     \n\t"
                  "pand      %%mm7, %%mm1     \n\t"
                  "pand      %%mm7, %%mm2     \n\t"

                  "pcmpeqb   %%mm6, %%mm0     \n\t"
                  "pcmpeqb   %%mm6, %%mm1     \n\t"
                  "pcmpeqb   %%mm6, %%mm2     \n\t"

// preload        "movl      len, %%ecx       \n\t" // load length of line
// preload        "movl      srcptr, %3       \n\t" // load source
// preload        "movl      dstptr, %4       \n\t" // load dest

                  "cmpl      $0, %%ecx        \n\t"
                  "jz        mainloop24end    \n\t"

                "mainloop24:                  \n\t"
                  "movq      (%3), %%mm4      \n\t"
                  "pand      %%mm0, %%mm4     \n\t"
                  "movq      %%mm0, %%mm6     \n\t"
                  "movq      (%4), %%mm7      \n\t"
                  "pandn     %%mm7, %%mm6     \n\t"
                  "por       %%mm6, %%mm4     \n\t"
                  "movq      %%mm4, (%4)      \n\t"

                  "movq      8(%3), %%mm5     \n\t"
                  "pand      %%mm1, %%mm5     \n\t"
                  "movq      %%mm1, %%mm7     \n\t"
                  "movq      8(%4), %%mm6     \n\t"
                  "pandn     %%mm6, %%mm7     \n\t"
                  "por       %%mm7, %%mm5     \n\t"
                  "movq      %%mm5, 8(%4)     \n\t"

                  "movq      16(%3), %%mm6    \n\t"
                  "pand      %%mm2, %%mm6     \n\t"
                  "movq      %%mm2, %%mm4     \n\t"
                  "movq      16(%4), %%mm7    \n\t"
                  "pandn     %%mm7, %%mm4     \n\t"
                  "por       %%mm4, %%mm6     \n\t"
                  "movq      %%mm6, 16(%4)    \n\t"

                  "add       $24, %3          \n\t" // inc by 24 bytes processed
                  "add       $24, %4          \n\t"
                  "subl      $8, %%ecx        \n\t" // dec by 8 pixels processed

                  "ja        mainloop24       \n\t"

                "mainloop24end:               \n\t"
// preload        "movl      diff, %%ecx      \n\t" // (diff is in eax)
                  "movl      %%eax, %%ecx     \n\t"
                  "cmpl      $0, %%ecx        \n\t"
                  "jz        end24            \n\t"
// preload        "movl      mask, %%edx      \n\t"
                  "sall      $24, %%edx       \n\t" // make low byte, high byte

                "secondloop24:                \n\t"
                  "sall      %%edx            \n\t" // move high bit to CF
                  "jnc       skip24           \n\t" // if CF = 0
                  "movw      (%3), %%ax       \n\t"
                  "movw      %%ax, (%4)       \n\t"
                  "xorl      %%eax, %%eax     \n\t"
                  "movb      2(%3), %%al      \n\t"
                  "movb      %%al, 2(%4)      \n\t"

                "skip24:                      \n\t"
                  "add       $3, %3           \n\t"
                  "add       $3, %4           \n\t"
                  "decl      %%ecx            \n\t"
                  "jnz       secondloop24     \n\t"

                "end24:                       \n\t"
                  "EMMS                       \n\t" // DONE

                  : "=a" (dummy_value_a),           // output regs (dummy)
                    "=d" (dummy_value_d),
                    "=c" (dummy_value_c),
                    "=S" (dummy_value_S),
                    "=D" (dummy_value_D)

                  : "0" (diff),        // eax       // input regs
                    "1" (mask),        // edx
                    "2" (len),         // ecx
// was (unmask)     "b"    RESERVED    // ebx       // Global Offset Table idx
                    "3" (srcptr),      // esi/rsi
                    "4" (dstptr)       // edi/rdi

#if defined(PNG_CLOBBER_MMX_REGS_SUPPORTED)
                  : "%mm0", "%mm1", "%mm2"          // clobber list
                  , "%mm4", "%mm5", "%mm6", "%mm7"
#endif
               );
            }
            else /* not _mmx_supported - use modified C routine */
            {
               register png_uint_32 i;
               png_uint_32 initial_val = BPP3 * png_pass_start[png_ptr->pass];
                 /* png.c:  png_pass_start[] = {0, 4, 0, 2, 0, 1, 0}; */
               register int stride = BPP3 * png_pass_inc[png_ptr->pass];
                 /* png.c:  png_pass_inc[] = {8, 8, 4, 4, 2, 2, 1}; */
               register int rep_bytes = BPP3 * png_pass_width[png_ptr->pass];
                 /* png.c:  png_pass_width[] = {8, 4, 4, 2, 2, 1, 1}; */
               png_uint_32 len = png_ptr->width &~7;  /* reduce to mult. of 8 */
               int diff = (int) (png_ptr->width & 7); /* amount lost */
               register png_uint_32 final_val = BPP3 * len;   /* GRR bugfix */

               srcptr = png_ptr->row_buf + 1 + initial_val;
               dstptr = row + initial_val;

               for (i = initial_val; i < final_val; i += stride)
               {
                  png_memcpy(dstptr, srcptr, rep_bytes);
                  srcptr += stride;
                  dstptr += stride;
               }
               if (diff)  /* number of leftover pixels:  3 for pngtest */
               {
                  final_val += diff*BPP3;
                  for (; i < final_val; i += stride)
                  {
                     if (rep_bytes > (int)(final_val-i))
                        rep_bytes = (int)(final_val-i);
                     png_memcpy(dstptr, srcptr, rep_bytes);
                     srcptr += stride;
                     dstptr += stride;
                  }
               }
            } /* end of else (_mmx_supported) */

            break;
         }       /* end 24 bpp */

         // formerly claimed to be most common case (combining 32-bit RGBA),
         // but almost certainly less common than 24-bit RGB case
         case 32:       /* png_ptr->row_info.pixel_depth */
         {
            png_bytep srcptr;
            png_bytep dstptr;

#if !defined(PNG_1_0_X)
            if (png_ptr->asm_flags & PNG_ASM_FLAG_MMX_READ_COMBINE_ROW)
#else
            if (_mmx_supported)
#endif
            {
               png_uint_32 len;
               int diff;

               srcptr = png_ptr->row_buf + 1;
               dstptr = row;
               len  = png_ptr->width & ~7;          // reduce to multiple of 8
               diff = (int) (png_ptr->width & 7);   // amount lost

               __asm__ __volatile__ (
                  "not       %%edx            \n\t" // mask => unmask
                  "movd      %%edx, %%mm7     \n\t" // load bit pattern
                  "not       %%edx            \n\t" // unmask => mask for later
                  "psubb     %%mm6, %%mm6     \n\t" // zero mm6
                  "punpcklbw %%mm7, %%mm7     \n\t"
                  "punpcklwd %%mm7, %%mm7     \n\t"
                  "punpckldq %%mm7, %%mm7     \n\t" // fill reg with 8 masks

                  LOAD_GOT_rbp
                  "movq   " MASK32_0 ", %%mm0 \n\t" // _mask32_0
                  "movq   " MASK32_1 ", %%mm1 \n\t" // _mask32_1
                  "movq   " MASK32_2 ", %%mm2 \n\t" // _mask32_2
                  "movq   " MASK32_3 ", %%mm3 \n\t" // _mask32_3
                  RESTORE_rbp

                  "pand      %%mm7, %%mm0     \n\t"
                  "pand      %%mm7, %%mm1     \n\t"
                  "pand      %%mm7, %%mm2     \n\t"
                  "pand      %%mm7, %%mm3     \n\t"

                  "pcmpeqb   %%mm6, %%mm0     \n\t"
                  "pcmpeqb   %%mm6, %%mm1     \n\t"
                  "pcmpeqb   %%mm6, %%mm2     \n\t"
                  "pcmpeqb   %%mm6, %%mm3     \n\t"

// preload        "movl      len, %%ecx       \n\t" // load length of line
// preload        "movl      srcptr, %3       \n\t" // load source
// preload        "movl      dstptr, %4       \n\t" // load dest

                  "cmpl      $0, %%ecx        \n\t" // lcr
                  "jz        mainloop32end    \n\t"

                "mainloop32:                  \n\t"
                  "movq      (%3), %%mm4      \n\t"
                  "pand      %%mm0, %%mm4     \n\t"
                  "movq      %%mm0, %%mm6     \n\t"
                  "movq      (%4), %%mm7      \n\t"
                  "pandn     %%mm7, %%mm6     \n\t"
                  "por       %%mm6, %%mm4     \n\t"
                  "movq      %%mm4, (%4)      \n\t"

                  "movq      8(%3), %%mm5     \n\t"
                  "pand      %%mm1, %%mm5     \n\t"
                  "movq      %%mm1, %%mm7     \n\t"
                  "movq      8(%4), %%mm6     \n\t"
                  "pandn     %%mm6, %%mm7     \n\t"
                  "por       %%mm7, %%mm5     \n\t"
                  "movq      %%mm5, 8(%4)     \n\t"

                  "movq      16(%3), %%mm6    \n\t"
                  "pand      %%mm2, %%mm6     \n\t"
                  "movq      %%mm2, %%mm4     \n\t"
                  "movq      16(%4), %%mm7    \n\t"
                  "pandn     %%mm7, %%mm4     \n\t"
                  "por       %%mm4, %%mm6     \n\t"
                  "movq      %%mm6, 16(%4)    \n\t"

                  "movq      24(%3), %%mm7    \n\t"
                  "pand      %%mm3, %%mm7     \n\t"
                  "movq      %%mm3, %%mm5     \n\t"
                  "movq      24(%4), %%mm4    \n\t"
                  "pandn     %%mm4, %%mm5     \n\t"
                  "por       %%mm5, %%mm7     \n\t"
                  "movq      %%mm7, 24(%4)    \n\t"

                  "add       $32, %3          \n\t" // inc by 32 bytes processed
                  "add       $32, %4          \n\t"
                  "subl      $8, %%ecx        \n\t" // dec by 8 pixels processed
                  "ja        mainloop32       \n\t"

                "mainloop32end:               \n\t"
// preload        "movl      diff, %%ecx      \n\t" // (diff is in eax)
                  "movl      %%eax, %%ecx     \n\t"
                  "cmpl      $0, %%ecx        \n\t"
                  "jz        end32            \n\t"
// preload        "movl      mask, %%edx      \n\t"
                  "sall      $24, %%edx       \n\t" // low byte => high byte

                "secondloop32:                \n\t"
                  "sall      %%edx            \n\t" // move high bit to CF
                  "jnc       skip32           \n\t" // if CF = 0
                  "movl      (%3), %%eax      \n\t"
                  "movl      %%eax, (%4)      \n\t"

                "skip32:                      \n\t"
                  "add       $4, %3           \n\t"
                  "add       $4, %4           \n\t"
                  "decl      %%ecx            \n\t"
                  "jnz       secondloop32     \n\t"

                "end32:                       \n\t"
                  "EMMS                       \n\t" // DONE

                  : "=a" (dummy_value_a),           // output regs (dummy)
                    "=d" (dummy_value_d),
                    "=c" (dummy_value_c),
                    "=S" (dummy_value_S),
                    "=D" (dummy_value_D)

                  : "0" (diff),        // eax       // input regs
                    "1" (mask),        // edx
                    "2" (len),         // ecx
// was (unmask)     "b"    RESERVED    // ebx       // Global Offset Table idx
                    "3" (srcptr),      // esi/rsi
                    "4" (dstptr)       // edi/rdi

#if defined(PNG_CLOBBER_MMX_REGS_SUPPORTED)
                  : "%mm0", "%mm1", "%mm2", "%mm3"  // clobber list
                  , "%mm4", "%mm5", "%mm6", "%mm7"
#endif
               );
            }
            else /* not _mmx_supported - use modified C routine */
            {
               register png_uint_32 i;
               png_uint_32 initial_val = BPP4 * png_pass_start[png_ptr->pass];
                 /* png.c:  png_pass_start[] = {0, 4, 0, 2, 0, 1, 0}; */
               register int stride = BPP4 * png_pass_inc[png_ptr->pass];
                 /* png.c:  png_pass_inc[] = {8, 8, 4, 4, 2, 2, 1}; */
               register int rep_bytes = BPP4 * png_pass_width[png_ptr->pass];
                 /* png.c:  png_pass_width[] = {8, 4, 4, 2, 2, 1, 1}; */
               png_uint_32 len = png_ptr->width &~7;  /* reduce to mult. of 8 */
               int diff = (int) (png_ptr->width & 7); /* amount lost */
               register png_uint_32 final_val = BPP4 * len;   /* GRR bugfix */

               srcptr = png_ptr->row_buf + 1 + initial_val;
               dstptr = row + initial_val;

               for (i = initial_val; i < final_val; i += stride)
               {
                  png_memcpy(dstptr, srcptr, rep_bytes);
                  srcptr += stride;
                  dstptr += stride;
               }
               if (diff)  /* number of leftover pixels:  3 for pngtest */
               {
                  final_val += diff*BPP4;
                  for (; i < final_val; i += stride)
                  {
                     if (rep_bytes > (int)(final_val-i))
                        rep_bytes = (int)(final_val-i);
                     png_memcpy(dstptr, srcptr, rep_bytes);
                     srcptr += stride;
                     dstptr += stride;
                  }
               }
            } /* end of else (_mmx_supported) */

            break;
         }       /* end 32 bpp */

         case 8:        /* png_ptr->row_info.pixel_depth */
         {
            png_bytep srcptr;
            png_bytep dstptr;

#if !defined(PNG_1_0_X)
            if (png_ptr->asm_flags & PNG_ASM_FLAG_MMX_READ_COMBINE_ROW)
#else
            if (_mmx_supported)
#endif
            {
               png_uint_32 len;
               int diff;

               srcptr = png_ptr->row_buf + 1;
               dstptr = row;
               len  = png_ptr->width & ~7;          // reduce to multiple of 8
               diff = (int) (png_ptr->width & 7);   // amount lost

               __asm__ __volatile__ (
                  "not       %%edx            \n\t" // mask => unmask
                  "movd      %%edx, %%mm7     \n\t" // load bit pattern
                  "not       %%edx            \n\t" // unmask => mask for later
                  "psubb     %%mm6, %%mm6     \n\t" // zero mm6
                  "punpcklbw %%mm7, %%mm7     \n\t"
                  "punpcklwd %%mm7, %%mm7     \n\t"
                  "punpckldq %%mm7, %%mm7     \n\t" // fill reg with 8 masks

                  LOAD_GOT_rbp
                  "movq   " MASK8_0 ", %%mm0  \n\t" // _mask8_0 -> mm0
                  RESTORE_rbp

                  "pand      %%mm7, %%mm0     \n\t" // nonzero if keep byte
                  "pcmpeqb   %%mm6, %%mm0     \n\t" // zeros->1s, v versa

// preload        "movl      len, %%ecx       \n\t" // load length of line
// preload        "movl      srcptr, %3       \n\t" // load source
// preload        "movl      dstptr, %4       \n\t" // load dest

                  "cmpl      $0, %%ecx        \n\t" // len == 0 ?
                  "je        mainloop8end     \n\t"

                "mainloop8:                   \n\t"
                  "movq      (%3), %%mm4      \n\t" // *srcptr
                  "pand      %%mm0, %%mm4     \n\t"
                  "movq      %%mm0, %%mm6     \n\t"
                  "pandn     (%4), %%mm6      \n\t" // *dstptr
                  "por       %%mm6, %%mm4     \n\t"
                  "movq      %%mm4, (%4)      \n\t"
                  "add       $8, %3           \n\t" // inc by 8 bytes processed
                  "add       $8, %4           \n\t"
                  "subl      $8, %%ecx        \n\t" // dec by 8 pixels processed
                  "ja        mainloop8        \n\t"

                "mainloop8end:                \n\t"
// preload        "movl      diff, %%ecx      \n\t" // (diff is in eax)
                  "movl      %%eax, %%ecx     \n\t"
                  "cmpl      $0, %%ecx        \n\t"
                  "jz        end8             \n\t"
// preload        "movl      mask, %%edx      \n\t"
                  "sall      $24, %%edx       \n\t" // make low byte, high byte

                "secondloop8:                 \n\t"
                  "sall      %%edx            \n\t" // move high bit to CF
                  "jnc       skip8            \n\t" // if CF = 0
                  "movb      (%3), %%al       \n\t"
                  "movb      %%al, (%4)       \n\t"

                "skip8:                       \n\t"
                  "inc       %3               \n\t"
                  "inc       %4               \n\t"
                  "decl      %%ecx            \n\t"
                  "jnz       secondloop8      \n\t"

                "end8:                        \n\t"
                  "EMMS                       \n\t" // DONE

                  : "=a" (dummy_value_a),           // output regs (dummy)
                    "=d" (dummy_value_d),
                    "=c" (dummy_value_c),
                    "=S" (dummy_value_S),
                    "=D" (dummy_value_D)

                  : "0" (diff),        // eax       // input regs
                    "1" (mask),        // edx
                    "2" (len),         // ecx
// was (unmask)     "b"    RESERVED    // ebx       // Global Offset Table idx
                    "3" (srcptr),      // esi/rsi
                    "4" (dstptr)       // edi/rdi

#if defined(PNG_CLOBBER_MMX_REGS_SUPPORTED)
                  : "%mm0", "%mm4", "%mm6", "%mm7"  // clobber list
#endif
               );
            }
            else /* not _mmx_supported - use modified C routine */
            {
               register png_uint_32 i;
               png_uint_32 initial_val = png_pass_start[png_ptr->pass];
                 /* png.c:  png_pass_start[] = {0, 4, 0, 2, 0, 1, 0}; */
               register int stride = png_pass_inc[png_ptr->pass];
                 /* png.c:  png_pass_inc[] = {8, 8, 4, 4, 2, 2, 1}; */
               register int rep_bytes = png_pass_width[png_ptr->pass];
                 /* png.c:  png_pass_width[] = {8, 4, 4, 2, 2, 1, 1}; */
               png_uint_32 len = png_ptr->width &~7;  /* reduce to mult. of 8 */
               int diff = (int) (png_ptr->width & 7); /* amount lost */
               register png_uint_32 final_val = len;  /* GRR bugfix */

               srcptr = png_ptr->row_buf + 1 + initial_val;
               dstptr = row + initial_val;

               for (i = initial_val; i < final_val; i += stride)
               {
                  png_memcpy(dstptr, srcptr, rep_bytes);
                  srcptr += stride;
                  dstptr += stride;
               }
               if (diff)  /* number of leftover pixels:  3 for pngtest */
               {
                  final_val += diff /* *BPP1 */ ;
                  for (; i < final_val; i += stride)
                  {
                     if (rep_bytes > (int)(final_val-i))
                        rep_bytes = (int)(final_val-i);
                     png_memcpy(dstptr, srcptr, rep_bytes);
                     srcptr += stride;
                     dstptr += stride;
                  }
               }

            } /* end of else (_mmx_supported) */

            break;
         }       /* end 8 bpp */

         case 1:        /* png_ptr->row_info.pixel_depth */
         {
            png_bytep sp;
            png_bytep dp;
            int s_inc, s_start, s_end;
            int m;
            int shift;
            png_uint_32 i;

            sp = png_ptr->row_buf + 1;
            dp = row;
            m = 0x80;
#if defined(PNG_READ_PACKSWAP_SUPPORTED)
            if (png_ptr->transformations & PNG_PACKSWAP)
            {
               s_start = 0;
               s_end = 7;
               s_inc = 1;
            }
            else
#endif
            {
               s_start = 7;
               s_end = 0;
               s_inc = -1;
            }

            shift = s_start;

            for (i = 0; i < png_ptr->width; i++)
            {
               if (m & mask)
               {
                  int value;

                  value = (*sp >> shift) & 0x1;
                  *dp &= (png_byte)((0x7f7f >> (7 - shift)) & 0xff);
                  *dp |= (png_byte)(value << shift);
               }

               if (shift == s_end)
               {
                  shift = s_start;
                  sp++;
                  dp++;
               }
               else
                  shift += s_inc;

               if (m == 1)
                  m = 0x80;
               else
                  m >>= 1;
            }
            break;
         }       /* end 1 bpp */

         case 2:        /* png_ptr->row_info.pixel_depth */
         {
            png_bytep sp;
            png_bytep dp;
            int s_start, s_end, s_inc;
            int m;
            int shift;
            png_uint_32 i;
            int value;

            sp = png_ptr->row_buf + 1;
            dp = row;
            m = 0x80;
#if defined(PNG_READ_PACKSWAP_SUPPORTED)
            if (png_ptr->transformations & PNG_PACKSWAP)
            {
               s_start = 0;
               s_end = 6;
               s_inc = 2;
            }
            else
#endif
            {
               s_start = 6;
               s_end = 0;
               s_inc = -2;
            }

            shift = s_start;

            for (i = 0; i < png_ptr->width; i++)
            {
               if (m & mask)
               {
                  value = (*sp >> shift) & 0x3;
                  *dp &= (png_byte)((0x3f3f >> (6 - shift)) & 0xff);
                  *dp |= (png_byte)(value << shift);
               }

               if (shift == s_end)
               {
                  shift = s_start;
                  sp++;
                  dp++;
               }
               else
                  shift += s_inc;
               if (m == 1)
                  m = 0x80;
               else
                  m >>= 1;
            }
            break;
         }       /* end 2 bpp */

         case 4:        /* png_ptr->row_info.pixel_depth */
         {
            png_bytep sp;
            png_bytep dp;
            int s_start, s_end, s_inc;
            int m;
            int shift;
            png_uint_32 i;
            int value;

            sp = png_ptr->row_buf + 1;
            dp = row;
            m = 0x80;
#if defined(PNG_READ_PACKSWAP_SUPPORTED)
            if (png_ptr->transformations & PNG_PACKSWAP)
            {
               s_start = 0;
               s_end = 4;
               s_inc = 4;
            }
            else
#endif
            {
               s_start = 4;
               s_end = 0;
               s_inc = -4;
            }

            shift = s_start;

            for (i = 0; i < png_ptr->width; i++)
            {
               if (m & mask)
               {
                  value = (*sp >> shift) & 0xf;
                  *dp &= (png_byte)((0xf0f >> (4 - shift)) & 0xff);
                  *dp |= (png_byte)(value << shift);
               }

               if (shift == s_end)
               {
                  shift = s_start;
                  sp++;
                  dp++;
               }
               else
                  shift += s_inc;
               if (m == 1)
                  m = 0x80;
               else
                  m >>= 1;
            }
            break;
         }       /* end 4 bpp */

         case 16:       /* png_ptr->row_info.pixel_depth */
         {
            png_bytep srcptr;
            png_bytep dstptr;

#if !defined(PNG_1_0_X)
            if (png_ptr->asm_flags & PNG_ASM_FLAG_MMX_READ_COMBINE_ROW)
#else
            if (_mmx_supported)
#endif
            {
               png_uint_32 len;
               int diff;

               srcptr = png_ptr->row_buf + 1;
               dstptr = row;
               len  = png_ptr->width & ~7;          // reduce to multiple of 8
               diff = (int) (png_ptr->width & 7);   // amount lost

               __asm__ __volatile__ (
                  "not       %%edx            \n\t" // mask => unmask
                  "movd      %%edx, %%mm7     \n\t" // load bit pattern
                  "not       %%edx            \n\t" // unmask => mask for later
                  "psubb     %%mm6, %%mm6     \n\t" // zero mm6
                  "punpcklbw %%mm7, %%mm7     \n\t"
                  "punpcklwd %%mm7, %%mm7     \n\t"
                  "punpckldq %%mm7, %%mm7     \n\t" // fill reg with 8 masks

                  LOAD_GOT_rbp
                  "movq   " MASK16_0 ", %%mm0 \n\t" // _mask16_0 -> mm0
                  "movq   " MASK16_1 ", %%mm1 \n\t" // _mask16_1 -> mm1
                  RESTORE_rbp

                  "pand      %%mm7, %%mm0     \n\t"
                  "pand      %%mm7, %%mm1     \n\t"

                  "pcmpeqb   %%mm6, %%mm0     \n\t"
                  "pcmpeqb   %%mm6, %%mm1     \n\t"

// preload        "movl      len, %%ecx       \n\t" // load length of line
// preload        "movl      srcptr, %3       \n\t" // load source
// preload        "movl      dstptr, %4       \n\t" // load dest

                  "cmpl      $0, %%ecx        \n\t"
                  "jz        mainloop16end    \n\t"

                "mainloop16:                  \n\t"
                  "movq      (%3), %%mm4      \n\t"
                  "pand      %%mm0, %%mm4     \n\t"
                  "movq      %%mm0, %%mm6     \n\t"
                  "movq      (%4), %%mm7      \n\t"
                  "pandn     %%mm7, %%mm6     \n\t"
                  "por       %%mm6, %%mm4     \n\t"
                  "movq      %%mm4, (%4)      \n\t"

                  "movq      8(%3), %%mm5     \n\t"
                  "pand      %%mm1, %%mm5     \n\t"
                  "movq      %%mm1, %%mm7     \n\t"
                  "movq      8(%4), %%mm6     \n\t"
                  "pandn     %%mm6, %%mm7     \n\t"
                  "por       %%mm7, %%mm5     \n\t"
                  "movq      %%mm5, 8(%4)     \n\t"

                  "add       $16, %3          \n\t" // inc by 16 bytes processed
                  "add       $16, %4          \n\t"
                  "subl      $8, %%ecx        \n\t" // dec by 8 pixels processed
                  "ja        mainloop16       \n\t"

                "mainloop16end:               \n\t"
// preload        "movl      diff, %%ecx      \n\t" // (diff is in eax)
                  "movl      %%eax, %%ecx     \n\t"
                  "cmpl      $0, %%ecx        \n\t"
                  "jz        end16            \n\t"
// preload        "movl      mask, %%edx      \n\t"
                  "sall      $24, %%edx       \n\t" // make low byte, high byte

                "secondloop16:                \n\t"
                  "sall      %%edx            \n\t" // move high bit to CF
                  "jnc       skip16           \n\t" // if CF = 0
                  "movw      (%3), %%ax       \n\t"
                  "movw      %%ax, (%4)       \n\t"

                "skip16:                      \n\t"
                  "add       $2, %3           \n\t"
                  "add       $2, %4           \n\t"
                  "decl      %%ecx            \n\t"
                  "jnz       secondloop16     \n\t"

                "end16:                       \n\t"
                  "EMMS                       \n\t" // DONE

                  : "=a" (dummy_value_a),           // output regs (dummy)
                    "=d" (dummy_value_d),
                    "=c" (dummy_value_c),
                    "=S" (dummy_value_S),
                    "=D" (dummy_value_D)

                  : "0" (diff),        // eax       // input regs
                    "1" (mask),        // edx
                    "2" (len),         // ecx
// was (unmask)     "b"    RESERVED    // ebx       // Global Offset Table idx
                    "3" (srcptr),      // esi/rsi
                    "4" (dstptr)       // edi/rdi

#if defined(PNG_CLOBBER_MMX_REGS_SUPPORTED)
                  : "%mm0", "%mm1", "%mm4"          // clobber list
                  , "%mm5", "%mm6", "%mm7"
#endif
               );
            }
            else /* not _mmx_supported - use modified C routine */
            {
               register png_uint_32 i;
               png_uint_32 initial_val = BPP2 * png_pass_start[png_ptr->pass];
                 /* png.c:  png_pass_start[] = {0, 4, 0, 2, 0, 1, 0}; */
               register int stride = BPP2 * png_pass_inc[png_ptr->pass];
                 /* png.c:  png_pass_inc[] = {8, 8, 4, 4, 2, 2, 1}; */
               register int rep_bytes = BPP2 * png_pass_width[png_ptr->pass];
                 /* png.c:  png_pass_width[] = {8, 4, 4, 2, 2, 1, 1}; */
               png_uint_32 len = png_ptr->width &~7;  /* reduce to mult. of 8 */
               int diff = (int) (png_ptr->width & 7); /* amount lost */
               register png_uint_32 final_val = BPP2 * len;   /* GRR bugfix */

               srcptr = png_ptr->row_buf + 1 + initial_val;
               dstptr = row + initial_val;

               for (i = initial_val; i < final_val; i += stride)
               {
                  png_memcpy(dstptr, srcptr, rep_bytes);
                  srcptr += stride;
                  dstptr += stride;
               }
               if (diff)  /* number of leftover pixels:  3 for pngtest */
               {
                  final_val += diff*BPP2;
                  for (; i < final_val; i += stride)
                  {
                     if (rep_bytes > (int)(final_val-i))
                        rep_bytes = (int)(final_val-i);
                     png_memcpy(dstptr, srcptr, rep_bytes);
                     srcptr += stride;
                     dstptr += stride;
                  }
               }
            } /* end of else (_mmx_supported) */

            break;
         }       /* end 16 bpp */

         case 48:       /* png_ptr->row_info.pixel_depth */
         {
            png_bytep srcptr;
            png_bytep dstptr;

#if !defined(PNG_1_0_X)
            if (png_ptr->asm_flags & PNG_ASM_FLAG_MMX_READ_COMBINE_ROW)
#else
            if (_mmx_supported)
#endif
            {
               png_uint_32 len;
               int diff;

               srcptr = png_ptr->row_buf + 1;
               dstptr = row;
               len  = png_ptr->width & ~7;          // reduce to multiple of 8
               diff = (int) (png_ptr->width & 7);   // amount lost

               __asm__ __volatile__ (
                  "not       %%edx            \n\t" // mask => unmask
                  "movd      %%edx, %%mm7     \n\t" // load bit pattern
                  "not       %%edx            \n\t" // unmask => mask for later
                  "psubb     %%mm6, %%mm6     \n\t" // zero mm6
                  "punpcklbw %%mm7, %%mm7     \n\t"
                  "punpcklwd %%mm7, %%mm7     \n\t"
                  "punpckldq %%mm7, %%mm7     \n\t" // fill reg with 8 masks

                  LOAD_GOT_rbp
                  "movq   " MASK48_0 ", %%mm0 \n\t" // _mask48_0 -> mm0
                  "movq   " MASK48_1 ", %%mm1 \n\t" // _mask48_1 -> mm1
                  "movq   " MASK48_2 ", %%mm2 \n\t" // _mask48_2 -> mm2
                  "movq   " MASK48_3 ", %%mm3 \n\t" // _mask48_3 -> mm3
                  "movq   " MASK48_4 ", %%mm4 \n\t" // _mask48_4 -> mm4
                  "movq   " MASK48_5 ", %%mm5 \n\t" // _mask48_5 -> mm5
                  RESTORE_rbp

                  "pand      %%mm7, %%mm0     \n\t"
                  "pand      %%mm7, %%mm1     \n\t"
                  "pand      %%mm7, %%mm2     \n\t"
                  "pand      %%mm7, %%mm3     \n\t"
                  "pand      %%mm7, %%mm4     \n\t"
                  "pand      %%mm7, %%mm5     \n\t"

                  "pcmpeqb   %%mm6, %%mm0     \n\t"
                  "pcmpeqb   %%mm6, %%mm1     \n\t"
                  "pcmpeqb   %%mm6, %%mm2     \n\t"
                  "pcmpeqb   %%mm6, %%mm3     \n\t"
                  "pcmpeqb   %%mm6, %%mm4     \n\t"
                  "pcmpeqb   %%mm6, %%mm5     \n\t"

// preload        "movl      len, %%ecx       \n\t" // load length of line
// preload        "movl      srcptr, %3       \n\t" // load source
// preload        "movl      dstptr, %4       \n\t" // load dest

                  "cmpl      $0, %%ecx        \n\t"
                  "jz        mainloop48end    \n\t"

                "mainloop48:                  \n\t"
                  "movq      (%3), %%mm7      \n\t"
                  "pand      %%mm0, %%mm7     \n\t"
                  "movq      %%mm0, %%mm6     \n\t"
                  "pandn     (%4), %%mm6      \n\t"
                  "por       %%mm6, %%mm7     \n\t"
                  "movq      %%mm7, (%4)      \n\t"

                  "movq      8(%3), %%mm6     \n\t"
                  "pand      %%mm1, %%mm6     \n\t"
                  "movq      %%mm1, %%mm7     \n\t"
                  "pandn     8(%4), %%mm7     \n\t"
                  "por       %%mm7, %%mm6     \n\t"
                  "movq      %%mm6, 8(%4)     \n\t"

                  "movq      16(%3), %%mm6    \n\t"
                  "pand      %%mm2, %%mm6     \n\t"
                  "movq      %%mm2, %%mm7     \n\t"
                  "pandn     16(%4), %%mm7    \n\t"
                  "por       %%mm7, %%mm6     \n\t"
                  "movq      %%mm6, 16(%4)    \n\t"

                  "movq      24(%3), %%mm7    \n\t"
                  "pand      %%mm3, %%mm7     \n\t"
                  "movq      %%mm3, %%mm6     \n\t"
                  "pandn     24(%4), %%mm6    \n\t"
                  "por       %%mm6, %%mm7     \n\t"
                  "movq      %%mm7, 24(%4)    \n\t"

                  "movq      32(%3), %%mm6    \n\t"
                  "pand      %%mm4, %%mm6     \n\t"
                  "movq      %%mm4, %%mm7     \n\t"
                  "pandn     32(%4), %%mm7    \n\t"
                  "por       %%mm7, %%mm6     \n\t"
                  "movq      %%mm6, 32(%4)    \n\t"

                  "movq      40(%3), %%mm7    \n\t"
                  "pand      %%mm5, %%mm7     \n\t"
                  "movq      %%mm5, %%mm6     \n\t"
                  "pandn     40(%4), %%mm6    \n\t"
                  "por       %%mm6, %%mm7     \n\t"
                  "movq      %%mm7, 40(%4)    \n\t"

                  "add       $48, %3          \n\t" // inc by 48 bytes processed
                  "add       $48, %4          \n\t"
                  "subl      $8, %%ecx        \n\t" // dec by 8 pixels processed

                  "ja        mainloop48       \n\t"

                "mainloop48end:               \n\t"
// preload        "movl      diff, %%ecx      \n\t" // (diff is in eax)
                  "movl      %%eax, %%ecx     \n\t"
                  "cmpl      $0, %%ecx        \n\t"
                  "jz        end48            \n\t"
// preload        "movl      mask, %%edx      \n\t"
                  "sall      $24, %%edx       \n\t" // make low byte, high byte

                "secondloop48:                \n\t"
                  "sall      %%edx            \n\t" // move high bit to CF
                  "jnc       skip48           \n\t" // if CF = 0
                  "movl      (%3), %%eax      \n\t"
                  "movl      %%eax, (%4)      \n\t"
                  "movw      4(%3), %%ax      \n\t" // GR-P bugfix 20070717
                  "movw      %%ax, 4(%4)      \n\t" // GR-P bugfix 20070717

                "skip48:                      \n\t"
                  "add       $6, %3           \n\t" // GR-P bugfix 20070717
                  "add       $6, %4           \n\t" // GR-P bugfix 20070717
                  "decl      %%ecx            \n\t"
                  "jnz       secondloop48     \n\t"

                "end48:                       \n\t"
                  "EMMS                       \n\t" // DONE

                  : "=a" (dummy_value_a),           // output regs (dummy)
                    "=d" (dummy_value_d),
                    "=c" (dummy_value_c),
                    "=S" (dummy_value_S),
                    "=D" (dummy_value_D)

                  : "0" (diff),        // eax       // input regs
                    "1" (mask),        // edx
                    "2" (len),         // ecx
// was (unmask)     "b"    RESERVED    // ebx       // Global Offset Table idx
                    "3" (srcptr),      // esi/rsi
                    "4" (dstptr)       // edi/rdi

#if defined(PNG_CLOBBER_MMX_REGS_SUPPORTED)
                  : "%mm0", "%mm1", "%mm2", "%mm3"  // clobber list
                  , "%mm4", "%mm5", "%mm6", "%mm7"
#endif
               );
            }
            else /* not _mmx_supported - use modified C routine */
            {
               register png_uint_32 i;
               png_uint_32 initial_val = BPP6 * png_pass_start[png_ptr->pass];
                 /* png.c:  png_pass_start[] = {0, 4, 0, 2, 0, 1, 0}; */
               register int stride = BPP6 * png_pass_inc[png_ptr->pass];
                 /* png.c:  png_pass_inc[] = {8, 8, 4, 4, 2, 2, 1}; */
               register int rep_bytes = BPP6 * png_pass_width[png_ptr->pass];
                 /* png.c:  png_pass_width[] = {8, 4, 4, 2, 2, 1, 1}; */
               png_uint_32 len = png_ptr->width &~7;  /* reduce to mult. of 8 */
               int diff = (int) (png_ptr->width & 7); /* amount lost */
               register png_uint_32 final_val = BPP6 * len;   /* GRR bugfix */

               srcptr = png_ptr->row_buf + 1 + initial_val;
               dstptr = row + initial_val;

               for (i = initial_val; i < final_val; i += stride)
               {
                  png_memcpy(dstptr, srcptr, rep_bytes);
                  srcptr += stride;
                  dstptr += stride;
               }
               if (diff)  /* number of leftover pixels:  3 for pngtest */
               {
                  final_val += diff*BPP6;
                  for (; i < final_val; i += stride)
                  {
                     if (rep_bytes > (int)(final_val-i))
                        rep_bytes = (int)(final_val-i);
                     png_memcpy(dstptr, srcptr, rep_bytes);
                     srcptr += stride;
                     dstptr += stride;
                  }
               }
            } /* end of else (_mmx_supported) */

            break;
         }       /* end 48 bpp */

         case 64:       /* png_ptr->row_info.pixel_depth */
         {
            png_bytep srcptr;
            png_bytep dstptr;
            register png_uint_32 i;
            png_uint_32 initial_val = BPP8 * png_pass_start[png_ptr->pass];
              /* png.c:  png_pass_start[] = {0, 4, 0, 2, 0, 1, 0}; */
            register int stride = BPP8 * png_pass_inc[png_ptr->pass];
              /* png.c:  png_pass_inc[] = {8, 8, 4, 4, 2, 2, 1}; */
            register int rep_bytes = BPP8 * png_pass_width[png_ptr->pass];
              /* png.c:  png_pass_width[] = {8, 4, 4, 2, 2, 1, 1}; */
            png_uint_32 len = png_ptr->width &~7;  /* reduce to mult. of 8 */
            int diff = (int) (png_ptr->width & 7); /* amount lost */
            register png_uint_32 final_val = BPP8 * len;   /* GRR bugfix */

            srcptr = png_ptr->row_buf + 1 + initial_val;
            dstptr = row + initial_val;

            for (i = initial_val; i < final_val; i += stride)
            {
               png_memcpy(dstptr, srcptr, rep_bytes);
               srcptr += stride;
               dstptr += stride;
            }
            if (diff)  /* number of leftover pixels:  3 for pngtest */
            {
               final_val += diff*BPP8;
               for (; i < final_val; i += stride)
               {
                  if (rep_bytes > (int)(final_val-i))
                     rep_bytes = (int)(final_val-i);
                  png_memcpy(dstptr, srcptr, rep_bytes);
                  srcptr += stride;
                  dstptr += stride;
               }
            }

            break;
         }       /* end 64 bpp */

         default: /* png_ptr->row_info.pixel_depth != 1,2,4,8,16,24,32,48,64 */
         {
            // ERROR:  SHOULD NEVER BE REACHED
#if defined(PNG_DEBUG)
            png_debug(1, "Internal libpng logic error (GCC "
              "png_combine_row() pixel_depth)\n");
#endif
            break;
         }
      } /* end switch (png_ptr->row_info.pixel_depth) */

   } /* end if (non-trivial mask) */

} /* end png_combine_row() */

#endif /* PNG_HAVE_MMX_COMBINE_ROW */




/*===========================================================================*/
/*                                                                           */
/*                 P N G _ D O _ R E A D _ I N T E R L A C E                 */
/*                                                                           */
/*===========================================================================*/

#if defined(PNG_READ_INTERLACING_SUPPORTED)
#if defined(PNG_HAVE_MMX_READ_INTERLACE)

/* png_do_read_interlace() is called after any 16-bit to 8-bit conversion
 * has taken place.  [GRR: what other steps come before and/or after?]
 */

void /* PRIVATE */
png_do_read_interlace(png_structp png_ptr)
{
   png_row_infop row_info = &(png_ptr->row_info);
   png_bytep row = png_ptr->row_buf + 1;
   int pass = png_ptr->pass;
#if defined(PNG_READ_PACKSWAP_SUPPORTED)
   png_uint_32 transformations = png_ptr->transformations;
#endif

   png_debug(1, "in png_do_read_interlace (pnggccrd.c)\n");

   if (_mmx_supported == 2) {
#if !defined(PNG_1_0_X)
       /* this should have happened in png_init_mmx_flags() already */
       png_warning(png_ptr, "asm_flags may not have been initialized");
#endif
       png_mmx_support();
   }

   if (row != NULL && row_info != NULL)
   {
      png_uint_32 final_width;

      final_width = row_info->width * png_pass_inc[pass];

      switch (row_info->pixel_depth)
      {
         case 1:
         {
            png_bytep sp, dp;
            int sshift, dshift;
            int s_start, s_end, s_inc;
            png_byte v;
            png_uint_32 i;
            int j;

            sp = row + (png_size_t)((row_info->width - 1) >> 3);
            dp = row + (png_size_t)((final_width - 1) >> 3);
#if defined(PNG_READ_PACKSWAP_SUPPORTED)
            if (transformations & PNG_PACKSWAP)
            {
               sshift = (int)((row_info->width + 7) & 7);
               dshift = (int)((final_width + 7) & 7);
               s_start = 7;
               s_end = 0;
               s_inc = -1;
            }
            else
#endif
            {
               sshift = 7 - (int)((row_info->width + 7) & 7);
               dshift = 7 - (int)((final_width + 7) & 7);
               s_start = 0;
               s_end = 7;
               s_inc = 1;
            }

            for (i = row_info->width; i; i--)
            {
               v = (png_byte)((*sp >> sshift) & 0x1);
               for (j = 0; j < png_pass_inc[pass]; j++)
               {
                  *dp &= (png_byte)((0x7f7f >> (7 - dshift)) & 0xff);
                  *dp |= (png_byte)(v << dshift);
                  if (dshift == s_end)
                  {
                     dshift = s_start;
                     dp--;
                  }
                  else
                     dshift += s_inc;
               }
               if (sshift == s_end)
               {
                  sshift = s_start;
                  sp--;
               }
               else
                  sshift += s_inc;
            }
            break;
         }

         case 2:
         {
            png_bytep sp, dp;
            int sshift, dshift;
            int s_start, s_end, s_inc;
            png_uint_32 i;

            sp = row + (png_size_t)((row_info->width - 1) >> 2);
            dp = row + (png_size_t)((final_width - 1) >> 2);
#if defined(PNG_READ_PACKSWAP_SUPPORTED)
            if (transformations & PNG_PACKSWAP)
            {
               sshift = (png_size_t)(((row_info->width + 3) & 3) << 1);
               dshift = (png_size_t)(((final_width + 3) & 3) << 1);
               s_start = 6;
               s_end = 0;
               s_inc = -2;
            }
            else
#endif
            {
               sshift = (png_size_t)((3 - ((row_info->width + 3) & 3)) << 1);
               dshift = (png_size_t)((3 - ((final_width + 3) & 3)) << 1);
               s_start = 0;
               s_end = 6;
               s_inc = 2;
            }

            for (i = row_info->width; i; i--)
            {
               png_byte v;
               int j;

               v = (png_byte)((*sp >> sshift) & 0x3);
               for (j = 0; j < png_pass_inc[pass]; j++)
               {
                  *dp &= (png_byte)((0x3f3f >> (6 - dshift)) & 0xff);
                  *dp |= (png_byte)(v << dshift);
                  if (dshift == s_end)
                  {
                     dshift = s_start;
                     dp--;
                  }
                  else
                     dshift += s_inc;
               }
               if (sshift == s_end)
               {
                  sshift = s_start;
                  sp--;
               }
               else
                  sshift += s_inc;
            }
            break;
         }

         case 4:
         {
            png_bytep sp, dp;
            int sshift, dshift;
            int s_start, s_end, s_inc;
            png_uint_32 i;

            sp = row + (png_size_t)((row_info->width - 1) >> 1);
            dp = row + (png_size_t)((final_width - 1) >> 1);
#if defined(PNG_READ_PACKSWAP_SUPPORTED)
            if (transformations & PNG_PACKSWAP)
            {
               sshift = (png_size_t)(((row_info->width + 1) & 1) << 2);
               dshift = (png_size_t)(((final_width + 1) & 1) << 2);
               s_start = 4;
               s_end = 0;
               s_inc = -4;
            }
            else
#endif
            {
               sshift = (png_size_t)((1 - ((row_info->width + 1) & 1)) << 2);
               dshift = (png_size_t)((1 - ((final_width + 1) & 1)) << 2);
               s_start = 0;
               s_end = 4;
               s_inc = 4;
            }

            for (i = row_info->width; i; i--)
            {
               png_byte v;
               int j;

               v = (png_byte)((*sp >> sshift) & 0xf);
               for (j = 0; j < png_pass_inc[pass]; j++)
               {
                  *dp &= (png_byte)((0xf0f >> (4 - dshift)) & 0xff);
                  *dp |= (png_byte)(v << dshift);
                  if (dshift == s_end)
                  {
                     dshift = s_start;
                     dp--;
                  }
                  else
                     dshift += s_inc;
               }
               if (sshift == s_end)
               {
                  sshift = s_start;
                  sp--;
               }
               else
                  sshift += s_inc;
            }
            break;
         }

       /*====================================================================*/

         default: /* 8-bit or larger (this is where the routine is modified) */
         {
            png_bytep sptr, dp;
            png_uint_32 i;
            png_size_t pixel_bytes;
            int width = (int)row_info->width;

            pixel_bytes = (row_info->pixel_depth >> 3);

            /* point sptr at the last pixel in the pre-expanded row: */
            sptr = row + (width - 1) * pixel_bytes;

            /* point dp at the last pixel position in the expanded row: */
            dp = row + (final_width - 1) * pixel_bytes;

            /* New code by Nirav Chhatrapati - Intel Corporation */

#if !defined(PNG_1_0_X)
            if (png_ptr->asm_flags & PNG_ASM_FLAG_MMX_READ_INTERLACE)
#else
            if (_mmx_supported)
#endif
            {
               int dummy_value_c;        // fix 'forbidden register spilled'
               png_bytep dummy_value_S;
               png_bytep dummy_value_D;
               png_bytep dummy_value_a;
               png_bytep dummy_value_d;

               //--------------------------------------------------------------
               if (pixel_bytes == BPP3)
               {
                  if (((pass == 4) || (pass == 5)) && width)
                  {
                     int width_mmx = ((width >> 1) << 1) - 8;   // GRR:  huh?
                     if (width_mmx < 0)
                         width_mmx = 0;
                     width -= width_mmx;        // 8 or 9 pix, 24 or 27 bytes
                     if (width_mmx)
                     {
                        // png_pass_inc[] = {8, 8, 4, 4, 2, 2, 1};
                        // sptr points at last pixel in pre-expanded row
                        // dp points at last pixel position in expanded row
                        __asm__ __volatile__ (
                           "sub  $3, %1             \n\t"
                           "sub  $9, %2             \n\t"
                                        // (png_pass_inc[pass] + 1)*pixel_bytes

                        ".loop3_pass4:              \n\t"
                           "movq (%1), %%mm0        \n\t" // x x 5 4 3 2 1 0
                           "movq %%mm0, %%mm1       \n\t" // x x 5 4 3 2 1 0
                           "movq %%mm0, %%mm2       \n\t" // x x 5 4 3 2 1 0
                           "psllq $24, %%mm0        \n\t" // 4 3 2 1 0 z z z
                           "pand (%3), %%mm1        \n\t" // z z z z z 2 1 0
                           "psrlq $24, %%mm2        \n\t" // z z z x x 5 4 3
                           "por %%mm1, %%mm0        \n\t" // 4 3 2 1 0 2 1 0
                           "movq %%mm2, %%mm3       \n\t" // z z z x x 5 4 3
                           "psllq $8, %%mm2         \n\t" // z z x x 5 4 3 z
                           "movq %%mm0, (%2)        \n\t"
                           "psrlq $16, %%mm3        \n\t" // z z z z z x x 5
                           "pand (%4), %%mm3        \n\t" // z z z z z z z 5
                           "por %%mm3, %%mm2        \n\t" // z z x x 5 4 3 5
                           "sub  $6, %1             \n\t"
                           "movd %%mm2, 8(%2)       \n\t"
                           "sub  $12, %2            \n\t"
                           "subl $2, %%ecx          \n\t"
                           "jnz .loop3_pass4        \n\t"
                           "EMMS                    \n\t" // DONE

                           : "=c" (dummy_value_c),        // output regs (dummy)
                             "=S" (dummy_value_S),
                             "=D" (dummy_value_D),
                             "=a" (dummy_value_a),
                             "=d" (dummy_value_d)

                           : "0" (width_mmx),     // ecx  // input regs
                             "1" (sptr),          // esi/rsi
                             "2" (dp),            // edi/rdi
#if defined(PNG_x86_64_USE_GOTPCREL)     // formerly _const4 and _const6:
                             "3" (&_c64._amask5_3_0), // (0x0000000000FFFFFFLL)
                             "4" (&_c64._amask7_1_0)  // (0x00000000000000FFLL)
#else
                             "3" (&_amask5_3_0),  // eax (0x0000000000FFFFFFLL)
                             "4" (&_amask7_1_0)   // edx (0x00000000000000FFLL)
#endif

#if defined(PNG_CLOBBER_MMX_REGS_SUPPORTED)
                           : "%mm0", "%mm1"               // clobber list
                           , "%mm2", "%mm3"
#endif
                        );
                     }

                     sptr -= width_mmx*BPP3;
                     dp -= width_mmx*2*BPP3;
                     for (i = width; i; i--)
                     {
                        png_byte v[8];
                        int j;

                        png_memcpy(v, sptr, BPP3);
                        for (j = 0; j < png_pass_inc[pass]; j++)
                        {
                           png_memcpy(dp, v, BPP3);
                           dp -= BPP3;
                        }
                        sptr -= BPP3;
                     }
                  }
                  else if (((pass == 2) || (pass == 3)) && width)
                  {
                     __asm__ __volatile__ (
                        "sub  $9, %2             \n\t"
                                     // (png_pass_inc[pass] - 1)*pixel_bytes

                     ".loop3_pass2:              \n\t"
                        "movd (%1), %%mm0        \n\t" // x x x x x 2 1 0
                        "pand (%3), %%mm0        \n\t" // z z z z z 2 1 0
                        "movq %%mm0, %%mm1       \n\t" // z z z z z 2 1 0
                        "psllq $16, %%mm0        \n\t" // z z z 2 1 0 z z
                        "movq %%mm0, %%mm2       \n\t" // z z z 2 1 0 z z
                        "psllq $24, %%mm0        \n\t" // 2 1 0 z z z z z
                        "psrlq $8, %%mm1         \n\t" // z z z z z z 2 1
                        "por %%mm2, %%mm0        \n\t" // 2 1 0 2 1 0 z z
                        "por %%mm1, %%mm0        \n\t" // 2 1 0 2 1 0 2 1
                        "movq %%mm0, 4(%2)       \n\t"
                        "psrlq $16, %%mm0        \n\t" // z z 2 1 0 2 1 0
                        "sub  $3, %1             \n\t"
                        "movd %%mm0, (%2)        \n\t"
                        "sub  $12, %2            \n\t"
                        "decl %%ecx              \n\t"
                        "jnz .loop3_pass2        \n\t"
                        "EMMS                    \n\t" // DONE

                        : "=c" (dummy_value_c),        // output regs (dummy)
                          "=S" (dummy_value_S),
                          "=D" (dummy_value_D),
                          "=a" (dummy_value_a)

                        : "0" (width),         // ecx  // input regs
                          "1" (sptr),          // esi/rsi
                          "2" (dp),            // edi/rdi
#if defined(PNG_x86_64_USE_GOTPCREL)           // formerly _const4:
                          "3" (&_c64._amask5_3_0)  // (0x0000000000FFFFFFLL)
#else
                          "3" (&_amask5_3_0)   // eax (0x0000000000FFFFFFLL)
#endif

#if defined(CLOBBER_MMX_REGS_SUPPORTED)
                        : "%mm0", "%mm1", "%mm2"       // clobber list
#endif
                     );
                  }
                  else if (width)  // && ((pass == 0) || (pass == 1))
                  {
                     __asm__ __volatile__ (
                        "sub  $21, %2            \n\t"
                                     // (png_pass_inc[pass] - 1)*pixel_bytes

                     ".loop3_pass0:              \n\t"
                        "movd (%1), %%mm0        \n\t" // x x x x x 2 1 0
                        "pand (%3), %%mm0        \n\t" // z z z z z 2 1 0
                        "movq %%mm0, %%mm1       \n\t" // z z z z z 2 1 0
                        "psllq $16, %%mm0        \n\t" // z z z 2 1 0 z z
                        "movq %%mm0, %%mm2       \n\t" // z z z 2 1 0 z z
                        "psllq $24, %%mm0        \n\t" // 2 1 0 z z z z z
                        "psrlq $8, %%mm1         \n\t" // z z z z z z 2 1
                        "por %%mm2, %%mm0        \n\t" // 2 1 0 2 1 0 z z
                        "por %%mm1, %%mm0        \n\t" // 2 1 0 2 1 0 2 1
                        "movq %%mm0, %%mm3       \n\t" // 2 1 0 2 1 0 2 1
                        "psllq $16, %%mm0        \n\t" // 0 2 1 0 2 1 z z
                        "movq %%mm3, %%mm4       \n\t" // 2 1 0 2 1 0 2 1
                        "punpckhdq %%mm0, %%mm3  \n\t" // 0 2 1 0 2 1 0 2
                        "movq %%mm4, 16(%2)      \n\t"
                        "psrlq $32, %%mm0        \n\t" // z z z z 0 2 1 0
                        "movq %%mm3, 8(%2)       \n\t"
                        "punpckldq %%mm4, %%mm0  \n\t" // 1 0 2 1 0 2 1 0
                        "sub  $3, %1             \n\t"
                        "movq %%mm0, (%2)        \n\t"
                        "sub  $24, %2            \n\t"
                        "decl %%ecx              \n\t"
                        "jnz .loop3_pass0        \n\t"
                        "EMMS                    \n\t" // DONE

                        : "=c" (dummy_value_c),        // output regs (dummy)
                          "=S" (dummy_value_S),
                          "=D" (dummy_value_D),
                          "=a" (dummy_value_a)

                        : "0" (width),         // ecx  // input regs
                          "1" (sptr),          // esi/rsi
                          "2" (dp),            // edi/rdi
#if defined(PNG_x86_64_USE_GOTPCREL)           // formerly _const4:
                          "3" (&_c64._amask5_3_0)  // (0x0000000000FFFFFFLL)
#else
                          "3" (&_amask5_3_0)   // eax (0x0000000000FFFFFFLL)
#endif

#if defined(PNG_CLOBBER_MMX_REGS_SUPPORTED)
                        : "%mm0", "%mm1", "%mm2"       // clobber list
                        , "%mm3", "%mm4"
#endif
                     );
                  }
               } /* end of pixel_bytes == 3 */

               //--------------------------------------------------------------
               else if (pixel_bytes == BPP4)
               {
                  if (((pass == 4) || (pass == 5)) && width)
                  {
                     int width_mmx = ((width >> 1) << 1) ;
                     width -= width_mmx;        // 0,1 pixels => 0,4 bytes
                     if (width_mmx)
                     {
                        __asm__ __volatile__ (
                           "sub  $4, %1             \n\t"
                           "sub  $12, %2            \n\t"

                        ".loop4_pass4:              \n\t"
                           "movq (%1), %%mm0        \n\t" // 7 6 5 4 3 2 1 0
                           "movq %%mm0, %%mm1       \n\t" // 7 6 5 4 3 2 1 0
                           "punpckldq %%mm0, %%mm0  \n\t" // 3 2 1 0 3 2 1 0
                           "punpckhdq %%mm1, %%mm1  \n\t" // 7 6 5 4 7 6 5 4
                           "movq %%mm0, (%2)        \n\t"
                           "sub  $8, %1             \n\t"
                           "movq %%mm1, 8(%2)       \n\t"
                           "sub  $16, %2            \n\t"
                           "subl $2, %%ecx          \n\t"
                           "jnz .loop4_pass4        \n\t"
                           "EMMS                    \n\t" // DONE

                           : "=c" (dummy_value_c),        // output regs (dummy)
                             "=S" (dummy_value_S),
                             "=D" (dummy_value_D)

                           : "0" (width_mmx),     // ecx  // input regs
                             "1" (sptr),          // esi/rsi
                             "2" (dp)             // edi/rdi

#if defined(CLOBBER_MMX_REGS_SUPPORTED)
                           : "%mm0", "%mm1"               // clobber list
#endif
                        );
                     }

                     sptr -= (width_mmx*BPP4 - BPP4); // sign fixed
                     dp -= (width_mmx*2*BPP4 - BPP4); // sign fixed
                     for (i = width; i; i--)
                     {
                        png_byte v[8];
                        int j;
                        sptr -= BPP4;
                        png_memcpy(v, sptr, BPP4);
                        for (j = 0; j < png_pass_inc[pass]; j++)
                        {
                           dp -= BPP4;
                           png_memcpy(dp, v, BPP4);
                        }
                     }
                  }
                  else if (((pass == 2) || (pass == 3)) && width)
                  {
                     int width_mmx = ((width >> 1) << 1);
                     width -= width_mmx;        // 0,1 pixels => 0,4 bytes
                     if (width_mmx)
                     {
                        __asm__ __volatile__ (
                           "sub  $4, %1             \n\t"
                           "sub  $28, %2            \n\t"

                        ".loop4_pass2:              \n\t"
                           "movq (%1), %%mm0        \n\t" // 7 6 5 4 3 2 1 0
                           "movq %%mm0, %%mm1       \n\t" // 7 6 5 4 3 2 1 0
                           "punpckldq %%mm0, %%mm0  \n\t" // 3 2 1 0 3 2 1 0
                           "punpckhdq %%mm1, %%mm1  \n\t" // 7 6 5 4 7 6 5 4
                           "movq %%mm0, (%2)        \n\t"
                           "movq %%mm0, 8(%2)       \n\t"
                           "movq %%mm1, 16(%2)      \n\t"
                           "movq %%mm1, 24(%2)      \n\t"
                           "sub  $8, %1             \n\t"
                           "sub  $32, %2            \n\t"
                           "subl $2, %%ecx          \n\t"
                           "jnz .loop4_pass2        \n\t"
                           "EMMS                    \n\t" // DONE

                           : "=c" (dummy_value_c),        // output regs (dummy)
                             "=S" (dummy_value_S),
                             "=D" (dummy_value_D)

                           : "0" (width_mmx),     // ecx  // input regs
                             "1" (sptr),          // esi/rsi
                             "2" (dp)             // edi/rdi

#if defined(CLOBBER_MMX_REGS_SUPPORTED)
                           : "%mm0", "%mm1"               // clobber list
#endif
                        );
                     }

                     sptr -= (width_mmx*4 - 4); // sign fixed
                     dp -= (width_mmx*16 - 4);  // sign fixed
                     for (i = width; i; i--)
                     {
                        png_byte v[8];
                        int j;
                        sptr -= 4;
                        png_memcpy(v, sptr, 4);
                        for (j = 0; j < png_pass_inc[pass]; j++)
                        {
                           dp -= 4;
                           png_memcpy(dp, v, 4);
                        }
                     }
                  }
                  else if (width)  // && ((pass == 0) || (pass == 1))
                  {
                     int width_mmx = ((width >> 1) << 1);
                     width -= width_mmx;        // 0,1 pixels => 0,4 bytes
                     if (width_mmx)
                     {
                        __asm__ __volatile__ (
                           "sub  $4, %1             \n\t"
                           "sub  $60, %2            \n\t"

                        ".loop4_pass0:              \n\t"
                           "movq (%1), %%mm0        \n\t" // 7 6 5 4 3 2 1 0
                           "movq %%mm0, %%mm1       \n\t" // 7 6 5 4 3 2 1 0
                           "punpckldq %%mm0, %%mm0  \n\t" // 3 2 1 0 3 2 1 0
                           "punpckhdq %%mm1, %%mm1  \n\t" // 7 6 5 4 7 6 5 4
                           "movq %%mm0, (%2)        \n\t"
                           "movq %%mm0, 8(%2)       \n\t"
                           "movq %%mm0, 16(%2)      \n\t"
                           "movq %%mm0, 24(%2)      \n\t"
                           "movq %%mm1, 32(%2)      \n\t"
                           "movq %%mm1, 40(%2)      \n\t"
                           "movq %%mm1, 48(%2)      \n\t"
                           "sub  $8, %1             \n\t"
                           "movq %%mm1, 56(%2)      \n\t"
                           "sub  $64, %2            \n\t"
                           "subl $2, %%ecx          \n\t"
                           "jnz .loop4_pass0        \n\t"
                           "EMMS                    \n\t" // DONE

                           : "=c" (dummy_value_c),        // output regs (dummy)
                             "=S" (dummy_value_S),
                             "=D" (dummy_value_D)

                           : "0" (width_mmx),     // ecx  // input regs
                             "1" (sptr),          // esi/rsi
                             "2" (dp)             // edi/rdi

#if defined(PNG_CLOBBER_MMX_REGS_SUPPORTED)
                           : "%mm0", "%mm1"               // clobber list
#endif
                        );
                     }

                     sptr -= (width_mmx*4 - 4); // sign fixed
                     dp -= (width_mmx*32 - 4);  // sign fixed
                     for (i = width; i; i--)
                     {
                        png_byte v[8];
                        int j;
                        sptr -= 4;
                        png_memcpy(v, sptr, 4);
                        for (j = 0; j < png_pass_inc[pass]; j++)
                        {
                           dp -= 4;
                           png_memcpy(dp, v, 4);
                        }
                     }
                  }
               } /* end of pixel_bytes == 4 */

               //--------------------------------------------------------------
               else if (pixel_bytes == 1)
               {
                  if (((pass == 4) || (pass == 5)) && width)
                  {
                     int width_mmx = ((width >> 3) << 3);
                     width -= width_mmx;        // 0-3 pixels => 0-3 bytes
                     if (width_mmx)
                     {
                        __asm__ __volatile__ (
                           "sub  $7, %1             \n\t"
                           "sub  $15, %2            \n\t"

                        ".loop1_pass4:              \n\t"
                           "movq (%1), %%mm0        \n\t" // 7 6 5 4 3 2 1 0
                           "movq %%mm0, %%mm1       \n\t" // 7 6 5 4 3 2 1 0
                           "punpcklbw %%mm0, %%mm0  \n\t" // 3 3 2 2 1 1 0 0
                           "punpckhbw %%mm1, %%mm1  \n\t" // 7 7 6 6 5 5 4 4
                           "movq %%mm1, 8(%2)       \n\t"
                           "sub  $8, %1             \n\t"
                           "movq %%mm0, (%2)        \n\t"
                           "sub  $16, %2            \n\t"
                           "subl $8, %%ecx          \n\t"
                           "jnz .loop1_pass4        \n\t"
                           "EMMS                    \n\t" // DONE

                           : "=c" (dummy_value_c),        // output regs (dummy)
                             "=S" (dummy_value_S),
                             "=D" (dummy_value_D)

                           : "0" (width_mmx),     // ecx  // input regs
                             "1" (sptr),          // esi/rsi
                             "2" (dp)             // edi/rdi

#if defined(PNG_CLOBBER_MMX_REGS_SUPPORTED)
                           : "%mm0", "%mm1"               // clobber list
#endif
                        );
                     }

                     sptr -= width_mmx;
                     dp -= width_mmx*2;
                     for (i = width; i; i--)
                     {
                        int j;

                        for (j = 0; j < png_pass_inc[pass]; j++)
                        {
                           *dp-- = *sptr;
                        }
                        --sptr;
                     }
                  }
                  else if (((pass == 2) || (pass == 3)) && width)
                  {
                     int width_mmx = ((width >> 2) << 2);
                     width -= width_mmx;        // 0-3 pixels => 0-3 bytes
                     if (width_mmx)
                     {
                        __asm__ __volatile__ (
                           "sub  $3, %1             \n\t"
                           "sub  $15, %2            \n\t"

                        ".loop1_pass2:              \n\t"
                           "movd (%1), %%mm0        \n\t" // x x x x 3 2 1 0
                           "punpcklbw %%mm0, %%mm0  \n\t" // 3 3 2 2 1 1 0 0
                           "movq %%mm0, %%mm1       \n\t" // 3 3 2 2 1 1 0 0
                           "punpcklwd %%mm0, %%mm0  \n\t" // 1 1 1 1 0 0 0 0
                           "punpckhwd %%mm1, %%mm1  \n\t" // 3 3 3 3 2 2 2 2
                           "movq %%mm0, (%2)        \n\t"
                           "sub  $4, %1             \n\t"
                           "movq %%mm1, 8(%2)       \n\t"
                           "sub  $16, %2            \n\t"
                           "subl $4, %%ecx          \n\t"
                           "jnz .loop1_pass2        \n\t"
                           "EMMS                    \n\t" // DONE

                           : "=c" (dummy_value_c),        // output regs (dummy)
                             "=S" (dummy_value_S),
                             "=D" (dummy_value_D)

                           : "0" (width_mmx),     // ecx  // input regs
                             "1" (sptr),          // esi/rsi
                             "2" (dp)             // edi/rdi

#if defined(PNG_CLOBBER_MMX_REGS_SUPPORTED)
                           : "%mm0", "%mm1"               // clobber list
#endif
                        );
                     }

                     sptr -= width_mmx;
                     dp -= width_mmx*4;
                     for (i = width; i; i--)
                     {
                        int j;

                        for (j = 0; j < png_pass_inc[pass]; j++)
                        {
                           *dp-- = *sptr;
                        }
                        --sptr;
                     }
                  }
                  else if (width)  // && ((pass == 0) || (pass == 1))
                  {
                     int width_mmx = ((width >> 2) << 2);
                     width -= width_mmx;        // 0-3 pixels => 0-3 bytes
                     if (width_mmx)
                     {
                        __asm__ __volatile__ (
                           "sub  $3, %1             \n\t"
                           "sub  $31, %2            \n\t"

                        ".loop1_pass0:              \n\t"
                           "movd (%1), %%mm0        \n\t" // x x x x 3 2 1 0
                           "movq %%mm0, %%mm1       \n\t" // x x x x 3 2 1 0
                           "punpcklbw %%mm0, %%mm0  \n\t" // 3 3 2 2 1 1 0 0
                           "movq %%mm0, %%mm2       \n\t" // 3 3 2 2 1 1 0 0
                           "punpcklwd %%mm0, %%mm0  \n\t" // 1 1 1 1 0 0 0 0
                           "movq %%mm0, %%mm3       \n\t" // 1 1 1 1 0 0 0 0
                           "punpckldq %%mm0, %%mm0  \n\t" // 0 0 0 0 0 0 0 0
                           "punpckhdq %%mm3, %%mm3  \n\t" // 1 1 1 1 1 1 1 1
                           "movq %%mm0, (%2)        \n\t"
                           "punpckhwd %%mm2, %%mm2  \n\t" // 3 3 3 3 2 2 2 2
                           "movq %%mm3, 8(%2)       \n\t"
                           "movq %%mm2, %%mm4       \n\t" // 3 3 3 3 2 2 2 2
                           "punpckldq %%mm2, %%mm2  \n\t" // 2 2 2 2 2 2 2 2
                           "punpckhdq %%mm4, %%mm4  \n\t" // 3 3 3 3 3 3 3 3
                           "movq %%mm2, 16(%2)      \n\t"
                           "sub  $4, %1             \n\t"
                           "movq %%mm4, 24(%2)      \n\t"
                           "sub  $32, %2            \n\t"
                           "subl $4, %%ecx          \n\t"
                           "jnz .loop1_pass0        \n\t"
                           "EMMS                    \n\t" // DONE

                           : "=c" (dummy_value_c),        // output regs (dummy)
                             "=S" (dummy_value_S),
                             "=D" (dummy_value_D)

                           : "0" (width_mmx),     // ecx  // input regs
                             "1" (sptr),          // esi/rsi
                             "2" (dp)             // edi/rdi

#if defined(PNG_CLOBBER_MMX_REGS_SUPPORTED)
                           : "%mm0", "%mm1", "%mm2"       // clobber list
                           , "%mm3", "%mm4"
#endif
                        );
                     }

                     sptr -= width_mmx;
                     dp -= width_mmx*8;
                     for (i = width; i; i--)
                     {
                        int j;

                       /* I simplified this part in version 1.0.4e
                        * here and in several other instances where
                        * pixel_bytes == 1  -- GR-P
                        *
                        * Original code:
                        *
                        * png_byte v[8];
                        * png_memcpy(v, sptr, pixel_bytes);
                        * for (j = 0; j < png_pass_inc[pass]; j++)
                        * {
                        *    png_memcpy(dp, v, pixel_bytes);
                        *    dp -= pixel_bytes;
                        * }
                        * sptr -= pixel_bytes;
                        *
                        * Replacement code is in the next three lines:
                        */

                        for (j = 0; j < png_pass_inc[pass]; j++)
                        {
                           *dp-- = *sptr;
                        }
                        --sptr;
                     }
                  }
               } /* end of pixel_bytes == 1 */

               //--------------------------------------------------------------
               else if (pixel_bytes == BPP2)
               {
                  if (((pass == 4) || (pass == 5)) && width)
                  {
                     int width_mmx = ((width >> 1) << 1) ;
                     width -= width_mmx;        // 0,1 pixels => 0,2 bytes
                     if (width_mmx)
                     {
                        __asm__ __volatile__ (
                           "sub  $2, %1             \n\t"
                           "sub  $6, %2             \n\t"

                        ".loop2_pass4:              \n\t"
                           "movd (%1), %%mm0        \n\t" // x x x x 3 2 1 0
                           "punpcklwd %%mm0, %%mm0  \n\t" // 3 2 3 2 1 0 1 0
                           "sub  $4, %1             \n\t"
                           "movq %%mm0, (%2)        \n\t"
                           "sub  $8, %2             \n\t"
                           "subl $2, %%ecx          \n\t"
                           "jnz .loop2_pass4        \n\t"
                           "EMMS                    \n\t" // DONE

                           : "=c" (dummy_value_c),        // output regs (dummy)
                             "=S" (dummy_value_S),
                             "=D" (dummy_value_D)

                           : "0" (width_mmx),     // ecx  // input regs
                             "1" (sptr),          // esi/rsi
                             "2" (dp)             // edi/rdi

#if defined(PNG_CLOBBER_MMX_REGS_SUPPORTED)
                           : "%mm0"                       // clobber list
#endif
                        );
                     }

                     sptr -= (width_mmx*BPP2 - BPP2); // sign fixed
                     dp -= (width_mmx*2*BPP2 - BPP2); // sign fixed
                     for (i = width; i; i--)
                     {
                        png_byte v[8];
                        int j;
                        sptr -= BPP2;
                        png_memcpy(v, sptr, BPP2);
                        for (j = 0; j < png_pass_inc[pass]; j++)
                        {
                           dp -= BPP2;
                           png_memcpy(dp, v, BPP2);
                        }
                     }
                  }
                  else if (((pass == 2) || (pass == 3)) && width)
                  {
                     int width_mmx = ((width >> 1) << 1) ;
                     width -= width_mmx;        // 0,1 pixels => 0,2 bytes
                     if (width_mmx)
                     {
                        __asm__ __volatile__ (
                           "sub  $2, %1             \n\t"
                           "sub  $14, %2            \n\t"

                        ".loop2_pass2:              \n\t"
                           "movd (%1), %%mm0        \n\t" // x x x x 3 2 1 0
                           "punpcklwd %%mm0, %%mm0  \n\t" // 3 2 3 2 1 0 1 0
                           "movq %%mm0, %%mm1       \n\t" // 3 2 3 2 1 0 1 0
                           "punpckldq %%mm0, %%mm0  \n\t" // 1 0 1 0 1 0 1 0
                           "punpckhdq %%mm1, %%mm1  \n\t" // 3 2 3 2 3 2 3 2
                           "movq %%mm0, (%2)        \n\t"
                           "sub  $4, %1             \n\t"
                           "movq %%mm1, 8(%2)       \n\t"
                           "sub  $16, %2            \n\t"
                           "subl $2, %%ecx          \n\t"
                           "jnz .loop2_pass2        \n\t"
                           "EMMS                    \n\t" // DONE

                           : "=c" (dummy_value_c),        // output regs (dummy)
                             "=S" (dummy_value_S),
                             "=D" (dummy_value_D)

                           : "0" (width_mmx),     // ecx  // input regs
                             "1" (sptr),          // esi/rsi
                             "2" (dp)             // edi/rdi

#if defined(CLOBBER_MMX_REGS_SUPPORTED)
                           : "%mm0", "%mm1"               // clobber list
#endif
                        );
                     }

                     sptr -= (width_mmx*2 - 2); // sign fixed
                     dp -= (width_mmx*8 - 2);   // sign fixed
                     for (i = width; i; i--)
                     {
                        png_byte v[8];
                        int j;
                        sptr -= 2;
                        png_memcpy(v, sptr, 2);
                        for (j = 0; j < png_pass_inc[pass]; j++)
                        {
                           dp -= 2;
                           png_memcpy(dp, v, 2);
                        }
                     }
                  }
                  else if (width)  // && ((pass == 0) || (pass == 1))
                  {
                     int width_mmx = ((width >> 1) << 1);
                     width -= width_mmx;        // 0,1 pixels => 0,2 bytes
                     if (width_mmx)
                     {
                        __asm__ __volatile__ (
                           "sub  $2, %1             \n\t"
                           "sub  $30, %2            \n\t"

                        ".loop2_pass0:              \n\t"
                           "movd (%1), %%mm0        \n\t" // x x x x 3 2 1 0
                           "punpcklwd %%mm0, %%mm0  \n\t" // 3 2 3 2 1 0 1 0
                           "movq %%mm0, %%mm1       \n\t" // 3 2 3 2 1 0 1 0
                           "punpckldq %%mm0, %%mm0  \n\t" // 1 0 1 0 1 0 1 0
                           "punpckhdq %%mm1, %%mm1  \n\t" // 3 2 3 2 3 2 3 2
                           "movq %%mm0, (%2)        \n\t"
                           "movq %%mm0, 8(%2)       \n\t"
                           "movq %%mm1, 16(%2)      \n\t"
                           "sub  $4, %1             \n\t"
                           "movq %%mm1, 24(%2)      \n\t"
                           "sub  $32, %2            \n\t"
                           "subl $2, %%ecx          \n\t"
                           "jnz .loop2_pass0        \n\t"
                           "EMMS                    \n\t" // DONE

                           : "=c" (dummy_value_c),        // output regs (dummy)
                             "=S" (dummy_value_S),
                             "=D" (dummy_value_D)

                           : "0" (width_mmx),     // ecx  // input regs
                             "1" (sptr),          // esi/rsi
                             "2" (dp)             // edi/rdi

#if defined(CLOBBER_MMX_REGS_SUPPORTED)
                           : "%mm0", "%mm1"               // clobber list
#endif
                        );
                     }

                     sptr -= (width_mmx*2 - 2); // sign fixed
                     dp -= (width_mmx*16 - 2);  // sign fixed
                     for (i = width; i; i--)
                     {
                        png_byte v[8];
                        int j;
                        sptr -= 2;
                        png_memcpy(v, sptr, 2);
                        for (j = 0; j < png_pass_inc[pass]; j++)
                        {
                           dp -= 2;
                           png_memcpy(dp, v, 2);
                        }
                     }
                  }
               } /* end of pixel_bytes == 2 */

               //--------------------------------------------------------------
               else if (pixel_bytes == BPP8)
               {
// GRR TEST:  should work, but needs testing (special 64-bit version of rpng2?)
                  // GRR NOTE:  no need to combine passes here!
                  if (((pass == 4) || (pass == 5)) && width)
                  {
                     // source is 8-byte RRGGBBAA
                     // dest is 16-byte RRGGBBAA RRGGBBAA
                     __asm__ __volatile__ (
                        "sub  $8, %2             \n\t" // start of last block

                     ".loop8_pass4:              \n\t"
                        "movq (%1), %%mm0        \n\t" // 7 6 5 4 3 2 1 0
                        "movq %%mm0, (%2)        \n\t"
                        "sub  $8, %1             \n\t"
                        "movq %%mm0, 8(%2)       \n\t"
                        "sub  $16, %2            \n\t"
                        "decl %%ecx              \n\t"
                        "jnz .loop8_pass4        \n\t"
                        "EMMS                    \n\t" // DONE

                        : "=c" (dummy_value_c),        // output regs (dummy)
                          "=S" (dummy_value_S),
                          "=D" (dummy_value_D)

                        : "0" (width),         // ecx  // input regs
                          "1" (sptr),          // esi/rsi
                          "2" (dp)             // edi/rdi

#if defined(CLOBBER_MMX_REGS_SUPPORTED)
                        : "%mm0"                       // clobber list
#endif
                     );
                  }
                  else if (((pass == 2) || (pass == 3)) && width)
                  {
                     // source is 8-byte RRGGBBAA
                     // dest is 32-byte RRGGBBAA RRGGBBAA RRGGBBAA RRGGBBAA
                     // (recall that expansion is _in place_:  sptr and dp
                     //  both point at locations within same row buffer)
                     __asm__ __volatile__ (
                        "sub  $24, %2            \n\t" // start of last block

                     ".loop8_pass2:              \n\t"
                        "movq (%1), %%mm0        \n\t" // 7 6 5 4 3 2 1 0
                        "movq %%mm0, (%2)        \n\t"
                        "movq %%mm0, 8(%2)       \n\t"
                        "movq %%mm0, 16(%2)      \n\t"
                        "sub  $8, %1             \n\t"
                        "movq %%mm0, 24(%2)      \n\t"
                        "sub  $32, %2            \n\t"
                        "decl %%ecx              \n\t"
                        "jnz .loop8_pass2        \n\t"
                        "EMMS                    \n\t" // DONE

                        : "=c" (dummy_value_c),        // output regs (dummy)
                          "=S" (dummy_value_S),
                          "=D" (dummy_value_D)

                        : "0" (width),         // ecx  // input regs
                          "1" (sptr),          // esi/rsi
                          "2" (dp)             // edi/rdi

#if defined(PNG_CLOBBER_MMX_REGS_SUPPORTED)
                        : "%mm0"                       // clobber list
#endif
                     );
                  }
                  else if (width)  // && ((pass == 0) || (pass == 1))
                  {
                     // source is 8-byte RRGGBBAA
                     // dest is 64-byte RRGGBBAA RRGGBBAA RRGGBBAA RRGGBBAA ...
                     __asm__ __volatile__ (
                        "sub  $56, %2            \n\t" // start of last block

                     ".loop8_pass0:              \n\t"
                        "movq (%1), %%mm0        \n\t" // 7 6 5 4 3 2 1 0
                        "movq %%mm0, (%2)        \n\t"
                        "movq %%mm0, 8(%2)       \n\t"
                        "movq %%mm0, 16(%2)      \n\t"
                        "movq %%mm0, 24(%2)      \n\t"
                        "movq %%mm0, 32(%2)      \n\t"
                        "movq %%mm0, 40(%2)      \n\t"
                        "movq %%mm0, 48(%2)      \n\t"
                        "sub  $8, %1             \n\t"
                        "movq %%mm0, 56(%2)      \n\t"
                        "sub  $64, %2            \n\t"
                        "decl %%ecx              \n\t"
                        "jnz .loop8_pass0        \n\t"
                        "EMMS                    \n\t" // DONE

                        : "=c" (dummy_value_c),        // output regs (dummy)
                          "=S" (dummy_value_S),
                          "=D" (dummy_value_D)

                        : "0" (width),         // ecx  // input regs
                          "1" (sptr),          // esi/rsi
                          "2" (dp)             // edi/rdi

#if defined(PNG_CLOBBER_MMX_REGS_SUPPORTED)
                        : "%mm0"                       // clobber list
#endif
                     );
                  }
               } /* end of pixel_bytes == 8 */

               //--------------------------------------------------------------
               else if (pixel_bytes == BPP6)   // why no MMX for this case?
               {
                  for (i = width; i; i--)
                  {
                     png_byte v[8];
                     int j;
                     png_memcpy(v, sptr, BPP6);
                     for (j = 0; j < png_pass_inc[pass]; j++)
                     {
                        png_memcpy(dp, v, BPP6);
                        dp -= BPP6;
                     }
                     sptr -= BPP6;
                  }
               } /* end of pixel_bytes == 6 */

               //--------------------------------------------------------------
               else
               {
                  // ERROR:  SHOULD NEVER BE REACHED
#if defined(PNG_DEBUG)
                  png_debug(1, "Internal libpng logic error (GCC "
                    "png_do_read_interlace() _mmx_supported)\n");
#endif
               }

            } // end of _mmx_supported ========================================

            else /* MMX not supported:  use modified C code - takes advantage
                  *   of inlining of png_memcpy for a constant */
            {
               if (pixel_bytes == BPP3)
               {
                  for (i = width; i; i--)
                  {
                     png_byte v[8];
                     int j;
                     png_memcpy(v, sptr, BPP3);
                     for (j = 0; j < png_pass_inc[pass]; j++)
                     {
                        png_memcpy(dp, v, BPP3);
                        dp -= BPP3;
                     }
                     sptr -= BPP3;
                  }
               }
               else if (pixel_bytes == BPP4)
               {
                  for (i = width; i; i--)
                  {
                     png_byte v[8];
                     int j;
                     png_memcpy(v, sptr, BPP4);
                     for (j = 0; j < png_pass_inc[pass]; j++)
                     {
#if defined(PNG_DEBUG) && defined(PNG_1_0_X)  // row_buf_size gone in 1.2.x
                        if (dp < row || dp+3 > row+png_ptr->row_buf_size)
                        {
                           printf("dp out of bounds: row=%10p, dp=%10p, "
                             "rp=%10p\n", row, dp, row+png_ptr->row_buf_size);
                           printf("row_buf_size=%lu\n", png_ptr->row_buf_size);
                        }
#endif
                        png_memcpy(dp, v, BPP4);
                        dp -= BPP4;
                     }
                     sptr -= BPP4;
                  }
               }
               else if (pixel_bytes == 1)
               {
                  for (i = width; i; i--)
                  {
                     int j;
                     for (j = 0; j < png_pass_inc[pass]; j++)
                     {
                        *dp-- = *sptr;
                     }
                     --sptr;
                  }
               }
               else if (pixel_bytes == BPP2)
               {
                  for (i = width; i; i--)
                  {
                     png_byte v[8];
                     int j;
                     png_memcpy(v, sptr, BPP2);
                     for (j = 0; j < png_pass_inc[pass]; j++)
                     {
                        png_memcpy(dp, v, BPP2);
                        dp -= BPP2;
                     }
                     sptr -= BPP2;
                  }
               }
               else if (pixel_bytes == BPP6)
               {
                  for (i = width; i; i--)
                  {
                     png_byte v[8];
                     int j;
                     png_memcpy(v, sptr, BPP6);
                     for (j = 0; j < png_pass_inc[pass]; j++)
                     {
                        png_memcpy(dp, v, BPP6);
                        dp -= BPP6;
                     }
                     sptr -= BPP6;
                  }
               }
               else if (pixel_bytes == BPP8)
               {
                  for (i = width; i; i--)
                  {
                     png_byte v[8];
                     int j;
                     png_memcpy(v, sptr, BPP8);
                     for (j = 0; j < png_pass_inc[pass]; j++)
                     {
                        png_memcpy(dp, v, BPP8);
                        dp -= BPP8;
                     }
                     sptr -= BPP8;
                  }
               }
               else
               {
                  // ERROR:  SHOULD NEVER BE REACHED
#if defined(PNG_DEBUG)
                  png_debug(1, "Internal libpng logic error (GCC "
                    "png_do_read_interlace() !_mmx_supported)\n");
#endif
               }

            } /* end if (MMX not supported) */
            break;
         } /* end default (8-bit or larger) */
      } /* end switch (row_info->pixel_depth) */

      row_info->width = final_width;

      row_info->rowbytes = PNG_ROWBYTES(row_info->pixel_depth,final_width);
   }

} /* end png_do_read_interlace() */

#endif /* PNG_HAVE_MMX_READ_INTERLACE */
#endif /* PNG_READ_INTERLACING_SUPPORTED */



#if defined(PNG_HAVE_MMX_READ_FILTER_ROW)
#if defined(PNG_MMX_READ_FILTER_AVG_SUPPORTED)

//===========================================================================//
//                                                                           //
//           P N G _ R E A D _ F I L T E R _ R O W _ M M X _ A V G           //
//                                                                           //
//===========================================================================//

// Optimized code for PNG Average filter decoder

static void /* PRIVATE */
png_read_filter_row_mmx_avg(png_row_infop row_info, png_bytep row,
                            png_bytep prev_row)
{
   unsigned FullLength, MMXLength;  // png_uint_32 is actually 64-bit on x86-64
   int bpp;
   int dummy_value_a;
   int dummy_value_c;   // fix 'forbidden register 2 (cx) was spilled' error
   int dummy_value_d;
   png_bytep dummy_value_S;
   png_bytep dummy_value_D;
   int diff; //     __attribute__((used));

   bpp = (row_info->pixel_depth + 7) >> 3;  // calc number of bytes per pixel
   FullLength = row_info->rowbytes;         // number of bytes to filter

   __asm__ __volatile__ (
   "avg_top:                       \n\t"
      SAVE_GOT_ebx
      SAVE_r15
      SAVE_ebp
      // initialize address pointers and offset
//pre "movl row, %5                \n\t" // edi/rdi:  ptr to Avg(x)
      "xorl %%ebx, %%ebx           \n\t" // ebx:  x
//pre "movl prev_row, %4           \n\t" // esi/rsi:  ptr to Prior(x)
      "mov  %5, " PDX "            \n\t" // copy of row ptr...
//pre "subl bpp, " PDX "           \n\t" // (bpp is preloaded into ecx)
      "sub  " PCX "," PDX "        \n\t" // edx/rdx:  ptr to Raw(x-bpp)
//pre "movl FullLength, %%eax      \n\t" // bring in via eax...
      SAVE_FullLength                    // ...but store for later use
      "xorl %%eax, %%eax           \n\t"

      // Compute the Raw value for the first bpp bytes
      //    Raw(x) = Avg(x) + (Prior(x)/2)
   "avg_rlp:                       \n\t"
      "movb (%4," PBX ",), %%al    \n\t" // load al with Prior(x)
      "incl %%ebx                  \n\t"
      "shrb %%al                   \n\t" // divide by 2
      "addb -1(%5," PBX ",), %%al  \n\t" // add Avg(x); -1 to offset inc ebx
//pre "cmpl bpp, %%ebx             \n\t" // (bpp is preloaded into ecx)
      "cmpl %%ecx, %%ebx           \n\t"
      "movb %%al, -1(%5," PBX ",)  \n\t" // write Raw(x); -1 to offset inc ebx
      "jb avg_rlp                  \n\t" // mov does not affect flags

      // get # of bytes to alignment (32-bit mask _would_ be good enough
      // [computing delta], but 32-bit ops are zero-extended on 64-bit, argh)
      // (if swapped edx and ebp, could do 8-bit or 16-bit mask...FIXME?)
      "mov  %5, " PBP "            \n\t" // take start of row
      "add  " PBX "," PBP "        \n\t" // add bpp
      "add  $0xf, " PBP "          \n\t" // add 7+8 to incr past alignment bdry
//    "andl $0xfffffff8, %%ebp     \n\t" // mask to alignment boundary (32-bit!)
      CLEAR_BOTTOM_3_BITS  PBP    "\n\t" // mask to alignment boundary
      "sub  %5, " PBP "            \n\t" // subtract row ptr again => ebp =
      "jz avg_go                   \n\t" //  target value of ebx at alignment

      "xorl %%ecx, %%ecx           \n\t"

      // fix alignment
      // Compute the Raw value for the bytes up to the alignment boundary
      //    Raw(x) = Avg(x) + ((Raw(x-bpp) + Prior(x))/2)
   "avg_lp1:                       \n\t"
      "xorl %%eax, %%eax           \n\t"
      "movb (%4," PBX ",), %%cl    \n\t" // load cl with Prior(x)
      "movb (" PDX "," PBX ",), %%al \n\t" // load al with Raw(x-bpp)
      "addw %%cx, %%ax             \n\t"
      "incl %%ebx                  \n\t"
      "shrw %%ax                   \n\t" // divide by 2
      "addb -1(%5," PBX ",), %%al  \n\t" // add Avg(x); -1 to offset inc ebx
      "cmpl %%ebp, %%ebx           \n\t" // check if at alignment boundary
      "movb %%al, -1(%5," PBX ",)  \n\t" // write Raw(x); -1 to offset inc ebx
      "jb avg_lp1                  \n\t" // repeat until at alignment boundary

   "avg_go:                        \n\t"
      RESTORE_FullLength "%%eax    \n\t" // FullLength -> eax
      "movl %%eax, %%ecx           \n\t" // copy -> ecx
      "subl %%ebx, %%eax           \n\t" // subtract alignment fix
      "andl $0x00000007, %%eax     \n\t" // calc bytes over mult of 8
      "subl %%eax, %%ecx           \n\t" // sub over-bytes from original length
//out "movl %%ecx, MMXLength       \n\t"
      "movl %%ebp, %%eax           \n\t" // ebp = diff, but no reg constraint(?)
      RESTORE_ebp                        //  (could swap ebp and edx functions)
      RESTORE_r15
      RESTORE_GOT_ebx

// "There is no way for you to specify that an input operand is modified
// without also specifying it as an output operand."  [makes sense]

// "Unless an output operand has the `&' constraint modifier, GCC may
// allocate it in the same register as an unrelated input operand, on the
// assumption the inputs are consumed before the outputs are produced."
// [trying to _force_ this]

// "`='   Means that this operand is write-only for this instruction:
//        the previous value is discarded and replaced by output data."
//        [operand == variable name, presumably]

      // output regs
      // these are operands 0-1 (originally 0-3):
      : "=c" (MMXLength),      // %0 -> %0
        "=a" (diff)            // %3 -> %1
//      "=S" (dummy_value_S),  // %1 -> GONE
//      "=D" (dummy_value_D),  // %2 -> GONE

      // input regs
      // these are operands 2-5 (originally 4-7); two of their constraints say
      // they must go in same places as operands 0-1 (originally 0-3) above:
      : "0" (bpp),         // %4 -> %2 ecx
        "1" (FullLength),  // %7 -> %3 eax
        "S" (prev_row),    // %5 -> %4 esi/rsi
        "D" (row)          // %6 -> %5 edi/rdi

      : "%edx"                           // clobber list
        _CLOBBER_r15
        _CLOBBER_ebp
        _CLOBBER_GOT_ebx
   );

   // now do the math for the rest of the row
   switch (bpp)
   {
      case 3:
      {
//       _ShiftBpp = 24;    // == 3 * 8
//       _ShiftRem = 40;    // == 64 - 24

         __asm__ __volatile__ (
            // re-init address pointers and offset
            LOAD_GOT_rbp
            "movq " AMASK5_3_0 ", %%mm7    \n\t" // _amask5_3_0 -> mm7
// preload  "movl  diff, %%ecx             \n\t" // ecx:  x = offset to
                                                 //  alignment boundary
            "movq " LB_CARRY_MASK ", %%mm5 \n\t" // [interleave for parallel.?]
// preload  "movl  row, %1                 \n\t" // edi:  Avg(x)
            "movq " HB_CLEAR_MASK ", %%mm4 \n\t" // _HBClearMask -> mm4
// preload  "movl  prev_row, %0            \n\t" // esi:  Prior(x)
            RESTORE_rbp

            // prime the pump:  load the first Raw(x-bpp) data set
            "movq  -8(%1," PCX ",), %%mm2 \n\t"// load previous aligned 8 bytes
                                               // (correct pos. in loop below)
         "avg_3lp:                        \n\t"
            "movq  (%1," PCX ",), %%mm0   \n\t" // load mm0 with Avg(x)
            "movq  %%mm5, %%mm3           \n\t"
            "psrlq $40, %%mm2             \n\t" // correct position Raw(x-bpp)
                                                // data
            "movq  (%0," PCX ",), %%mm1   \n\t" // load mm1 with Prior(x)
            "movq  %%mm7, %%mm6           \n\t"
            "pand  %%mm1, %%mm3           \n\t" // get lsb for each prevrow byte
            "psrlq $1, %%mm1              \n\t" // divide prev_row bytes by 2
            "pand  %%mm4, %%mm1           \n\t" // clear invalid bit 7 of each
                                                // byte
            "paddb %%mm1, %%mm0           \n\t" // add (Prev_row/2) to Avg for
                                                // each byte
            // add 1st active group (Raw(x-bpp)/2) to average with LBCarry
            "movq  %%mm3, %%mm1           \n\t" // now use mm1 for getting
                                                // LBCarrys
            "pand  %%mm2, %%mm1           \n\t" // get LBCarrys for each byte
                                                // where both lsb's were == 1
                                                // (valid only for active group)
            "psrlq $1, %%mm2              \n\t" // divide raw bytes by 2
            "pand  %%mm4, %%mm2           \n\t" // clear invalid bit 7 of each
                                                // byte
            "paddb %%mm1, %%mm2           \n\t" // add LBCarrys to Raw(x-bpp)/2
                                                // for each byte
            "pand  %%mm6, %%mm2           \n\t" // leave only Active Group 1
                                                // bytes to add to Avg
            "paddb %%mm2, %%mm0           \n\t" // add (Raw/2) + LBCarrys to
                                                // Avg for each Active byte
            // add 2nd active group (Raw(x-bpp)/2) to average with _LBCarry
            "psllq $24, %%mm6             \n\t" // shift the mm6 mask to cover
                                                // bytes 3-5
            "movq  %%mm0, %%mm2           \n\t" // mov updated Raws to mm2
            "psllq $24, %%mm2             \n\t" // shift data to pos. correctly
            "movq  %%mm3, %%mm1           \n\t" // now use mm1 for getting
                                                // LBCarrys
            "pand  %%mm2, %%mm1           \n\t" // get LBCarrys for each byte
                                                // where both lsb's were == 1
                                                // (valid only for active group)
            "psrlq $1, %%mm2              \n\t" // divide raw bytes by 2
            "pand  %%mm4, %%mm2           \n\t" // clear invalid bit 7 of each
                                                // byte
            "paddb %%mm1, %%mm2           \n\t" // add LBCarrys to Raw(x-bpp)/2
                                                // for each byte
            "pand  %%mm6, %%mm2           \n\t" // leave only Active Group 2
                                                // bytes to add to Avg
            "paddb %%mm2, %%mm0           \n\t" // add (Raw/2) + LBCarrys to
                                                // Avg for each Active byte

            // add 3rd active group (Raw(x-bpp)/2) to average with _LBCarry
            "psllq $24, %%mm6             \n\t" // shift mm6 mask to cover last
                                                // two bytes
            "movq  %%mm0, %%mm2           \n\t" // mov updated Raws to mm2
            "psllq $24, %%mm2             \n\t" // shift data to pos. correctly
                              // Data need be shifted only once here to
                              // get the correct x-bpp offset.
            "movq  %%mm3, %%mm1           \n\t" // now use mm1 for getting
                                                // LBCarrys
            "pand  %%mm2, %%mm1           \n\t" // get LBCarrys for each byte
                                                // where both
                              // lsb's were == 1 (only valid for active group)
            "psrlq $1, %%mm2              \n\t" // divide raw bytes by 2
            "pand  %%mm4, %%mm2           \n\t" // clear invalid bit 7 of each
                                                // byte
            "paddb %%mm1, %%mm2           \n\t" // add LBCarrys to Raw(x-bpp)/2
                                                // for each byte
            "pand  %%mm6, %%mm2           \n\t" // leave only Active Group 2
                                                // bytes to add to Avg
            "addl  $8, %%ecx              \n\t"
            "paddb %%mm2, %%mm0           \n\t" // add (Raw/2) + LBCarrys to
                                                // Avg for each Active byte
            // now ready to write back to memory
            "movq  %%mm0, -8(%1," PCX ",) \n\t"
            // move updated Raw(x) to use as Raw(x-bpp) for next loop
            "cmpl  %%eax, %%ecx           \n\t" // MMXLength
            "movq  %%mm0, %%mm2           \n\t" // mov updated Raw(x) to mm2
            "jb avg_3lp                   \n\t"

            : "=S" (dummy_value_S),            // output regs (dummy)
              "=D" (dummy_value_D),
              "=c" (dummy_value_c),
              "=a" (dummy_value_a)

            : "0" (prev_row),    // esi/rsi    // input regs
              "1" (row),         // edi/rdi
              "2" (diff),        // ecx
              "3" (MMXLength)    // eax

#if defined(CLOBBER_MMX_REGS_SUPPORTED)
            : "%mm0", "%mm1", "%mm2", "%mm3"   // clobber list
            , "%mm4", "%mm5", "%mm6", "%mm7"
#endif
         );
      }
      break;  // end 3 bpp

      case 4:   // formerly shared with 6 bpp case via _ShiftBpp and _ShiftRem,
      {         // but loop uses all 8 MMX regs, and psrlq/psllq require 64-bit
                // mem (PIC/.so problems), MMX reg (none left), or immediate
//       _ShiftBpp = bpp << 3;        // 32 (psllq)
//       _ShiftRem = 64 - _ShiftBpp;  // 32 (psrlq)

         __asm__ __volatile__ (
            LOAD_GOT_rbp
            "movq " HB_CLEAR_MASK ", %%mm4 \n\t" // _HBClearMask -> mm4
            "movq " LB_CARRY_MASK ", %%mm5 \n\t" // _LBCarryMask -> mm5
            // re-init address pointers and offset
// preload  "movl  diff, %%ecx             \n\t" // ecx:  x = offset to
                                                 // alignment boundary
            "movq " AMASK0_8_0 ", %%mm7    \n\t" // _amask0_8_0 -> mm7
            RESTORE_rbp

            // ... and clear all bytes except for 1st active group
// preload  "movl  row, %1               \n\t" // edi:  Avg(x)
            "psrlq $32, %%mm7            \n\t" // was _ShiftRem
// preload  "movl  prev_row, %0          \n\t" // esi:  Prior(x)
            "movq  %%mm7, %%mm6          \n\t"
            "psllq $32, %%mm6            \n\t" // mask for 2nd active group

            // prime the pump:  load the first Raw(x-bpp) data set
            "movq -8(%1," PCX ",), %%mm2 \n\t" // load previous aligned 8 bytes
                                             // (we correct pos. in loop below)
         "avg_4lp:                       \n\t"
            "movq (%1," PCX ",), %%mm0   \n\t"
            "psrlq $32, %%mm2            \n\t" // shift data to pos. correctly
            "movq (%0," PCX ",), %%mm1   \n\t"
            // add (Prev_row/2) to average
            "movq %%mm5, %%mm3           \n\t"
            "pand %%mm1, %%mm3           \n\t" // get lsb for each prev_row byte
            "psrlq $1, %%mm1             \n\t" // divide prev_row bytes by 2
            "pand  %%mm4, %%mm1          \n\t" // clear invalid bit 7 of each
                                               // byte
            "paddb %%mm1, %%mm0          \n\t" // add (Prev_row/2) to Avg for
                                               // each byte
            // add 1st active group (Raw(x-bpp)/2) to average with _LBCarry
            "movq %%mm3, %%mm1           \n\t" // now use mm1 for getting
                                               // LBCarrys
            "pand %%mm2, %%mm1           \n\t" // get LBCarrys for each byte
                                               // where both
                              // lsb's were == 1 (only valid for active group)
            "psrlq $1, %%mm2             \n\t" // divide raw bytes by 2
            "pand  %%mm4, %%mm2          \n\t" // clear invalid bit 7 of each
                                               // byte
            "paddb %%mm1, %%mm2          \n\t" // add LBCarrys to (Raw(x-bpp)/2)
                                               // for each byte
            "pand %%mm7, %%mm2           \n\t" // leave only Active Group 1
                                               // bytes to add to Avg
            "paddb %%mm2, %%mm0          \n\t" // add (Raw/2) + LBCarrys to Avg
                                               // for each Active
                              // byte
            // add 2nd active group (Raw(x-bpp)/2) to average with _LBCarry
            "movq %%mm0, %%mm2           \n\t" // mov updated Raws to mm2
            "psllq $32, %%mm2            \n\t" // shift data to pos. correctly
            "addl $8, %%ecx              \n\t"
            "movq %%mm3, %%mm1           \n\t" // now use mm1 for getting
                                               // LBCarrys
            "pand %%mm2, %%mm1           \n\t" // get LBCarrys for each byte
                                               // where both
                              // lsb's were == 1 (only valid for active group)
            "psrlq $1, %%mm2             \n\t" // divide raw bytes by 2
            "pand  %%mm4, %%mm2          \n\t" // clear invalid bit 7 of each
                                               // byte
            "paddb %%mm1, %%mm2          \n\t" // add LBCarrys to (Raw(x-bpp)/2)
                                               // for each byte
            "pand %%mm6, %%mm2           \n\t" // leave only Active Group 2
                                               // bytes to add to Avg
            "paddb %%mm2, %%mm0          \n\t" // add (Raw/2) + LBCarrys to
                                               // Avg for each Active byte
            "cmpl %%eax, %%ecx           \n\t" // MMXLength
            // now ready to write back to memory
            "movq %%mm0, -8(%1," PCX ",) \n\t"
            // prep Raw(x-bpp) for next loop
            "movq %%mm0, %%mm2           \n\t" // mov updated Raws to mm2
            "jb avg_4lp                  \n\t"

            : "=S" (dummy_value_S),            // output regs (dummy)
              "=D" (dummy_value_D),
              "=c" (dummy_value_c),
              "=a" (dummy_value_a)

            : "0" (prev_row),    // esi/rsi    // input regs
              "1" (row),         // edi/rdi
              "2" (diff),        // ecx
              "3" (MMXLength)    // eax

#if defined(CLOBBER_MMX_REGS_SUPPORTED)
            : "%mm0", "%mm1", "%mm2", "%mm3"   // clobber list
            , "%mm4", "%mm5", "%mm6", "%mm7"
#endif
         );
      }
      break;  // end 4 bpp

      case 1:
      {
         __asm__ __volatile__ (
            // re-init address pointers and offset
// preload  "movl diff, %%ecx            \n\t" // ecx: x = offset to align. bdry
// preload  "movl row, %1                \n\t" // edi/rdi:  Avg(x)
// preload  "movl FullLength, %%eax      \n\t"
            "cmpl %%eax, %%ecx           \n\t" // test if offset at end of array
            "jnb avg_1end                \n\t"

            SAVE_ebp

            // do Avg decode for remaining bytes
// preload  "movl prev_row, %0           \n\t" // esi/rsi:  Prior(x)
            "mov  %1, " PBP "            \n\t" // copy of row pointer...
            "dec  " PBP "                \n\t" // ebp/rbp:  Raw(x-bpp)
            "xorl %%edx, %%edx           \n\t" // zero edx before using dl & dx
                                               //  in loop below
            SAVE_GOT_ebx

         "avg_1lp:                       \n\t"
            // Raw(x) = Avg(x) + ((Raw(x-bpp) + Prior(x))/2)
            "xorl %%ebx, %%ebx           \n\t"
            "movb (%0," PCX ",), %%dl    \n\t" // load dl with Prior(x)
            "movb (" PBP "," PCX ",), %%bl \n\t" // load bl with Raw(x-bpp)
            "addw %%dx, %%bx             \n\t"
            "incl %%ecx                  \n\t"
            "shrw %%bx                   \n\t" // divide by 2
            "addb -1(%1," PCX ",), %%bl  \n\t" // add Avg(x); -1 to offset
                                               // inc ecx
            "cmpl %%eax, %%ecx           \n\t" // check if at end of array
            "movb %%bl, -1(%1," PCX ",)  \n\t" // write back Raw(x);
                         // mov does not affect flags; -1 to offset inc ecx
            "jb avg_1lp                  \n\t"

            RESTORE_GOT_ebx
            RESTORE_ebp

         "avg_1end:                      \n\t"

            : "=S" (dummy_value_S),            // output regs (dummy)
              "=D" (dummy_value_D),
              "=c" (dummy_value_c),
              "=a" (dummy_value_a)

            : "0" (prev_row),    // esi/rsi    // input regs
              "1" (row),         // edi/rdi
              "2" (diff),        // ecx
              "3" (FullLength)   // eax

            : "%edx"                           // clobber list
              _CLOBBER_GOT_ebx
              _CLOBBER_ebp
         );
      }
      return;  // end 1 bpp

      case 2:
      {
//       _ShiftBpp = 16;   // == 2 * 8
//       _ShiftRem = 48;   // == 64 - _ShiftBpp

         __asm__ __volatile__ (
            LOAD_GOT_rbp
            // load (former) _ActiveMask
            "movq " AMASK6_2_0 ", %%mm7    \n\t" // _amask6_2_0 -> mm7
            // re-init address pointers and offset
// preload  "movl  diff, %%ecx             \n\t" // ecx:  x = offset to
                                                 // alignment boundary
            "movq " LB_CARRY_MASK ", %%mm5 \n\t" // _LBCarryMask -> mm5
// preload  "movl  row, %1                 \n\t" // edi:  Avg(x)
            "movq " HB_CLEAR_MASK ", %%mm4 \n\t" // _HBClearMask -> mm4
// preload  "movl  prev_row, %0            \n\t" // esi:  Prior(x)
            RESTORE_rbp

            // prime the pump:  load the first Raw(x-bpp) data set
            "movq -8(%1," PCX ",), %%mm2 \n\t" // load previous aligned 8 bytes
                                             // (we correct pos. in loop below)
         "avg_2lp:                       \n\t"
            "movq (%1," PCX ",), %%mm0   \n\t"
            "psrlq $48, %%mm2            \n\t" // shift data to pos. correctly
            "movq (%0," PCX ",), %%mm1   \n\t" //  (GRR BUGFIX:  was psllq)
            // add (Prev_row/2) to average
            "movq %%mm5, %%mm3           \n\t"
            "pand %%mm1, %%mm3           \n\t" // get lsb for each prev_row byte
            "psrlq $1, %%mm1             \n\t" // divide prev_row bytes by 2
            "pand  %%mm4, %%mm1          \n\t" // clear invalid bit 7 of each
                                               // byte
            "movq %%mm7, %%mm6           \n\t"
            "paddb %%mm1, %%mm0          \n\t" // add (Prev_row/2) to Avg for
                                               // each byte

            // add 1st active group (Raw(x-bpp)/2) to average with _LBCarry
            "movq %%mm3, %%mm1           \n\t" // now use mm1 for getting
                                               // LBCarrys
            "pand %%mm2, %%mm1           \n\t" // get LBCarrys for each byte
                                               // where both
                                               // lsb's were == 1 (only valid
                                               // for active group)
            "psrlq $1, %%mm2             \n\t" // divide raw bytes by 2
            "pand  %%mm4, %%mm2          \n\t" // clear invalid bit 7 of each
                                               // byte
            "paddb %%mm1, %%mm2          \n\t" // add LBCarrys to (Raw(x-bpp)/2)
                                               // for each byte
            "pand %%mm6, %%mm2           \n\t" // leave only Active Group 1
                                               // bytes to add to Avg
            "paddb %%mm2, %%mm0          \n\t" // add (Raw/2) + LBCarrys to Avg
                                               // for each Active byte

            // add 2nd active group (Raw(x-bpp)/2) to average with _LBCarry
            "psllq $16, %%mm6            \n\t" // shift the mm6 mask to cover
                                               // bytes 2 & 3
            "movq %%mm0, %%mm2           \n\t" // mov updated Raws to mm2
            "psllq $16, %%mm2            \n\t" // shift data to pos. correctly
            "movq %%mm3, %%mm1           \n\t" // now use mm1 for getting
                                               // LBCarrys
            "pand %%mm2, %%mm1           \n\t" // get LBCarrys for each byte
                                               // where both
                                               // lsb's were == 1 (only valid
                                               // for active group)
            "psrlq $1, %%mm2             \n\t" // divide raw bytes by 2
            "pand  %%mm4, %%mm2          \n\t" // clear invalid bit 7 of each
                                               // byte
            "paddb %%mm1, %%mm2          \n\t" // add LBCarrys to (Raw(x-bpp)/2)
                                               // for each byte
            "pand %%mm6, %%mm2           \n\t" // leave only Active Group 2
                                               // bytes to add to Avg
            "paddb %%mm2, %%mm0          \n\t" // add (Raw/2) + LBCarrys to
                                               // Avg for each Active byte

            // add 3rd active group (Raw(x-bpp)/2) to average with _LBCarry
            "psllq $16, %%mm6            \n\t" // shift the mm6 mask to cover
                                               // bytes 4 & 5
            "movq %%mm0, %%mm2           \n\t" // mov updated Raws to mm2
            "psllq $16, %%mm2            \n\t" // shift data to pos. correctly
            "movq %%mm3, %%mm1           \n\t" // now use mm1 for getting
                                               // LBCarrys
            "pand %%mm2, %%mm1           \n\t" // get LBCarrys for each byte
                                               // where both lsb's were == 1
                                               // (only valid for active group)
            "psrlq $1, %%mm2             \n\t" // divide raw bytes by 2
            "pand  %%mm4, %%mm2          \n\t" // clear invalid bit 7 of each
                                               // byte
            "paddb %%mm1, %%mm2          \n\t" // add LBCarrys to (Raw(x-bpp)/2)
                                               // for each byte
            "pand %%mm6, %%mm2           \n\t" // leave only Active Group 2
                                               // bytes to add to Avg
            "paddb %%mm2, %%mm0          \n\t" // add (Raw/2) + LBCarrys to
                                               // Avg for each Active byte

            // add 4th active group (Raw(x-bpp)/2) to average with _LBCarry
            "psllq $16, %%mm6            \n\t" // shift the mm6 mask to cover
                                               // bytes 6 & 7
            "movq %%mm0, %%mm2           \n\t" // mov updated Raws to mm2
            "psllq $16, %%mm2            \n\t" // shift data to pos. correctly
            "addl $8, %%ecx              \n\t"
            "movq %%mm3, %%mm1           \n\t" // now use mm1 for getting
                                               // LBCarrys
            "pand %%mm2, %%mm1           \n\t" // get LBCarrys for each byte
                                               // where both
                                               // lsb's were == 1 (only valid
                                               // for active group)
            "psrlq $1, %%mm2             \n\t" // divide raw bytes by 2
            "pand  %%mm4, %%mm2          \n\t" // clear invalid bit 7 of each
                                               // byte
            "paddb %%mm1, %%mm2          \n\t" // add LBCarrys to (Raw(x-bpp)/2)
                                               // for each byte
            "pand %%mm6, %%mm2           \n\t" // leave only Active Group 2
                                               // bytes to add to Avg
            "paddb %%mm2, %%mm0          \n\t" // add (Raw/2) + LBCarrys to
                                               // Avg for each Active byte
            "cmpl %%eax, %%ecx           \n\t" // MMXLength
            // now ready to write back to memory
            "movq %%mm0, -8(%1," PCX ",) \n\t"
            // prep Raw(x-bpp) for next loop
            "movq %%mm0, %%mm2           \n\t" // mov updated Raws to mm2
            "jb avg_2lp                  \n\t"

            : "=S" (dummy_value_S),            // output regs (dummy)
              "=D" (dummy_value_D),
              "=c" (dummy_value_c),
              "=a" (dummy_value_a)

            : "0" (prev_row),    // esi/rsi    // input regs
              "1" (row),         // edi/rdi
              "2" (diff),        // ecx
              "3" (MMXLength)    // eax

#if defined(CLOBBER_MMX_REGS_SUPPORTED)
            : "%mm0", "%mm1", "%mm2", "%mm3"   // clobber list
            , "%mm4", "%mm5", "%mm6", "%mm7"
#endif
         );
      }
      break;  // end 2 bpp

      case 6:   // formerly shared with 4 bpp case (see comments there)
      {
//       _ShiftBpp = bpp << 3;        // 48 (psllq)
//       _ShiftRem = 64 - _ShiftBpp;  // 16 (psrlq)

         __asm__ __volatile__ (
            LOAD_GOT_rbp
            "movq " HB_CLEAR_MASK ", %%mm4 \n\t" // _HBClearMask -> mm4
            "movq " LB_CARRY_MASK ", %%mm5 \n\t" // _LBCarryMask -> mm5
            // re-init address pointers and offset
// preload  "movl  diff, %%ecx             \n\t" // ecx:  x = offset to
                                                 // alignment boundary
            "movq " AMASK0_8_0 ", %%mm7    \n\t" // _amask0_8_0 -> mm7
            RESTORE_rbp

            // ... and clear all bytes except for 1st active group
// preload  "movl  row, %1               \n\t" // edi:  Avg(x)
            "psrlq $16, %%mm7            \n\t"
// preload  "movl  prev_row, %0          \n\t" // esi:  Prior(x)
            "movq  %%mm7, %%mm6          \n\t"
            "psllq $48, %%mm6            \n\t" // mask for 2nd active group

            // prime the pump:  load the first Raw(x-bpp) data set
            "movq -8(%1," PCX ",), %%mm2 \n\t" // load previous aligned 8 bytes
                                             // (we correct pos. in loop below)
         "avg_6lp:                       \n\t"
            "movq (%1," PCX ",), %%mm0   \n\t"
            "psrlq $16, %%mm2            \n\t" // shift data to pos. correctly
            "movq (%0," PCX ",), %%mm1   \n\t"
            // add (Prev_row/2) to average
            "movq %%mm5, %%mm3           \n\t"
            "pand %%mm1, %%mm3           \n\t" // get lsb for each prev_row byte
            "psrlq $1, %%mm1             \n\t" // divide prev_row bytes by 2
            "pand  %%mm4, %%mm1          \n\t" // clear invalid bit 7 of each
                                               // byte
            "paddb %%mm1, %%mm0          \n\t" // add (Prev_row/2) to Avg for
                                               // each byte
            // add 1st active group (Raw(x-bpp)/2) to average with _LBCarry
            "movq %%mm3, %%mm1           \n\t" // now use mm1 for getting
                                               // LBCarrys
            "pand %%mm2, %%mm1           \n\t" // get LBCarrys for each byte
                                               // where both
                              // lsb's were == 1 (only valid for active group)
            "psrlq $1, %%mm2             \n\t" // divide raw bytes by 2
            "pand  %%mm4, %%mm2          \n\t" // clear invalid bit 7 of each
                                               // byte
            "paddb %%mm1, %%mm2          \n\t" // add LBCarrys to (Raw(x-bpp)/2)
                                               // for each byte
            "pand %%mm7, %%mm2           \n\t" // leave only Active Group 1
                                               // bytes to add to Avg
            "paddb %%mm2, %%mm0          \n\t" // add (Raw/2) + LBCarrys to Avg
                                               // for each Active
                              // byte
            // add 2nd active group (Raw(x-bpp)/2) to average with _LBCarry
            "movq %%mm0, %%mm2           \n\t" // mov updated Raws to mm2
            "psllq $48, %%mm2            \n\t" // shift data to pos. correctly
            "addl $8, %%ecx              \n\t"
            "movq %%mm3, %%mm1           \n\t" // now use mm1 for getting
                                               // LBCarrys
            "pand %%mm2, %%mm1           \n\t" // get LBCarrys for each byte
                                               // where both
                              // lsb's were == 1 (only valid for active group)
            "psrlq $1, %%mm2             \n\t" // divide raw bytes by 2
            "pand  %%mm4, %%mm2          \n\t" // clear invalid bit 7 of each
                                               // byte
            "paddb %%mm1, %%mm2          \n\t" // add LBCarrys to (Raw(x-bpp)/2)
                                               // for each byte
            "pand %%mm6, %%mm2           \n\t" // leave only Active Group 2
                                               // bytes to add to Avg
            "paddb %%mm2, %%mm0          \n\t" // add (Raw/2) + LBCarrys to
                                               // Avg for each Active byte
            "cmpl %%eax, %%ecx           \n\t" // MMXLength
            // now ready to write back to memory
            "movq %%mm0, -8(%1," PCX ",) \n\t"
            // prep Raw(x-bpp) for next loop
            "movq %%mm0, %%mm2           \n\t" // mov updated Raws to mm2
            "jb avg_6lp                  \n\t"

            : "=S" (dummy_value_S),            // output regs (dummy)
              "=D" (dummy_value_D),
              "=c" (dummy_value_c),
              "=a" (dummy_value_a)

            : "0" (prev_row),    // esi/rsi    // input regs
              "1" (row),         // edi/rdi
              "2" (diff),        // ecx
              "3" (MMXLength)    // eax

#if defined(CLOBBER_MMX_REGS_SUPPORTED)
            : "%mm0", "%mm1", "%mm2", "%mm3"   // clobber list
            , "%mm4", "%mm5", "%mm6", "%mm7"
#endif
         );
      }
      break;  // end 6 bpp

      case 8:
      {
         __asm__ __volatile__ (
            // re-init address pointers and offset
// preload  "movl  diff, %%ecx             \n\t" // ecx:  x = offset to
                                                 // alignment boundary
            LOAD_GOT_rbp
            "movq " LB_CARRY_MASK ", %%mm5 \n\t" // [interleave for parallel.?]
// preload  "movl  row, %1                 \n\t" // edi:  Avg(x)
            "movq " HB_CLEAR_MASK ", %%mm4 \n\t" // _HBClearMask -> mm4
// preload  "movl  prev_row, %0            \n\t" // esi:  Prior(x)
            RESTORE_rbp

            // prime the pump:  load the first Raw(x-bpp) data set
            "movq -8(%1," PCX ",), %%mm2 \n\t" // load previous aligned 8 bytes
                                      // (NO NEED to correct pos. in loop below)

         "avg_8lp:                       \n\t"
            "movq (%1," PCX ",), %%mm0   \n\t"
            "movq %%mm5, %%mm3           \n\t"
            "movq (%0," PCX ",), %%mm1   \n\t"
            "addl $8, %%ecx              \n\t"
            "pand %%mm1, %%mm3           \n\t" // get lsb for each prev_row byte
            "psrlq $1, %%mm1             \n\t" // divide prev_row bytes by 2
            "pand %%mm2, %%mm3           \n\t" // get LBCarrys for each byte
                                               //  where both lsb's were == 1
            "psrlq $1, %%mm2             \n\t" // divide raw bytes by 2
            "pand  %%mm4, %%mm1          \n\t" // clear invalid bit 7, each byte
            "paddb %%mm3, %%mm0          \n\t" // add LBCarrys to Avg, each byte
            "pand  %%mm4, %%mm2          \n\t" // clear invalid bit 7, each byte
            "paddb %%mm1, %%mm0          \n\t" // add (Prev_row/2) to Avg, each
            "paddb %%mm2, %%mm0          \n\t" // add (Raw/2) to Avg for each
            "cmpl %%eax, %%ecx           \n\t" // MMXLength
            "movq %%mm0, -8(%1," PCX ",) \n\t"
            "movq %%mm0, %%mm2           \n\t" // reuse as Raw(x-bpp)
            "jb avg_8lp                  \n\t"

            : "=S" (dummy_value_S),            // output regs (dummy)
              "=D" (dummy_value_D),
              "=c" (dummy_value_c),
              "=a" (dummy_value_a)

            : "0" (prev_row),    // esi/rsi    // input regs
              "1" (row),         // edi/rdi
              "2" (diff),        // ecx
              "3" (MMXLength)    // eax

#if defined(CLOBBER_MMX_REGS_SUPPORTED)
            : "%mm0", "%mm1", "%mm2"           // clobber list
            , "%mm3", "%mm4", "%mm5"
#endif
         );
      }
      break;  // end 8 bpp

      default:                // bpp != 1,2,3,4,6,8:  doesn't exist
      {
         // ERROR:  SHOULD NEVER BE REACHED
#if defined(PNG_DEBUG)
         png_debug(1, "Internal libpng logic error (GCC "
           "png_read_filter_row_mmx_avg())\n");
#endif
      }
      break;

   } // end switch (bpp)

   __asm__ __volatile__ (
      // MMX acceleration complete; now do clean-up
      // check if any remaining bytes left to decode
//pre "movl FullLength, %%edx      \n\t"
//pre "movl MMXLength, %%eax       \n\t" // eax:  x == offset bytes after MMX
//pre "movl row, %2                \n\t" // edi:  Avg(x)
      "cmpl %%edx, %%eax           \n\t" // test if offset at end of array
      "jnb avg_end                 \n\t"

      SAVE_ebp

      // do Avg decode for remaining bytes
//pre "movl prev_row, %1           \n\t" // esi:  Prior(x)
      "mov  %2, " PBP "            \n\t" // copy of row pointer...
//pre "subl bpp, " PBP "           \n\t" // (bpp is preloaded into ecx)
      "sub  " PCX "," PBP "        \n\t" // ebp:  Raw(x-bpp)
      "xorl %%ecx, %%ecx           \n\t" // zero ecx before using cl & cx below

      SAVE_GOT_ebx

   "avg_lp2:                       \n\t"
      // Raw(x) = Avg(x) + ((Raw(x-bpp) + Prior(x))/2)
      "xorl %%ebx, %%ebx           \n\t"
      "movb (%1," PAX ",), %%cl    \n\t" // load cl with Prior(x)
      "movb (" PBP "," PAX ",), %%bl \n\t" // load bl with Raw(x-bpp)
      "addw %%cx, %%bx             \n\t"
      "incl %%eax                  \n\t"
      "shrw %%bx                   \n\t" // divide by 2
      "addb -1(%2," PAX ",), %%bl  \n\t" // add Avg(x); -1 to offset inc eax
      "cmpl %%edx, %%eax           \n\t" // check if at end of array
      "movb %%bl, -1(%2," PAX ",)  \n\t" // write back Raw(x) [mov does not
      "jb avg_lp2                  \n\t" //  affect flags; -1 to offset inc eax]

      RESTORE_GOT_ebx
      RESTORE_ebp

   "avg_end:                       \n\t"
      "EMMS                        \n\t" // end MMX; prep for poss. FP instrs.

      : "=c" (dummy_value_c),            // output regs (dummy)
        "=S" (dummy_value_S),
        "=D" (dummy_value_D),
        "=a" (dummy_value_a),
        "=d" (dummy_value_d)

      : "0" (bpp),         // ecx        // input regs
        "1" (prev_row),    // esi/rsi
        "2" (row),         // edi/rdi
        "3" (MMXLength),   // eax
        "4" (FullLength)   // edx

      CLOB_COLON_ebx_ebp                 // clobber list
        CLOBBER_GOT_ebx
        CLOB_COMMA_ebx_ebp
        CLOBBER_ebp
   );

} /* end png_read_filter_row_mmx_avg() */

#endif /* PNG_MMX_READ_FILTER_AVG_SUPPORTED */



#if defined(PNG_MMX_READ_FILTER_PAETH_SUPPORTED)
#if defined(PNG_x86_64_USE_GOTPCREL) || defined(PNG_THREAD_UNSAFE_OK)

//===========================================================================//
//                                                                           //
//         P N G _ R E A D _ F I L T E R _ R O W _ M M X _ P A E T H         //
//                                                                           //
//===========================================================================//

// Optimized code for PNG Paeth filter decoder

static void /* PRIVATE */
png_read_filter_row_mmx_paeth(png_row_infop row_info, png_bytep row,
                              png_bytep prev_row)
{
   unsigned FullLength, MMXLength;  // png_uint_32 is actually 64-bit on x86-64
   int bpp;
   int dummy_value_a;
   int dummy_value_c;   // fix 'forbidden register 2 (cx) was spilled' error
   int dummy_value_d;
   png_charp dummy_value_S;
   png_charp dummy_value_D;
   int diff; //     __attribute__((used));

   bpp = (row_info->pixel_depth + 7) >> 3;  // calc number of bytes per pixel
   FullLength = row_info->rowbytes;         // number of bytes to filter

   __asm__ __volatile__ (
      SAVE_GOT_ebx
      SAVE_r15
      SAVE_ebp
//pre "movl row, %2                \n\t" // edi/rdi
      "xorl %%ebx, %%ebx           \n\t" // ebx:  x offset
//pre "movl prev_row, %1           \n\t" // esi/rsi
      "xorl %%edx, %%edx           \n\t" // edx:  x-bpp offset
//pre "movl FullLength, %%eax      \n\t" // bring in via eax...
      SAVE_FullLength                    // ...but store for later use
      "xorl %%eax, %%eax           \n\t"

      // Compute the Raw value for the first bpp bytes
      // Note: the formula works out to be always
      //   Paeth(x) = Raw(x) + Prior(x)      where x < bpp
   "paeth_rlp:                     \n\t"
      "movb (%2," PBX ",), %%al    \n\t"
      "addb (%1," PBX ",), %%al    \n\t"
      "incl %%ebx                  \n\t"
//pre "cmpl bpp, %%ebx             \n\t" (bpp is preloaded into ecx)
      "cmpl %%ecx, %%ebx           \n\t"
      "movb %%al, -1(%2," PBX ",)  \n\t"
      "jb paeth_rlp                \n\t"

      // get # of bytes to alignment (note:  computing _delta_ of two pointers,
      // so hereafter %%ebp is sufficient even on 64-bit)
      "mov  %2, " PBP "            \n\t" // take start of row
      "add  " PBX "," PBP "        \n\t" // add bpp
      "add  $0xf, " PBP "          \n\t" // add 7+8 to incr past alignment bdry
//    "andl $0xfffffff8, %%ebp     \n\t" // mask to alignment boundary (32-bit!)
      CLEAR_BOTTOM_3_BITS  PBP    "\n\t" // mask to alignment boundary
      "sub  %2, " PBP "            \n\t" // subtract row ptr again => ebp =
      "jz paeth_go                 \n\t" //  target value of ebx at alignment

      "xorl %%ecx, %%ecx           \n\t"

      SAVE_r11_r12_r13

      // fix alignment
   "paeth_lp1:                     \n\t"
      "xorl %%eax, %%eax           \n\t"
      // pav = p - a = (a + b - c) - a = b - c
      "movb (%1," PBX ",), %%al    \n\t" // load Prior(x) into al
      "movb (%1," PDX ",), %%cl    \n\t" // load Prior(x-bpp) into cl
      "subl %%ecx, %%eax           \n\t" // subtract Prior(x-bpp)
      "movl %%eax, " pa_TEMP "     \n\t" // Save pav for later use
      "xorl %%eax, %%eax           \n\t"
      // pbv = p - b = (a + b - c) - b = a - c
      "movb (%2," PDX ",), %%al    \n\t" // load Raw(x-bpp) into al
      "subl %%ecx, %%eax           \n\t" // subtract Prior(x-bpp)
      "movl %%eax, %%ecx           \n\t"
      // pcv = p - c = (a + b - c) - c = (a - c) + (b - c) = pav + pbv
      "addl " pa_TEMP ", %%eax     \n\t" // pcv = pav + pbv
      // pc = abs(pcv)
      "testl $0x80000000, %%eax    \n\t"
      "jz paeth_pca                \n\t"
      "negl %%eax                  \n\t" // reverse sign of neg values

   "paeth_pca:                     \n\t"
      "movl %%eax, " pc_TEMP "     \n\t" // save pc for later use
      // pb = abs(pbv)
      "testl $0x80000000, %%ecx    \n\t"
      "jz paeth_pba                \n\t"
      "negl %%ecx                  \n\t" // reverse sign of neg values

   "paeth_pba:                     \n\t"
      "movl %%ecx, " pb_TEMP "     \n\t" // save pb for later use
      // pa = abs(pav)
      "movl " pa_TEMP ", %%eax     \n\t"
      "testl $0x80000000, %%eax    \n\t"
      "jz paeth_paa                \n\t"
      "negl %%eax                  \n\t" // reverse sign of neg values

   "paeth_paa:                     \n\t"
      "movl %%eax, " pa_TEMP "     \n\t" // save pa for later use
      // test if pa <= pb
      "cmpl %%ecx, %%eax           \n\t"
      "jna paeth_abb               \n\t"
      // pa > pb; now test if pb <= pc
      "cmpl " pc_TEMP ", %%ecx     \n\t"
      "jna paeth_bbc               \n\t"
      // pb > pc; Raw(x) = Paeth(x) + Prior(x-bpp)
      "movb (%1," PDX ",), %%cl    \n\t" // load Prior(x-bpp) into cl
      "jmp paeth_paeth             \n\t"

   "paeth_bbc:                     \n\t"
      // pb <= pc; Raw(x) = Paeth(x) + Prior(x)
      "movb (%1," PBX ",), %%cl    \n\t" // load Prior(x) into cl
      "jmp paeth_paeth             \n\t"

   "paeth_abb:                     \n\t"
      // pa <= pb; now test if pa <= pc
      "cmpl " pc_TEMP ", %%eax     \n\t"
      "jna paeth_abc               \n\t"
      // pa > pc; Raw(x) = Paeth(x) + Prior(x-bpp)
      "movb (%1," PDX ",), %%cl    \n\t" // load Prior(x-bpp) into cl
      "jmp paeth_paeth             \n\t"

   "paeth_abc:                     \n\t"
      // pa <= pc; Raw(x) = Paeth(x) + Raw(x-bpp)
      "movb (%2," PDX ",), %%cl    \n\t" // load Raw(x-bpp) into cl

   "paeth_paeth:                   \n\t"
      "incl %%ebx                  \n\t"
      "incl %%edx                  \n\t"
      // Raw(x) = (Paeth(x) + Paeth_Predictor( a, b, c )) mod 256
      "addb %%cl, -1(%2," PBX ",)  \n\t"
      "cmpl %%ebp, %%ebx           \n\t"
      "jb paeth_lp1                \n\t"

      RESTORE_r11_r12_r13

   "paeth_go:                      \n\t"
      RESTORE_FullLength "%%ecx    \n\t" // FullLength -> ecx
      "movl %%ecx, %%eax           \n\t"
      "subl %%ebx, %%eax           \n\t" // subtract alignment fix
      "andl $0x00000007, %%eax     \n\t" // calc bytes over mult of 8
      "subl %%eax, %%ecx           \n\t" // drop over bytes from original length
//out "movl %%ecx, MMXLength       \n\t"
      "movl %%ebp, %%eax           \n\t" // ebp = diff, but no reg constraint(?)
      RESTORE_ebp                        //  (could swap ebp and edx functions)
      RESTORE_r15
      RESTORE_GOT_ebx

      : "=c" (MMXLength),                // output regs
        "=S" (dummy_value_S),
        "=D" (dummy_value_D),
        "=a" (diff)

      : "0" (bpp),         // ecx        // input regs
        "1" (prev_row),    // esi/rsi
        "2" (row),         // edi/rdi
        "3" (FullLength)   // eax

      : "%edx"                           // clobber list
        _CLOBBER_r11_r12_r13
        _CLOBBER_r15
        _CLOBBER_ebp
        _CLOBBER_GOT_ebx
   );

   // now do the math for the rest of the row
   switch (bpp)
   {
      case 3:
      {
//       _ShiftBpp = 24;    // == bpp * 8
//       _ShiftRem = 40;    // == 64 - _ShiftBpp

         __asm__ __volatile__ (
            LOAD_GOT_rbp
// preload  "movl diff, %%ecx            \n\t"
// preload  "movl row, %1                \n\t" // edi/rdi
// preload  "movl prev_row, %0           \n\t" // esi/rsi
            "pxor %%mm0, %%mm0           \n\t"

            // prime the pump:  load the first Raw(x-bpp) data set
            "movq -8(%1," PCX ",), %%mm1 \n\t"
         "paeth_3lp:                     \n\t"
            "psrlq $40, %%mm1            \n\t" // shift last 3 bytes to 1st
                                               // 3 bytes
            "movq (%0," PCX ",), %%mm2   \n\t" // load b=Prior(x)
            "punpcklbw %%mm0, %%mm1      \n\t" // unpack High bytes of a
            "movq -8(%0," PCX ",), %%mm3 \n\t" // prep c=Prior(x-bpp) bytes
            "punpcklbw %%mm0, %%mm2      \n\t" // unpack High bytes of b
            "psrlq $40, %%mm3            \n\t" // shift last 3 bytes to 1st
                                               // 3 bytes
            // pav = p - a = (a + b - c) - a = b - c
            "movq %%mm2, %%mm4           \n\t"
            "punpcklbw %%mm0, %%mm3      \n\t" // unpack High bytes of c
            // pbv = p - b = (a + b - c) - b = a - c
            "movq %%mm1, %%mm5           \n\t"
            "psubw %%mm3, %%mm4          \n\t"
            "pxor %%mm7, %%mm7           \n\t"
            // pcv = p - c = (a + b - c) -c = (a - c) + (b - c) = pav + pbv
            "movq %%mm4, %%mm6           \n\t"
            "psubw %%mm3, %%mm5          \n\t"

            // pa = abs(p-a) = abs(pav)
            // pb = abs(p-b) = abs(pbv)
            // pc = abs(p-c) = abs(pcv)
            "pcmpgtw %%mm4, %%mm0        \n\t" // create mask pav bytes < 0
            "paddw %%mm5, %%mm6          \n\t"
            "pand %%mm4, %%mm0           \n\t" // only pav bytes < 0 in mm7
            "pcmpgtw %%mm5, %%mm7        \n\t" // create mask pbv bytes < 0
            "psubw %%mm0, %%mm4          \n\t"
            "pand %%mm5, %%mm7           \n\t" // only pbv bytes < 0 in mm0
            "psubw %%mm0, %%mm4          \n\t"
            "psubw %%mm7, %%mm5          \n\t"
            "pxor %%mm0, %%mm0           \n\t"
            "pcmpgtw %%mm6, %%mm0        \n\t" // create mask pcv bytes < 0
            "pand %%mm6, %%mm0           \n\t" // only pav bytes < 0 in mm7
            "psubw %%mm7, %%mm5          \n\t"
            "psubw %%mm0, %%mm6          \n\t"
            //  test pa <= pb
            "movq %%mm4, %%mm7           \n\t"
            "psubw %%mm0, %%mm6          \n\t"
            "pcmpgtw %%mm5, %%mm7        \n\t" // pa > pb?
            "movq %%mm7, %%mm0           \n\t"
            // use mm7 mask to merge pa & pb
            "pand %%mm7, %%mm5           \n\t"
            // use mm0 mask copy to merge a & b
            "pand %%mm0, %%mm2           \n\t"
            "pandn %%mm4, %%mm7          \n\t"
            "pandn %%mm1, %%mm0          \n\t"
            "paddw %%mm5, %%mm7          \n\t"
            "paddw %%mm2, %%mm0          \n\t"
            //  test  ((pa <= pb)? pa:pb) <= pc
            "pcmpgtw %%mm6, %%mm7        \n\t" // pab > pc?
            "pxor %%mm1, %%mm1           \n\t"
            "pand %%mm7, %%mm3           \n\t"
            "pandn %%mm0, %%mm7          \n\t"
            "paddw %%mm3, %%mm7          \n\t"
            "pxor %%mm0, %%mm0           \n\t"
            "packuswb %%mm1, %%mm7       \n\t"
            "movq (%0," PCX ",), %%mm3   \n\t" // load c=Prior(x-bpp)
            "pand " AMASK5_3_0 ", %%mm7  \n\t" // _amask5_3_0 (was _ActiveMask)
            "movq %%mm3, %%mm2           \n\t" // load b=Prior(x) step 1
            "paddb (%1," PCX ",), %%mm7  \n\t" // add Paeth predictor + Raw(x)
            "punpcklbw %%mm0, %%mm3      \n\t" // unpack High bytes of c
            "movq %%mm7, (%1," PCX ",)   \n\t" // write back updated value
            "movq %%mm7, %%mm1           \n\t" // now mm1 will be used as
                                               // Raw(x-bpp)
            // now do Paeth for 2nd set of bytes (3-5)
            "psrlq $24, %%mm2            \n\t" // load b=Prior(x) step 2
            "punpcklbw %%mm0, %%mm1      \n\t" // unpack High bytes of a
            "pxor %%mm7, %%mm7           \n\t"
            "punpcklbw %%mm0, %%mm2      \n\t" // unpack High bytes of b
            // pbv = p - b = (a + b - c) - b = a - c
            "movq %%mm1, %%mm5           \n\t"
            // pav = p - a = (a + b - c) - a = b - c
            "movq %%mm2, %%mm4           \n\t"
            "psubw %%mm3, %%mm5          \n\t"
            "psubw %%mm3, %%mm4          \n\t"
            // pcv = p - c = (a + b - c) -c = (a - c) + (b - c) =
            //       pav + pbv = pbv + pav
            "movq %%mm5, %%mm6           \n\t"
            "paddw %%mm4, %%mm6          \n\t"

            // pa = abs(p-a) = abs(pav)
            // pb = abs(p-b) = abs(pbv)
            // pc = abs(p-c) = abs(pcv)
            "pcmpgtw %%mm5, %%mm0        \n\t" // create mask pbv bytes < 0
            "pcmpgtw %%mm4, %%mm7        \n\t" // create mask pav bytes < 0
            "pand %%mm5, %%mm0           \n\t" // only pbv bytes < 0 in mm0
            "pand %%mm4, %%mm7           \n\t" // only pav bytes < 0 in mm7
            "psubw %%mm0, %%mm5          \n\t"
            "psubw %%mm7, %%mm4          \n\t"
            "psubw %%mm0, %%mm5          \n\t"
            "psubw %%mm7, %%mm4          \n\t"
            "pxor %%mm0, %%mm0           \n\t"
            "pcmpgtw %%mm6, %%mm0        \n\t" // create mask pcv bytes < 0
            "pand %%mm6, %%mm0           \n\t" // only pav bytes < 0 in mm7
            "psubw %%mm0, %%mm6          \n\t"
            //  test pa <= pb
            "movq %%mm4, %%mm7           \n\t"
            "psubw %%mm0, %%mm6          \n\t"
            "pcmpgtw %%mm5, %%mm7        \n\t" // pa > pb?
            "movq %%mm7, %%mm0           \n\t"
            // use mm7 mask to merge pa & pb
            "pand %%mm7, %%mm5           \n\t"
            // use mm0 mask copy to merge a & b
            "pand %%mm0, %%mm2           \n\t"
            "pandn %%mm4, %%mm7          \n\t"
            "pandn %%mm1, %%mm0          \n\t"
            "paddw %%mm5, %%mm7          \n\t"
            "paddw %%mm2, %%mm0          \n\t"
            //  test  ((pa <= pb)? pa:pb) <= pc
            "pcmpgtw %%mm6, %%mm7        \n\t" // pab > pc?
            "movq (%0," PCX ",), %%mm2   \n\t" // load b=Prior(x)
            "pand %%mm7, %%mm3           \n\t"
            "pandn %%mm0, %%mm7          \n\t"
            "pxor %%mm1, %%mm1           \n\t"
            "paddw %%mm3, %%mm7          \n\t"
            "pxor %%mm0, %%mm0           \n\t"
            "packuswb %%mm1, %%mm7       \n\t"
            "movq %%mm2, %%mm3           \n\t" // load c=Prior(x-bpp) step 1
            "pand " AMASK5_3_0 ", %%mm7  \n\t" // _amask5_3_0 (was _ActiveMask)
            "punpckhbw %%mm0, %%mm2      \n\t" // unpack High bytes of b
            "psllq $24, %%mm7            \n\t" // shift bytes to 2nd group of
                                               // 3 bytes
             // pav = p - a = (a + b - c) - a = b - c
            "movq %%mm2, %%mm4           \n\t"
            "paddb (%1," PCX ",), %%mm7  \n\t" // add Paeth predictor + Raw(x)
            "psllq $24, %%mm3            \n\t" // load c=Prior(x-bpp) step 2
            "movq %%mm7, (%1," PCX ",)   \n\t" // write back updated value
            "movq %%mm7, %%mm1           \n\t"
            "punpckhbw %%mm0, %%mm3      \n\t" // unpack High bytes of c
            "psllq $24, %%mm1            \n\t" // shift bytes (was _ShiftBpp)
                                    // now mm1 will be used as Raw(x-bpp)
            // now do Paeth for 3rd, and final, set of bytes (6-7)
            "pxor %%mm7, %%mm7           \n\t"
            "punpckhbw %%mm0, %%mm1      \n\t" // unpack High bytes of a
            "psubw %%mm3, %%mm4          \n\t"
            // pbv = p - b = (a + b - c) - b = a - c
            "movq %%mm1, %%mm5           \n\t"
            // pcv = p - c = (a + b - c) -c = (a - c) + (b - c) = pav + pbv
            "movq %%mm4, %%mm6           \n\t"
            "psubw %%mm3, %%mm5          \n\t"
            "pxor %%mm0, %%mm0           \n\t"
            "paddw %%mm5, %%mm6          \n\t"

            // pa = abs(p-a) = abs(pav)
            // pb = abs(p-b) = abs(pbv)
            // pc = abs(p-c) = abs(pcv)
            "pcmpgtw %%mm4, %%mm0        \n\t" // create mask pav bytes < 0
            "pcmpgtw %%mm5, %%mm7        \n\t" // create mask pbv bytes < 0
            "pand %%mm4, %%mm0           \n\t" // only pav bytes < 0 in mm7
            "pand %%mm5, %%mm7           \n\t" // only pbv bytes < 0 in mm0
            "psubw %%mm0, %%mm4          \n\t"
            "psubw %%mm7, %%mm5          \n\t"
            "psubw %%mm0, %%mm4          \n\t"
            "psubw %%mm7, %%mm5          \n\t"
            "pxor %%mm0, %%mm0           \n\t"
            "pcmpgtw %%mm6, %%mm0        \n\t" // create mask pcv bytes < 0
            "pand %%mm6, %%mm0           \n\t" // only pav bytes < 0 in mm7
            "psubw %%mm0, %%mm6          \n\t"
            //  test pa <= pb
            "movq %%mm4, %%mm7           \n\t"
            "psubw %%mm0, %%mm6          \n\t"
            "pcmpgtw %%mm5, %%mm7        \n\t" // pa > pb?
            "movq %%mm7, %%mm0           \n\t"
            // use mm0 mask copy to merge a & b
            "pand %%mm0, %%mm2           \n\t"
            // use mm7 mask to merge pa & pb
            "pand %%mm7, %%mm5           \n\t"
            "pandn %%mm1, %%mm0          \n\t"
            "pandn %%mm4, %%mm7          \n\t"
            "paddw %%mm2, %%mm0          \n\t"
            "paddw %%mm5, %%mm7          \n\t"
            //  test  ((pa <= pb)? pa:pb) <= pc
            "pcmpgtw %%mm6, %%mm7        \n\t" // pab > pc?
            "pand %%mm7, %%mm3           \n\t"
            "pandn %%mm0, %%mm7          \n\t"
            "paddw %%mm3, %%mm7          \n\t"
            "pxor %%mm1, %%mm1           \n\t"
            "packuswb %%mm7, %%mm1       \n\t"
            // step ecx to next set of 8 bytes and repeat loop til done
            "addl $8, %%ecx              \n\t"
            "pand " AMASK0_2_6 ", %%mm1  \n\t" // _amask0_2_6 (_ActiveMaskEnd)
            "paddb -8(%1," PCX ",), %%mm1 \n\t" // add Paeth predictor + Raw(x)
            "cmpl %%eax, %%ecx           \n\t" // MMXLength
            "pxor %%mm0, %%mm0           \n\t" // pxor does not affect flags
            "movq %%mm1, -8(%1," PCX ",) \n\t" // write back updated value
                                 // mm1 will be used as Raw(x-bpp) next loop
                           // mm3 ready to be used as Prior(x-bpp) next loop
            "jb paeth_3lp                \n\t"
            RESTORE_rbp

            : "=S" (dummy_value_S),            // output regs (dummy)
              "=D" (dummy_value_D),
              "=c" (dummy_value_c),
              "=a" (dummy_value_a)

            : "0" (prev_row),  // esi/rsi      // input regs
              "1" (row),       // edi/rdi
              "2" (diff),      // ecx
              "3" (MMXLength)  // eax

#if defined(CLOBBER_MMX_REGS_SUPPORTED)
            : "%mm0", "%mm1", "%mm2", "%mm3"   // clobber list
            , "%mm4", "%mm5", "%mm6", "%mm7"
#endif
         );
      }
      break;  // end 3 bpp

      case 4:
      {
         __asm__ __volatile__ (
// preload  "movl diff, %%ecx            \n\t"
// preload  "movl row, %1                \n\t" // edi/rdi
// preload  "movl prev_row, %0           \n\t" // esi/rsi
            "pxor %%mm0, %%mm0           \n\t"
            // prime the pump:  load the first Raw(x-bpp) data set
            "movq -8(%1," PCX ",), %%mm1 \n\t" // only time should need to read
                                               //  a=Raw(x-bpp) bytes
         "paeth_4lp:                     \n\t"
            // do first set of 4 bytes
            "movq -8(%0," PCX ",), %%mm3 \n\t" // read c=Prior(x-bpp) bytes
            "punpckhbw %%mm0, %%mm1      \n\t" // unpack Low bytes of a
            "movq (%0," PCX ",), %%mm2   \n\t" // load b=Prior(x)
            "punpcklbw %%mm0, %%mm2      \n\t" // unpack High bytes of b
            // pav = p - a = (a + b - c) - a = b - c
            "movq %%mm2, %%mm4           \n\t"
            "punpckhbw %%mm0, %%mm3      \n\t" // unpack High bytes of c
            // pbv = p - b = (a + b - c) - b = a - c
            "movq %%mm1, %%mm5           \n\t"
            "psubw %%mm3, %%mm4          \n\t"
            "pxor %%mm7, %%mm7           \n\t"
            // pcv = p - c = (a + b - c) -c = (a - c) + (b - c) = pav + pbv
            "movq %%mm4, %%mm6           \n\t"
            "psubw %%mm3, %%mm5          \n\t"
            // pa = abs(p-a) = abs(pav)
            // pb = abs(p-b) = abs(pbv)
            // pc = abs(p-c) = abs(pcv)
            "pcmpgtw %%mm4, %%mm0        \n\t" // create mask pav bytes < 0
            "paddw %%mm5, %%mm6          \n\t"
            "pand %%mm4, %%mm0           \n\t" // only pav bytes < 0 in mm7
            "pcmpgtw %%mm5, %%mm7        \n\t" // create mask pbv bytes < 0
            "psubw %%mm0, %%mm4          \n\t"
            "pand %%mm5, %%mm7           \n\t" // only pbv bytes < 0 in mm0
            "psubw %%mm0, %%mm4          \n\t"
            "psubw %%mm7, %%mm5          \n\t"
            "pxor %%mm0, %%mm0           \n\t"
            "pcmpgtw %%mm6, %%mm0        \n\t" // create mask pcv bytes < 0
            "pand %%mm6, %%mm0           \n\t" // only pav bytes < 0 in mm7
            "psubw %%mm7, %%mm5          \n\t"
            "psubw %%mm0, %%mm6          \n\t"
            //  test pa <= pb
            "movq %%mm4, %%mm7           \n\t"
            "psubw %%mm0, %%mm6          \n\t"
            "pcmpgtw %%mm5, %%mm7        \n\t" // pa > pb?
            "movq %%mm7, %%mm0           \n\t"
            // use mm7 mask to merge pa & pb
            "pand %%mm7, %%mm5           \n\t"
            // use mm0 mask copy to merge a & b
            "pand %%mm0, %%mm2           \n\t"
            "pandn %%mm4, %%mm7          \n\t"
            "pandn %%mm1, %%mm0          \n\t"
            "paddw %%mm5, %%mm7          \n\t"
            "paddw %%mm2, %%mm0          \n\t"
            //  test  ((pa <= pb)? pa:pb) <= pc
            "pcmpgtw %%mm6, %%mm7        \n\t" // pab > pc?
            "pxor %%mm1, %%mm1           \n\t"
            "pand %%mm7, %%mm3           \n\t"
            "pandn %%mm0, %%mm7          \n\t"
            "paddw %%mm3, %%mm7          \n\t"
            "pxor %%mm0, %%mm0           \n\t"
            "packuswb %%mm1, %%mm7       \n\t"
            "movq (%0," PCX ",), %%mm3   \n\t" // load c=Prior(x-bpp)
            LOAD_GOT_rbp
            "pand " AMASK4_4_0 ", %%mm7  \n\t" // _amask4_4_0 (was _ActiveMask)
            RESTORE_rbp
            "movq %%mm3, %%mm2           \n\t" // load b=Prior(x) step 1
            "paddb (%1," PCX ",), %%mm7  \n\t" // add Paeth predictor + Raw(x)
            "punpcklbw %%mm0, %%mm3      \n\t" // unpack High bytes of c
            "movq %%mm7, (%1," PCX ",)   \n\t" // write back updated value
            "movq %%mm7, %%mm1           \n\t" // now mm1 will be used as
                                               // Raw(x-bpp)
            // do second set of 4 bytes
            "punpckhbw %%mm0, %%mm2      \n\t" // unpack Low bytes of b
            "punpcklbw %%mm0, %%mm1      \n\t" // unpack Low bytes of a
            // pav = p - a = (a + b - c) - a = b - c
            "movq %%mm2, %%mm4           \n\t"
            // pbv = p - b = (a + b - c) - b = a - c
            "movq %%mm1, %%mm5           \n\t"
            "psubw %%mm3, %%mm4          \n\t"
            "pxor %%mm7, %%mm7           \n\t"
            // pcv = p - c = (a + b - c) -c = (a - c) + (b - c) = pav + pbv
            "movq %%mm4, %%mm6           \n\t"
            "psubw %%mm3, %%mm5          \n\t"
            // pa = abs(p-a) = abs(pav)
            // pb = abs(p-b) = abs(pbv)
            // pc = abs(p-c) = abs(pcv)
            "pcmpgtw %%mm4, %%mm0        \n\t" // create mask pav bytes < 0
            "paddw %%mm5, %%mm6          \n\t"
            "pand %%mm4, %%mm0           \n\t" // only pav bytes < 0 in mm7
            "pcmpgtw %%mm5, %%mm7        \n\t" // create mask pbv bytes < 0
            "psubw %%mm0, %%mm4          \n\t"
            "pand %%mm5, %%mm7           \n\t" // only pbv bytes < 0 in mm0
            "psubw %%mm0, %%mm4          \n\t"
            "psubw %%mm7, %%mm5          \n\t"
            "pxor %%mm0, %%mm0           \n\t"
            "pcmpgtw %%mm6, %%mm0        \n\t" // create mask pcv bytes < 0
            "pand %%mm6, %%mm0           \n\t" // only pav bytes < 0 in mm7
            "psubw %%mm7, %%mm5          \n\t"
            "psubw %%mm0, %%mm6          \n\t"
            //  test pa <= pb
            "movq %%mm4, %%mm7           \n\t"
            "psubw %%mm0, %%mm6          \n\t"
            "pcmpgtw %%mm5, %%mm7        \n\t" // pa > pb?
            "movq %%mm7, %%mm0           \n\t"
            // use mm7 mask to merge pa & pb
            "pand %%mm7, %%mm5           \n\t"
            // use mm0 mask copy to merge a & b
            "pand %%mm0, %%mm2           \n\t"
            "pandn %%mm4, %%mm7          \n\t"
            "pandn %%mm1, %%mm0          \n\t"
            "paddw %%mm5, %%mm7          \n\t"
            "paddw %%mm2, %%mm0          \n\t"
            //  test  ((pa <= pb)? pa:pb) <= pc
            "pcmpgtw %%mm6, %%mm7        \n\t" // pab > pc?
            "pxor %%mm1, %%mm1           \n\t"
            "pand %%mm7, %%mm3           \n\t"
            "pandn %%mm0, %%mm7          \n\t"
            "pxor %%mm1, %%mm1           \n\t"
            "paddw %%mm3, %%mm7          \n\t"
            "pxor %%mm0, %%mm0           \n\t"
            // step ecx to next set of 8 bytes and repeat loop til done
            "addl $8, %%ecx              \n\t"
            "packuswb %%mm7, %%mm1       \n\t"
            "paddb -8(%1," PCX ",), %%mm1 \n\t" // add predictor with Raw(x)
            "cmpl %%eax, %%ecx           \n\t" // MMXLength
            "movq %%mm1, -8(%1," PCX ",) \n\t" // write back updated value
                                 // mm1 will be used as Raw(x-bpp) next loop
            "jb paeth_4lp                \n\t"

            : "=S" (dummy_value_S),            // output regs (dummy)
              "=D" (dummy_value_D),
              "=c" (dummy_value_c),
              "=a" (dummy_value_a)

            : "0" (prev_row),  // esi/rsi      // input regs
              "1" (row),       // edi/rdi
              "2" (diff),      // ecx
              "3" (MMXLength)  // eax

#if defined(CLOBBER_MMX_REGS_SUPPORTED)
            : "%mm0", "%mm1", "%mm2", "%mm3"   // clobber list
            , "%mm4", "%mm5", "%mm6", "%mm7"
#endif
         );
      }
      break;  // end 4 bpp

      case 1:
      case 2:
      {
         __asm__ __volatile__ (
// preload  "movl diff, %%eax            \n\t" // eax: x = offset to align. bdry
// preload  "movl FullLength, %%edx      \n\t"
            "cmpl %%edx, %%eax           \n\t"
            "jnb paeth_dend              \n\t"

            SAVE_ebp

// preload  "movl row, %2                \n\t" // edi/rdi
            // do Paeth decode for remaining bytes
// preload  "movl prev_row, %1           \n\t" // esi/rsi
            "movl %%eax, %%ebp           \n\t"
// preload  "subl bpp, %%ebp             \n\t" // (bpp is preloaded into ecx)
            "subl %%ecx, %%ebp           \n\t" // ebp = eax - bpp
            "xorl %%ecx, %%ecx           \n\t" // zero ecx before using cl & cx

            SAVE_GOT_ebx
            SAVE_r11_r12_r13

         "paeth_dlp:                     \n\t"
            "xorl %%ebx, %%ebx           \n\t"
            // pav = p - a = (a + b - c) - a = b - c
            "movb (%1," PAX ",), %%bl    \n\t" // load Prior(x) into bl
            "movb (%1," PBP ",), %%cl    \n\t" // load Prior(x-bpp) into cl
            "subl %%ecx, %%ebx           \n\t" // subtract Prior(x-bpp)
            "movl %%ebx, " pa_TEMP "     \n\t" // Save pav for later use
            "xorl %%ebx, %%ebx           \n\t"
            // pbv = p - b = (a + b - c) - b = a - c
            "movb (%2," PBP ",), %%bl    \n\t" // load Raw(x-bpp) into bl
            "subl %%ecx, %%ebx           \n\t" // subtract Prior(x-bpp)
            "movl %%ebx, %%ecx           \n\t"
            // pcv = p - c = (a + b - c) -c = (a - c) + (b - c) = pav + pbv
            "addl " pa_TEMP ", %%ebx     \n\t" // pcv = pav + pbv
            // pc = abs(pcv)
            "testl $0x80000000, %%ebx    \n\t"
            "jz paeth_dpca               \n\t"
            "negl %%ebx                  \n\t" // reverse sign of neg values

         "paeth_dpca:                    \n\t"
            "movl %%ebx, " pc_TEMP "     \n\t" // save pc for later use
            // pb = abs(pbv)
            "testl $0x80000000, %%ecx    \n\t"
            "jz paeth_dpba               \n\t"
            "negl %%ecx                  \n\t" // reverse sign of neg values

         "paeth_dpba:                    \n\t"
            "movl %%ecx, " pb_TEMP "     \n\t" // save pb for later use
            // pa = abs(pav)
            "movl " pa_TEMP ", %%ebx     \n\t"
            "testl $0x80000000, %%ebx    \n\t"
            "jz paeth_dpaa               \n\t"
            "negl %%ebx                  \n\t" // reverse sign of neg values

         "paeth_dpaa:                    \n\t"
            "movl %%ebx, " pa_TEMP "     \n\t" // save pa for later use
            // test if pa <= pb
            "cmpl %%ecx, %%ebx           \n\t"
            "jna paeth_dabb              \n\t"
            // pa > pb; now test if pb <= pc
            "cmpl " pc_TEMP ", %%ecx     \n\t"
            "jna paeth_dbbc              \n\t"
            // pb > pc; Raw(x) = Paeth(x) + Prior(x-bpp)
            "movb (%1," PBP ",), %%cl    \n\t" // load Prior(x-bpp) into cl
            "jmp paeth_dpaeth            \n\t"

         "paeth_dbbc:                    \n\t"
            // pb <= pc; Raw(x) = Paeth(x) + Prior(x)
            "movb (%1," PAX ",), %%cl    \n\t" // load Prior(x) into cl
            "jmp paeth_dpaeth            \n\t"

         "paeth_dabb:                    \n\t"
            // pa <= pb; now test if pa <= pc
            "cmpl " pc_TEMP ", %%ebx     \n\t"
            "jna paeth_dabc              \n\t"
            // pa > pc; Raw(x) = Paeth(x) + Prior(x-bpp)
            "movb (%1," PBP ",), %%cl   \n\t" // load Prior(x-bpp) into cl
            "jmp paeth_dpaeth            \n\t"

         "paeth_dabc:                    \n\t"
            // pa <= pc; Raw(x) = Paeth(x) + Raw(x-bpp)
            "movb (%2," PBP ",), %%cl    \n\t" // load Raw(x-bpp) into cl

         "paeth_dpaeth:                  \n\t"
            "incl %%eax                  \n\t"
            "incl %%ebp                  \n\t"
            // Raw(x) = (Paeth(x) + Paeth_Predictor( a, b, c )) mod 256
            "addb %%cl, -1(%2," PAX ",)  \n\t"
            "cmpl %%edx, %%eax           \n\t" // check against FullLength
            "jb paeth_dlp                \n\t"

            RESTORE_r11_r12_r13
            RESTORE_GOT_ebx
            RESTORE_ebp

         "paeth_dend:                    \n\t"

            : "=c" (dummy_value_c),            // output regs (dummy)
              "=S" (dummy_value_S),
              "=D" (dummy_value_D),
              "=a" (dummy_value_a),
              "=d" (dummy_value_d)

            : "0" (bpp),         // ecx        // input regs
              "1" (prev_row),    // esi/rsi
              "2" (row),         // edi/rdi
              "3" (diff),        // eax
              "4" (FullLength)   // edx

            CLOB_COLON_ebx_ebp_r1X             // clobber list
              CLOBBER_GOT_ebx
              CLOB_COMMA_ebx_ebp
              CLOBBER_ebp
              CLOB_COMMA_ebX_r1X
              CLOBBER_r11_r12_r13
         );
      }
      return; // end 1 or 2 bpp (no need to go further with this one)

      case 6:
      {
//       _ActiveMask2 = 0xffffffff00000000LL;  // NOT USED ("_amask_0_4_4")
//       _ShiftBpp = 48;       // bpp << 3 == bpp * 8
//       _ShiftRem = 16;       // 64 - _ShiftBpp

         __asm__ __volatile__ (
// preload  "movl diff, %%ecx            \n\t"
// preload  "movl row, %1                \n\t" // edi/rdi
// preload  "movl prev_row, %0           \n\t" // esi/rsi
            // prime the pump:  load the first Raw(x-bpp) data set
            "movq -8(%1," PCX ",), %%mm1 \n\t"
            "pxor %%mm0, %%mm0           \n\t"

         "paeth_6lp:                     \n\t"
            // must shift to position Raw(x-bpp) data
            "psrlq $16, %%mm1            \n\t" // was _ShiftRem
            // do first set of 4 bytes
            "movq -8(%0," PCX ",), %%mm3 \n\t" // read c=Prior(x-bpp) bytes
            "punpcklbw %%mm0, %%mm1      \n\t" // unpack Low bytes of a
            "movq (%0," PCX ",), %%mm2   \n\t" // load b=Prior(x)
            "punpcklbw %%mm0, %%mm2      \n\t" // unpack Low bytes of b
            // must shift to position Prior(x-bpp) data
            "psrlq $16, %%mm3            \n\t" // was _ShiftRem
            // pav = p - a = (a + b - c) - a = b - c
            "movq %%mm2, %%mm4           \n\t"
            "punpcklbw %%mm0, %%mm3      \n\t" // unpack Low bytes of c
            // pbv = p - b = (a + b - c) - b = a - c
            "movq %%mm1, %%mm5           \n\t"
            "psubw %%mm3, %%mm4          \n\t"
            "pxor %%mm7, %%mm7           \n\t"
            // pcv = p - c = (a + b - c) -c = (a - c) + (b - c) = pav + pbv
            "movq %%mm4, %%mm6           \n\t"
            "psubw %%mm3, %%mm5          \n\t"
            // pa = abs(p-a) = abs(pav)
            // pb = abs(p-b) = abs(pbv)
            // pc = abs(p-c) = abs(pcv)
            "pcmpgtw %%mm4, %%mm0        \n\t" // create mask pav bytes < 0
            "paddw %%mm5, %%mm6          \n\t"
            "pand %%mm4, %%mm0           \n\t" // only pav bytes < 0 in mm7
            "pcmpgtw %%mm5, %%mm7        \n\t" // create mask pbv bytes < 0
            "psubw %%mm0, %%mm4          \n\t"
            "pand %%mm5, %%mm7           \n\t" // only pbv bytes < 0 in mm0
            "psubw %%mm0, %%mm4          \n\t"
            "psubw %%mm7, %%mm5          \n\t"
            "pxor %%mm0, %%mm0           \n\t"
            "pcmpgtw %%mm6, %%mm0        \n\t" // create mask pcv bytes < 0
            "pand %%mm6, %%mm0           \n\t" // only pav bytes < 0 in mm7
            "psubw %%mm7, %%mm5          \n\t"
            "psubw %%mm0, %%mm6          \n\t"
            //  test pa <= pb
            "movq %%mm4, %%mm7           \n\t"
            "psubw %%mm0, %%mm6          \n\t"
            "pcmpgtw %%mm5, %%mm7        \n\t" // pa > pb?
            "movq %%mm7, %%mm0           \n\t"
            // use mm7 mask to merge pa & pb
            "pand %%mm7, %%mm5           \n\t"
            // use mm0 mask copy to merge a & b
            "pand %%mm0, %%mm2           \n\t"
            "pandn %%mm4, %%mm7          \n\t"
            "pandn %%mm1, %%mm0          \n\t"
            "paddw %%mm5, %%mm7          \n\t"
            "paddw %%mm2, %%mm0          \n\t"
            //  test  ((pa <= pb)? pa:pb) <= pc
            "pcmpgtw %%mm6, %%mm7        \n\t" // pab > pc?
            "pxor %%mm1, %%mm1           \n\t"
            "pand %%mm7, %%mm3           \n\t"
            "pandn %%mm0, %%mm7          \n\t"
            "paddw %%mm3, %%mm7          \n\t"
            "pxor %%mm0, %%mm0           \n\t"
            "packuswb %%mm1, %%mm7       \n\t"
            "movq -8(%0," PCX ",), %%mm3 \n\t" // load c=Prior(x-bpp)
            LOAD_GOT_rbp
            "pand " AMASK4_4_0 ", %%mm7  \n\t" // _amask4_4_0 (was _ActiveMask)
            RESTORE_rbp
            "psrlq $16, %%mm3            \n\t"
            "movq (%0," PCX ",), %%mm2   \n\t" // load b=Prior(x) step 1
            "paddb (%1," PCX ",), %%mm7  \n\t" // add Paeth predictor + Raw(x)
            "movq %%mm2, %%mm6           \n\t"
            "movq %%mm7, (%1," PCX ",)   \n\t" // write back updated value
            "movq -8(%1," PCX ",), %%mm1 \n\t"
            "psllq $48, %%mm6            \n\t" // bpp * 8 = bits per pixel
            "movq %%mm7, %%mm5           \n\t"
            "psrlq $16, %%mm1            \n\t" // 64 - (bpp * 8) = remainder
            "por %%mm6, %%mm3            \n\t"
            "psllq $48, %%mm5            \n\t" // was _ShiftBpp
            "punpckhbw %%mm0, %%mm3      \n\t" // unpack High bytes of c
            "por %%mm5, %%mm1            \n\t"
            // do second set of 4 bytes
            "punpckhbw %%mm0, %%mm2      \n\t" // unpack High bytes of b
            "punpckhbw %%mm0, %%mm1      \n\t" // unpack High bytes of a
            // pav = p - a = (a + b - c) - a = b - c
            "movq %%mm2, %%mm4           \n\t"
            // pbv = p - b = (a + b - c) - b = a - c
            "movq %%mm1, %%mm5           \n\t"
            "psubw %%mm3, %%mm4          \n\t"
            "pxor %%mm7, %%mm7           \n\t"
            // pcv = p - c = (a + b - c) -c = (a - c) + (b - c) = pav + pbv
            "movq %%mm4, %%mm6           \n\t"
            "psubw %%mm3, %%mm5          \n\t"
            // pa = abs(p-a) = abs(pav)
            // pb = abs(p-b) = abs(pbv)
            // pc = abs(p-c) = abs(pcv)
            "pcmpgtw %%mm4, %%mm0        \n\t" // create mask pav bytes < 0
            "paddw %%mm5, %%mm6          \n\t"
            "pand %%mm4, %%mm0           \n\t" // only pav bytes < 0 in mm7
            "pcmpgtw %%mm5, %%mm7        \n\t" // create mask pbv bytes < 0
            "psubw %%mm0, %%mm4          \n\t"
            "pand %%mm5, %%mm7           \n\t" // only pbv bytes < 0 in mm0
            "psubw %%mm0, %%mm4          \n\t"
            "psubw %%mm7, %%mm5          \n\t"
            "pxor %%mm0, %%mm0           \n\t"
            "pcmpgtw %%mm6, %%mm0        \n\t" // create mask pcv bytes < 0
            "pand %%mm6, %%mm0           \n\t" // only pav bytes < 0 in mm7
            "psubw %%mm7, %%mm5          \n\t"
            "psubw %%mm0, %%mm6          \n\t"
            //  test pa <= pb
            "movq %%mm4, %%mm7           \n\t"
            "psubw %%mm0, %%mm6          \n\t"
            "pcmpgtw %%mm5, %%mm7        \n\t" // pa > pb?
            "movq %%mm7, %%mm0           \n\t"
            // use mm7 mask to merge pa & pb
            "pand %%mm7, %%mm5           \n\t"
            // use mm0 mask copy to merge a & b
            "pand %%mm0, %%mm2           \n\t"
            "pandn %%mm4, %%mm7          \n\t"
            "pandn %%mm1, %%mm0          \n\t"
            "paddw %%mm5, %%mm7          \n\t"
            "paddw %%mm2, %%mm0          \n\t"
            //  test  ((pa <= pb)? pa:pb) <= pc
            "pcmpgtw %%mm6, %%mm7        \n\t" // pab > pc?
            "pxor %%mm1, %%mm1           \n\t"
            "pand %%mm7, %%mm3           \n\t"
            "pandn %%mm0, %%mm7          \n\t"
            "pxor %%mm1, %%mm1           \n\t"
            "paddw %%mm3, %%mm7          \n\t"
            "pxor %%mm0, %%mm0           \n\t"
            // step ecx to next set of 8 bytes and repeat loop til done
            "addl $8, %%ecx              \n\t"
            "packuswb %%mm7, %%mm1       \n\t"
            "paddb -8(%1," PCX ",), %%mm1 \n\t" // add Paeth predictor + Raw(x)
            "cmpl %%eax, %%ecx           \n\t" // MMXLength
            "movq %%mm1, -8(%1," PCX ",) \n\t" // write back updated value
                                 // mm1 will be used as Raw(x-bpp) next loop
            "jb paeth_6lp                \n\t"

            : "=S" (dummy_value_S),            // output regs (dummy)
              "=D" (dummy_value_D),
              "=c" (dummy_value_c),
              "=a" (dummy_value_a)

            : "0" (prev_row),  // esi/rsi      // input regs
              "1" (row),       // edi/rdi
              "2" (diff),      // ecx
              "3" (MMXLength)  // eax

#if defined(CLOBBER_MMX_REGS_SUPPORTED)
            : "%mm0", "%mm1", "%mm2", "%mm3"   // clobber list
            , "%mm4", "%mm5", "%mm6", "%mm7"
#endif
         );
      }
      break;  // end 6 bpp

      case 8:                          // bpp == 8
      {
         __asm__ __volatile__ (
// preload  "movl diff, %%ecx            \n\t"
// preload  "movl row, %1                \n\t" // edi/rdi
// preload  "movl prev_row, %0           \n\t" // esi/rsi
            "pxor %%mm0, %%mm0           \n\t"
            // prime the pump:  load the first Raw(x-bpp) data set
            "movq -8(%1," PCX ",), %%mm1 \n\t" // only time should need to read
                                               //  a=Raw(x-bpp) bytes
         "paeth_8lp:                     \n\t"
            // do first set of 4 bytes
            "movq -8(%0," PCX ",), %%mm3 \n\t" // read c=Prior(x-bpp) bytes
            "punpcklbw %%mm0, %%mm1      \n\t" // unpack Low bytes of a
            "movq (%0," PCX ",), %%mm2   \n\t" // load b=Prior(x)
            "punpcklbw %%mm0, %%mm2      \n\t" // unpack Low bytes of b
            // pav = p - a = (a + b - c) - a = b - c
            "movq %%mm2, %%mm4           \n\t"
            "punpcklbw %%mm0, %%mm3      \n\t" // unpack Low bytes of c
            // pbv = p - b = (a + b - c) - b = a - c
            "movq %%mm1, %%mm5           \n\t"
            "psubw %%mm3, %%mm4          \n\t"
            "pxor %%mm7, %%mm7           \n\t"
            // pcv = p - c = (a + b - c) -c = (a - c) + (b - c) = pav + pbv
            "movq %%mm4, %%mm6           \n\t"
            "psubw %%mm3, %%mm5          \n\t"
            // pa = abs(p-a) = abs(pav)
            // pb = abs(p-b) = abs(pbv)
            // pc = abs(p-c) = abs(pcv)
            "pcmpgtw %%mm4, %%mm0        \n\t" // create mask pav bytes < 0
            "paddw %%mm5, %%mm6          \n\t"
            "pand %%mm4, %%mm0           \n\t" // only pav bytes < 0 in mm7
            "pcmpgtw %%mm5, %%mm7        \n\t" // create mask pbv bytes < 0
            "psubw %%mm0, %%mm4          \n\t"
            "pand %%mm5, %%mm7           \n\t" // only pbv bytes < 0 in mm0
            "psubw %%mm0, %%mm4          \n\t"
            "psubw %%mm7, %%mm5          \n\t"
            "pxor %%mm0, %%mm0           \n\t"
            "pcmpgtw %%mm6, %%mm0        \n\t" // create mask pcv bytes < 0
            "pand %%mm6, %%mm0           \n\t" // only pav bytes < 0 in mm7
            "psubw %%mm7, %%mm5          \n\t"
            "psubw %%mm0, %%mm6          \n\t"
            //  test pa <= pb
            "movq %%mm4, %%mm7           \n\t"
            "psubw %%mm0, %%mm6          \n\t"
            "pcmpgtw %%mm5, %%mm7        \n\t" // pa > pb?
            "movq %%mm7, %%mm0           \n\t"
            // use mm7 mask to merge pa & pb
            "pand %%mm7, %%mm5           \n\t"
            // use mm0 mask copy to merge a & b
            "pand %%mm0, %%mm2           \n\t"
            "pandn %%mm4, %%mm7          \n\t"
            "pandn %%mm1, %%mm0          \n\t"
            "paddw %%mm5, %%mm7          \n\t"
            "paddw %%mm2, %%mm0          \n\t"
            //  test  ((pa <= pb)? pa:pb) <= pc
            "pcmpgtw %%mm6, %%mm7        \n\t" // pab > pc?
            "pxor %%mm1, %%mm1           \n\t"
            "pand %%mm7, %%mm3           \n\t"
            "pandn %%mm0, %%mm7          \n\t"
            "paddw %%mm3, %%mm7          \n\t"
            "pxor %%mm0, %%mm0           \n\t"
            "packuswb %%mm1, %%mm7       \n\t"
            "movq -8(%0," PCX ",), %%mm3 \n\t" // read c=Prior(x-bpp) bytes
            LOAD_GOT_rbp
            "pand " AMASK4_4_0 ", %%mm7  \n\t" // _amask4_4_0 (was _ActiveMask)
            RESTORE_rbp
            "movq (%0," PCX ",), %%mm2   \n\t" // load b=Prior(x)
            "paddb (%1," PCX ",), %%mm7  \n\t" // add Paeth predictor + Raw(x)
            "punpckhbw %%mm0, %%mm3      \n\t" // unpack High bytes of c
            "movq %%mm7, (%1," PCX ",)   \n\t" // write back updated value
            "movq -8(%1," PCX ",), %%mm1 \n\t" // read a=Raw(x-bpp) bytes

            // do second set of 4 bytes
            "punpckhbw %%mm0, %%mm2      \n\t" // unpack High bytes of b
            "punpckhbw %%mm0, %%mm1      \n\t" // unpack High bytes of a
            // pav = p - a = (a + b - c) - a = b - c
            "movq %%mm2, %%mm4           \n\t"
            // pbv = p - b = (a + b - c) - b = a - c
            "movq %%mm1, %%mm5           \n\t"
            "psubw %%mm3, %%mm4          \n\t"
            "pxor %%mm7, %%mm7           \n\t"
            // pcv = p - c = (a + b - c) -c = (a - c) + (b - c) = pav + pbv
            "movq %%mm4, %%mm6           \n\t"
            "psubw %%mm3, %%mm5          \n\t"
            // pa = abs(p-a) = abs(pav)
            // pb = abs(p-b) = abs(pbv)
            // pc = abs(p-c) = abs(pcv)
            "pcmpgtw %%mm4, %%mm0        \n\t" // create mask pav bytes < 0
            "paddw %%mm5, %%mm6          \n\t"
            "pand %%mm4, %%mm0           \n\t" // only pav bytes < 0 in mm7
            "pcmpgtw %%mm5, %%mm7        \n\t" // create mask pbv bytes < 0
            "psubw %%mm0, %%mm4          \n\t"
            "pand %%mm5, %%mm7           \n\t" // only pbv bytes < 0 in mm0
            "psubw %%mm0, %%mm4          \n\t"
            "psubw %%mm7, %%mm5          \n\t"
            "pxor %%mm0, %%mm0           \n\t"
            "pcmpgtw %%mm6, %%mm0        \n\t" // create mask pcv bytes < 0
            "pand %%mm6, %%mm0           \n\t" // only pav bytes < 0 in mm7
            "psubw %%mm7, %%mm5          \n\t"
            "psubw %%mm0, %%mm6          \n\t"
            //  test pa <= pb
            "movq %%mm4, %%mm7           \n\t"
            "psubw %%mm0, %%mm6          \n\t"
            "pcmpgtw %%mm5, %%mm7        \n\t" // pa > pb?
            "movq %%mm7, %%mm0           \n\t"
            // use mm7 mask to merge pa & pb
            "pand %%mm7, %%mm5           \n\t"
            // use mm0 mask copy to merge a & b
            "pand %%mm0, %%mm2           \n\t"
            "pandn %%mm4, %%mm7          \n\t"
            "pandn %%mm1, %%mm0          \n\t"
            "paddw %%mm5, %%mm7          \n\t"
            "paddw %%mm2, %%mm0          \n\t"
            //  test  ((pa <= pb)? pa:pb) <= pc
            "pcmpgtw %%mm6, %%mm7        \n\t" // pab > pc?
            "pxor %%mm1, %%mm1           \n\t"
            "pand %%mm7, %%mm3           \n\t"
            "pandn %%mm0, %%mm7          \n\t"
            "pxor %%mm1, %%mm1           \n\t"
            "paddw %%mm3, %%mm7          \n\t"
            "pxor %%mm0, %%mm0           \n\t"
            // step ecx to next set of 8 bytes and repeat loop til done
            "addl $8, %%ecx              \n\t"
            "packuswb %%mm7, %%mm1       \n\t"
            "paddb -8(%1," PCX ",), %%mm1 \n\t" // add Paeth predictor + Raw(x)
            "cmpl %%eax, %%ecx           \n\t" // MMXLength
            "movq %%mm1, -8(%1," PCX ",) \n\t" // write back updated value
                                 // mm1 will be used as Raw(x-bpp) next loop
            "jb paeth_8lp                \n\t"

            : "=S" (dummy_value_S),            // output regs (dummy)
              "=D" (dummy_value_D),
              "=c" (dummy_value_c),
              "=a" (dummy_value_a)

            : "0" (prev_row),  // esi/rsi      // input regs
              "1" (row),       // edi/rdi
              "2" (diff),      // ecx
              "3" (MMXLength)  // eax

#if defined(CLOBBER_MMX_REGS_SUPPORTED)
            : "%mm0", "%mm1", "%mm2", "%mm3"   // clobber list
            , "%mm4", "%mm5", "%mm6", "%mm7"
#endif
         );
      }
      break;  // end 8 bpp

      default:                // bpp != 1,2,3,4,6,8:  doesn't exist
      {
         // ERROR:  SHOULD NEVER BE REACHED
#if defined(PNG_DEBUG)
         png_debug(1, "Internal libpng logic error (GCC "
           "png_read_filter_row_mmx_paeth())\n");
#endif
      }
      break;

   } // end switch (bpp)

   __asm__ __volatile__ (
      // MMX acceleration complete; now do clean-up
      // check if any remaining bytes left to decode
//pre "movl FullLength, %%edx      \n\t"
//pre "movl MMXLength, %%eax       \n\t"
      "cmpl %%edx, %%eax           \n\t"
      "jnb paeth_end               \n\t"

      SAVE_ebp

//pre "movl row, %2                \n\t" // edi/rdi
//pre "movl prev_row, %1           \n\t" // esi/rsi
      // do Paeth decode for remaining bytes
      "movl %%eax, %%ebp           \n\t"
//pre "subl bpp, %%ebp             \n\t" // (bpp is preloaded into ecx)
      "subl %%ecx, %%ebp           \n\t" // ebp = eax - bpp
      "xorl %%ecx, %%ecx           \n\t" // zero ecx before using cl & cx below

      SAVE_GOT_ebx
      SAVE_r11_r12_r13

   "paeth_lp2:                     \n\t"
      "xorl %%ebx, %%ebx           \n\t"
      // pav = p - a = (a + b - c) - a = b - c
      "movb (%1," PAX ",), %%bl    \n\t" // load Prior(x) into bl
      "movb (%1," PBP ",), %%cl    \n\t" // load Prior(x-bpp) into cl
      "subl %%ecx, %%ebx           \n\t" // subtract Prior(x-bpp)
      "movl %%ebx, " pa_TEMP "     \n\t" // Save pav for later use
      "xorl %%ebx, %%ebx           \n\t"
      // pbv = p - b = (a + b - c) - b = a - c
      "movb (%2," PBP ",), %%bl    \n\t" // load Raw(x-bpp) into bl
      "subl %%ecx, %%ebx           \n\t" // subtract Prior(x-bpp)
      "movl %%ebx, %%ecx           \n\t"
      // pcv = p - c = (a + b - c) - c = (a - c) + (b - c) = pav + pbv
      "addl " pa_TEMP ", %%ebx     \n\t" // pcv = pav + pbv
      // pc = abs(pcv)
      "testl $0x80000000, %%ebx    \n\t"
      "jz paeth_pca2               \n\t"
      "negl %%ebx                  \n\t" // reverse sign of neg values

   "paeth_pca2:                    \n\t"
      "movl %%ebx, " pc_TEMP "     \n\t" // save pc for later use
      // pb = abs(pbv)
      "testl $0x80000000, %%ecx    \n\t"
      "jz paeth_pba2               \n\t"
      "negl %%ecx                  \n\t" // reverse sign of neg values

   "paeth_pba2:                    \n\t"
      "movl %%ecx, " pb_TEMP "     \n\t" // save pb for later use
      // pa = abs(pav)
      "movl " pa_TEMP ", %%ebx     \n\t"
      "testl $0x80000000, %%ebx    \n\t"
      "jz paeth_paa2               \n\t"
      "negl %%ebx                  \n\t" // reverse sign of neg values

   "paeth_paa2:                    \n\t"
      "movl %%ebx, " pa_TEMP "     \n\t" // save pa for later use
      // test if pa <= pb
      "cmpl %%ecx, %%ebx           \n\t"
      "jna paeth_abb2              \n\t"
      // pa > pb; now test if pb <= pc
      "cmpl " pc_TEMP ", %%ecx     \n\t"
      "jna paeth_bbc2              \n\t"
      // pb > pc; Raw(x) = Paeth(x) + Prior(x-bpp)
      "movb (%1," PBP ",), %%cl    \n\t" // load Prior(x-bpp) into cl
      "jmp paeth_paeth2            \n\t"

   "paeth_bbc2:                    \n\t"
      // pb <= pc; Raw(x) = Paeth(x) + Prior(x)
      "movb (%1," PAX ",), %%cl    \n\t" // load Prior(x) into cl
      "jmp paeth_paeth2            \n\t"

   "paeth_abb2:                    \n\t"
      // pa <= pb; now test if pa <= pc
      "cmpl " pc_TEMP ", %%ebx     \n\t"
      "jna paeth_abc2              \n\t"
      // pa > pc; Raw(x) = Paeth(x) + Prior(x-bpp)
      "movb (%1," PBP ",), %%cl    \n\t" // load Prior(x-bpp) into cl
      "jmp paeth_paeth2            \n\t"

   "paeth_abc2:                    \n\t"
      // pa <= pc; Raw(x) = Paeth(x) + Raw(x-bpp)
      "movb (%2," PBP ",), %%cl    \n\t" // load Raw(x-bpp) into cl

   "paeth_paeth2:                  \n\t"
      "incl %%eax                  \n\t"
      "incl %%ebp                  \n\t"
      // Raw(x) = (Paeth(x) + Paeth_Predictor( a, b, c )) mod 256
      "addb %%cl, -1(%2," PAX ",)  \n\t"
      "cmpl %%edx, %%eax           \n\t" // check against FullLength
      "jb paeth_lp2                \n\t"

      RESTORE_r11_r12_r13
      RESTORE_GOT_ebx
      RESTORE_ebp

   "paeth_end:                     \n\t"
      "EMMS                        \n\t" // end MMX; prep for poss. FP instrs.

      : "=c" (dummy_value_c),            // output regs (dummy)
        "=S" (dummy_value_S),
        "=D" (dummy_value_D),
        "=a" (dummy_value_a),
        "=d" (dummy_value_d)

      : "0" (bpp),         // ecx        // input regs
        "1" (prev_row),    // esi/rsi
        "2" (row),         // edi/rdi
        "3" (MMXLength),   // eax
        "4" (FullLength)   // edx

      CLOB_COLON_ebx_ebp_r1X             // clobber list
        CLOBBER_GOT_ebx
        CLOB_COMMA_ebx_ebp
        CLOBBER_ebp
        CLOB_COMMA_ebX_r1X
        CLOBBER_r11_r12_r13
   );

} /* end png_read_filter_row_mmx_paeth() */

#endif // PNG_x86_64_USE_GOTPCREL || PNG_THREAD_UNSAFE_OK
#endif /* PNG_MMX_READ_FILTER_PAETH_SUPPORTED */




#if defined(PNG_MMX_READ_FILTER_SUB_SUPPORTED)

//===========================================================================//
//                                                                           //
//           P N G _ R E A D _ F I L T E R _ R O W _ M M X _ S U B           //
//                                                                           //
//===========================================================================//

// Optimized code for PNG Sub filter decoder

static void /* PRIVATE */
png_read_filter_row_mmx_sub(png_row_infop row_info, png_bytep row)
{
   unsigned FullLength, MMXLength;  // png_uint_32 is actually 64-bit on x86-64
   int bpp;
   int dummy_value_a;
   int dummy_value_c;
   int dummy_value_d;
   png_bytep dummy_value_D;
   int diff; //     __attribute__((used));

   bpp = (row_info->pixel_depth + 7) >> 3;  // calc number of bytes per pixel
   FullLength = row_info->rowbytes - bpp;   // number of bytes to filter
     // (why do we subtract off bpp?  not so in avg or paeth...)

   __asm__ __volatile__ (
      SAVE_r15
      SAVE_ebp
//pre "movl row, %1                \n\t" // edi/rdi
      "mov  %1, " PSI "            \n\t" // lp = row
//pre "movl bpp, %%ecx             \n\t"
      "add  " PCX ", %1            \n\t" // rp = row + bpp
//pre "movl FullLength, %%eax      \n\t" // bring in via eax...
      SAVE_FullLength                    // ...but store for later use

      "xorl %%eax, %%eax           \n\t"

      // get # of bytes to alignment (note:  computing _delta_ of two pointers,
      // so hereafter %%ebp is sufficient even on 64-bit)
      "mov  %1, " PBP "            \n\t" // take start of row
      "add  $0xf, " PBP "          \n\t" // add 7+8 to incr past alignment bdry
//    "andl $0xfffffff8, %%ebp     \n\t" // mask to alignment boundary (32-bit!)
      CLEAR_BOTTOM_3_BITS  PBP    "\n\t" // mask to alignment boundary
      "sub  %1, " PBP "            \n\t" // subtract row ptr again => ebp =
      "jz sub_go                   \n\t" //  target value of eax at alignment

   "sub_lp1:                       \n\t" // fix alignment
      "movb (" PSI "," PAX ",), %%cl \n\t"
      "addb %%cl, (%1," PAX ",)    \n\t"
      "incl %%eax                  \n\t"
      "cmpl %%ebp, %%eax           \n\t"
      "jb sub_lp1                  \n\t"

   "sub_go:                        \n\t"
      RESTORE_FullLength "%%ecx    \n\t" // FullLength -> ecx
      "movl %%ecx, %%edx           \n\t"
      "subl %%eax, %%edx           \n\t" // subtract alignment fix
      "andl $0x00000007, %%edx     \n\t" // calc bytes over mult of 8
      "subl %%edx, %%ecx           \n\t" // drop over bytes from length
//out "movl %%ecx, MMXLength       \n\t"
      "movl %%ebp, %%eax           \n\t" // ebp = diff, but no reg constraint(?)
      RESTORE_ebp                        //  (could swap ebp and ecx functions,
      RESTORE_r15                        //  but %%cl issues...)

      : "=c" (MMXLength),       // 0     // output regs
        "=D" (dummy_value_D),   // 1
        "=a" (diff)             // 2

      : "0" (bpp),              // ecx   // input regs
        "1" (row),              // edi
        "2" (FullLength)        // eax

      : "%esi", "%edx"                   // clobber list
        _CLOBBER_r15
        _CLOBBER_ebp
   );

   // now do the math for the rest of the row
   switch (bpp)
   {
      case 3:
      {
//       _ShiftBpp = 24;       // == 3 * 8
//       _ShiftRem  = 40;      // == 64 - 24

         __asm__ __volatile__ (
// preload  "mov  row, %1                 \n\t" // edi/rdi
            LOAD_GOT_rbp
            // load (former) _ActiveMask for 2nd active byte group
            "movq " AMASK2_3_3 ", %%mm7   \n\t" // _amask2_3_3
            RESTORE_rbp

// notused  "mov  %1, " PSI "             \n\t" // lp = row
// preload  "movl bpp, %%ecx              \n\t"
            "add  " PCX ", %1             \n\t" // rp = row + bpp
            "movq %%mm7, %%mm6            \n\t"
// preload  "movl diff, %%edx             \n\t"
            "psllq $24, %%mm6             \n\t" // move mask in mm6 to cover
                                                //  3rd active byte group
            // prime the pump:  load the first Raw(x-bpp) data set
            "movq -8(%1," PDX ",), %%mm1  \n\t"

         "sub_3lp:                        \n\t" // shift data for adding first
            "psrlq $40, %%mm1             \n\t" //  bpp bytes (no need for mask;
                                                //  shift clears inactive bytes)
            // add 1st active group
            "movq (%1," PDX ",), %%mm0    \n\t"
            "paddb %%mm1, %%mm0           \n\t"

            // add 2nd active group
            "movq %%mm0, %%mm1            \n\t" // mov updated Raws to mm1
            "psllq $24, %%mm1             \n\t" // shift data to pos. correctly
            "pand %%mm7, %%mm1            \n\t" // mask to use 2nd active group
            "paddb %%mm1, %%mm0           \n\t"

            // add 3rd active group
            "movq %%mm0, %%mm1            \n\t" // mov updated Raws to mm1
            "psllq $24, %%mm1             \n\t" // shift data to pos. correctly
            "pand %%mm6, %%mm1            \n\t" // mask to use 3rd active group
            "addl $8, %%edx               \n\t"
            "paddb %%mm1, %%mm0           \n\t"

            "cmpl %%eax, %%edx            \n\t" // MMXLength
            "movq %%mm0, -8(%1," PDX ",)  \n\t" // write updated Raws to array
            "movq %%mm0, %%mm1            \n\t" // prep 1st add at top of loop
            "jb sub_3lp                   \n\t"

            : "=c" (dummy_value_c),   // 0      // output regs (dummy)
              "=D" (dummy_value_D),   // 1
              "=d" (dummy_value_d),   // 2
              "=a" (dummy_value_a)    // 3

            : "0" (bpp),              // ecx    // input regs
              "1" (row),              // edi
              "2" (diff),             // edx
              "3" (MMXLength)         // eax

#if defined(CLOBBER_MMX_REGS_SUPPORTED)
            : "%mm0", "%mm1", "%mm6", "%mm7"    // clobber list
#endif
         );
      }
      break;  // end 3 bpp

      case 4:   // formerly shared with 6 bpp case via _ShiftBpp and _ShiftRem,
      {         // but 64-bit PIC/.so problems (could still share, moving vars
                // into unused MMX regs via ecx/edx, but kludgy)
//       _ShiftBpp = bpp << 3;        // 32 (psllq)
//       _ShiftRem = 64 - _ShiftBpp;  // 32 (psrlq)

         __asm__ __volatile__ (
// preload  "mov  row, %1                 \n\t" // edi/rdi
// preload  "movl diff, %%edx             \n\t"
// notused  "mov  %1, " PSI "             \n\t" // lp = row
// preload  "movl bpp, %%ecx              \n\t"
            "add  " PCX ", %1             \n\t" // rp = row + bpp

            // prime the pump:  load the first Raw(x-bpp) data set
            "movq -8(%1," PDX ",), %%mm1  \n\t"

         "sub_4lp:                        \n\t" // shift data for adding first
            "psrlq $32, %%mm1             \n\t" //  bpp bytes (no need for mask;
                                                //  shift clears inactive bytes)
            "movq (%1," PDX ",), %%mm0    \n\t"
            "paddb %%mm1, %%mm0           \n\t"

            // add 2nd active group
            "movq %%mm0, %%mm1            \n\t" // mov updated Raws to mm1
            "psllq $32, %%mm1             \n\t" // shift data to pos. correctly
            "addl $8, %%edx               \n\t"
            "paddb %%mm1, %%mm0           \n\t"

            "cmpl %%eax, %%edx            \n\t" // MMXLength
            "movq %%mm0, -8(%1," PDX ",)  \n\t" // write updated Raws to array
            "movq %%mm0, %%mm1            \n\t" // prep 1st add at top of loop
            "jb sub_4lp                   \n\t"

            : "=c" (dummy_value_c),   // 0      // output regs (dummy)
              "=D" (dummy_value_D),   // 1
              "=d" (dummy_value_d),   // 2
              "=a" (dummy_value_a)    // 3

            : "0" (bpp),              // ecx    // input regs
              "1" (row),              // edi
              "2" (diff),             // edx
              "3" (MMXLength)         // eax

#if defined(CLOBBER_MMX_REGS_SUPPORTED)
            : "%mm0", "%mm1"                    // clobber list
#endif
         );
      }
      break;  // end 4 bpp

      case 1:
      {
         __asm__ __volatile__ (
// preload  "movl diff, %%edx              \n\t"
// preload  "mov  row, %1                  \n\t" // edi/rdi
// preload  "cmpl FullLength, %%edx        \n\t"
            "cmpl %%eax, %%edx             \n\t"
            "jnb sub_1end                  \n\t"
            "mov  %1, " PSI "              \n\t" // lp = row
// irrel.   "xorl %%ecx, %%ecx             \n\t" // (actually bug with preload)
// preload  "movl bpp, %%ecx               \n\t"
            "add  " PCX ", %1              \n\t" // rp = row + bpp

         "sub_1lp:                         \n\t"
            "movb (" PSI "," PDX ",), %%cl \n\t"
            "addb %%cl, (%1," PDX ",)      \n\t"
            "incl %%edx                    \n\t"
            "cmpl %%eax, %%edx             \n\t" // compare with FullLength
            "jb sub_1lp                    \n\t"

         "sub_1end:                        \n\t"

            : "=c" (dummy_value_c),   // 0      // output regs (dummy)
              "=D" (dummy_value_D),   // 1
              "=d" (dummy_value_d),   // 2
              "=a" (dummy_value_a)    // 3

            : "0" (bpp),              // ecx    // input regs
              "1" (row),              // edi
              "2" (diff),             // edx
              "3" (FullLength)        // eax

            : "%esi"                            // clobber list
         );
      }
      return;  // end 1 bpp (bypassing cleanup block!)

      case 2:
      {
//       _ShiftBpp = 16;       // == 2 * 8
//       _ShiftRem = 48;       // == 64 - 16

         __asm__ __volatile__ (
            LOAD_GOT_rbp
            // load (former) _ActiveMask for 2nd active byte group
            "movq " AMASK4_2_2 ", %%mm7   \n\t" // _amask4_2_2
            RESTORE_rbp
// preload  "movl diff, %%edx             \n\t"
            "movq %%mm7, %%mm6            \n\t"
// preload  "mov  row, %1                 \n\t" // edi/rdi
            "psllq $16, %%mm6             \n\t" // move mask in mm6 to cover
                                                //  3rd active byte group
// notused  "mov  %1, " PSI "             \n\t" // lp = row
            "movq %%mm6, %%mm5            \n\t"
// preload  "movl bpp, %%ecx              \n\t"
            "add  " PCX ", %1             \n\t" // rp = row + bpp
            "psllq $16, %%mm5             \n\t" // move mask in mm5 to cover
                                                //  4th active byte group
            // prime the pump:  load the first Raw(x-bpp) data set
            "movq -8(%1," PDX ",), %%mm1  \n\t"

         "sub_2lp:                        \n\t" // shift data for adding first
            "psrlq $48, %%mm1             \n\t" //  bpp bytes (no need for mask;
                                                //  shift clears inactive bytes)
            // add 1st active group
            "movq (%1," PDX ",), %%mm0    \n\t"
            "paddb %%mm1, %%mm0           \n\t"

            // add 2nd active group
            "movq %%mm0, %%mm1            \n\t" // mov updated Raws to mm1
            "psllq $16, %%mm1             \n\t" // shift data to pos. correctly
            "pand %%mm7, %%mm1            \n\t" // mask to use 2nd active group
            "paddb %%mm1, %%mm0           \n\t"

            // add 3rd active group
            "movq %%mm0, %%mm1            \n\t" // mov updated Raws to mm1
            "psllq $16, %%mm1             \n\t" // shift data to pos. correctly
            "pand %%mm6, %%mm1            \n\t" // mask to use 3rd active group
            "paddb %%mm1, %%mm0           \n\t"

            // add 4th active group
            "movq %%mm0, %%mm1            \n\t" // mov updated Raws to mm1
            "psllq $16, %%mm1             \n\t" // shift data to pos. correctly
            "pand %%mm5, %%mm1            \n\t" // mask to use 4th active group
            "addl $8, %%edx               \n\t"
            "paddb %%mm1, %%mm0           \n\t"
            "cmpl %%eax, %%edx            \n\t" // MMXLength
            "movq %%mm0, -8(%1," PDX ",)  \n\t" // write updated Raws to array
            "movq %%mm0, %%mm1            \n\t" // prep 1st add at top of loop
            "jb sub_2lp                   \n\t"

            : "=c" (dummy_value_c),   // 0      // output regs (dummy)
              "=D" (dummy_value_D),   // 1
              "=d" (dummy_value_d),   // 2
              "=a" (dummy_value_a)    // 3

            : "0" (bpp),              // ecx    // input regs
              "1" (row),              // edi
              "2" (diff),             // edx
              "3" (MMXLength)         // eax

#if defined(CLOBBER_MMX_REGS_SUPPORTED)
            : "%mm0", "%mm1", "%mm5", "%mm6"    // clobber list
            , "%mm7"
#endif
         );
      }
      break;  // end 2 bpp

      case 6:   // formerly shared with 4 bpp case (see comments there)
      {
//       _ShiftBpp = bpp << 3;        // 48 (psllq)
//       _ShiftRem = 64 - _ShiftBpp;  // 16 (psrlq)

         __asm__ __volatile__ (
// preload  "mov  row, %1                 \n\t" // edi/rdi
// preload  "movl diff, %%edx             \n\t"
// notused  "mov  %1, " PSI "             \n\t" // lp = row
// preload  "movl bpp, %%ecx              \n\t"
            "add  " PCX ", %1             \n\t" // rp = row + bpp

            // prime the pump:  load the first Raw(x-bpp) data set
            "movq -8(%1," PDX ",), %%mm1  \n\t"

         "sub_6lp:                        \n\t" // shift data for adding first
            "psrlq $16, %%mm1             \n\t" //  bpp bytes (no need for mask;
                                                //  shift clears inactive bytes)
            "movq (%1," PDX ",), %%mm0    \n\t"
            "paddb %%mm1, %%mm0           \n\t"

            // add 2nd active group
            "movq %%mm0, %%mm1            \n\t" // mov updated Raws to mm1
            "psllq $48, %%mm1             \n\t" // shift data to pos. correctly
            "addl $8, %%edx               \n\t"
            "paddb %%mm1, %%mm0           \n\t"

            "cmpl %%eax, %%edx            \n\t" // MMXLength
            "movq %%mm0, -8(%1," PDX ",)  \n\t" // write updated Raws to array
            "movq %%mm0, %%mm1            \n\t" // prep 1st add at top of loop
            "jb sub_6lp                   \n\t"

            : "=c" (dummy_value_c),   // 0      // output regs (dummy)
              "=D" (dummy_value_D),   // 1
              "=d" (dummy_value_d),   // 2
              "=a" (dummy_value_a)    // 3

            : "0" (bpp),              // ecx    // input regs
              "1" (row),              // edi
              "2" (diff),             // edx
              "3" (MMXLength)         // eax

#if defined(CLOBBER_MMX_REGS_SUPPORTED)
            : "%mm0", "%mm1"                    // clobber list
#endif
         );
      }
      break;  // end 6 bpp

      case 8:
      {
         __asm__ __volatile__ (
// preload  "mov  row, %1                 \n\t" // edi/rdi
// preload  "movl diff, %%edx             \n\t"
// notused  "mov  %1, " PSI "             \n\t" // lp = row
// preload  "movl bpp, %%ecx              \n\t"
            "add  " PCX ", %1             \n\t" // rp = row + bpp
// preload  "movl MMXLength, %%eax        \n\t"

            // prime the pump:  load the first Raw(x-bpp) data set
            "movq -8(%1," PDX ",), %%mm7  \n\t"
            "movl %%eax, %%esi            \n\t" // copy of MMXLength -> esi
            "andl $0x0000003f, %%esi      \n\t" // calc bytes over mult of 64

         "sub_8lp:                        \n\t"
            "movq (%1," PDX ",), %%mm0    \n\t" // load Sub(x) for 1st 8 bytes
            "paddb %%mm7, %%mm0           \n\t"
            "movq 8(%1," PDX ",), %%mm1   \n\t" // load Sub(x) for 2nd 8 bytes
            "movq %%mm0, (%1," PDX ",)    \n\t" // write Raw(x) for 1st 8 bytes

            // Now mm0 will be used as Raw(x-bpp) for the 2nd group of 8 bytes.
            // This will be repeated for each group of 8 bytes with the 8th
            // group being used as the Raw(x-bpp) for the 1st group of the
            // next loop.

            "paddb %%mm0, %%mm1           \n\t"
            "movq 16(%1," PDX ",), %%mm2  \n\t" // load Sub(x) for 3rd 8 bytes
            "movq %%mm1, 8(%1," PDX ",)   \n\t" // write Raw(x) for 2nd 8 bytes
            "paddb %%mm1, %%mm2           \n\t"
            "movq 24(%1," PDX ",), %%mm3  \n\t" // load Sub(x) for 4th 8 bytes
            "movq %%mm2, 16(%1," PDX ",)  \n\t" // write Raw(x) for 3rd 8 bytes
            "paddb %%mm2, %%mm3           \n\t"
            "movq 32(%1," PDX ",), %%mm4  \n\t" // load Sub(x) for 5th 8 bytes
            "movq %%mm3, 24(%1," PDX ",)  \n\t" // write Raw(x) for 4th 8 bytes
            "paddb %%mm3, %%mm4           \n\t"
            "movq 40(%1," PDX ",), %%mm5  \n\t" // load Sub(x) for 6th 8 bytes
            "movq %%mm4, 32(%1," PDX ",)  \n\t" // write Raw(x) for 5th 8 bytes
            "paddb %%mm4, %%mm5           \n\t"
            "movq 48(%1," PDX ",), %%mm6  \n\t" // load Sub(x) for 7th 8 bytes
            "movq %%mm5, 40(%1," PDX ",)  \n\t" // write Raw(x) for 6th 8 bytes
            "paddb %%mm5, %%mm6           \n\t"
            "movq 56(%1," PDX ",), %%mm7  \n\t" // load Sub(x) for 8th 8 bytes
            "movq %%mm6, 48(%1," PDX ",)  \n\t" // write Raw(x) for 7th 8 bytes
            "addl $64, %%edx              \n\t"
            "paddb %%mm6, %%mm7           \n\t"
            "cmpl %%esi, %%edx            \n\t" // cmp to bytes over mult of 64
            "movq %%mm7, -8(%1," PDX ",)  \n\t" // write Raw(x) for 8th 8 bytes
            "jb sub_8lp                   \n\t"

            "cmpl %%eax, %%edx            \n\t" // compare to MMXLength
            "jnb sub_8lt8                 \n\t"

         "sub_8lpA:                       \n\t"
            "movq (%1," PDX ",), %%mm0    \n\t"
            "addl $8, %%edx               \n\t"
            "paddb %%mm7, %%mm0           \n\t"
            "cmpl %%eax, %%edx            \n\t" // compare to MMXLength
            "movq %%mm0, -8(%1," PDX ",)  \n\t" // -8 to offset early addl edx
            "movq %%mm0, %%mm7            \n\t" // move calculated Raw(x) data
            "jb sub_8lpA                  \n\t" //  to mm7 to be new Raw(x-bpp)
                                                //  for next loop
         "sub_8lt8:                       \n\t"

            : "=c" (dummy_value_c),   // 0      // output regs (dummy)
              "=D" (dummy_value_D),   // 1
              "=d" (dummy_value_d),   // 2
              "=a" (dummy_value_a)    // 3

            : "0" (bpp),              // ecx    // input regs
              "1" (row),              // edi
              "2" (diff),             // edx
              "3" (MMXLength)         // eax

            : "%esi"                            // clobber list
#if defined(CLOBBER_MMX_REGS_SUPPORTED)
            , "%mm0", "%mm1", "%mm2", "%mm3"
            , "%mm4", "%mm5", "%mm6", "%mm7"
#endif
         );
      }
      break;  // end 8 bpp

      default:                // bpp != 1,2,3,4,6,8:  doesn't exist
      {
         // ERROR:  SHOULD NEVER BE REACHED
#if defined(PNG_DEBUG)
         png_debug(1, "Internal libpng logic error (GCC "
           "png_read_filter_row_mmx_sub())\n");
#endif
      }
      break;

   } // end switch (bpp)

   __asm__ __volatile__ (
//pre "movl MMXLength, %%eax         \n\t"
//pre "mov  row, %1                  \n\t" // edi/rdi
//pre "cmpl FullLength, %%eax        \n\t"
      "cmpl %%edx, %%eax             \n\t"
      "jnb sub_end                   \n\t"

      "mov  %1, " PSI "              \n\t" // lp = row
//pre "movl bpp, %%ecx               \n\t"
      "add  " PCX ", %1              \n\t" // rp = row + bpp
      "xorl %%ecx, %%ecx             \n\t"

   "sub_lp2:                         \n\t"
      "movb (" PSI "," PAX ",), %%cl \n\t"
      "addb %%cl, (%1," PAX ",)      \n\t"
      "incl %%eax                    \n\t"
      "cmpl %%edx, %%eax             \n\t" // FullLength
      "jb sub_lp2                    \n\t"

   "sub_end:                         \n\t"
      "EMMS                          \n\t" // end MMX instructions

      : "=c" (dummy_value_c),   // 0      // output regs (dummy)
        "=D" (dummy_value_D),   // 1
        "=a" (dummy_value_a),   // 2
        "=d" (dummy_value_d)    // 3

      : "0" (bpp),              // ecx    // input regs
        "1" (row),              // edi
        "2" (MMXLength),        // eax
        "3" (FullLength)        // edx

      : "%esi"                            // clobber list
   );

} // end of png_read_filter_row_mmx_sub()

#endif /* PNG_MMX_READ_FILTER_SUB_SUPPORTED */




#if defined(PNG_MMX_READ_FILTER_UP_SUPPORTED)

//===========================================================================//
//                                                                           //
//            P N G _ R E A D _ F I L T E R _ R O W _ M M X _ U P            //
//                                                                           //
//===========================================================================//

// Optimized code for PNG Up filter decoder

static void /* PRIVATE */
png_read_filter_row_mmx_up(png_row_infop row_info, png_bytep row,
                           png_bytep prev_row)
{
   unsigned len;        // png_uint_32 is actually 64-bit on x86-64
   int dummy_value_d;   // fix 'forbidden register 3 (dx) was spilled' error
   png_bytep dummy_value_S;
   png_bytep dummy_value_D;

   len = row_info->rowbytes;              // number of bytes to filter

   __asm__ __volatile__ (
      SAVE_GOT_ebx
//pre "mov  prev_row, %1           \n\t" // esi/rsi
//pre "movl row, %2                \n\t" // edi/rdi

      "xorl %%ebx, %%ebx           \n\t"
      "xorl %%eax, %%eax           \n\t"

      // get # of bytes to alignment (note:  computing _delta_ of two pointers,
      // so hereafter %%ecx is sufficient even on 64-bit)
      "mov  %2, " PCX "            \n\t" // take start of row
      "add  $0x7, " PCX "          \n\t" // add 7 to incr past alignment bdry
//    "andl $0xfffffff8, %%ecx     \n\t" // mask to alignment boundary (32-bit!)
      CLEAR_BOTTOM_3_BITS  PCX    "\n\t" // mask to alignment boundary
      "sub  %2, " PCX "            \n\t" // subtract row ptr again => ebp =
      "jz up_go                    \n\t" //  target value of ecx at alignment

   "up_lp1:                        \n\t" // fix alignment
      "movb (%2," PBX ",), %%al    \n\t"
      "addb (%1," PBX ",), %%al    \n\t"
      "incl %%ebx                  \n\t"
      "cmpl %%ecx, %%ebx           \n\t"
      "movb %%al, -1(%2," PBX ",)  \n\t" // mov does not affect flags; -1 to
      "jb up_lp1                   \n\t" //  offset incl ebx

   "up_go:                         \n\t"
//pre "movl len, %%edx             \n\t"
      "movl %%edx, %%ecx           \n\t"
      "subl %%ebx, %%edx           \n\t" // subtract alignment fix
      "andl $0x0000003f, %%edx     \n\t" // calc bytes over mult of 64
      "subl %%edx, %%ecx           \n\t" // sub over-bytes from original length

      // unrolled loop - use all MMX registers and interleave to reduce
      // number of branch instructions (loops) and reduce partial stalls
   "up_loop:                       \n\t"
      "movq (%1," PBX ",), %%mm1   \n\t"
      "movq (%2," PBX ",), %%mm0   \n\t"
      "movq 8(%1," PBX ",), %%mm3  \n\t"
      "paddb %%mm1, %%mm0          \n\t"
      "movq 8(%2," PBX ",), %%mm2  \n\t"
      "movq %%mm0, (%2," PBX ",)   \n\t"
      "paddb %%mm3, %%mm2          \n\t"
      "movq 16(%1," PBX ",), %%mm5 \n\t"
      "movq %%mm2, 8(%2," PBX ",)  \n\t"
      "movq 16(%2," PBX ",), %%mm4 \n\t"
      "movq 24(%1," PBX ",), %%mm7 \n\t"
      "paddb %%mm5, %%mm4          \n\t"
      "movq 24(%2," PBX ",), %%mm6 \n\t"
      "movq %%mm4, 16(%2," PBX ",) \n\t"
      "paddb %%mm7, %%mm6          \n\t"
      "movq 32(%1," PBX ",), %%mm1 \n\t"
      "movq %%mm6, 24(%2," PBX ",) \n\t"
      "movq 32(%2," PBX ",), %%mm0 \n\t"
      "movq 40(%1," PBX ",), %%mm3 \n\t"
      "paddb %%mm1, %%mm0          \n\t"
      "movq 40(%2," PBX ",), %%mm2 \n\t"
      "movq %%mm0, 32(%2," PBX ",) \n\t"
      "paddb %%mm3, %%mm2          \n\t"
      "movq 48(%1," PBX ",), %%mm5 \n\t"
      "movq %%mm2, 40(%2," PBX ",) \n\t"
      "movq 48(%2," PBX ",), %%mm4 \n\t"
      "movq 56(%1," PBX ",), %%mm7 \n\t"
      "paddb %%mm5, %%mm4          \n\t"
      "movq 56(%2," PBX ",), %%mm6 \n\t"
      "movq %%mm4, 48(%2," PBX ",) \n\t"
      "addl $64, %%ebx             \n\t"
      "paddb %%mm7, %%mm6          \n\t"
      "cmpl %%ecx, %%ebx           \n\t"
      "movq %%mm6, -8(%2," PBX ",) \n\t" // (+56)movq does not affect flags;
      "jb up_loop                  \n\t" //  -8 to offset addl ebx

      "cmpl $0, %%edx              \n\t" // test for bytes over mult of 64
      "jz up_end                   \n\t"

      "cmpl $8, %%edx              \n\t" // test for less than 8 bytes
      "jb up_lt8                   \n\t" //  [added by lcreeve at netins.net]

      "addl %%edx, %%ecx           \n\t"
      "andl $0x00000007, %%edx     \n\t" // calc bytes over mult of 8
      "subl %%edx, %%ecx           \n\t" // drop over-bytes from length
      "jz up_lt8                   \n\t"

   "up_lpA:                        \n\t" // use MMX regs to update 8 bytes sim.
      "movq (%1," PBX ",), %%mm1   \n\t"
      "movq (%2," PBX ",), %%mm0   \n\t"
      "addl $8, %%ebx              \n\t"
      "paddb %%mm1, %%mm0          \n\t"
      "cmpl %%ecx, %%ebx           \n\t"
      "movq %%mm0, -8(%2," PBX ",) \n\t" // movq does not affect flags; -8 to
      "jb up_lpA                   \n\t" //  offset add ebx
      "cmpl $0, %%edx              \n\t" // test for bytes over mult of 8
      "jz up_end                   \n\t"

   "up_lt8:                        \n\t"
      "xorl %%eax, %%eax           \n\t"
      "addl %%edx, %%ecx           \n\t" // move over byte count into counter

   "up_lp2:                        \n\t" // use x86 regs for remaining bytes
      "movb (%2," PBX ",), %%al    \n\t"
      "addb (%1," PBX ",), %%al    \n\t"
      "incl %%ebx                  \n\t"
      "cmpl %%ecx, %%ebx           \n\t"
      "movb %%al, -1(%2," PBX ",)  \n\t" // mov does not affect flags; -1 to
      "jb up_lp2                   \n\t" //  offset inc ebx

   "up_end:                        \n\t"
      "EMMS                        \n\t" // conversion of filtered row complete
      RESTORE_GOT_ebx

      : "=d" (dummy_value_d),   // 0     // output regs (dummy)
        "=S" (dummy_value_S),   // 1
        "=D" (dummy_value_D)    // 2

      : "0" (len),              // edx   // input regs
        "1" (prev_row),         // esi
        "2" (row)               // edi

      : "%eax", "%ecx"                   // clobber list (no input regs!)
        _CLOBBER_GOT_ebx
#if defined(PNG_CLOBBER_MMX_REGS_SUPPORTED)
      , "%mm0", "%mm1", "%mm2", "%mm3"
      , "%mm4", "%mm5", "%mm6", "%mm7"
#endif
   );

} // end of png_read_filter_row_mmx_up()

#endif /* PNG_MMX_READ_FILTER_UP_SUPPORTED */




/*===========================================================================*/
/*                                                                           */
/*                   P N G _ R E A D _ F I L T E R _ R O W                   */
/*                                                                           */
/*===========================================================================*/

/* Optimized png_read_filter_row routines */

void /* PRIVATE */
png_read_filter_row(png_structp png_ptr, png_row_infop row_info, png_bytep
   row, png_bytep prev_row, int filter)
{
#if defined(PNG_DEBUG)
   char filtname[10];
#endif

   if (_mmx_supported == 2) {
#if !defined(PNG_1_0_X)
       /* this should have happened in png_init_mmx_flags() already */
       png_warning(png_ptr, "asm_flags may not have been initialized");
#endif
       png_mmx_support();
   }

#if defined(PNG_DEBUG)
   png_debug(1, "in png_read_filter_row (pnggccrd.c)\n");
   switch (filter)
   {
      case 0:
         png_snprintf(filtname, 10, "none");
         break;

      case 1:
         png_snprintf(filtname, 10, "sub-%s",
#ifdef PNG_MMX_READ_FILTER_SUB_SUPPORTED
#if !defined(PNG_1_0_X)
           ((png_ptr->asm_flags & PNG_ASM_FLAG_MMX_READ_FILTER_SUB) &&
            (row_info->pixel_depth >= png_ptr->mmx_bitdepth_threshold) &&
            (row_info->rowbytes >= png_ptr->mmx_rowbytes_threshold))
#else
           _mmx_supported
#endif
           ? "MMX" :
#endif
           "C");
         break;

      case 2:
         png_snprintf(filtname, 10, "up-%s",
#ifdef PNG_MMX_READ_FILTER_UP_SUPPORTED
#if !defined(PNG_1_0_X)
           ((png_ptr->asm_flags & PNG_ASM_FLAG_MMX_READ_FILTER_UP) &&
            (row_info->pixel_depth >= png_ptr->mmx_bitdepth_threshold) &&
            (row_info->rowbytes >= png_ptr->mmx_rowbytes_threshold))
#else
           _mmx_supported
#endif
           ? "MMX" :
#endif
           "C");
         break;

      case 3:
         png_snprintf(filtname, 10, "avg-%s",
#ifdef PNG_MMX_READ_FILTER_AVG_SUPPORTED
#if !defined(PNG_1_0_X)
           ((png_ptr->asm_flags & PNG_ASM_FLAG_MMX_READ_FILTER_AVG) &&
            (row_info->pixel_depth >= png_ptr->mmx_bitdepth_threshold) &&
            (row_info->rowbytes >= png_ptr->mmx_rowbytes_threshold))
#else
           _mmx_supported
#endif
           ? "MMX" :
#endif
           "C");
         break;

      case 4:
         png_snprintf(filtname, 10, "paeth-%s",
#ifdef PNG_MMX_READ_FILTER_PAETH_SUPPORTED
#if defined(PNG_x86_64_USE_GOTPCREL) || defined(PNG_THREAD_UNSAFE_OK)
#if !defined(PNG_1_0_X)
           ((png_ptr->asm_flags & PNG_ASM_FLAG_MMX_READ_FILTER_PAETH) &&
            (row_info->pixel_depth >= png_ptr->mmx_bitdepth_threshold) &&
            (row_info->rowbytes >= png_ptr->mmx_rowbytes_threshold))
#else
           _mmx_supported
#endif
           ? "MMX" :
#endif /* PNG_x86_64_USE_GOTPCREL || PNG_THREAD_UNSAFE_OK */
#endif
           "C");
         break;

      default:
         png_snprintf(filtname, 10, "unknown");
         break;
   }
   png_debug2(2, "row_number=%ld, %s, ", png_ptr->row_number, filtname);
   //png_debug1(0, "png_ptr=%10p, ", png_ptr);
   //png_debug1(0, "asm_flags=0x%08lx, ", png_ptr->asm_flags);
   png_debug1(0, "row=%10p, ", row);
   png_debug2(0, "pixdepth=%d, bytes=%d, ", (int)row_info->pixel_depth,
      (int)((row_info->pixel_depth + 7) >> 3));
   png_debug1(0, "rowbytes=%ld\n", row_info->rowbytes);
#endif /* PNG_DEBUG */

   switch (filter)
   {
      case PNG_FILTER_VALUE_NONE:
         break;

      case PNG_FILTER_VALUE_SUB:
#ifdef PNG_MMX_READ_FILTER_SUB_SUPPORTED
#if !defined(PNG_1_0_X)
         if ((png_ptr->asm_flags & PNG_ASM_FLAG_MMX_READ_FILTER_SUB) &&
             (row_info->pixel_depth >= png_ptr->mmx_bitdepth_threshold) &&
             (row_info->rowbytes >= png_ptr->mmx_rowbytes_threshold))
#else
         if (_mmx_supported)
#endif
         {
            png_read_filter_row_mmx_sub(row_info, row);
         }
         else
#endif
         {
            png_uint_32 i;
            png_uint_32 istop = row_info->rowbytes;
            png_uint_32 bpp = (row_info->pixel_depth + 7) >> 3;
            png_bytep rp = row + bpp;
            png_bytep lp = row;

            for (i = bpp; i < istop; i++)
            {
               *rp = (png_byte)(((int)(*rp) + (int)(*lp++)) & 0xff);
               rp++;
            }
         }  /* end !UseMMX_sub */
         break;

      case PNG_FILTER_VALUE_UP:
#ifdef PNG_MMX_READ_FILTER_UP_SUPPORTED
#if !defined(PNG_1_0_X)
         if ((png_ptr->asm_flags & PNG_ASM_FLAG_MMX_READ_FILTER_UP) &&
             (row_info->pixel_depth >= png_ptr->mmx_bitdepth_threshold) &&
             (row_info->rowbytes >= png_ptr->mmx_rowbytes_threshold))
#else
         if (_mmx_supported)
#endif
         {
            png_read_filter_row_mmx_up(row_info, row, prev_row);
         }
          else
#endif
         {
            png_uint_32 i;
            png_uint_32 istop = row_info->rowbytes;
            png_bytep rp = row;
            png_bytep pp = prev_row;

            for (i = 0; i < istop; ++i)
            {
               *rp = (png_byte)(((int)(*rp) + (int)(*pp++)) & 0xff);
               rp++;
            }
         }  /* end !UseMMX_up */
         break;

      case PNG_FILTER_VALUE_AVG:
#ifdef PNG_MMX_READ_FILTER_AVG_SUPPORTED
#if !defined(PNG_1_0_X)
         if ((png_ptr->asm_flags & PNG_ASM_FLAG_MMX_READ_FILTER_AVG) &&
             (row_info->pixel_depth >= png_ptr->mmx_bitdepth_threshold) &&
             (row_info->rowbytes >= png_ptr->mmx_rowbytes_threshold))
#else
         if (_mmx_supported)
#endif
         {
            png_read_filter_row_mmx_avg(row_info, row, prev_row);
         }
         else
#endif
         {
            png_uint_32 i;
            png_bytep rp = row;
            png_bytep pp = prev_row;
            png_bytep lp = row;
            png_uint_32 bpp = (row_info->pixel_depth + 7) >> 3;
            png_uint_32 istop = row_info->rowbytes - bpp;

            for (i = 0; i < bpp; i++)
            {
               *rp = (png_byte)(((int)(*rp) +
                  ((int)(*pp++) >> 1)) & 0xff);
               rp++;
            }

            for (i = 0; i < istop; i++)
            {
               *rp = (png_byte)(((int)(*rp) +
                  ((int)(*pp++ + *lp++) >> 1)) & 0xff);
               rp++;
            }
         }  /* end !UseMMX_avg */
         break;

      case PNG_FILTER_VALUE_PAETH:
#ifdef PNG_MMX_READ_FILTER_PAETH_SUPPORTED
#if defined(PNG_x86_64_USE_GOTPCREL) || defined(PNG_THREAD_UNSAFE_OK)
#if !defined(PNG_1_0_X)
         if ((png_ptr->asm_flags & PNG_ASM_FLAG_MMX_READ_FILTER_PAETH) &&
             (row_info->pixel_depth >= png_ptr->mmx_bitdepth_threshold) &&
             (row_info->rowbytes >= png_ptr->mmx_rowbytes_threshold))
#else
         if (_mmx_supported)
#endif
         {
            png_read_filter_row_mmx_paeth(row_info, row, prev_row);
         }
         else
#endif /* PNG_x86_64_USE_GOTPCREL || PNG_THREAD_UNSAFE_OK */
#endif
         {
            png_uint_32 i;
            png_bytep rp = row;
            png_bytep pp = prev_row;
            png_bytep lp = row;
            png_bytep cp = prev_row;
            png_uint_32 bpp = (row_info->pixel_depth + 7) >> 3;
            png_uint_32 istop = row_info->rowbytes - bpp;

            for (i = 0; i < bpp; i++)
            {
               *rp = (png_byte)(((int)(*rp) + (int)(*pp++)) & 0xff);
               rp++;
            }

            for (i = 0; i < istop; i++)   /* use leftover rp,pp */
            {
               int a, b, c, pa, pb, pc, p;

               a = *lp++;
               b = *pp++;
               c = *cp++;

               p = b - c;
               pc = a - c;

#if defined(PNG_USE_ABS)
               pa = abs(p);
               pb = abs(pc);
               pc = abs(p + pc);
#else
               pa = p < 0 ? -p : p;
               pb = pc < 0 ? -pc : pc;
               pc = (p + pc) < 0 ? -(p + pc) : p + pc;
#endif

               /*
                  if (pa <= pb && pa <= pc)
                     p = a;
                  else if (pb <= pc)
                     p = b;
                  else
                     p = c;
                */

               p = (pa <= pb && pa <= pc) ? a : (pb <= pc) ? b : c;

               *rp = (png_byte)(((int)(*rp) + p) & 0xff);
               rp++;
            }
         }  /* end !UseMMX_paeth */
         break;

      default:
         png_warning(png_ptr, "Ignoring bad row-filter type");
         *row=0;
         break;
   }
}

#endif /* PNG_HAVE_MMX_READ_FILTER_ROW */


#endif /* PNG_ASSEMBLER_CODE_SUPPORTED && PNG_USE_PNGGCCRD */
#endif /* __GNUC__ */
