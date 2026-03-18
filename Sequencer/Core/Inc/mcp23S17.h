#pragma once

#include "spi.h"

#include <array>
#include <cstdint>

class Mcp23s17
{
public:

	Mcp23s17();

	void init();
	void onGpioInterrupt(std::uint16_t inGpioPin);
	void processInterrupts();

	std::uint8_t getPortAValue() const;
	std::uint8_t getPortBValue() const;
	bool consumePortAChanged();
	bool consumePortBChanged();

private:
	enum class Register : std::uint8_t
	{
		IodirA = 0x00U,
		IodirB = 0x01U,
		IpolA = 0x02U,
		IpolB = 0x03U,
		GpintenA = 0x04U,
		GpintenB = 0x05U,
		DefvalA = 0x06U,
		DefvalB = 0x07U,
		IntconA = 0x08U,
		IntconB = 0x09U,
		Iocon = 0x0AU,
		GppuA = 0x0CU,
		GppuB = 0x0DU,
		IntfA = 0x0EU,
		IntfB = 0x0FU,
		IntcapA = 0x10U,
		IntcapB = 0x11U,
		GpioA = 0x12U,
		GpioB = 0x13U,
		OlatA = 0x14U,
		OlatB = 0x15U,
	};

	union PinRegister
	{
		std::uint8_t raw;
		struct
		{
			std::uint8_t gp0 : 1;
			std::uint8_t gp1 : 1;
			std::uint8_t gp2 : 1;
			std::uint8_t gp3 : 1;
			std::uint8_t gp4 : 1;
			std::uint8_t gp5 : 1;
			std::uint8_t gp6 : 1;
			std::uint8_t gp7 : 1;
		} bits;
	};

	union IoconRegister
	{
		std::uint8_t raw;
		struct
		{
			std::uint8_t reserved0 : 1;
			std::uint8_t intcc : 1;
			std::uint8_t intpol : 1;
			std::uint8_t odr : 1;
			std::uint8_t haen : 1;
			std::uint8_t disslw : 1;
			std::uint8_t seqop : 1;
			std::uint8_t bank : 1;
		} bits;
	};

	union ControlByte
	{
		std::uint8_t raw;
		struct
		{
			std::uint8_t readNotWrite : 1;
			std::uint8_t hardwareAddress : 3;
			std::uint8_t opcode : 4;
		} bits;
	};

	union WriteFrame
	{
		std::uint8_t raw[3];
		struct
		{
			ControlByte control;
			std::uint8_t reg;
			std::uint8_t data;
		} fields;
	};

	union ReadFrame
	{
		std::uint8_t tx[3];
		std::uint8_t rx[3];
	};

	struct PendingWrite
	{
		Register reg;
		std::uint8_t value;
	};

	static void onSpiTxCompleteStatic(SPI_HandleTypeDef *inHspi, void *inContext);
	static void onSpiTxRxCompleteStatic(SPI_HandleTypeDef *inHspi, void *inContext);
	static void onSpiErrorStatic(SPI_HandleTypeDef *inHspi, void *inContext);

	void onSpiTxComplete(SPI_HandleTypeDef *inHspi);
	void onSpiTxRxComplete(SPI_HandleTypeDef *inHspi);
	void onSpiError(SPI_HandleTypeDef *inHspi);
	bool writeRegister(Register inRegister, std::uint8_t inData);
	bool startWriteTransfer(Register inRegister, std::uint8_t inData);
	void processWriteQueue();
	bool startReadRegister(Register inRegister);
	void consumeReadResult();
	bool ensureCallbacksRegistered();

	static constexpr std::uint8_t sControlOpcode = 0x4U;
	static constexpr std::uint8_t sControlReadBit = 0x1U;
	static constexpr std::uint8_t sDefaultHardwareAddress = 0x0U;
	static constexpr std::uint32_t sTransferTimeoutMs = 10U;
	static constexpr std::uint8_t sWriteQueueSize = 16U;

	WriteFrame mTxFrame;
	ReadFrame mReadFrame;
	std::array<PendingWrite, sWriteQueueSize> mWriteQueue;
	std::uint8_t mWriteHead;
	std::uint8_t mWriteTail;
	std::uint8_t mWriteCount;
	bool mCallbacksRegistered;
	volatile bool mTransferDone;
	volatile bool mTransferError;
	volatile bool mWriteInProgress;
	std::uint32_t mWriteStartTick;
	volatile bool mReadInProgress;
	volatile bool mReadCompleted;
	volatile bool mReadError;
	std::uint32_t mReadStartTick;
	Register mActiveReadRegister;
	volatile bool mIntaPending;
	volatile bool mIntbPending;
	std::uint8_t mPortAValue;
	std::uint8_t mPortBValue;
	volatile bool mPortAChanged;
	volatile bool mPortBChanged;
};
