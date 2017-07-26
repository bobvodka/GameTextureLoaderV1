// Internal structures etc
// Not for public viewing
#ifndef GTL_INTERNAL_HPP
#define GTL_INTERNAL_HPP
#include <gtl/GameTextureLoader.hpp>
#include <gtl/GTLRunTimeTextureFilter.hpp>

namespace GameTextureLoader
{
	typedef boost::shared_array<unsigned char> ImageData_t; 

	struct ImageImpl : public Image
	{

		ImageImpl(int height, int width,int depth,int colourdepth,ImgFormat format,int numImages,int numMipMaps, ImageData_t imgdata)
			: height(height), width(width), depth(depth), colourdepth(colourdepth), format(format), numImages(numImages), 
			numMipMaps(numMipMaps),imgdata(imgdata)
		{

		};
		ImageImpl(const ImageImpl &rhs) : height(rhs.height), width(rhs.width), depth(rhs.depth), colourdepth(rhs.colourdepth), format(rhs.format), numImages(rhs.numImages), 
			numMipMaps(rhs.numMipMaps),imgdata(rhs.imgdata)
		{

		};
		ImageImpl() : height(0), width(0), depth(0), colourdepth(0), format(FORMAT_NONE), numImages(0), 
			numMipMaps(0),imgdata(NULL)
		{
		};

		~ImageImpl()
		{
		};

		virtual unsigned char * getDataPtrImpl(int imgnumber, int mipmaplvl);
		virtual int getWidthImpl(int mipmaplvl);
		virtual int getHeightImpl(int mipmaplvl);
		virtual int getDepthImpl(int mipmaplvl);
		virtual int getSizeImpl(int mipmaplvl);
		virtual int getNumMipMapsImpl();
		virtual int getNumImagesImpl();
		virtual int getColourDepthImpl();
		virtual ImgFormat getFormatImpl();
		int getMipMapChainSize(int mipmaplvl);

		const int height;
		const int width;
		const int depth;
		const int colourdepth;
		const ImgFormat format;
		const int numImages;
		const int numMipMaps;
		ImageData_t imgdata;
	private:
		ImageImpl operator=(ImageImpl const &rhs);
		
	};

	// Some Helper functions
	std::string ExtractFileExtension(std::string const &filename);

	void CreateImageInfo(LoaderImgData_t &headerdata, Imageinfo &infodata);
	void FlipX(ImageData_t &src, LoaderImgData_t &headerdata);
	void FlipY(ImageData_t &src, LoaderImgData_t &headerdata);

}
#endif