// Implementation of the FilterRegister class
#include "FilterRegister.hpp"
#include <boost/lexical_cast.hpp>
namespace GameTextureLoader 
{
	namespace Filters
	{
		FilterRegister::texmap_t * FilterRegister::texturefilters = NULL;
		int FilterRegister::count = 0;
		int FilterRegister::nextFilterID = 1;

		FilterRegister::FilterRegister(FileTypes id, std::string const &key, FilterCreator_t creator)
		{
			if(texturefilters == NULL)
				texturefilters = new texmap_t();

			//		texturefilters->insert(std::make_pair(key,creator));	// this blows up, probably a static initalisation problem... 
			texturefilters->insert(FilterCreatorDetails(id, key, creator));
			++count;
			++nextFilterID;
		}

		FilterRegister::FilterRegister()
		{
			++count;
		}

		FilterRegister::~FilterRegister()
		{
			--count;
			if(count == 0)
			{
				delete texturefilters;
				texturefilters = NULL;
			}
		}
		FilterCreator_t FilterRegister::getCreator(std::string const &val)
		{
			typedef FilterRegister::texmap_t::index<key>::type tex_by_name;	
			tex_by_name::iterator it = texturefilters->get<key>().find(val);
			if(it == texturefilters->get<key>().end())
				throw LoaderNotFoundException("Texture Loader not found for texture type : " + val);

			return (*it).creator_;
		}

		FilterCreator_t FilterRegister::getCreator(FileTypes val)
		{
			typedef texmap_t::index<id>::type tex_by_id;
			tex_by_id::iterator it = texturefilters->get<id>().find(val);
			if(it == texturefilters->get<id>().end())
				throw LoaderNotFoundException("Texture Loader not found for texture of type : " + boost::lexical_cast<std::string>(val));

			return (*it).creator_;
		}

		int FilterRegister::RegisterRunTimeFilter(std::string const &key, FilterCreator_t creator)
		{
			int oldID = nextFilterID;
			texturefilters->insert(FilterCreatorDetails(nextFilterID, key, creator));
			++nextFilterID;
			return oldID;
		}

		void FilterRegister::UnRegisterRunTimeFilter(int filterid)
		{
			typedef texmap_t::index<id>::type tex_by_id;
			tex_by_id::iterator it = texturefilters->get<id>().find(filterid);
			if(it != texturefilters->get<id>().end())
				texturefilters->erase(it);
		}

	}
}