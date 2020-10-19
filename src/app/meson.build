blinky_stm32l4r5zi = executable('blinky_stm32l4r5zi',
	files('main.cpp'),
	dependencies: [
		nucleo_l4r5zi_demo_platform_dep,
	],
	install: false,
	link_args: host_map_file.format(meson.current_build_dir() / 'blinky_stm32l4r5zi'),
	build_by_default: meson.is_subproject() == false
)

#############################
# Output Conversion Targets #
#############################

blinky_hex = custom_target('blinky_stm32l4r5zi.hex',
	input: blinky_stm32l4r5zi,
	output: 'blinky_stm32l4r5zi.hex',
	command: host_hex_conversion,
	build_by_default: meson.is_subproject() == false
)

blinky_hex = custom_target('blinky_stm32l4r5zi.bin',
	input: blinky_stm32l4r5zi,
	output: 'blinky_stm32l4r5zi.bin',
	command: host_bin_conversion,
	build_by_default: meson.is_subproject() == false
)

