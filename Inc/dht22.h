#ifndef DHT22_H
#define DHT22_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;

} dht22_t;


HAL_StatusTypeDef dht22_init(dht22_t *dev, GPIO_TypeDef *port, uint16_t pin);

HAL_StatusTypeDef dht22_read(dht22_t *dev, float *temp_c, float *hum_pct);

#ifdef __cplusplus
}
#endif

#endif