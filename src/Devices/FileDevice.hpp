// Basic FileDevice wrapper class
#ifndef GTL_FILEDEVICE_HPP
#define GTL_FILEDEVICE_HPP

#include "DeviceBase.hpp"
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/detail/ios.hpp>  // openmode, seekdir, int types.

namespace GameTextureLoader
{
	namespace Devices
	{
		namespace io = boost::iostreams;

		class FileDevice : public DeviceBase
		{
		public:
			FileDevice(std::string const &name);
			virtual std::streamsize read(char* s, std::streamsize n);
			virtual std::streamsize write(const char* s, std::streamsize n);
			virtual std::streampos seek(io::stream_offset off, std::ios_base::seekdir way);
		protected:
		private:
			io::file_descriptor filedev_;
		};
	}
}

#endif