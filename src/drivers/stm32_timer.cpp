#include "stm32_timer.hpp"
#include "stm32_rcc.hpp"
#include <array>
#include <nvic.hpp>
#include <stm32l4xx_ll_bus.h>
#include <stm32l4xx_ll_tim.h>
#include <volatile/volatile.hpp>

// TODO: decouple RCC from this class, handle instead in the hardware platform?

extern "C" void TIM1_CC_IRQHandler();
extern "C" void TIM2_IRQHandler();
extern "C" void TIM3_IRQHandler();
extern "C" void TIM4_IRQHandler();
extern "C" void TIM5_IRQHandler();
extern "C" void TIM6_IRQHandler();
extern "C" void TIM7_IRQHandler();
extern "C" void TIM8_CC_IRQHandler();

namespace
{
constexpr std::array<TIM_TypeDef* const, 9> timer_instance = {nullptr, TIM1, TIM2, TIM3, TIM4,
															  TIM5,	   TIM6, TIM7, TIM8};

static std::array<embvm::timer::cb_t, 9> tim_callbacks = {nullptr};

constexpr std::array<uint8_t, 9> irq_num = {
	0, // invalid for CH0
	TIM1_CC_IRQn, // TODO: Figure out proper use here
				  // Note that options are: TIM1_UP_TIM16_IRQN, TIM1_BRK_TIM15_IRQn,
				  // TIM1_TRG_COM_TIM17_IRQn, TIM1_CC_IRQn, LPTIM1_IRQn
	TIM2_IRQn, // Can also pick: LPTIM2_IRQn
	TIM3_IRQn, TIM4_IRQn, TIM5_IRQn,
	TIM6_IRQn, // Also: TIM6_DAC_IRQn
	TIM7_IRQn,
	TIM8_CC_IRQn, // TODO: Figure out proper use here
				  // Options are: TIM8_BRK_IRQn, TIM8_UP_IRQn, TIM8_TRG_COM_IRQn, TIM8_CC_IRQn
};
} // namespace

// TODO: should this be handled with a bottom half handler instead, using
// an interrupt queue?
static void timer_interrupt_handler(embvm::timer::channel ch)
{
	// TODO: do we need to check for the appropiate flags?
	// Right now we are just blanket-clearing
	volatile TIM_TypeDef* const reg = timer_instance[ch];
	embutil::volatile_store(&reg->SR, UINT32_C(0));

	if(tim_callbacks[ch])
	{
		tim_callbacks[ch]();
	}
}

extern "C" void TIM1_CC_IRQHandler()
{
	timer_interrupt_handler(embvm::timer::channel::CH1);
}

extern "C" void TIM2_IRQHandler()
{
	timer_interrupt_handler(embvm::timer::channel::CH2);
}

extern "C" void TIM3_IRQHandler()
{
	timer_interrupt_handler(embvm::timer::channel::CH3);
}

extern "C" void TIM4_IRQHandler()
{
	timer_interrupt_handler(embvm::timer::channel::CH4);
}

extern "C" void TIM5_IRQHandler()
{
	timer_interrupt_handler(embvm::timer::channel::CH5);
}

extern "C" void TIM6_IRQHandler()
{
	timer_interrupt_handler(embvm::timer::channel::CH6);
}

extern "C" void TIM7_IRQHandler()
{
	timer_interrupt_handler(embvm::timer::channel::CH7);
}

extern "C" void TIM8_CC_IRQHandler()
{
	timer_interrupt_handler(embvm::timer::channel::CH8);
}

/**
 * @file stm32_timer.cpp
 *
 * Implementation for Embedded VM timer driver interface using the STM32.
 *
 * The driver is currently implemented using Time base strategy for generating
 * internal timer events. We recommend using TIM6 or TIM7 for this purpose,
 * since they are simple timers without I/O channels.
 *
 * Implementation notes come from the STM32 Cube HAL and LL drivers.
 * In particular, stm32l4xx_hal_tim.c provides excellent instructions
 * for properly setting up the timer. If you need to expand this driver,
 * we recommend consulting the notes in the STM32 HAL and LL HAL files.
 */

// We have period_ for the period in microseconds
// state_ for the current state
// stopped
// expired
// armed
// config_ for the current config -
// oneshot
// periodic

