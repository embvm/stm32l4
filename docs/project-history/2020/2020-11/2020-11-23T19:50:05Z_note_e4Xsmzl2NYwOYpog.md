Adjust STM32L4 build based on Ethan's feedback:

> Oh and one thing I noticed is that you wrap the STM32L4Cube github repo. If you didn't notice that repo is really bad because for some reason there is something like a gig of MP3 files in it! haha.
> 
> If all you want is the HAL and CMSIS I have always grabbed just Those. If you actually want other stuff I don't know a good solution.

- https://github.com/STMicroelectronics/stm32l4xx_hal_driver
- https://github.com/STMicroelectronics/cmsis_device_l4
- https://github.com/ARM-software/CMSIS_5

We're already pulling in CMSIS-5 separately, so we ditched the massive repo to use this one. Clones are much faster!

