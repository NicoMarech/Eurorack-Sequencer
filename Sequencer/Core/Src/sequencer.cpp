#include "sequencer.h"

#include "main.h"

Sequencer::Sequencer(LedMatrix &inLedMatrix,
                     Mcp4822 &inMcp4822,
                     TIM_HandleTypeDef &inEncoderTimer,
                     std::uint16_t inBpm,
                     std::uint8_t inStepsPerBeat)
    : mLedMatrix(inLedMatrix)
    , mMcp4822(inMcp4822)
    , mEncoderTimer(inEncoderTimer)
    , mSteps{}
    , mBpm((inBpm == 0U) ? 120U : inBpm)
    , mStepsPerBeat((inStepsPerBeat == 0U) ? 4U : inStepsPerBeat)
    , mLastStepTick(0U)
    , mLastEncoderCount(0)
    , mCurrentStep(0U)
    , mWasButtonPressed(false)
    , mOutputsDirty(true)
{
}

void Sequencer::init()
{
    static constexpr std::array<std::uint16_t, sStepCount> kInitialStepValues = {
        0U, 1024U, 2048U, 3072U,
        512U, 1536U, 2560U, 3584U,
        768U, 1792U, 2816U, 3840U,
        256U, 1280U, 2304U, 3328U
    };

    for (std::uint8_t stepIndex = 0U; stepIndex < sStepCount; ++stepIndex)
    {
        mSteps[stepIndex] = Step{kInitialStepValues[stepIndex], true};
    }

    mCurrentStep = 0U;
    mLastStepTick = HAL_GetTick();
    mLastEncoderCount = static_cast<std::int32_t>(__HAL_TIM_GET_COUNTER(&mEncoderTimer));
    mWasButtonPressed = (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET);
    mOutputsDirty = true;

    writeStepToOutputs();
}

void Sequencer::update()
{
    pollEncoder();
    pollButton();

    const std::uint32_t stepPeriodMs = calculateStepPeriodMs();
    const std::uint32_t now = HAL_GetTick();
    if (static_cast<std::uint32_t>(now - mLastStepTick) >= stepPeriodMs)
    {
        mLastStepTick = now;
        advanceStep();
    }

    if (mOutputsDirty)
    {
        writeStepToOutputs();
    }
}

void Sequencer::pollEncoder()
{
    
}

void Sequencer::pollButton()
{
    const bool isButtonPressed = (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET);
    if (isButtonPressed && !mWasButtonPressed)
    {
        mSteps[mCurrentStep].enabled = !mSteps[mCurrentStep].enabled;
        mOutputsDirty = true;
    }

    mWasButtonPressed = isButtonPressed;
}

void Sequencer::advanceStep()
{
    mCurrentStep = static_cast<std::uint8_t>((mCurrentStep + 1U) % sStepCount);
    mOutputsDirty = true;
}

void Sequencer::writeStepToOutputs()
{
    const Step &step = mSteps[mCurrentStep];
    const std::uint8_t nextStep = static_cast<std::uint8_t>((mCurrentStep + 1U) % sStepCount);

    mLedMatrix.setLed(nextStep);
    mMcp4822.writeRaw(Mcp4822::Channel::A, step.value, Mcp4822::Gain::X1, true);
    mMcp4822.writeRaw(Mcp4822::Channel::B,
                      step.enabled ? sMaxDacValue : 0U,
                      Mcp4822::Gain::X1,
                      true);

    mOutputsDirty = false;
}

std::uint32_t Sequencer::calculateStepPeriodMs() const
{
    static constexpr std::uint32_t kMsPerMinute = 60000U;

    const std::uint32_t stepsPerMinute = static_cast<std::uint32_t>(mBpm) * static_cast<std::uint32_t>(mStepsPerBeat);
    if (stepsPerMinute == 0U)
    {
        return 1U;
    }

    const std::uint32_t period = kMsPerMinute / stepsPerMinute;
    return (period == 0U) ? 1U : period;
}
