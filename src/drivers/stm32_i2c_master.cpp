#include "stm32_i2c_master.hpp"
#include <array>
#include <cassert>
#include <driver/gpio.hpp> // for embvm::gpio::port
#include <nvic.hpp>
#include <processor_includes.hpp>
#include <stm32_rcc.hpp>
#include <stm32l4xx_ll_dma.h> // For configuration of DMA channel; TODO: break dependency
#include <stm32l4xx_ll_gpio.h> // TODO: break dependency
#include <stm32l4xx_ll_i2c.h>

// TODO: how can I move internal items like this, as well as other processor-dep. settings,
// into the processor layer? This would help us keep drivers common.
// One thought is taking the config-table approach that Beningo uses in his drivers.
// Then we can make defs public, and keep the impl. private, accessing them through pointers.
// TODO: Set a timer to trigger a timeout callback (using timer manager?)

/* Useful Developer Notes
 *
 * Working in start/stop mode:
 *   In order to start a transfer in Master mode, I2C Control Register 2 must be written with the
 *   Start condition request, the slave address, the transfer direction, the number of bytes to be
 *   transferred, and the End of Transfer mode. End of Transfer mode is configured by the AUTOEND
 *   bit. If it is set, the Stop condition is automatically sent after the programmed number of
 *   bytes is transferred.
 *
 *   If the AUTOEND bit is not set, the end of transfer is managed by software. After the programmed
 *   number of bytes is transferred, the Transfer Complete (TC) flag is set and an interrupt is
 *   generated, if enabled. Then a Repeated Start or a Stop condition can be requested by
 *   software.
 *
 *   The data transfer can be managed by interrupts or by the DMA.
 *
 *   For payloads > 255: the Reload bit must be set in I2C_CR2
 *   No reload == NBYTES data transferred, followed by STOP or RESTART
 *   Reload == NBYTES is reloaded after NBYTES of data is transfered; data transfer will resume
 *     Interrupt is generated if enabled, and TCR flag is set.
 */

#pragma mark - Definitions -

constexpr size_t MAX_I2C_TRANSFER_SIZE_BYTES = 255;

#pragma mark - Types and Declarations -

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
    .sda_pin= 0,
    .sda_alt_func = LL_GPIO_AF_4,
    .scl_port = embvm::gpio::port::F,
    .scl_pin = 1,
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

constexpr std::array<I2C_TypeDef* const, 4> i2c_instance = {I2C1, I2C2, I2C3, I2C4};

constexpr std::array<uint32_t, 4> dma_tx_routing = {LL_DMAMUX_REQ_I2C1_TX, LL_DMAMUX_REQ_I2C2_TX,
													LL_DMAMUX_REQ_I2C3_TX, LL_DMAMUX_REQ_I2C4_TX};

constexpr std::array<uint32_t, 4> dma_rx_routing = {LL_DMAMUX_REQ_I2C1_RX, LL_DMAMUX_REQ_I2C2_RX,
													LL_DMAMUX_REQ_I2C3_RX, LL_DMAMUX_REQ_I2C4_RX};

// TODO:
// static std::array<embvm::timer::cb_t, 9> i2c_callbacks = {nullptr};

constexpr std::array<uint8_t, 4> event_irq_num = {I2C1_EV_IRQn, I2C2_EV_IRQn, I2C3_EV_IRQn,
												  I2C4_EV_IRQn};

constexpr std::array<uint8_t, 4> error_irq_num = {I2C1_ER_IRQn, I2C2_ER_IRQn, I2C3_ER_IRQn,
												  I2C4_ER_IRQn};

static std::array<size_t, 4> transfer_reload_size = {0};
static std::array<bool, 4> transfer_reload_autoend = {false};

#pragma mark - Helpers -

static inline void enableDMATx(STM32DMA& ch_, I2C_TypeDef* inst, const uint8_t* buffer, size_t size)
{
	ch_.setAddresses(
		const_cast<uint8_t*>(
			buffer), // This is frowned upon, but the underlying STM32 code doesn't take const.
		reinterpret_cast<uint32_t*>(LL_I2C_DMA_GetRegAddr(inst, LL_I2C_DMA_REG_DATA_TRANSMIT)),
		size);
	ch_.enable();
}

