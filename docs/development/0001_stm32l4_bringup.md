# Development Log 0001: STM32L4 Processor Bring-up
 
The primary goal of this repository is to create a processor implementation layer for the Embedded VM abstractions, as well as to supply any necessary processor libraries and boot code required by our program. We will also provide example hardware platform files for STM32L4R5 Nucleo development kit.

## Sense of Timing

The process documented in the log below took me about 6.5 hours, which includes debugging the `libc` startup issue described below. 

## Starting Point

In the [previous development log](0000_stm32l4_patch_file_creation.md), we created the repository for our new processor module. We also created a wrap file for the `STM32CubeL4` project and the necessary Meson patch files required for using the repository as a subproject.

For this log, we will be working in the `embvm/stm32l4` build files.

We also need to create a wrap file to access the `embvm-core` project:

```
# embvm-core.wrap
[wrap-git]
url = git@github.com:embvm/embvm-core.git
revision = master
clone-recursive = true
```

We will also import the `arm-architecture` project, since this is an ARM processor family.

```
# arm-architecture.wrap

[wrap-git]
url = https://github.com/embvm/arm-architecture.git
revision = master
clone-recursive = true
```

## Creating Structure

I find that having some structure in place helps me work through a problem. I've defined the boundary of the system, and then I can easily fill in the missing pieces.

### Blinky Application

To create this structure, we'll define an example application and create template pieces for the necessary files we'll need inside of this repository. We'll start with the application and define additional build targets to satisfy the dependency requirements.

In `src/app`, We will rename the default `main.c` file to `main.cpp` and add write a simple Embedded VM `blinky` program:

```
#include <platform.hpp>

volatile bool abort_program_ = false;

int main()
{
    printf("Blinky application booted!\n");

    auto& platform = VirtualPlatform::inst();

    printf("Starting blink\n");
    platform.startBlink();

    while(!abort_program_)
    {
        // spin
        // Not implemented yet:
        // embvm::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    return 0;
}
```

We then need to define a corresponding executable target. I will compile `main.cpp` and link against a platform dependency.

```
# src/app/meson,build

blinky_stm32l4r5zi = executable('blinky_stm32l4r5zi',
    files('main.cpp'),
    dependencies: [
        nucleo_l4r5zi_demo_platform_dep,
    ],
    install: false,
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
```

### Platform

The platform dependency referenced above doesn't exist, so we're going to need to create it.

We'll start by creating the `src/platform` directory and a corresponding `meson.build` file within it. Eventually, we'll probably bring up multiple development boards, so we'll put each test platform in its own folder. 

```
# src/platform/meson.build

subdir('nucleo_l4r5zi_demo')
```

We'll make a `nucleo_l4r5zi_demo` platform folder, and put another `meson.build` file inside of it. Again, we'll add our initial platform build structure:

```
# src/platform/nucleo_l4r5zi_demo/meson.build

nucleo_l4r5zi_demo_platform_inc = include_directories('.')

nucleo_l4r5zi_demo_platform_dep = declare_dependency(
    sources: files('platform.cpp'),
    include_directories: nucleo_l4r5zi_demo_platform_inc,
    dependencies: [
        nucleo_l4r5zi_hw_platform_dep,
        framework_threadless_dep
    ],
    link_args: [
        '-L' + meson.current_source_dir(),
        '-Tblinky_gcc_nucleo_l4rzi.ld',
    ],
)
```

We'll create a skeleton platform header file with the standard `platform.hpp` name:

```
/*
 * Copyright © 2020 Embedded Artistry LLC.
 * See LICENSE file for licensing information.
 */

#ifndef NUCLEO_L4RZI_DEMO_PLATFORM_HPP_
#define NUCLEO_L4RZI_DEMO_PLATFORM_HPP_

#include <platform/virtual_platform.hpp>
#include <boot/boot_sequencer.hpp>
#include <NucleoL4RZI_HWPlatform.hpp>
#include <platform_options.hpp>

/// Signal variable to exit the main() loop
/// Declared in main.cpp
extern volatile bool abort_program_;

class NucleoL4RZI_DemoPlatform final
    : public embvm::VirtualPlatformBase<NucleoL4RZI_DemoPlatform,
                                        PlatformDriverRegistry>
{
    using PlatformBase =
        embvm::VirtualPlatformBase<NucleoL4RZI_DemoPlatform,
                                   PlatformDriverRegistry>;

  public:
      // APIs to required by base class
    static void earlyInitHook_() noexcept;
    static void initOS_() noexcept;
    void init_() noexcept;
    void initProcessor_() noexcept;
    void initHWPlatform_() noexcept;

    // Platform APIs
    void startBlink() noexcept;

    // Constructor/destructor
    NucleoL4RZI_DemoPlatform() noexcept : PlatformBase("Nucleo-L4R5ZI Demo Platform") {}
    ~NucleoL4RZI_DemoPlatform() = default;

  private:
    NucleoL4R5ZI_HWPlatform hw_platform_;
};

using VirtualPlatform = NucleoL4RZI_DemoPlatform;
using PlatformBootSequencer = embvm::BootSequencer<embvm::DefaultBootStrategy<VirtualPlatform>>;

#endif // NUCLEO_L4RZI_DEMO_PLATFORM_HPP_
```

We'll also create the corresponding `.cpp` file:

