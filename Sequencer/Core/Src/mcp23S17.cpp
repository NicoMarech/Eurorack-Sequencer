#include "mcp23S17.h"

#include "main.h"
#include "spi_abstraction.h"
#include "spi.h"

Mcp23s17::Mcp23s17()
	: mTxFrame{}
	, mReadFrame{}
	, mWriteQueue{}
	, mWriteHead(0U)
	, mWriteTail(0U)
	, mWriteCount(0U)
	, mCallbacksRegistered(false)
	, mTransferDone(false)
	, mTransferError(false)
	, mWriteInProgress(false)
	, mWriteStartTick(0U)
	, mReadInProgress(false)
	, mReadCompleted(false)
	, mReadError(false)
	, mReadStartTick(0U)
	, mActiveReadRegister(Register::IntcapA)
	, mIntaPending(false)
	, mIntbPending(false)
	, mPortAValue(0U)
	, mPortBValue(0U)
	, mPortAChanged(false)
	, mPortBChanged(false)
{
}

void Mcp23s17::init()
{
	HAL_GPIO_WritePin(IOEXPANDER_CS_GPIO_Port, IOEXPANDER_CS_Pin, GPIO_PIN_SET);
	(void)ensureCallbacksRegistered();

	(void)writeRegister(Register::IodirA, 0xFFU);
	(void)writeRegister(Register::IodirB, 0xFFU);
	(void)writeRegister(Register::GppuA, 0xFFU);
	(void)writeRegister(Register::GppuB, 0xFFU);
	(void)writeRegister(Register::IntconA, 0x00U);
	(void)writeRegister(Register::IntconB, 0x00U);
	(void)writeRegister(Register::GpintenA, 0xFFU);
	(void)writeRegister(Register::GpintenB, 0xFFU);
}

void Mcp23s17::onGpioInterrupt(std::uint16_t inGpioPin)
{
	if (inGpioPin == IOEXPANDER_INTA_Pin)
	{
		mIntaPending = true;
	}

	if (inGpioPin == IOEXPANDER_INTB_Pin)
	{
		mIntbPending = true;
	}
}

void Mcp23s17::processInterrupts()
{
	processWriteQueue();

	if (mReadInProgress)
	{
		if (mReadCompleted)
		{
			consumeReadResult();
		}
		else if (mReadError || ((HAL_GetTick() - mReadStartTick) >= sTransferTimeoutMs))
		{
			(void)HAL_SPI_Abort(&hspi1);
			HAL_GPIO_WritePin(IOEXPANDER_CS_GPIO_Port, IOEXPANDER_CS_Pin, GPIO_PIN_SET);
			mReadInProgress = false;
			mReadCompleted = false;
			mReadError = false;
		}
	}

	if (!mWriteInProgress && !mReadInProgress && mIntaPending)
	{
		if (startReadRegister(Register::IntcapA))
		{
			mIntaPending = false;
		}
	}

	if (!mWriteInProgress && !mReadInProgress && mIntbPending)
	{
		if (startReadRegister(Register::IntcapB))
		{
			mIntbPending = false;
		}
	}
}

std::uint8_t Mcp23s17::getPortAValue() const
{
	return mPortAValue;
}

std::uint8_t Mcp23s17::getPortBValue() const
{
	return mPortBValue;
}

bool Mcp23s17::consumePortAChanged()
{
	const bool changed = mPortAChanged;
	mPortAChanged = false;
	return changed;
}

bool Mcp23s17::consumePortBChanged()
{
	const bool changed = mPortBChanged;
	mPortBChanged = false;
	return changed;
}

bool Mcp23s17::writeRegister(Register inRegister, std::uint8_t inData)
{
	if (mWriteCount >= sWriteQueueSize)
	{
		return false;
	}

	mWriteQueue[mWriteTail] = PendingWrite{inRegister, inData};
	mWriteTail = static_cast<std::uint8_t>((mWriteTail + 1U) % sWriteQueueSize);
	++mWriteCount;

	return true;
}

bool Mcp23s17::startWriteTransfer(Register inRegister, std::uint8_t inData)
{
	if (!ensureCallbacksRegistered())
	{
		return false;
	}

	if (mWriteInProgress || mReadInProgress)
	{
		return false;
	}

	mTxFrame.raw[0] = static_cast<std::uint8_t>((sControlOpcode << 4U) |
		((sDefaultHardwareAddress & 0x07U) << 1U));
	mTxFrame.raw[1] = static_cast<std::uint8_t>(inRegister);
	mTxFrame.raw[2] = inData;

	mTransferDone = false;
	mTransferError = false;
	mWriteInProgress = true;
	mWriteStartTick = HAL_GetTick();

	HAL_GPIO_WritePin(IOEXPANDER_CS_GPIO_Port, IOEXPANDER_CS_Pin, GPIO_PIN_RESET);
	const HAL_StatusTypeDef status = HAL_SPI_Transmit_DMA(&hspi1, mTxFrame.raw, 3U);
	if (status != HAL_OK)
	{
		HAL_GPIO_WritePin(IOEXPANDER_CS_GPIO_Port, IOEXPANDER_CS_Pin, GPIO_PIN_SET);
		mWriteInProgress = false;
		mTransferError = true;
		return false;
	}

	return true;
}

