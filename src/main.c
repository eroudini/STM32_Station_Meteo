#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdio.h>
#include "bmp280.h"


I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart2;
bmp280_t bmp;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);

static void MX_I2C1_Init(void);
static void Error_Handler(const char *why);
static void I2C1_Scan(void);

static void uart_print(const char *s)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)s, (uint16_t)strlen(s), HAL_MAX_DELAY);
}

static void uart_println(const char *s)
{
    uart_print(s);
    uart_print("\r\n");
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_USART2_UART_Init();
    MX_I2C1_Init();

    uart_print("Init BMP280...");
    if (bmp280_init(&bmp, &hi2c1, 0x76) != HAL_OK)
    {
        uart_println("BMP280 @0x76 KO, on tente 0x77...");
        if(bmp280_init(&bmp, &hi2c1, 0x77) != HAL_OK) {
            Error_Handler("BMP280 Non detecté");
        }
    }

    uart_println("BMP280 Ok");
    

    uart_print("=== Boot Ok (STM32F407 @168Mhz) ===");
    uart_println("Test : scan I2C...");
    I2C1_Scan();

    while (1)
    {

        // LECTURE DU CAPTEUR BMP280 toutes les 2 secondes
        
        static uint32_t t0 = 0;
        if(HAL_GetTick() - t0 >= 2000) {
            t0 = HAL_GetTick();

            float tc = 0.0f, pa = 0.0f;
            if(bmp280_read_fixed(&bmp, &tc, &pa) == HAL_OK) {
                char line[96];
                snprintf(line, sizeof(line), "CSV: %.2f;%.2f\n", tc, pa);
                uart_print(line);
            } else {
                uart_println("BMP280 Lecture KO");
            }
        }


        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_8);
        HAL_Delay(500);
    }
    
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
        /* Sélection des sources d'horloges + PLL  */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON; // active HSE -> Quartz externe
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON; // Allume la PLL -> Circuit multiplicateur
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE; // PLL alimentée par HSE=8 MHz
    RCC_OscInitStruct.PLL.PLLM = 8; // 8MHZ / 8 = 1 MHz (VCO_In)
    RCC_OscInitStruct.PLL.PLLN = 336; // 1 * 336 = 336 MHz (VCO_out)
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2; // 336/2 = 168 MHz (SYSCLK)
    RCC_OscInitStruct.PLL.PLLQ = 7; // 336/7 ≈ 48 MHz (USB/SDIO)
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler("HAL_RCC_OscConfig failed");

        /* Configuration des bus AHB/APB */
    
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
    {
        Error_Handler("HAL_RCC_ClockConfig failed");

    }
    
    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

    }
    
}

        /* INITIALISATION I2C1 (PB6/PB7) */

static void MX_I2C1_Init(void)
{
    // activer l'horloge du periphérique I2C1 

    __HAL_RCC_I2C1_CLK_ENABLE();

    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    {
        Error_Handler("HAL_I2C_Init failed") ; 
    }
    

}

     //INITIALISATION USART2 (PA2/PA3), 115200 8N1

static void MX_USART2_UART_Init(void)
{
    // activer l'horloge du periphérique USART2 

    __HAL_RCC_USART2_CLK_ENABLE();

    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
        Error_Handler("HAL_UART_Init failed");
    }
    
}

    //INITIALISATION GPIO (PA8, PA2/3, PB6/7)

static void MX_GPIO_Init(void)
{

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    // horloges des ports GPIO utiliser

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
        // configuration de la broche PA8 : DHT22 
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);


        // config de PA2/PA3 : USART2

    GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        // config de PB6/PB7 : I2C1 

    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    
}

        // Init bas-niveau : alimente et configure les périphériques I2C1 / USART2


void HAL_I2C_MspInit(I2C_HandleTypeDef* i2cHandle)
{
    if (i2cHandle->Instance == I2C1)
    {
        __HAL_RCC_I2C1_CLK_ENABLE(); 
    }
    
}

void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
    if (huart->Instance == USART2)
    {
        __HAL_RCC_USART2_CLK_ENABLE();
    }
    
}
    // Vérifie la présence des capteurs I2C sur le bus (détection d’adresses ACK)

static void I2C1_Scan(void)
{
    char line[64];
    uint8_t found = 0;

    for(u_int16_t addr = 0x08; addr <= 0x77; addr++) 
    {

        if (HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(addr<<1), 1, 5) == HAL_OK)
        {
            snprintf(line, sizeof(line), "I2C : device @ 0x%02lX\r\n", (unsigned long)addr);
            uart_print(line);
            found = 1;
        }
        
    }

    if (!found)
    {
        uart_println("I2C : Aucun peripherique detecté ");
    }
    

}



    // Handler centralisé 

static void Error_Handler(const char *why) 
{
    
    uart_print("ERROR: ");
    uart_println(why);

    for(;;) {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_8);
        HAL_Delay(120);
    }
}
