// DDS Loading filter
// Can deal with both compressed and uncompressed data, leaves compressed data compressed!


#include <boost/iostreams/categories.hpp> // input_filter_tag
#include <boost/iostreams/operations.hpp>

#include <boost/cstdint.hpp>
#include <algorithm>
#include <cassert>

#include <gtl/FilterBase.hpp>
#include "../internals/utils.hpp"

namespace GameTextureLoader { namespace Filters
{

	#define FOURCC(c0, c1, c2, c3) (c0 | (c1 << 8) | (c2 << 16) | (c3 << 24))

	// header should contain 'DDS ' (yes, that space is ment to be there!)
	namespace DDS
	{
#pragma pack (push, 1)
		struct DDSStruct
		{
			boost::int32_t 	size;		// equals size of struct (which is part of the data file!)
			boost::int32_t	flags;
			boost::int32_t	height,width;
			boost::int32_t	sizeorpitch;
			boost::int32_t	depth;
			boost::int32_t	mipmapcount;
			boost::int32_t	reserved[11];
			struct pixelformatstruct
			{
				boost::int32_t	size;	// equals size of struct (which is part of the data file!)
				boost::int32_t	flags;
				boost::int32_t	fourCC;
				boost::int32_t	RGBBitCount;
				boost::int32_t	rBitMask;
				boost::int32_t	gBitMask;
				boost::int32_t	bBitMask;
				boost::int32_t	alpahbitmask;
			} pixelformat;
			struct ddscapsstruct
			{
				boost::int32_t	caps1,caps2;
				boost::int32_t  reserved[2];
			} ddscaps;
			boost::int32_t reserved2;
		};
#pragma pack (pop)
		// DDSStruct Flags
		const boost::int32_t	DDSD_CAPS = 0x00000001;
		const boost::int32_t	DDSD_HEIGHT = 0x00000002;
		const boost::int32_t	DDSD_WIDTH = 0x00000004;
		const boost::int32_t	DDSD_PITCH = 0x00000008;
		const boost::int32_t	DDSD_PIXELFORMAT = 0x00001000;
		const boost::int32_t	DDSD_MIPMAPCOUNT = 0x00020000;
		const boost::int32_t	DDSD_LINEARSIZE = 0x00080000;
		const boost::int32_t	DDSD_DEPTH = 0x00800000;

		// pixelformat values
		const boost::int32_t	DDPF_ALPHAPIXELS = 0x00000001;
		const boost::int32_t	DDPF_FOURCC = 0x00000004;
		const boost::int32_t	DDPF_RGB = 0x00000040;

		// ddscaps
		// caps1
		const boost::int32_t	DDSCAPS_COMPLEX = 0x00000008;
		const boost::int32_t	DDSCAPS_TEXTURE = 0x00001000;
		const boost::int32_t	DDSCAPS_MIPMAP = 0x00400000;
		// caps2
		const boost::int32_t	DDSCAPS2_CUBEMAP = 0x00000200;
		const boost::int32_t	DDSCAPS2_CUBEMAP_POSITIVEX = 0x00000400;
		const boost::int32_t	DDSCAPS2_CUBEMAP_NEGATIVEX = 0x00000800;
		const boost::int32_t	DDSCAPS2_CUBEMAP_POSITIVEY = 0x00001000;
		const boost::int32_t	DDSCAPS2_CUBEMAP_NEGATIVEY = 0x00002000;
		const boost::int32_t	DDSCAPS2_CUBEMAP_POSITIVEZ = 0x00004000;
		const boost::int32_t	DDSCAPS2_CUBEMAP_NEGATIVEZ = 0x00008000;
		const boost::int32_t	DDSCAPS2_VOLUME = 0x00200000;

	};

	class DDSFilter : public FilterBase
	{

	public:
		typedef char                       char_type;
		typedef io::multichar_input_filter_tag  category;

		DDSFilter(LoaderImgData_t &imgdata) : imgdata_(imgdata),headerdone_(false)
		{
			memset(&surfacedata_,0,sizeof(surfacedata_));
		};

		DDSFilter(DDSFilter const &lhs) : imgdata_(lhs.imgdata_),headerdone_(lhs.headerdone_),surfacedata_(lhs.surfacedata_)
		{

		};

		~DDSFilter()
		{

		}

		virtual std::streamsize read(streambuf_t *src,char * s, std::streamsize n)
		{

			if(!headerdone_)
			{
				// Read in header and decode
				boost::uint32_t header = 0;
				if(io::read(*src,reinterpret_cast</*Source*/streambuf_t::char_type*>(&header),4) == -1)
					return -1;

				if(FOURCC('D','D','S',' ') != header)
					return -1;

				// Load DDS header.
				if(io::read(*src,reinterpret_cast</*Source*/streambuf_t::char_type*>(&surfacedata_),sizeof(DDS::DDSStruct)) == -1)
					return -1;

				assert(surfacedata_.size == 124);
				imgdata_.height = surfacedata_.height;
				imgdata_.width = surfacedata_.width;
				imgdata_.depth = surfacedata_.depth;
				imgdata_.colourdepth = surfacedata_.pixelformat.RGBBitCount;
				imgdata_.numMipMaps = surfacedata_.mipmapcount - 1;	// DDS always reports one mipmap, so -1 to standardise
				
				imgdata_.numImages = getNumImages();

				imgdata_.size = calculateStoreageSize();
				if(imgdata_.size == -1)
					return -1;

				imgdata_.format = getTextureFormat();
				if(imgdata_.format == -1)
					return -1;

				headerdone_ = true;
			}

			// Read in remaining data
			return io::read(*src,s,n);
		}
	protected:
	private:
		int getMinDXTSize()
		{
			return Utils::getMinSize(getDXTFormat());
		}

