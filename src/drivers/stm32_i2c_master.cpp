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
// TODO: DMA support

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

extern "C" void I2C1_ER_IRQHandler(void);
extern "C" void I2C1_EV_IRQHandler(void);
extern "C" void I2C2_ER_IRQHandler(void);
extern "C" void I2C2_EV_IRQHandler(void);
extern "C" void I2C3_ER_IRQHandler(void);
extern "C" void I2C3_EV_IRQHandler(void);
extern "C" void I2C4_ER_IRQHandler(void);
extern "C" void I2C4_EV_IRQHandler(void);

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

static void i2c_error_handler(STM32I2CMaster::device dev)
{
	assert(0); // TODO:
}

static void i2c_event_handler(STM32I2CMaster::device dev)
{
	assert(0); // TODO:
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
	LL_I2C_EnableIT_TX(inst);
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

	// TODO: are these actually needed for DMA mode?
	LL_I2C_DisableIT_TX(inst);
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
	auto i2c_inst = i2c_instance[device_];
	assert(i2c_inst); // Instance is not valid if failed

	assert(0); // TODO: NOT FINISHED

	// TODO: handle transfers > 255 bytes using reload mode
	assert(op.tx_size <= 255 && op.rx_size <= 255);

	switch(op.op)
	{
		case embvm::i2c::operation::continueWriteStop:
		case embvm::i2c::operation::write: {
			// TODO: is this case really separated, because continue write I don't want to generate
			// a start? Or is it fine?
			LL_I2C_HandleTransfer(i2c_instance[device_], op.address, LL_I2C_ADDRSLAVE_7BIT,
								  op.tx_size, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);
			break;
		}
		case embvm::i2c::operation::writeNoStop:
		case embvm::i2c::operation::continueWriteNoStop: {
			LL_I2C_HandleTransfer(i2c_instance[device_], op.address, LL_I2C_ADDRSLAVE_7BIT,
								  op.tx_size, LL_I2C_MODE_SOFTEND, LL_I2C_GENERATE_START_WRITE);
			break;
		}
		case embvm::i2c::operation::read: {
			LL_I2C_HandleTransfer(i2c_instance[device_], op.address, LL_I2C_ADDRSLAVE_7BIT,
								  op.tx_size, LL_I2C_MODE_SOFTEND, LL_I2C_GENERATE_START_READ);
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
			LL_I2C_HandleTransfer(i2c_instance[device_], op.address, LL_I2C_ADDRSLAVE_7BIT, 0,
								  LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);
			break;
		}
		case embvm::i2c::operation::stop: {
			// TODO?
			// LL_I2C_GenerateStopCondition(i2c_inst);
			// Instance, slave address, slave address size, transfer size, end mode, request);
			LL_I2C_HandleTransfer(i2c_instance[device_], op.address, LL_I2C_ADDRSLAVE_7BIT, 0,
								  LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_STOP);
			break;
		}
		case embvm::i2c::operation::restart: {
			// TODO: does this work? Do we need to disable it? Should we use handle transfer?
			LL_I2C_GenerateStartCondition(i2c_inst);
			break;
		}
	}

	// TODO: how to set status?

	// Non-dma, we have:
	// void LL_I2C_TransmitData8(I2C_TypeDef *I2Cx, uint8_t Data)
	// uint8_t LL_I2C_ReceiveData8(I2C_TypeDef *I2Cx)

	return status;
}

void STM32I2CMaster::configure_(embvm::i2c::pullups pullup) {}

#if 0
/**
  * @brief  This Function handle Master events to perform a transmission process
  * @note  This function is composed in different steps :
  *        -1- Configure DMA parameters for Command Code transfer.
  *        -2- Enable DMA transfer.
  *        -3- Initiate a Start condition to the Slave device.
  *        -4- Loop until end of DMA transfer completed (DMA TC raised).
  *        -5- Loop until end of master transfer completed (STOP flag raised).
  *        -6- Clear pending flags, Data Command Code are checking into Slave process.
  * @param  None
  * @retval None
  */
