	.arch armv8-a
	.file	"test.c"
	.text
	.section	.rodata
	.align	3
.LC0:
	.string	"Buffer address = 0x%p"
	.text
	.align	2
	.global	main
	.type	main, %function
main:
.LFB6:
	.cfi_startproc
	stp	x29, x30, [sp, -32]!
	.cfi_def_cfa_offset 32
	.cfi_offset 29, -32
	.cfi_offset 30, -24
	mov	x29, sp
	mov	x0, 16384
	movk	x0, 0x6, lsl 16
	bl	malloc
	str	x0, [sp, 24]
	mov	x2, 16384
	movk	x2, 0x6, lsl 16
	mov	w1, 0
	ldr	x0, [sp, 24]
	bl	memset
	ldr	x1, [sp, 24]
	adrp	x0, .LC0
	add	x0, x0, :lo12:.LC0
	bl	printf
	ldr	x0, [sp, 24]
	bl	free
	mov	w0, 0
	ldp	x29, x30, [sp], 32
	.cfi_restore 30
	.cfi_restore 29
	.cfi_def_cfa_offset 0
	ret
	.cfi_endproc
.LFE6:
	.size	main, .-main
	.ident	"GCC: (Ubuntu 11.3.0-1ubuntu1~22.04) 11.3.0"
	.section	.note.GNU-stack,"",@progbits
