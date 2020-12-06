;*
;* tmap2.nas
;* The tilted plane inner loop.
;*
;*---------------------------------------------------------------------------
;* Copyright 1998-2001 Randy Heit
;* All rights reserved.
;*
;* Redistribution and use in source and binary forms, with or without
;* modification, are permitted provided that the following conditions
;* are met:
;*
;* 1. Redistributions of source code must retain the above copyright
;*    notice, this list of conditions and the following disclaimer.
;* 2. Redistributions in binary form must reproduce the above copyright
;*    notice, this list of conditions and the following disclaimer in the
;*    documentation and/or other materials provided with the distribution.
;* 3. The name of the author may not be used to endorse or promote products
;*    derived from this software without specific prior written permission.
;*
;* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
;* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
;* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
;* IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
;* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
;* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
;* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
;* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
;* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
;* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;*---------------------------------------------------------------------------
;*
;* I tried doing the ROL trick that R_DrawSpanP_ASM uses, and it was
;* actually slightly slower than the more straight-forward approach
;* used here, probably because the trick requires too much setup time.
;*

BITS 32

%define SPACEFILLER4	(0x44444444)

%ifndef M_TARGET_LINUX

%define plane_sz			_plane_sz
%define plane_su			_plane_su
%define plane_sv			_plane_sv
%define plane_shade			_plane_shade
%define planelightfloat			_planelightfloat
%define spanend				_spanend
%define ylookup				_ylookup
%define ds_colormap			_ds_colormap
%define ds_source			_ds_source
%define centery				_centery
%define centerx				_centerx
%define ds_curtiltedsource		_ds_curtiltedsource
%define pviewx				_pviewx
%define pviewy				_pviewy
%define tiltlighting			_tiltlighting

%define R_DrawTiltedPlane_ASM		_R_DrawTiltedPlane_ASM
%define R_SetTiltedSpanSource_ASM	_R_SetTiltedSpanSource_ASM
%define R_CalcTiltedLighting		_R_CalcTiltedLighting

%endif

EXTERN plane_sz
EXTERN plane_su
EXTERN plane_sv
EXTERN planelightfloat
EXTERN spanend
EXTERN ylookup
EXTERN ds_colormap
EXTERN centery
EXTERN centerx
EXTERN ds_source
EXTERN plane_shade
EXTERN pviewx
EXTERN pviewy
EXTERN tiltlighting
EXTERN R_CalcTiltedLighting

GLOBAL ds_curtiltedsource

%define sv_i		plane_sv
%define sv_j		plane_sv+4
%define sv_k		plane_sv+8

%define su_i		plane_su
%define su_j		plane_su+4
%define su_k		plane_su+8

%define sz_i		plane_sz
%define sz_j		plane_sz+4
%define sz_k		plane_sz+8

%define SPANBITS	3

	section .bss

start_u:		resq	1
start_v:		resq	1
step_u:			resq	1
step_v:			resq	1

step_iz:		resq	1
step_uz:		resq	1
step_vz:		resq	1

end_z:			resd	1

	section .data

ds_curtiltedsource:	dd	SPACEFILLER4

fp_1:
spanrecips:		dd	0x3f800000	; 1/1
			dd	0x3f000000	; 1/2
			dd	0x3eaaaaab	; 1/3
			dd	0x3e800000	; 1/4
			dd	0x3e4ccccd	; 1/5
			dd	0x3e2aaaab	; 1/6
			dd	0x3e124925	; 1/7
fp_8recip:		dd	0x3e000000	; 1/8
			dd	0x3de38e39	; 1/9
			dd	0x3dcccccd	; 1/10
			dd	0x3dba2e8c	; 1/11
			dd	0x3daaaaab	; 1/12
			dd	0x3d9d89d9	; 1/13
			dd	0x3d924925	; 1/14
			dd	0x3d888889	; 1/15

fp_quickint:		dd	0x3f800000	; 1
			dd	0x40000000	; 2
			dd	0x40400000	; 3
			dd	0x40800000	; 4
			dd	0x40a00000	; 5
			dd	0x40c00000	; 6
			dd	0x40e00000	; 7
fp_8:			dd	0x41000000	; 8

	section .text

GLOBAL	R_SetTiltedSpanSource_ASM
GLOBAL	@R_SetTiltedSpanSource_ASM@4

R_SetTiltedSpanSource_ASM:
	mov	ecx,[esp+4]

