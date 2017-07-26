// Internal structures etc implementations
// not for external users to see
#ifdef WIN32
#pragma warning(push)	//disable some warnings for this cpp FileDevice
#pragma warning(disable : 4996)	// disable complains about not using the secure CRT in 
#endif
#include <gtl/GameTextureLoader.hpp>
#include "internal.hpp"
#include "utils.hpp"

// boost and the string header
#include <algorithm>
#ifdef WIN32
#pragma warning(pop)
#endif

namespace GameTextureLoader
{
	int ImageImpl::getMipMapChainSize(int mipmaplvl)
	{
		if(mipmaplvl == 0)
			return 0;

		if (mipmaplvl < 0 || mipmaplvl > getNumMipMaps())
			throw std::out_of_range("mipmaplvl not in the range [0..getNumMipMaps()]");

		int size = 0;
		for(int i = 0; i < mipmaplvl; ++i)
			size += getSize(i);

		return size;
	}

	unsigned char * ImageImpl::getDataPtrImpl(int mipmaplvl, int imgnumber)
	{	
		if (mipmaplvl < 0 || mipmaplvl >= getNumMipMaps())
			throw std::out_of_range("mipmaplvl not in the range [0..getNumMipMaps()-1]");
		if (imgnumber < 0 || imgnumber >= getNumImages())
			throw std::out_of_range("imgnumber not in the range [0..getNumImages()-1]");

		unsigned char* dataptr = imgdata_.get();
		for(int i = 0; i < imgnumber; ++i)
			dataptr += getMipMapChainSize(getNumMipMaps());

		dataptr += getMipMapChainSize(mipmaplvl);		
		return dataptr;
	}

	int ImageImpl::getWidthImpl(int mipmaplvl)
	{
		if (mipmaplvl < 0 || mipmaplvl >= getNumMipMaps())
			throw std::out_of_range("mipmaplvl not in the range [0..getNumMipMaps()-1]");
		return std::max(width_ / (1 << mipmaplvl),1);
	}
	int ImageImpl::getHeightImpl(int mipmaplvl)
	{
		if (mipmaplvl < 0 || mipmaplvl >= getNumMipMaps())
			throw std::out_of_range("mipmaplvl not in the range [0..getNumMipMaps()-1]");
		return std::max(height_ / (1 << mipmaplvl), 1);
	}
	int ImageImpl::getDepthImpl(int mipmaplvl)	// Returns the number of slices at this mipmap level
	{
		if (mipmaplvl < 0 || mipmaplvl >= getNumMipMaps())
			throw std::out_of_range("mipmaplvl not in the range [0..getNumMipMaps()-1]");
		if(depth_ == 0)
			return depth_;
		
		return std::max(depth_ / (1 << mipmaplvl),1);
	}
	int ImageImpl::getSizeImpl( int mipmaplvl)
	{
		if (mipmaplvl < 0 || mipmaplvl >= getNumMipMaps())
			throw std::out_of_range("mipmaplvl not in the range [0..getNumMipMaps()-1]");

		//int minsize = Utils::getMinSize(getFormat());
		int size = 0;
		int width = getWidth(mipmaplvl);
		int height = getHeight(mipmaplvl);
		int depth = getDepth(mipmaplvl);

		return Utils::GetMipLevelSize(width, height, depth, this->format_);
/*
		switch(getFormat()) 
		{
		case FORMAT_DXT1:
		case FORMAT_DXT2:
		case FORMAT_DXT3:
		case FORMAT_DXT4:
		case FORMAT_DXT5:
		case FORMAT_3DC:
			size = std::max(1,(width+3)/4) * std::max(1, (height+3)/4) * minsize;
			break;
		default:
			size = width * height * (getColourDepth()/8);
			size *= (depth != 0) ? depth : 1;
			break;
		}
		return size;*/
	}

