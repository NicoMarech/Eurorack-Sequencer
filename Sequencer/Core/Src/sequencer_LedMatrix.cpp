#include "sequencer_LedMatrix.h"

LedMatrix::LedMatrix()
    : mShiftRegister(std::make_unique<ShiftRegister>())
{
}

void LedMatrix::setLed(std::uint8_t inLedIndex)
{
    if (inLedIndex >= LedMatrix::sLedCount)
    {
        clear();
        return;
    }

    const std::uint8_t anodeBit = static_cast<std::uint8_t>(inLedIndex % LedMatrix::sMatrixSize);
    const std::uint8_t cathodeBit = static_cast<std::uint8_t>(LedMatrix::sCathodeBaseBit + (inLedIndex / LedMatrix::sMatrixSize));
    std::uint8_t pattern = LedMatrix::sCathodeMask;

    pattern = static_cast<std::uint8_t>((pattern & static_cast<std::uint8_t>(~LedMatrix::sAnodeMask)) | static_cast<std::uint8_t>(1U << anodeBit));
    pattern = static_cast<std::uint8_t>(pattern & static_cast<std::uint8_t>(~(1U << cathodeBit)));

    mShiftRegister->shift595_write(pattern);
}

void LedMatrix::clear()
{
    mShiftRegister->shift595_write(LedMatrix::sCathodeMask);
}
