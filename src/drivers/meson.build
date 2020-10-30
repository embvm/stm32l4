stm32_common_drivers_include = include_directories('.')

stm32_common_drivers_dep = declare_dependency(
	include_directories: stm32_common_drivers_include,
	sources: [
		'helpers/gpio_helper.cpp',
		'stm32_i2c_master.cpp',
		'stm32_rcc.cpp',
		'stm32_timer.cpp',
	],
	dependencies: stm32_ll_dep,
)