	int ImageImpl::getNumMipMapsImpl()
	{
		return numMipMaps_;
	}
	int ImageImpl::getNumImagesImpl()
	{
		return numImages_;
	}
	int ImageImpl::getColourDepthImpl()
	{
		return colourdepth_;
	}
	ImgFormat ImageImpl::getFormatImpl()
	{
		return format_;
	}

	void ImageImpl::decompressImpl()
	{
		// Decompresses this image
		// First check we are a compressed image type#
		ImgFormat srcformat = getFormat();
		if(srcformat < FORMAT_DXT1 || srcformat > FORMAT_DXT5)
			return;
		// Now we need to work out how much space we are going to need
		// First off work out our destination format
		ImgFormat destFormat = (getFormat() == FORMAT_DXT1)? FORMAT_RGB : FORMAT_RGBA;
		int destbpp;
		if(destFormat == FORMAT_RGBA)
			destbpp = 4;	// four bytes per pixel
		else if(destFormat == FORMAT_RGB)
			destbpp = 3;

		size_t totalspace = 0;
		for (int faces = 0; faces < getNumImages(); faces++)
		{
			for(int mipmaps = 0; mipmaps < getNumMipMaps() + 1; mipmaps++)
			{
				int width = getWidth(mipmaps);
				int height = getHeight(mipmaps);
				int depth = getDepth(mipmaps);
				int size = width * height * (destbpp);		
				size *= (depth != 0) ? depth : 1;
				totalspace += size;
			}
		}
		ImageData_t dest(new unsigned char[totalspace]);	// reserve as much space as we need to build this image
		unsigned char * dst = dest.get();
		
		// Now, we loop over all the image data and expand it
		for (int faces = 0; faces < getNumImages(); faces++)
		{
			for(int mipmaps = 0; mipmaps < getNumMipMaps() +1; mipmaps++)
			{
				int width = getWidth(mipmaps);
				int height = getHeight(mipmaps);
				int depth = getDepth(mipmaps);
				depth = (depth != 0) ? depth : 1;
				unsigned char * src = getDataPtr(mipmaps, faces);

				int dstSliceSize = width * height * destbpp;	
				int srcSliceSize = getSize(mipmaps);	// get how far we have to skip for this mipmap level

				for (int slice = 0; slice < depth; slice++)
				{
					decodeCompressedImage(dst, src, width, height, srcformat);
					// need to push forward a slice size
					dst += dstSliceSize;
					src += srcSliceSize;
				}
			
			}
		}
		setFormat(destFormat);	// set our new internal format
		setDataPtr(dest);		// and replace the data
		setColorDepth(destbpp*8);	// set the bits per pixel (bytes per pixel *8)
	}

	void ImageImpl::setFormat(ImgFormat format)
	{
		format_ = format;
	}
	void ImageImpl::setDataPtr(ImageData_t dataptr)
	{
		imgdata_ = dataptr;
	}

	void ImageImpl::setColorDepth(int depth)
	{
		colourdepth_ = depth;
	}

	// Helper functions
	void FlipY(ImageData_t &src, LoaderImgData_t &headerdata)
	{
		ImageData_t dest(new unsigned char[headerdata.size]);

		ImageData_t::element_type * srcptr = src.get();
		ImageData_t::element_type * destptr = dest.get();
		
		for(int faces = 0; faces < headerdata.numImages; faces++)
		{
			for(int mipmaplvl = 0; mipmaplvl < headerdata.numMipMaps; mipmaplvl++)	
			{
				size_t const rowsize = std::max(1, headerdata.width / (1 << mipmaplvl)) * (headerdata.colourdepth/8);
				size_t const height = std::max(1, headerdata.height / (1 << mipmaplvl));

				// Need to fix this so that srcptr goes from the END of a given mipmap to the start and copies into destptr
				// this is going to be tricky.
				// Off the top of my head;
				// we need to keep a note of where we are next going to start from and use a tmp ptr to work backwards from there
				// suggestion; move srcptr to the end of the image data for this mipmap and then assign it to a tmpptr
				// use that tmp ptr in the loop and then next time around move srcptr down the next image and so on.
				srcptr += rowsize * (height-1);
				ImageData_t::element_type * src = srcptr;
				for(size_t row = 0; row < height; ++row)
				{
					// size_t srcoffset = rowsize * (height - row - 1);
					std::copy(src /*+ srcoffset*/ , src /*+ srcoffset*/ + rowsize, destptr);
					src -= rowsize;
					destptr += rowsize;
				}
				srcptr += rowsize;	// move to the end of the current data
			}
		}
		
		src = dest;
	}

