;/********************************************************
; * Some code. Copyright (C) 2003 by Pascal Massimino.   *
; * All Rights Reserved.      (http://skal.planet-d.net) *
; * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
; ********************************************************/
;//////////////////////////////////////////////////////////
;// NASM macros
;//////////////////////////////////////////////////////////

%ifdef LINUX

;//////////////////////////////////////////////////////////
;  LINUX / egcs / macros
;//////////////////////////////////////////////////////////

%macro extrn 1
  extern %1
  %define %1 %1
%endmacro
%macro globl 1
  global %1
  %define %1 %1
%endmacro

%macro DATA 0
[section data align=16 write alloc USE32]
%endmacro
%macro TEXT 0
[section text align=16 nowrite alloc exec USE32]
%endmacro

%endif    ; LINUX

;//////////////////////////////////////////////////////////

%ifdef WIN32

%macro extrn 1
  extern _%1
  %define %1 _%1
%endmacro

%macro globl 1
  global _%1
  %define %1 _%1
%endmacro

%macro DATA 0
[section .data align=16 write alloc USE32]
%endmacro
%macro TEXT 0
[section .text align=16 nowrite alloc exec USE32]
%endmacro

%endif    ; WIN32

;//////////////////////////////////////////////////////////
;
; MACRO for timing. NASM.
;     Total additional code size is 0xb0.
;     this keep code alignment right.

;extrn Skl_Cur_Count_
;extrn Skl_Print_Tics

%macro SKL_USE_RDSTC 0
extrn SKL_RDTSC_0_ASM
extrn SKL_RDTSC_1_ASM
extrn SKL_RDTSC_2_ASM
%endmacro
%define SKL_RDTSC_OFFSET 15 ; check value with skl_rdtsc.h...

%macro SKL_RDTSC_IN   0
   SKL_USE_RDSTC
   call SKL_RDTSC_0_ASM
.Skl_RDTSC_Loop_:
   call SKL_RDTSC_1_ASM
%endmacro

;%macro SKL_RDTSC_OUT   0
;   call SKL_RDTSC_2_ASM
;   dec dword [Skl_Cur_Count_]
;   jge near .Skl_RDTSC_Loop_
;   push dword 53
;   call Skl_Print_Tics
;%endmacro

;//////////////////////////////////////////////////////////
