#ifndef STM32_DMA_HPP_
#define STM32_DMA_HPP_

#include <cassert>
#include <driver/driver.hpp>
#include <inplace_function/inplace_function.hpp>

// TODO: document requirement to enable the DMA clock in the hardware platform, since
// we can have multiple channels configured. That means we can't just start/stop DMA.
// TODO: need to take in priority for interrupt for the channel
// TODO: I can replace the constant table lookups in every function with a single lookup,
// preferably in the constructor, where the device pointer is stored in a void* in the class
// 	(probably better than device_ itself?)
// Then I can perform a cast instead of a lookup for each function call.

// TODO: workflow decision
#if 0
Do we require users to start() and stop() the driver to change addresses and to kick off a transfer?
or do we also have an enable() and disable() that can be used, where users can still
change hte memory addresS (LL_DMA_SetMemoryAddress) and data length (LL_DMA_SetDataLength) before
enabling and disabling the channel?
	I guess the real question is: is there additional overhead with start/stop vs enable/disable APIs?
#endif

// TODO: template, make compile-time configuration?
// Would require a forwarding helper class
/** Object representing a single DMA channel
 *
 *
 * The initial setup steps are:
 *	1. Set the configuration (@see SetConfiguration())
 *	2. Start the driver (@see start_)
 *
 * When you're ready to perform a transfer, you will:
 *
 *	1. Set the addresses for the transfer (@see setAddresses())
 *	2. Enable the DMA channel (@see enable())
 *	3. <Transer occurs in background>
 *	4. Disable the DMA channel when transfer is complete (@see disable())
 *
 * @code
 * tx_channel_.setAddresses(reinterpret_cast<uint32_t>(op.tx_buffer),
 *       reinterpret_cast<uint32_t>(LL_I2C_DMA_GetRegAddr(i2c_inst, LL_I2C_DMA_REG_DATA_TRANSMIT)));
 * tx_channel_.enable();
 * @endcode
 *
 * This driver does not enable the clock for the associated DMA device. You will
 * need to do that manually in the hardware platform.
 *
 * @code
 * // We need to turn on the DMA clocks before we start/stop the dma drivers
 * STM32ClockControl::dmaEnable(STM32DMA::device::dma1);
 * @endcode
 *
 * @see STM32ClockControl
 */
class STM32DMA final : public embvm::DriverBase
{
  public:
	enum class status
	{
		ok = 0,
		error
		// TODO: expand
	};

	using cb_t = stdext::inplace_function<void(STM32DMA::status)>;

	enum device : uint8_t
	{
		dma1 = 0,
		dma2,
		MAX_DMA
	};

	enum channel : uint8_t
	{
		CH1 = 0,
		CH2,
		CH3,
		CH4,
		CH5,
		CH6,
		CH7,
		MAX_CH
	};

	explicit STM32DMA(device d, channel ch) noexcept
		: embvm::DriverBase(embvm::DriverType::DMA), device_(d), channel_(ch)
	{
	}
	~STM32DMA() = default;

	void enableInterrupts() noexcept;
	void disableInterrupts() noexcept;

	/** Set the DMA source buffer address.
	 *
	 * This command changes the source and destination buffer addresses
	 * for this DMA channel.
	 *
	 * While this function sets the variables, the changese can only be applied
	 * when the driver is disabled. So you will need to `disable()`, `setAddresses()`, and
	 * `enable()` in order for a change to taek full effect.
	 *
	 * @precondition The DMA device is disabled.
	 * @precondition source_address and dest_address are non-nullptr
	 * @precondition transfer_size > 0
	 *
	 * @param [in] source_address The memory address to copy data from.
	 * @param [in] dest_address The memory address to copy data to.
	 * @param [in] transfer_size The size of the buffer/transfer.
	 */
	void setAddresses(void* source_address, void* dest_address, size_t transfer_size) noexcept;

	// TODO: make API toggles for all of these items so you can do it programmatically?
	// Or even set from ctor?
	// TODO: programmatic mux selection?
	/** Set the raw configuration value
	 *
	 * If you prefer to set the configuration option using STM32 flags,
	 * you can do it in one fell swoop using this API.
	 *
	 * @code
	 * dma2_2.setConfiguration(LL_DMA_DIRECTION_MEMORY_TO_PERIPH |
	 *							LL_DMA_PRIORITY_HIGH              |
	 *							LL_DMA_MODE_NORMAL                |
	 *							LL_DMA_PERIPH_NOINCREMENT         |
	 *							LL_DMA_MEMORY_INCREMENT           |
	 *							LL_DMA_PDATAALIGN_BYTE            |
	 *							LL_DMA_MDATAALIGN_BYTE,
	 *							LL_DMA_REQUEST_3);
	 * @endcode
	 *
	 * @precondition The DMA device is stopped.
	 *
	 * @param [in] raw_value The raw 32-bit configuration value to write.
	 *	See the stm32l4xx_ll_dma.h header for options.
	 * @param pin] mux_request The mux configuration value to set.
	 */
	void setConfiguration(uint32_t configuration, uint32_t mux_request) noexcept
	{
		assert(started() == false);
		configuration_ = configuration;
		mux_request_ = mux_request;
	}

	/** Enable the DMA device for executing transfer.
	 *
	 * @precondition The DMA device is disabled.
	 * @precondition Valid addresses and transfer size have been supplied via setAddresses().
	 * @postcondition The DMA device is enabled.
	 */
	void enable() noexcept;

	/** Disable the DMA device for executing transfer.
	 *
	 * @postcondition The DMA device is disabled.
	 */
	void disable() noexcept;

	// TODO: document
	void registerCallback(const cb_t& cb) noexcept;
	void registerCallback(cb_t&& cb) noexcept;

  private:
	// Driver base functions
	void start_() noexcept final;
	void stop_() noexcept final;

  private:
	const device device_;
	const channel channel_;
	uint32_t* source_address_ = nullptr;
	uint32_t* dest_address_ = nullptr;
	size_t buffer_size_ = 0;
	/// Raw DMA configuration settings that are passed to the device.
	uint32_t configuration_;
	/// Peripheral Request (DMA Mux)
	uint32_t mux_request_;
};

#endif // STM32_DMA_HPP_
