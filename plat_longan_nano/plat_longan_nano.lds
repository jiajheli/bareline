MEMORY {
	ram (rwx) : ORIGIN = 0x20000000, LENGTH = 32K
}
SECTIONS {
	. = ALIGN(4);
	.text :	{
		*(.text*)
	} >ram

	. = ALIGN(4);
	.rodata : {
		*(.rodata*)
	} >ram

	. = ALIGN(4);
	bl_cmd_tab : {
	  __start_bl_cmd_tab = .;
		*(bl_cmd_tab)
		__stop_bl_cmd_tab = ALIGN(4);
	} >ram

	_stack_start = ORIGIN(ram) + LENGTH(ram) - 8;
}
