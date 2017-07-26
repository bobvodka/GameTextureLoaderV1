// Simple File Device

#include "FileDevice.hpp"

namespace GameTextureLoader
{
	namespace Devices
	{
		FileDevice::FileDevice(std::string const &name) : filedev_(name)
		{
		}

		std::streamsize FileDevice::read(char* s, std::streamsize n)
		{
			return filedev_.read(s,n);
		}
		std::streamsize FileDevice::write(const char* s, std::streamsize n)
		{
			return filedev_.write(s,n);
		}
		std::streampos FileDevice::seek(io::stream_offset off, std::ios_base::seekdir way)
		{
			return static_cast<std::streamoff>(filedev_.seek(off,way));
		}
	}
}