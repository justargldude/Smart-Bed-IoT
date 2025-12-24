/**
 * @file drv_loadcell.c
 * @brief Implementation of HX711 Driver.
 */

#include "drv_loadcell.h"

// Private Helper Functions

/**
 * @brief Microsecond delay using DWT (Data Watchpoint and Trace).
 * Requires loadcell_init_dwt() to be called once.
 */
static inline void delay_us(uint32_t us) 
{
    uint32_t start = DWT->CYCCNT;
    uint32_t cycles = us * (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - start) < cycles);
}

// Public Functions 

void loadcell_init_dwt(void) 
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

HAL_StatusTypeDef loadcell_init(loadcell_t *sensor, GPIO_TypeDef *dout_port, uint16_t dout_pin, GPIO_TypeDef *sck_port, uint16_t sck_pin) 
{
    sensor->dout_port = dout_port;
    sensor->dout_pin = dout_pin;
    sensor->sck_port = sck_port;
    sensor->sck_pin = sck_pin;
    sensor->offset = 0;
    sensor->scale = 1.0f;
    sensor->is_initialized = true;

    // Ensure SCK is low to prevent entering power-down mode unexpectedly
    HAL_GPIO_WritePin(sensor->sck_port, sensor->sck_pin, GPIO_PIN_RESET);
    
    return HAL_OK;
}

int32_t loadcell_read_raw(loadcell_t *sensor) 
{
    if (!sensor->is_initialized) return LC_ERROR_CODE;

    // 1. Wait for DOUT to go low (Data Ready)
    int16_t timeout = LC_TIMEOUT_US;
    while (HAL_GPIO_ReadPin(sensor->dout_port, sensor->dout_pin) == GPIO_PIN_SET) 
		{
        delay_us(1);
        if (--timeout <= 0) return LC_ERROR_CODE;
    }

    uint32_t value = 0;

    // 2. Critical Section: Disable interrupts to prevent timing distortion
    __disable_irq();

    // 3. Read 24 bits (MSB First)
    for (int i = 0; i < 24; i++) 
		{
        // Clock High
        HAL_GPIO_WritePin(sensor->sck_port, sensor->sck_pin, GPIO_PIN_SET);
        delay_us(1);

        // Shift buffer
        value = value << 1;

        // Sample DOUT
        if (HAL_GPIO_ReadPin(sensor->dout_port, sensor->dout_pin) == GPIO_PIN_SET) 
				{
            value++;
        }

        // Clock Low
        HAL_GPIO_WritePin(sensor->sck_port, sensor->sck_pin, GPIO_PIN_RESET);
        delay_us(1);
    }

    // 4. 25th Pulse for Gain 128 / Channel A
    HAL_GPIO_WritePin(sensor->sck_port, sensor->sck_pin, GPIO_PIN_SET);
    delay_us(1);
    HAL_GPIO_WritePin(sensor->sck_port, sensor->sck_pin, GPIO_PIN_RESET);
    delay_us(1);

    // 5. Re-enable interrupts
    __enable_irq();

    // 6. Sign Extension (24-bit to 32-bit signed)
    if (value & (1UL << 23)) 
		{
        value |= HX711_SIGN_MASK;
    }

    return (int32_t)value;
}

int32_t loadcell_read_average(loadcell_t *sensor, uint8_t times) 
{
    if (times < 1) times = 1;
    
    int64_t sum = 0;
    uint8_t valid_count = 0;
    
    for (uint8_t i = 0; i < times; i++) 
		{
        int32_t raw = loadcell_read_raw(sensor);
        if (raw != LC_ERROR_CODE) 
				{
            sum += raw;
            valid_count++;
        }
        // Small delay between readings isn't strictly necessary for HX711 
        // as it updates at 10Hz/80Hz, but helps stability
        HAL_Delay(10); 
    }

    if (valid_count == 0) return LC_ERROR_CODE;
    return (int32_t)(sum / valid_count);
}

void loadcell_tare(loadcell_t *sensor) {
    // Take average of 10 readings for tare
    int32_t avg = loadcell_read_average(sensor, 10);
    if (avg != LC_ERROR_CODE) {
        sensor->offset = avg;
    }
}

void loadcell_set_scale(loadcell_t *sensor, float scale_value) {
    sensor->scale = scale_value;
}

float loadcell_get_weight(loadcell_t *sensor) {
    int32_t raw = loadcell_read_raw(sensor);
    if (raw == LC_ERROR_CODE) return 0.0f;
    
    return (float)(raw - sensor->offset) / sensor->scale;
}