void Handle_I2C_Master_Transmit(void)
{
  /* (1) Configure DMA parameters for Command Code transfer *******************/
  pMasterTransmitBuffer    = (uint32_t*)(&aCommandCode[ubMasterCommandIndex][0]);
  ubMasterNbDataToTransmit = strlen((char *)pMasterTransmitBuffer[0]);

  LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_2, (uint32_t)(*pMasterTransmitBuffer));
  LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, ubMasterNbDataToTransmit);

  /* (2) Enable DMA transfer **************************************************/
  LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);

  /* (3) Initiate a Start condition to the Slave device ***********************/

  /* Master Generate Start condition for a write request:
   *  - to the Slave with a 7-Bit SLAVE_OWN_ADDRESS
   *  - with a auto stop condition generation when transmit all bytes
   *  - No specific answer is needed from Slave Device, configure auto-stop condition
   */
  LL_I2C_HandleTransfer(I2C3, SLAVE_OWN_ADDRESS, LL_I2C_ADDRSLAVE_7BIT, ubMasterNbDataToTransmit, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);

  /* (4) Loop until end of transfer completed (DMA TC raised) *****************/

#if(USE_TIMEOUT == 1)
  Timeout = DMA_SEND_TIMEOUT_TC_MS;
#endif /* USE_TIMEOUT */

  /* Loop until DMA transfer complete event */
  while(!ubMasterTransferComplete)
  {
#if(USE_TIMEOUT == 1)
    /* Check Systick counter flag to decrement the time-out value */
    if (LL_SYSTICK_IsActiveCounterFlag())
    {
      if(Timeout-- == 0)
      {
        /* Time-out occurred. Set LED to blinking mode */
        LED_Blinking(LED_BLINK_SLOW);
      }
    }
#endif /* USE_TIMEOUT */
  }

  /* (5) Loop until end of master process completed (STOP flag raised) ********/
#if(USE_TIMEOUT == 1)
  Timeout = I2C_SEND_TIMEOUT_STOP_MS;
#endif /* USE_TIMEOUT */

  /* Loop until STOP flag is raised  */
  while(!LL_I2C_IsActiveFlag_STOP(I2C3))
  {
#if(USE_TIMEOUT == 1)
    /* Check Systick counter flag to decrement the time-out value */
    if (LL_SYSTICK_IsActiveCounterFlag())
    {
      if(Timeout-- == 0)
      {
        /* Time-out occurred. Set LED2 to blinking mode */
        LED_Blinking(LED_BLINK_SLOW);
      }
    }
#endif /* USE_TIMEOUT */
  }

  /* (6) Clear pending flags, Data Command Code are checking into Slave process */
  /* End of Master Process */
  LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
  LL_I2C_ClearFlag_STOP(I2C3);

  /* Display through external Terminal IO the Slave Answer received */
  printf("%s : %s\n\r", (char*)(aCommandCode[ubMasterCommandIndex][0]), (char*)aMasterReceiveBuffer);

  /* Turn LED2 On */
  /* Master sequence completed successfully*/
  LED_On();
  /* Keep LED2 On, 500 MilliSeconds */
  LL_mDelay(500);
  LED_Off();

  /* Clear and Reset process variables and arrays */
  ubMasterTransferComplete = 0;
  ubMasterNbDataToTransmit = 0;
  ubMasterReceiveIndex     = 0;
  FlushBuffer8(aMasterReceiveBuffer);
}

/**
  * @brief  This Function handle Master events to perform a transmission then a reception process
  * @note   This function is composed in different steps :
  *         -1- Configure DMA parameters for Command Code transfer.
  *         -2- Enable DMA transfer.
  *         -3- Initiate a Start condition to the Slave device.
  *         -4- Loop until end of DMA transfer completed (DMA TC raised).
  *         -5- Loop until end of master transfer completed (TC flag raised).
  *         -6- Configure DMA to receive data from slave.
  *         -7- Initiate a ReStart condition to the Slave device.
  *         -8- Loop until end of master process completed (STOP flag raised).
  *         -9- Clear pending flags, Data Command Code are checking into Slave process.
  * @param  None
  * @retval None
  */
void Handle_I2C_Master_TransmitReceive(void)
{
  /* (1) Configure DMA parameters for Command Code transfer *******************/
  pMasterTransmitBuffer    = (uint32_t*)(&aCommandCode[ubMasterCommandIndex][0]);
  ubMasterNbDataToTransmit = strlen((char *)pMasterTransmitBuffer[0]);

  LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_2, (uint32_t)(*pMasterTransmitBuffer));
  LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, ubMasterNbDataToTransmit);

  /* (2) Enable DMA transfer **************************************************/
  LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);

  /* (3) Initiate a Start condition to the Slave device ***********************/

  /* Master Generate Start condition for a write request:
   *  - to the Slave with a 7-Bit SLAVE_OWN_ADDRESS
   *  - with a no stop condition generation when transmit all bytes
   *  - A specific answer is needed from Slave Device, configure no-stop condition
   */
  LL_I2C_HandleTransfer(I2C3, SLAVE_OWN_ADDRESS, LL_I2C_ADDRSLAVE_7BIT, ubMasterNbDataToTransmit, LL_I2C_MODE_SOFTEND, LL_I2C_GENERATE_START_WRITE);

  /* (4) Loop until end of transfer completed (DMA TC raised) *****************/

