OUTPUT_ARCH(arm)

SECTIONS {
	.text : {
		*(.text)
		*(.text.*)

		*(.data)
		*(.data.*)

		*(.rodata)
		*(.rodata.*)

		*(.bss)
		*(.bss.*)

		*(COMMON)
		*(COMMON.*)
	}
}
