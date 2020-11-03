// Copyright 2020 Embedded Artistry LLC
// SPDX-License-Identifier: MIT

#include "stm32_rcc.hpp"
#include <array>
#include <processor_includes.hpp>
#include <stm32l4xx_ll_rcc.h>
#include <volatile/volatile.hpp>

// TODO: how can we be flexible here, adjusting for other chips?

namespace
{
constexpr std::array<unsigned, 9> gpio_enable_bits = {
	RCC_AHB2ENR_GPIOAEN, RCC_AHB2ENR_GPIOBEN, RCC_AHB2ENR_GPIOCEN,
	RCC_AHB2ENR_GPIODEN, RCC_AHB2ENR_GPIOEEN, RCC_AHB2ENR_GPIOFEN,
	RCC_AHB2ENR_GPIOGEN, RCC_AHB2ENR_GPIOHEN, RCC_AHB2ENR_GPIOIEN};

// TODO: fix this, use an enum instead so we don't have to waste a slot
// CH0 is not valid for STM32 timers
constexpr std::array<volatile uint32_t* const, 9> timer_enable_reg = {
	nullptr,		&RCC->AHB2ENR,	&RCC->APB1ENR1, &RCC->APB1ENR1, &RCC->APB1ENR1,
	&RCC->APB1ENR1, &RCC->APB1ENR1, &RCC->APB1ENR1, &RCC->AHB2ENR};

// CH0 is not valid for STM32 timers
constexpr std::array<unsigned, 9> timer_enable_bits = {0,
													   RCC_APB2ENR_TIM1EN,
													   RCC_APB1ENR1_TIM2EN,
													   RCC_APB1ENR1_TIM3EN,
													   RCC_APB1ENR1_TIM4EN,
													   RCC_APB1ENR1_TIM5EN,
													   RCC_APB1ENR1_TIM6EN,
													   RCC_APB1ENR1_TIM7EN,
													   RCC_APB2ENR_TIM8EN_Pos};

constexpr std::array<volatile uint32_t* const, 4> i2c_enable_reg = {&RCC->APB1ENR1, &RCC->APB1ENR1,
																	&RCC->APB1ENR1, &RCC->AHB2ENR};
constexpr std::array<unsigned, 4> i2c_enable_bits = {RCC_APB1ENR1_I2C1EN, RCC_APB1ENR1_I2C2EN,
													 RCC_APB1ENR1_I2C3EN, RCC_APB1ENR2_I2C4EN};

// TODO: should we have a way to select other clocks? Or just enforce sysclock for now?
constexpr std::array<unsigned, 4> i2c_clock_source = {
	LL_RCC_I2C1_CLKSOURCE_SYSCLK, LL_RCC_I2C2_CLKSOURCE_SYSCLK, LL_RCC_I2C3_CLKSOURCE_SYSCLK,
	LL_RCC_I2C4_CLKSOURCE_SYSCLK};

constexpr std::array<unsigned, 2> dma_enable_bits = {RCC_AHB1ENR_DMA1EN, RCC_AHB1ENR_DMA2EN};

}; // namespace

void STM32ClockControl::gpioEnable(embvm::gpio::port port) noexcept
{
	uint32_t val = embutil::volatile_load(&RCC->AHB2ENR);
	val |= gpio_enable_bits[port];
	embutil::volatile_store(&RCC->AHB2ENR, val);
}

void STM32ClockControl::gpioDisable(embvm::gpio::port port) noexcept
{
	uint32_t val = embutil::volatile_load(&RCC->AHB2ENR);
	val &= ~(gpio_enable_bits[port]);
	embutil::volatile_store(&RCC->AHB2ENR, val);
}

void STM32ClockControl::timerEnable(embvm::timer::channel timer) noexcept
{
	volatile uint32_t* const reg = timer_enable_reg[timer];
	assert(reg); // Check for a valid timer channel
	uint32_t val = embutil::volatile_load(reg);
	val |= timer_enable_bits[timer];
	embutil::volatile_store(reg, val);
}

void STM32ClockControl::timerDisable(embvm::timer::channel timer) noexcept
{
	volatile uint32_t* const reg = timer_enable_reg[timer];
	assert(reg); // Check for a valid timer channel
	uint32_t val = embutil::volatile_load(reg);
	val &= ~(timer_enable_bits[timer]);
	embutil::volatile_store(reg, val);
}

void STM32ClockControl::i2cEnable(uint8_t device) noexcept
{
	volatile uint32_t* const reg = i2c_enable_reg[device];
	assert(reg);
	uint32_t val = embutil::volatile_load(reg);
	val |= i2c_enable_bits[device];
	embutil::volatile_store(reg, val);
	LL_RCC_SetI2CClockSource(i2c_clock_source[device]);
}

void STM32ClockControl::i2cDisable(uint8_t device) noexcept
{
	volatile uint32_t* const reg = i2c_enable_reg[device];
	assert(reg);
	uint32_t val = embutil::volatile_load(reg);
	val &= ~(i2c_enable_bits[device]);
	embutil::volatile_store(reg, val);
}

void STM32ClockControl::dmaEnable(uint8_t device) noexcept
{
	uint32_t val = embutil::volatile_load(&RCC->AHB1ENR);
	val |= dma_enable_bits[device];
	embutil::volatile_store(&RCC->AHB1ENR, val);
}

void STM32ClockControl::dmaDisable(uint8_t device) noexcept
{
	uint32_t val = embutil::volatile_load(&RCC->AHB1ENR);
	val &= ~(dma_enable_bits[device]);
	embutil::volatile_store(&RCC->AHB1ENR, val);
}

void STM32ClockControl::dmaMuxEnable() noexcept
{
	uint32_t val = embutil::volatile_load(&RCC->AHB1ENR);
	val |= RCC_AHB1ENR_DMAMUX1EN;
	embutil::volatile_store(&RCC->AHB1ENR, val);
}

void STM32ClockControl::dmaMuxDisable() noexcept
{
	uint32_t val = embutil::volatile_load(&RCC->AHB1ENR);
	val &= ~RCC_AHB1ENR_DMAMUX1EN;
	embutil::volatile_store(&RCC->AHB1ENR, val);
}