static inline void enableDMARx(STM32DMA& ch_, I2C_TypeDef* inst, uint8_t* const buffer, size_t size)
{
	ch_.setAddresses(
		reinterpret_cast<uint32_t*>(LL_I2C_DMA_GetRegAddr(inst, LL_I2C_DMA_REG_DATA_RECEIVE)),
		buffer, size);
	ch_.enable();
}

/** Check and adjust transfer size to handle transfers > 255 bytes
 *
 * @precondition op_buffer_size > 0
 * @param [in] op_buffer_size The size of the TX or RX buffer as specified in the op request
 * @param [inout] bytes_remaining The value of the bytes remaining in additional transfers.
 *	This parameter will be non-zero when
 * @param [in] desired_end_mode The end mode that you want to set if the transfer size is within
 *	limits for a single transfer.
 * @returns A std::tuple containing:
 *	1. the active transfer size in bytes (size_t)
 *	2. the adjusted end mode (uint32_t)
 *		If the transfer size is > 255 bytes, then the end mode will be set to LL_I2C_MODE_RELOAD,
 *		otherwise it will be set to desired_end_mode.
 *
 * @note This function needs to be modified to support SMBus, beacuse the SMBus modes are not
 * considered here, especially for the autoend value.
 */
static std::tuple<size_t, uint32_t> check_and_adjust_transfer_size(const STM32I2CMaster::device dev,
																   const uint32_t& op_buffer_size,
																   uint32_t desired_end_mode)
{
	uint32_t bytes_for_immediate_transfer;
	uint32_t actual_end_mode;

	if(op_buffer_size > MAX_I2C_TRANSFER_SIZE_BYTES)
	{
		transfer_reload_size[dev] = op_buffer_size - MAX_I2C_TRANSFER_SIZE_BYTES;
		bytes_for_immediate_transfer = MAX_I2C_TRANSFER_SIZE_BYTES;
		actual_end_mode = LL_I2C_MODE_RELOAD;
		transfer_reload_autoend[dev] = (desired_end_mode == LL_I2C_MODE_AUTOEND);
	}
	else
	{
		bytes_for_immediate_transfer = op_buffer_size;
		transfer_reload_size[dev] = 0;
		actual_end_mode = desired_end_mode;
		transfer_reload_autoend[dev] = false;
	}

	return {bytes_for_immediate_transfer, actual_end_mode};
}

/**
 *
 * @param [in] dev The device ID, which is used for accessing the arrays
 * @param [in] inst The device instance, which saves us a dereference + assert since
 * 	we've already checked this in the caller's context.
 */
static void transfer_reload_or_end(const STM32I2CMaster::device dev, I2C_TypeDef* const inst)
{
	auto size = transfer_reload_size[dev];
	if(size > MAX_I2C_TRANSFER_SIZE_BYTES)
	{
		transfer_reload_size[dev] = size - MAX_I2C_TRANSFER_SIZE_BYTES;
		size = MAX_I2C_TRANSFER_SIZE_BYTES;
		LL_I2C_SetTransferSize(inst, size);
	}
	else if(size == 0)
	{
		// Check auto-end logic
		LL_I2C_DisableReloadMode(inst);
		LL_I2C_GenerateStopCondition(inst);
	}
	else
	{
		LL_I2C_SetTransferSize(inst, size);
	}
}

#pragma mark - Interrupt Handlers -

extern "C" void I2C1_ER_IRQHandler(void);
extern "C" void I2C1_EV_IRQHandler(void);
extern "C" void I2C2_ER_IRQHandler(void);
extern "C" void I2C2_EV_IRQHandler(void);
extern "C" void I2C3_ER_IRQHandler(void);
extern "C" void I2C3_EV_IRQHandler(void);
extern "C" void I2C4_ER_IRQHandler(void);
extern "C" void I2C4_EV_IRQHandler(void);

static void i2c_error_handler(STM32I2CMaster::device dev)
{
	(void)dev;
	assert(0); // TODO: Handle error
}

static void i2c_event_handler(STM32I2CMaster::device dev)
{
	auto inst = i2c_instance[dev];
	assert(inst); // invalid instance

	if(LL_I2C_IsActiveFlag_TCR(inst))
	{
		transfer_reload_or_end(dev, inst);
	}
	else if(LL_I2C_IsActiveFlag_STOP(inst))
	{
		// End of transfer?
		LL_I2C_ClearFlag_STOP(inst);
		// Todo: master complete callback
	}
}

