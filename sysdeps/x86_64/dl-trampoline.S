/* PLT trampolines.  x86-64 version.
   Copyright (C) 2004, 2005, 2007, 2009 Free Software Foundation, Inc.
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

#include <config.h>
#include <sysdep.h>
#include <link-defines.h>

	.text
	.globl _dl_runtime_resolve
	.type _dl_runtime_resolve, @function
	.align 16
	cfi_startproc
_dl_runtime_resolve:
	subq $56,%rsp
	cfi_adjust_cfa_offset(72) # Incorporate PLT
	movq %rax,(%rsp)	# Preserve registers otherwise clobbered.
	movq %rcx, 8(%rsp)
	movq %rdx, 16(%rsp)
	movq %rsi, 24(%rsp)
	movq %rdi, 32(%rsp)
	movq %r8, 40(%rsp)
	movq %r9, 48(%rsp)
	movq 64(%rsp), %rsi	# Copy args pushed by PLT in register.
	movq 56(%rsp), %rdi	# %rdi: link_map, %rsi: reloc_index
	call _dl_fixup		# Call resolver.
	movq %rax, %r11		# Save return value
	movq 48(%rsp), %r9	# Get register content back.
	movq 40(%rsp), %r8
	movq 32(%rsp), %rdi
	movq 24(%rsp), %rsi
	movq 16(%rsp), %rdx
	movq 8(%rsp), %rcx
	movq (%rsp), %rax
	addq $72, %rsp		# Adjust stack(PLT did 2 pushes)
	cfi_adjust_cfa_offset(-72)
	jmp *%r11		# Jump to function address.
	cfi_endproc
	.size _dl_runtime_resolve, .-_dl_runtime_resolve


#ifndef PROF
	.globl _dl_runtime_profile
	.type _dl_runtime_profile, @function
	.align 16
	cfi_startproc

_dl_runtime_profile:
	cfi_adjust_cfa_offset(16) # Incorporate PLT
	/* The La_x86_64_regs data structure pointed to by the
	   fourth paramater must be 16-byte aligned.  This must
	   be explicitly enforced.  We have the set up a dynamically
	   sized stack frame.  %rbx points to the top half which
	   has a fixed size and preserves the original stack pointer.  */

	subq $32, %rsp		# Allocate the local storage.
	cfi_adjust_cfa_offset(32)
	movq %rbx, (%rsp)
	cfi_rel_offset(%rbx, 0)

	/* On the stack:
		56(%rbx)	parameter #1
		48(%rbx)	return address

		40(%rbx)	reloc index
		32(%rbx)	link_map

		24(%rbx)	La_x86_64_regs pointer
		16(%rbx)	framesize
		 8(%rbx)	rax
		  (%rbx)	rbx
	*/

	movq %rax, 8(%rsp)
	movq %rsp, %rbx
	cfi_def_cfa_register(%rbx)

	/* Actively align the La_x86_64_regs structure.  */
	andq $0xfffffffffffffff0, %rsp
# ifdef HAVE_AVX_SUPPORT
	/* sizeof(La_x86_64_regs).  Need extra space for 8 SSE registers
	   to detect if any xmm0-xmm7 registers are changed by audit
	   module.  */
	subq $(LR_SIZE + XMM_SIZE*8), %rsp
# else
	subq $LR_SIZE, %rsp		# sizeof(La_x86_64_regs)
# endif
	movq %rsp, 24(%rbx)

	/* Fill the La_x86_64_regs structure.  */
	movq %rdx, LR_RDX_OFFSET(%rsp)
	movq %r8,  LR_R8_OFFSET(%rsp)
	movq %r9,  LR_R9_OFFSET(%rsp)
	movq %rcx, LR_RCX_OFFSET(%rsp)
	movq %rsi, LR_RSI_OFFSET(%rsp)
	movq %rdi, LR_RDI_OFFSET(%rsp)
	movq %rbp, LR_RBP_OFFSET(%rsp)

	leaq 48(%rbx), %rax
	movq %rax, LR_RSP_OFFSET(%rsp)

	/* We always store the XMM registers even if AVX is available.
	   This is to provide backward binary compatility for existing
	   audit modules.  */
	movaps %xmm0,		   (LR_XMM_OFFSET)(%rsp)
	movaps %xmm1, (LR_XMM_OFFSET +   XMM_SIZE)(%rsp)
	movaps %xmm2, (LR_XMM_OFFSET + XMM_SIZE*2)(%rsp)
	movaps %xmm3, (LR_XMM_OFFSET + XMM_SIZE*3)(%rsp)
	movaps %xmm4, (LR_XMM_OFFSET + XMM_SIZE*4)(%rsp)
	movaps %xmm5, (LR_XMM_OFFSET + XMM_SIZE*5)(%rsp)
	movaps %xmm6, (LR_XMM_OFFSET + XMM_SIZE*6)(%rsp)
	movaps %xmm7, (LR_XMM_OFFSET + XMM_SIZE*7)(%rsp)

