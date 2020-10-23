#ifndef STM32_GPIO_DRIVER_HPP_
#define STM32_GPIO_DRIVER_HPP_

#include "helpers/gpio_helper.hpp"
#include <cstdint>
#include <driver/gpio.hpp>

// TODO: is there a way to handle the translatoin between TPort and the actual STM32 pointer
// just once, instead of doing it every time we make a call?
// Constexpr function called during construction, somehow?
template<embvm::gpio::port TPort, uint8_t TPin>
class STM32GPIOOutput final : public embvm::gpio::output
{
  public:
	/** Construct a generic GPIO output
	 */
	STM32GPIOOutput() noexcept = default;

	/// Default destructor
	~STM32GPIOOutput() = default;

	inline void set(bool v) noexcept final
	{
		if(v)
		{
			STM32GPIOTranslator::set(TPort, TPin);
		}
		else
		{
			STM32GPIOTranslator::clear(TPort, TPin);
		}
	}

  private:
	inline void start_() noexcept final
	{
		STM32GPIOTranslator::configure_output(TPort, TPin);
	}

	inline void stop_() noexcept final
	{
		STM32GPIOTranslator::configure_default(TPort, TPin);
	}
};

#if 0
template<uint8_t TPort, uint8_t TPin>
class STM32GPIOInput final : public embvm::gpio::input
{
  public:
	/** Construct a generic GPIO input
	 */
	explicit STM32GPIOInput() : embvm::gpio::input("nRF GPIO Input") {}

	/** Construct a named GPIO input
	 *
	 * @param name The name of the GPIO pin
	 */
	explicit STM32GPIOInput(const char* name) : embvm::gpio::input(name) {}

	/// Default destructor
	~STM32GPIOInput() final = default;

	bool get() final;

  private:
	void start_() final;
	void stop_() final;
};
#endif

#endif // STM32_GPIO_DRIVER_HPP_
