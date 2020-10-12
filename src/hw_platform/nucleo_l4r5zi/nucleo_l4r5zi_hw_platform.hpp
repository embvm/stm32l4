#ifndef NUCLEO_L4R5ZI_HW_PLATFORM_HPP_
#define NUCLEO_L4R5ZI_HW_PLATFORM_HPP_

#include <driver/led.hpp>
#include <hw_platform/virtual_hw_platform.hpp>
#include <stm32l4r5.hpp>

class NucleoL4R5ZI_HWPlatform : public embvm::VirtualHwPlatformBase<NucleoL4R5ZI_HWPlatform>
{
	using HWPlatformBase = embvm::VirtualHwPlatformBase<NucleoL4R5ZI_HWPlatform>;

  public:
	/// @brief Default constructor.
	NucleoL4R5ZI_HWPlatform() noexcept : PlatformBase("NUCLEO-L4R5ZI Development Board") {}

	/// @brief Default destructor.
	~NucleoL4R5ZI_HWPlatform() noexcept;

	// Required functions
	static void earlyInitHook_() noexcept;
	void init_() noexcept;
	void initProcessor_() noexcept;
	void soft_reset_() noexcept;
	void hard_reset_() noexcept;

	// Public APIs
	void leds_off() noexcept;
	void startBlink() noexcept;

  private:
	stm32l4r5 processor_;

	//nRFGPIOOutput<0, 13> led1_pin{};
	//nRFGPIOOutput<0, 14> led2_pin{};
	//nRFGPIOOutput<0, 15> led3_pin{};

	//embvm::led::gpioActiveLow led1{led1_pin, "led1"};
	//embvm::led::gpioActiveLow led2{led2_pin, "led2"};
	//embvm::led::gpioActiveLow led3{led3_pin, "led3"};
};

#endif // NUCLEO_L4R5ZI_HW_PLATFORM_HPP_
