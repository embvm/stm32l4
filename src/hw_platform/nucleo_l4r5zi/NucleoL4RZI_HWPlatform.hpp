#ifndef NUCLEO_L4R5ZI_HW_PLATFORM_HPP_
#define NUCLEO_L4R5ZI_HW_PLATFORM_HPP_

#include <driver/led.hpp>
#include <hw_platform/virtual_hw_platform.hpp>
#include <stm32_gpio.hpp>
#include <stm32l4r5.hpp>

#if 0

/** @defgroup STM32L4XX_NUCLEO_144_LED LED
  * @{
  */
#define LEDn 3

#define LED1_PIN GPIO_PIN_7
#define LED1_GPIO_PORT GPIOC
#define LED1_GPIO_CLK_ENABLE() __HAL_RCC_GPIOC_CLK_ENABLE()
#define LED1_GPIO_CLK_DISABLE() __HAL_RCC_GPIOC_CLK_DISABLE()

#define LED2_PIN GPIO_PIN_7
#define LED2_GPIO_PORT GPIOB
#define LED2_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()
#define LED2_GPIO_CLK_DISABLE() __HAL_RCC_GPIOB_CLK_DISABLE()

#define LED3_PIN GPIO_PIN_14
#define LED3_GPIO_PORT GPIOB
#define LED3_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()
#define LED3_GPIO_CLK_DISABLE() __HAL_RCC_GPIOB_CLK_DISABLE()

#define LEDx_GPIO_CLK_ENABLE(__INDEX__)   \
	do                                    \
	{                                     \
		if((__INDEX__) == 0)              \
		{                                 \
			__HAL_RCC_GPIOC_CLK_ENABLE(); \
		}                                 \
		else                              \
		{                                 \
			__HAL_RCC_GPIOB_CLK_ENABLE(); \
		}                                 \
	} while(0)
#define LEDx_GPIO_CLK_DISABLE(__INDEX__)   \
	do                                     \
	{                                      \
		if((__INDEX__) == 0)               \
		{                                  \
			__HAL_RCC_GPIOC_CLK_DISABLE(); \
		}                                  \
		else                               \
		{                                  \
			__HAL_RCC_GPIOB_CLK_DISABLE(); \
		}                                  \
	} while(0)

/**
  * @}
  */

/** @defgroup STM32L4XX_NUCLEO_144_BUTTON BUTTON
  * @{
  */
#define BUTTONn 1

/**
 * @brief Key push-button
 */
#define USER_BUTTON_PIN GPIO_PIN_13
#define USER_BUTTON_GPIO_PORT GPIOC
#define USER_BUTTON_GPIO_CLK_ENABLE() __HAL_RCC_GPIOC_CLK_ENABLE()
#define USER_BUTTON_GPIO_CLK_DISABLE() __HAL_RCC_GPIOC_CLK_DISABLE()
#define USER_BUTTON_EXTI_LINE GPIO_PIN_13
#define USER_BUTTON_EXTI_IRQn EXTI15_10_IRQn

#define BUTTONx_GPIO_CLK_ENABLE(__INDEX__) USER_BUTTON_GPIO_CLK_ENABLE()
#define BUTTONx_GPIO_CLK_DISABLE(__INDEX__) USER_BUTTON_GPIO_CLK_DISABLE()

/* Aliases */
#define KEY_BUTTON_PIN USER_BUTTON_PIN
#define KEY_BUTTON_GPIO_PORT USER_BUTTON_GPIO_PORT
#define KEY_BUTTON_GPIO_CLK_ENABLE() USER_BUTTON_GPIO_CLK_ENABLE()
#define KEY_BUTTON_GPIO_CLK_DISABLE() USER_BUTTON_GPIO_CLK_DISABLE()
#define KEY_BUTTON_EXTI_LINE USER_BUTTON_EXTI_LINE
#define KEY_BUTTON_EXTI_IRQn USER_BUTTON_EXTI_IRQn

/**
  * @}
  */


/** @defgroup STM32L4XX_NUCLEO_144_PIN PIN
  * @{
  */
#define OTG_FS1_OVER_CURRENT_PIN GPIO_PIN_5
#define OTG_FS1_OVER_CURRENT_PORT GPIOG
#define OTG_FS1_OVER_CURRENT_PORT_CLK_ENABLE() __HAL_RCC_GPIOG_CLK_ENABLE()

#define OTG_FS1_POWER_SWITCH_PIN GPIO_PIN_6
#define OTG_FS1_POWER_SWITCH_PORT GPIOG
#define OTG_FS1_POWER_SWITCH_PORT_CLK_ENABLE() __HAL_RCC_GPIOG_CLK_ENABLE()
#endif

typedef enum
{
	LED1 = 0,
	LED_GREEN = LED1,
	LED2 = 1,
	LED_BLUE = LED2,
	LED3 = 2,
	LED_RED = LED3
} Led_TypeDef;

typedef enum
{
	BUTTON_USER = 0,
	/* Alias */
	BUTTON_KEY = BUTTON_USER
} Button_TypeDef;

typedef enum
{
	BUTTON_MODE_GPIO = 0,
	BUTTON_MODE_EXTI = 1
} ButtonMode_TypeDef;

typedef enum
{
	JOY_NONE = 0,
	JOY_SEL = 1,
	JOY_DOWN = 2,
	JOY_LEFT = 3,
	JOY_RIGHT = 4,
	JOY_UP = 5
} JOYState_TypeDef;

class NucleoL4R5ZI_HWPlatform : public embvm::VirtualHwPlatformBase<NucleoL4R5ZI_HWPlatform>
{
	using HWPlatformBase = embvm::VirtualHwPlatformBase<NucleoL4R5ZI_HWPlatform>;

  public:
	/// @brief Default constructor.
	NucleoL4R5ZI_HWPlatform() noexcept;

	/// @brief Default destructor.
	~NucleoL4R5ZI_HWPlatform() noexcept;

	// Required functions
	static void earlyInitHook_() noexcept;
	void init_() noexcept;
	void initProcessor_() noexcept;
	void soft_reset_() noexcept;
	void hard_reset_() noexcept;

	// Public APIs
	void leds_off() noexcept;
	void startBlink() noexcept;

  private:
	// TODO: maybe all of this can be hidden in the .cpp file, meaning we dont' need to
	// Expose any dependnecies or non-portable headers here!!!!
	stm32l4r5 processor_;

	STM32GPIOOutput<STM32GPIOPort::C, 7> led1_pin{};
	STM32GPIOOutput<STM32GPIOPort::B, 7> led2_pin{};
	STM32GPIOOutput<STM32GPIOPort::B, 14> led3_pin{};

	embvm::led::gpioActiveHigh led1{led1_pin};
	embvm::led::gpioActiveHigh led2{led2_pin};
	embvm::led::gpioActiveHigh led3{led3_pin};
};

#endif // NUCLEO_L4R5ZI_HW_PLATFORM_HPP_
