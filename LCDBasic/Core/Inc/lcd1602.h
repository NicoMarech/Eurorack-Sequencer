#ifndef LCD1602_H
#define LCD1602_H

#include "lcd.h"

class Lcd1602
{
public:
    void init() const { LCD_Init(); }
    void setCursor(std::uint8_t row, std::uint8_t col) const { LCD_SetCursor(row, col); }
    void print(std::string_view text) const { LCD_Print(text); }
    void clear() const
    {
        LCD_SetCursor(0, 0);
        LCD_Print("                ");
        LCD_SetCursor(1, 0);
        LCD_Print("                ");
        LCD_SetCursor(0, 0);
    }
};

#endif