```
#include "platform.hpp"

void NucleoL4RZI_DemoPlatform::earlyInitHook_() noexcept
{
    NucleoL4R5ZI_HWPlatform::earlyInitHook();
}

void NucleoL4RZI_DemoPlatform::initOS_(void (*main_thread_func)()) noexcept
{
}

void NucleoL4RZI_DemoPlatform::initHWPlatform_() noexcept
{
    hw_platform_.init();
}

void NucleoL4RZI_DemoPlatform::initProcessor_() noexcept
{
    hw_platform_.initProcessor();
}

void NucleoL4RZI_DemoPlatform::init_() noexcept
{
}

void NucleoL4RZI_DemoPlatform::startBlink() noexcept
{
    hw_platform_.startBlink();
}
```

We will also create the standard `hw_platform_options.hpp` that will supply our driver registry definition to the hardware platform class.

```
#ifndef NUCLEO_L4R5ZI_DEMO_HW_PLATFORM_OPTIONS_HPP
#define NUCLEO_L4R5ZI_DEMO_HW_PLATFORM_OPTIONS_HPP

#include <driver/driver_registry.hpp>

using PlatformDriverRegistry = embvm::StaticDriverRegistry<8>;

#endif // NUCLEO_L4R5ZI_DEMO_HW_PLATFORM_OPTIONS_HPP
```

We're going to leave the linker script alone for now, but we'll come back to it later. There's no chance of forgetting about it since our build will fail due to a missing linker script.

### Hardware Platform Structure

Now, we need to create the skeleton for our hardware platform. We'll create the `src/hw_platform/` directory and place a `meson.build` file there. Like the platform build structure, our hardware platform definition for the dev board is going to go in its own subdirectory.

```
# src/hw_platform/meson.build

subdir('nucleo_l4r5zi')
```

We'll then create `src/hw_platform/nucleo_l4r5zi` and a `meson.build` file there. This is where we can define the `nucleo_l4r5zi_hw_platform_dep` that is used in our platform build rules.

```
# src/hw_platform/nucleo_l4r5zi/meson.build

nucleo_l4r5zi_hw_platform_dep = declare_dependency(
    include_directories: include_directories('.'),
    sources: files('nucleo_l4r5zi_hw_platform.cpp'),
    dependencies: stm32l4r5_processor_dep
)
```

We'll create `NucleoL4R5ZI_HWPlatform.hpp` to hold our basic hardware platform skeleton:

```
#ifndef NUCLEO_L4R5ZI_HW_PLATFORM_HPP_
#define NUCLEO_L4R5ZI_HW_PLATFORM_HPP_

#include <hw_platform/virtual_hw_platform.hpp>
#include <stm32l4r5.hpp>

class NucleoL4R5ZI_HWPlatform : public embvm::VirtualHwPlatformBase<NucleoL4R5ZI_HWPlatform>
{
    using HWPlatformBase = embvm::VirtualHwPlatformBase<NucleoL4R5ZI_HWPlatform>;

  public:
    /// @brief Default constructor.
    NucleoL4R5ZI_HWPlatform() noexcept : HWPlatformBase("NUCLEO-L4R5ZI Development Board") {}

    /// @brief Default destructor.
    ~NucleoL4R5ZI_HWPlatform() noexcept;

    // Required functions
    static void earlyInitHook_() noexcept;
    void init_() noexcept;
    void initProcessor_() noexcept;
    void soft_reset_() noexcept;
    void hard_reset_() noexcept;

    // Public APIs
    void leds_off() noexcept;
    void startBlink() noexcept;

  private:
    stm32l4r5 processor_;
};

#endif // NUCLEO_L4R5ZI_HW_PLATFORM_HPP_
```

And `NucleoL4RZI_HWPlatform.cpp` to hold our basic implementations:

```
#include "NucleoL4RZI_HWPlatform.hpp"

NucleoL4R5ZI_HWPlatform::~NucleoL4R5ZI_HWPlatform() noexcept {}

void NucleoL4R5ZI_HWPlatform::earlyInitHook_() noexcept {}

void NucleoL4R5ZI_HWPlatform::init_() noexcept {}

void NucleoL4R5ZI_HWPlatform::leds_off() noexcept {}

void NucleoL4R5ZI_HWPlatform::hard_reset_() noexcept
{
    // We cannot perform a hard reset from software, so perform
    // a soft reset instead.
    soft_reset_();
}

void NucleoL4R5ZI_HWPlatform::soft_reset_() noexcept
{
    processor_.reset();
}

void NucleoL4R5ZI_HWPlatform::initProcessor_() noexcept
{
    processor_.init();
}

void NucleoL4R5ZI_HWPlatform::startBlink() noexcept
{
    // TODO
}
```

### Processor Skeleton

Finally, the last piece of the skeleton puzzle. We'll create the folder `src/processor` and put a `meson.build` file in it. This will redirect to a subdirectory, `STM32L4R5`, which holds the build files and rules for the processor. 

```
# src/processor/meson.build

subdir('STM32L4R5')
```

We'll create the `STM32L4R5` folder and put a `meson.build` file in it. This is where we will keep all of our processor-specific files that are used by the Embedded VM implementation. Right now, we're going to start with a simple skeleton for the build, and we'll add to it over time.

```
# src/processor/STM32L4R5/meson.build

stm32l4r5 = static_library('stm32l4r5',
    sources: [
        files('stm32l4r5.cpp'),
    ],
    c_args: [
        '-DSTM32L4R5xx',
    ],
    cpp_args: [
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
```

