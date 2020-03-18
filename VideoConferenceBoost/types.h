#pragma once
typedef unsigned char uchar;
namespace PacketType //Maybe make a class
{
	constexpr uchar Image = 0x00;
	constexpr uchar Sound = 0x01;
}
//enum PacketType : uchar {Image, Sound};