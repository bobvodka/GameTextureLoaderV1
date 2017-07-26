// Base for all devices used
#ifndef GTL_DEVICEBASE_HPP
#define GTL_DEVICEBASE_HPP

#include <boost/iostreams/detail/ios.hpp>  // openmode, seekdir, int types.
#include <boost/iostreams/positioning.hpp>
#include <boost/shared_ptr.hpp>

namespace GameTextureLoader { 
	namespace Devices
	{
		namespace io = boost::iostreams;

		struct DeviceBase
		{
			virtual ~DeviceBase()
			{
			}
			virtual std::streamsize read(char* s, std::streamsize n) = 0;
			virtual std::streamsize write(const char* s, std::streamsize n) = 0;
			virtual std::streampos seek(io::stream_offset off, std::ios_base::seekdir way) = 0;
		};
	}
}


#endif