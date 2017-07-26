// Proxy Device used to redirecting read/writing/seeking to real device pointer
#ifndef GTL_DEVICEPROXY_HPP
#define GTL_DEVICEPROXY_HPP

#include <boost/iostreams/detail/ios.hpp>  // openmode, seekdir, int types.
#include <boost/iostreams/positioning.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/shared_ptr.hpp>

namespace GameTextureLoader
{
	namespace Devices
	{
		namespace ios = boost::iostreams;

		struct DeviceBase;

		class DeviceProxy
		{
		public:
			typedef char char_type;
			typedef ios::seekable_device_tag category;

			DeviceProxy(DeviceBase *dev) : dev_(dev)
			{
			}
			~DeviceProxy()
			{
			}		
			std::streamsize read(char* s, std::streamsize n);
			std::streamsize write(const char* s, std::streamsize n);
			std::streampos seek(ios::stream_offset off, std::ios_base::seekdir way);

		protected:
		private:
			DeviceBase *dev_;
		};
	}

	typedef boost::iostreams::stream_buffer<GameTextureLoader::Devices::DeviceProxy> streambuf_t;
}

#endif
