/* Partial PLT profile trampoline to save and restore x86-64 vector
   registers.
   Copyright (C) 2009 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifdef RESTORE_AVX
	/* This is to support AVX audit modules.  */
# if HAVE_NO_AVX512_ASM_SUPPORT
	/* Restore AVX-512 registers. Use .byte becaues we lack assembler support.  */
	.byte 0x62,0xf1,0xfe,0x48,0x7f,0x44,0x24,0x03 # vmovdqu64 %zmm0,0xc0(%rsp)
	.byte 0x62,0xf1,0xfe,0x48,0x7f,0x4c,0x24,0x04 # vmovdqu64 %zmm1,0x100(%rsp)
	.byte 0x62,0xf1,0xfe,0x48,0x7f,0x54,0x24,0x05 # vmovdqu64 %zmm2,0x140(%rsp)
	.byte 0x62,0xf1,0xfe,0x48,0x7f,0x5c,0x24,0x06 # vmovdqu64 %zmm3,0x180(%rsp)
	.byte 0x62,0xf1,0xfe,0x48,0x7f,0x64,0x24,0x07 # vmovdqu64 %zmm4,0x1c0(%rsp)
	.byte 0x62,0xf1,0xfe,0x48,0x7f,0x6c,0x24,0x08 # vmovdqu64 %zmm5,0x200(%rsp)
	.byte 0x62,0xf1,0xfe,0x48,0x7f,0x74,0x24,0x09 # vmovdqu64 %zmm6,0x240(%rsp)
	.byte 0x62,0xf1,0xfe,0x48,0x7f,0x7c,0x24,0x0a # vmovdqu64 %zmm7,0x280(%rsp)
# else
	VMOV %VEC(0),		      (LR_VECTOR_OFFSET)(%rsp)
	VMOV %VEC(1), (LR_VECTOR_OFFSET +   VECTOR_SIZE)(%rsp)
	VMOV %VEC(2), (LR_VECTOR_OFFSET + VECTOR_SIZE*2)(%rsp)
	VMOV %VEC(3), (LR_VECTOR_OFFSET + VECTOR_SIZE*3)(%rsp)
	VMOV %VEC(4), (LR_VECTOR_OFFSET + VECTOR_SIZE*4)(%rsp)
	VMOV %VEC(5), (LR_VECTOR_OFFSET + VECTOR_SIZE*5)(%rsp)
	VMOV %VEC(6), (LR_VECTOR_OFFSET + VECTOR_SIZE*6)(%rsp)
	VMOV %VEC(7), (LR_VECTOR_OFFSET + VECTOR_SIZE*7)(%rsp)
# endif

	/* Save xmm0-xmm7 registers to detect if any of them are
	   changed by audit module.  */
	vmovdqa %xmm0,		    (LR_SIZE)(%rsp)
	vmovdqa %xmm1, (LR_SIZE +   XMM_SIZE)(%rsp)
	vmovdqa %xmm2, (LR_SIZE + XMM_SIZE*2)(%rsp)
	vmovdqa %xmm3, (LR_SIZE + XMM_SIZE*3)(%rsp)
	vmovdqa %xmm4, (LR_SIZE + XMM_SIZE*4)(%rsp)
	vmovdqa %xmm5, (LR_SIZE + XMM_SIZE*5)(%rsp)
	vmovdqa %xmm6, (LR_SIZE + XMM_SIZE*6)(%rsp)
	vmovdqa %xmm7, (LR_SIZE + XMM_SIZE*7)(%rsp)
#endif

	movq %rsp, %rcx		# La_x86_64_regs pointer to %rcx.
	movq 48(%rbx), %rdx	# Load return address if needed.
	movq 40(%rbx), %rsi	# Copy args pushed by PLT in register.
	movq 32(%rbx), %rdi	# %rdi: link_map, %rsi: reloc_index
	leaq 16(%rbx), %r8
	call _dl_profile_fixup	# Call resolver.

	movq %rax, %r11		# Save return value.

	movq 8(%rbx), %rax	# Get back register content.
	movq LR_RDX_OFFSET(%rsp), %rdx
	movq  LR_R8_OFFSET(%rsp), %r8
	movq  LR_R9_OFFSET(%rsp), %r9

	movaps		    (LR_XMM_OFFSET)(%rsp), %xmm0
	movaps	 (LR_XMM_OFFSET + XMM_SIZE)(%rsp), %xmm1
	movaps (LR_XMM_OFFSET + XMM_SIZE*2)(%rsp), %xmm2
	movaps (LR_XMM_OFFSET + XMM_SIZE*3)(%rsp), %xmm3
	movaps (LR_XMM_OFFSET + XMM_SIZE*4)(%rsp), %xmm4
	movaps (LR_XMM_OFFSET + XMM_SIZE*5)(%rsp), %xmm5
	movaps (LR_XMM_OFFSET + XMM_SIZE*6)(%rsp), %xmm6
	movaps (LR_XMM_OFFSET + XMM_SIZE*7)(%rsp), %xmm7

