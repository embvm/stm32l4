# Development Log 0000: Creating STM32L4 Patch Files

We want to add support for the STM32L4 processor family to the Embedded VM ecosystem, particularly the STM32L4R5xx processors and the associated Nucleo development board. In order to accelerate development, we'll implement our driver API using the STM32 Low-level HAL drivers. Before we can dive into implementation, however, we need to understand how to properly use STM's "Cube" development packages, which contain the HAL, BSP, middleware, and CMSIS code for different processor families.

For our processor, we'll need to use the [STM32CubeL4](https://github.com/STMicroelectronics/STM32CubeL4 ) repository. 

Our primary goal is to assemble an example application using the files in the `STM32CubeL4` repository. To declare success, we want the compiled application to run on the device. Once that's done, we can reorganize the build into a form that's usable as a subproject. 

## Project Familiarization

For bring-up, we're using an STM32L4R5 Nucleo-144 Development board, which features the STM32L4R5ZIT6P processor in particular. Inside of the repository, I can see that there is example code for the NUCLEO-L4R5ZI, as well as the NUCLEO-L4R5ZI-P. Looking at my board, I notice that a sticker has the -P version, so we'll look at that variant first.

I have an  STM32L4R5 Nucleo-144 board, featuring the STM32L4R5ZIT6P processor. Looking at STM32CubeL4/Projects, I can see that there is example code for NUCLEO-L4R5ZI (as well as a -P version). I see a -P on the sticker on my board (NUCLEO--L4R5ZI-P), so we'll focus on the -P variant. 

## Project Setup