	// problem in here :(
	void FlipX(ImageData_t &src, LoaderImgData_t &headerdata)
	{
		ImageData_t dest(new unsigned char[headerdata.size]);

		size_t const rowsize = headerdata.width * (headerdata.colourdepth/8);
		size_t const height = headerdata.height;
		size_t const width = headerdata.width;
		size_t const bbp = headerdata.colourdepth/8;
		for(size_t row = 0; row < height; ++row)
		{
			size_t rowstart = row * rowsize;
			for(size_t col = 0, srcbyte = rowstart; col < width; ++col, srcbyte += bbp)
			{
				
				for(size_t i = 0; i < bbp; ++i)
				{
					dest[srcbyte + i] = src[rowstart + rowsize - (bbp*col) + i];
				}
			}
		}
		src = dest;
	}

	// Allows user to control the flipping
	extern bool xflip;
	extern bool yflip;

	void SetFlips(LoaderImgData_t &imgdata, bool vertical, bool horizontal)
	{
		imgdata.xflip = xflip ? horizontal : !horizontal;
		imgdata.yflip = yflip ? vertical : !vertical;
	}

	void CreateImageInfo(LoaderImgData_t &headerdata, Imageinfo &infodata)
	{
		infodata.colourdepth = headerdata.colourdepth;
		infodata.depth = headerdata.depth;
		infodata.format = headerdata.format;
		infodata.height = headerdata.height;
		infodata.numImages = headerdata.numImages;
		infodata.numMipMaps = headerdata.numMipMaps;
		infodata.width = headerdata.width;
	}

	std::string ExtractFileExtension(std::string const &filename)
	{
		std::string ext;
		std::string::size_type offset = filename.rfind('.');
		if( offset != std::string::npos)
		{
			ext = filename.substr(offset+1,filename.length() - offset);
		}
		else
			throw std::runtime_error("Bad Filename");

		std::transform(ext.begin(),ext.end(),ext.begin(),toupper);

		return ext;
	}

	// DXT decoding functionality
	int getChannelCount(ImgFormat format)
	{
		int rtn;
		switch(format)
		{
		case FORMAT_DXT1:
			rtn = 3;
			break;
		case FORMAT_DXT3:
		case FORMAT_DXT5:
			rtn = 4;
		    break;
		case FORMAT_3DC:
			rtn = 1;
		    break;
		default:
		    break;
		}
		return rtn;
	}

