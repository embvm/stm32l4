stm32l4r5_processor_default_files = files(
	'internal/stm32l4xx_it.c', # TODO: can this be shared across processors? Or is it just a common name with diff implementations for each board/proc?
	'internal/system_stm32l4xx.c', # TODO: can this be shared across processors? Or is it just a common name with diff implementations for each board/proc?
	'internal/startup_stm32l4r5xx.s',
)
stm32l4r5_processor_files = get_variable('stm32l4r5_processor_files', stm32l4r5_processor_default_files)

stm32l4r5 = static_library('stm32l4r5',
	sources: [
		files('stm32l4r5.cpp'),
		stm32l4r5_processor_files,
	],
	c_args: [
		'-DSTM32L4R5xx',
	],
	cpp_args: [
		'-DSTM32L4R5xx',
	],
	include_directories: [
		include_directories('internal'),
		cmsis_cortex_m_include,
	],
	dependencies: [
	    framework_include_dep,
        framework_host_include_dep,
        stm32l4_cmsis_device_dep,
        arm_dep,
        stm32_common_drivers_dep,
	],
	native: false,
	build_by_default: meson.is_subproject() == false
)

stm32l4r5_processor_dep = declare_dependency(
	include_directories: [
		include_directories('.', is_system: true),
		stm32_common_drivers_include
	],
	dependencies: [

	],
	link_with: stm32l4r5,
)