We'll also create the processor header (`stm32l4r5.hpp`):

```
#ifndef STM32L4R5_PROCESSOR_HPP_
#define STM32L4R5_PROCESSOR_HPP_

#include <cstdint>
#include <processor/virtual_processor.hpp>

class stm32l4r5 : public embvm::VirtualProcessorBase<stm32l4r5>
{
    using ProcessorBase = embvm::VirtualProcessorBase<stm32l4r5>;

  public:
    /// @brief Default constructor.
    stm32l4r5() noexcept : ProcessorBase("nrF52840") {}

    /// @brief Default destructor.
    ~stm32l4r5();

    // Required Functions:

    static void earlyInitHook_() noexcept;
    void init_() noexcept;
    void reset_() noexcept;

#pragma mark - Custom Functions -
};

#endif // STM32L4R5_PROCESSOR_HPP_
```

And the corresponding `.cpp` file:

```
#include "stm32l4r5.hpp"

#pragma mark - Definitions -

#pragma mark - Helpers -

#pragma mark - Interface Functions -

stm32l4r5::~stm32l4r5() {}

void stm32l4r5::earlyInitHook_() noexcept {}

void stm32l4r5::init_() noexcept {}

void stm32l4r5::reset_() noexcept
{
}
```

### Completing the Build Setup

Now, we don't need the `lib` or `utility` folders created by the original skeleton we used, so we are going to delete them. We will also need to remove them from `src/meson.build` and update the folders that are parsed:

```
subdir('processor')
subdir('hw_platform')
subdir('platform')
subdir('app')
```

In the top-level `meson.build` file, we need to set up the necessary framework dependencies and include directories.

```
embvm_core_subproject = subproject('embvm-core')
framework_include_dep = embvm_core_subproject.get_variable('framework_include_dep')
framework_host_include_dep = embvm_core_subproject.get_variable('framework_host_include_dep')
framework_threadless_dep = embvm_core_subproject.get_variable('framework_threadless_dep')
```

Now we can run the build on our development machines, and we can see that the skeleton build files compile. We fail to link the application on OS X with Clang, because `-T` isn't a valid argument:

```
[5/5] Linking target src/app/blinky_stm32l4r5zi
FAILED: src/app/blinky_stm32l4r5zi
c++  -o src/app/blinky_stm32l4r5zi src/app/blinky_stm32l4r5zi.p/main.cpp.o src/app/blinky_stm32l4r5zi.p/.._platform_nucleo_l4r5zi_demo_platform.cpp.o src/app/blinky_stm32l4r5zi.p/.._hw_platform_nucleo_l4r5zi_NucleoL4RZI_HWPlatform.cpp.o src/app/blinky_stm32l4r5zi.p/.._.._subprojects_embvm-core_src_core_driver_driver_base.cpp.o src/app/blinky_stm32l4r5zi.p/.._.._subprojects_embvm-core_src_core_boot_boot.cpp.o -Wl,-dead_strip_dylibs -Wl,-headerpad_max_install_names -Wl,-undefined,error -Wl,-dead_strip src/processor/STM32L4R5/libstm32l4r5.a subprojects/embvm-core/src/utilities/libutil.a subprojects/embvm-core/src/core/libcore_threadless.a subprojects/libcpp/libc++.a subprojects/libcpp/libc++experimental.a subprojects/libcpp/libc++abi.a subprojects/libmemory/src/libmemory_hosted.a subprojects/libc/src/libc_hosted.a -Tblinky_gcc_nucleo_l4rzi.ld -L/Users/phillip/src/ea/framework/stm32l4/src/platform/nucleo_l4r5zi_demo -nostdlib -lSystem -nodefaultlibs -Wl,-U,___cxa_thread_atexit_impl -nodefaultlibs -Wl,-U,___cxa_thread_atexit_impl -nodefaultlibs -Wl,-U,___cxa_thread_atexit_impl
ld: unknown option: -T
clang: error: linker command failed with exit code 1 (use -v to see invocation)
ninja: build stopped: subcommand failed.
make: *** [default] Error 1
```

If we try with the ARM GCC compiler, we will get a failure because our linker script doesn't exist (yet):

```
FAILED: src/app/blinky_stm32l4r5zi
arm-none-eabi-c++  -o src/app/blinky_stm32l4r5zi src/app/blinky_stm32l4r5zi.p/main.cpp.o src/app/blinky_stm32l4r5zi.p/.._platform_nucleo_l4r5zi_demo_platform.cpp.o src/app/blinky_stm32l4r5zi.p/.._hw_platform_nucleo_l4r5zi_NucleoL4RZI_HWPlatform.cpp.o src/app/blinky_stm32l4r5zi.p/.._.._subprojects_embvm-core_src_core_driver_driver_base.cpp.o src/app/blinky_stm32l4r5zi.p/.._.._subprojects_embvm-core_src_core_boot_boot.cpp.o -Wl,--as-needed -Wl,--no-undefined -Wl,--gc-sections -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mabi=aapcs -mthumb -Wl,--start-group src/processor/STM32L4R5/libstm32l4r5.a subprojects/embvm-core/src/utilities/libutil.a subprojects/embvm-core/src/core/libcore_threadless.a subprojects/libcpp/libc++.a subprojects/libcpp/libc++experimental.a subprojects/libcpp/libc++abi.a subprojects/libmemory/src/libmemory_hosted.a subprojects/libc/src/libc.a -Wl,--end-group -Tblinky_gcc_nucleo_l4rzi.ld -L/Users/phillip/src/ea/framework/stm32l4/src/platform/nucleo_l4r5zi_demo -nodefaultlibs -nodefaultlibs -nodefaultlibs
/usr/local/toolchains/gcc-arm-none-eabi/bin/../lib/gcc/arm-none-eabi/9.2.1/../../../../arm-none-eabi/bin/ld: cannot open linker script file blinky_gcc_nucleo_l4rzi.ld: No such file or directory
collect2: error: ld returned 1 exit status
```

