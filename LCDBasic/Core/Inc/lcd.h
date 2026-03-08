#ifndef LCD_H
#define LCD_H

#include <cstdint>
#include <string>
#include <string_view>

void LCD_Init(void);
void LCD_SetCursor(std::uint8_t row, std::uint8_t col);
void LCD_Print(std::string_view text);

inline void LCD_Print(const std::string &text)
{
    LCD_Print(std::string_view(text));
}

inline void LCD_Print(const char *text)
{
    LCD_Print(text ? std::string_view(text) : std::string_view());
}

#endif
