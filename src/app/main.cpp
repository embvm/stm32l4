/*
 * Copyright © 2020 Embedded Artistry LLC.
 * See LICENSE file for licensing information.
 */

#include <platform.hpp>

volatile bool abort_program_ = false;

int main()
{
	printf("Blinky application booted!\n");

	auto& platform = VirtualPlatform::inst();

	printf("Starting blink\n");
	platform.startBlink();

	while(!abort_program_)
	{
		// spin
		// embvm::this_thread::sleep_for(std::chrono::milliseconds(250));
	}

	return 0;
}
