#include "mcp4822.h"

#include "main.h"
#include "spi.h"

union Mcp4822Command
{
	std::uint16_t raw;
	struct
	{
		std::uint16_t data : 12;
		std::uint16_t shutdown : 1;
		std::uint16_t gain : 1;
		std::uint16_t buffer : 1;
		std::uint16_t channel : 1;
	} bits;
};

Mcp4822::Mcp4822()
{
}

void Mcp4822::init()
{
	HAL_GPIO_WritePin(DAC_CS_GPIO_Port, DAC_CS_Pin, GPIO_PIN_SET);
}

bool Mcp4822::writeRaw(Channel channel, std::uint16_t value, Gain gain, bool active)
{
	Mcp4822Command command{};
	command.bits.data = static_cast<std::uint16_t>(value & kMaxValue);
	command.bits.shutdown = active ? 1U : 0U;
	command.bits.gain = (gain == Gain::X1) ? 1U : 0U;
	command.bits.buffer = 1U;
	command.bits.channel = (channel == Channel::B) ? 1U : 0U;

	return writeCommand(command.raw);
}

bool Mcp4822::shutdown(Channel channel)
{
	return writeRaw(channel, 0U, Gain::X1, false);
}

bool Mcp4822::writeCommand(std::uint16_t command)
{
	std::uint8_t txBuffer[2] = {0U, 0U};
	txBuffer[0] = static_cast<std::uint8_t>(command & 0xFFU);
	txBuffer[1] = static_cast<std::uint8_t>((command >> 8U) & 0xFFU);

	HAL_GPIO_WritePin(DAC_CS_GPIO_Port, DAC_CS_Pin, GPIO_PIN_RESET);

	const HAL_StatusTypeDef status = HAL_SPI_Transmit(
		&hspi3,
		txBuffer,
		1U,
		HAL_MAX_DELAY);

	HAL_GPIO_WritePin(DAC_CS_GPIO_Port, DAC_CS_Pin, GPIO_PIN_SET);

	return status == HAL_OK;
}
