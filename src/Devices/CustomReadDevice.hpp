// A device which allows the caller to give functions to read/seek into a file stream
#ifndef GTL_CUSTOMREADDEVICE_HPP
#define GTL_CUSTOMREADDEVICE_HPP

#include <gtl/GameTextureLoader.hpp>
#include "DeviceBase.hpp"
#include <boost/iostreams/detail/ios.hpp>

#include <boost/function.hpp>

namespace GameTextureLoader
{
	namespace Devices
	{
		namespace GTL = GameTextureLoader;
		class CustomReadDevice : public DeviceBase
		{
		public:
			CustomReadDevice(GTL::ReadFunc_t reader, GTL::SeekFunc_t seeker);
			virtual std::streamsize read(char* s, std::streamsize n);
			virtual std::streamsize write(const char* s, std::streamsize n);
			virtual std::streampos seek(io::stream_offset off, std::ios_base::seekdir way);
		protected:
		private:
			GTL::ReadFunc_t reader_;
			GTL::SeekFunc_t seeker_;
		};
	}
}

#endif