void I2C1_ER_IRQHandler()
{
	i2c_error_handler(STM32I2CMaster::device::i2c1);
}

void I2C1_EV_IRQHandler()
{
	i2c_event_handler(STM32I2CMaster::device::i2c1);
}

void I2C2_ER_IRQHandler()
{
	i2c_error_handler(STM32I2CMaster::device::i2c2);
}

void I2C2_EV_IRQHandler()
{
	i2c_event_handler(STM32I2CMaster::device::i2c2);
}

void I2C3_ER_IRQHandler()
{
	i2c_error_handler(STM32I2CMaster::device::i2c3);
}

void I2C3_EV_IRQHandler()
{
	i2c_event_handler(STM32I2CMaster::device::i2c3);
}

void I2C4_ER_IRQHandler()
{
	i2c_error_handler(STM32I2CMaster::device::i2c4);
}

void I2C4_EV_IRQHandler()
{
	i2c_event_handler(STM32I2CMaster::device::i2c4);
}

#pragma mark - Driver APIs -

// TODO: need to control fast vs slow mode
// currently configured for fast mode
// This value is seriously hard-coded??? I2C_TIMING
#if 0

  /* Configure the SDA setup, hold time and the SCL high, low period */
  /* Timing register value is computed with the STM32CubeMX Tool,
    * Fast Mode @400kHz with I2CCLK = 80 MHz,
    * rise time = 100ns, fall time = 10ns
    * Timing Value = (uint32_t)0x00F02B86
    */
  timing = __LL_I2C_CONVERT_TIMINGS(0x0, 0xF, 0x0, 0x2B, 0x86);
  LL_I2C_SetTiming(I2C1, timing);



/* Timing register value is computed with the STM32CubeMX Tool,
  * Fast Mode @400kHz with I2CCLK = 80 MHz,
  * rise time = 100ns, fall time = 10ns
  * Timing Value = (uint32_t)0x00F02B86
  */
#define I2C_TIMING 0x00F02B86

Maybe we provide a way to make this configurable, ALA table, allowing users to modify it
to fit their needs
Would then come from the hardware platform.

Zephyr approach
int stm32_i2c_configure_timing(const struct device *dev, uint32_t clock)
{
  const struct i2c_stm32_config *cfg = DEV_CFG(dev);
  struct i2c_stm32_data *data = DEV_DATA(dev);
  I2C_TypeDef *i2c = cfg->i2c;
  uint32_t i2c_hold_time_min, i2c_setup_time_min;
  uint32_t i2c_h_min_time, i2c_l_min_time;
  uint32_t presc = 1U;
  uint32_t timing = 0U;

  /*  Look for an adequate preset timing value */
  for (uint32_t i = 0; i < cfg->n_timings; i++) {
    const struct i2c_config_timing *preset = &cfg->timings[i];
    uint32_t speed = i2c_map_dt_bitrate(preset->i2c_speed);

    if ((I2C_SPEED_GET(speed) == I2C_SPEED_GET(data->dev_config))
       && (preset->periph_clock == clock)) {
      /*  Found a matching periph clock and i2c speed */
      LL_I2C_SetTiming(i2c, preset->timing_setting);
      return 0;
    }
  }

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

  if (presc >= 16U) {
    LOG_DBG("I2C:failed to find prescaler value");
    return -EINVAL;
  }

  LL_I2C_SetTiming(i2c, timing);

  return 0;
}
#endif
#define I2C_TIMING 0x00F02B86 // Also try: 0x0020098E, 0x00F02B86, 0x00D00E28
// the 002 version is 100kHz with HSI = 16MHz
// 00D is 1MHz with I2C source as SYSCLK = 80 MHz
// 00F is 400Khz with I2CCLK = 80MHz
// If this doesn't work, try cubeMX generation
void STM32I2CMaster::start_() noexcept
{
	auto i2c_inst = i2c_instance[device_];
	assert(i2c_inst); // if failed, device is invalid

	configure_i2c_pins_();

	STM32ClockControl::i2cEnable(device_);

	// Disable before modifying configuration registers
	// TODO once working: can we remove?
	LL_I2C_Disable(i2c_inst);

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

	configureDMA();

	LL_I2C_EnableDMAReq_RX(i2c_inst);
	LL_I2C_EnableDMAReq_TX(i2c_inst);

	auto r = LL_I2C_Init(i2c_inst, &initializer);
	assert(r == 0);

	enableInterrupts();
}

