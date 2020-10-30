#include "stm32_i2c_master.hpp"
#include <array>
#include <cassert>
#include <driver/gpio.hpp> // for embvm::gpio::port
#include <nvic.hpp>
#include <stm32_rcc.hpp>
#include <stm32l4xx_ll_gpio.h> // TODO: break dependency
#include <stm32l4xx_ll_i2c.h>

// TODO: how can I move internal items like this, as well as other processor-dep. settings,
// into the processor layer? This would help us keep drivers common.
// One thought is taking the config-table approach that Beningo uses in his drivers.
// Then we can make defs public, and keep the impl. private, accessing them through pointers.

struct STM32_I2C_Pins_t
{
	embvm::gpio::port sda_port;
	uint8_t sda_pin;
	uint8_t sda_alt_func;
	embvm::gpio::port scl_port;
	uint8_t scl_pin;
	// Because processors and pin selections require different alternate setting values, we need
	// to store the items in the table for our processor
	uint8_t scl_alt_func;
} __attribute__((packed));

// TODO: actually, I just need to pull this out of this class and expand the GPIO class to
// accomodate alternate functions
// TODO: how can we make this more configurable for users, since the pins will
// really change based on the board design? E.g., you can select different AF settings
// for diff pins. I2C3 SDA might be PB4/PC1 with AF4, or it might be PC9 with AF6.
// Maybe this really needs to be done with the GPIO class, so the pins are configured properly.
// clang-format off
constexpr std::array<STM32_I2C_Pins_t, 4> i2c_pins =
{
  // TODO: I2C1 GPIO
  STM32_I2C_Pins_t{
    .sda_port = embvm::gpio::port::A,
    .sda_pin= 0,
    .sda_alt_func = LL_GPIO_AF_0,
    .scl_port = embvm::gpio::port::A,
    .scl_pin = 0,
    .scl_alt_func = LL_GPIO_AF_0

  },
  // I2C2 GPIO
  STM32_I2C_Pins_t{
    .sda_port = embvm::gpio::port::F,
    .sda_pin= 1,
    .sda_alt_func = LL_GPIO_AF_0,
    .scl_port = embvm::gpio::port::F,
    .scl_pin = 0,
    .scl_alt_func = LL_GPIO_AF_4
  },
  // TODO: I2C3 GPIO
  STM32_I2C_Pins_t{
    .sda_port = embvm::gpio::port::A,
    .sda_pin= 0,
    .sda_alt_func = LL_GPIO_AF_0,
    .scl_port = embvm::gpio::port::A,
    .scl_pin = 0,
    .scl_alt_func = LL_GPIO_AF_0,
  },
  // TODO: I2C4 GPIO
  STM32_I2C_Pins_t{
    .sda_port = embvm::gpio::port::A,
    .sda_pin= 0,
    .sda_alt_func = LL_GPIO_AF_0,
    .scl_port = embvm::gpio::port::A,
    .scl_pin = 0,
    .scl_alt_func = LL_GPIO_AF_0
  }
};
// clang-format on

// I2C4 not supported
constexpr std::array<I2C_TypeDef* const, 4> i2c_instance = {I2C1, I2C2, I2C3, nullptr};

// TODO:
// static std::array<embvm::timer::cb_t, 9> i2c_callbacks = {nullptr};

constexpr std::array<uint8_t, 4> event_irq_num = {I2C1_EV_IRQn, I2C2_EV_IRQn, I2C3_EV_IRQn,
												  I2C4_EV_IRQn};

constexpr std::array<uint8_t, 4> error_irq_num = {I2C1_ER_IRQn, I2C2_ER_IRQn, I2C3_ER_IRQn,
												  I2C4_ER_IRQn};

// TODO: need to control fast vs slow mode
// currently configured for fast mode
// This value is seriously hard-coded??? I2C_TIMING
#if 0
/* Timing register value is computed with the STM32CubeMX Tool,
  * Fast Mode @400kHz with I2CCLK = 80 MHz,
  * rise time = 100ns, fall time = 10ns
  * Timing Value = (uint32_t)0x00F02B86
  */
#define I2C_TIMING 0x00F02B86

Zephyr approach
/* No preset timing was provided, let's dynamically configure */
  switch (I2C_SPEED_GET(data->dev_config)) {
  case I2C_SPEED_STANDARD:
    i2c_h_min_time = 4000U;
    i2c_l_min_time = 4700U;
    i2c_hold_time_min = 500U;
    i2c_setup_time_min = 1250U;
    break;
  case I2C_SPEED_FAST:
    i2c_h_min_time = 600U;
    i2c_l_min_time = 1300U;
    i2c_hold_time_min = 375U;
    i2c_setup_time_min = 500U;
    break;
  default:
    return -EINVAL;
  }

  /* Calculate period until prescaler matches */
  do {
    uint32_t t_presc = clock / presc;
    uint32_t ns_presc = NSEC_PER_SEC / t_presc;
    uint32_t sclh = i2c_h_min_time / ns_presc;
    uint32_t scll = i2c_l_min_time / ns_presc;
    uint32_t sdadel = i2c_hold_time_min / ns_presc;
    uint32_t scldel = i2c_setup_time_min / ns_presc;

    if ((sclh - 1) > 255 ||  (scll - 1) > 255) {
      ++presc;
      continue;
    }

    if (sdadel > 15 || (scldel - 1) > 15) {
      ++presc;
      continue;
    }

    timing = __LL_I2C_CONVERT_TIMINGS(presc - 1,
          scldel - 1, sdadel, sclh - 1, scll - 1);
    break;
  } while (presc < 16);