That was all the boilerplate, now it's time for the real setup!

## Processor Bring-up

Let's start by expanding our processor build rules using the dependencies we've already created in the previous development log.

First though, a refactoring of our previous approach.

### Updating the Build

We have a common ARM architecture subproject that can be used to provide the CMSIS rules, as well as other framework ARM concepts and implementations. So we need to split up this dependency in the STM32CubeL4 patch build files:

```
stm32_cmsis_dep = declare_dependency(
    include_directories: include_directories(
        'Drivers/CMSIS/Include',
        'Drivers/CMSIS/Device/ST/STM32L4xx/Include',
        is_system: true
    )
)
```

Into two separate dependencies:

```
stm32_cmsis_dep = declare_dependency(
    include_directories: include_directories(
        'Drivers/CMSIS/Include',
        is_system: true
    )
)

stm32_cmsis_device_dep = declare_dependency(
    include_directories: include_directories(
        'Drivers/CMSIS/Device/ST/STM32L4xx/Include',
        is_system: true
    )
)
```

Now we can include the dependencies in our main meson.build file. For now I'm going to include both the HAL and LL drivers, since I'm not sure which route I want to take for our implementation.

```
stm32cube_subproject = subproject('STM32CubeL4')
stm32_cmsis_device_dep = stm32cube_subproject.get_variable('stm32_cmsis_device_dep')
stm32_hal_dep = stm32cube_subproject.get_variable('stm32_hal_dep')
stm32_ll_dep = stm32cube_subproject.get_variable('stm32_ll_dep')
```

Pulling in the arm-architecture dependencies:

```
arm_architecture_subproject = subproject('arm-architecture')
arm_dep = arm_architecture_subproject.get_variable('arm_dep')
arm_common_utilties_dep = arm_architecture_subproject.get_variable('arm_common_utilties_dep')
cmsis_cortex_m_include = arm_architecture_subproject.get_variable('cmsis_cortex_m_include')
```

[Development note: Up to this point in the project took me about 3 hours, including setting up the initial STM32L4 build system]

### Processor Bring-up

We know we're going to need more files for our program on the processor side:

```
stm32l4xx_it.c \
stm32l4xx_hal_msp.c \
system_stm32l4xx.c
startup_stm32l4r5xx.s
```

These files are typically tuned per-system and are generated by CubeMX. Templates are provided inside of the STM32CubeL4 repository that we can use. We'll tweak some aspects for the Embedded VM and provide defaults. We'll also need a way for users to override the settings (eventually).

We can find template files in the STM32CubeL4 projects here: `STM32CubeL4/Projects/NUCLEO-L4R5ZI/Templates`. These have example source files in `Src/`, as well as startup code and the linker script in the `SW4STM32` folder. There's a header we need in `Inc/`: `stm32l4xx_it.h`. We will copy these files into the `src/processor/STM32L4R5` folder, and put them in a subdirectory called `internal/`.

```
    stm32l4xx_it.c
    system_stm32l4xx.c
    startup_stm32l4r5xx.s
    stm32l4xx_it.h
```

Note that the `stm32l4xx_hal_msp.c` file is for STM32 HAL overrides for weak functions; these files are blank in the templates and we can override any HAL functions in our processor source files instead. Looking at the `Template` vs `Template_LL` directories, `system_stym32l4xx.c` and `startup_stm32l4r5xx.s` are the same. It doesn't matter which we pick.

We should also copy the linker script for now: `STM32L4R5ZITx_FLASH.ld`, but we're going to handle tuning this file at a later point.

We'll add the files to the library build by referencing this new variable:

```
stm32l4r5_processor_files = files(
    'internal/stm32l4xx_it.c', # TODO: can this be shared across processors? Or is it just a common name with diff implementations for each board/proc?
    'internal/system_stm32l4xx.c', # TODO: can this be shared across processors? Or is it just a common name with diff implementations for each board/proc?
    'internal/startup_stm32l4r5xx.s',
)
```

We'll also add the `internal/` subdirectory to the include directories on the library (not the dependency)

```
    include_directories: [
        include_directories('internal')
    ],
```

### Fixing Build Errors

Now we can test out our build, and we'll seen an include failure:

```
../src/processor/STM32L4R5/system_stm32l4xx.c:92:10: fatal error: stm32l4xx.h: No such file or directory
   92 | #include "stm32l4xx.h"
      |          ^~~~~~~~~~~~~
compilation terminated.
[5/10] Compiling C++ object subprojects/libcpp/libc++.a.p/src_c++_chrono.cpp.o
ninja: build stopped: subcommand failed.
```


