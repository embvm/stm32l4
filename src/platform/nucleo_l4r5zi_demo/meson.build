nucleo_l4r5zi_demo_platform_inc = include_directories('.')

nucleo_l4r5zi_demo_platform_dep = declare_dependency(
	sources: files('platform.cpp'),
	include_directories: nucleo_l4r5zi_demo_platform_inc,
	dependencies: [
		nucleo_l4r5zi_hw_platform_dep,
		framework_threadless_dep,
	],
	link_args: [
		'-L' + meson.current_source_dir(),
		'-Tblinky_gcc_nucleo_l4rzi.ld',
	],
)
