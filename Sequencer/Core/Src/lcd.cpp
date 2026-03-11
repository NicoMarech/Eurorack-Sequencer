#include "lcd.h"
#include "main.h"

static void LCD_Write4Bits(std::uint8_t nibble)
{
    HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin, (nibble & 0x01U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin, (nibble & 0x02U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, (nibble & 0x04U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin, (nibble & 0x08U) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(LCD_Enable_GPIO_Port, LCD_Enable_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(LCD_Enable_GPIO_Port, LCD_Enable_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);
}

static void LCD_SendCommand(std::uint8_t command)
{
    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);
    LCD_Write4Bits((command >> 4) & 0x0FU);
    LCD_Write4Bits(command & 0x0FU);
    HAL_Delay(2);
}

static void LCD_SendData(std::uint8_t data)
{
    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_SET);
    LCD_Write4Bits((data >> 4) & 0x0FU);
    LCD_Write4Bits(data & 0x0FU);
    HAL_Delay(1);
}

void LCD_Init(void)
{
    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_Enable_GPIO_Port, LCD_Enable_Pin, GPIO_PIN_RESET);

    HAL_Delay(50);

    LCD_Write4Bits(0x03);
    HAL_Delay(5);
    LCD_Write4Bits(0x03);
    HAL_Delay(1);
    LCD_Write4Bits(0x03);
    HAL_Delay(1);
    LCD_Write4Bits(0x02);

    LCD_SendCommand(0x28);
    LCD_SendCommand(0x08);
    LCD_SendCommand(0x01);
    HAL_Delay(2);
    LCD_SendCommand(0x06);
    LCD_SendCommand(0x0C);
}

void LCD_SetCursor(std::uint8_t row, std::uint8_t col)
{
    static constexpr std::uint8_t row_offsets[] = {0x00U, 0x40U};
    std::uint8_t safe_row = (row > 1U) ? 1U : row;
    LCD_SendCommand(static_cast<std::uint8_t>(0x80U | (row_offsets[safe_row] + col)));
}

void LCD_Print(std::string_view text)
{
    for (char ch : text)
    {
        LCD_SendData(static_cast<std::uint8_t>(ch));
    }
}
