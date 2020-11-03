// Copyright 2020 Embedded Artistry LLC
// SPDX-License-Identifier: MIT

#ifndef STM32_I2C_MASTER_HPP_
#define STM32_I2C_MASTER_HPP_

#include <driver/i2c.hpp>
#include <stm32_dma.hpp>
#include <stm32_gpio.hpp>
// TODO: #include <driver/hal_driver.hpp>

// TODO: Handle interrupt priority - as a constructor parameter
// TODO: use HAL base class to support bottom-half interrupt handler registration

/**
 *
 * The current implementation does not support SMBus.
 *
 * Note that this class is implemented using DMA, so you need to create
 * DMA instances for the tx_channel and rx_channel in order to use this driver.
 *
 * @code
 * STM32DMA dma_ch_i2c_tx{STM32DMA::device::dma1, STM32DMA::channel::CH2};
 * STM32DMA dma_ch_i2c_rx{STM32DMA::device::dma1, STM32DMA::channel::CH3};
 * STM32I2CMaster i2c2{STM32I2CMaster::device::i2c2, dma_ch_i2c_tx, dma_ch_i2c_rx};
 * @endcode
 *
 * The I2C driver will handle its specific configuration, address assignment, interrupt handlers,
 * and starting/stopping of the driver internally. You must, however, enable the appropriate
 * DMA device clock in the hardware platform; the I2C driver will not handle that.
 *
 * @see STM32DMA
 */
class STM32I2CMaster final : public embvm::i2c::master
{
  public:
	// TODO: refactor into base class?
	enum device : uint8_t
	{
		i2c1 = 0,
		i2c2,
		i2c3,
		i2c4,
		NUM_I2C_DEVICES
	};

  public:
	explicit STM32I2CMaster(STM32I2CMaster::device dev, STM32DMA& tx_channel,
							STM32DMA& rx_channel) noexcept
		: device_(dev), tx_channel_(tx_channel), rx_channel_(rx_channel)
	{
	}
	~STM32I2CMaster() noexcept = default;

	void enableInterrupts() noexcept;
	void disableInterrupts() noexcept;

  private:
	/*
	 * I2C base required functions
	 */
	/** Start the I2C Device
	 *
	 *
	 */
	void start_() noexcept final;
	void stop_() noexcept final;
	embvm::i2c::pullups setPullups_(embvm::i2c::pullups pullups) noexcept final;
	embvm::i2c::baud baudrate_(embvm::i2c::baud baud) noexcept final;
	embvm::i2c::status transfer_(const embvm::i2c::op_t& op,
								 const embvm::i2c::master::cb_t& cb) noexcept final;
	void configure_(embvm::i2c::pullups pullup) noexcept final;
	void configure_i2c_pins_() noexcept;
	void configureDMA() noexcept;

  private:
	const STM32I2CMaster::device device_;
	STM32DMA& tx_channel_;
	STM32DMA& rx_channel_;
	// TODO: temporary, refactor out
	bool transfer_completed_ = false;
	// TODO: ?
	//embvm::i2c::master::cb_t active_cb_{nullptr};
	embvm::i2c::op_t active_op_{};
};

#endif // STM32_I2C_HPP_
