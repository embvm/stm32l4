/*
 * Copyright © 2020 Embedded Artistry LLC.
 * See LICENSE file for licensing information.
 */

#ifndef NUCLEO_L4RZI_DEMO_PLATFORM_HPP_
#define NUCLEO_L4RZI_DEMO_PLATFORM_HPP_

#include <NucleoL4RZI_HWPlatform.hpp>
#include <boot/boot_sequencer.hpp>
#include <platform/virtual_platform.hpp>
#include <platform_options.hpp>

/// Signal variable to exit the main() loop
/// Declared in main.cpp
extern volatile bool abort_program_;

class NucleoL4RZI_DemoPlatform final
	: public embvm::VirtualPlatformBase<NucleoL4RZI_DemoPlatform, PlatformDriverRegistry>
{
	using PlatformBase =
		embvm::VirtualPlatformBase<NucleoL4RZI_DemoPlatform, PlatformDriverRegistry>;

  public:
	// APIs to required by base class
	static void earlyInitHook_() noexcept;
	static void initOS_() noexcept;
	void init_() noexcept;
	void initProcessor_() noexcept;
	void initHWPlatform_() noexcept;

	// Platform APIs
	void startBlink() noexcept;

	// Constructor/destructor
	NucleoL4RZI_DemoPlatform() noexcept {}
	~NucleoL4RZI_DemoPlatform() noexcept = default;

  private:
	NucleoL4R5ZI_HWPlatform hw_platform_;
};

using VirtualPlatform = NucleoL4RZI_DemoPlatform;
using PlatformBootSequencer = embvm::BootSequencer<embvm::DefaultBootStrategy<VirtualPlatform>>;

#endif // NUCLEO_L4RZI_DEMO_PLATFORM_HPP_
