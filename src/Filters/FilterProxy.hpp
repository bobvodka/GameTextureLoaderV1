// Proxy for filters so we can pass by reference from a factory which gives us
// a pointer
#ifndef GTL_FILTERPROXY_HPP
#define GTL_FILTERPROXY_HPP
#include <gtl/FilterBase.hpp>
#include <boost/iostreams/categories.hpp> // input_filter_tag
#include <boost/iostreams/detail/ios.hpp> 

#include <gtl/DeviceProxy.hpp>

#include <iostream>
#include <typeinfo>

namespace GameTextureLoader
{
	namespace Filters
	{
		namespace io = boost::iostreams;

		class FilterProxy
		{

		public:
			typedef char                       char_type;
			typedef io::multichar_input_filter_tag  category;

			FilterProxy(filterptr filter) : filter_(filter)
			{
			}
			FilterProxy(FilterProxy const &lhs) : filter_(lhs.filter_)
			{
			}
			template<typename Source>
				std::streamsize read(Source& src, char* s, std::streamsize n)
			{
				streambuf_t *lsrc = static_cast<streambuf_t*>(&src);
				return filter_->read(lsrc,s,n);
			}
		private:
			filterptr filter_;
		};
	}
}

#endif
