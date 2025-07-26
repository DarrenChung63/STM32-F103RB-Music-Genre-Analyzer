#include "delay.h"
#include "stm32f1xx_hal.h" // HAL library include

// Global variable not strictly needed for the new SysTick-based delay functions
// volatile uint32_t delay_us_ticks;

void Delay_Timer_Init(void) {
    // SysTick is typically configured as part of HAL_Init()
    // For specific timing, ensure SysTick clock source is HCLK (72MHz)
     HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000); // 1ms tick, if needed for general purposes
     HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
    // The following direct register access for microsecond delay is not standard HAL,
    // but mimics the previous SPL approach using SysTick as a countdown timer.
    // Ensure SysTick is configured to use HCLK (72MHz on NUCLEO-F103RB if SystemClock_Config sets it to 72MHz)
}

void Delay_ms(uint32_t ms) {
    HAL_Delay(ms); // Use the standard HAL_Delay function which uses SysTick
}

void Delay_us(uint32_t us) {
    if (us == 0) return;

    // Direct SysTick register manipulation for microsecond delay
    // This assumes SysTick is clocked at HCLK (72 MHz)
    uint32_t initial_value = SysTick->VAL;
    uint32_t reload_value = SysTick->LOAD;

    // Calculate the number of ticks for the desired microsecond delay
    // At 72 MHz, 1 us = 72 ticks
    uint32_t ticks_to_wait = us * (HAL_RCC_GetHCLKFreq() / 1000000UL); // Calculate based on HCLK

    if (ticks_to_wait == 0) ticks_to_wait = 1; // Ensure at least 1 tick delay for very small 'us'

    // Configure SysTick for one-shot counting
    SysTick->CTRL = 0; // Disable SysTick
    SysTick->LOAD = ticks_to_wait - 1; // Set reload value
    SysTick->VAL = 0; // Clear current value
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk; // Use HCLK, enable SysTick

    // Wait for the COUNTFLAG to be set (indicates reload occurred)
    while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0);

    SysTick->CTRL = 0; // Disable SysTick after delay
    SysTick->LOAD = reload_value; // Restore original reload value (if HAL_Delay uses it)
    SysTick->VAL = initial_value; // Restore original current value
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk; // Re-enable with interrupt for HAL_Delay
}
