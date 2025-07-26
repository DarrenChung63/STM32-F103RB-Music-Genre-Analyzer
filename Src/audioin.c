#include "audioin.h"
#include <stdio.h>
#include "usart.h"

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

// Configure ADC1 on PA0
void ADC1_Init(uint16_t *adcBuffer, uint16_t adcBufferSize) {
    ADC_ChannelConfTypeDef sConfig = {0};

    hadc1.Instance = ADC1;
    hadc1.Init.ScanConvMode = DISABLE;
    hadc1.Init.ContinuousConvMode = ENABLE;
    hadc1.Init.NbrOfConversion = 1;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;

    if (HAL_ADC_Init(&hadc1) != HAL_OK) {
        printf("ADC1 configure failed.\n");
    }

    // Configure Regular Channel
    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5; // Corresponds to 55.5 cycles
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
        printf("Configure ADC channel failed.\n");
    }

    // Calibrate ADC1
    if (HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK) {
    	printf("ADC calibrate failed.\n");
    }
}

// Configure DMA1 for ADC1
void DMA_ADC1_Config(uint16_t *adcBuffer, uint16_t adcBufferSize) {
    __HAL_RCC_DMA1_CLK_ENABLE();

    /* DMA1_Channel1_IRQn interrupt configuration */
    // This part is generally done in stm32f1xx_hal_msp.c for DMA
    // NVIC_SetPriority(DMA1_Channel1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 1, 0));
    // HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

    hdma_adc1.Instance = DMA1_Channel1;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_CIRCULAR;
    hdma_adc1.Init.Priority = DMA_PRIORITY_HIGH;

    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK) {
        printf("DMA initialization failed.\n");
    }

    __HAL_LINKDMA(&hadc1, DMA_Handle, hdma_adc1); // Link ADC to DMA
}

void AudioIn_Init(uint16_t *adcBuffer, uint16_t adcBufferSize){
    ADC1_Init(adcBuffer, adcBufferSize);
    DMA_ADC1_Config(adcBuffer, adcBufferSize);
}

