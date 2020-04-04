#pragma once
namespace PacketType
{
	constexpr uint8_t Image = 0x00;
	constexpr uint8_t Sound = 0x01;
	constexpr uint8_t Server = 0x02;
	namespace ServerType
	{
		constexpr uint8_t ClientConnected = 0x00;
		constexpr uint8_t ClientDisconnected = 0x01;
	}
	constexpr uint8_t Setup = 0x03;
	namespace SetupType
	{
		constexpr uint8_t SampleRate = 0x00;
	}
}