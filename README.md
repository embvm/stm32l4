# Embedded Virtual Machine STM32L4 Support

This repository provides Embedded VM ecosystem support for SMT32L4 processors.



The [source code for this project](src) is released under the MIT license. Note that the [`embvm-core`](https://github.com/embvm/embvm-core) project itself is dual-licensed as GPLv3 with a commercial licensing option. Please note that using this module as-is will pull in the `embvm-core` project for compiling the end application, requiring you to abide by the GPLv3 or commercial licensing terms.

## Table of Contents

1. [About the Project](#about-the-project)
2. [Project Status](#project-status)
3. [Getting Started](#getting-started)
    1. [Requirements](#requirements)
        1. [git-lfs](#git-lfs)
        1. [Meson Build System](#meson-build-system)
    2. [Getting the Source](#getting-the-source)
    3. [Building](#building)
    4. [Testing](#testing)
4. [Configuration Options](#configuration-options)
5. [Documentation](#documentation)
6. [Need Help?](#need-help)
7. [Contributing](#contributing)
8. [Further Reading](#further-reading)
9. [Authors](#authors)
10. [License](#license)

# About the Project

This repository provides support for STM32L4 processor family; currently targeted at support for STM32L4R5xx processors, with support expanding in the near future.

Currently supported drivers:
+ GPIO
+ DMA
+ Timer (in Timer Base mode)
+ I2C Master (DMA + Interrupts only)

This project also provides a minimal hardware platform, platform, and blinky application that work on an STM32L4R5 Nucleo board.

> **Note:** This implementation wraps the STM32 LL HAL library with Embedded VM compatible interfaces.

**[Back to top](#table-of-contents)**

# Project Status

This module is a work-in-progress. Initial support for the STM32L4R5 processor family is enabled. Only a minimal number of drivers are supported. We will be expanding driver and processor support over the coming months.

Please file an issue if you need particular support, and we will get to it ASAP.

**[Back to top](#table-of-contents)**

## Getting Started

If you are unfamiliar with the Embedded VM project, please see the [`embvm-core`](https://github.com/embvm/embvm-core/) repository for more information and a [Getting Started guide](https://github.com/embvm/embvm-core/tree/master/docs/UsingTheFramework/getting_started.m).

### Requirements

This project uses [Embedded Artistry's standard Meson build system](https://embeddedartistry.com/fieldatlas/embedded-artistrys-standardized-meson-build-system/), and dependencies are described in detail [on our website](https://embeddedartistry.com/fieldatlas/embedded-artistrys-standardized-meson-build-system/).

At a minimum you will need:

* [`git-lfs`](https://git-lfs.github.com), which is used to store binary files in this repository
* [Meson](#meson-build-system) is the build system
* Some kind of compiler for your target system.
    - This repository has been tested with:
        - gcc-7, gcc-8, gcc-9
        - arm-none-eabi-gcc
        - Apple clang
        - Mainline clang

#### git-lfs

This project stores some files using [`git-lfs`](https://git-lfs.github.com).

To install `git-lfs` on Linux:

```
sudo apt install git-lfs
```

To install `git-lfs` on OS X:

```
brew install git-lfs
```

Additional installation instructions can be found on the [`git-lfs` website](https://git-lfs.github.com).

#### Meson Build System

The [Meson](https://mesonbuild.com) build system depends on `python3` and `ninja-build`.

To install on Linux:

```
sudo apt-get install python3 python3-pip ninja-build
```

To install on OSX:

```
brew install python3 ninja
```

Meson can be installed through `pip3`:

```
pip3 install meson
```

If you want to install Meson globally on Linux, use:

```
sudo -H pip3 install meson
```

**[Back to top](#table-of-contents)**

### Getting the Source

This project uses [`git-lfs`](https://git-lfs.github.com), so please install it before cloning. If you cloned prior to installing `git-lfs`, simply run `git lfs pull` after installation.

This project is hosted on GitHub. You can clone the project directly using this command:

```
git clone --recursive https://github.com/embvm/embvm-core
```

If you don't clone recursively, be sure to run the following command in the repository or your build will fail:

```
git submodule update --init --recursive
```

**[Back to top](#table-of-contents)**

### Building

If Make is installed, the project can be built by issuing the following command:

```
make
```

This will build all targets for your current architecture.

You can clean builds using:

```
make clean
```

You can eliminate the generated `buildresults` folder using:

```
make distclean
```

You can also use  `meson` directly for compiling.

Create a build output folder:

```
meson buildresults
```

And build all targets by running

```
ninja -C buildresults
```

Cross-compilation is handled using `meson` cross files. Example files are included in the [`build/cross`](build/cross/) folder. You can write your own cross files for your specific processor by defining the toolchain, compilation flags, and linker flags. These settings will be used to compile the project.

Cross-compilation must be configured using the meson command when creating the build output folder. For files stored within `build/cross`, we provide a Makefile `CROSS` to simplify the process. This variable will automatically supply the proper Meson argument, `build/cross/` prefix, and `.txt` filename extension.

You can use a single file, or you can layer multiple files by separating the names with a colon.

```
make CROSS=arm:cortex-m4_hardfloat
```

You can also do this manually with the Meson interface. Note, however, that you will need to include a special `--cross-file=build/cross/embvm.txt` cross file to ensure that the required Embedded VM settings are applied.

```
meson buildresults --cross-file build/cross/arm.txt --cross-file build/cross/cortex-m4_hardfloat.txt --cross-file=build/cross/embvm.txt
```

Following that, you can run `make` (at the project root) or `ninja -C buildresults` to build the project.

> **Note:** Tests will not be cross-compiled. They will only be built for the native platform.

**Full instructions for working with the build system, including topics like using alternate toolchains and running supporting tooling, are documented in [Embedded Artistry's Standardized Meson Build System](https://embeddedartistry.com/fieldatlas/embedded-artistrys-standardized-meson-build-system/) on our website.**

**[Back to top](#table-of-contents)**

## Configuration Options

Build configuration is covered in depth in the [Configuration and Tuning](https://github.com/embvm/embvm-core/tree/master/docs/UsingTheFramework/configuration_and_tuning.md) guide in the [`embvm-core`](https://github.com/embvm/embvm-core) project.

**[Back to top](#table-of-contents)**

## Documentation

The bulk of the Embedded VM documentation is contained in the [`embvm-core` repository](https://github.com/embvm/embvm-core). This repository only contains documentation relevant to this specific module.

### Building the Documentation

Full documentation can be built locally by running the following command:

```
make docs
```

Documentation can be found in `buildresults/docs`, and the root page is `index.html`.

**[Back to top](#table-of-contents)**

## Need help?

If you need further assistance or have any questions, please file a GitHub issue or send us an email using the [Embedded Artistry Contact Form](http://embeddedartistry.com/contact).

You can also [reach out on Twitter: mbeddedartistry](https://twitter.com/mbeddedartistry/).

## Contributing

If you are interested in contributing to this project, please read our [contributing guidelines](docs/CONTRIBUTING.md).

## Authors

* **[Phillip Johnston](https://github.com/phillipjohnston)**

## License

Copyright © 2020 Embedded Artistry LLC

The [source code for this project](src) is released under the MIT license. Note that the [`embvm-core`](https://github.com/embvm/embvm-core) project itself is dual-licensed as GPLv3 with a commercial licensing option. Please note that using this module as-is will pull in the `embvm-core` project for compiling the end application, requiring you to abide by the GPLv3 or commercial licensing terms.

See the [LICENSE.md](LICENSE.md) file for further licensing details.

**[Back to top](#table-of-contents)**