		int calculateStoreageSize()
		{
			int size = 0;
			for(int i = 0; i < imgdata_.numImages; ++i)
			{
				// To work out the size we need to take into account mipmap images
				if(surfacedata_.mipmapcount > 1)
				{
					if(surfacedata_.flags & DDS::DDSD_PITCH || surfacedata_.pixelformat.flags & DDS::DDPF_RGB)
					{
						size += calculateUnCompressedSize();
					}
					else if(surfacedata_.flags & DDS::DDSD_LINEARSIZE)
					{
						size += calculateCompressedSize();
					}
					else
						return size = -1;;	// we dont know how to work out the size!
				}
				else if(surfacedata_.flags & DDS::DDSD_PITCH)
					size += surfacedata_.height * surfacedata_.width * (surfacedata_.pixelformat.RGBBitCount/8);
				else if(surfacedata_.flags & DDS::DDSD_LINEARSIZE)
					size += surfacedata_.sizeorpitch;
				else
					size = -1;	// dont know how to work out the size!
			}
			return size;
		}
		int calculateUnCompressedSize()
		{
			int width = imgdata_.width;
			int height = imgdata_.height;
			int size = 0;
			// uncompressed texture
			// calculate the max size, reduce dimentions by half and repeat
			int bbp = surfacedata_.pixelformat.RGBBitCount/8;
			for(int i = 0; i < surfacedata_.mipmapcount; ++i)
			{
				size += width*height*bbp;
				width /= 2;
				height /= 2;
			}
			return size;

		}
		int calculateCompressedSize()
		{
			int width = imgdata_.width;
			int height = imgdata_.height;
			int size = 0;
			// compressed texture
			int minsize = getMinDXTSize();
			for(int i = 0; i < surfacedata_.mipmapcount; ++i)
			{
				size += std::max(1,width/4) * std::max(1, height/4) * minsize;
				width /= 2;
				height /= 2;
			}
			return size;
		}

		ImgFormat getTextureFormat()
		{
			ImgFormat format = FORMAT_NONE;

			if(surfacedata_.pixelformat.flags & DDS::DDPF_FOURCC)
			{
				format = getDXTFormat();

			} 
			else if(surfacedata_.pixelformat.flags & DDS::DDPF_RGB)
			{
				if(surfacedata_.pixelformat.flags & DDS::DDPF_ALPHAPIXELS)
				{
					if(surfacedata_.pixelformat.bBitMask = 0xff)
						format = FORMAT_BGRA;
					else
						format = FORMAT_RGBA;
				}
				else
				{
					if(surfacedata_.pixelformat.bBitMask = 0xff)
						format = FORMAT_BGR;
					else
						format = FORMAT_RGB;
				}
			}
			return format;
		}

		int getNumImages()
		{
			if(!(surfacedata_.ddscaps.caps2 & DDS::DDSCAPS2_CUBEMAP))
				return 1;

			// We are a cubemap, so work out how many sides we have
			boost::uint32_t mask = DDS::DDSCAPS2_CUBEMAP_POSITIVEX;
			int count = 0;
			for(int n = 0; n < 6; ++n)
			{
				if(surfacedata_.ddscaps.caps2 & mask)
					++count;
				mask *= 2;	// move to next face
			}
			return count;		
		}

		ImgFormat getDXTFormat()
		{
			ImgFormat format = FORMAT_NONE;
			switch(surfacedata_.pixelformat.fourCC) 
			{
			case FOURCC('D','X','T','1'):
				format = FORMAT_DXT1;
				break;
			case FOURCC('D','X','T','2'):
				format = FORMAT_DXT2;
				break;
			case FOURCC('D','X','T','3'):
				format = FORMAT_DXT3;
				break;
			case FOURCC('D','X','T','4'):
				format = FORMAT_DXT4;
				break;
			case FOURCC('D','X','T','5'):
				format = FORMAT_DXT5;
				break;
			case FOURCC('A','T','I','2'):
				format = FORMAT_3DC;
				break;
			default:
				break;
			}
			return format;
		}


		DDS::DDSStruct surfacedata_;
		LoaderImgData_t &imgdata_;
		bool headerdone_;
	};

	filterptr MakeDDSFilter(LoaderImgData_t &imgdata)
	{
		return filterptr(new DDSFilter(imgdata));
	}

	DECLARE_TEXTURE_LOADER(TYPE_DDS,"DDS",MakeDDSFilter)
}
}