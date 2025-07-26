#include "stm32f1xx_hal.h"
#include "sh1106.h"
#include <stdio.h>
#include <string.h>

#define SH1106_I2C_ADDR 0x78
#define SH1106_WIDTH 128
#define SH1106_HEIGHT 64

I2C_HandleTypeDef hi2c1;
DMA_HandleTypeDef hdma_i2c1_tx;

static uint8_t SH1106_Buffer[SH1106_WIDTH * SH1106_HEIGHT / 8];

void I2C1_Init(void) {
	hi2c1.Instance = I2C1;
	hi2c1.Init.ClockSpeed = 400000;
	hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK){
		printf("I2C initialization failed\n");
	}

	__HAL_RCC_DMA1_CLK_ENABLE();
	hdma_i2c1_tx.Instance = DMA1_Channel6;
	hdma_i2c1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
	hdma_i2c1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma_i2c1_tx.Init.MemInc = DMA_MINC_ENABLE;
	hdma_i2c1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma_i2c1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	hdma_i2c1_tx.Init.Mode = DMA_NORMAL;
	hdma_i2c1_tx.Init.Priority = DMA_PRIORITY_HIGH;
	if (HAL_DMA_Init(&hdma_i2c1_tx) != HAL_OK) {
		printf("I2C DMA initialization failed\n");
	}
	__HAL_LINKDMA(&hi2c1, hdmatx, hdma_i2c1_tx);
}

void SH1106_WriteCommand(uint8_t cmd) {
    uint8_t data[2] = {0x00, cmd};
    HAL_I2C_Master_Transmit(&hi2c1, SH1106_I2C_ADDR, data, 2, HAL_MAX_DELAY);
}

void SH1106_Init(void) {
	I2C1_Init();
	HAL_Delay(200);
    SH1106_WriteCommand(0xAE); // Display off
    SH1106_WriteCommand(0x20); // Set Memory Addressing Mode
    SH1106_WriteCommand(0x00); // Horizontal
    SH1106_WriteCommand(0xB0); // Page Start
    SH1106_WriteCommand(0xC8); // COM Output Scan Direction (reverse)
//    SH1106_WriteCommand(0xC0); // COM Output Scan Direction (normal) - CHANGE THIS LINE
    SH1106_WriteCommand(0x00); // Low Column
    SH1106_WriteCommand(0x10); // High Column
    SH1106_WriteCommand(0x40); // Start Line
    SH1106_WriteCommand(0x81); // Contrast
    SH1106_WriteCommand(0x7F);
    SH1106_WriteCommand(0xA1); // Segment remap
    SH1106_WriteCommand(0xA6); // Normal display
    SH1106_WriteCommand(0xA8); // Multiplex ratio
    SH1106_WriteCommand(0x3F);
    SH1106_WriteCommand(0xA4); // Display RAM
    SH1106_WriteCommand(0xD3); SH1106_WriteCommand(0x00); // Offset
    SH1106_WriteCommand(0xD5); SH1106_WriteCommand(0xF0); // Clock divide
    SH1106_WriteCommand(0xD9); SH1106_WriteCommand(0x22); // Pre-charge
    SH1106_WriteCommand(0xDA); SH1106_WriteCommand(0x12); // COM Pins
    SH1106_WriteCommand(0xDB); SH1106_WriteCommand(0x20); // VCOMH
    SH1106_WriteCommand(0x8D); SH1106_WriteCommand(0x14); // Charge Pump
    SH1106_WriteCommand(0xAF); // Display ON
    printf("SH1106 initialization success.\n");

	// Display text and rectangle
	SH1106_Puts(2, 0, "Hello guys,", &Font_7x10, 1);
	SH1106_Puts(2, 12, "I'm Darren.", &Font_7x10, 1);
	SH1106_DrawRectangle(1, 0, 100, 22, 1);
	SH1106_UpdateScreen();
}

void SH1106_UpdateScreen(void) {
    for (uint8_t page = 0; page < 8; page++) {
        SH1106_WriteCommand(0xB0 + page);
        SH1106_WriteCommand(0x02); // Start at column 2 (SH1106 has 2-pixel offset)
        SH1106_WriteCommand(0x10);
        HAL_I2C_Mem_Write(&hi2c1, SH1106_I2C_ADDR, 0x40, 1,
                          &SH1106_Buffer[SH1106_WIDTH * page],
                          SH1106_WIDTH, HAL_MAX_DELAY);
    }
}

void SH1106_UpdateScreenPartial(uint8_t start_page, uint8_t end_page) {
    if (start_page > 7 || end_page > 7 || start_page > end_page) {
        return; // Invalid page range
    }
    for (uint8_t page = start_page; page <= end_page; page++) {
        SH1106_WriteCommand(0xB0 + page);
        SH1106_WriteCommand(0x02);
        SH1106_WriteCommand(0x10);
        HAL_I2C_Mem_Write(&hi2c1, SH1106_I2C_ADDR, 0x40, 1,
                          &SH1106_Buffer[SH1106_WIDTH * page],
                          SH1106_WIDTH, HAL_MAX_DELAY);
    }
    for (uint8_t page = 0; page <= 7; page++) { // Pages 0-7 cover y=0 to 63
		memset(&SH1106_Buffer[page * SH1106_WIDTH], 0x00, SH1106_WIDTH);
	}
}

void SH1106_Clear(void) {
    memset(SH1106_Buffer, 0x00, sizeof(SH1106_Buffer));
    SH1106_UpdateScreen();
}

