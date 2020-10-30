#include "stm32_dma.hpp"
#include <array>
#include <cassert>
#include <nvic.hpp>
#include <stm32l4xx_ll_dma.h>

#pragma mark - Variables -

static std::array<DMA_TypeDef* const, STM32DMA::device::MAX_DMA> dma_devices = {DMA1, DMA2};
static std::array<uint32_t const, STM32DMA::channel::MAX_CH> transfer_complete_flags = {
	DMA_ISR_TCIF1, DMA_ISR_TCIF2, DMA_ISR_TCIF3, DMA_ISR_TCIF4,
	DMA_ISR_TCIF5, DMA_ISR_TCIF6, DMA_ISR_TCIF7};

static std::array<uint32_t const, STM32DMA::channel::MAX_CH> transfer_error_flags = {
	DMA_ISR_TEIF1, DMA_ISR_TEIF2, DMA_ISR_TEIF3, DMA_ISR_TEIF4,
	DMA_ISR_TEIF5, DMA_ISR_TEIF6, DMA_ISR_TEIF7};

constexpr std::array<std::array<unsigned, STM32DMA::channel::MAX_CH>, 2> irq_num = {
	std::array<unsigned, STM32DMA::channel::MAX_CH>{
		DMA1_Channel1_IRQn, DMA1_Channel2_IRQn, DMA1_Channel3_IRQn, DMA1_Channel4_IRQn,
		DMA1_Channel5_IRQn, DMA1_Channel6_IRQn, DMA1_Channel7_IRQn},
	std::array<unsigned, STM32DMA::channel::MAX_CH>{
		DMA2_Channel1_IRQn, DMA2_Channel2_IRQn, DMA2_Channel3_IRQn, DMA2_Channel4_IRQn,
		DMA2_Channel5_IRQn, DMA2_Channel6_IRQn, DMA2_Channel7_IRQn},
};

#pragma mark - Helper Functions -

static inline bool check_dma_complete_flag(STM32DMA::device dev, STM32DMA::channel ch)
{
	return READ_BIT(dma_devices[dev]->ISR, transfer_complete_flags[ch]);
}

static inline bool check_dma_error_flag(STM32DMA::device dev, STM32DMA::channel ch)
{
	return READ_BIT(dma_devices[dev]->ISR, transfer_error_flags[ch]);
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

static void dma_handler(STM32DMA::device dev, STM32DMA::channel ch)
{
	assert(0); // TODO

	if(check_dma_complete_flag(dev, ch))
	{
		// TODO: complete callback
	}
	else if(check_dma_error_flag(dev, ch))
	{
		// TODO: error callback
	}
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

void STM32DMA::start_() noexcept
{
	enableInterrupts();

#if 0
/**
  * @brief  This function configures the DMA Channels for I2C3(TXDR) and I2C3(RXDR).
  * @note   This function is used to :
  *         -1- Enable the clock of DMA1.
  *         -2- Configure NVIC for DMA1_Channel2 and DMA1_Channel3.
  *         -3- Configure the DMA functional parameters for Master Transmit.
  *         -4- Configure the DMA functional parameters for Master Receive.
  *         -5- Enable DMA1 interrupts complete/error.
  * @param   None
  * @retval  None
  */
void Configure_DMA(void)
{
  /* (3) Configure the DMA functional parameters for Master Transmit */
  LL_DMA_ConfigTransfer(DMA1, LL_DMA_CHANNEL_2, LL_DMA_DIRECTION_MEMORY_TO_PERIPH | \
                                                LL_DMA_PRIORITY_HIGH              | \
                                                LL_DMA_MODE_NORMAL                | \
                                                LL_DMA_PERIPH_NOINCREMENT         | \
                                                LL_DMA_MEMORY_INCREMENT           | \
                                                LL_DMA_PDATAALIGN_BYTE            | \
                                                LL_DMA_MDATAALIGN_BYTE);
  LL_DMA_ConfigAddresses(DMA1, LL_DMA_CHANNEL_2, (uint32_t)(*pMasterTransmitBuffer), (uint32_t)LL_I2C_DMA_GetRegAddr(I2C3, LL_I2C_DMA_REG_DATA_TRANSMIT), LL_DMA_GetDataTransferDirection(DMA1, LL_DMA_CHANNEL_2));
  LL_DMA_SetPeriphRequest(DMA1, LL_DMA_CHANNEL_2, LL_DMA_REQUEST_3);

  /* (4) Configure the DMA functional parameters for Master Receive */
  LL_DMA_ConfigTransfer(DMA1, LL_DMA_CHANNEL_3, LL_DMA_DIRECTION_PERIPH_TO_MEMORY | \
                                                LL_DMA_PRIORITY_HIGH              | \
                                                LL_DMA_MODE_NORMAL                | \
                                                LL_DMA_PERIPH_NOINCREMENT         | \
                                                LL_DMA_MEMORY_INCREMENT           | \
                                                LL_DMA_PDATAALIGN_BYTE            | \
                                                LL_DMA_MDATAALIGN_BYTE);
  LL_DMA_ConfigAddresses(DMA1, LL_DMA_CHANNEL_3, (uint32_t)LL_I2C_DMA_GetRegAddr(I2C3, LL_I2C_DMA_REG_DATA_RECEIVE), (uint32_t)&(aMasterReceiveBuffer), LL_DMA_GetDataTransferDirection(DMA1, LL_DMA_CHANNEL_3));
  LL_DMA_SetPeriphRequest(DMA1, LL_DMA_CHANNEL_3, LL_DMA_REQUEST_3);
}
#endif
}

void STM32DMA::stop_() noexcept
{
	disableInterrupts();
}

void STM32DMA::enableInterrupts() noexcept
{
	auto irq = irq_num[device_][channel_];
	assert(irq); // Check that channel is supported
	NVICControl::priority(irq, 4); // TODO: how to configure priority for the driver?
	NVICControl::enable(irq);

	// TODO:
#if 0
  /* (5) Enable DMA1 interrupts complete/error */
  LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_2);
  LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_2);
  LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_3);
  LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_3);
#endif
}

void STM32DMA::disableInterrupts() noexcept
{
	auto irq = irq_num[device_][channel_];
	assert(irq); // Check that channel is supported
	NVICControl::disable(irq);

	// TODO: - disable
#if 0
  /* (5) Enable DMA1 interrupts complete/error */
  LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_2);
  LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_2);
  LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_3);
  LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_3);
#endif
}
