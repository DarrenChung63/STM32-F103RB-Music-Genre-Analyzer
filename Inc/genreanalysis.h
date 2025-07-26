#ifndef __GENREANALYSIS_H
#define __GENREANALYSIS_H

float calculateAmplitude(uint16_t *buffer, uint16_t size);
float calculateZeroCrossingRate(uint16_t *buffer, uint16_t size);
float calculateEnergyVariance(uint16_t *buffer, uint16_t size);
float estimateFrameFrequency(uint16_t *buffer, uint16_t start, uint16_t frameSize);
float calculatePitchStability(uint16_t *buffer, uint16_t size);
float calcEnergyFluctuation(uint16_t *buffer, uint16_t size);
float estimateSpectralRoughness(uint16_t *buffer, uint16_t size);
float calcEnergyRatio(uint16_t *buffer, uint16_t size);
float calcVocalToMusicRatio(uint16_t *buffer, uint16_t size);
char* classifyMusicStyle(float amplitude, float zcr, float variance, float pitchStability,
		float energyFluctuation, float spectralRoughness, float energyRatio, float vocalToMusicRatio);
#endif
