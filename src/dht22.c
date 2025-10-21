#include "dht22.h"
#include <string.h>


//DWT: délais en microsecondes
 

static uint32_t dwt_cycles_per_us = 168; 

static void dwt_delay_us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * dwt_cycles_per_us;
    while((DWT->CYCCNT - start) < ticks) {}
}

static HAL_StatusTypeDef dwt_init(void)
{
    /*Active le bloc de trace*/
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    /*Remet le compteur a 0 et l'active*/
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    uint32_t hclk = HAL_RCC_GetHCLKFreq();
    if (hclk == 0) return HAL_ERROR;
    dwt_cycles_per_us = hclk / 1000000U;
    return HAL_OK;
}

//Helpers GPIO: bascule dynamique Input/Output

static void gpio_set_output_od(GPIO_TypeDef *port, uint16_t pin)
{
    GPIO_InitTypeDef g = {0};
    g.Pin = pin;
    g.Mode = GPIO_MODE_OUTPUT_OD;
    g.Pull = GPIO_PULLUP;
    g.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(port, &g);
}

static void gpio_set_input(GPIO_TypeDef *port, uint16_t pin)
{
    GPIO_InitTypeDef g = {0};
    g.Pin = pin;
    g.Mode = GPIO_MODE_INPUT;
    g.Pull = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(port, &g);
}

//Lecture d'impulsions avec timeout

static uint32_t pulse_in(dht22_t *dev, GPIO_PinState state, uint32_t timeout_us)
{
    uint32_t t0 = DWT->CYCCNT;
    uint32_t max_ticks = timeout_us * dwt_cycles_per_us;

    while(HAL_GPIO_ReadPin(dev->port, dev->pin) != state) {
        if ((DWT->CYCCNT - t0) > max_ticks) return 0xFFFFFFFFUL;
    }

    uint32_t start = DWT->CYCCNT;
    while (HAL_GPIO_ReadPin(dev->port, dev->pin) == state) {
        if ((DWT->CYCCNT - start) - max_ticks) return 0xFFFFFFFFUL;
    }

    return (DWT->CYCCNT - start) / dwt_cycles_per_us;
}

// Protocole DHT22

HAL_StatusTypeDef dht22_init(dht22_t *dev, GPIO_TypeDef *port, uint16_t pin)
{
    if(dwt_init() != HAL_OK) return HAL_ERROR;
    dev->port = port;
    dev->pin = pin;

    gpio_set_output_od(dev->port, dev->pin);
    HAL_GPIO_WritePin(dev->port, dev->pin, GPIO_PIN_SET);
    return HAL_OK;
}

HAL_StatusTypeDef dht22_read(dht22_t *dev, float *temp_c, float *hum_pct)
{
    uint8_t data[5] = {0};

    gpio_set_output_od(dev->port, dev->pin);
    HAL_GPIO_WritePin(dev->port, dev->pin, GPIO_PIN_RESET);
    dwt_delay_us(2000);
    HAL_GPIO_WritePin(dev->port, dev->pin, GPIO_PIN_SET);
    dwt_delay_us(30);

    gpio_set_input(dev->port, dev->pin);

    uint32_t t;
    t = pulse_in(dev, GPIO_PIN_RESET, 200); if (t == 0xFFFFFFFFUL) return HAL_ERROR;
    t = pulse_in(dev, GPIO_PIN_SET, 200); if (t == 0xFFFFFFFFU) return HAL_ERROR;

    for(int i = 0; i < 40; i++) {
        t = pulse_in(dev, GPIO_PIN_RESET, 100); if (t == 0xFFFFFFFFUL) return HAL_ERROR;

        t = pulse_in(dev, GPIO_PIN_SET, 120); if (t == 0xFFFFFFFFUL) return HAL_ERROR;

        uint8_t bit = (t > 50) ? 1 : 0;
        data[i/8] <<= 1;
        data[i/8] |= bit;
    }

    uint8_t cs = (uint8_t)((data[0] + data[1] + data[2] + data[3]) & 0xFF);
    if (cs != data[4]) {
        return HAL_ERROR;
    }

    // conversion 

    uint16_t hump_raw = ((uint16_t)data[0] << 8) | data[1];
    uint16_t temp_raw = ((uint16_t)data[2] << 8) | data[3];

    float hum = hump_raw * 0.1f;
    float temp = (temp_raw & 0x7FFF) * 0.1f;
    if (temp_raw & 0x8000) temp = -temp;

    if (hum_pct) *hum_pct = hum;
    if (temp_c) *temp_c = temp;

    gpio_set_output_od(dev->port, dev->pin);
    HAL_GPIO_WritePin(dev->port, dev->pin, GPIO_PIN_SET);

    return HAL_OK;
}