#endif
#define I2C_TIMING 0x00F02B86 // Also try: 0x0020098E, 0x00F02B86, 0x00D00E28
// the 002 version is 100kHz with HSI = 16MHz
// 00D is 1MHz with I2C source as SYSCLK = 80 MHz
// 00F is 400Khz with I2CCLK = 80MHz
// If this doesn't work, try cubeMX generation
void STM32I2CMaster::start_() noexcept
{
	STM32ClockControl::i2cEnable(device_);

	// TODO: can this be constexpr? LL init func doens't take const...
	static LL_I2C_InitTypeDef initializer = {
		.PeripheralMode = LL_I2C_MODE_I2C,
		.Timing = I2C_TIMING,
		.AnalogFilter = LL_I2C_ANALOGFILTER_ENABLE,
		.DigitalFilter = 0x00,
		.OwnAddress1 = 0x00,
		.TypeAcknowledge = LL_I2C_ACK,
		.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT,
	};

	auto r = LL_I2C_Init(i2c_instance[device_], &initializer);
	assert(r == 0);

	enableInterrupts();
}

void STM32I2CMaster::stop_() noexcept
{
	disableInterrupts();

	LL_I2C_DeInit(i2c_instance[device_]);

	STM32ClockControl::i2cDisable(device_);
}

void STM32I2CMaster::configure_i2c_pins_() noexcept
{
	assert(device_ == STM32I2CMaster::device::i2c2); // Others not currently supported

	// TODO: move out of here and into hardware platform??
	auto pins = i2c_pins[device_];
	STM32GPIOTranslator::configure_alternate_i2c(pins.scl_port, pins.scl_pin, pins.scl_alt_func);
	STM32GPIOTranslator::configure_alternate_i2c(pins.sda_port, pins.sda_pin, pins.sda_alt_func);
}

embvm::i2c::pullups STM32I2CMaster::setPullups_(embvm::i2c::pullups pullups) noexcept
{
	// TODO: right now this is just set internally
	return pullups;
}

embvm::i2c::baud STM32I2CMaster::baudrate_(embvm::i2c::baud baud) noexcept
{
	// TODO:
	return baud;
}

void STM32I2CMaster::enableInterrupts() noexcept
{
	uint8_t error_irq = error_irq_num[device_];
	uint8_t event_irq = event_irq_num[device_];
	auto inst = i2c_instance[device_];
	assert(error_irq && event_irq && inst); // Check that channel is supported

	NVICControl::priority(error_irq, 0); // TODO: how to configure priority for the driver?
	NVICControl::enable(error_irq);
	NVICControl::priority(event_irq, 0); // TODO: how to configure priority for the driver?
	NVICControl::enable(event_irq);

	/* Enable I2C transfer complete/error interrupts:
	 *  - Enable Receive Interrupt
	 *  - Enable Not acknowledge received interrupt
	 *  - Enable Error interrupts
	 *  - Enable Stop interrupt
	 */
	LL_I2C_EnableIT_RX(inst);
	LL_I2C_EnableIT_NACK(inst);
	LL_I2C_EnableIT_ERR(inst);
	LL_I2C_EnableIT_STOP(inst);
}

void STM32I2CMaster::disableInterrupts() noexcept
{
	uint8_t error_irq = error_irq_num[device_];
	uint8_t event_irq = event_irq_num[device_];
	auto inst = i2c_instance[device_];
	assert(error_irq && event_irq && inst); // Check that channel is supported

	LL_I2C_DisableIT_RX(inst);
	LL_I2C_DisableIT_NACK(inst);
	LL_I2C_DisableIT_ERR(inst);
	LL_I2C_DisableIT_STOP(inst);
	NVICControl::disable(error_irq);
	NVICControl::disable(event_irq);
}

// Blocking implementation - nonblocking to come
embvm::i2c::status STM32I2CMaster::transfer_(const embvm::i2c::op_t& op,
											 const embvm::i2c::master::cb_t& cb) noexcept
{
	auto status = embvm::i2c::status::ok;

#if 0
  nRFTWIMTranslator::set_transfer_address(TTWIIndex, op.address);
  switch(op.op)
  {
    case embvm::i2c::operation::continueWriteStop:
    case embvm::i2c::operation::write: {
      status = nRFTWIMTranslator::tx_transfer_blocking(
        TTWIIndex, op.tx_buffer, op.tx_size, nRFTWIMTranslator::STOP);
      break;
    }
    case embvm::i2c::operation::writeNoStop:
    case embvm::i2c::operation::continueWriteNoStop: {
      status = nRFTWIMTranslator::tx_transfer_blocking(
        TTWIIndex, op.tx_buffer, op.tx_size, nRFTWIMTranslator::NO_STOP);
      break;
    }
    case embvm::i2c::operation::read: {
      status =
        nRFTWIMTranslator::rx_transfer_blocking(TTWIIndex, op.rx_buffer, op.rx_size);
      break;
    }
    case embvm::i2c::operation::writeRead: {
      status = nRFTWIMTranslator::txrx_transfer_blocking(
        TTWIIndex, op.tx_buffer, op.tx_size, op.rx_buffer, op.rx_size);
      break;
    }
    case embvm::i2c::operation::ping: {
      static uint8_t ping_dummy_byte_;

      status = nRFTWIMTranslator::rx_transfer_blocking(TTWIIndex, &ping_dummy_byte_,
                               sizeof(ping_dummy_byte_));
      break;
    }
    case embvm::i2c::operation::stop: {
      nRFTWIMTranslator::stop_condition(TTWIIndex);
      break;
    }
    case embvm::i2c::operation::restart: {
      // TODO: is this enough?
      nRFTWIMTranslator::stop_condition(TTWIIndex);
      break;
    }
  }
#endif

	return status;
}

void STM32I2CMaster::configure_(embvm::i2c::pullups pullup) {}
