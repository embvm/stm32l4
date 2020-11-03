// Copyright 2020 Embedded Artistry LLC
// SPDX-License-Identifier: MIT

#include "stm32l4r5.hpp"
#include <processor_architecture.hpp>
#include <processor_includes.hpp>

#include <nvic.hpp> // for assert

// TODO: figure out why assert isn't working on arm
// TODO: Add disableInterrupts() and enableInterrupts() APIs to
// the base class, and move this implementation to the platform layer.
void __assert_fail(const char* expr, const char* file,

				   unsigned int line, const char* function)
{
	(void)expr;
	(void)file;
	(void)line;
	(void)function;
	while(1)
		;
	NVICControl::disableInterrupts();
}

#pragma mark - Definitions -

#pragma mark - Helpers -

#pragma mark - Interface Functions -

stm32l4r5::~stm32l4r5() {}

void stm32l4r5::earlyInitHook_() noexcept {}

void stm32l4r5::init_() noexcept {}

void stm32l4r5::reset_() noexcept
{
	ProcessorArch::systemReset();
}
