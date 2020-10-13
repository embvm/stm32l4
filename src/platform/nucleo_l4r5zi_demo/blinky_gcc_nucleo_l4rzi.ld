/* Linker script to configure memory regions. */

/* Specify the memory areas */
MEMORY
{
RAM (xrw)      : ORIGIN = 0x20000000, LENGTH = 640K
FLASH (rx)      : ORIGIN = 0x8000000, LENGTH = 2048K
}

INCLUDE "gcc_arm_common.ld"