@R_SetTiltedSpanSource_ASM@4:
	mov	[fetch1+3],ecx
	mov	[fetch2+3],ecx
	mov	[fetch3+3],ecx
	mov	[fetch4+3],ecx
	mov	[fetch5+3],ecx
	mov	[fetch6+3],ecx
	mov	[fetch7+3],ecx
	mov	[fetch8+3],ecx
	mov	[fetch9+3],ecx
	mov	[fetch10+3],ecx
	mov	[ds_curtiltedsource],ecx
	ret

	section .data

GLOBAL	R_DrawTiltedPlane_ASM
GLOBAL	@R_DrawTiltedPlane_ASM@8

R_DrawTiltedPlane_ASM:
	mov	ecx,[esp+4]
	mov	edx,[esp+8]

	; ecx = y
	; edx = x

@R_DrawTiltedPlane_ASM@8:
	push	ebx
	push	esi
	push	edi
	push	ebp

	mov	eax,[centery]
	movzx	ebx,word [spanend+ecx*2]
	sub	eax,ecx			; eax = centery-y
	sub	ebx,edx			; ebx = span length - 1
	mov	edi,[ylookup+ecx*4]
	push	eax
	add	edi,edx			; edi = frame buffer pointer
	sub	edx,[centerx]		; edx = x-centerx
	push	edx
	xor	eax,eax

	fild	dword [esp+4]		; ymul
	fild	dword [esp]		; xmul | ymul
	fld	dword [sv_j]		; sv.j | xmul | ymul
	fmul	st0,st2			; sv.j*ymul | xmul | ymul
	fld	dword [su_j]		; su.j | sv.j*ymul | xmul | ymul
	fmul	st0,st3			; su.j*ymul | sv.j*ymul | xmul | ymul
	fld	dword [sz_j]		; sz.j | su.j*ymul | sv.j*ymul | xmul | ymul
	fmulp	st4,st0			; su.j*ymul | sv.j*ymul | xmul | sz.j*ymul
	fld	dword [sv_i]		; sv.i | su.j*ymul | sv.j*ymul | xmul | sz.j*ymul
	fmul	st0,st3			; sv.i*xmul | su.j*ymul | sv.j*ymul | xmul | sz.j*ymul
	fld	dword [su_i]		; su.i | sv.i*xmul | su.j*ymul | sv.j*ymul | xmul | sz.j*ymul
	fmul	st0,st4			; su.i*xmul | sv.i*xmul | su.j*ymul | sv.j*ymul | xmul | sz.j*ymul
	fld	dword [sz_i]		; sz.i | su.i*xmul | sv.i*xmul | su.j*ymul | sv.j*ymul | xmul | sz.j*ymul
	fmulp	st5,st0			; su.i*xmul | sv.i*xmul | su.j*ymul | sv.j*ymul | sz.i*xmul | sz.j*ymul
	fxch	st1			; sv.i*xmul | su.i*xmul | su.j*ymul | sv.j*ymul | sz.i*xmul | sz.j*ymul
	faddp	st3,st0			; su.i*xmul | su.j*ymul | sv.i*xmul+sv.j*ymul | sz.i*xmul | sz.j*ymul
	faddp	st1,st0			; su.i*xmul+su.j*ymul | sv.i*xmul+sv.j*ymul | sz.i*xmul | sz.j*ymul
	fxch	st3			; sz.j*ymul | sv.i*xmul+sv.j*ymul | sz.i*xmul | su.i*xmul+su.j*ymul
	faddp	st2,st0			; sv.i*xmul+sv.j*ymul | sz.i*xmul+sz.j*ymul | su.i*xmul+su.j*ymul
	fadd	dword [sv_k]		; v/z | sz.i*xmul+sz.j*ymul | su.i*xmul+su.j*ymul
	fxch	st1			; sz.i*xmul+sz.j*ymul | v/z | su.i*xmul+su.j*ymul
	fadd	dword [sz_k]		; 1/z | v/z | su.i*xmul+su.j*ymul
	fxch	st2			; su.i*xmul+su.j*ymul | v/z | 1/z
	fadd	dword [su_k]		; u/z | v/z | 1/z
	fxch	st2			; 1/z | v/z | u/z
	fxch	st1			; v/z | 1/z | u/z

; if lighting is on, fill out the light table
	mov	al,[plane_shade]
	test	al,al
	jz	.litup

	push	ebx
	fild	dword [esp]		; width | v/z | 1/z | u/z
	fmul	dword [sz_i]		; width*sz.i | v/z | 1/z | u/z
	fadd	st0,st2			; 1/endz | v/z | 1/z | u/z
	fld	st2			; 1/z | 1/endz | v/z | 1/z | u/z
	fmul	dword [planelightfloat]
	fxch	st1
	fmul	dword [planelightfloat]
	sub	esp,8
	fistp	dword [esp]
	fistp	dword [esp+4]
	call	R_CalcTiltedLighting
	add	esp, 12
	xor	eax, eax
	