We need to add in our processor dependencies. In this case, we need to add `stm32_cmsis_device_dep`, which comes from the STM32CubeL4 subproject patch files.

```
    dependencies: [
        framework_include_dep,
        framework_host_include_dep,
        stm32_cmsis_device_dep
    ],
```

Now I get new failures:

```
../src/processor/STM32L4R5/stm32l4xx_it.c:23:10: fatal error: main.h: No such file or directory
   23 | #include "main.h"
```

We don't need this header in our program. We can also remove it from `stm32l4xx_it.h`.

Next error that comes up:

```
In file included from ../subprojects/STM32CubeL4/Drivers/CMSIS/Device/ST/STM32L4xx/Include/stm32l4xx.h:164,
                 from ../src/processor/STM32L4R5/internal/system_stm32l4xx.c:92:
../subprojects/STM32CubeL4/Drivers/CMSIS/Device/ST/STM32L4xx/Include/stm32l4r5xx.h:170:10: fatal error: core_cm4.h: No such file or directory
  170 | #include "core_cm4.h"             /* Cortex-M4 processor and core peripherals */
```

This is because we included the CMSIS device headers, but not the CMSIS core headers. Now, we could use the version in the STM directory, but we have a common ARM architecture module that brings in CMSIS and some other helpful drivers and Embedded VM functionality. So we're going to use that instead.

We're going to add `arm_dep` to the library dependencies, as well as the `cmsis_include`:

```
    include_directories: [
        include_directories('internal'),
        cmsis_cortex_m_include,
    ],
    dependencies: [
        framework_include_dep,
        framework_host_include_dep,
        stm32_cmsis_device_dep,
        arm_dep,
    ],
```

This dependency requires a new header file: ``<processor_includes.hpp>``. We'll put this in the `internal/ directory.

```
#ifndef STM32L4R5_PROCESSOR_INCLUDES_HPP_
#define STM32L4R5_PROCESSOR_INCLUDES_HPP_

#include "stm32l4xx.h"

#endif // STM32L4R5_PROCESSOR_INCLUDES_HPP_
```

Now we're back to a failing build, but it's because of the linker script again:

```
/usr/local/toolchains/gcc-arm-none-eabi/bin/../lib/gcc/arm-none-eabi/9.2.1/../../../../arm-none-eabi/bin/ld: cannot open linker script file blinky_gcc_nucleo_l4rzi.ld: No such file or directory
```

### Architecture Integration

Before we switch tasks, I can implement another required header: `processor_architecture.hpp`.

```
#ifndef STM32L4R5_PROCESSOR_ARCH_DEF_HPP_
#define STM32L4R5_PROCESSOR_ARCH_DEF_HPP_

#include <arm_cortex_m.hpp>

using ProcessorArch = ARMCortexMArch;

#endif // STM32L4R5_PROCESSOR_ARCH_DEF_HPP_
```

Now we can hook up the `reset_()` function in `stm32l4r5.cpp`

```
#include "stm32l4r5.hpp"
#include <processor_architecture.hpp>
#include <processor_includes.hpp>

#pragma mark - Definitions -

#pragma mark - Helpers -

#pragma mark - Interface Functions -

stm32l4r5::~stm32l4r5() {}

void stm32l4r5::earlyInitHook_() noexcept {}

void stm32l4r5::init_() noexcept {}

void stm32l4r5::reset_() noexcept
{
    ProcessorArch::systemReset();
}
```

## Linking

Now, the ARM architecture has a common linker script that can be used for ARM projects: `gcc_arm_common.ld`.

Looking at the STM32 linker script and the common linker script, I don't see anything that is necessary in their version of the file. Everything is pretty standard. Entry points are the same too.

We just need to note these values for our processor:

```
/* Highest address of the user mode stack */
_estack = 0x200A0000;    /* end of RAM */ ----- this i s used by the STM32 startup files! (does nordic also have it

/* Specify the memory areas */
MEMORY
{
RAM (xrw)      : ORIGIN = 0x20000000, LENGTH = 640K
FLASH (rx)      : ORIGIN = 0x8000000, LENGTH = 2048K
}
```

We can delete `STM32L4R5ZITx_FLASH.ld` from our project. In the `platform/nucleo_l4r5zi_demo` folder, we'll create the missing `blinky_gcc_nucleo_l4rzi.ld` file. We'll past our saved memory values above, and then include the common linker script.

```
/* Linker script to configure memory regions. */

/* Highest address of the user mode stack */
_estack = 0x200A0000;    /* end of RAM */ ----- this i s used by the STM32 startup files! (does nordic also have it

/* Specify the memory areas */
MEMORY
{
RAM (xrw)      : ORIGIN = 0x20000000, LENGTH = 640K
FLASH (rx)      : ORIGIN = 0x8000000, LENGTH = 2048K
}

INCLUDE "gcc_arm_common.ld"
```

Now we can run our build, and we get new failures!

