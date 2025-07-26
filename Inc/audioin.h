#ifndef __AUDIOIN_H
#define __AUDIOIN_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

void ADC1_Init(uint16_t *adcBuffer, uint16_t adcBufferSize);
void DMA_ADC1_Config(uint16_t *adcBuffer, uint16_t adcBufferSize);

void AudioIn_Init(uint16_t *adcBuffer, uint16_t adcBufferSize);

#endif