.litup	add	esp, 8

; calculate initial z, u, and v values
	fld	st1			; 1/z | v/z | 1/z | u/z
	fdivr	dword [fp_1]		; z | v/z | 1/z | u/z

	fld	st3			; u/z | z | v/z | 1/z | u/z
	fmul	st0,st1			; u | z | v/z | 1/z | u/z
	fld	st2			; v/z | u | z | v/z | 1/z | u/z
	fmulp	st2,st0			; u | v | v/z | 1/z | u/z
	fld	st0
	fistp	qword [start_u]
	fld	st1
	fistp	qword [start_v]

	cmp	ebx,7			; Do we have at least 8 pixels to plot?
	jl	near ShortStrip

; yes, we do, so figure out tex coords at end of this span

; multiply i values by span length (8)
	fld	dword [su_i]		; su.i
	fmul	dword [fp_8]		; su.i*8
	fld	dword [sv_i]		; sv.i | su.i*8
	fmul	dword [fp_8]		; sv.i*8 | su.i*8
	fld	dword [sz_i]		; sz.i | sv.i*8 | su.i*8
	fmul	dword [fp_8]		; sz.i*8 | sv.i*8 | su.i*8
	fxch	st2			; su.i*8 | sv.i*8 | sz.i*8
	fstp	qword [step_uz]		; sv.i*8 | sz.i*8
	fstp	qword [step_vz]		; sz.i*8
	fst	qword [step_iz]		; sz.i*8

; find tex coords at start of next span
	faddp	st4
	fld	qword [step_vz]
	faddp	st3
	fld	qword [step_uz]
	faddp	st5

	fld	st3			; 1/z | u | v | v/z | 1/z | u/z
	fdivr	dword [fp_1]		; z | u | v | v/z | 1/z | u/z
	fst	dword [end_z]
	fld	st5			; u/z | z | u | v | v/z | 1/z | u/z
	fmul	st0,st1			; u' | z | u | v | v/z | 1/z | u/z
	fxch	st1			; z | u' | u | v | v/z | 1/z | u/z
	fmul	st0,st4			; v' | u' | u | v | v/z | 1/z | u/z
	fxch	st3			; v | u' | u | v' | v/z | 1/z | u/z

; now subtract to get stepping values for this span
	fsubr	st0,st3			; v'-v | u' | u | v' | v/z | 1/z | u/z
	fxch	st2			; u | u' | v'-v | v' | v/z | 1/z | u/z
	fsubr	st0,st1			; u'-u | u' | v'-v | v' | v/z | 1/z | u/z
	fxch	st2			; v'-v | u' | u'-u | v' | v/z | 1/z | u/z
	fmul	dword [fp_8recip]	; vstep | u' | u'-u | v' | v/z | 1/z | u/z
	fxch	st1			; u' | vstep | u'-u | v' | v/z | 1/z | u/z
	fxch	st2			; u'-u | vstep | u' | v' | v/z | 1/z | u/z
	fmul	dword [fp_8recip]	; ustep | vstep | u' | v' | v/z | 1/z | u/z
	fxch	st1			; vstep | ustep | u' | v' | v/z | 1/z | u/z
	fistp	qword [step_v]		; ustep | u' | v' | v/z | 1/z | u/z
	fistp	qword [step_u]		; u | v | v/z | 1/z | u/z

FullSpan:
	xor	eax,eax
	cmp	ebx,15			; is there another complete span after this one?
	jl	NextIsShort

; there is a complete span after this one
	fld	qword [step_iz]
	faddp	st4,st0
	fld	qword [step_vz]
	faddp	st3,st0
	fld	qword [step_uz]
	faddp	st5,st0
	jmp	StartDiv

NextIsShort:
	cmp	ebx,8			; if span is no more than 1 pixel, then we already
	jle	DrawFullSpan		; know everything we need to draw it

	fld	dword [sz_i]		; sz.i | u | v | v/z | 1/z | u/z
	fmul	dword [fp_quickint-8*4+ebx*4]
	fld	dword [sv_i]		; sv.i | sz.i | u | v | v/z | 1/z | u/z
	fmul	dword [fp_quickint-8*4+ebx*4]
	fld	dword [su_i]		; su.i | sv.i | sz.i | u | v | v/z | 1/z | u/z
	fmul	dword [fp_quickint-8*4+ebx*4]
	fxch	st2			; sz.i | sv.i | su.i | u | v | v/z | 1/z | u/z
	faddp	st6,st0			; sv.i | su.i | u | v | v/z | 1/z | u/z
	faddp	st4,st0			; su.i | u | v | v/z | 1/z | u/z
	faddp	st5,st0			; u | v | v/z | 1/z | u/z