```
(.text.Reset_Handler+0x3c): undefined reference to `_sidata'
/usr/local/toolchains/gcc-arm-none-eabi/bin/../lib/gcc/arm-none-eabi/9.2.1/../../../../arm-none-eabi/bin/ld: (.text.Reset_Handler+0x40): undefined reference to `_sdata'
/usr/local/toolchains/gcc-arm-none-eabi/bin/../lib/gcc/arm-none-eabi/9.2.1/../../../../arm-none-eabi/bin/ld: (.text.Reset_Handler+0x44): undefined reference to `_edata'
/usr/local/toolchains/gcc-arm-none-eabi/bin/../lib/gcc/arm-none-eabi/9.2.1/../../../../arm-none-eabi/bin/ld: (.text.Reset_Handler+0x48): undefined reference to `_sbss'
/usr/local/toolchains/gcc-arm-none-eabi/bin/../lib/gcc/arm-none-eabi/9.2.1/../../../../arm-none-eabi/bin/ld: (.text.Reset_Handler+0x4c): undefined reference to `_ebss'
```

### Updating Initialization Code

These missing symbols are used by the initialization code and were previously defined by the STM32 linker script.

Our common linker script already defines these values, which have a different name but serve the same purpose:

```
 *   __StackLimit
 *   __StackTop
 *   __stack
 *   __data_start__
 *  etc...
```

For example, We can change the initialization code to refer to __StackLimit instead of _estack.

## Tuning the Startup File

This is actually the hard part of integrating a new processor. That's because there's no easy solution. You have to look at the assembly and make sure it's doing things correctly.

The primary objective is to perform any a) basic initialization that is required before b) kicking off the libc initialization process (in `crt0.S` defined by the architecture), which will perform stuff like setting up stacks and clearing BSS, and then it will run the framework boot strategy defined by the platform.

So for the STM32 startup file, the first thing we need to do is replace the `bl main` with `bl _start`, ensuring we jump to the proper `libc` entry point.

```
/* Call the libc entry point.*/
    bl    _start
```

Now, let's just work backwards. the following code above the `bl _start` call is equivalent to what's happening in `libc`, so we can just remove it:

```
/* Zero fill the bss segment. */
FillZerobss:
    movs    r3, #0
    str    r3, [r2], #4

LoopFillZerobss:
    ldr    r3, = _ebss
    cmp    r2, r3
    bcc    FillZerobss

/* Call static constructors */
    bl __libc_init_array
```

Next we need to adjust the "data copy loop" to use our common linker script symbols instead of the STM32 ones. We'll use this form intead of the STM32 approach:

```
/* Loop to copy data from read only memory to RAM.
 * The ranges of copy from/to are specified by following symbols:
 *      __etext: LMA of start of the section to copy from. Usually end of text
 *      __data_start__: VMA of start of the section to copy to.
 *      __bss_start__: VMA of end of the section to copy to. Normally __data_end__ is used, but by using __bss_start__
 *                    the user can add their own initialized data section before BSS section with the INSERT AFTER command.
 *
 * All addresses must be aligned to 4 bytes boundary.
 */
#ifndef __STARTUP_SKIP_ETEXT
    ldr r1, =__etext
    ldr r2, =__data_start__
    ldr r3, =__bss_start__

    subs r3, r3, r2
    ble .L_loop1_done

.L_loop1:
    subs r3, r3, #4
    ldr r0, [r1,r3]
    str r0, [r2,r3]
    bgt .L_loop1

.L_loop1_done:
#endif
```

The symbols used above come from our linker script. We can delete these:

```
/* start address for the initialization values of the .data section.
defined in linker script */
.word    _sidata
/* start address for the .data section. defined in linker script */
.word    _sdata
/* end address for the .data section. defined in linker script */
.word    _edata
/* start address for the .bss section. defined in linker script */
.word    _sbss
/* end address for the .bss section. defined in linker script */
.word    _ebss
```

Finally, we'll change this value: 

```
ldr   sp, =_estack    /* Set stack pointer */
```

to 

```
ldr sp, =__StackTop
```

and then change 

```
.word _estack
```

to 

```
.word __StackTop
```

### putchar

We also need a `_putchar` reference. Let's add that to the platform:

```
#include <printf.h> // for _putchar definition

void _putchar(char c)
{
    // TODO:
}
```

Now the builds compiles and links successfully!!!!

## Getting to Blinky

First we need to create a GPIO driver that we can use with our GPIO-LED driver provided by the `embvm-core` project.

We'll make a new `src/drivers` folder, as well as `src/drivers/helpers`. The `drivers/` folder will have our main interfaces, while `helpers/` will have shims and internal details that shouldn't be public.

### GPIO Driver Implementation

> **Note:** The GPIO base class was redesigned after this development log was written, so the exact implementation notes are slightly outdated. However, the process is still valid.

We'll start by including our basic GPIO class:

```
#include <driver/gpio.hpp>
```

I'll also describe a basic GPIO output class that takes a templated port/pin. Because we don't want to leak implementation details, I'm going to forward these calls to translation class that handles the implementation details under the hood.

```
template<uint8_t TPort, uint8_t TPin>
class STM32GPIOOutput final : public embvm::gpio::output
{
  public:
    /** Construct a generic GPIO output
     */
    explicit STM32GPIOOutput() noexcept : embvm::gpio::output("nRF GPIO Output") {}

    /** Construct a named GPIO output
     *
     * @param name The name of the GPIO pin
     */
    explicit STM32GPIOOutput(const char* name) noexcept : embvm::gpio::output(name) {}

    /// Default destructor
    ~STM32GPIOOutput() = default;

    void set(bool v) noexcept final
    {
        if(v)
        {
            STM32GPIOTranslator::set(TPort, TPin);
        }
        else
        {
            STM32GPIOTranslator::clear(TPort, TPin);
        }
    }

