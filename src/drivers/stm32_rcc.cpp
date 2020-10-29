#include "stm32_rcc.hpp"
#include <array>
#include <processor_includes.hpp>
#include <volatile/volatile.hpp>

// TODO: how can we be flexible here, adjusting for other chips?

namespace
{
constexpr std::array<unsigned, 9> gpio_enable_bits = {
	RCC_AHB2ENR_GPIOAEN, RCC_AHB2ENR_GPIOBEN, RCC_AHB2ENR_GPIOCEN,
	RCC_AHB2ENR_GPIODEN, RCC_AHB2ENR_GPIOEEN, RCC_AHB2ENR_GPIOFEN,
	RCC_AHB2ENR_GPIOGEN, RCC_AHB2ENR_GPIOHEN, RCC_AHB2ENR_GPIOIEN};

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
