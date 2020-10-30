#ifndef STM32_I2C_MASTER_HPP_
#define STM32_I2C_MASTER_HPP_

#include <driver/i2c.hpp>
#include <stm32_gpio.hpp>
// TODO: #include <driver/hal_driver.hpp>

// TODO: rename file to i2c master.hpp
// TODO: Handle interrupt priority - as a constructor parameter

class STM32I2CMaster final : public embvm::i2c::master
{
  public:
	enum device : uint8_t
	{
		i2c1 = 0,
		i2c2,
		i2c3,
		i2c4
	};

  public:
	STM32I2CMaster(STM32I2CMaster::device dev) noexcept : device_(dev) {}
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
	void configure_i2c_pins_();

  private:
	const STM32I2CMaster::device device_;
};

#endif // STM32_I2C_HPP_