  private:
    void start_() noexcept final
    {
        STM32GPIOTranslator::configure_output(TPort, TPin);
    }

    void stop_() noexcept final
    {
        STM32GPIOTranslator::configure_default(TPort, TPin);
    }
};
```

We'll preemptively include this header:

```
#include "helpers/gpio_helper.hpp"
```

Now to make `gpio_helper.hpp` and `gpio_helper.cpp` in `drivers/helpers`.

The header file is going to be a simple interface that defines static functions

```
#ifndef STM32_GPIO_HELPER_HPP_
#define STM32_GPIO_HELPER_HPP_

#include <cstdint>

/** Translation class which handles STM32 GPIO Configuration.
 *
 * This represents a bridge pattern: the implementation of the GPIO functions is separated from the
 * main interfaces (STM32GPIOOutput, STM32GPIOInput, etc.).
 *
 * The GPIO function implementations are isolated from this header because we do not want to make
 * the STM32 headers accessible from the rest of the system.
 *
 * This class cannot be directly instantiated.
 */
class STM32GPIOTranslator
{
  public:
    static void configure_output(uint8_t port, uint8_t pin) noexcept;
    static void configure_input(uint8_t port, uint8_t pin, uint8_t pull_config) noexcept;
    static void configure_default(uint8_t port, uint8_t pin) noexcept;
    static void set(uint8_t port, uint8_t pin) noexcept;
    static void clear(uint8_t port, uint8_t pin) noexcept;

  private:
    /// This class can't be instantiated
    STM32GPIOTranslator() = default;
    ~STM32GPIOTranslator() = default;
};

#endif // STM32_GPIO_HELPER_HPP_
```

The source file will be implementations of these functions.

We'll start with the includes:

```
#include "gpio_helper.h"
#include <processor_includes.hpp>
```

We also need to decide whether we want to use the HAL or LL drivers at this point. Eventually we can supply implementations for both, but we need to pick one fundamentally. For now, we're going to use the LL drivers.

In the `.cpp` file, we need to include `<stm32l4xx_ll_gpio.h>`.

Something comes to mind looking at the STM32 files. Predictably, there are multiple ports supported on this processor: `GPIOA...GPIOI`

Now, we don't want to parameterize our template classes by using pointers, and we don't want to expose any headers or implementation details on the STM32 side to the rest of the program. So what can we do? Declare an `enum` that is mapped to the values internally:

```
enum STM32GPIOPort : uint8_t {
    A = 0,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
};
```

Then we change our template header:
`template<STM32GPIOPort TPort, uint8_t TPin>`

Inside of our` gpio_helper.cpp` file, we can define an array that can map these values to our actual pointer types:

```
constexpr std::array<GPIO_TypeDef*,9> ports = {GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG, GPIOH, GPIOI};
```

We also need to translate our integral pin number [0..15] into the proper STM32 representation, which is a bitmask.

```
#define PIN_INT_TO_STM32(x) (1 << x)
```

Now we can map our helper functions to the LL functions and types:

```
void STM32GPIOTranslator::configure_output(uint8_t port, uint8_t pin) noexcept
{
    LL_GPIO_InitTypeDef gpio_init = {
        .Pin = PIN_INT_TO_STM32(pin),
        .Mode = LL_GPIO_MODE_OUTPUT,
        .Speed = LL_GPIO_SPEED_FREQ_MEDIUM,
        .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
        .Pull = LL_GPIO_PULL_NO,
        .Alternate = LL_GPIO_AF_0
    };

    LL_GPIO_Init([ports[port], &gpio_init); // GPIOx, GPIO_InitStruct
}

void STM32GPIOTranslator::configure_input(uint8_t port, uint8_t pin, uint8_t pull_config) noexcept
{
    LL_GPIO_InitTypeDef gpio_init = {
        .Pin = PIN_INT_TO_STM32(pin),
        .Mode = LL_GPIO_MODE_INPUT,
        .Speed = LL_GPIO_SPEED_FREQ_MEDIUM,
        .Pull = LL_GPIO_PULL_NO,
        .Alternate = LL_GPIO_AF_0,
    };

    LL_GPIO_Init(ports[port], &gpio_init); // GPIOx, GPIO_InitStruct
}

void STM32GPIOTranslator::configure_default(uint8_t port, uint8_t pin) noexcept
{
    configure_input(port, pin, 0); // TODO: set to no-pull
}

void STM32GPIOTranslator::set(uint8_t port, uint8_t pin) noexcept
{
    LL_GPIO_SetOutputPin(ports[port], PIN_INT_TO_STM32(pin)); // GPIOx, PinMask
}

void STM32GPIOTranslator::clear(uint8_t port, uint8_t pin) noexcept
{
    LL_GPIO_ResetOutputPin(ports[port], PIN_INT_TO_STM32(pin)); // GPIOx, PinMask
}
```

Next we need to get this into the build. In `src/drivers/meson.build`:

```
# src/drivers/meson.build

stm32_common_drivers_include = include_directories('.')

stm32_common_drivers_dep = declare_dependency(
    include_directories: stm32_common_drivers_include,
    sources: [
        'helpers/gpio_helper.cpp',
    ],
    dependencies: stm32_ll_dep,
)
```

We'll add the `subdir` call to the `src/meson.build` file. It needs to be included before processor so we can use the driver dependency in the processor build target.

```
subdir('drivers')
subdir('processor')
subdir('hw_platform')
subdir('platform')
subdir('app')
```

In our processor library target, we'll add the driver dependency to the library build and the include directory to the processor dependency:

```
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
        stm32_cmsis_device_dep,
        arm_dep,
        stm32_common_drivers_dep,
    ],
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
```

### Setting up the Hardware Platform

In the hardware platform header file, we can include the `stm32_gpio.hpp` header and declare some GPIO pins:
    
```
    STM32GPIOOutput<STM32GPIOPort::C, 7> led1_pin;
    STM32GPIOOutput<STM32GPIOPort::B, 7> led2_pin;
    STM32GPIOOutput<STM32GPIOPort::B, 14> led3_pin;
