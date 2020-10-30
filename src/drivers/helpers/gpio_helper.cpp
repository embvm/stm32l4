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

// TODO: how do we abstract this for general stm32 types?
// Maybe we have an include for each processor that defines values such as this, the RCC clock bits,
// etc.? This is important at least for the GPIO banks, because the differne tprocessor families
// have different bank counts.
constexpr std::array<GPIO_TypeDef*, 9> ports = {GPIOA, GPIOB, GPIOC, GPIOD, GPIOE,
												GPIOF, GPIOG, GPIOH, GPIOI};

#pragma mark - Implementations -

void STM32GPIOTranslator::configure_output(uint8_t port, uint8_t pin) noexcept
{
	LL_GPIO_InitTypeDef gpio_init = {.Pin = PIN_INT_TO_STM32(pin),
									 .Mode = LL_GPIO_MODE_OUTPUT,
									 .Speed = LL_GPIO_SPEED_FREQ_MEDIUM,
									 .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
									 .Pull = LL_GPIO_PULL_NO,
									 .Alternate = LL_GPIO_AF_0};

	LL_GPIO_Init(ports[port], &gpio_init); // GPIOx, GPIO_InitStruct
}

// TODO: address pull-up setting
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

// TODO: add a method for configuring pullup setting
void STM32GPIOTranslator::configure_alternate_i2c(uint8_t port, uint8_t pin,
												  uint8_t alt_func) noexcept
{
	LL_GPIO_InitTypeDef gpio_init = {
		.Pin = PIN_INT_TO_STM32(pin),
		.Mode = LL_GPIO_MODE_ALTERNATE,
		.Speed = LL_GPIO_SPEED_FREQ_HIGH,
		.OutputType = LL_GPIO_OUTPUT_OPENDRAIN,
		.Pull = LL_GPIO_PULL_UP,
		.Alternate = alt_func,
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

void STM32GPIOTranslator::toggle(uint8_t port, uint8_t pin) noexcept
{
	LL_GPIO_TogglePin(ports[port], PIN_INT_TO_STM32(pin));
}

bool STM32GPIOTranslator::get(uint8_t port, uint8_t pin) noexcept
{
	return static_cast<bool>(LL_GPIO_IsInputPinSet(ports[port], PIN_INT_TO_STM32(pin)));
}
