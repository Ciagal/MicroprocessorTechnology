#ifndef SHT35_H
#define SHT35_H

#include "MKL05Z4.h"


void SHT35_Init(void);
void SHT35_StartMeasurement(void);
uint8_t SHT35_Read(float *temperature, float *humidity);

#endif /* SHT35_H */