void STM32I2CMaster::stop_() noexcept
{
	auto i2c_inst = i2c_instance[device_];
	assert(i2c_inst); // if failed, device is invalid

	disableInterrupts();

	LL_I2C_DeInit(i2c_inst);

	LL_I2C_DisableDMAReq_RX(i2c_inst);
	LL_I2C_DisableDMAReq_TX(i2c_inst);

	STM32ClockControl::i2cDisable(device_);
}

void STM32I2CMaster::configureDMA() noexcept
{
	tx_channel_.setConfiguration(LL_DMA_DIRECTION_MEMORY_TO_PERIPH | LL_DMA_PRIORITY_HIGH |
									 LL_DMA_MODE_NORMAL | LL_DMA_PERIPH_NOINCREMENT |
									 LL_DMA_MEMORY_INCREMENT | LL_DMA_PDATAALIGN_BYTE |
									 LL_DMA_MDATAALIGN_BYTE,
								 dma_tx_routing[device_]);
	rx_channel_.setConfiguration(LL_DMA_DIRECTION_PERIPH_TO_MEMORY | LL_DMA_PRIORITY_HIGH |
									 LL_DMA_MODE_NORMAL | LL_DMA_PERIPH_NOINCREMENT |
									 LL_DMA_MEMORY_INCREMENT | LL_DMA_PDATAALIGN_BYTE |
									 LL_DMA_MDATAALIGN_BYTE,
								 dma_rx_routing[device_]);

	// TODO: does this happen here? or only if we need to handle special cases?
	tx_channel_.registerCallback([this](STM32DMA::status status) {
		if(status == STM32DMA::status::ok)
		{
			// TODO: proper to do this here? tx_channel_.disable();
			transfer_completed_ = true;
		}
		else
		{
			// TODO: handle error?
			assert(0); // transfer failed
		}
	});
	rx_channel_.registerCallback([this](STM32DMA::status status) {
		if(status == STM32DMA::status::ok)
		{
			/// TODO: proper to do this here? rx_channel_.disable();
			transfer_completed_ = true;
		}
		else
		{
			// TODO: handle error?
			assert(0); // transfer failed
		}
	});

	tx_channel_.start();
	rx_channel_.start();
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
	// TODO: are these actually needed for DMA mode?
	// LL_I2C_EnableIT_TX(inst);
	// LL_I2C_EnableIT_RX(inst);
	LL_I2C_EnableIT_NACK(inst);
	LL_I2C_EnableIT_ERR(inst); // Enables Arbitration loss, Bus error, overrun/underrun errors
	LL_I2C_EnableIT_STOP(inst);
	LL_I2C_EnableIT_TC(inst); // Enables both transfer complete and transfer complete reload
}

void STM32I2CMaster::disableInterrupts() noexcept
{
	uint8_t error_irq = error_irq_num[device_];
	uint8_t event_irq = event_irq_num[device_];
	auto inst = i2c_instance[device_];
	assert(error_irq && event_irq && inst); // Check that channel is supported

	// TODO: are these actually needed for DMA mode?
	// LL_I2C_DisableIT_TX(inst);
	// LL_I2C_DisableIT_RX(inst);
	LL_I2C_DisableIT_NACK(inst);
	LL_I2C_DisableIT_ERR(inst);
	LL_I2C_DisableIT_STOP(inst);
	LL_I2C_DisableIT_TC(inst);
	NVICControl::disable(error_irq);
	NVICControl::disable(event_irq);
}

