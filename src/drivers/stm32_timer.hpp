#ifndef STM32_TIMER_HPP_
#define STM32_TIMER_HPP_

#include <driver/timer.hpp>
//#include <driver/hal_driver.hpp>

// TODO: maybe this driver just needs to be set to a "Capture-Compare driver", indicating that
// a user can write other drivers?
// TODO: do we need to separate the concepts for timer device (TIM1...) and channel (Channel 1...6)?
// TODO: does it make more sense for timer class to store period or frequency?
// TODO: does this need to derive from HAL base?
// Threading is not supported, so that causes an error...
// TODO: support multiple callbacks? Use templates for that?
// TODO: how to set interrupt priority? look at nrf52 I2C driver - takes it as a constructor param
// Does that need to be part of the HAL base?
// Does the user need to manually dot hat in the HW platform?
// Is that just a processor-level API somehow?
// TODO: we can convert this to being compile-time setting of the period/clock,
// which we can use for constexpr calculations. We'll need a helper class probably.

/** STM32 Timer Driver Implementation
 *
 * Timer channel number match those for the internal STM32 device IDs. That is,
 * embvm::timer::channel::CH1 corresponds to TIM1. Since the Embedded VM channel counters
 * start at 0, embvm::timer::channel::CH0 is invalid. Using this channel will result
 * in a program assertion being triggered.
 *
 * @see embvm::Timer
 * @see embvm::HALDriverBase
 */
class STM32Timer final : public embvm::timer::Timer //, public embvm::HALDriverBase
{
  public:
	/** Construct an STM32 Timer Object
	 *
	 * @param [in] ch Timer hardware device to use.
	 * 	Timer channel number match those for the internal STM32 device IDs. That is,
	 * 	embvm::timer::channel::CH1 corresponds to TIM1. Since the Embedded VM channel counters
	 * 	start at 0, embvm::timer::channel::CH0 is invalid. Using this channel will result
	 * 	in a program assertion being triggered.
	 */
	explicit STM32Timer(embvm::timer::channel ch) noexcept : channel_(ch) {}

	/** Construct an STM32 Timer Object with a stated period
	 *
	 * @param [in] ch Timer hardware device to use.
	 * 	Timer channel number match those for the internal STM32 device IDs. That is,
	 * 	embvm::timer::channel::CH1 corresponds to TIM1. Since the Embedded VM channel counters
	 * 	start at 0, embvm::timer::channel::CH0 is invalid. Using this channel will result
	 * 	in a program assertion being triggered.
	 * @param [in] p The timer period to use when starting the timer driver.
	 */
	explicit STM32Timer(embvm::timer::channel ch, embvm::timer::timer_period_t p) noexcept
		: channel_(ch)
	{
		period(p);
	}

	~STM32Timer() noexcept = default;

	/*
	 * Timer base class required interfaces
	 */
	void registerCallback(const embvm::timer::cb_t& cb) noexcept final;
	void registerCallback(embvm::timer::cb_t&& cb) noexcept final;
	embvm::timer::timer_period_t count() const noexcept final;

	/*
	 * HAL base class required interfaces
	 */
	void enableInterrupts() noexcept /*final*/;
	void disableInterrupts() noexcept /*final*/;

  private:
	/// Start the timer
	/// @precondition The timer is stopped
	/// @postcondition The RCC clock to the timer is enabled
	/// @postcondition The timer peripheral is configured using the stored period_
	/// 	and enabled.
	/// @postcondition Timer channel interrupt is enabled.
	void start_() noexcept final;

	/// Stop the timer
	/// @precondition The timer is started
	/// @postcondition The timer RCC clock is disabled
	/// @postcondition The timer peripheral is disabled
	/// @postcondition Timer channel interrupt is disabled
	void stop_() noexcept final;

  private:
	const embvm::timer::channel channel_;
};

#endif // STM32_TIMER_HPP_
