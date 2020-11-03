// Copyright 2020 Embedded Artistry LLC
// SPDX-License-Identifier: MIT

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

	/** Disable the peripheral clock to one of the timer devices.
	 *
	 * @precondition timer is a valid channel for the STM32 processor.
	 * @postcondition Timer peripheral clock is disabled.
	 *
	 * @param [in] timer The timer device to disable.
	 */
	static void timerDisable(embvm::timer::channel timer) noexcept;

	// TODO: define a portable type like the ones above.
	/** Enable the peripheral clock to one of the I2C devices
	 *
	 * @precondition I2C Device is valid for the STM32 processor.
	 * @postcondition I2C device's peripheral clock is enabled.
	 *
	 * @param [in] timer The timer device to enable.
	 */
	static void i2cEnable(uint8_t device) noexcept;

	/** Disable the peripheral clock to one of the I2C devices.
	 *
	 * @precondition I2C Device is valid for the STM32 processor.
	 * @postcondition I2C device's peripheral clock is disabled.
	 *
	 * @param [in] timer The timer device to disable.
	 */
	static void i2cDisable(uint8_t device) noexcept;

	// TODO: define a portable type like the ones above
	/** Enable the peripheral clock to one of the DMA devices
	 *
	 * @precondition DMA device is valid for the STM32 processor.
	 * @postcondition DMA device's peripheral clock is enabled
	 *
	 * @param [in] The DMA device ID to enable.
	 */
	static void dmaEnable(uint8_t device) noexcept;

	/** Disable the peripheral clock to one of the DMA devices
	 *
	 * @precondition DMA device is valid for the STM32 processor.
	 * @postcondition DMA device's peripheral clock is disabled
	 *
	 * @param [in] The DMA device ID to enable.
	 */
	static void dmaDisable(uint8_t device) noexcept;

	// TODO: document. Do we fold into dmaDisable?
	static void dmaMuxEnable() noexcept;
	static void dmaMuxDisable() noexcept;

  private:
	/// This class can't be instantiated
	STM32ClockControl() = default;
	~STM32ClockControl() = default;
};

#endif // STM32_RCC_HELPER_HPP_
