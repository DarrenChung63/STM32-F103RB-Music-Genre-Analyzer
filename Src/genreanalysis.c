#include "stm32f1xx_hal.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define FRAME_SIZE      64
#define FRAME_SHIFT     32
#define SAMPLE_RATE     22050  // Adjust based on your ADC config

float calculateAmplitude(uint16_t *buffer, uint16_t size) {
    uint32_t sum = 0;
    for (uint16_t i = 0; i < size; i++) {
        int16_t sample = buffer[i] - 2048;
        sum += abs(sample);
    }
    return (float)sum / size;
}

float calculateZeroCrossingRate(uint16_t *buffer, uint16_t size) {
    int zeroCrossings = 0;
    for (uint16_t i = 1; i < size; i++) {
        int16_t current = buffer[i] - 2048;
        int16_t previous = buffer[i - 1] - 2048;
        if ((current > 0 && previous < 0) || (current < 0 && previous > 0)) {
            zeroCrossings++;
        }
    }
    return (float)zeroCrossings / size;
}

float calculateEnergyVariance(uint16_t *buffer, uint16_t size) {
    float mean = 0, meanSq = 0;
    for (uint16_t i = 0; i < size; i++) {
        float sample = (float)(buffer[i] - 2048);
        mean += sample;
        meanSq += sample * sample;
    }
    mean /= size;
    meanSq /= size;
    return meanSq - mean * mean;
}

// Pitch estimation for a frame using zero-crossing count
float estimateFrameFrequency(uint16_t *buffer, uint16_t start, uint16_t frameSize) {
    uint16_t zeroCrossings = 0;
    for (uint16_t i = start + 1; i < start + frameSize; i++) {
        int16_t prev = buffer[i - 1] - 2048;
        int16_t curr = buffer[i] - 2048;
        if ((prev > 0 && curr < 0) || (prev < 0 && curr > 0))
            zeroCrossings++;
    }
    float freq = (zeroCrossings * SAMPLE_RATE) / (2.0f * frameSize);
    return freq;
}


float calculatePitchStability(uint16_t *buffer, uint16_t size) {
    const uint16_t numFrames = (size - FRAME_SIZE) / FRAME_SHIFT + 1;
    if(numFrames < 2) return 0.0f;

    float freqs[numFrames];
    for(uint16_t i = 0; i < numFrames; i++){
        uint16_t start = i * FRAME_SHIFT;
        freqs[i] = estimateFrameFrequency(buffer, start, FRAME_SIZE);
    }
    float sum = 0.0f;
    for(uint16_t i = 0; i < numFrames; i++){
        sum += freqs[i];
    }
    float avgFreq = sum / numFrames;
    float diffSum = 0.0f;
    for(uint16_t i = 1; i < numFrames; i++){
        diffSum += fabsf(freqs[i] - freqs[i - 1]);
    }
    float avgDiff = diffSum / (numFrames - 1);
    // Compute relative pitch fluctuation (if avgFreq is very low, default to high fluctuation)
    float relFluctuation = avgFreq > 0 ? avgDiff / avgFreq : 1.0f;
    // Derive stability score: lower fluctuation gives higher stability (range 0 to 1)
    float pitchStability = 1.0f / (1.0f + relFluctuation);
    return pitchStability;
}


float calcEnergyFluctuation(uint16_t *buffer, uint16_t size) {
    uint16_t numFrames = size / FRAME_SIZE;
    float energies[64] = {0}; // Up to 64 frames safely

    for (uint16_t i = 0; i < numFrames; i++) {
        float sum = 0.0f;
        for (uint16_t j = 0; j < FRAME_SIZE; j++) {
            int16_t sample = buffer[i * FRAME_SIZE + j] - 2048;
            sum += sample * sample;
        }
        energies[i] = sum / FRAME_SIZE;
    }

    float mean = 0, meanSq = 0;
    for (uint16_t i = 0; i < numFrames; i++) {
        mean += energies[i];
        meanSq += energies[i] * energies[i];
    }
    mean /= numFrames;
    meanSq /= numFrames;
    return meanSq - mean * mean;
}

float estimateSpectralRoughness(uint16_t *buffer, uint16_t size) {
    float roughness = 0.0f;
    for (uint16_t i = 2; i < size; i++) {
        int16_t d1 = buffer[i - 1] - buffer[i - 2];
        int16_t d2 = buffer[i] - buffer[i - 1];
        roughness += fabsf((float)(d2 - d1));
    }
    return roughness / size;
}


float calcEnergyRatio(uint16_t *buffer, uint16_t size) {
    uint32_t totalEnergy = 0;
    uint32_t highFreqEnergy = 0;
    for (uint16_t i = 1; i < size; i++) {
        int16_t diff = buffer[i] - buffer[i - 1];
        totalEnergy += abs(buffer[i] - 2048);
        highFreqEnergy += abs(diff);
    }
    if (totalEnergy == 0) return 0.0f;
    return (float)highFreqEnergy / totalEnergy;
}

float calcVocalToMusicRatio(uint16_t *buffer, uint16_t size) {
    float vocalEnergy = 0.0f;
    float musicEnergy = 0.0f;
    for (uint16_t i = 0; i < size; i++) {
        int16_t sample = buffer[i] - 2048;
        if (sample > 250 || sample < -250) {
            musicEnergy += fabsf((float)sample); // Use fabsf for float absolute value
        } else {
            vocalEnergy += fabsf((float)sample);
        }
    }
    if ((vocalEnergy + musicEnergy) == 0) return 0.0f;
    return vocalEnergy / (vocalEnergy + musicEnergy);
}

char* classifyMusicStyle(float amplitude, float zcr, float variance, float pitchStability, float energyFluctuation,
		float spectralRoughness, float energyRatio, float vocalToMusicRatio) {
    if (zcr < 0.7f && variance < 100000)  {
        return "Classic";
    }
    if (variance > 200 && pitchStability < 0.6f && vocalToMusicRatio <= 0.4f && energyFluctuation <= 6e11) {
        return "Pop";
    }
    if (spectralRoughness <= 9) {
		return "Hip-Hop";
	}
    if (spectralRoughness > 9 && energyFluctuation >= 1e6) {
		return "Metal";
	}
    if (spectralRoughness <= 9) {
        return "Hip-Hop";
    }

    return "Unknown";
}
