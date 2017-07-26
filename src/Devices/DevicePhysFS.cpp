// PhysFS implimentation file

#include "DevicePhysFS.hpp"

namespace GameTextureLoader
{
	namespace Devices
	{
		PhysFSDevice::PhysFSDevice(PHYSFS_File* file) : file_(file)
		{

		}

		std::streamsize PhysFSDevice::read(char* s, std::streamsize n)
		{
			if(PHYSFS_eof(file_))
				return -1;	// we're reached the end of this file
			
			PHYSFS_sint64 len = PHYSFS_read(file_, s, 1, n);
			
			return static_cast<std::streamsize>(len);
				
		}

		std::streampos PhysFSDevice::seek(io::stream_offset off, std::ios_base::seekdir way)
		{
			PHYSFS_sint64 offset = 0;
			if(way == std::ios_base::cur)
			{
				// We are trying to seek from the current location, as such we need to find where
				// we are in the file in order to work out the correct offset from the beginning
				offset = PHYSFS_tell(file_);
				if(offset == -1)
					return -1;
			}

			int rt = PHYSFS_seek(file_, offset + off);	// This always seeks from the beginning of the file
			if(rt == 0)
				return -1;

			return static_cast<std::streamoff>(offset + off);

		}

		std::streamsize PhysFSDevice::write(const char* s, std::streamsize n)
		{
			return static_cast<std::streamsize>(PHYSFS_write(file_,s,1,n));
		}

	}
}