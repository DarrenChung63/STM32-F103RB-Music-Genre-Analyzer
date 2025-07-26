# STM32-F103RB Music Genre Analyzer

## Overview
The **STM32-F103RB Music Genre Analyzer** is a real-time audio processing project built on the STM32F103RB microcontroller. It captures audio input through a 3.5mm TRS audio jack, processes the signal using ADC and DMA, and analyzes audio features to classify music into genres such as Classic, Pop, Hip-Hop, Metal, or Unknown. The analysis results are displayed on a SH1106 OLED display and printed to a serial terminal via USART2 (e.g., using PuTTY). This project showcases embedded systems programming, signal processing, and real-time audio analysis, leveraging the STM32 HAL library for hardware abstraction.

## Demo Video
![image](https://github.com/DarrenChung63/STM32-F103RB-Music-Genre-Analyzer/blob/main/assets/demo-Die_With_A%20_Smile.gif)  
Test music: [Lady Gaga, Bruno Mars - Die With a Smile](https://youtu.be/kPa7bsKwL-c?si=mSoSqakVT_kw9lkq) (0:44 to 1:19)

## Features
- **Audio Input**: Captures audio via a 3.5mm TRS jack connected to ADC1 (PA0).
- **Signal Processing**: Computes audio features including amplitude, zero-crossing rate (ZCR), energy variance, pitch stability, energy fluctuation, spectral roughness, energy ratio, and vocal-to-music ratio.
- **Genre Classification**: Classifies music into genres based on audio features.
- **Display Output**: Shows key audio features (ZCR, pitch stability, spectral roughness, vocal-to-music ratio) and detected genre on a SH1106 OLED display (128x64 pixels).
- **Serial Output**: Prints detailed analysis results over USART2 at 115200 baud rate to a serial terminal.
- **Delay Management**: Implements precise microsecond and millisecond delays using SysTick via HAL.
- **Hardware**: Built on the STM32F103RB microcontroller with a 72 MHz system clock.

## Hardware Requirements
- **Microcontroller**: STM32F103RB (e.g., Nucleo-F103RB or equivalent board).
- **Audio Input Assembly**: 3.5mm TRS audio jack, 4.7μF capacitor × 1, 10kΩ resistor × 2, Schottky diode × 2, LM358 two-stage amplifier module for weak signals.
- **Audio Source**: Device with a 3.5mm TRS audio output (e.g., phone, MP3 player).
- **Display**: SH1106 OLED (128x64 pixels, I2C interface).
- **Serial Interface**: USB-to-serial adapter or onboard ST-Link for USART2 communication.
- **Schematic**: Refer to `Schematic Prints.pdf` for wiring details, including connections for the 3.5mm TRS jack to PA0, USART2 (PA2 for TX), and SH1106 OLED (I2C on I2C1 via PB8 and PB9).

## Software Requirements
- **Development Environment**: STM32CubeIDE or Keil uVision for compiling and flashing the code.
- **Libraries**: STM32F1xx HAL library (If using STM32CubeIDE, this is generated automatically upon project creation; if using Keil uVision, download and install the STM32F1xx HAL library using STM32CubeMX).
- **Serial Terminal**: PuTTY or any terminal emulator supporting 115200 baud rate.
- **Toolchain**: ARM GCC or equivalent for building the project.

## Project Structure
```
├── src/
│   ├── main.c              # Main program loop and audio feature analysis
│   ├── audioin.c           # ADC and DMA configuration for audio input
│   ├── usart.c             # USART2 configuration for serial output
│   ├── delay.c             # Delay functions using SysTick (HAL-based)
│   ├── genreanalysis.c     # Audio feature extraction and genre
│   ├── sh1106.c            # SH1106 OLED display driver
│   ├── fonts.c             # Font definitions for SH1106 display
│   ├── stm32f1xx_it.c      # Interrupt handlers (e.g., SysTick, DMA)
│   ├── stm32f1xx_hal_msp.c # HAL MSP initialization
├── inc/
│   ├── audioin.h
│   ├── usart.h
│   ├── delay.h
│   ├── genreanalysis.h
│   ├── sh1106.h
│   ├── fonts.h
├── Schematic Prints.pdf    # Schematic diagram for hardware connections
└── README.md              
```

## Setup Instructions
1. **Hardware Setup**:
   - Refer to `Schematic Prints.pdf` for detailed wiring.
   - Ensure the STM32F103RB is powered and properly connected (e.g., via ST-Link).

2. **Software Setup**:
   - Open the project in STM32CubeIDE or your preferred IDE.
   - Configure the project to use the STM32F1xx HAL library.
   - Build and flash the code to the STM32F103RB.

3. **Serial Output**:
   - Open PuTTY or a similar terminal emulator.
   - Configure the serial port to 115200 Baud, 8N1 (8 data bits, no parity, 1 stop bit), no flow control.
   - Connect to the COM port corresponding to your STM32's USART2.

4. **Running the Analyzer**:
   - Play audio through the 3.5mm TRS input.
   - The program processes the audio, displays key features (ZCR, pitch stability, spectral roughness, vocal-to-music ratio) and the detected genre on the SH1106 OLED, and prints detailed results to the serial terminal every 250 μs.
   - The OLED shows a dynamic rectangle around the genre based on amplitude levels.
   - Detailed results are sent via USART2 for real-time monitoring.

## How It Works
- **Audio Capture**: ADC1 captures audio samples at 22.05 kHz using DMA for efficient transfer to a 2048-sample buffer.
- **Feature Extraction**: The program calculates:
  - **Amplitude**: Average signal magnitude.
  - **Zero-Crossing Rate (ZCR)**: Rate of sign changes for frequency estimation.
  - **Energy Variance**: Variance of signal energy.
  - **Pitch Stability**: Consistency of pitch across frames.
  - **Energy Fluctuation**: Variance of energy across frames.
  - **Spectral Roughness**: Measure of signal roughness.
  - **Energy Ratio**: Ratio of high-frequency to total energy.
  - **Vocal-to-Music Ratio**: Estimates vocal content vs. instrumental.
- **Genre Classification**: Uses threshold rules based on features (e.g., ZCR, variance, pitch stability, energy fluctuation, spectral roughness, vocal-to-music ratio) to classify music into genres.

## Example Output
**Serial Terminal (PuTTY):**
```
Amp: 123.456, ZCR: 0.015 Hz, Var: 150000.000, PitchStab: 0.850, EF: 500000.000, SR: 8.500, ER: 0.300, V2MR: 0.010 -> Pop
```

## Limitations
- The genre classification algorithm uses simple thresholds and may not accurately classify all music genres.
- The audio input assumes a clean signal; noisy inputs may affect accuracy.
- The system is tuned for a 22.05 kHz sample rate; changes require recalibration of parameters.
- The SH1106 OLED has limited resolution, restricting the amount of displayed information.

## Future Improvements
- Enhance genre classification with machine learning or advanced signal processing.
- Add support for multiple ADC channels for stereo input.
- Implement real-time visualization of waveforms or spectrograms on the OLED.
- Optimize DMA and ADC configurations for higher sample rates.
- Add user interface elements (e.g., buttons) for interactive control.

## License
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Acknowledgments
- Built using the STM32F1xx HAL library.
- Inspired by real-time audio processing techniques for embedded systems.
- SH1106 OLED driver adapted from open-source display libraries.
