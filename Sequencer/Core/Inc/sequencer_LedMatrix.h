#pragma once

#include "shiftRegister.h"
#include <memory>

class LedMatrix
{
public:
    LedMatrix();
    virtual ~LedMatrix() = default;

    void setLed(std::uint8_t inLedIndex);
    void clear();

private:    
    static constexpr std::uint8_t sMatrixSize = 4U;
    static constexpr std::uint8_t sLedCount = sMatrixSize * sMatrixSize;
    static constexpr std::uint8_t sCathodeBaseBit = 4U;
    static constexpr std::uint8_t sAnodeMask = 0x0FU;
    static constexpr std::uint8_t sCathodeMask = 0xF0U;

private:
    std::unique_ptr<ShiftRegister> mShiftRegister;
};
