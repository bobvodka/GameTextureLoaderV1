// Base Filter for all Filters

#ifndef GTL_FILTERBASE_HPP
#define GTL_FILTERBASE_HPP

#include <boost/iostreams/detail/ios.hpp> 
#include <boost/shared_ptr.hpp>

#include <boost/iostreams/device/file_descriptor.hpp>	// for the typedef
#include <boost/iostreams/filtering_streambuf.hpp>		// for the typedef

#include <gtl/DeviceProxy.hpp>

namespace GameTextureLoader 
{
	namespace Filters
	{
		class FilterBase
		{
		public:
			virtual ~FilterBase()
			{
			}
			virtual std::streamsize read(streambuf_t *src,char * s, std::streamsize n) = 0;
		};
		typedef boost::shared_ptr<FilterBase> filterptr;
	};

};

#endif
