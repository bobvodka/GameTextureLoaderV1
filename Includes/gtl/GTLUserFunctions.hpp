//////////////////////////////////////////////////////////////////////////
// Header to support user defined call backs for GTL
//
// Defines some functions and a couple of typedefs needed for the call
// backs to work
//////////////////////////////////////////////////////////////////////////
#ifndef GTL_GTLUSERFUNCTIONS_HPP
#define GTL_GTLUSERFUNCTIONS_HPP
#include <boost/shared_array.hpp>
namespace GameTextureLoader
{
	typedef boost::shared_array<unsigned char> ImageData_t;
	
	typedef boost::function<void (GameTextureLoader::ImageData_t&, Imageinfo &)> LoadCallBack_t;

	Image* LoadTexture(std::string const &filename, LoadCallBack_t  callback);
	Image* LoadTexture(std::string const &filename, FileTypes val, LoadCallBack_t callback);
	Image* LoadTexture(ReadFunc_t reader, SeekFunc_t seeker, FileTypes val, LoadCallBack_t callback);

#if defined(GTL_PHYSFS_SUPPORT)
	Image* LoadTexture(PHYSFS_File* file, FileTypes val, LoadCallBack_t callback);
	Image* LoadTexture(PHYSFS_File*file, std::string const &filename, LoadCallBack_t callback);

	inline ImagePtr LoadTextureSafe(PHYSFS_File* file, FileTypes val, LoadCallBack_t callback)
	{
		return ImagePtr(LoadTexture(file,val,callback),FreeTexture);
	}

	inline ImagePtr LoadTextureSafe(PHYSFS_File* file, std::string const &filename, LoadCallBack_t callback)
	{
		return ImagePtr(LoadTexture(file,filename.c_str(),callback),FreeTexture);
	}
#endif

	namespace UserFunctionSupport
	{
		// Access functions for user callback functions
		unsigned char * getData(ImageData_t imgdata, Imageinfo const &img, int imgnumber = 0, int mipmaplvl = 0);
		int getWidth(Imageinfo const &img, int mipmaplvl = 0);
		int getHeight(Imageinfo const &img, int mipmaplvl = 0);
		int getDepth(Imageinfo const &img, int mipmaplvl = 0);
		int getSize(Imageinfo const &img, int mipmaplvl = 0);
	}
}
#endif