#ifdef RESTORE_AVX
	/* Check if any xmm0-xmm7 registers are changed by audit
	   module.  */
	vpcmpeqq (LR_SIZE)(%rsp), %xmm0, %xmm8
	vpmovmskb %xmm8, %esi
	cmpl $0xffff, %esi
	je 2f
	vmovdqa	%xmm0, (LR_VECTOR_OFFSET)(%rsp)
	jmp 1f
# if HAVE_NO_AVX512_ASM_SUPPORT
2:	.byte 0x62,0xf1,0xfe,0x48,0x6f,0x44,0x24,0x03 # vmovdqu64 0xc0(%rsp),%zmm0
# else
2:	VMOV (LR_VECTOR_OFFSET)(%rsp), %VEC(0)
# endif
	vmovdqa	%xmm0, (LR_XMM_OFFSET)(%rsp)

1:	vpcmpeqq (LR_SIZE + XMM_SIZE)(%rsp), %xmm1, %xmm8
	vpmovmskb %xmm8, %esi
	cmpl $0xffff, %esi
	je 2f
	vmovdqa	%xmm1, (LR_VECTOR_OFFSET + VECTOR_SIZE)(%rsp)
	jmp 1f
# if HAVE_NO_AVX512_ASM_SUPPORT
2:	.byte 0x62,0xf1,0xfe,0x48,0x6f,0x4c,0x24,0x04 # vmovdqu64 0x100(%rsp),%zmm1
# else
2:	VMOV (LR_VECTOR_OFFSET + VECTOR_SIZE)(%rsp), %VEC(1)
# endif
	vmovdqa	%xmm1, (LR_XMM_OFFSET + XMM_SIZE)(%rsp)

1:	vpcmpeqq (LR_SIZE + XMM_SIZE*2)(%rsp), %xmm2, %xmm8
	vpmovmskb %xmm8, %esi
	cmpl $0xffff, %esi
	je 2f
	vmovdqa	%xmm2, (LR_VECTOR_OFFSET + VECTOR_SIZE*2)(%rsp)
	jmp 1f
# if HAVE_NO_AVX512_ASM_SUPPORT
2:	.byte 0x62,0xf1,0xfe,0x48,0x6f,0x54,0x24,0x05 # vmovdqu64 0x140(%rsp),%zmm2
# else
2:	VMOV (LR_VECTOR_OFFSET + VECTOR_SIZE*2)(%rsp), %VEC(2)
# endif
	vmovdqa	%xmm2, (LR_XMM_OFFSET + XMM_SIZE*2)(%rsp)

1:	vpcmpeqq (LR_SIZE + XMM_SIZE*3)(%rsp), %xmm3, %xmm8
	vpmovmskb %xmm8, %esi
	cmpl $0xffff, %esi
	je 2f
	vmovdqa	%xmm3, (LR_VECTOR_OFFSET + VECTOR_SIZE*3)(%rsp)
	jmp 1f
# if HAVE_NO_AVX512_ASM_SUPPORT
2:	.byte 0x62,0xf1,0xfe,0x48,0x6f,0x5c,0x24,0x06 # vmovdqu64 0x180(%rsp),%zmm3
# else
2:	VMOV (LR_VECTOR_OFFSET + VECTOR_SIZE*3)(%rsp), %VEC(3)
# endif
	vmovdqa	%xmm3, (LR_XMM_OFFSET + XMM_SIZE*3)(%rsp)

1:	vpcmpeqq (LR_SIZE + XMM_SIZE*4)(%rsp), %xmm4, %xmm8
	vpmovmskb %xmm8, %esi
	cmpl $0xffff, %esi
	je 2f
	vmovdqa	%xmm4, (LR_VECTOR_OFFSET + VECTOR_SIZE*4)(%rsp)
	jmp 1f
# if HAVE_NO_AVX512_ASM_SUPPORT
2:	.byte 0x62,0xf1,0xfe,0x48,0x6f,0x64,0x24,0x07 # vmovdqu64 0x1c0(%rsp),%zmm4
# else
2:	VMOV (LR_VECTOR_OFFSET + VECTOR_SIZE*4)(%rsp), %VEC(4)
# endif
	vmovdqa	%xmm4, (LR_XMM_OFFSET + XMM_SIZE*4)(%rsp)

