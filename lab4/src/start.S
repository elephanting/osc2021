.section ".text.boot"

.global _start

_start:
    // read cpu id, stop slave cores
    mrs x1, mpidr_el1
    and x1, x1, #3
    cbz x1, 2f
    // cpu id > 0, stop
1:  
    wfe
    b 1b
2:
    bl from_el2_to_el1

3:
    bl set_exception_vector_table

4:  
    // set stack before our code
    ldr x1, =__stack_top
    mov sp, x1

    // clear bss
    ldr x1, =__bss_start
    ldr x2, =__bss_size

clear_bss:
    cbz x2, 5f

    // post-indexed store
    str xzr, [x1], #8
    sub x2, x2, #1
    cbnz x2, clear_bss

5:
    bl main

6: 
    b 1b


from_el2_to_el1:
    mov x0, (1 << 31) // EL1 uses aarch64
    msr hcr_el2, x0
    mov x0, 0x3c5 // EL1h (SPSel = 1) with interrupt disabled
    msr spsr_el2, x0
    msr elr_el2, x30 // armv8 link register
    eret // return to EL1
    b 3b

set_exception_vector_table:
    ldr x0, =exception_vector_table
    msr vbar_el1, x0
    b 4b

.align 11 // vector table should be aligned to 0x800
.global exception_vector_table
exception_vector_table:
    b NotImplemented // 0x0 (base)
    .align 7 // // entry size is 0x80, .align will pad 0
    b NotImplemented // 0x80
    .align 7
    b NotImplemented // 0x100
    .align 7
    b NotImplemented // 0x180
    .align 7

    b NotImplemented // 0x200
    .align 7
    b NotImplemented // 0x280
    .align 7
    b NotImplemented // 0x300
    .align 7
    b NotImplemented // 0x380
    .align 7

    b Synchronous // 0x400
    .align 7
    b IRQ // 0x480
    .align 7
    b NotImplemented // 0x500
    .align 7
    b NotImplemented // 0x580
    .align 7

    b NotImplemented // 0x600
    .align 7
    b NotImplemented // 0x680
    .align 7
    b NotImplemented // 0x700
    .align 7
    b NotImplemented // 0x780

.macro save_regs
	sub sp, sp, 32 * 8
	stp x0, x1, [sp ,16 * 0]
	stp x2, x3, [sp ,16 * 1]
	stp x4, x5, [sp ,16 * 2]
	stp x6, x7, [sp ,16 * 3]
	stp x8, x9, [sp ,16 * 4]
	stp x10, x11, [sp ,16 * 5]
	stp x12, x13, [sp ,16 * 6]
	stp x14, x15, [sp ,16 * 7]
	stp x16, x17, [sp ,16 * 8]
	stp x18, x19, [sp ,16 * 9]
	stp x20, x21, [sp ,16 * 10]
	stp x22, x23, [sp ,16 * 11]
	stp x24, x25, [sp ,16 * 12]
	stp x26, x27, [sp ,16 * 13]
	stp x28, x29, [sp ,16 * 14]
	str x30, [sp, 16 * 15]
.endm

.macro load_regs
	ldp x0, x1, [sp ,16 * 0]
	ldp x2, x3, [sp ,16 * 1]
	ldp x4, x5, [sp ,16 * 2]
	ldp x6, x7, [sp ,16 * 3]
	ldp x8, x9, [sp ,16 * 4]
	ldp x10, x11, [sp ,16 * 5]
	ldp x12, x13, [sp ,16 * 6]
	ldp x14, x15, [sp ,16 * 7]
	ldp x16, x17, [sp ,16 * 8]
	ldp x18, x19, [sp ,16 * 9]
	ldp x20, x21, [sp ,16 * 10]
	ldp x22, x23, [sp ,16 * 11]
	ldp x24, x25, [sp ,16 * 12]
	ldp x26, x27, [sp ,16 * 13]
	ldp x28, x29, [sp ,16 * 14]
	ldr x30, [sp, 16 * 15]
	add sp, sp, 32 * 8
.endm


NotImplemented:
    save_regs
    bl notImplemented
    load_regs
    eret

Synchronous:
    save_regs
    bl exception_handler
    load_regs
    eret

IRQ:
    save_regs
    bl interrupt_handler
    load_regs
    eret
