#ifndef STM32_DMA_HPP_
#define STM32_DMA_HPP_

#include <driver/driver.hpp>

// TODO: document requirement to enable the DMA clock in the hardware platform, since
// we can have multiple channels configured. That means we can't just start/stop DMA.

// TODO: template, make compile-time configuration?
// Would require a forwarding helper class
class STM32DMA final : public embvm::DriverBase
{
  public:
	enum device : uint8_t
	{
		dma1 = 0,
		dma2,
		MAX_DMA
	};

	enum channel : uint8_t
	{
		CH0 = 0,
		CH1,
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

  private:
	// Driver base functions
	void start_() noexcept final;
	void stop_() noexcept final;

  private:
	const device device_;
	const channel channel_;
};

#endif // STM32_DMA_HPP_
