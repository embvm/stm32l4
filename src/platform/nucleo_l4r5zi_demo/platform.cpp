/*
 * Copyright © 2020 Embedded Artistry LLC.
 * See LICENSE file for licensing information.
 */

#include "platform.hpp"
#include <printf.h> // for _putchar definition
// TODO: test memory setup with linker scripts
//#include <malloc.h>

// extern int __HeapBase;
// extern int __HeapLimit;

void _putchar(char c)
{
	(void)c;
	// TODO: implement
}

namespace
{
// TODO: FreeRTOS threaded support
// static constexpr size_t MAIN_THREAD_STACK_SIZE = 4096; // bytes
// static constexpr size_t LED_THREAD_STACK_SIZE = 2048;
// static embvm::VirtualThread* main_thread_ = nullptr;
} // namespace

void NucleoL4RZI_DemoPlatform::earlyInitHook_() noexcept
{
	// malloc_addblock(&__HeapBase, reinterpret_cast<uintptr_t>(&__HeapLimit) -
	//								 reinterpret_cast<uintptr_t>(&__HeapBase));

	NucleoL4R5ZI_HWPlatform::earlyInitHook();
}

void NucleoL4RZI_DemoPlatform::initOS_() noexcept
{
	// TODO: freertos threaded support
}

void NucleoL4RZI_DemoPlatform::initHWPlatform_() noexcept
{
	hw_platform_.init();
}

void NucleoL4RZI_DemoPlatform::initProcessor_() noexcept
{
	hw_platform_.initProcessor();
}

void NucleoL4RZI_DemoPlatform::init_() noexcept {}

void NucleoL4RZI_DemoPlatform::startBlink() noexcept
{
	hw_platform_.startBlink();
}

// TODO: freeRTOS threaded support
#if 0
void nRF52DK_FrameworkDemoPlatform::led_blink_thread_() noexcept
{
	static const auto led_list = hw_platform_.getLEDInstances();
	static const auto delay = std::chrono::milliseconds(500);

	while(1)
	{
		for(const auto& led : led_list)
		{
			led->toggle();
			embvm::this_thread::sleep_for(delay);
		}
	}
}
#endif
