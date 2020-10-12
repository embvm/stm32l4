#include "nucleo_l4r5zi_hw_platform.hpp"

NucleoL4R5ZI_HWPlatform::~NucleoL4R5ZI_HWPlatform() noexcept {}

void NucleoL4R5ZI_HWPlatform::earlyInitHook_() noexcept {}

void NucleoL4R5ZI_HWPlatform::init_() noexcept {}

void NucleoL4R5ZI_HWPlatform::leds_off() noexcept {}

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
