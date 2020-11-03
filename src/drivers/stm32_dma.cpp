// Copyright 2020 Embedded Artistry LLC
// SPDX-License-Identifier: MIT

#include "stm32_dma.hpp"
#include <array>
#include <cassert>
#include <nvic.hpp>
#include <processor_includes.hpp>
#include <stm32l4xx_ll_dma.h>
#include <volatile/volatile.hpp>

#pragma mark - Variables -

constexpr std::array<DMA_TypeDef* const, STM32DMA::device::MAX_DMA> dma_devices = {DMA1, DMA2};
constexpr std::array<uint32_t const, STM32DMA::channel::MAX_CH> transfer_complete_flags = {
	DMA_ISR_TCIF1, DMA_ISR_TCIF2, DMA_ISR_TCIF3, DMA_ISR_TCIF4,
	DMA_ISR_TCIF5, DMA_ISR_TCIF6, DMA_ISR_TCIF7};

constexpr std::array<uint32_t const, STM32DMA::channel::MAX_CH> transfer_error_flags = {
	DMA_ISR_TEIF1, DMA_ISR_TEIF2, DMA_ISR_TEIF3, DMA_ISR_TEIF4,
	DMA_ISR_TEIF5, DMA_ISR_TEIF6, DMA_ISR_TEIF7};

constexpr std::array<uint32_t const, STM32DMA::channel::MAX_CH> clear_transfer_general_flags = {
	DMA_ISR_GIF1, DMA_ISR_GIF2, DMA_ISR_GIF3, DMA_ISR_GIF4,
	DMA_ISR_GIF5, DMA_ISR_GIF6, DMA_ISR_GIF7};

constexpr std::array<uint32_t const, STM32DMA::channel::MAX_CH> clear_transfer_complete_flags = {
	DMA_IFCR_CTCIF1, DMA_IFCR_CTCIF2, DMA_IFCR_CTCIF3, DMA_IFCR_CTCIF4,
	DMA_IFCR_CTCIF5, DMA_IFCR_CTCIF6, DMA_IFCR_CTCIF7};

constexpr std::array<std::array<IRQn_Type, STM32DMA::channel::MAX_CH>, STM32DMA::device::MAX_DMA>
	irq_num = {
		std::array<IRQn_Type, STM32DMA::channel::MAX_CH>{
			DMA1_Channel1_IRQn, DMA1_Channel2_IRQn, DMA1_Channel3_IRQn, DMA1_Channel4_IRQn,
			DMA1_Channel5_IRQn, DMA1_Channel6_IRQn, DMA1_Channel7_IRQn},
		std::array<IRQn_Type, STM32DMA::channel::MAX_CH>{
			DMA2_Channel1_IRQn, DMA2_Channel2_IRQn, DMA2_Channel3_IRQn, DMA2_Channel4_IRQn,
			DMA2_Channel5_IRQn, DMA2_Channel6_IRQn, DMA2_Channel7_IRQn},
};

static std::array<std::array<STM32DMA::cb_t, STM32DMA::channel::MAX_CH>, STM32DMA::device::MAX_DMA>
	irq_handlers = {nullptr};

#pragma mark - Helper Functions -

static inline bool check_dma_complete_flag(STM32DMA::device dev, STM32DMA::channel ch)
{
	return READ_BIT(dma_devices[dev]->ISR, transfer_complete_flags[ch]);
}

static inline bool check_dma_error_flag(STM32DMA::device dev, STM32DMA::channel ch)
{
	return READ_BIT(dma_devices[dev]->ISR, transfer_error_flags[ch]);
}

static inline void clear_dma_general_int_flag(STM32DMA::device dev, STM32DMA::channel ch)
{
	embutil::volatile_store(&dma_devices[dev]->IFCR, clear_transfer_general_flags[ch]);
}

static inline void clear_transfer_complete_flag(STM32DMA::device dev, STM32DMA::channel ch)
{
	embutil::volatile_store(&dma_devices[dev]->IFCR, clear_transfer_complete_flags[ch]);
}

#pragma mark - Interrupt Handling -

extern "C" void DMA1_Channel1_IRQHandler();
extern "C" void DMA1_Channel2_IRQHandler();
extern "C" void DMA1_Channel3_IRQHandler();
extern "C" void DMA1_Channel4_IRQHandler();
extern "C" void DMA1_Channel5_IRQHandler();
extern "C" void DMA1_Channel6_IRQHandler();
extern "C" void DMA1_Channel7_IRQHandler();
extern "C" void DMA2_Channel1_IRQHandler();
extern "C" void DMA2_Channel2_IRQHandler();
extern "C" void DMA2_Channel3_IRQHandler();
extern "C" void DMA2_Channel4_IRQHandler();
extern "C" void DMA2_Channel5_IRQHandler();
extern "C" void DMA2_Channel6_IRQHandler();
extern "C" void DMA2_Channel7_IRQHandler();

// TODO: ?
// DMA2D_IRQHandler
// DMAMUX1_OVR_IRQHandler

// TODO: bottom half handler or dispatch
static void dma_handler(STM32DMA::device dev, STM32DMA::channel ch)
{
	auto handler = irq_handlers[dev][ch];
	STM32DMA::status status;
	assert(handler); // check to see if a valid handler is registered

	if(check_dma_complete_flag(dev, ch))
	{
		clear_transfer_complete_flag(dev, ch);
		status = STM32DMA::status::ok;
	}
	else if(check_dma_error_flag(dev, ch))
	{
		status = STM32DMA::status::error;
	}
	else
	{
		assert(0); // Case not handled!
	}

	clear_dma_general_int_flag(dev, ch);
	handler(status);
}

