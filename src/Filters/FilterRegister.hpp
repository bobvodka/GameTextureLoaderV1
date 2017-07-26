// Filter Register class
#ifndef GTL_FILTERREGISTER_HPP
#define GTL_FILTERREGISTER_HPP

#include <gtl/GameTextureLoader.hpp>
#include <gtl/GTLRunTimeTextureFilter.hpp>
#include <gtl/FilterBase.hpp>
#include "../internals/internal.hpp"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>

#include <boost/function.hpp>

namespace GameTextureLoader 
{ 
	namespace Filters
	{
		namespace mi = boost::multi_index;

		struct FilterCreatorDetails
		{
			FileTypes id_;
			std::string key_;
			FilterCreator_t creator_;
			
			FilterCreatorDetails(FileTypes id, std::string const &key, FilterCreator_t creator) : id_(id), key_(key), creator_(creator)
			{
			}
			FilterCreatorDetails(FilterCreatorDetails const &lhs) : id_(lhs.id_), key_(lhs.key_), creator_(lhs.creator_)
			{
			}
			bool operator<(FilterCreatorDetails const &lhs) const 
			{ 
				return id_ < lhs.id_;
			};
		};

		struct key{};
		struct id{};

		// mi::ordered_unique< mi::tag<id> , mi::identity<FilterCreatorDetails> >,	// orders by operator <
		// old id line

		class FilterRegister
		{
		public:
			//		typedef std::map<std::string, FilterCreator_t > texmap_t;
			typedef mi::multi_index_container<
				FilterCreatorDetails,
				mi::indexed_by<
				//mi::ordered_unique< mi::tag<id> , mi::identity<FilterCreatorDetails> >,	// orders by operator <
				mi::ordered_unique< mi::tag<id> , mi::member<FilterCreatorDetails,FileTypes, &FilterCreatorDetails::id_> >,
				//mi::ordered_unique< mi::tag<id>, BOOST_MULTI_INDEX_MEMBER(FilterCreatorDetails, FileTypes, id_) >,
				//mi::ordered_unique< mi::tag<id>, ::boost::multi_index::member<FilterCreatorDetails, FileTypes, &FilterCreatorDetails::id_> >,
				mi::ordered_non_unique< mi::tag<key> , mi::member<FilterCreatorDetails,std::string, &FilterCreatorDetails::key_> >
				> 
			> texmap_t;

			FilterRegister(FileTypes id, std::string const &key, FilterCreator_t creator);
			FilterRegister();
			~FilterRegister();
			FilterCreator_t getCreator(std::string const &val);
			FilterCreator_t getCreator(FileTypes val);
			int RegisterRunTimeFilter(std::string const &key, FilterCreator_t creator);
			void UnRegisterRunTimeFilter(int filterid);
		protected:
		private:

			static texmap_t* texturefilters;
			static int count;
			static int nextFilterID;
		};

#define DECLARE_TEXTURE_LOADER( idnum, loaderName, callbackFunction ) \
	static FilterRegister FilterRegister_##callbackFunction ( idnum, loaderName, callbackFunction );
			
	}
}


#endif
