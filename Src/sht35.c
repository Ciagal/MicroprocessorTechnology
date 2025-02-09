#include "sht35.h"
#include "i2c.h"


#define SHT35_ADDRESS 0x45          // sht35 address
#define SHT35_MEAS_HIGHREP 0x2C06   // meas command

volatile uint8_t sht35_ready = 0;  
volatile uint8_t sht35_timer = 0;  


void SHT35_Init(void) 
{
    // soft reset
    I2C_WriteReg(SHT35_ADDRESS, 0x30, 0xA2);
}

uint8_t cmd_high = SHT35_MEAS_HIGHREP >> 8;
uint8_t cmd_low = SHT35_MEAS_HIGHREP & 0xFF;

void SHT35_StartMeasurement(void) 
{
    I2C_WriteReg(SHT35_ADDRESS, cmd_high, cmd_low);

    sht35_timer = 2;  	//SysTick interrupt  10 ms, so 2 x 10 ms = 20 ms
    sht35_ready = 0;   
}

uint8_t data[6]; 

uint8_t SHT35_Read(float *temperature, float *humidity) {
    if (!sht35_ready) return 0;  // if measurment not ready, not read

    I2C_ReadRegBlock(SHT35_ADDRESS, 0x00, 6, data);

    uint16_t temp_raw = (data[0] << 8) | data[1];
    uint16_t hum_raw = (data[3] << 8) | data[4];

    *temperature = -45.0 + (175.0 * temp_raw / 65535.0);
    *humidity = 100.0 * hum_raw / 65535.0;

    return 1;  //1, if reading was successfull
}
