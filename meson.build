project('PROJECT_NAME',
	['c', 'cpp'],
	default_options : [
		'warning_level=3',
		'werror=false',
		# This project defaults to a release build
		'debug=false',
		'optimization=2',
		# `build.*` options affect `native: true targets`
		# plain options affect `native: false` targets.
		'c_std=c11', 'build.c_std=c11',
		'cpp_std=c++17', 'build.cpp_std=c++17',
	],
	meson_version: '>=0.56.0',
	version: '0.1'
)

################################
# Project Options and Settings #
################################

# Pick up our common compiler variables + desired_*_flags variables
subdir('meson/compiler')
subdir('meson/compiler/c')
subdir('meson/compiler/cpp')

if get_option('debug')
	add_project_arguments('-DDEBUG',
		language: ['c', 'cpp'])
	add_project_arguments('-DDEBUG',
		language: ['c', 'cpp'], native: true)
endif

if get_option('disable-builtins')
	desired_common_compile_flags += '-fno-builtin'
endif

if get_option('disable-stack-protection')
	desired_common_compile_flags += '-fno-stack-protector'
endif

if get_option('disable-rtti')
	desired_cpp_compile_flags += '-fno-rtti'
	desired_native_cpp_compile_flags += '-fno-rtti'
endif

if get_option('disable-exceptions')
	desired_cpp_compile_flags += ['-fno-exceptions', '-fno-unwind-tables']
	desired_native_cpp_compile_flags += ['-fno-exceptions', '-fno-unwind-tables']
endif

if get_option('enable-pedantic')
	desired_common_compile_flags += '-pedantic'
else
	desired_common_compile_flags += '-Wno-pedantic'
endif

if get_option('enable-pedantic-error')
	desired_common_compile_flags += '-pedantic-error'
endif

if get_option('enable-threading') == false
    # This will stop the compiler from emiting __cxa_guard_acquire and __cxa_guard_release
    desired_cpp_compile_flags += ['-fno-threadsafe-statics', '-fno-use-cxa-atexit']
    desired_native_cpp_compile_flags += ['-fno-threadsafe-statics', '-fno-use-cxa-atexit']
endif

# Silence the warning when -nostdinc and -nostdinc++ are specified together
desired_common_compile_flags += ['-Wno-unused-command-line-argument']

compile_settings_list = [
	{'lang': 'c', 'compiler': host_c_compiler, 'flags': desired_c_compile_flags, 'isnative': false},
	{'lang': 'c', 'compiler': native_c_compiler, 'flags': desired_native_c_compile_flags, 'isnative': true},
	{'lang': 'cpp', 'compiler': host_cpp_compiler, 'flags': desired_cpp_compile_flags, 'isnative': false},
	{'lang': 'cpp', 'compiler': native_cpp_compiler, 'flags': desired_native_cpp_compile_flags, 'isnative': true},
]

link_settings_list = [
	{'lang': 'c', 'compiler': host_c_compiler, 'flags': [], 'isnative': false},
	{'lang': 'c', 'compiler': native_c_compiler, 'flags': [], 'isnative': true},
	{'lang': 'cpp', 'compiler': host_cpp_compiler, 'flags': [], 'isnative': false},
	{'lang': 'cpp', 'compiler': native_cpp_compiler, 'flags': [], 'isnative': true},
]

# Process the compilation flags
subdir('meson/compiler/check-and-apply-flags')

#################
# Build Modules #
#################

# Include reusable build modules here via subdir() calls
subdir('meson/linker/linker-script-as-property')
subdir('meson/linker/linker-map')
subdir('meson/test/cmocka')
subdir('meson/objcopy')

###############
# Subprojects #
###############

embvm_core_subproject = subproject('embvm-core')
framework_include_dep = embvm_core_subproject.get_variable('framework_include_dep')
framework_host_include_dep = embvm_core_subproject.get_variable('framework_host_include_dep')
framework_threadless_dep = embvm_core_subproject.get_variable('framework_threadless_dep')

cmsis_device_l4_subproject = subproject('cmsis_device_l4')
stm32l4_cmsis_device_dep = cmsis_device_l4_subproject.get_variable('stm32l4_cmsis_device_dep')

stm32l4_hal_subproject = subproject('stm32l4xx_hal_driver')
stm32_hal_dep = stm32l4_hal_subproject.get_variable('stm32_hal_dep')
stm32_ll_dep = stm32l4_hal_subproject.get_variable('stm32_ll_dep')

arm_architecture_subproject = subproject('arm-architecture')
arm_dep = arm_architecture_subproject.get_variable('arm_dep')
arm_common_utilties_dep = arm_architecture_subproject.get_variable('arm_common_utilties_dep')
cmsis_cortex_m_include = arm_architecture_subproject.get_variable('cmsis_cortex_m_include')

# Configure the project to include `libc` headers by default
#######################
# Process Source Tree #
#######################

# Add files to this variable if you want them analyzed by clang-tidy
clangtidy_files = []
# Add dependencies to this array for testing with Catch2
catch2_tests_dep = []

subdir('src')
#subdir('test')

# Defined after src and test so catch2_dep is fully populated
# when creating the built-in targets
#subdir('meson/test/catch2')

###################
# Tooling Modules #
###################

clangformat_targets = [
	meson.source_root() / 'src'
]

subdir('meson/analysis/clang-tidy')
subdir('meson/analysis/complexity')
subdir('meson/analysis/cppcheck')
subdir('meson/analysis/sloccount')
subdir('meson/analysis/vale')
subdir('meson/format')
subdir('meson/docs/doxygen')

#############
# Packaging #
#############

# These macros allow you to simplify the declaration of includes for common
# include paths. Examples:
#	build_root_include.format('doc'),
#	app_include.format('APP')
build_root_include = meson.build_root() + ':@0@'
app_include = meson.build_root() / 'src/app:@0@'
lib_include = meson.build_root() / 'src/lib:@0@'

host_pkg_files = [
	build_root_include.format('docs'),
	app_include.format('APP')
]

native_pkg_files = [
	build_root_include.format('docs'),
	app_include.format('APP_native')
]

# Invoke the package module
subdir('meson/package')
