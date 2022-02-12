	.globl   longan_nano_entry

	.section .text.asm.longan_nano_entry, "ax"
longan_nano_entry:
	csrwi    mstatus, 0
	csrwi    mip, 0

	la       sp, _stack_start

	//clean up bss and stack
	la       t0, __stop_bl_cmd_tab
_zeroing_bss_n_stack:
	sw       zero, 0(t0)
	addi     t0, t0, 4
	bne      t0, sp, _zeroing_bss_n_stack

	j        main
	nop
	.end     longan_nano_entry
