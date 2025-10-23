#include "bmp280.h"
#include <string.h>

/*Helpers I2C*/

static HAL_StatusTypeDef reg_read(bmp280_t *dev, uint8_t reg, uint8_t *buf, uint16_t len) 
{
    return HAL_I2C_Mem_Read(dev->i2c, (dev->addr7 << 1), reg, 1, buf, len, HAL_MAX_DELAY);
}
static HAL_StatusTypeDef reg_write(bmp280_t *dev, uint8_t reg, uint8_t val) {
    return HAL_I2C_Mem_Write(dev->i2c, (dev->addr7 << 1), reg, 1, &val, 1, HAL_MAX_DELAY);
}

/*Lecture et stockage des coef de calibrage*/

static HAL_StatusTypeDef read_calib(bmp280_t *dev) 
{
    uint8_t raw[24];
    if(reg_read(dev, BMP280_REG_CALIB_START, raw, sizeof(raw)) != HAL_OK) return HAL_ERROR;

    dev->calib.dig_T1 = (uint16_t)(raw[1]<<8 | raw[0]);
    dev->calib.dig_T2 = (uint16_t)(raw[3]<<8 | raw[2]);
    dev->calib.dig_T3 = (uint16_t)(raw[5]<<8 | raw[4]);

    dev->calib.dig_P1 = (uint16_t)(raw[7]<<8 | raw[6]);
    dev->calib.dig_P2 = (uint16_t)(raw[9]<<8 | raw[8]);
    dev->calib.dig_P3 = (uint16_t)(raw[11]<<8 | raw[10]);
    dev->calib.dig_P4 = (uint16_t)(raw[13]<<8 | raw[12]);
    dev->calib.dig_P5 = (uint16_t)(raw[15]<<8 | raw[14]);
    dev->calib.dig_P6 = (uint16_t)(raw[17]<<8 | raw[16]);
    dev->calib.dig_P7 = (uint16_t)(raw[19]<<8 | raw[18]);
    dev->calib.dig_P8 = (uint16_t)(raw[21]<<8 | raw[20]);
    dev->calib.dig_P9 = (uint16_t)(raw[23]<<8 | raw[22]);

    return HAL_OK;
}

/*Compensation temperature)*/

static int32_t compensate_T(bmp280_t *dev, int32_t adc_T, float *temp_c) 
{
    int32_t var1 = ((((adc_T >> 3 ) - ((int32_t)dev->calib.dig_T1 << 1))) * ((int32_t)dev->calib.dig_T2)) >> 11;
    int32_t var2 = (((((adc_T >> 4) - ((int32_t)dev->calib.dig_T1)) * ((adc_T >> 4) - ((int32_t)dev->calib.dig_T1))) >> 12) * ((int32_t)dev->calib.dig_T3)) >> 14;

    dev->calib.t_fine = var1 + var2;
    int32_t T = (dev->calib.t_fine * 5 + 128) >> 8;
    if (temp_c) *temp_c = T / 100.0f;
    return T;
}

/*Compensation pression*/

static uint32_t compensate_P(bmp280_t *dev, int32_t adc_P, float *press_pa) 
{
    int64_t var1, var2, p;
    var1 = ((int64_t)dev->calib.t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dev->calib.dig_P6;
    var2 = var2 + ((var1 * (int64_t)dev->calib.dig_P5) << 17);
    var2 = var2 + (((int64_t)dev->calib.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dev->calib.dig_P3) >> 8) + ((var1 * (int64_t)dev->calib.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1) * ((int64_t)dev->calib.dig_P1)) >> 33; 

    if(var1 == 0) { if (press_pa) * press_pa = 0; return 0;}

    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dev->calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dev->calib.dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)dev->calib.dig_P7) << 4);

    if (press_pa) *press_pa = (float)p / 256.0f; // Pa
    return (uint32_t)p;
}

/*Init capteur : verif ID, lecture calib, config mesure*/

HAL_StatusTypeDef bmp280_init(bmp280_t *dev, I2C_HandleTypeDef *hi2c, uint8_t addr7)
{
    dev->i2c = hi2c;
    dev->addr7 = addr7;

    uint8_t id = 0;

    if(reg_read(dev, BMP280_REG_CHIPID, &id, 1) != HAL_OK) return HAL_ERROR;
    if(id != 0x58) return HAL_ERROR;

    if(read_calib(dev) != HAL_OK) return HAL_ERROR;

    if(reg_write(dev, BMP280_REG_CTRL_MEAS, 0x27) != HAL_OK) return HAL_ERROR;
    if(reg_write(dev, BMP280_REG_CONFIG, 0x90) != HAL_OK) return HAL_ERROR;

    return HAL_OK;

}

/*Lecture brute + compensation -> temperature et pression*/

HAL_StatusTypeDef bmp280_read_fixed(bmp280_t *dev, float *temp_c, float *press_pa)
{
    uint8_t buf[6];
    if (reg_read(dev, BMP280_REG_PRESS_MSB, buf, 6) != HAL_OK) return HAL_ERROR;

    int32_t adc_P = ((int32_t)buf[0] << 12) | ((int32_t)buf[1] << 4 | (buf[2] >> 4));
    int32_t adc_T = ((int32_t)buf[3] << 12) | ((int32_t)buf[4] << 4 | (buf[5] >> 4));

    compensate_T(dev, adc_T, temp_c);
    compensate_P(dev, adc_P, press_pa);
    return HAL_OK;
    
}

