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

	// Output Functions
	static void set(uint8_t port, uint8_t pin) noexcept;
	static void clear(uint8_t port, uint8_t pin) noexcept;
	static void toggle(uint8_t port, uint8_t pin) noexcept;

	// Input functions
	static bool get(uint8_t port, uint8_t pin) noexcept;

  private:
	/// This class can't be instantiated
	STM32GPIOTranslator() = default;
	~STM32GPIOTranslator() = default;
};

#endif // STM32_GPIO_HELPER_HPP_