First, we're going to create a new repository to contain this processor family. Let's call it `stm32l4`. I set up this repository using the `init_repo` macro described in the [Automating the Deployment of a Reusable Project Skeleton](https://embeddedartistry.com/course/automating-the-deployment-of-a-reusable-project-skeleton/) course.

The eventual goal is to use the STM32CubeL4 repository files as a "subproject", while the primary project provides the implementations for the Embedded VM framework. The STM32CubeL4 project provides IDE template files, but doesn't have any other build system. We are going to create Meson patch files that will be added to the subproject after it's cloned, allowing us to work with this project just like we would a normal Meson project. With local patch file support, we can maintain a build system and build rules that are independent of the original project. 

> This is the approach we typically take when we want to use non-Meson projects in our build.
> 
## Setting up the Subproject

First, we need to download the files as a subproject. I'm going to create a file called `STM32CubeL4.wrap` in the `subprojects/` folder of my repository. 

```
# STM32CubeL4.wrap
[wrap-git] 
url = https://github.com/STMicroelectronics/STM32CubeL4.git 
revision = v1.16.0 
patch_directory = STM32CubeL4-build 
depth = 1 
```

Note the `patch_directory` entry - this refers to a folder that contains my patch files for this project. These will be placed in `subprojects/packagefiles/STM32CubeL4-build`, since `subprojects/packagefiles` is the location that Meson will search for local patches. 

We will create an initial `meson.build` file for the `STM32CubeL4` project. We'll start with a project definition:

```
project('STM32CubeL4', 
    version: '1.16.0' 
) 
```

> Note that the version matches the revision we've specified in our .wrap file.

In the primary project's top-level `meson.build` file, we will include this project: 

```
stm32cube_subproject = subproject('STM32CubeL4') 
```

Now we can configure the build output folder, and Meson will clone the repository for us and automatically copy in the patch files: 

```
Cloning into 'STM32CubeL4'... 
remote: Enumerating objects: 37019, done. 
remote: Total 37019 (delta 0), reused 0 (delta 0), pack-reused 37019 
Receiving objects: 100% (37019/37019), 325.18 MiB | 4.31 MiB/s, done. 
Resolving deltas: 100% (22189/22189), done. 
Updating files: 100% (32667/32667), done. 
Note: switching to 'v1.16.0'. 

[... cut snippet]  

HEAD is now at d023c0d5 Release v1.16.0 
|Executing subproject STM32CubeL4 method meson 
| 
|Project name: STM32CubeL4 
|Project version: 1.16.0 
|Build targets in project: 1 
|Subproject STM32CubeL4 finished. 
```

All of the remaining changes are going to take place in the subproject's `meson.build` patch files. 

## Building an Example Application

Looking through the repository, there are a number of items we need to integrate. Luckily, there is a [getting started document](https://github.com/STMicroelectronics/STM32CubeL4/blob/master/Documentation/STM32CubeL4GettingStarted.pdf) that we can peruse. This document gives us some clues as to the structure and purpose of the repository directories. Items of interest are noted below.

- [`Drivers/BSP](https://github.com/STMicroelectronics/STM32CubeL4/tree/master/Drivers/BSP/)
    + The BSP package for each evaluation or demonstration board provided by this STM32 series  
    + We want to use [`Drivers/BSP/STM32L4xx_nucleo_144`](https://github.com/STMicroelectronics/STM32CubeL4/tree/master/Drivers/BSP/STM32L4xx_Nucleo_144)
- [`Drivers/BSP/Components`](https://github.com/STMicroelectronics/STM32CubeL4/tree/master/Drivers/BSP/Components)
    - Drivers for external components that are on the board (i.e., external to the processor)
    - Doesn't seem like anything applies to our specific Nucleo Board
- [`Drivers/CMSIS`](https://github.com/STMicroelectronics/STM32CubeL4/tree/master/Drivers/CMSIS/)
    + CMSIS code, helpfully contained within the STM32CubeL4 project
    + [`Drivers/CMSIS/Device/ST/STM32L4xx/Include/`](https://github.com/STMicroelectronics/STM32CubeL4/tree/master/Drivers/CMSIS/Device/ST/STM32L4xx/Include)
        * Device-specific headers that we will need to include
    + [`Drivers/CMSIS/Include`](https://github.com/STMicroelectronics/STM32CubeL4/tree/master/Drivers/CMSIS/Include)
        * General CMSIS headers that we will need to include
- [`Drivers/STM32L4xx_HAL_Driver`](https://github.com/STMicroelectronics/STM32CubeL4/tree/master/Drivers/STM32L4xx_HAL_Driver)
    + Contains both HAL and Low-layer (LL) driver implementations
    + "The HAL drivers offer high-level function-oriented highly-portable APIs. They hide the MCU and peripheral complexity to end user."
    + "The low-layer APIs provide low-level APIs at register level, with better optimization but less portability. They require a deep knowledge of MCU and peripheral specifications."
- [`Projects/NUCLEO-L4R5ZI-P`](https://github.com/STMicroelectronics/STM32CubeL4/tree/master/Projects/NUCLEO-L4R5ZI-P  )
    - Looks like this is just a subset of examples, with more in the [`Projects/NUCLEO-L4R5ZI`](https://github.com/STMicroelectronics/STM32CubeL4/tree/master/Projects/NUCLEO-L4R5ZI) directory
    - For our build, we will start with the [`PWR_RUN_SMPS` example](https://github.com/STMicroelectronics/STM32CubeL4/tree/master/Projects/NUCLEO-L4R5ZI-P/Examples/PWR/PWR_RUN_SMPS)
- Our example doesn't use any Middleware components, so we're going to ignore that directory for now.

### Template Build Files

Within the repository, there there are template build files. Unfortunately, it seems that ST expects you to use an IDE with this repository, because there is no example implementation that uses Make. That's a little strange, since CubeMX will generate a Makefile. 

For now, we're going to take a stab at creating a build without using CubeMX as a reference. We can always fall back to that program for generating an example Makefile that we can reference.

## Setting up the Meson Build Rules

We'll start with the easiest dependency to create: the Nucleo-144 BSP. We will need to create a dependency that will cause the relevant `.c` file to be compiled, as well as forwarding the appropriate BSP include directory.

```
stm32l4xx_nucleo_144_bsp_dep = declare_dependency( 
    sources: files( 
        'Drivers/BSP/STM32L4xx_Nucleo_144/stm32l4xx_nucleo_144.c', 
    ), 
    include_directories: include_directories( 
        'Drivers/BSP/STM32L4xx_Nucleo_144/', 
        is_system: true 
    ) 
) 
```

Another easy dependency to create is the CMSIS include dependency.

```
stm32_cmsis_dep = declare_dependency( 
    include_directories: include_directories( 
        'Drivers/CMSIS/Include', 
        'Drivers/CMSIS/Device/ST/STM32L4xx', 
        is_system: true 
    ) 
) 
```

We can create a processor dependency that specifies appropriate definitions and includes any processor-specific files, such as startup code. For now, we will just put placeholder comments for whatever the possible files might be.

```
stm32l4r5_dep = declare_dependency(
    sources: files(
        #startup_stm32l4r5xx.s
    ),
    compile_args: [
        '-DSTM32L4R5xx',
    ]
)
```

Next, we'll create dependencies for the HAL and LL drivers.

```
stm32_hal_dep = declare_dependency( 
    sources: files( 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_adc.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_adc_ex.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_can.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_comp.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_cortex.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_crc.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_crc_ex.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_cryp.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_cryp_ex.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dac.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dac_ex.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dcmi.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dfsdm.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dfsdm_ex.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dma.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dma2d.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dma_ex.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dsi.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_exti.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_firewall.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ex.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ramfunc.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_gfxmmu.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_gpio.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_hash.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_hash_ex.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_hcd.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_i2c.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_i2c_ex.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_irda.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_iwdg.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_lcd.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_lptim.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_ltdc.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_ltdc_ex.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_mmc.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_mmc_ex.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_msp_template.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_nand.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_nor.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_opamp.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_opamp_ex.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_ospi.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pcd.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pcd_ex.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pka.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pssi.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr_ex.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_qspi.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc_ex.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rng.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rng_ex.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rtc.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rtc_ex.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_sai.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_sai_ex.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_sd.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_sd_ex.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_smartcard.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_smartcard_ex.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_smbus.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_spi.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_spi_ex.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_sram.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_swpmi.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_tim.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_tim_ex.c', 
        #' Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_timebase_tim_template.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_tsc.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_uart.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_uart_ex.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_usart.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_usart_ex.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_wwdg.c', 
    ), 
    include_directories: include_directories( 
        'Drivers/STM32L4xx_HAL_Driver/Inc', 
        is_system: true 
    ), 
    compile_args: [ 
        '-DUSE_HAL_DRIVER', 
    ], 
) 

stm32_ll_dep = declare_dependency( 
    sources: files( 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_adc.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_comp.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_crc.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_crs.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_dac.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_dma.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_dma2d.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_exti.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_fmc.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_gpio.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_i2c.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_lptim.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_lpuart.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_opamp.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_pka.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_pwr.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_rcc.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_rng.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_rtc.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_sdmmc.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_spi.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_swpmi.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_tim.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_usart.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_usb.c', 
        'Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_utils.c', 
    ), 
    include_directories: include_directories( 
        'Drivers/STM32L4xx_HAL_Driver/Inc', 
        is_system: true 
    ),
    compile_args: ['-DUSE_FULL_LL_DRIVER']
) 
```

Now that we've set up the appropriate dependencies, we can work on building our application. All the files we need are in the [`Projects/NUCLEO-L4R5ZI-P/Examples/PWR/PWR_RUN_SMPS/Src/`](https://github.com/STMicroelectronics/STM32CubeL4/tree/master/Projects/NUCLEO-L4R5ZI-P/Examples/PWR/PWR_RUN_SMPS/Src/) folder, except for two essential files: the startup code and the linker script.

We can take a stab at building an initial executable build target for our application. This will import the dependencies we just created.

```
power_run_smps = executable('example_power_run_smps', 
    sources: files( 
        'Projects/NUCLEO-L4R5ZI-P/Examples/PWR/PWR_RUN_SMPS/Src/main.c', 
        'Projects/NUCLEO-L4R5ZI-P/Examples/PWR/PWR_RUN_SMPS/Src/stm32l4xx_hal_msp.c', 
        'Projects/NUCLEO-L4R5ZI-P/Examples/PWR/PWR_RUN_SMPS/Src/stm32l4xx_it.c', 
        'Projects/NUCLEO-L4R5ZI-P/Examples/PWR/PWR_RUN_SMPS/Src/system_stm32l4xx.c', 
    ), 
    include_directories: include_directories('Projects/NUCLEO-L4R5ZI-P/Examples/PWR/PWR_RUN_SMPS/Inc'), 
    c_args: [ 
        '-DUSE_STM32L4XX_NUCLEO_144_SMPS', 
        '-DUSE_ADP5301ACBZ', 
    ], 
    dependencies: [ 
        stm32_hal_dep, 
        stm32l4r5_dep, 
        stm32_cmsis_dep, 
        stm32l4xx_nucleo_144_bsp_dep 
    ] 
) 
```

> **Note:** I had to poke around the example source code to notice the relevant definitions I needed to set for the application to work properly.

The template build files in the Cube project are only for IDEs. However, if you look in the [`SW4STM32`](https://github.com/STMicroelectronics/STM32CubeL4/tree/master/Projects/NUCLEO-L4R5ZI-P/Examples/PWR/PWR_RUN_SMPS/SW4STM32) directory, you will find an example startup file and linker script that we can use.

We'll pick up the appropriate linker script by specifying the `link_args`. We'll also add the startup file to our `sources` list.

```
power_run_smps = executable('example_power_run_smps', 
    sources: files( 
        'Projects/NUCLEO-L4R5ZI-P/Examples/PWR/PWR_RUN_SMPS/Src/main.c', 
        'Projects/NUCLEO-L4R5ZI-P/Examples/PWR/PWR_RUN_SMPS/Src/stm32l4xx_hal_msp.c', 
        'Projects/NUCLEO-L4R5ZI-P/Examples/PWR/PWR_RUN_SMPS/Src/stm32l4xx_it.c', 
        'Projects/NUCLEO-L4R5ZI-P/Examples/PWR/PWR_RUN_SMPS/Src/system_stm32l4xx.c', 
        'Projects/NUCLEO-L4R5ZI-P/Examples/PWR/PWR_RUN_SMPS/SW4STM32/startup_stm32l4r5xx.s' 
    ), 
    include_directories: include_directories('Projects/NUCLEO-L4R5ZI-P/Examples/PWR/PWR_RUN_SMPS/Inc'), 
    c_args: [ 
        '-DUSE_STM32L4XX_NUCLEO_144_SMPS', 
        '-DUSE_ADP5301ACBZ', 
    ], 
    link_args: [ 
        '-L' + meson.current_source_dir() / 'Projects/NUCLEO-L4R5ZI-P/Examples/PWR/PWR_RUN_SMPS/SW4STM32/STM32L4R5ZI_NUCLEO', 
        '-T' + 'STM32L4R5ZITx_FLASH.ld', 
    ], 
    dependencies: [ 
        stm32_hal_dep, 
        stm32l4r5_dep, 
        stm32_cmsis_dep, 
        stm32l4xx_nucleo_144_bsp_dep 
    ] 
) 
```

Normally, we would build projects against the [Embedded Artistry libc](https://github.com/embeddedartistry/libc) project, but for now we will adjust our `cortex-m4_hardfloat.txt` cross file at the moment to add the necessary flags for `picolibc` and `nosys`.

```
# Meson Cross-compilation File for Cortex-M4 processors using Hardware FP 
# This file should be layered after arm.txt 
# Requires that arm-none-eabi-* is found in your PATH 
# For more information: http://mesonbuild.com/Cross-compilation.html 

[properties] 
c_args = [ '-mcpu=cortex-m4', '-mfloat-abi=hard', '-mfpu=fpv4-sp-d16', '-mabi=aapcs', '-mthumb',] 
c_link_args = [ '-mcpu=cortex-m4', '-mfloat-abi=hard', '-mfpu=fpv4-sp-d16', '-mabi=aapcs', '-mthumb', '-specs=nano.specs', '-lc', '-lm', '-lnosys',] 
cpp_args = [ '-mcpu=cortex-m4', '-mfloat-abi=hard', '-mfpu=fpv4-sp-d16', '-mabi=aapcs', '-mthumb', '-specs=nano.specs', '-lc', '-lm', '-lnosys',] 
cpp_link_args = [ '-mcpu=cortex-m4', '-mfloat-abi=hard', '-mfpu=fpv4-sp-d16', '-mabi=aapcs', '-mthumb',] 

[host_machine] 
cpu = 'cortex-m4' 
```

Now we can run the build and everything compiles successfully:

```
$ make 
ninja: Entering directory `buildresults' 
[15/84] Compiling C object subprojects..._PWR_PWR_RUN_SMPS_Src_stm32l4xx_it.c.o 
../subprojects/STM32CubeL4/Projects/NUCLEO-L4R5ZI-P/Examples/PWR/PWR_RUN_SMPS/Src/stm32l4xx_it.c:34:1: warning: 'extern' is not at beginning of declaration [-Wold-style-declaration] 
   34 | uint32_t extern button_pressed; 
      | ^~~~~~~~ 
[80/84] Compiling C object subprojects...xx_Nucleo_144_stm32l4xx_nucleo_144.c.o 
../subprojects/STM32CubeL4/Drivers/BSP/STM32L4xx_Nucleo_144/stm32l4xx_nucleo_144.c: In function 'BSP_SMPS_Init': 
../subprojects/STM32CubeL4/Drivers/BSP/STM32L4xx_Nucleo_144/stm32l4xx_nucleo_144.c:494:33: warning: unused parameter 'VoltageRange' [-Wunused-parameter] 
  494 | uint32_t BSP_SMPS_Init(uint32_t VoltageRange) 
      |                        ~~~~~~~~~^~~~~~~~~~~~ 
[82/84] Compiling C object subprojects...x_HAL_Driver_Src_stm32l4xx_hal_rcc.c.o 
../subprojects/STM32CubeL4/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc.c: In function 'HAL_RCC_OscConfig': 
../subprojects/STM32CubeL4/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc.c:946:1: warning: embedding a directive within macro arguments is not portable 
  946 | #if defined(RCC_PLLP_SUPPORT) 
      | ^ 
../subprojects/STM32CubeL4/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc.c:948:1: warning: embedding a directive within macro arguments is not portable 
  948 | #endif 
      | ^ 
[84/84] Linking target subprojects/STM32CubeL4/example_power_run_smps 
```

## Testing the Build

Next, we need to convert this to the proper file format so we can test our example program on real hardware. We'll define a `custom_target` that will convert the `ELF` file to a `.hex` file: 

> **Note:** Normally we would use our build system's built-in module for this. However,  it's not available to the subproject patch files.

```
power_run_smps_hex = custom_target('power_run_smps.hex', 
    input: power_run_smps, 
    output: 'power_run_smps.hex', 
    command: [ 
        meson.get_external_property('objcopy', '', native: false), 
        '-O', 'ihex', '@INPUT@', '@OUTPUT@' 
    ], 
    build_by_default: true 
) 
```

When we run the build now, we will see:

```
[1/1] Generating power_run_smps.hex with a custom command 
```

Normally we would use the open-source `st-flash` command-line tool for programming the hardware, but this doesn't appear to work properly for L4 processors.

```
$ st-flash --format ihex write buildresults/subprojects/STM32CubeL4/power_run_smps.hex 
[... Truncated by Phillip]
2020-10-08T10:19:23 INFO common.c: Starting Flash write for F2/F4/L4 
2020-10-08T10:19:23 INFO flash_loader.c: Successfully loaded flash loader in sram 
size: 32768 
2020-10-08T10:19:23 ERROR flash_loader.c: flash loader run error 
2020-10-08T10:19:23 ERROR common.c: stlink_flash_loader_run(0x8000000) failed! == -1 
stlink_fwrite_flash() == -1 
```

Instead, we converted the onboard ST-Link into a J-Link and used the `JFlashLite` program to flash the generated hex file to the device.

The program starts up, and I see the green LED (LED1) blinking. Just to be sure the code is running, we can modify the blinking logic to blink both LED1 and LED2 at the same time.

```
void HAL_SYSTICK_Callback(void) 
{ 
  HAL_IncTick(); 

  if (TimingDelay != 0) 
  { 
    TimingDelay--; 
  } 
  else 
  { 
    /* Toggle LED1 */ 

    BSP_LED_Toggle(LED1); 
    BSP_LED_Toggle(LED2); 
    TimingDelay = LED_TOGGLE_DELAY; 
  } 
} 
```

This change is reflected at runtime, so we can be confident that we are correctly building the example application!

## Summary

Now, let's summarize what we've learned:

- To use this project to build an application, you will need:
    - The general CMSIS includes
    - The processor-specific CMSIS includes
    - Either the HAL or LL drivers
    - Processor startup script
    - Compiler definition for your specific processor (e.g., `-DSTM32L4R5xx`) 
* For your system, you will need to use the templates (or CubeMX) to create versions of these files that are tuned for your specific application:
    - `stm32l4xx_hal_msp.c` (If using the HAL drivers)
    - `stm32l44xx_it.c` (You can also implement handlers directly in your own code, which is what we typically do)
    - `system_stm32l4xx.c `

In order to user our new build rules, we can use the existing dependencies that we have already built. You can remove the example application build target if you prefer, or you can set it so the target is not automatically built when you are in subproject mode:

```
power_run_smps = executable('example_power_run_smps',
    sources: files(
        'Projects/NUCLEO-L4R5ZI-P/Examples/PWR/PWR_RUN_SMPS/Src/main.c',
        'Projects/NUCLEO-L4R5ZI-P/Examples/PWR/PWR_RUN_SMPS/Src/stm32l4xx_hal_msp.c',
        'Projects/NUCLEO-L4R5ZI-P/Examples/PWR/PWR_RUN_SMPS/Src/stm32l4xx_it.c',
        'Projects/NUCLEO-L4R5ZI-P/Examples/PWR/PWR_RUN_SMPS/Src/system_stm32l4xx.c',
        'Projects/NUCLEO-L4R5ZI-P/Examples/PWR/PWR_RUN_SMPS/SW4STM32/startup_stm32l4r5xx.s'
    ),
    include_directories: include_directories('Projects/NUCLEO-L4R5ZI-P/Examples/PWR/PWR_RUN_SMPS/Inc'),
    c_args: [
        '-DUSE_STM32L4XX_NUCLEO_144_SMPS',
        '-DUSE_ADP5301ACBZ',
    ],
    link_args: [
        '-L' + meson.current_source_dir() / 'Projects/NUCLEO-L4R5ZI-P/Examples/PWR/PWR_RUN_SMPS/SW4STM32/STM32L4R5ZI_NUCLEO',
        '-T' + 'STM32L4R5ZITx_FLASH.ld',
    ],
    dependencies: [
        stm32_hal_dep,
        stm32l4r5_dep,
        stm32_cmsis_dep,
        stm32_cmsis_device_dep,
        stm32l4xx_nucleo_144_bsp_dep
    ],
    build_by_default: meson.is_subproject() == false
)

power_run_smps_hex = custom_target('power_run_smps.hex',
    input: power_run_smps,
    output: 'power_run_smps.hex',
    command: [
        meson.get_external_property('objcopy', '', native: false),
        '-O', 'ihex', '@INPUT@', '@OUTPUT@'
    ],
    build_by_default: meson.is_subproject() == false
)
```
