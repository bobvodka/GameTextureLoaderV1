// Main Header for the Game Texture Loader
// Defines all the public interface for the C++
// version of the code.

#ifndef GTL_GAMETEXTURELOADER_HPP
#define GTL_GAMETEXTURELOADER_HPP
#include "config.hpp"
#include <stdexcept>
#ifndef GTL_MANAGED
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#endif
#include <string>
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

		//FORMAT_FLOAT16,
		//FORMAT_FLOAT32,
		//some more 3DS formats follow
		FORMAT_R32G32B32A32F,	//4 channel fp32
		FORMAT_R16G16B16A16F,	//4 channel fp16
		FORMAT_G16R16F,			//2 channel fp16
		FORMAT_G32R32F,			//2 channel fp32
		FORMAT_R16F,			//1 channel fp16
		FORMAT_R32F,			//1 channel fp16

		//additional formats for dds mainly
		//rgb formats
		FORMAT_R5G6B5,			//16bit
		FORMAT_X1R5G5B5,		//15bit
		FORMAT_A1R5G5B5,		//15bit + 1 bit alpha
		FORMAT_L8,				//luminance 
		FORMAT_A8L8,			//alpha, luminance
		FORMAT_L16,				//luminance 16bit
		FORMAT_A8,				//alpha only
		FORMAT_G16R16,			//?? normal maps? L16A16 in opengl?

		//normal map formats
		FORMAT_V8U8,			//signed format, nv_texture_shader
		FORMAT_V16U16,			//signed, nv_texture_shader
		FORMAT_Q8W8V8U8,		//signed, nv_texture_shader

		// Additional formats for PNG images
		FORMAT_RGBA16,			// RGBA 16bit (not floating point)
		FORMAT_RGB16,			// RGB 16bit (not floating point)
		FORMAT_A16,				// 16bit alpha only
		FORMAT_A16L16			// 16bit alpha and luminance
	};

	const int TYPE_BMP = 1;
	const int TYPE_JPG = 2;
	const int TYPE_TGA = 3;
	const int TYPE_PNG = 4;
	const int TYPE_DDS = 5;

	enum ImgOrigin
	{
		ORIGIN_TOP_LEFT = 0,
		ORIGIN_TOP_RIGHT,
		ORIGIN_BOTTOM_LEFT,
		ORIGIN_BOTTOM_RIGHT
	};

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

	struct GTL_API Image
	{
		Image()
		{

		};
		Image(const Image &rhs)
		{

		};

		virtual ~Image(){};

		// Access functions
		unsigned char * getDataPtr(int mipmaplvl = 0, int imgnumber = 0);
		int getWidth(int mipmaplvl = 0);
		int getHeight(int mipmaplvl = 0);
		int getDepth(int mipmaplvl = 0);
		int getSize(int mipmaplvl = 0);
		int getNumMipMaps();
		int getNumImages();
		int getColourDepth();
		int getColorDepth();	// For our American friends
		ImgFormat getFormat();

		void decompress();	// decompress a DDS texture

	private:
		virtual unsigned char * getDataPtrImpl( int mipmaplvl, int imgnumber) = 0;
		virtual int getWidthImpl(int mipmaplvl) = 0;
		virtual int getHeightImpl(int mipmaplvl) = 0;
		virtual int getDepthImpl(int mipmaplvl) = 0;
		virtual int getSizeImpl(int mipmaplvl) = 0;
		virtual int getNumMipMapsImpl() = 0;
		virtual int getNumImagesImpl() = 0;
		virtual int getColourDepthImpl() = 0;
		virtual ImgFormat getFormatImpl() = 0;
		virtual void decompressImpl() = 0;
	};

#ifndef GTL_MANAGED
	typedef boost::shared_ptr<Image> ImagePtr;
	typedef boost::function<std::streamsize (char *, std::streamsize)> ReadFunc_t;
	typedef boost::function<std::streampos (boost::iostreams::stream_offset, std::ios_base::seekdir)> SeekFunc_t;
#endif	

	class LoaderNotFoundException : public std::runtime_error
	{
	public:
		LoaderNotFoundException (const std::string & msg) : std::runtime_error(msg)
		{
		}
	};

	// Texture Loading functions
	GTL_API Image* LoadTexture(std::string const &filename);
	GTL_API Image* LoadTexture(std::string const &filename, FileTypes val);
#ifndef GTL_MANAGED
	Image* LoadTexture(ReadFunc_t reader, SeekFunc_t seeker, FileTypes val);
#endif

	GTL_API void FreeTexture(Image* img);

	// Sets up the origin for all images loaded after this call
	GTL_API void SetOrigin(ImgOrigin origin);

#ifndef GTL_MANAGED
	// inline "Safe" Loading functions which return a Boost::shared_ptr
	typedef boost::shared_ptr<Image> ImagePtr;
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
#endif

#if defined(GTL_PHYSFS_SUPPORT)
	GTL_API Image* LoadTexture(PHYSFS_File* file, FileTypes val);
	GTL_API Image* LoadTexture(PHYSFS_File*file, std::string const &filename);

	inline ImagePtr LoadTextureSafe(PHYSFS_File* file, FileTypes val)
	{
		return ImagePtr(LoadTexture(file,val),FreeTexture);
	}

	inline ImagePtr LoadTextureSafe(PHYSFS_File* file, std::string const &filename)
	{
		return ImagePtr(LoadTexture(file,filename.c_str()),FreeTexture);
	}
#endif

	// Some aux functionality
	// probably wants to end up in its own name space

//	void DecompressImage(Image * img);	// decompresses a DXT image
}

#endif