# ifdef HAVE_AVX_SUPPORT
	.data
L(have_avx):
	.zero 4
	.size L(have_avx), 4
	.previous

	cmpl	$0, L(have_avx)(%rip)
	jne	L(defined)
	movq	%rbx, %r11		# Save rbx
	movl	$1, %eax
	cpuid
	movq	%r11,%rbx		# Restore rbx
	xorl	%eax, %eax
	// AVX and XSAVE supported?
	andl	$((1 << 28) | (1 << 27)), %ecx
	cmpl	$((1 << 28) | (1 << 27)), %ecx
	jne	10f
	// AVX512 supported in processor?
	movq	%rbx, %r11		# Save rbx
	xorl	%ecx, %ecx
	mov	$0x7, %eax
	cpuid
	andl	$(1 << 16), %ebx
	xorl	%ecx, %ecx
	// Get XFEATURE_ENABLED_MASK
	xgetbv
	test	%ebx, %ebx
	movq	%r11, %rbx		# Restore rbx
	je	20f
	// Verify that XCR0[7:5] = '111b' and
	// XCR0[2:1] = '11b' which means
	// that zmm state is enabled
	andl	$0xe6, %eax
	cmpl	$0xe6, %eax
	jne	20f
	movl	%eax, L(have_avx)(%rip)
L(avx512):
#   define RESTORE_AVX
#   define HAVE_NO_AVX512_ASM_SUPPORT 1
#   define VMOV    vmovdqu64
#   define VEC(i)  zmm##i
#   define MORE_CODE
#   include "dl-trampoline.h"
#   undef VMOV
#   undef VEC
#   undef RESTORE_AVX
#   undef HAVE_NO_AVX512_ASM_SUPPORT
20:	andl	$0x6, %eax
10:	subl	$0x5, %eax
	movl	%eax, L(have_avx)(%rip)
	cmpl	$0, %eax

L(defined):
	js	L(no_avx)
	cmpl	$0xe6, L(have_avx)(%rip)
	je	L(avx512)


#  define RESTORE_AVX
#  define VMOV    vmovdqu
#  define VEC(i)  ymm##i
#  define MORE_CODE
#  include "dl-trampoline.h"

	.align 16
L(no_avx):
# endif

# undef RESTORE_AVX
# include "dl-trampoline.h"

	cfi_endproc
	.size _dl_runtime_profile, .-_dl_runtime_profile
#endif


#ifdef SHARED
	.globl _dl_x86_64_save_sse
	.type _dl_x86_64_save_sse, @function
	.align 16
	cfi_startproc
_dl_x86_64_save_sse:
# ifdef HAVE_AVX_SUPPORT
	cmpl	$0, L(have_avx)(%rip)
	jne	L(defined_5)
	movq	%rbx, %r11		# Save rbx
	movl	$1, %eax
	cpuid
	movq	%r11,%rbx		# Restore rbx
	xorl	%eax, %eax
	// AVX and XSAVE supported?
	andl	$((1 << 28) | (1 << 27)), %ecx
	cmpl	$((1 << 28) | (1 << 27)), %ecx
	jne	1f
	// AVX512 supported in a processor?
	movq	%rbx, %r11              # Save rbx
	xorl	%ecx,%ecx
	mov	$0x7,%eax
	cpuid
	andl	$(1 << 16), %ebx
	xorl	%ecx, %ecx
	// Get XFEATURE_ENABLED_MASK
	xgetbv
	test	%ebx, %ebx
	movq	%r11, %rbx		# Restore rbx
	je	2f
	// Verify that XCR0[7:5] = '111b' and
	// XCR0[2:1] = '11b' which means
	// that zmm state is enabled
	andl	$0xe6, %eax
	movl	%eax, L(have_avx)(%rip)
	cmpl	$0xe6, %eax
	je	L(avx512_5)

2:	andl	$0x6, %eax
1:	subl	$0x5, %eax
	movl	%eax, L(have_avx)(%rip)
	cmpl	$0, %eax

