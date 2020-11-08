# Development Log 0001: STM32L4 Processor Bring-up
 
The primary goal of this repository is to create a processor implementation layer for the Embedded VM abstractions, as well as to supply any necessary processor libraries and boot code required by our program. We will also provide example hardware platform files for STM32L4R5 Nucleo development kit.

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

