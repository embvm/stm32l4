#ifndef STM32_RCC_HELPER_HPP_
#define STM32_RCC_HELPER_HPP_

#include <cstdint>
#include <driver/gpio.hpp>
#include <driver/timer.hpp>

/** Translation class which handles STM32 RCC Interactions.
 *
 * The GPIO function implementations are isolated from this header because we do not want to make
 * the STM32 headers accessible from the rest of the system.
 *
 * This class cannot be directly instantiated.
 */
class STM32ClockControl
{
  public:
	/** Enable the peripheral clock to one of the GPIO banks.
	 *
	 * @precondition port is a valid port for the STM32 processor.
	 * @postcondition GPIO peripheral clock is enabled.
	 *
	 * @param [in] port The GPIO port to enable.
	 */
	static void gpioEnable(embvm::gpio::port port) noexcept;

	/** Disable the peripheral clock to one of the GPIO banks.
	 *
	 * @precondition port is a valid port for the STM32 processor.
	 * @postcondition GPIO peripheral clock is disabled.
	 *
	 * @param [in] port The GPIO port to disable.
	 */
	static void gpioDisable(embvm::gpio::port port) noexcept;

	/** Enable the peripheral clock to one of the timer devices.
	 *
	 * @precondition timer is a valid channel for the STM32 processor.
	 * @postcondition Timer peripheral clock is enabled.
	 *
	 * @param [in] timer The timer device to enable.
	 */
	static void timerEnable(embvm::timer::channel timer) noexcept;

	/** Disable the peripheral clock to one of the timer devices..
	 *
	 * @precondition timer is a valid channel for the STM32 processor.
	 * @postcondition Timer peripheral clock is disabled.
	 *
	 * @param [in] timer The timer device to disable.
	 */
	static void timerDisable(embvm::timer::channel timer) noexcept;

  private:
	/// This class can't be instantiated
	STM32ClockControl() = default;
	~STM32ClockControl() = default;
};

#endif // STM32_RCC_HELPER_HPP_
