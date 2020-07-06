	.globl   mips_entry

	.section .text.asm.mips_entry, "ax"
	.ent     mips_entry
	.set     noreorder
mips_entry:
	la       $29, _stack_start

	//clean up bss and stack
	la       $28, __stop_bl_cmd_tab
_zeroing_bss_n_stack:
	sw       $0, 0($28)
	bne      $28, $29, _zeroing_bss_n_stack
	addi     $28, 4

	j        main
	nop
	.end     mips_entry
