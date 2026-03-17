#pragma once

#include "mcp4822.h"
#include "sequencer_LedMatrix.h"
#include "tim.h"

#include <array>
#include <cstdint>

class Sequencer
{
public:
    Sequencer(LedMatrix &inLedMatrix,
              Mcp4822 &inMcp4822,
              TIM_HandleTypeDef &inEncoderTimer,
              std::uint16_t inBpm = 120U,
              std::uint8_t inStepsPerBeat = 4U);

    void init();
    void update();

private:
    struct Step
    {
        std::uint16_t value;
        bool enabled;
    };

    static constexpr std::uint8_t sStepCount = 16U;
    static constexpr std::uint16_t sMaxDacValue = 4095U;
    static constexpr std::uint16_t sEncoderStepSize = 16U;

    void pollEncoder();
    void pollButton();
    void advanceStep();
    void writeStepToOutputs();
    std::uint32_t calculateStepPeriodMs() const;

private:
    LedMatrix &mLedMatrix;
    Mcp4822 &mMcp4822;
    TIM_HandleTypeDef &mEncoderTimer;
    std::array<Step, sStepCount> mSteps;

    std::uint16_t mBpm;
    std::uint8_t mStepsPerBeat;
    std::uint32_t mLastStepTick;
    std::int32_t mLastEncoderCount;
    std::uint8_t mCurrentStep;
    bool mWasButtonPressed;
    bool mOutputsDirty;
};