StartDiv:
	fld	st3			; 1/z | u | v | v/z | 1/z | u/z
	fdivr	dword [fp_1]		; z | u | v | v/z | 1/z | u/z

DrawFullSpan
	mov	ecx,[start_v]
	mov	edx,[start_u]

	add	ecx,[pviewy]
	add	edx,[pviewx]

	mov	esi,ecx
	mov	ebp,edx
	shr	ebp,26
	and	esi,0xfc000000
	shr	esi,20
	add	ecx,[step_v]
	add	edx,[step_u]
fetch1	mov	al,[ebp+esi+SPACEFILLER4]
	mov	ebp,[tiltlighting+ebx*4]
	mov	esi,ecx
	mov	al,[ebp+eax]
	mov	ebp,edx
	mov	[edi+0],al

	shr	ebp,26
	and	esi,0xfc000000
	shr	esi,20
	add	ecx,[step_v]
	add	edx,[step_u]
fetch2	mov	al,[ebp+esi+SPACEFILLER4]
	mov	ebp,[tiltlighting+ebx*4-4]
	mov	esi,ecx
	mov	al,[ebp+eax]
	mov	ebp,edx
	mov	[edi+1],al

	shr	ebp,26
	and	esi,0xfc000000
	shr	esi,20
	add	ecx,[step_v]
	add	edx,[step_u]
fetch3	mov	al,[ebp+esi+SPACEFILLER4]
	mov	ebp,[tiltlighting+ebx*4-8]
	mov	esi,ecx
	mov	al,[ebp+eax]
	mov	ebp,edx
	mov	[edi+2],al

	shr	ebp,26
	and	esi,0xfc000000
	shr	esi,20
	add	ecx,[step_v]
	add	edx,[step_u]
fetch4	mov	al,[ebp+esi+SPACEFILLER4]
	mov	ebp,[tiltlighting+ebx*4-12]
	mov	esi,ecx
	mov	al,[ebp+eax]
	mov	ebp,edx
	mov	[edi+3],al

	shr	ebp,26
	and	esi,0xfc000000
	shr	esi,20
	add	ecx,[step_v]
	add	edx,[step_u]
fetch5	mov	al,[ebp+esi+SPACEFILLER4]
	mov	ebp,[tiltlighting+ebx*4-16]
	mov	esi,ecx
	mov	al,[ebp+eax]
	mov	ebp,edx
	mov	[edi+4],al

	shr	ebp,26
	and	esi,0xfc000000
	shr	esi,20
	add	ecx,[step_v]
	add	edx,[step_u]
fetch6	mov	al,[ebp+esi+SPACEFILLER4]
	mov	ebp,[tiltlighting+ebx*4-20]
	mov	esi,ecx
	mov	al,[ebp+eax]
	mov	ebp,edx
	mov	[edi+5],al

	shr	ebp,26
	and	esi,0xfc000000
	shr	esi,20
	add	ecx,[step_v]
	add	edx,[step_u]
fetch7	mov	al,[ebp+esi+SPACEFILLER4]
	mov	ebp,[tiltlighting+ebx*4-24]
	shr	edx,26
	mov	al,[ebp+eax]
	and	ecx,0xfc000000
	mov	[edi+6],al

	shr	ecx,20
	mov	ebp,[tiltlighting+ebx*4-28]
fetch8	mov	al,[edx+ecx+SPACEFILLER4]
	mov	al,[ebp+eax]
	mov	[edi+7],al
	add	edi,8

	sub	ebx,8
	jl	near Done	

	fld	st1
	fistp	qword [start_u]
	fld	st2
	fistp	qword [start_v]

	cmp	ebx,7
	jl	near EndIsShort

	fst	dword [end_z]
	fld	st5			; u/z | z | u | v | v/z | 1/z | u/z
	fmul	st0,st1			; u' | z | u | v | v/z | 1/z | u/z
	fxch	st1			; z | u' | u | v | v/z | 1/z | u/z
	fmul	st0,st4			; v' | u' | u | v | v/z | 1/z | u/z
	fxch	st3			; v | u' | u | v' | v/z | 1/z | u/z
	fsubr	st0,st3			; v'-v | u' | u | v' | v/z | 1/z | u/z
	fxch	st2			; u | u' | v'-v | v' | v/z | 1/z | u/z
	fsubr	st0,st1			; u'-u | u' | v'-v | v' | v/z | 1/z | u/z
	fxch	st2			; v'-v | u' | u'-u | v' | v/z | 1/z | u/z
	fmul	dword [fp_8recip]	; vstep | u' | u'-u | v' | v/z | 1/z | u/z
	fxch	st1			; u' | vstep | u'-u | v' | v/z | 1/z | u/z
	fxch	st2			; u'-u | vstep | u' | v' | v/z | 1/z | u/z
	fmul	dword [fp_8recip]	; ustep | vstep | u' | v' | v/z | 1/z | u/z
	fxch	st1			; vstep | ustep | u' | v' | v/z | 1/z | u/z
	fistp	qword [step_v]		; ustep | u' | v' | v/z | 1/z | u/z
	fistp	qword [step_u]		; u | v | v/z | 1/z | u/z
	jmp	FullSpan