#if(USE_TIMEOUT == 1)
  Timeout = DMA_SEND_TIMEOUT_TC_MS;
#endif /* USE_TIMEOUT */

  /* Loop until DMA transfer complete event */
  while(!ubMasterTransferComplete)
  {
#if(USE_TIMEOUT == 1)
    /* Check Systick counter flag to decrement the time-out value */
    if (LL_SYSTICK_IsActiveCounterFlag())
    {
      if(Timeout-- == 0)
      {
        /* Time-out occurred. Set LED to blinking mode */
        LED_Blinking(LED_BLINK_SLOW);
      }
    }
#endif /* USE_TIMEOUT */
  }

  /* Reset ubMasterTransferComplete flag */
  ubMasterTransferComplete = 0;

  /* (5) Loop until end of master transfer completed (TC flag raised) *********/
  /* Wait Master Transfer completed */
#if(USE_TIMEOUT == 1)
  Timeout = I2C_SEND_TIMEOUT_TC_MS;
#endif /* USE_TIMEOUT */

  while(LL_I2C_IsActiveFlag_TC(I2C3) != 1)
  {
#if(USE_TIMEOUT == 1)
    /* Check Systick counter flag to decrement the time-out value */
    if (LL_SYSTICK_IsActiveCounterFlag())
    {
      if(Timeout-- == 0)
      {
        /* Time-out occurred. Set LED to blinking mode */
        LED_Blinking(LED_BLINK_SLOW);
      }
    }
#endif /* USE_TIMEOUT */
  }

  /* (6) Configure DMA to receive data from slave *****************************/
  LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
  LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, ubMasterNbDataToReceive);
  LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3);

  /* (7) Initiate a ReStart condition to the Slave device *********************/
  /* Master Generate Start condition for a write request:
   *    - to the Slave with a 7-Bit SLAVE_OWN_ADDRESS
   *    - with a auto stop condition generation when transmit all bytes
   */
  LL_I2C_HandleTransfer(I2C3, SLAVE_OWN_ADDRESS, LL_I2C_ADDRSLAVE_7BIT, ubMasterNbDataToReceive, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_RESTART_7BIT_READ);

  /* (8) Loop until end of master process completed (STOP flag raised) ********/
#if(USE_TIMEOUT == 1)
  Timeout = I2C_SEND_TIMEOUT_STOP_MS;
#endif /* USE_TIMEOUT */

  /* Loop until STOP flag is raised  */
  while(!LL_I2C_IsActiveFlag_STOP(I2C3))
  {
#if(USE_TIMEOUT == 1)
    /* Check Systick counter flag to decrement the time-out value */
    if (LL_SYSTICK_IsActiveCounterFlag())
    {
      if(Timeout-- == 0)
      {
        /* Time-out occurred. Set LED2 to blinking mode */
        LED_Blinking(LED_BLINK_SLOW);
      }
    }
#endif /* USE_TIMEOUT */
  }

  /* (9) Clear pending flags, Data Command Code are checking into Slave process */
  /* End of Master Process */
  LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3);
  LL_I2C_ClearFlag_STOP(I2C3);

  /* Display through external Terminal IO the Slave Answer received */
  printf("%s : %s\n\r", (char*)(aCommandCode[ubMasterCommandIndex][0]), (char*)aMasterReceiveBuffer);

  /* Turn LED2 On */
  /* Master sequence completed successfully*/
  LED_On();
  /* Keep LED2 On, 500 MilliSeconds */
  LL_mDelay(500);
  LED_Off();

  /* Clear and Reset process variables and arrays */
  ubMasterTransferComplete = 0;
  ubMasterNbDataToTransmit = 0;
  ubMasterReceiveIndex     = 0;
  FlushBuffer8(aMasterReceiveBuffer);
}
#endif
