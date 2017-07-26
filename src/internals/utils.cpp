// Util functions

#include <boost/cstdint.hpp>
#include "internal.hpp"

namespace GameTextureLoader
{
	typedef unsigned char byte;

	namespace Utils
	{

		inline boost::uint16_t read16_le(const byte* b) 
		{
			return b[0] + (b[1] << 8);
		}

		inline void write16_le(byte* b, boost::uint16_t value) 
		{
			b[0] = value & 0xFF;
			b[1] = value >> 8;
		}

		inline boost::uint16_t read16_be(const byte* b) 
		{
			return (b[0] << 8) + b[1];
		}

		inline void write16_be(byte* b, boost::uint16_t value) 
		{
			b[0] = value >> 8;
			b[1] = value & 0xFF;
		}

		inline boost::uint32_t read32_le(const byte* b) 
		{
			return read16_le(b) + (read16_le(b + 2) << 16);
		}

		inline boost::uint32_t read32_be(const byte* b) 
		{
			return (read16_be(b) << 16) + read16_be(b + 2);
		}

		// count the number of consecutive zeroes on the right side of a
		// binary number
		// 0x00F0 will return 4
		int count_right_zeroes(boost::uint32_t n) 
		{
			int total = 0;
			boost::uint32_t c = 1;
			while ((total < 32) && ((n & c) == 0)) 
			{
				c <<= 1;
				++total;
			}
			return total;
		}

		// count the number of ones in a binary number
		// 0x00F1 will return 5
		int count_ones(boost::uint32_t n) 
		{
			int total = 0;
			boost::uint32_t c = 1;
			for (int i = 0; i < 32; ++i) 
			{
				if (n & c) 
				{
					++total;
				}
				c <<= 1;
			}
			return total;
		}

		int getMinSize(ImgFormat flag)
		{
			int minsize = 1;

			switch(flag) 
			{
			case FORMAT_DXT1:
				minsize = 8;
				break;
			case FORMAT_DXT2:
			case FORMAT_DXT3:
			case FORMAT_DXT4:
			case FORMAT_DXT5:
			case FORMAT_3DC:
				minsize = 16;
				break;
			case FORMAT_NONE:
				minsize = 0;
			default:
				break;
			}
			return minsize;

		}
	}
}