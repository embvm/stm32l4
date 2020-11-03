#include "NucleoL4R5ZI_HWPlatform.hpp"
#include <stm32_rcc.hpp>

// TODO: shoudl I check/increae clock speed?

NucleoL4R5ZI_HWPlatform::NucleoL4R5ZI_HWPlatform() noexcept
{
	registerDriver("led1", &led1);
	registerDriver("led2", &led2);
	registerDriver("led3", &led3);
	registerDriver("timer0", &timer0);
}

NucleoL4R5ZI_HWPlatform::~NucleoL4R5ZI_HWPlatform() noexcept {}

void NucleoL4R5ZI_HWPlatform::earlyInitHook_() noexcept {}

void NucleoL4R5ZI_HWPlatform::init_() noexcept
{
	// We need to turn on the GPIO clocks before we start the gpio/led drivers.
	STM32ClockControl::gpioEnable(embvm::gpio::port::B);
	STM32ClockControl::gpioEnable(embvm::gpio::port::C);
	STM32ClockControl::gpioEnable(embvm::gpio::port::F);

	// We need to turn on the DMA clocks before we start/stop the dma drivers
	STM32ClockControl::dmaEnable(STM32DMA::device::dma1);
	STM32ClockControl::dmaMuxEnable();

	// start all LEDs
	// turn them off? Or just trust that they start off?
	led1.start();
	led2.start();
	led3.start();

	timer0.registerCallback([this]() noexcept {
		led1.toggle();
		led2.toggle();
		led3.toggle();
	});

	i2c2.start();
}

void NucleoL4R5ZI_HWPlatform::leds_off() noexcept
{
	led1.off();
	led2.off();
	led3.off();
}

void NucleoL4R5ZI_HWPlatform::hard_reset_() noexcept
{
	// We cannot perform a hard reset from software, so perform
	// a soft reset instead.
	soft_reset_();
}

void NucleoL4R5ZI_HWPlatform::soft_reset_() noexcept
{
	processor_.reset();
}

void NucleoL4R5ZI_HWPlatform::initProcessor_() noexcept
{
	processor_.init();
}

void NucleoL4R5ZI_HWPlatform::startBlink() noexcept
{
	led1.on();
	led2.off();
	led3.on();

	timer0.start();
}