1:	vpcmpeqq (LR_SIZE + XMM_SIZE*5)(%rsp), %xmm5, %xmm8
	vpmovmskb %xmm8, %esi
	cmpl $0xffff, %esi
	je 2f
	vmovdqa	%xmm5, (LR_VECTOR_OFFSET + VECTOR_SIZE*5)(%rsp)
	jmp 1f
# if HAVE_NO_AVX512_ASM_SUPPORT
2:	.byte 0x62,0xf1,0xfe,0x48,0x6f,0x6c,0x24,0x08 # vmovdqu64 0x200(%rsp),%zmm5
# else
2:	VMOV (LR_VECTOR_OFFSET + VECTOR_SIZE*5)(%rsp), %VEC(5)
# endif
	vmovdqa	%xmm5, (LR_XMM_OFFSET + XMM_SIZE*5)(%rsp)

1:	vpcmpeqq (LR_SIZE + XMM_SIZE*6)(%rsp), %xmm6, %xmm8
	vpmovmskb %xmm8, %esi
	cmpl $0xffff, %esi
	je 2f
	vmovdqa	%xmm6, (LR_VECTOR_OFFSET + VECTOR_SIZE*6)(%rsp)
	jmp 1f
# if HAVE_NO_AVX512_ASM_SUPPORT
2:	.byte 0x62,0xf1,0xfe,0x48,0x6f,0x74,0x24,0x09 # vmovdqu64 0x240(%rsp),%zmm6
# else
2:	VMOV (LR_VECTOR_OFFSET + VECTOR_SIZE*6)(%rsp), %VEC(6)
# endif
	vmovdqa	%xmm6, (LR_XMM_OFFSET + XMM_SIZE*6)(%rsp)

1:	vpcmpeqq (LR_SIZE + XMM_SIZE*7)(%rsp), %xmm7, %xmm8
	vpmovmskb %xmm8, %esi
	cmpl $0xffff, %esi
	je 2f
	vmovdqa	%xmm7, (LR_VECTOR_OFFSET + VECTOR_SIZE*7)(%rsp)
	jmp 1f
# if HAVE_NO_AVX512_ASM_SUPPORT
2:	.byte 0x62,0xf1,0xfe,0x48,0x6f,0x7c,0x24,0x0a # vmovdqu64 0x280(%rsp),%zmm7
# else
2:	VMOV (LR_VECTOR_OFFSET + VECTOR_SIZE*7)(%rsp), %VEC(7)
# endif
	vmovdqa	%xmm7, (LR_XMM_OFFSET + XMM_SIZE*7)(%rsp)

1:
#endif
	movq 16(%rbx), %r10	# Anything in framesize?
	testq %r10, %r10
	jns 3f

	/* There's nothing in the frame size, so there
	   will be no call to the _dl_call_pltexit. */

	/* Get back registers content.  */
	movq LR_RCX_OFFSET(%rsp), %rcx
	movq LR_RSI_OFFSET(%rsp), %rsi
	movq LR_RDI_OFFSET(%rsp), %rdi

	movq %rbx, %rsp
	movq (%rsp), %rbx
	cfi_restore(rbx)
	cfi_def_cfa_register(%rsp)

	addq $48, %rsp		# Adjust the stack to the return value
				# (eats the reloc index and link_map)
	cfi_adjust_cfa_offset(-48)
	jmp *%r11		# Jump to function address.

3:
	cfi_adjust_cfa_offset(48)
	cfi_rel_offset(%rbx, 0)
	cfi_def_cfa_register(%rbx)

	/* At this point we need to prepare new stack for the function
	   which has to be called.  We copy the original stack to a
	   temporary buffer of the size specified by the 'framesize'
	   returned from _dl_profile_fixup */

	leaq LR_RSP_OFFSET(%rbx), %rsi	# stack
	addq $8, %r10
	andq $0xfffffffffffffff0, %r10
	movq %r10, %rcx
	subq %r10, %rsp
	movq %rsp, %rdi
	shrq $3, %rcx
	rep
	movsq

	movq 24(%rdi), %rcx	# Get back register content.
	movq 32(%rdi), %rsi
	movq 40(%rdi), %rdi

	call *%r11

	mov 24(%rbx), %rsp	# Drop the copied stack content

	/* Now we have to prepare the La_x86_64_retval structure for the
	   _dl_call_pltexit.  The La_x86_64_regs is being pointed by rsp now,
	   so we just need to allocate the sizeof(La_x86_64_retval) space on
	   the stack, since the alignment has already been taken care of. */
