// Copyright 2020 Embedded Artistry LLC
// SPDX-License-Identifier: MIT

#ifndef STM32L4R5_PROCESSOR_HPP_
#define STM32L4R5_PROCESSOR_HPP_

#include <cstdint>
#include <processor/virtual_processor.hpp>

class stm32l4r5 : public embvm::VirtualProcessorBase<stm32l4r5>
{
	using ProcessorBase = embvm::VirtualProcessorBase<stm32l4r5>;

  public:
	/// @brief Default constructor.
	stm32l4r5() noexcept = default;

	/// @brief Default destructor.
	~stm32l4r5();

	// Required Functions:

	static void earlyInitHook_() noexcept;
	void init_() noexcept;
	void reset_() noexcept;

#pragma mark - Custom Functions -
};

#endif // STM32L4R5_PROCESSOR_HPP_
