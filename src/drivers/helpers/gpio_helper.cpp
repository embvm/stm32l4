#include "gpio_helper.hpp"
#include <array>
#include <processor_includes.hpp>
#include <stm32l4xx_ll_gpio.h>

/// Implementation of the GPIO drivers is handled here so we can keep the ARM/STM headers
/// decoupled from the rest of the system.

/** STM32 Implementation Notes
 *
 * *** OPEN TASKS ***
 * TODO: support speed configuration (Part of STM32 driver, but not core interface)
 * TODO: support output type configuration (Part of STM32 driver, but not core interface)
 * TODO: support alternate configuration (Part of STM32 driver, but not core interface)
 * TODO: pull_config/pullup support
 * 	@code
 * 	#define LL_GPIO_PULL_NO                    (0x00000000U)
 * 	#define LL_GPIO_PULL_UP                    GPIO_PUPDR_PUPD0_0
 * 	#define LL_GPIO_PULL_DOWN                  GPIO_PUPDR_PUPD0_1
 *	@endcode
 */

#pragma mark - Macros -

/* Convert GPIO integral pin representation to the version expected by STM32
 *
 * As you can see, each pin gets its own bit:
 *
 * @code
 * #define GPIO_PIN_0                 ((uint16_t)0x0001)
 * #define GPIO_PIN_1                 ((uint16_t)0x0002)
 * #define GPIO_PIN_2                 ((uint16_t)0x0004)
 * #define GPIO_PIN_3                 ((uint16_t)0x0008)
 * @endcode
 *
 * @param x The integral pin number representation [0..15]
 * @returns The STM32 bitmask for the pin
 */
#define PIN_INT_TO_STM32(x) (1U << x)

constexpr std::array<GPIO_TypeDef*, 9> ports = {GPIOA, GPIOB, GPIOC, GPIOD, GPIOE,
												GPIOF, GPIOG, GPIOH, GPIOI};

#define __HAL_RCC_GPIOB_CLK_ENABLE()                          \
	do                                                        \
	{                                                         \
		__IO uint32_t tmpreg;                                 \
		SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOBEN);           \
		/* Delay after an RCC peripheral clock enabling */    \
		tmpreg = READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOBEN); \
	} while(0)

#define __HAL_RCC_GPIOC_CLK_ENABLE()                          \
	do                                                        \
	{                                                         \
		__IO uint32_t tmpreg;                                 \
		SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOCEN);           \
		/* Delay after an RCC peripheral clock enabling */    \
		tmpreg = READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOCEN); \
	} while(0)

#pragma mark - Implementations -

void STM32GPIOTranslator::configure_output(uint8_t port, uint8_t pin) noexcept
{
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	LL_GPIO_InitTypeDef gpio_init = {.Pin = PIN_INT_TO_STM32(pin),
									 .Mode = LL_GPIO_MODE_OUTPUT,
									 .Speed = LL_GPIO_SPEED_FREQ_MEDIUM,
									 .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
									 .Pull = LL_GPIO_PULL_NO,
									 .Alternate = LL_GPIO_AF_0};

	LL_GPIO_Init(ports[port], &gpio_init); // GPIOx, GPIO_InitStruct
}

void STM32GPIOTranslator::configure_input(uint8_t port, uint8_t pin,
										  [[maybe_unused]] uint8_t pull_config) noexcept
{
	LL_GPIO_InitTypeDef gpio_init = {
		.Pin = PIN_INT_TO_STM32(pin),
		.Mode = LL_GPIO_MODE_INPUT,
		.Speed = LL_GPIO_SPEED_FREQ_MEDIUM,
		.OutputType = LL_GPIO_OUTPUT_PUSHPULL,
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
