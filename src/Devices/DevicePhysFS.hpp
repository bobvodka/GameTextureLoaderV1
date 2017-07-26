// Device for working with PhysFS based files
#ifndef GTL_DEVICEPHYSFS_HPP
#define GTL_DEVICEPHYSFS_HPP

#include <physfs.h>
#include "DeviceBase.hpp"
#include <boost/iostreams/detail/ios.hpp>

namespace GameTextureLoader 
{
	namespace Devices
	{

		namespace io = boost::iostreams;

		class PhysFSDevice : public DeviceBase
		{
		public:
			PhysFSDevice(PHYSFS_File* file);
			virtual std::streamsize read(char* s, std::streamsize n);
			virtual std::streamsize write(const char* s, std::streamsize n);
			virtual std::streampos seek(io::stream_offset off, std::ios_base::seekdir way);
		protected:
		private:
			PHYSFS_File* file_;
		};

	}
}

#endif