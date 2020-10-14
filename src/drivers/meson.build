stm32_common_drivers_include = include_directories('.')

stm32_common_drivers_dep = declare_dependency(
	include_directories: stm32_common_drivers_include,
	sources: [
		'helpers/gpio_helper.cpp',
	],
	dependencies: stm32_ll_dep,
)
