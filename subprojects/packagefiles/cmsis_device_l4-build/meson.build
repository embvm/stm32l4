project('STM32CubeL4 HAL',
	'c',
	version: '1.7.0'
)

# TODO: need to do something with CMSIS/Device/ST/STM32L4xx/Source/Templates/*.c and Templates/gcc...
#linker script: STM32L4R5ZITxP_FLASH.ld

#############
# STM CMSIS #
#############

stm32l4_cmsis_device_dep = declare_dependency(
	include_directories: include_directories(
		'Include',
		is_system: true
	)
)