```

Next, we'll declare LED drivers that map to our GPIO pins:

```
    embvm::led::gpioActiveHigh led1{led1_pin, "led1"};
    embvm::led::gpioActiveHigh led2{led2_pin, "led2"};
    embvm::led::gpioActiveHigh led3{led3_pin, "led3"};
```

In our `.cpp` file, we need to actually start the drivers and use them in our code.

```
void NucleoL4R5ZI_HWPlatform::init_() noexcept
{
    // start all LEDs
    // turn them off? Or just trust that they start off?
    led1.start();
    led2.start();
    led3.start();
    led4.start();
}

void NucleoL4R5ZI_HWPlatform::leds_off() noexcept {
    led1.off();
    led2.off();
    led3.off();
}

For our start_blink implementation, we'll just turn them on for now so it's an obvious sign

void NucleoL4R5ZI_HWPlatform::startBlink() noexcept
{
    led1.on();
    led2.on();
    led3.on();
}
```

### Pit-stop: Another Build Error

Now when we build the project, there's an annoying error from a missing STM32 header: `stm32l4xx_hal_conf.h`

```
In file included from ../subprojects/STM32CubeL4/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_sdmmc.c:167:
../subprojects/STM32CubeL4/Drivers/STM32L4xx_HAL_Driver/Inc/stm32l4xx_hal.h:30:10: fatal error: stm32l4xx_hal_conf.h: No such file or directory
   30 | #include "stm32l4xx_hal_conf.h"
      |          ^~~~~~~~~~~~~~~~~~~~~~
compilation terminated.
```
    
We're not using the HAL, so why is this even necessary for LL files? For now, we're just going to disable the following files in the LL dependency:

* `*_ll_usb.c`
* `*_ll_sdmmc.c`
* `*_ll_fmc.c`

This gets us building, but now we have an undefined reference to free:

```
/../../arm-none-eabi/bin/ld: subprojects/libcpp/libc++.a(src_c++_new_terminate_badalloc.cpp.o): in function `operator delete(void*)':
new_terminate_badalloc.cpp:(.text._ZdlPv+0x0): undefined reference to `free'
collect2: error: ld returned 1 exit status
ninja: build stopped: subcommand failed.
make: *** [default] Error 1
```

## First Boot Attempt

Well, I flashed the device and the LEDs did not turn on. So now it's time to debug. I fired up a J-Link GDB sever and connected, loaded my file, and then generated a backtrace:

```
(gdb) bt
#0  0x200001ec in __cxa_unexpected_handler ()
#1  0x08000cc6 in __libc_init_array ()
#2  0x08000802 in void embvm::DefaultBootStrategy<NucleoL4RZI_DemoPlatform>()
    ()
#3  0x0800090e in entry ()
#4  0x08002016 in _start ()
```

The `__cxa_unexpected_handler` should not be called as part of the initialization routine, so something is going on with the boot code.

This debugging investigation was unrelated to the STM32 bringup process, but instead [exposed a bug in the program startup code within libc itself](https://embeddedartistry.com/lesson/exploring-a-pointer-math-bug-in-program-startup/). 

After the adjustments (pull in from libc), we can properly boot the system. But the GPIO doesn't come on!

### Turning On Clocks

One thing we forgot to do was turn on the clocks for the GPIO banks we're using!

For the first attempt I just did this, because I haven't yet found the equivalent function in the LL code... ALL THREE LEDS CAME ON!!!!

```
#define __HAL_RCC_GPIOB_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOBEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOBEN); \
                                               } while(0)


#define __HAL_RCC_GPIOC_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOCEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOCEN); \
                                               } while(0)


#pragma mark - Implementations -


void STM32GPIOTranslator::configure_output(uint8_t port, uint8_t pin) noexcept
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();


    LL_GPIO_InitTypeDef gpio_init = {
        .Pin = PIN_INT_TO_STM32(pin),
        .Mode = LL_GPIO_MODE_OUTPUT,
        .Speed = LL_GPIO_SPEED_FREQ_MEDIUM,
        .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
        .Pull = LL_GPIO_PULL_NO,
        .Alternate = LL_GPIO_AF_0
    };


    LL_GPIO_Init(ports[port], &gpio_init); // GPIOx, GPIO_InitStruct
}
```

## Expanding the Implementation

After realizing that I would need to manage the clocks for each peripheral that I'm using, so I build a clock management class (`stm32_rcc.hpp`/`.cpp`). I then went on to implement a timer driver (using the TimerBase approach), DMA support, and an I2C master driver. This process followed similarly to the GPIO driver implementation above, so it is not documented here.
