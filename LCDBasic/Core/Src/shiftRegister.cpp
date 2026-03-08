#include "shiftRegister.h"

void shift595_write(uint8_t value)
{
    HAL_GPIO_WritePin(SHIFTRES_RCLK_GPIO_Port, SHIFTRES_RCLK_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi2, &value, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(SHIFTRES_RCLK_GPIO_Port, SHIFTRES_RCLK_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SHIFTRES_RCLK_GPIO_Port, SHIFTRES_RCLK_Pin, GPIO_PIN_RESET);
}

void shift595_drive_led(uint8_t anode_bit, uint8_t cathode_bit, bool on)
{
    constexpr uint8_t ANODE_MASK = 0x0FU;   // QA..QD
    constexpr uint8_t CATHODE_MASK = 0xF0U; // QE..QH

    if (!on)
    {
        // All anodes low, all cathodes high -> all LEDs off
        shift595_write(CATHODE_MASK);
        return;
    }

    if (anode_bit > 3U || cathode_bit < 4U || cathode_bit > 7U)
    {
        shift595_write(CATHODE_MASK);
        return;
    }

    // Start from OFF state: cathodes high, anodes low
    uint8_t pattern = CATHODE_MASK;

    // Set selected anode high
    pattern = static_cast<uint8_t>((pattern & static_cast<uint8_t>(~ANODE_MASK)) | static_cast<uint8_t>(1U << anode_bit));

    // Set selected cathode low
    pattern = static_cast<uint8_t>(pattern & static_cast<uint8_t>(~(1U << cathode_bit)));

    shift595_write(pattern);
}