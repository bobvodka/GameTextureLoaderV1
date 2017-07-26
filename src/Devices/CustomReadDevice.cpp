// Implimentation of the CustomReadDevice

#include <gtl/GameTextureLoader.hpp>
#include "CustomReadDevice.hpp"

namespace GameTextureLoader
{
	namespace Devices
	{
		CustomReadDevice::CustomReadDevice(GTL::ReadFunc_t reader, GTL::SeekFunc_t seeker) : reader_(reader), seeker_(seeker)
		{
		}

		std::streamsize CustomReadDevice::read(char* s, std::streamsize n)
		{
			return reader_(s,n);
		}

		std::streampos CustomReadDevice::seek(io::stream_offset off, std::ios_base::seekdir way)
		{
			return seeker_(off, way);
		}

		std::streamsize CustomReadDevice::write(const char* s, std::streamsize n)
		{
			return -1;
		}
	}
}