OnlyOnePixelAtEnd:
	fld	st0
	fistp	qword [start_u]
	fld	st1
	fistp	qword [start_v]

OnlyOnePixel:
	mov	edx,[start_u]
	mov	ecx,[start_v]
	add	edx,[pviewx]
	add	ecx,[pviewy]
	shr	edx,26
	and	ecx,0xfc000000
	shr	ecx,20
	mov	ebp,[tiltlighting]
fetch9	mov	al,[ecx+edx+SPACEFILLER4]
	mov	al,[ebp+eax]
	mov	[edi],al

Done:
	fcompp
	fcompp
	fstp	st0

	pop	ebp
	pop	edi
	pop	esi
	pop	ebx
	ret

ShortStrip:
	cmp	ebx,0
	jle	near OnlyOnePixel

MoreThanOnePixel:
	fld	dword [sz_i]		; sz.i | u | v | v/z | 1/z | u/z
	fmul	dword [fp_quickint+ebx*4]
	fld	dword [sv_i]		; sv.i | sz.i | u | v | v/z | 1/z | u/z
	fmul	dword [fp_quickint+ebx*4]
	fld	dword [su_i]		; su.i | sv.i | sz.i | u | v | v/z | 1/z | u/z
	fmul	dword [fp_quickint+ebx*4]
	fxch	st2			; sz.i | sv.i | su.i | u | v | v/z | 1/z | u/z
	faddp	st6,st0			; sv.i | su.i | u | v | v/z | 1/z | u/z
	faddp	st4,st0			; su.i | u | v | v/z | 1/z | u/z
	faddp	st5,st0			; u | v | v/z | 1/z | u/z
	fld	st3			; 1/z | u | v | v/z | 1/z | u/z
	fdivr	dword [fp_1]		; z | u | v | v/z | 1/z | u/z
	jmp	CalcPartialSteps

EndIsShort:
	cmp	ebx,0
	je	near OnlyOnePixelAtEnd

CalcPartialSteps:
	fst	dword [end_z]
	fld	st5			; u/z | z | u | v | v/z | 1/z | u/z
	fmul	st0,st1			; u' | z | u | v | v/z | 1/z | u/z
	fxch	st1			; z | u' | u | v | v/z | 1/z | u/z
	fmul	st0,st4			; v' | u' | u | v | v/z | 1/z | u/z
	fxch	st1			; u' | v' | u | v | v/z | 1/z | u/z
	fsubrp	st2,st0			; v' | u'-u | v | v/z | 1/z | u/z
	fsubrp	st2,st0			; u'-u | v'-v | v/z | 1/z | u/z
	fmul	dword [spanrecips+ebx*4] ;ustep | v'-v | v/z | 1/z | u/z
	fxch	st1			; v'-v | ustep | v/z | 1/z | u/z
	fmul	dword [spanrecips+ebx*4] ;vstep | ustep | v/z | 1/z | u/z
	fxch	st1			; ustep | vstep | v/z | 1/z | u/z
	fistp	qword [step_u]		; vstep | v/z | 1/z | u/z
	fistp	qword [step_v]		; v/z | 1/z | u/z

	mov	ecx,[start_v]
	mov	edx,[start_u]

	add	ecx,[pviewy]
	add	edx,[pviewx]

	mov	esi,ecx
	mov	ebp,edx

endloop	shr	ebp,26
	and	esi,0xfc000000

	shr	esi,20
	inc	edi

	add	ecx,[step_v]
	add	edx,[step_u]

fetch10	mov	al,[ebp+esi+SPACEFILLER4]
	mov	ebp,[tiltlighting+ebx*4]

	mov	esi,ecx
	dec	ebx

	mov	al,[ebp+eax]
	mov	ebp,edx

	mov	[edi-1],al
	jge	endloop

	fcompp
	fstp	st0

	pop	ebp
	pop	edi
	pop	esi
	pop	ebx
	ret