	.arch armv8-a
	.file	"syscall.c"
	.text
	.align	2
	.global	getpid
	.type	getpid, %function
getpid:
	sub	sp, sp, #16
#APP
// 6 "inc/syscall.c" 1
			svc 1
		mov x0, x0
	
// 0 "" 2
#NO_APP
	str	x0, [sp, 8]
	ldr	x0, [sp, 8]
	add	sp, sp, 16
	ret
	.size	getpid, .-getpid
	.align	2
	.global	uart_read
	.type	uart_read, %function
uart_read:
	sub	sp, sp, #32
	str	x0, [sp, 8]
	str	w1, [sp, 4]
#APP
// 15 "inc/syscall.c" 1
			svc 2
		mov x0, x0
	
// 0 "" 2
#NO_APP
	str	x0, [sp, 24]
	ldr	x0, [sp, 24]
	add	sp, sp, 32
	ret
	.size	uart_read, .-uart_read
	.align	2
	.global	uart_write
	.type	uart_write, %function
uart_write:
	sub	sp, sp, #32
	str	x0, [sp, 8]
	str	w1, [sp, 4]
#APP
// 24 "inc/syscall.c" 1
			svc 3
		mov x0, x0
	
// 0 "" 2
#NO_APP
	str	x0, [sp, 24]
	ldr	x0, [sp, 24]
	add	sp, sp, 32
	ret
	.size	uart_write, .-uart_write
	.align	2
	.global	uart_printf
	.type	uart_printf, %function
uart_printf:
	stp	x29, x30, [sp, -400]!
	add	x29, sp, 0
	str	x0, [x29, 56]
	str	x1, [x29, 344]
	str	x2, [x29, 352]
	str	x3, [x29, 360]
	str	x4, [x29, 368]
	str	x5, [x29, 376]
	str	x6, [x29, 384]
	str	x7, [x29, 392]
	str	q0, [x29, 208]
	str	q1, [x29, 224]
	str	q2, [x29, 240]
	add	x0, x29, 256
	str	q3, [x0]
	add	x0, x29, 272
	str	q4, [x0]
	add	x0, x29, 288
	str	q5, [x0]
	add	x0, x29, 304
	str	q6, [x0]
	add	x0, x29, 320
	str	q7, [x0]
	add	x0, x29, 400
	str	x0, [x29, 72]
	add	x0, x29, 400
	str	x0, [x29, 80]
	add	x0, x29, 336
	str	x0, [x29, 88]
	mov	w0, -56
	str	w0, [x29, 96]
	mov	w0, -128
	str	w0, [x29, 100]
	add	x2, x29, 16
	add	x3, x29, 72
	ldp	x0, x1, [x3]
	stp	x0, x1, [x2]
	ldp	x0, x1, [x3, 16]
	stp	x0, x1, [x2, 16]
	add	x1, x29, 16
	add	x0, x29, 104
	mov	x2, x1
	ldr	x1, [x29, 56]
	bl	vsprintf
	str	w0, [x29, 204]
	ldr	w0, [x29, 204]
	cmp	w0, 1
	bne	.L8
.L9:
	b	.L9
.L8:
	ldr	w1, [x29, 204]
	add	x0, x29, 104
	bl	uart_write
	ldr	w0, [x29, 204]
	ldp	x29, x30, [sp], 400
	ret
	.size	uart_printf, .-uart_printf
	.align	2
	.global	exec
	.type	exec, %function
exec:
	sub	sp, sp, #32
	str	x0, [sp, 8]
	str	x1, [sp]
#APP
// 45 "inc/syscall.c" 1
			svc 4
		mov x0, x0
	
// 0 "" 2
#NO_APP
	str	x0, [sp, 24]
	ldr	x0, [sp, 24]
	add	sp, sp, 32
	ret
	.size	exec, .-exec
	.section	.rodata
	.align	3
.LC0:
	.string	"syscall exit fail\n"
	.text
	.align	2
	.global	exit
	.type	exit, %function
exit:
	stp	x29, x30, [sp, -16]!
	add	x29, sp, 0
#APP
// 53 "inc/syscall.c" 1
	svc 5

// 0 "" 2
#NO_APP
	adrp	x0, .LC0
	add	x0, x0, :lo12:.LC0
	bl	uart_printf
.L14:
	b	.L14
	.size	exit, .-exit
	.align	2
	.global	fork
	.type	fork, %function
fork:
	sub	sp, sp, #16
#APP
// 60 "inc/syscall.c" 1
			svc 6
		mov x0, x0
	
// 0 "" 2
#NO_APP
	str	x0, [sp, 8]
	ldr	x0, [sp, 8]
	add	sp, sp, 16
	ret
	.size	fork, .-fork
	.align	2
	.global	open
	.type	open, %function
open:
	sub	sp, sp, #32
	str	x0, [sp, 8]
	str	w1, [sp, 4]
#APP
// 69 "inc/syscall.c" 1
			svc 7
		mov x0, x0
	
// 0 "" 2
#NO_APP
	str	w0, [sp, 28]
	ldr	w0, [sp, 28]
	add	sp, sp, 32
	ret
	.size	open, .-open
	.align	2
	.global	close
	.type	close, %function
close:
	sub	sp, sp, #16
	str	w0, [sp, 12]
#APP
// 76 "inc/syscall.c" 1
	svc 8
// 0 "" 2
#NO_APP
	mov	w0, 0
	add	sp, sp, 16
	ret
	.size	close, .-close
	.align	2
	.global	write
	.type	write, %function
write:
	sub	sp, sp, #32
	str	w0, [sp, 12]
	str	x1, [sp]
	str	w2, [sp, 8]
#APP
// 82 "inc/syscall.c" 1
			svc 9
		mov x0, x0
	
// 0 "" 2
#NO_APP
	str	w0, [sp, 28]
	ldr	w0, [sp, 28]
	add	sp, sp, 32
	ret
	.size	write, .-write
	.align	2
	.global	read
	.type	read, %function
read:
	sub	sp, sp, #32
	str	w0, [sp, 12]
	str	x1, [sp]
	str	w2, [sp, 8]
#APP
// 91 "inc/syscall.c" 1
			svc 10
		mov x0, x0
	
// 0 "" 2
#NO_APP
	str	w0, [sp, 28]
	ldr	w0, [sp, 28]
	add	sp, sp, 32
	ret
	.size	read, .-read
	.ident	"GCC: (Ubuntu/Linaro 5.4.0-6ubuntu1~16.04.9) 5.4.0 20160609"
	.section	.note.GNU-stack,"",@progbits