// Blocking implementation - nonblocking to come
embvm::i2c::status STM32I2CMaster::transfer_(const embvm::i2c::op_t& op,
											 const embvm::i2c::master::cb_t& cb) noexcept
{
	auto status = embvm::i2c::status::ok;
	auto i2c_inst = i2c_instance[device_];
	assert(i2c_inst); // Instance is not valid if failed
	uint32_t generate_mode = LL_I2C_GENERATE_STOP;
	uint32_t end_mode = LL_I2C_MODE_AUTOEND;
	uint32_t transfer_size = 0;

	// Note that the STM32 calls don't shift the address for us, so we need
	// to do this manually.
	uint32_t address = static_cast<uint32_t>(op.address << 1);

	transfer_completed_ = false;

	/** A Note on Large Transfers (> 255 bytes)
	 *
	 * For large transfers, we need to use the "reload" capability to ensure that the Transfer
	 * Complete Reload (TCR) flag/interrupt will fire. After the programmed number of bytes has been
	 * transferred, you'll get an interrupt. The additional number of bytes to be transferred is
	 * programmed, the TCR bit is set once more, and then the data transfer will resume.
	 *
	 * Note that when RELOAD is set, AUTOEND has no effect. We need to STOP the transfer manually.
	 *
	 * To handle this logic, use the check_and_adjust_transfer_size() function, which will set
	 * variables that are accessible in an interrupt context and can be used for handling these
	 * large transfers without necessary intervention on your part.
	 *
	 * Also note that since the DMA block can handle sizes > 255, we will pass the full
	 * buffer size to the enableDMATx/Rx functions.
	 */
	switch(op.op)
	{
		case embvm::i2c::operation::continueWriteStop:
		case embvm::i2c::operation::write: {
			// TODO: is this case really separated, because continue write I don't want to generate
			// a start? Or is it fine? I think I might need to separate them for a different
			// behavior - e.g. GENERATE_NOSTARTSTOP + AutoEnd?
			std::tie(transfer_size, end_mode) =
				check_and_adjust_transfer_size(device_, op.tx_size, LL_I2C_MODE_AUTOEND);
			generate_mode = LL_I2C_GENERATE_START_WRITE;
			enableDMATx(tx_channel_, i2c_inst, op.tx_buffer, op.tx_size);
			break;
		}
		case embvm::i2c::operation::writeNoStop:
		case embvm::i2c::operation::continueWriteNoStop: {
			std::tie(transfer_size, end_mode) =
				check_and_adjust_transfer_size(device_, op.tx_size, LL_I2C_MODE_SOFTEND);
			// TODO: for continue, does this need to be separated as a case?
			generate_mode = LL_I2C_GENERATE_START_WRITE;
			enableDMATx(tx_channel_, i2c_inst, op.tx_buffer, op.tx_size);
			break;
		}
		case embvm::i2c::operation::read: {
			std::tie(transfer_size, end_mode) =
				check_and_adjust_transfer_size(device_, op.tx_size, LL_I2C_MODE_AUTOEND);
			generate_mode = LL_I2C_GENERATE_START_READ;
			enableDMARx(rx_channel_, i2c_inst, op.rx_buffer, op.rx_size);
			break;
		}
		case embvm::i2c::operation::writeRead: {
			// TODO: implement... need to call twice
			// First, START_WRITE, then START READ?
			assert(0);
			break;
		}
		case embvm::i2c::operation::ping: {
			// TODO: zero-bytes correct here?
			// TODO: do we need to enable LL_I2C_EnableIT_ADDR, at least for ping?? Need to disable
			// somewhere too..
			assert(0);
			transfer_size = 0;
			end_mode = LL_I2C_MODE_AUTOEND;
			generate_mode = LL_I2C_GENERATE_START_WRITE;
			break;
		}
		case embvm::i2c::operation::stop: {
			// TODO?
			// LL_I2C_GenerateStopCondition(i2c_inst);
			// Instance, slave address, slave address size, transfer size, end mode, request);
			assert(0);
			transfer_size = 0;
			end_mode = LL_I2C_MODE_AUTOEND;
			generate_mode = LL_I2C_GENERATE_STOP;
			break;
		}
		case embvm::i2c::operation::restart: {
			// TODO: does this work?
			assert(0);
			transfer_size = 0;
			end_mode = LL_I2C_MODE_SOFTEND;
			generate_mode = LL_I2C_GENERATE_RESTART_7BIT_WRITE;
			break;
		}
	}

	LL_I2C_HandleTransfer(i2c_inst, address, LL_I2C_ADDRSLAVE_7BIT, transfer_size, end_mode,
						  generate_mode);

	// TODO: this needs to return enqueued and handle things asynchronously... for now we block.
	while(!transfer_completed_ && !LL_I2C_IsActiveFlag_STOP(i2c_inst))
		;

	// TODO: this doesn't belong here, but the method of disabling the channel during the interrupt
	// Was not reliable and was causing assert failures in setAddresses
	// We need a more reliable method... I2C transfer done?
	tx_channel_.disable();

	return status;
}

void STM32I2CMaster::configure_(embvm::i2c::pullups pullup)
{
	assert(0); // TODO:
}
