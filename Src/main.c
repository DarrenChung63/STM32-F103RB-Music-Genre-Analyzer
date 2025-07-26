#include "stm32f1xx_hal.h" // HAL library include
#include "usart.h"
#include <stdio.h>
#include <string.h>
#include "delay.h"
#include "audioin.h"
#include "genreanalysis.h"
#include "sh1106.h"

#define ADC_BUFFER_SIZE 2048
uint16_t adcBuffer[ADC_BUFFER_SIZE];

// Declare global handles
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;
UART_HandleTypeDef huart2;

void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI; // Using HSI as per original
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2; // HSI_Div2
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9; // PLLMul_9 -> 72MHz
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        printf("Clock oscillator configuration failed.\n");
    }
    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK; // PLL as system clock
    RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2; // 36MHz
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1; // 72MHz

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) { // Flash latency for 72MHz - Uncommented
        printf("Clock initialization failed.\n");
    }
}

void LD2_GPIO_Config(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // Configure PA5 (LD2)
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}



int main(void) {
    HAL_Init();

    SystemClock_Config();

    LD2_GPIO_Config();
    USART2_UART_Init();
    Delay_Timer_Init();

    AudioIn_Init(adcBuffer, ADC_BUFFER_SIZE); // Initialize ADC and DMA
    SH1106_Init(); // Initialize SH1106 OLED
    SH1106_Clear();

    // Start ADC conversion and DMA transfer
    if (HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adcBuffer, ADC_BUFFER_SIZE) != HAL_OK) {
        printf("ADC conversion and DMA transfer failed.\n");
    }

    char SH1106_zcr[13];
	char SH1106_pstab[15];
//	char SH1106_var[17];
	char SH1106_specr[17];
	char SH1106_v2mr[13];
	char* SH1106_genre;
	float SH1106_amplitude;
    uint16_t loopCnt = 0;
    uint8_t displayRect = 0;
    while (1) {
        if (loopCnt == 100) loopCnt = 0;
        displayRect = (displayRect == 0);
    	// Ensure DMA transfer is complete or manage data flow based on circular buffer
        // For continuous sampling, DMA will be continuously filling the buffer.
        // We can process the entire buffer periodically.

        float amplitude = calculateAmplitude(adcBuffer, ADC_BUFFER_SIZE);
        float zcr = calculateZeroCrossingRate(adcBuffer, ADC_BUFFER_SIZE);
        float variance = calculateEnergyVariance(adcBuffer, ADC_BUFFER_SIZE);
        float pitchStability = calculatePitchStability(adcBuffer, ADC_BUFFER_SIZE);
        float energyFluctuation = calcEnergyFluctuation(adcBuffer, ADC_BUFFER_SIZE);
        float spectralRoughness = estimateSpectralRoughness(adcBuffer, ADC_BUFFER_SIZE);
        float energyRatio = calcEnergyRatio(adcBuffer, ADC_BUFFER_SIZE);
        float vocalToMusicRatio = calcVocalToMusicRatio(adcBuffer, ADC_BUFFER_SIZE);

        char* genre = classifyMusicStyle(amplitude, zcr, variance, pitchStability, energyFluctuation,
        		spectralRoughness, energyRatio, vocalToMusicRatio);

        printf("Amp: %.3f, ZCR: %.3f Hz, Var: %.3f, PitchStab: %.3f, EF: %.3f, SR: %.3f , ER: %.3f, "
        		"V2MR: %.3f -> %s\n", amplitude, zcr, variance, pitchStability,
					   energyFluctuation, spectralRoughness, energyRatio, vocalToMusicRatio, genre);

		if (loopCnt % 10 == 0) {
			snprintf(SH1106_zcr, sizeof(SH1106_zcr), "ZCR: %.3f", zcr);
			snprintf(SH1106_pstab, sizeof(SH1106_pstab), "PStab: %.3f", pitchStability);
//			snprintf(SH1106_var, sizeof(SH1106_var), "Var: %7.3f", variance);
			snprintf(SH1106_specr, sizeof(SH1106_specr), "SpecR: %2.3f", spectralRoughness);
			snprintf(SH1106_v2mr, sizeof(SH1106_v2mr), "V2MR: %.3f", vocalToMusicRatio);

		}

		SH1106_Puts(0, 0, SH1106_zcr, &Font_7x10, 1);
		SH1106_Puts(0, 11, SH1106_pstab, &Font_7x10, 1);
//		SH1106_Puts(0, 22, SH1106_var, &Font_7x10, 1);
		SH1106_Puts(0, 22, SH1106_specr, &Font_7x10, 1);
		SH1106_Puts(0, 33, SH1106_v2mr, &Font_7x10, 1);
		if (loopCnt % 10 == 0) SH1106_genre = genre;
		if (strcmp(SH1106_genre, "Classic") == 0) {
			SH1106_Puts(35, 49, "Classic", &Font_7x10, 1);
		}
		else if (strcmp(SH1106_genre, "Pop") == 0) {
			SH1106_Puts(49, 49, "Pop", &Font_7x10, 1);
		}
		else if (strcmp(SH1106_genre, "Hip-Hop") == 0) {
			SH1106_Puts(35, 49, "Hip-Hop", &Font_7x10, 1);
		}
		else if (strcmp(SH1106_genre, "Metal") == 0) {
			SH1106_Puts(42, 49, "Metal", &Font_7x10, 1);
		}
		else {
			SH1106_Puts(35, 49, "Unknown", &Font_7x10, 1);
		}



		// Show amplitude rectangle around the genre
		if (loopCnt % 5 == 0) SH1106_amplitude = amplitude;
		if (170 <= SH1106_amplitude && SH1106_amplitude < 340) {
			if (displayRect == 1) SH1106_DrawRectangle(33, 47, 57, 12, 1);
			else SH1106_DrawRectangle(33, 47, 57, 12, 0);
		}
		else if (340 <= SH1106_amplitude && SH1106_amplitude < 510) {
			SH1106_DrawRectangle(33, 47, 57, 12, 1);
			if (displayRect == 1) SH1106_DrawRectangle(31, 45, 61, 16, 1);
			else SH1106_DrawRectangle(31, 45, 61, 16, 0);
		}
		else if (SH1106_amplitude > 510) {
			SH1106_DrawRectangle(33, 47, 57, 12, 1);
			SH1106_DrawRectangle(31, 45, 61, 16, 1);
			if (displayRect == 1) SH1106_DrawRectangle(29, 43, 65, 20, 1);
			else SH1106_DrawRectangle(29, 43, 65, 20, 0);
		}

		SH1106_UpdateScreenPartial(0, 7);

		loopCnt++;
//		HAL_Delay(1);
		Delay_us(250);
//        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    }
}
