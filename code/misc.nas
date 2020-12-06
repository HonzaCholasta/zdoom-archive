;* Miscellanoeus assembly functions. Many of these could have
;* easily been done using VC++ inline assembly, but that's not
;* nearly as portable as using NASM.

BITS 32

%ifdef M_TARGET_LINUX

extern	HaveRDTSC
extern	CPUFamily
extern	CPUModel
extern	CPUStepping

%else

extern	_HaveRDTSC
extern	_CPUFamily
extern	_CPUModel
extern	_CPUStepping

%define	HaveRDTSC	_HaveRDTSC
%define	CPUFamily	_CPUFamily
%define	CPUModel	_CPUModel
%define CPUStepping	_CPUStepping

%endif

%ifdef M_TARGET_WATCOM
  SEGMENT DATA PUBLIC ALIGN=16 CLASS=DATA USE32
  SEGMENT DATA
%else
  SECTION .data
%endif


%ifdef M_TARGET_WATCOM
  SEGMENT CODE PUBLIC ALIGN=16 CLASS=CODE USE32
  SEGMENT CODE
%else
  SECTION .text
%endif

;-----------------------------------------------------------
;
; FixedMul_ASM
;
; Assembly version of FixedMul.
;
;-----------------------------------------------------------

GLOBAL	FixedMul_ASM
GLOBAL	_FixedMul_ASM

	align	16

FixedMul_ASM:
_FixedMul_ASM:
	mov	eax,[esp+4]
	imul	dword [esp+8]
	shrd	eax,edx,16
	ret

; Version of FixedMul_ASM for MSVC's __fastcall

GLOBAL @FixedMul_ASM@8

	align	16

@FixedMul_ASM@8:
	mov	eax,ecx
	imul	edx
	shrd	eax,edx,16
	ret

;-----------------------------------------------------------
;
; FixedDiv_ASM
;
; This is mostly what VC++ outputted for the original C
; routine (which used to contain inline assembler
; but doesn't anymore).
;
;-----------------------------------------------------------

GLOBAL	FixedDiv_ASM
GLOBAL	_FixedDiv_ASM

	align	16

FixedDiv_ASM:
_FixedDiv_ASM:
	push	ebp
	push	ebx

; 89   : 	if ((abs (a) >> 14) >= abs(b))

	mov	edx,[esp+12]		; get a
	 mov	eax,[esp+16]		; get b
	mov	ecx,edx
	 mov	ebx,eax
	sar	ecx,31
	 mov	ebp,edx
	sar	ebx,31
	 xor	ebp,ecx
	xor	eax,ebx
	 sub	ebp,ecx			; ebp is now abs(a)
	sub	eax,ebx			; eax is now abs(b)

	sar	ebp,14
	 pop	ebx
	cmp	ebp,eax
	 jl	.L206

; 90   : 		return (a^b)<0 ? MININT : MAXINT;

	xor	edx,[esp+12]
	 xor	eax,eax
	pop	ebp
	 test	edx,edx
	setl	al
	add	eax,0x7fffffff
	ret

	align	16

.L206:
	sar	edx,16			; (edx = ----aaaa)
	 mov	eax,[esp+8]		; (eax = aaaaAAAA)
	shl	eax,16			; (eax = AAAA0000)
	 pop	ebp
	idiv	dword [esp+8]

	ret



;-----------------------------------------------------------
;
; CheckMMX
;
; Checks for the presence of MMX instructions on the
; current processor. This code is adapted from the samples
; in AMD's document entitled "AMD-K6™ MMX Processor
; Multimedia Extensions." Also fills in the vendor
; information string.
;
;-----------------------------------------------------------

GLOBAL	CheckMMX
GLOBAL	_CheckMMX

; boolean CheckMMX (char *vendorid)

CheckMMX:
_CheckMMX:
	xor	eax,eax
	push	ebx
	push	eax		; Will be set true if MMX is present

	pushfd			; save EFLAGS
	pop	eax		; store EFLAGS in EAX
	mov	ebx,eax		; save in EBX for later testing
	xor	eax,0x00200000	; toggle bit 21
	push	eax		; put to stack
	popfd			; save changed EAX to EFLAGS
	pushfd			; push EFLAGS to TOS
	pop	eax		; store EFLAGS in EAX
	cmp	eax,ebx		; see if bit 21 has changed
	jz	.nommx		; if no change, no CPUID

	mov	eax,0		; setup function 0
	CPUID			; call the function
	mov	eax,[esp+12]	; fill in the vendorid string
	mov	[eax],ebx
	mov	[eax+4],edx
	mov	[eax+8],ecx

	xor	eax,eax
	mov	eax,1		; setup function 1
	CPUID			; call the function
	and	ah,0xf
	mov	[CPUFamily],ah
	mov	ah,al
	shr	ah,4
	and	al,0xf
	mov	[CPUModel],ah
	mov	[CPUStepping],al
	xor	eax,eax
	test	edx,0x00000010	; test for RDTSC
	setnz	al		; al=1 if RDTSC is available
	mov	[HaveRDTSC],eax

	test	edx,0x00800000	; test 23rd bit
	jz	.nommx

	inc	dword [esp]
.nommx	pop	eax
	pop	ebx
	ret

;-----------------------------------------------------------
;
; EndMMX
;
; Signal the end of MMX code
;
;-----------------------------------------------------------

GLOBAL	EndMMX
GLOBAL	_EndMMX

EndMMX:
_EndMMX:
	emms
	ret