#if 0
// TODO: how do we deterine the channel?
#define LL_TIM_CHANNEL_CH1 TIM_CCER_CC1E /*!< Timer input/output channel 1 */
#define LL_TIM_CHANNEL_CH1N TIM_CCER_CC1NE /*!< Timer complementary output channel 1 */
#define LL_TIM_CHANNEL_CH2 TIM_CCER_CC2E /*!< Timer input/output channel 2 */
#define LL_TIM_CHANNEL_CH2N TIM_CCER_CC2NE /*!< Timer complementary output channel 2 */
#define LL_TIM_CHANNEL_CH3 TIM_CCER_CC3E /*!< Timer input/output channel 3 */
#define LL_TIM_CHANNEL_CH3N TIM_CCER_CC3NE /*!< Timer complementary output channel 3 */
#define LL_TIM_CHANNEL_CH4 TIM_CCER_CC4E /*!< Timer input/output channel 4 */
#define LL_TIM_CHANNEL_CH5 TIM_CCER_CC5E /*!< Timer output channel 5 */
#define LL_TIM_CHANNEL_CH6 TIM_CCER_CC6E /*!< Timer output channel 6 */

#endif

embvm::timer::timer_period_t STM32Timer::count() const noexcept
{
	// TODO:
	assert(0);
	return embvm::timer::timer_period_t(0);
}

void STM32Timer::start_() noexcept
{
	STM32ClockControl::timerEnable(channel_);

	auto prescaler = __LL_TIM_CALC_PSC(SystemCoreClock, 1000000);

	LL_TIM_InitTypeDef initializer = {
		// Set clock to have a 1 microsecond period (1 MHz freq)
		.Prescaler = static_cast<uint16_t>(prescaler),
		.CounterMode = LL_TIM_COUNTERMODE_UP,
		.Autoreload = static_cast<uint32_t>(period_.count()), // TODO: correct?
		.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1, // TODO: correct?
		.RepetitionCounter = UINT8_C(0),
	};

	auto r = LL_TIM_Init(timer_instance[channel_], &initializer);
	assert(r == 0);

	/* Enable TIM2_ARR register preload. Writing to or reading from the         */
	/* auto-reload register accesses the preload register. The content of the   */
	/* preload register are transferred into the shadow register at each update */
	/* event (UEV).                                                             */
	LL_TIM_EnableARRPreload(timer_instance[channel_]);

	/* Enable TIM2_CCR1 register preload. Read/Write operations access the      */
	/* preload register. TIM2_CCR1 preload value is loaded in the active        */
	/* at each update event.                                                    */
	LL_TIM_OC_EnablePreload(timer_instance[channel_], LL_TIM_CHANNEL_CH1);
	// TODO: how do we determine channel? can't just use one!

	/* Enable the capture/compare interrupt for channel*/
	LL_TIM_EnableIT_CC1(timer_instance[channel_]);
	// TODO: how to properly set this corresponding to channel above?
	// Maybe we use the same idea as the DMA driver

	/**********************************/
	/* Start output signal generation */
	/**********************************/
	/* Enable output channel 1 */
	LL_TIM_CC_EnableChannel(timer_instance[channel_], LL_TIM_CHANNEL_CH1);
	// TODO: how to select proper channel?

	/* Enable counter */
	LL_TIM_EnableCounter(timer_instance[channel_]);

	/* Force update generation */
	LL_TIM_GenerateEvent_UPDATE(timer_instance[channel_]);

	enableInterrupts();
}

void STM32Timer::registerCallback(const embvm::timer::cb_t& cb) noexcept
{
	tim_callbacks[channel_] = cb;
}

void STM32Timer::registerCallback(embvm::timer::cb_t&& cb) noexcept
{
	tim_callbacks[channel_] = std::move(cb);
}

void STM32Timer::stop_() noexcept
{
	LL_TIM_DisableIT_CC1(timer_instance[channel_]); // TODO: how to select proper channel?
	disableInterrupts();
	LL_TIM_DeInit(timer_instance[channel_]);
	STM32ClockControl::timerDisable(channel_);
}

void STM32Timer::enableInterrupts() noexcept
{
	auto inst = irq_num[channel_];
	assert(inst); // Check that channel is supported
	NVICControl::priority(inst, 0); // TODO: how to configure priority for the driver?
	NVICControl::enable(inst);
}

void STM32Timer::disableInterrupts() noexcept
{
	auto inst = irq_num[channel_];
	assert(inst); // Check that channel is supported
	NVICControl::disable(inst);
}