#ifdef RESTORE_AVX
	/* sizeof(La_x86_64_retval).  Need extra space for 2 SSE
	   registers to detect if xmm0/xmm1 registers are changed
	   by audit module.  */
	subq $(LRV_SIZE + XMM_SIZE*2), %rsp
#else
	subq $LRV_SIZE, %rsp	# sizeof(La_x86_64_retval)
#endif
	movq %rsp, %rcx		# La_x86_64_retval argument to %rcx.

	/* Fill in the La_x86_64_retval structure.  */
	movq %rax, LRV_RAX_OFFSET(%rcx)
	movq %rdx, LRV_RDX_OFFSET(%rcx)

	movaps %xmm0, LRV_XMM0_OFFSET(%rcx)
	movaps %xmm1, LRV_XMM1_OFFSET(%rcx)

#ifdef RESTORE_AVX
	/* This is to support AVX audit modules.  */
# if HAVE_NO_AVX512_ASM_SUPPORT
	.byte 0x62,0xf1,0xfe,0x48,0x7f,0x81,0x50,0x00,0x00,0x00 # vmovdqu64 %zmm0,0x50(%rcx)
	.byte 0x62,0xf1,0xfe,0x48,0x7f,0x89,0x90,0x00,0x00,0x00 # vmovdqu64 %zmm1,0x90(%rcx)
# else
	VMOV %VEC(0), LRV_VECTOR0_OFFSET(%rcx)
	VMOV %VEC(1), LRV_VECTOR1_OFFSET(%rcx)
# endif

	/* Save xmm0/xmm1 registers to detect if they are changed
	   by audit module.  */
	vmovdqa %xmm0,		  (LRV_SIZE)(%rcx)
	vmovdqa %xmm1, (LRV_SIZE + XMM_SIZE)(%rcx)
#endif

	fstpt LRV_ST0_OFFSET(%rcx)
	fstpt LRV_ST1_OFFSET(%rcx)

	movq 24(%rbx), %rdx	# La_x86_64_regs argument to %rdx.
	movq 40(%rbx), %rsi	# Copy args pushed by PLT in register.
	movq 32(%rbx), %rdi	# %rdi: link_map, %rsi: reloc_index
	call _dl_call_pltexit

	/* Restore return registers.  */
	movq LRV_RAX_OFFSET(%rsp), %rax
	movq LRV_RDX_OFFSET(%rsp), %rdx

	movaps LRV_XMM0_OFFSET(%rsp), %xmm0
	movaps LRV_XMM1_OFFSET(%rsp), %xmm1

#ifdef RESTORE_AVX
	/* Check if xmm0/xmm1 registers are changed by audit module.  */
	vpcmpeqq (LRV_SIZE)(%rsp), %xmm0, %xmm2
	vpmovmskb %xmm2, %esi
	cmpl $0xffff, %esi
	jne 1f
# if HAVE_NO_AVX512_ASM_SUPPORT
	.byte 0x62,0xf1,0xfe,0x48,0x6f,0x84,0x24,0x50,0x00,0x00,0x00 # vmovdqu64 0x50(%rsp),%zmm0
# else
	VMOV LRV_VECTOR0_OFFSET(%rsp), %VEC(0)
# endif

1:	vpcmpeqq (LRV_SIZE + XMM_SIZE)(%rsp), %xmm1, %xmm2
	vpmovmskb %xmm2, %esi
	cmpl $0xffff, %esi
	jne 1f
# if HAVE_NO_AVX512_ASM_SUPPORT
	.byte 0x62,0xf1,0xfe,0x48,0x6f,0x8c,0x24,0x90,0x00,0x00,0x00 # vmovdqu64 0x90(%rsp),%zmm1
# else
	VMOV LRV_VECTOR1_OFFSET(%rsp), %VEC(1)
# endif

1:
#endif

	fldt LRV_ST1_OFFSET(%rsp)
	fldt LRV_ST0_OFFSET(%rsp)

	movq %rbx, %rsp
	movq (%rsp), %rbx
	cfi_restore(rbx)
	cfi_def_cfa_register(%rsp)

	addq $48, %rsp		# Adjust the stack to the return value
				# (eats the reloc index and link_map)
	cfi_adjust_cfa_offset(-48)
	retq

#ifdef MORE_CODE
	cfi_adjust_cfa_offset(48)
	cfi_rel_offset(%rbx, 0)
	cfi_def_cfa_register(%rbx)
# undef MORE_CODE
#endif
