#ifndef BMP280_H
#define BMP280_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif 


/* Addressage I2C : 0x76*/

#ifndef BMP280_I2C_ADDR
#define BMP280_I2C_ADDR
#endif

/*Registres principaux */

#define BMP280_REG_CHIPID 0XD0
#define BMP280_REG_RESET 0xE0
#define BMP280_REG_STATUS 0xF3
#define BMP280_REG_CTRL_MEAS 0xF4
#define BMP280_REG_CONFIG 0xF5
#define BMP280_REG_PRESS_MSB 0xF7


/* Calibrations T/P */

#define BMP280_REG_CALIB_START 0x88

/* Structure des coefficents de calibrations */

typedef struct {
    
    uint16_t dig_T1; int16_t dig_T2; int16_t dig_T3;
    uint16_t dig_P1; int16_t dig_P2; int16_t dig_P3; int16_t dig_P4;
    int16_t dig_P5;  int16_t dig_P6; int16_t dig_P7; int16_t dig_P8; int16_t dig_P9;
    int32_t  t_fine;
} bmp280_calib_t;

/* Handle du driver */

typedef struct  {
    I2C_HandleTypeDef *i2c;
    uint8_t addr7;
    bmp280_calib_t calib;
}bmp280_t;

/*API*/

HAL_StatusTypeDef bmp280_init(bmp280_t *dev, I2C_HandleTypeDef *hi2c, uint8_t addr7);
HAL_StatusTypeDef bmp280_read_fixed(bmp280_t *dev, float *temp_c, float *press_pa);

#ifdef __cplusplus
}
#endif
#endif
