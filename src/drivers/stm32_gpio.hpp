#ifndef STM32_GPIO_DRIVER_HPP_
#define STM32_GPIO_DRIVER_HPP_

#include "helpers/gpio_helper.hpp"
#include <cstdint>
#include <driver/gpio.hpp>

// TODO: support for special function pins
// TODO: support for slew rate
// TODO: support for internal pull up/down resistors

// TODO: is there a way to handle the translatoin between TPort and the actual STM32 pointer
// just once, instead of doing it every time we make a call?
// Constexpr function called during construction, somehow?
template<embvm::gpio::port TPort, uint8_t TPin>
class STM32GPIO final : public embvm::gpio::base
{
  public:
	/** Construct a generic GPIO output
	 */
	STM32GPIO() noexcept : mode_(embvm::gpio::mode::input) {}

	/// Construct a GPIO with a mode
	/// @param [in] mode The desired mode to initialize the GPIO to when
	/// 	starting the pin.
	explicit STM32GPIO(embvm::gpio::mode mode) noexcept : mode_(mode) {}

	/// Default destructor
	~STM32GPIO() = default;

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

	inline void toggle() noexcept final
	{
		STM32GPIOTranslator::toggle(TPort, TPin);
	}

	inline bool get() noexcept final
	{
		return STM32GPIOTranslator::get(TPort, TPin);
	}

	void setMode(embvm::gpio::mode m) noexcept final
	{
		switch(m)
		{
			case embvm::gpio::mode::input:

				break;
			case embvm::gpio::mode::output:
				STM32GPIOTranslator::configure_output(TPort, TPin);
				break;
			case embvm::gpio::mode::special:
				// Currently unsupported mode
			case embvm::gpio::mode::MAX_MODE:
			default:
				assert(false);
		}

		mode_ = m;
	}

	inline embvm::gpio::mode mode() noexcept final
	{
		return mode_;
	}

  private:
	inline void start_() noexcept final
	{
		setMode(mode_);
	}

	inline void stop_() noexcept final
	{
		STM32GPIOTranslator::configure_default(TPort, TPin);
	}

  private:
	embvm::gpio::mode mode_;
};

#endif // STM32_GPIO_DRIVER_HPP_
