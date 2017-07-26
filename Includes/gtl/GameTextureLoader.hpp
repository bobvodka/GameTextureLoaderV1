// Main Header for the Game Texture Loader
// Defines all the public interface for the C++
// version of the code.

#ifndef GTL_GAMETEXTURELOADER_HPP
#define GTL_GAMETEXTURELOADER_HPP
#include "config.hpp"
#include <stdexcept>
#include <boost/shared_ptr.hpp>
#include <string>
#include <boost/function.hpp>
#include <iosfwd>
#include <boost/iostreams/positioning.hpp>

#if defined(GTL_PHYSFS_SUPPORT)
	#include <physfs.h>
#endif

namespace GameTextureLoader
{
	enum ImgFormat
	{
		FORMAT_NONE = 0,
		FORMAT_RGB,
		FORMAT_BGR,
		FORMAT_RGBA,
		FORMAT_BGRA,
		FORMAT_ABGR,
		FORMAT_DXT1,
		FORMAT_DXT2,
		FORMAT_DXT3,
		FORMAT_DXT4,
		FORMAT_DXT5,
		FORMAT_3DC,
		FORMAT_FLOAT16,
		FORMAT_FLOAT32
	};

	const int TYPE_BMP = 1;
	const int TYPE_JPG = 2;
	const int TYPE_TGA = 3;
	const int TYPE_PNG = 4;
	const int TYPE_DDS = 5;

	typedef int FileTypes;

	struct Imageinfo 
	{
		int height;
		int width;
		int depth;
		int colourdepth;
		ImgFormat format;
		int numImages;
		int numMipMaps;
	};

	struct Image
	{
		Image()
		{

		};
		Image(const Image &rhs)
		{

		};

		virtual ~Image(){};

		// Access functions
		unsigned char * getDataPtr(int imgnumber = 0, int mipmaplvl = 0);
		int getWidth(int mipmaplvl = 0);
		int getHeight(int mipmaplvl = 0);
		int getDepth(int mipmaplvl = 0);
		int getSize(int mipmaplvl = 0);
		int getNumMipMaps();
		int getNumImages();
		int getColourDepth();
		ImgFormat getFormat();

	private:
		virtual unsigned char * getDataPtrImpl(int imgnumber, int mipmaplvl) = 0;
		virtual int getWidthImpl(int mipmaplvl) = 0;
		virtual int getHeightImpl(int mipmaplvl) = 0;
		virtual int getDepthImpl(int mipmaplvl) = 0;
		virtual int getSizeImpl(int mipmaplvl) = 0;
		virtual int getNumMipMapsImpl() = 0;
		virtual int getNumImagesImpl() = 0;
		virtual int getColourDepthImpl() = 0;
		virtual ImgFormat getFormatImpl() = 0;
	};

	typedef boost::function<std::streamsize (char *, std::streamsize)> ReadFunc_t;
	typedef boost::function<std::streampos (boost::iostreams::stream_offset, std::ios_base::seekdir)> SeekFunc_t;
	typedef boost::shared_ptr<Image> ImagePtr;

	class LoaderNotFoundException : public std::runtime_error
	{
	public:
		LoaderNotFoundException (const std::string & msg) : std::runtime_error(msg)
		{
		}
	};

	// Texture Loading functions
	Image* LoadTexture(std::string const &filename);
	Image* LoadTexture(std::string const &filename, FileTypes val);
	Image* LoadTexture(ReadFunc_t reader, SeekFunc_t seeker, FileTypes val);
	
	void FreeTexture(Image* img);

	// inline "Safe" Loading functions which return a Boost::shared_ptr
	inline ImagePtr LoadTextureSafe(std::string const &filename)
	{
		return ImagePtr(LoadTexture(filename.c_str()),FreeTexture);
	}

	inline ImagePtr LoadTextureSafe(std::string const &filename, FileTypes val)
	{
		return ImagePtr(LoadTexture(filename.c_str(),val),FreeTexture);
	}

	inline ImagePtr LoadTextureSafe(ReadFunc_t reader, SeekFunc_t seeker, FileTypes val)
	{
		return ImagePtr(LoadTexture(reader,seeker,val),FreeTexture);
	}

#if defined(GTL_PHYSFS_SUPPORT)
	Image* LoadTexture(PHYSFS_File* file, FileTypes val);
	Image* LoadTexture(PHYSFS_File*file, std::string const &filename);

	inline ImagePtr LoadTextureSafe(PHYSFS_File* file, FileTypes val)
	{
		return ImagePtr(LoadTexture(file,val),FreeTexture);
	}

	inline ImagePtr LoadTextureSafe(PHYSFS_File* file, std::string const &filename)
	{
		return ImagePtr(LoadTexture(file,filename.c_str()),FreeTexture);
	}
#endif
}

#endif