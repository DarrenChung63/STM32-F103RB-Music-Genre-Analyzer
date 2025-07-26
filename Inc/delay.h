#ifndef __DELAY_H
#define __DELAY_H

#include "stm32f1xx_hal.h" // HAL library include
#include <stdint.h>

void Delay_Timer_Init(void);
void Delay_ms(uint32_t ms);
void Delay_us(uint32_t us);

#endif