	void decodeColorBlock(unsigned char *dest, const int w, const int h, const int xOff, const int yOff, const ImgFormat format, unsigned char *src)
	{
		unsigned char colors[4][3];

		unsigned short c0 = *(unsigned short *) src;
		unsigned short c1 = *(unsigned short *) (src + 2);

		// Extract the two stored colors
		colors[0][0] = ((c0 >> 11) & 0x1F) << 3;
		colors[0][1] = ((c0 >>  5) & 0x3F) << 2;
		colors[0][2] =  (c0        & 0x1F) << 3;

		colors[1][0] = ((c1 >> 11) & 0x1F) << 3;
		colors[1][1] = ((c1 >>  5) & 0x3F) << 2;
		colors[1][2] =  (c1        & 0x1F) << 3;

		// Compute the other two colors
		if (c0 > c1 || format == FORMAT_DXT5){
			for (int i = 0; i < 3; i++){
				colors[2][i] = (2 * colors[0][i] +     colors[1][i] + 1) / 3;
				colors[3][i] = (    colors[0][i] + 2 * colors[1][i] + 1) / 3;
			}
		} else {
			for (int i = 0; i < 3; i++){
				colors[2][i] = (colors[0][i] + colors[1][i] + 1) >> 1;
				colors[3][i] = 0;
			}
		}

		src += 4;
		for (int y = 0; y < h; y++){
			unsigned char *dst = dest + yOff * y;
			unsigned int indexes = src[y];
			for (int x = 0; x < w; x++){
				unsigned int index = indexes & 0x3;
				dst[0] = colors[index][0];
				dst[1] = colors[index][1];
				dst[2] = colors[index][2];
				indexes >>= 2;

				dst += xOff;
			}
		}
	}
	void decodeDXT3AlphaBlock(unsigned char *dest, const int w, const int h, const int xOff, const int yOff, unsigned char *src)
	{
		for (int y = 0; y < h; y++){
			unsigned char *dst = dest + yOff * y;
			unsigned int alpha = ((unsigned short *) src)[y];
			for (int x = 0; x < w; x++){
				*dst = (alpha & 0xF) * 17;
				alpha >>= 4;
				dst += xOff;
			}
		}
	}
	void decodeDXT5AlphaBlock(unsigned char *dest, const int w, const int h, const int xOff, const int yOff, unsigned char *src)
	{
		unsigned char a0 = src[0];
		unsigned char a1 = src[1];
		boost::uint64_t alpha = (*(boost::uint64_t *) src) >> 16;

		for (int y = 0; y < h; y++){
			unsigned char *dst = dest + yOff * y;
			for (int x = 0; x < w; x++){
				int k = ((unsigned int) alpha) & 0x7;
				if (k == 0){
					*dst = a0;
				} else if (k == 1){
					*dst = a1;
				} else if (a0 > a1){
					*dst = ((8 - k) * a0 + (k - 1) * a1) / 7;
				} else if (k >= 6){
					*dst = (k == 6)? 0 : 255;
				} else {
					*dst = ((6 - k) * a0 + (k - 1) * a1) / 5;
				}
				alpha >>= 3;

				dst += xOff;
			}
			if (w < 4) alpha >>= (3 * (4 - w));
		}
	}
	void decodeCompressedImage(unsigned char *dest, unsigned char *src, const int width, const int height, const ImgFormat format)
	{
		int sx = (width  < 4)? width  : 4;
		int sy = (height < 4)? height : 4;

		int nChannels = getChannelCount(format);
		for (int y = 0; y < height; y += 4)
		{
			for (int x = 0; x < width; x += 4)
			{
				unsigned char *dst = dest + (y * width + x) * nChannels;
				if (format == FORMAT_DXT3)
				{
					decodeDXT3AlphaBlock(dst + 3, sx, sy, 4, width * 4, src);
					src += 8;
				} else if (format == FORMAT_DXT5)
				{
					decodeDXT5AlphaBlock(dst + 3, sx, sy, 4, width * 4, src);
					src += 8;
				}
				if (format <= FORMAT_DXT5)
				{
					decodeColorBlock(dst, sx, sy, nChannels, width * nChannels, format, src);
					src += 8;
				} else {
					if (format == /*FORMAT_ATI1N*/ FORMAT_3DC)
					{
						decodeDXT5AlphaBlock(dst, sx, sy, 1, width, src);
						src += 8;
					} else {
						decodeDXT5AlphaBlock(dst,     sx, sy, 2, width * 2, src + 8);
						decodeDXT5AlphaBlock(dst + 1, sx, sy, 2, width * 2, src);
						src += 16;
					}
				}
			}
		}
	}
}
