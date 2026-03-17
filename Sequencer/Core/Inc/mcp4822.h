#pragma once

#include <cstdint>

class Mcp4822
{
public:
	enum class Channel : std::uint8_t
	{
		A = 0U,
		B = 1U
	};

	enum class Gain : std::uint8_t
	{
		X1 = 0U,
		X2 = 1U
	};

	Mcp4822();

	void init();
	bool writeRaw(Channel channel, std::uint16_t value, Gain gain = Gain::X1, bool active = true);
	bool shutdown(Channel channel);

private:
	static constexpr std::uint16_t kMaxValue = 0x0FFFU;

	bool writeCommand(std::uint16_t command);
};
