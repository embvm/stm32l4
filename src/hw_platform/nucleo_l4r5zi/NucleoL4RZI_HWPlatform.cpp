#include "NucleoL4RZI_HWPlatform.hpp"

NucleoL4R5ZI_HWPlatform::NucleoL4R5ZI_HWPlatform() noexcept : HWPlatformBase("NUCLEO-L4R5ZI Development Board")
{
	registerDriver("led1", &led1);
	registerDriver("led3", &led2);
	registerDriver("led2", &led3);
}

NucleoL4R5ZI_HWPlatform::~NucleoL4R5ZI_HWPlatform() noexcept {}

void NucleoL4R5ZI_HWPlatform::earlyInitHook_() noexcept {}

void NucleoL4R5ZI_HWPlatform::init_() noexcept
{
	// start all LEDs
	// turn them off? Or just trust that they start off?
	led1.start();
	led2.start();
	led3.start();
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
	led2.on();
	led3.on();
}
