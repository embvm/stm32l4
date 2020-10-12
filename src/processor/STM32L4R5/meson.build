stm32l4r5 = static_library('stm32l4r5',
	sources: [
		files('stm32l4r5.cpp'),
	],
	compile_args: [
		'-DSTM32L4R5xx',
	],
	include_directories: [

	],
	dependencies: [
	    framework_include_dep,
        framework_host_include_dep,
	],
	build_by_default: meson.is_subproject() == false
)

stm32l4r5_processor_dep = declare_dependency(
	include_directories: [
		include_directories('.', is_system: true),
	],
	dependencies: [

	],
	link_with: stm32l4r5,
)
