#include "stm32_rcc.hpp"
#include <volatile/volatile.hpp>
#include <array>
#include <processor_includes.hpp>

constexpr std::array<unsigned, 9> gpio_enable_bits = {
	RCC_AHB2ENR_GPIOAEN, RCC_AHB2ENR_GPIOBEN, RCC_AHB2ENR_GPIOCEN,
	RCC_AHB2ENR_GPIODEN, RCC_AHB2ENR_GPIOEEN, RCC_AHB2ENR_GPIOFEN,
	RCC_AHB2ENR_GPIOGEN, RCC_AHB2ENR_GPIOHEN, RCC_AHB2ENR_GPIOIEN};

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
