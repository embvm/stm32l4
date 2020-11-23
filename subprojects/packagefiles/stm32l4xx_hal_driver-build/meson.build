project('CMSIS Device L4',
	'c',
	version: '1.16.0'
)

#####################
# STM32 HAL Drivers #
#####################

stm32_hal_dep = declare_dependency(
	sources: files(
		'Src/stm32l4xx_hal.c',
		'Src/stm32l4xx_hal_adc.c',
		'Src/stm32l4xx_hal_adc_ex.c',
		'Src/stm32l4xx_hal_can.c',
		'Src/stm32l4xx_hal_comp.c',
		'Src/stm32l4xx_hal_cortex.c',
		'Src/stm32l4xx_hal_crc.c',
		'Src/stm32l4xx_hal_crc_ex.c',
		'Src/stm32l4xx_hal_cryp.c',
		'Src/stm32l4xx_hal_cryp_ex.c',
		'Src/stm32l4xx_hal_dac.c',
		'Src/stm32l4xx_hal_dac_ex.c',
		'Src/stm32l4xx_hal_dcmi.c',
		'Src/stm32l4xx_hal_dfsdm.c',
		'Src/stm32l4xx_hal_dfsdm_ex.c',
		'Src/stm32l4xx_hal_dma.c',
		'Src/stm32l4xx_hal_dma2d.c',
		'Src/stm32l4xx_hal_dma_ex.c',
		'Src/stm32l4xx_hal_dsi.c',
		'Src/stm32l4xx_hal_exti.c',
		'Src/stm32l4xx_hal_firewall.c',
		'Src/stm32l4xx_hal_flash.c',
		'Src/stm32l4xx_hal_flash_ex.c',
		'Src/stm32l4xx_hal_flash_ramfunc.c',
		'Src/stm32l4xx_hal_gfxmmu.c',
		'Src/stm32l4xx_hal_gpio.c',
		'Src/stm32l4xx_hal_hash.c',
		'Src/stm32l4xx_hal_hash_ex.c',
		'Src/stm32l4xx_hal_hcd.c',
		'Src/stm32l4xx_hal_i2c.c',
		'Src/stm32l4xx_hal_i2c_ex.c',
		'Src/stm32l4xx_hal_irda.c',
		'Src/stm32l4xx_hal_iwdg.c',
		'Src/stm32l4xx_hal_lcd.c',
		'Src/stm32l4xx_hal_lptim.c',
		'Src/stm32l4xx_hal_ltdc.c',
		'Src/stm32l4xx_hal_ltdc_ex.c',
		'Src/stm32l4xx_hal_mmc.c',
		'Src/stm32l4xx_hal_mmc_ex.c',
		'Src/stm32l4xx_hal_msp_template.c',
		'Src/stm32l4xx_hal_nand.c',
		'Src/stm32l4xx_hal_nor.c',
		'Src/stm32l4xx_hal_opamp.c',
		'Src/stm32l4xx_hal_opamp_ex.c',
		'Src/stm32l4xx_hal_ospi.c',
		'Src/stm32l4xx_hal_pcd.c',
		'Src/stm32l4xx_hal_pcd_ex.c',
		'Src/stm32l4xx_hal_pka.c',
		'Src/stm32l4xx_hal_pssi.c',
		'Src/stm32l4xx_hal_pwr.c',
		'Src/stm32l4xx_hal_pwr_ex.c',
		'Src/stm32l4xx_hal_qspi.c',
		'Src/stm32l4xx_hal_rcc.c',
		'Src/stm32l4xx_hal_rcc_ex.c',
		'Src/stm32l4xx_hal_rng.c',
		'Src/stm32l4xx_hal_rng_ex.c',
		'Src/stm32l4xx_hal_rtc.c',
		'Src/stm32l4xx_hal_rtc_ex.c',
		'Src/stm32l4xx_hal_sai.c',
		'Src/stm32l4xx_hal_sai_ex.c',
		'Src/stm32l4xx_hal_sd.c',
		'Src/stm32l4xx_hal_sd_ex.c',
		'Src/stm32l4xx_hal_smartcard.c',
		'Src/stm32l4xx_hal_smartcard_ex.c',
		'Src/stm32l4xx_hal_smbus.c',
		'Src/stm32l4xx_hal_spi.c',
		'Src/stm32l4xx_hal_spi_ex.c',
		'Src/stm32l4xx_hal_sram.c',
		'Src/stm32l4xx_hal_swpmi.c',
		'Src/stm32l4xx_hal_tim.c',
		'Src/stm32l4xx_hal_tim_ex.c',
		#'Src/stm32l4xx_hal_timebase_tim_template.c',
		'Src/stm32l4xx_hal_tsc.c',
		'Src/stm32l4xx_hal_uart.c',
		'Src/stm32l4xx_hal_uart_ex.c',
		'Src/stm32l4xx_hal_usart.c',
		'Src/stm32l4xx_hal_usart_ex.c',
		'Src/stm32l4xx_hal_wwdg.c',
	),
	include_directories: include_directories(
		'Inc',
		is_system: true
	),
	compile_args: [
		'-DUSE_HAL_DRIVER',
	],
)

stm32_ll_dep = declare_dependency(
	sources: files(
		'Src/stm32l4xx_ll_adc.c',
		'Src/stm32l4xx_ll_comp.c',
		'Src/stm32l4xx_ll_crc.c',
		'Src/stm32l4xx_ll_crs.c',
		'Src/stm32l4xx_ll_dac.c',
		'Src/stm32l4xx_ll_dma.c',
		'Src/stm32l4xx_ll_dma2d.c',
		'Src/stm32l4xx_ll_exti.c',
		#'Src/stm32l4xx_ll_fmc.c', # Actually depends on HAL
		'Src/stm32l4xx_ll_gpio.c',
		'Src/stm32l4xx_ll_i2c.c',
		'Src/stm32l4xx_ll_lptim.c',
		'Src/stm32l4xx_ll_lpuart.c',
		'Src/stm32l4xx_ll_opamp.c',
		'Src/stm32l4xx_ll_pka.c',
		'Src/stm32l4xx_ll_pwr.c',
		'Src/stm32l4xx_ll_rcc.c',
		'Src/stm32l4xx_ll_rng.c',
		'Src/stm32l4xx_ll_rtc.c',
		#'Src/stm32l4xx_ll_sdmmc.c', # Actually depends on HAL
		'Src/stm32l4xx_ll_spi.c',
		'Src/stm32l4xx_ll_swpmi.c',
		'Src/stm32l4xx_ll_tim.c',
		'Src/stm32l4xx_ll_usart.c',
		#'Src/stm32l4xx_ll_usb.c', # Actually depends on HAL
		'Src/stm32l4xx_ll_utils.c',
	),
	include_directories: include_directories(
		'Inc',
		is_system: true
	),
	compile_args: '-DUSE_FULL_LL_DRIVER',
)

##############
# Processors #
##############

stm32l4r5_dep = declare_dependency(
	sources: files(
		#Core/Src/stm32l4xx_it.c \
		#Core/Src/stm32l4xx_hal_msp.c \
		#Core/Src/system_stm32l4xx.c
		#startup_stm32l4r5xx.s
	),
	compile_args: [
		'-DSTM32L4R5xx',
	]
)