L(defined_5):
	js	L(no_avx5)
	cmpl	$0xe6, L(have_avx)(%rip)
	je	L(avx512_5)
 
	vmovdqa %ymm0, %fs:RTLD_SAVESPACE_SSE+0*YMM_SIZE
	vmovdqa %ymm1, %fs:RTLD_SAVESPACE_SSE+1*YMM_SIZE
	vmovdqa %ymm2, %fs:RTLD_SAVESPACE_SSE+2*YMM_SIZE
	vmovdqa %ymm3, %fs:RTLD_SAVESPACE_SSE+3*YMM_SIZE
	vmovdqa %ymm4, %fs:RTLD_SAVESPACE_SSE+4*YMM_SIZE
	vmovdqa %ymm5, %fs:RTLD_SAVESPACE_SSE+5*YMM_SIZE
	vmovdqa %ymm6, %fs:RTLD_SAVESPACE_SSE+6*YMM_SIZE
	vmovdqa %ymm7, %fs:RTLD_SAVESPACE_SSE+7*YMM_SIZE
	ret
L(avx512_5):
# Original instructions:
#	vmovdqu64 %zmm0, %fs:RTLD_SAVESPACE_SSE+0*ZMM_SIZE
#	vmovdqu64 %zmm1, %fs:RTLD_SAVESPACE_SSE+1*ZMM_SIZE
#	vmovdqu64 %zmm2, %fs:RTLD_SAVESPACE_SSE+2*ZMM_SIZE
#	vmovdqu64 %zmm3, %fs:RTLD_SAVESPACE_SSE+3*ZMM_SIZE
#	vmovdqu64 %zmm4, %fs:RTLD_SAVESPACE_SSE+4*ZMM_SIZE
#	vmovdqu64 %zmm5, %fs:RTLD_SAVESPACE_SSE+5*ZMM_SIZE
#	vmovdqu64 %zmm6, %fs:RTLD_SAVESPACE_SSE+6*ZMM_SIZE
#	vmovdqu64 %zmm7, %fs:RTLD_SAVESPACE_SSE+7*ZMM_SIZE
# Assembled instructions:
	.byte 0x64,0x62,0xf1,0xfe,0x48,0x7f,0x04,0x25,0x80,0x00,0x00,0x00 # vmovdqu64 %zmm0,%fs:0x80
	.byte 0x64,0x62,0xf1,0xfe,0x48,0x7f,0x0c,0x25,0xc0,0x00,0x00,0x00 # vmovdqu64 %zmm1,%fs:0xc0
	.byte 0x64,0x62,0xf1,0xfe,0x48,0x7f,0x14,0x25,0x00,0x01,0x00,0x00 # vmovdqu64 %zmm2,%fs:0x100
	.byte 0x64,0x62,0xf1,0xfe,0x48,0x7f,0x1c,0x25,0x40,0x01,0x00,0x00 # vmovdqu64 %zmm3,%fs:0x140
	.byte 0x64,0x62,0xf1,0xfe,0x48,0x7f,0x24,0x25,0x80,0x01,0x00,0x00 # vmovdqu64 %zmm4,%fs:0x180
	.byte 0x64,0x62,0xf1,0xfe,0x48,0x7f,0x2c,0x25,0xc0,0x01,0x00,0x00 # vmovdqu64 %zmm5,%fs:0x1c0
	.byte 0x64,0x62,0xf1,0xfe,0x48,0x7f,0x34,0x25,0x00,0x02,0x00,0x00 # vmovdqu64 %zmm6,%fs:0x200
	.byte 0x64,0x62,0xf1,0xfe,0x48,0x7f,0x3c,0x25,0x40,0x02,0x00,0x00 # vmovdqu64 %zmm7,%fs:0x240
	ret
L(no_avx5):
# endif
	movdqa	%xmm0, %fs:RTLD_SAVESPACE_SSE+0*XMM_SIZE
	movdqa	%xmm1, %fs:RTLD_SAVESPACE_SSE+1*XMM_SIZE
	movdqa	%xmm2, %fs:RTLD_SAVESPACE_SSE+2*XMM_SIZE
	movdqa	%xmm3, %fs:RTLD_SAVESPACE_SSE+3*XMM_SIZE
	movdqa	%xmm4, %fs:RTLD_SAVESPACE_SSE+4*XMM_SIZE
	movdqa	%xmm5, %fs:RTLD_SAVESPACE_SSE+5*XMM_SIZE
	movdqa	%xmm6, %fs:RTLD_SAVESPACE_SSE+6*XMM_SIZE
	movdqa	%xmm7, %fs:RTLD_SAVESPACE_SSE+7*XMM_SIZE
	ret
	cfi_endproc
	.size _dl_x86_64_save_sse, .-_dl_x86_64_save_sse


	.globl _dl_x86_64_restore_sse
	.type _dl_x86_64_restore_sse, @function
	.align 16
	cfi_startproc
