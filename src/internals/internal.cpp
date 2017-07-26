// Internal structures etc implementations
// not for external users to see
#pragma warning(push)	//disable some warnings for this cpp FileDevice
#pragma warning(disable : 4996)	// disable complains about not using the secure CRT in 
#include <gtl/GameTextureLoader.hpp>
#include "internal.hpp"
#include "utils.hpp"

// boost and the string header
#include <algorithm>
#pragma warning(pop)

namespace GameTextureLoader
{
	int ImageImpl::getMipMapChainSize(int mipmaplvl)
	{
		if(mipmaplvl == 0)
			return 0;

		int size = 0;
		for(int i = 0; i < mipmaplvl; ++i)
			size += getSize(i);

		return size;
	}

	unsigned char * ImageImpl::getDataPtrImpl(int imgnumber, int mipmaplvl)
	{	
		unsigned char* dataptr = imgdata.get();

		for(int i = 0; i < imgnumber; ++i)
			dataptr += getMipMapChainSize(numMipMaps);

		dataptr += getMipMapChainSize(mipmaplvl);		
		return dataptr;
	}

	int ImageImpl::getWidthImpl(int mipmaplvl)
	{
		return std::max(width / (1 << mipmaplvl),1);
	}
	int ImageImpl::getHeightImpl(int mipmaplvl)
	{
		return std::max(height / (1 << mipmaplvl), 1);
	}
	int ImageImpl::getDepthImpl(int mipmaplvl)	// Returns the number of slices at this mipmap level
	{
		return std::max(depth / (1 << mipmaplvl),1);
	}
	int ImageImpl::getSizeImpl( int mipmaplvl)
	{
		int minsize = Utils::getMinSize(format);
		int size = 0;
		int width = getWidthImpl(mipmaplvl);
		int height = getHeightImpl(mipmaplvl);
		int depth = getDepthImpl(mipmaplvl);

		switch(format) 
		{
		case FORMAT_DXT1:
		case FORMAT_DXT2:
		case FORMAT_DXT3:
		case FORMAT_DXT4:
		case FORMAT_DXT5:
		case FORMAT_3DC:
			size = std::max(1,width/4) * std::max(1, height/4) * minsize;
			break;
		default:
			size = width * height * (colourdepth/8);		
			size *= (depth != 0 ? depth : 1);
			break;
		}
		return size;
	}

	int ImageImpl::getNumMipMapsImpl()
	{
		return numMipMaps;
	}
	int ImageImpl::getNumImagesImpl()
	{
		return numImages;
	}
	int ImageImpl::getColourDepthImpl()
	{
		return colourdepth;
	}
	ImgFormat ImageImpl::getFormatImpl()
	{
		return format;
	}

	// Helper functions
	void FlipY(ImageData_t &src, LoaderImgData_t &headerdata)
	{
		ImageData_t dest(new unsigned char[headerdata.size]);

		ImageData_t::element_type * srcptr = src.get();
		ImageData_t::element_type * destptr = dest.get();
		size_t const rowsize = headerdata.width * (headerdata.colourdepth/8);
		size_t const height = headerdata.height;
		for(size_t row = 0; row < height; ++row)
		{
			size_t srcoffset = rowsize * (height - row - 1);
			std::copy(src.get() + srcoffset , src.get() + srcoffset + rowsize, dest.get() + rowsize * row);
		}
		src = dest;
	}

	void FlipX(ImageData_t &src, LoaderImgData_t &headerdata)
	{
		ImageData_t dest(new unsigned char[headerdata.size]);

		size_t const rowsize = headerdata.width * (headerdata.colourdepth/8);
		size_t const height = headerdata.height;
		size_t const width = headerdata.width / 2;
		size_t const bbp = headerdata.colourdepth/8;
		for(size_t row = 0; row < height; ++row)
		{
			for(size_t col = 0, srcbyte = 0; col < width; ++col, srcbyte += bbp)
			{
				ImageData_t::element_type tmp;
				for(size_t i = 0; i < bbp; ++i)
				{
					tmp = src[srcbyte + i];
					src[srcbyte + i] = dest[rowsize - bbp + i];
					dest[rowsize - bbp + i] = tmp;
				}
			}
		}
		src = dest;
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
}