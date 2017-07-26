// Header to allow the addition of runtime texture filter/loaders
#ifndef GTL_GTLRUNTIMETEXTUREFILTER_HPP
#define GTL_GTLRUNTIMETEXTUREFILTER_HPP

#include <gtl/GameTextureLoader.hpp>
#include <boost/shared_array.hpp>
#include <boost/function.hpp>
#include <gtl/FilterBase.hpp>

namespace GameTextureLoader
{
	struct LoaderImgData
	{
		LoaderImgData():height(0),width(0),depth(0),colourdepth(0),size(0),numImages(0),numMipMaps(0),format(FORMAT_NONE),xflip(false),yflip(false)
		{
		};
		int height,width,depth,colourdepth,size;
		int numImages,numMipMaps;
		ImgFormat format;
		bool xflip;
		bool yflip;
	};

	typedef LoaderImgData LoaderImgData_t;
	
	typedef boost::function<Filters::filterptr (LoaderImgData_t&)> FilterCreator_t;

	int RegisterFilter(std::string const &ext, FilterCreator_t filterCreator);
	void UnRegisterFilter(int filterID);
}
#endif
