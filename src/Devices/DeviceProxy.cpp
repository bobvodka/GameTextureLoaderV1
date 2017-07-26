// Code for the device proxy class
#include "DeviceBase.hpp"
#include <gtl/DeviceProxy.hpp>
namespace GameTextureLoader
{
	namespace Devices
	{
		std::streamsize DeviceProxy::read(char* s, std::streamsize n) 
		{
			return dev_->read(s,n);
		}
		std::streamsize DeviceProxy::write(const char* s, std::streamsize n) 
		{
			return dev_->write(s,n);
		}
		std::streampos DeviceProxy::seek(io::stream_offset off, std::ios_base::seekdir way) 
		{
			return dev_->seek(off,way);
		}
	}
}