void SH1106_Puts(uint8_t x, uint8_t y, char* str, FontDef_t* Font, uint8_t color) {
    while (*str) {
        // Check bounds to prevent buffer overflow for the character's width
        if (x + Font->FontWidth > SH1106_WIDTH) {
            // If the current character goes beyond the screen width,
            // you might want to wrap to the next line or stop.
            // For now, let's just break.
            break;
        }

        // Check for character validity
        if (*str < 32 || *str > 126) { // Skip unsupported characters
            str++;
            continue;
        }

        // Calculate the index in the font data (each character's font data starts after the previous one)
        // Ensure your FontDef_t.data is structured correctly for this access.
        // For a 7x10 font, each character is FontWidth bytes.
        uint32_t char_offset_in_font_data = (*str - 32) * Font->FontWidth;

        // Process each column of the character
        for (uint8_t col = 0; col < Font->FontWidth; col++) {
            // Get the byte representing the current column of the character
            uint8_t column_data = Font->data[char_offset_in_font_data + col];

            // Iterate through each pixel (row) in the current character column
            for (uint8_t pixel_row = 0; pixel_row < Font->FontHeight; pixel_row++) {
                // Calculate the absolute Y-coordinate on the screen for this pixel
                uint8_t absolute_y = y + pixel_row;

                // Check bounds for absolute Y-coordinate
                if (absolute_y >= SH1106_HEIGHT) {
                    // If the character extends beyond the screen height, stop plotting this character.
                    // This prevents writing out of bounds for the display.
                    break;
                }

                // Calculate the page number for this pixel
                uint8_t page = absolute_y / 8;

                // Calculate the bit offset within the byte for this pixel
                // If 0xC0 (normal COM scan) is used, bit 0 is the top pixel of the 8-pixel page.
                uint8_t bit_offset_in_byte = absolute_y % 8; // CHANGE: Removed 7 - for normal COM scan

                // Calculate the linear buffer index
                // SH1106 has a 2-pixel column offset from the start of the RAM (0x00 to 0x7F).
                // Your SH1106_UpdateScreen sends from column 2 (0x02).
                // So, the buffer index in relation to the physical display columns might need to account for this.
                // Assuming CurrentX is already adjusted for this offset in SH1106_GotoXY or here.
                uint32_t buffer_index = page * SH1106_WIDTH + x + col;

                // Ensure buffer_index is within the overall buffer bounds
                if (buffer_index >= sizeof(SH1106_Buffer)) {
                    continue; // Skip if out of buffer bounds (should ideally not happen with proper bounds checking)
                }

                // Determine if the current pixel in the font data is set
                // (column_data >> pixel_row) & 0x01: Assumes font data has LSB as top pixel and MSB as bottom.
                if ((column_data >> pixel_row) & 0x01) {
                    // Set the pixel
                    SH1106_Buffer[buffer_index] |= (1 << bit_offset_in_byte);
                } else {
                    // Clear the pixel
                    SH1106_Buffer[buffer_index] &= ~(1 << bit_offset_in_byte);
                }
            }
        }

        x += Font->FontWidth + 1; // Add 1-pixel spacing between characters
        str++;
    }
}

void SH1106_DrawRectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color) {
    if (x >= SH1106_WIDTH || y >= SH1106_HEIGHT || width == 0 || height == 0) {
        return;
    }
    if (x + width > SH1106_WIDTH) {
        width = SH1106_WIDTH - x;
    }
    if (y + height > SH1106_HEIGHT) {
        height = SH1106_HEIGHT - y;
    }

    // Invert y-coordinates for reverse COM scan
//    uint8_t top_y = SH1106_HEIGHT - 1 - y;
    uint8_t top_y = y;
//    uint8_t bottom_y = SH1106_HEIGHT - 1 - (y + height - 1);
    uint8_t bottom_y = (y + height - 1);

    // Draw top and bottom lines (horizontal)
    for (uint8_t i = x; i < x + width; i++) {
        // Top line
        uint8_t page_top = top_y / 8;
        uint8_t bit_top = top_y % 8; // Normal bit order due to y-inversion
        uint32_t index_top = page_top * SH1106_WIDTH + i;
        if (index_top < sizeof(SH1106_Buffer)) {
            if (color) {
                SH1106_Buffer[index_top] |= (1 << bit_top);
            } else {
                SH1106_Buffer[index_top] &= ~(1 << bit_top);
            }
        }

        // Bottom line
        uint8_t page_bottom = bottom_y / 8;
        uint8_t bit_bottom = bottom_y % 8;
        uint32_t index_bottom = page_bottom * SH1106_WIDTH + i;
        if (index_bottom < sizeof(SH1106_Buffer)) {
            if (color) {
                SH1106_Buffer[index_bottom] |= (1 << bit_bottom);
            } else {
                SH1106_Buffer[index_bottom] &= ~(1 << bit_bottom);
            }
        }
    }

    // Draw left and right lines (vertical)
    for (uint8_t j = top_y; j <= bottom_y; j++) {
        // Left line
        uint8_t page_left = j / 8;
        uint8_t bit_left = j % 8;
        uint32_t index_left = page_left * SH1106_WIDTH + x;
        if (index_left < sizeof(SH1106_Buffer)) {
            if (color) {
                SH1106_Buffer[index_left] |= (1 << bit_left);
            } else {
                SH1106_Buffer[index_left] &= ~(1 << bit_left);
            }
        }

        // Right line
        uint8_t page_right = j / 8;
        uint8_t bit_right = j % 8;
        uint32_t index_right = page_right * SH1106_WIDTH + (x + width - 1);
        if (index_right < sizeof(SH1106_Buffer)) {
            if (color) {
                SH1106_Buffer[index_right] |= (1 << bit_right);
            } else {
                SH1106_Buffer[index_right] &= ~(1 << bit_right);
            }
        }
    }
}
