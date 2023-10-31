OUTPUT_ARCH(arm)

SECTIONS {
	.text : {
		KEEP(*(.text))
		KEEP(*(.text.*))

		KEEP(*(.data))
		KEEP(*(.data.*))

		KEEP(*(.rodata))
		KEEP(*(.rodata.*))

		KEEP(*(.bss))
		KEEP(*(.bss.*))

		KEEP(*(COMMON))
		KEEP(*(COMMON.*))
	}
}
