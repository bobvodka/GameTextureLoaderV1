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
			: height_(height), width_(width), depth_(depth), colourdepth_(colourdepth), format_(format), numImages_(numImages), 
			numMipMaps_(numMipMaps),imgdata_(imgdata)
		{

		};
		ImageImpl(const ImageImpl &rhs) : height_(rhs.height_), width_(rhs.width_), depth_(rhs.depth_), colourdepth_(rhs.colourdepth_), format_(rhs.format_), numImages_(rhs.numImages_), 
			numMipMaps_(rhs.numMipMaps_),imgdata_(rhs.imgdata_)
		{

		};
		ImageImpl() : height_(0), width_(0), depth_(0), colourdepth_(0), format_(FORMAT_NONE), numImages_(0), 
			numMipMaps_(0),imgdata_(NULL)
		{
		};

		~ImageImpl()
		{
		};

		virtual unsigned char * getDataPtrImpl(int mipmaplvl,int imgnumber);
		virtual int getWidthImpl(int mipmaplvl);
		virtual int getHeightImpl(int mipmaplvl);
		virtual int getDepthImpl(int mipmaplvl);
		virtual int getSizeImpl(int mipmaplvl);
		virtual int getNumMipMapsImpl();
		virtual int getNumImagesImpl();
		virtual int getColourDepthImpl();
		virtual ImgFormat getFormatImpl();
		int getMipMapChainSize(int mipmaplvl);
		void decompressImpl();

		int height_;
		int width_;
		int depth_;
		int colourdepth_;
		ImgFormat format_;
		int numImages_;
		int numMipMaps_;
		ImageData_t imgdata_;
private:	
		ImageImpl operator=(ImageImpl const &rhs);
		void setFormat(ImgFormat format);
		void setDataPtr(ImageData_t dataptr);
		void setColorDepth(int depth);
	};

	// Some Helper functions
	std::string ExtractFileExtension(std::string const &filename);

	void CreateImageInfo(LoaderImgData_t &headerdata, Imageinfo &infodata);
	void FlipX(ImageData_t &src, LoaderImgData_t &headerdata);
	void FlipY(ImageData_t &src, LoaderImgData_t &headerdata);
	void SetFlips(LoaderImgData_t &imgdata, bool vertical, bool horizontal);

	// DXT decoding functionality
	void decodeColorBlock(unsigned char *dest, const int w, const int h, const int xOff, const int yOff, const ImgFormat format, unsigned char *src);
	void decodeDXT3AlphaBlock(unsigned char *dest, const int w, const int h, const int xOff, const int yOff, unsigned char *src);
	void decodeDXT5AlphaBlock(unsigned char *dest, const int w, const int h, const int xOff, const int yOff, unsigned char *src);
	void decodeCompressedImage(unsigned char *dest, unsigned char *src, const int width, const int height, const ImgFormat format);

}
#endif