void Mcp23s17::processWriteQueue()
{
	if (mWriteInProgress)
	{
		if (mTransferDone)
		{
			mTransferDone = false;
			mTransferError = false;
			mWriteInProgress = false;
		}
		else if (mTransferError || ((HAL_GetTick() - mWriteStartTick) >= sTransferTimeoutMs))
		{
			(void)HAL_SPI_Abort(&hspi1);
			HAL_GPIO_WritePin(IOEXPANDER_CS_GPIO_Port, IOEXPANDER_CS_Pin, GPIO_PIN_SET);
			mTransferDone = false;
			mTransferError = true;
			mWriteInProgress = false;
		}

		return;
	}

	if (mWriteCount == 0U)
	{
		return;
	}

	const PendingWrite &pendingWrite = mWriteQueue[mWriteHead];
	if (startWriteTransfer(pendingWrite.reg, pendingWrite.value))
	{
		mWriteHead = static_cast<std::uint8_t>((mWriteHead + 1U) % sWriteQueueSize);
		--mWriteCount;
	}
}

bool Mcp23s17::startReadRegister(Register inRegister)
{
	if (!ensureCallbacksRegistered())
	{
		return false;
	}

	if (mReadInProgress)
	{
		return false;
	}

	mReadFrame.tx[0] = static_cast<std::uint8_t>((sControlOpcode << 4U) |
		((sDefaultHardwareAddress & 0x07U) << 1U) |
		sControlReadBit);
	mReadFrame.tx[1] = static_cast<std::uint8_t>(inRegister);
	mReadFrame.tx[2] = 0x00U;
	mActiveReadRegister = inRegister;

	mReadCompleted = false;
	mReadError = false;
	mReadInProgress = true;
	mReadStartTick = HAL_GetTick();

	HAL_GPIO_WritePin(IOEXPANDER_CS_GPIO_Port, IOEXPANDER_CS_Pin, GPIO_PIN_RESET);
	const HAL_StatusTypeDef status = HAL_SPI_TransmitReceive_DMA(&hspi1,
		mReadFrame.tx,
		mReadFrame.rx,
		3U);

	if (status != HAL_OK)
	{
		HAL_GPIO_WritePin(IOEXPANDER_CS_GPIO_Port, IOEXPANDER_CS_Pin, GPIO_PIN_SET);
		mReadInProgress = false;
		mReadError = true;
		return false;
	}

	return true;
}


void Mcp23s17::consumeReadResult()


{
	if (!mReadCompleted)
	{
		return;
	}

	const std::uint8_t value = mReadFrame.rx[2];
	if (mActiveReadRegister == Register::IntcapA)
	{
		mPortAValue = value;
		mPortAChanged = true;
	}
	else if (mActiveReadRegister == Register::IntcapB)
	{
		mPortBValue = value;
		mPortBChanged = true;
	}

	mReadInProgress = false;
	mReadCompleted = false;
	mReadError = false;
}

bool Mcp23s17::ensureCallbacksRegistered()
{
	if (mCallbacksRegistered)
	{
		return true;
	}

	const bool txRegistered = SpiAbstraction::registerTxCompleteCallback(&hspi1, &Mcp23s17::onSpiTxCompleteStatic, this);
	const bool txRxRegistered = SpiAbstraction::registerTxRxCompleteCallback(&hspi1, &Mcp23s17::onSpiTxRxCompleteStatic, this);
	const bool errorRegistered = SpiAbstraction::registerErrorCallback(&hspi1, &Mcp23s17::onSpiErrorStatic, this);

	mCallbacksRegistered = txRegistered && txRxRegistered && errorRegistered;
	return mCallbacksRegistered;
}

void Mcp23s17::onSpiTxCompleteStatic(SPI_HandleTypeDef *inHspi, void *inContext)
{
	if (inContext == nullptr)
	{
		return;
	}

	static_cast<Mcp23s17 *>(inContext)->onSpiTxComplete(inHspi);
}

void Mcp23s17::onSpiErrorStatic(SPI_HandleTypeDef *inHspi, void *inContext)
{
	if (inContext == nullptr)
	{
		return;
	}

	static_cast<Mcp23s17 *>(inContext)->onSpiError(inHspi);
}

void Mcp23s17::onSpiTxRxCompleteStatic(SPI_HandleTypeDef *inHspi, void *inContext)
{
	if (inContext == nullptr)
	{
		return;
	}

	static_cast<Mcp23s17 *>(inContext)->onSpiTxRxComplete(inHspi);
}

void Mcp23s17::onSpiTxComplete(SPI_HandleTypeDef *inHspi)
{
	if (inHspi != &hspi1)
	{
		return;
	}

	HAL_GPIO_WritePin(IOEXPANDER_CS_GPIO_Port, IOEXPANDER_CS_Pin, GPIO_PIN_SET);
	mTransferDone = true;
}

void Mcp23s17::onSpiError(SPI_HandleTypeDef *inHspi)
{
	if (inHspi != &hspi1)
	{
		return;
	}

	HAL_GPIO_WritePin(IOEXPANDER_CS_GPIO_Port, IOEXPANDER_CS_Pin, GPIO_PIN_SET);
	mTransferError = true;
	mReadError = true;

}

void Mcp23s17::onSpiTxRxComplete(SPI_HandleTypeDef *inHspi)
{
	if (inHspi != &hspi1)
	{
		return;
	}

	HAL_GPIO_WritePin(IOEXPANDER_CS_GPIO_Port, IOEXPANDER_CS_Pin, GPIO_PIN_SET);
	mReadCompleted = true;
}