_dl_x86_64_restore_sse:
# ifdef HAVE_AVX_SUPPORT
	cmpl	$0, L(have_avx)(%rip)
	js	L(no_avx6)
	cmpl	$0xe6, L(have_avx)(%rip)
	je	L(avx512_6)

	vmovdqa %fs:RTLD_SAVESPACE_SSE+0*YMM_SIZE, %ymm0
	vmovdqa %fs:RTLD_SAVESPACE_SSE+1*YMM_SIZE, %ymm1
	vmovdqa %fs:RTLD_SAVESPACE_SSE+2*YMM_SIZE, %ymm2
	vmovdqa %fs:RTLD_SAVESPACE_SSE+3*YMM_SIZE, %ymm3
	vmovdqa %fs:RTLD_SAVESPACE_SSE+4*YMM_SIZE, %ymm4
	vmovdqa %fs:RTLD_SAVESPACE_SSE+5*YMM_SIZE, %ymm5
	vmovdqa %fs:RTLD_SAVESPACE_SSE+6*YMM_SIZE, %ymm6
	vmovdqa %fs:RTLD_SAVESPACE_SSE+7*YMM_SIZE, %ymm7
	ret
L(avx512_6):
# Original instructions:
#	vmovdqu64 %fs:RTLD_SAVESPACE_SSE+0*ZMM_SIZE, %zmm0
#	vmovdqu64 %fs:RTLD_SAVESPACE_SSE+1*ZMM_SIZE, %zmm1
#	vmovdqu64 %fs:RTLD_SAVESPACE_SSE+2*ZMM_SIZE, %zmm2
#	vmovdqu64 %fs:RTLD_SAVESPACE_SSE+3*ZMM_SIZE, %zmm3
#	vmovdqu64 %fs:RTLD_SAVESPACE_SSE+4*ZMM_SIZE, %zmm4
#	vmovdqu64 %fs:RTLD_SAVESPACE_SSE+5*ZMM_SIZE, %zmm5
#	vmovdqu64 %fs:RTLD_SAVESPACE_SSE+6*ZMM_SIZE, %zmm6
#	vmovdqu64 %fs:RTLD_SAVESPACE_SSE+7*ZMM_SIZE, %zmm7
# Assembled instructions:
	.byte 0x64,0x62,0xf1,0xfe,0x48,0x6f,0x04,0x25,0x80,0x00,0x00,0x00 # vmovdqu64 %fs:0x80,%zmm0
	.byte 0x64,0x62,0xf1,0xfe,0x48,0x6f,0x0c,0x25,0xc0,0x00,0x00,0x00 # vmovdqu64 %fs:0xc0,%zmm1
	.byte 0x64,0x62,0xf1,0xfe,0x48,0x6f,0x14,0x25,0x00,0x01,0x00,0x00 # vmovdqu64 %fs:0x100,%zmm2
	.byte 0x64,0x62,0xf1,0xfe,0x48,0x6f,0x1c,0x25,0x40,0x01,0x00,0x00 # vmovdqu64 %fs:0x140,%zmm3
	.byte 0x64,0x62,0xf1,0xfe,0x48,0x6f,0x24,0x25,0x80,0x01,0x00,0x00 # vmovdqu64 %fs:0x180,%zmm4
	.byte 0x64,0x62,0xf1,0xfe,0x48,0x6f,0x2c,0x25,0xc0,0x01,0x00,0x00 # vmovdqu64 %fs:0x1c0,%zmm5
	.byte 0x64,0x62,0xf1,0xfe,0x48,0x6f,0x34,0x25,0x00,0x02,0x00,0x00 # vmovdqu64 %fs:0x200,%zmm6
	.byte 0x64,0x62,0xf1,0xfe,0x48,0x6f,0x3c,0x25,0x40,0x02,0x00,0x00 # vmovdqu64 %fs:0x240,%zmm7
	ret
L(no_avx6):
# endif
	movdqa	%fs:RTLD_SAVESPACE_SSE+0*XMM_SIZE, %xmm0
	movdqa	%fs:RTLD_SAVESPACE_SSE+1*XMM_SIZE, %xmm1
	movdqa	%fs:RTLD_SAVESPACE_SSE+2*XMM_SIZE, %xmm2
	movdqa	%fs:RTLD_SAVESPACE_SSE+3*XMM_SIZE, %xmm3
	movdqa	%fs:RTLD_SAVESPACE_SSE+4*XMM_SIZE, %xmm4
	movdqa	%fs:RTLD_SAVESPACE_SSE+5*XMM_SIZE, %xmm5
	movdqa	%fs:RTLD_SAVESPACE_SSE+6*XMM_SIZE, %xmm6
	movdqa	%fs:RTLD_SAVESPACE_SSE+7*XMM_SIZE, %xmm7
	ret
	cfi_endproc
	.size _dl_x86_64_restore_sse, .-_dl_x86_64_restore_sse
#endif
