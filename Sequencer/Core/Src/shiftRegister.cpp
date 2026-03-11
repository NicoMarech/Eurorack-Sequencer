#include "shiftRegister.h"
#include "gpio.h"
#include "main.h"
#include "spi.h"

void ShiftRegister::shift595_write(uint8_t value)
{
    HAL_GPIO_WritePin(SHIFTRES_RCLK_GPIO_Port, SHIFTRES_RCLK_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi2, &value, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(SHIFTRES_RCLK_GPIO_Port, SHIFTRES_RCLK_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SHIFTRES_RCLK_GPIO_Port, SHIFTRES_RCLK_Pin, GPIO_PIN_RESET);
}