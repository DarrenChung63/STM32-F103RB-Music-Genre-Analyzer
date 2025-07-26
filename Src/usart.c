#include "usart.h"
#include <stdio.h> // For __FILE__, __LINE__ if using printf

extern UART_HandleTypeDef huart2;

void USART2_UART_Init(void) {
	__HAL_RCC_USART2_CLK_ENABLE();
	huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX; // Only TX is configured in original
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
//    huart2.Init.OverSampling = UART_OVERSAMPLING_16; // Default for many STM32CubeMX projects

    if (HAL_UART_Init(&huart2) != HAL_OK) {
        // Initialization Error
        printf("UART clock configure failed.\n");
    }
}


int _write(int file, char *ptr, int len) {
    if (HAL_UART_Transmit(&huart2, (uint8_t*)ptr, len, 1000) == HAL_OK) {
        return len;
    }
    return -1; // Indicate error
}