void DMA1_Channel1_IRQHandler()
{
	dma_handler(STM32DMA::device::dma1, STM32DMA::channel::CH1);
}

void DMA1_Channel2_IRQHandler()
{
	dma_handler(STM32DMA::device::dma1, STM32DMA::channel::CH2);
}

void DMA1_Channel3_IRQHandler()
{
	dma_handler(STM32DMA::device::dma1, STM32DMA::channel::CH3);
}

void DMA1_Channel4_IRQHandler()
{
	dma_handler(STM32DMA::device::dma1, STM32DMA::channel::CH4);
}

void DMA1_Channel5_IRQHandler()
{
	dma_handler(STM32DMA::device::dma1, STM32DMA::channel::CH5);
}

void DMA1_Channel6_IRQHandler()
{
	dma_handler(STM32DMA::device::dma1, STM32DMA::channel::CH6);
}

void DMA1_Channel7_IRQHandler()
{
	dma_handler(STM32DMA::device::dma1, STM32DMA::channel::CH7);
}

void DMA2_Channel1_IRQHandler()
{
	dma_handler(STM32DMA::device::dma2, STM32DMA::channel::CH1);
}

void DMA2_Channel2_IRQHandler()
{
	dma_handler(STM32DMA::device::dma2, STM32DMA::channel::CH2);
}

void DMA2_Channel3_IRQHandler()
{
	dma_handler(STM32DMA::device::dma2, STM32DMA::channel::CH3);
}

void DMA2_Channel4_IRQHandler()
{
	dma_handler(STM32DMA::device::dma2, STM32DMA::channel::CH4);
}

void DMA2_Channel5_IRQHandler()
{
	dma_handler(STM32DMA::device::dma2, STM32DMA::channel::CH5);
}

void DMA2_Channel6_IRQHandler()
{
	dma_handler(STM32DMA::device::dma2, STM32DMA::channel::CH6);
}

void DMA2_Channel7_IRQHandler()
{
	dma_handler(STM32DMA::device::dma2, STM32DMA::channel::CH7);
}

#pragma mark - Driver -

void STM32DMA::setAddresses(void* source_address, void* dest_address, size_t transfer_size) noexcept
{
	auto inst = dma_devices[device_];
	assert(inst); // Instance invalid

	// This function can only be called if channel disabled
	assert(LL_DMA_IsEnabledChannel(inst, channel_) == false);
	assert(source_address && dest_address && transfer_size); // need non-nullptr and non-zero data

	LL_DMA_ConfigAddresses(inst, channel_, reinterpret_cast<uint32_t>(source_address),
						   reinterpret_cast<uint32_t>(dest_address),
						   LL_DMA_GetDataTransferDirection(inst, channel_));
	LL_DMA_SetDataLength(inst, channel_, transfer_size);
}

void STM32DMA::enable() noexcept
{
	auto inst = dma_devices[device_];
	assert(inst); // Check for invalid device instance
	assert(LL_DMA_IsEnabledChannel(inst, channel_) == false);

	LL_DMA_EnableChannel(inst, channel_);
}

void STM32DMA::disable() noexcept
{
	auto inst = dma_devices[device_];
	assert(inst); // Check for invalid device instance

	LL_DMA_DisableChannel(inst, channel_);
}

void STM32DMA::start_() noexcept
{
	auto inst = dma_devices[device_];
	assert(inst && configuration_);

	LL_DMA_ConfigTransfer(inst, channel_, configuration_);
	LL_DMA_SetPeriphRequest(inst, channel_, mux_request_);

	enableInterrupts();
}

void STM32DMA::stop_() noexcept
{
	auto inst = dma_devices[device_];
	assert(inst); // Check for invalid device instance

	disableInterrupts();
	LL_DMA_DisableChannel(inst, channel_); // TODO: does this need to be here, or elsewhere?
}

void STM32DMA::enableInterrupts() noexcept
{
	auto irq = irq_num[device_][channel_];
	assert(irq); // Check that channel is supported
	NVICControl::priority(irq, 4); // TODO: how to configure priority for the driver?
	NVICControl::enable(irq);

	// Enable complete/error interrupts
	LL_DMA_EnableIT_TC(dma_devices[device_], channel_);
	LL_DMA_EnableIT_TE(dma_devices[device_], channel_);
}

void STM32DMA::disableInterrupts() noexcept
{
	auto irq = irq_num[device_][channel_];
	assert(irq); // Check that channel is supported
	NVICControl::disable(irq);

	// Disable complete/error interrupts
	// Enable complete/error interrupts
	LL_DMA_DisableIT_TC(dma_devices[device_], channel_);
	LL_DMA_DisableIT_TE(dma_devices[device_], channel_);
}

void STM32DMA::registerCallback(const STM32DMA::cb_t& cb) noexcept
{
	irq_handlers[device_][channel_] = cb;
}

void STM32DMA::registerCallback(STM32DMA::cb_t&& cb) noexcept
{
	irq_handlers[device_][channel_] = std::move(cb);
}
