// DDS Loading filter
// Can deal with both compressed and uncompressed data, leaves compressed data compressed!


#include <boost/iostreams/categories.hpp> // input_filter_tag
#include <boost/iostreams/operations.hpp>

#include <boost/cstdint.hpp>
#include <algorithm>
#include <cassert>

#include <gtl/FilterBase.hpp>
#include "../internals/utils.hpp"

// Note: This might cause troubles on windows system with big endian CPUs,
//       if recent windows versions ever get ported to such platform. ;-)
#ifdef WIN32
#define __i386__
#endif

/**
 * Convert an platform specific int32 to little endian. Does nothing
 * with the passed value on little endian machines.
 */
//static inline boost::int32_t little_endian(boost::int32_t v)
//{
//#if defined (__i386__)
//	return v;
//#elif defined (__ppc__) || defined(__ppc64__)
//	return (0x000000FF & v) << 24 |
//	       (0x0000FF00 & v) << 8  |
//	       (0x00FF0000 & v) >> 8  |
//	       (0xFF000000 & v) >> 24;
//#else
//	#warning "Auto detecting endianess."
//	// auto detect byte order
//	boost::int8_t a[4] = {
//		(v      ) & 0xFF,  // LSB
//		(v >>  8) & 0xFF,
//		(v >> 16) & 0xFF,
//		(v >> 24) & 0xFF   // MSB
//	};
//	return *((boost::int32_t*) a);
//#endif
//}

namespace GameTextureLoader { namespace Filters
{

	#define FOURCC(c0, c1, c2, c3) (c0 | (c1 << 8) | (c2 << 16) | (c3 << 24))

	// header should contain 'DDS ' (yes, that space is meant to be there!)
	namespace DDS
	{
//#pragma pack (push, 1)
		struct DDSStruct
		{
			boost::uint32_t 	size;		// equals size of struct (which is part of the data file!)
			boost::uint32_t	flags;
			boost::uint32_t	height,width;
			boost::uint32_t	sizeorpitch;
			boost::uint32_t	depth;
			boost::uint32_t	mipmapcount;
			boost::uint32_t	reserved[11];
			struct pixelformatstruct
			{
				boost::uint32_t	size;	// equals size of struct (which is part of the data file!)
				boost::uint32_t	flags;
				boost::uint32_t	fourCC;
				boost::uint32_t	RGBBitCount;
				boost::uint32_t	rBitMask;
				boost::uint32_t	gBitMask;
				boost::uint32_t	bBitMask;
				boost::uint32_t	alpahbitmask;
			} pixelformat;
			struct ddscapsstruct
			{
				boost::uint32_t	caps1,caps2;
				boost::uint32_t  reserved[2];
			} ddscaps;
			boost::uint32_t reserved2;
			//#ifndef __i386__
			//void to_little_endian()
			//{
			//	size_t size = sizeof(DDSStruct);
			//	assert(size % 4 == 0);
			//	size /= 4;
			//	for (size_t i=0; i<size; i++)
			//	{
			//		((int32_t*) this)[i] = little_endian(((int32_t*) this)[i]);
			//	}
			//}
			//#endif
		};
//#pragma pack (pop)
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
				if (!ReadHeader( *src, surfacedata_))
					return false;

				if (surfacedata_.mipmapcount==0)
					surfacedata_.mipmapcount=1;

				imgdata_.height = surfacedata_.height;
				imgdata_.width = surfacedata_.width;

				if(surfacedata_.flags & DDS::DDSD_DEPTH)
					imgdata_.depth = surfacedata_.depth;
				else
					imgdata_.depth = 0;	// no depth to these images

				imgdata_.colourdepth = surfacedata_.pixelformat.RGBBitCount;
				imgdata_.numMipMaps = surfacedata_.mipmapcount;
				imgdata_.format = getTextureFormat();
				imgdata_.numImages = getNumImages();
				imgdata_.size = calculateStoreageSize();
				if(0 >= imgdata_.size)
					return -1;

				
				if(-1 == imgdata_.format)
					return -1;
				SetFlips(imgdata_,false,false);
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
				int width=imgdata_.width;
				int height=imgdata_.height;
				int depth=imgdata_.depth;

				for (int m=0; m<imgdata_.numMipMaps; ++m)
				{
					size+=Utils::GetMipLevelSize(width, height, depth, imgdata_.format);
					width = std::max(width>>1, 1);
					height = std::max(height>>1, 1);
					depth = std::max(depth>>1, 1);
				}
			}

			return size;
		}

/*
		int calculateUnCompressedSize( )
		{
			int width = imgdata_.width;
			int height = imgdata_.height;
			int size = 0;
			// uncompressed texture
			// calculate the max size, reduce dimensions by half and repeat
			int bbp = surfacedata_.pixelformat.RGBBitCount/8;
			for(int i = 0; i < (int)surfacedata_.mipmapcount; ++i)
			{
				size += std::max(1,width) * std::max(height,1)*bbp;
				width = width/2;
				height = height/2;
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
			for(int i = 0; i < (int)surfacedata_.mipmapcount; ++i)
			{
				size += std::max(1,(width+3)/4) * std::max(1, (height+3)/4) * minsize;
				width /= 2;
				height /= 2;
			}
			return size;
		}
*/
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
					if (0xff == surfacedata_.pixelformat.bBitMask &&
						0xff00 == surfacedata_.pixelformat.gBitMask &&
						0xff0000 == surfacedata_.pixelformat.rBitMask &&
						0xff000000 == surfacedata_.pixelformat.alpahbitmask)
					{
						format = FORMAT_BGRA;
					} else if (	0xff == surfacedata_.pixelformat.rBitMask &&
								0xff00 == surfacedata_.pixelformat.gBitMask &&
								0xff0000 == surfacedata_.pixelformat.bBitMask &&
								0xff000000 == surfacedata_.pixelformat.alpahbitmask)
					{
						format = FORMAT_RGBA;
					} else if (	0xff == surfacedata_.pixelformat.alpahbitmask &&
								0xff00 == surfacedata_.pixelformat.bBitMask &&
								0xff0000 == surfacedata_.pixelformat.gBitMask &&
								0xff000000 == surfacedata_.pixelformat.rBitMask)
					{
						format = FORMAT_ABGR;
					} else if (	0x8000 == surfacedata_.pixelformat.alpahbitmask &&
								0x1F == surfacedata_.pixelformat.bBitMask &&
								0x3E0 == surfacedata_.pixelformat.gBitMask &&
								0x7C00 == surfacedata_.pixelformat.rBitMask)
					{
						format = FORMAT_A1R5G5B5;
					}
				}
				else
				{
					if (0xff == surfacedata_.pixelformat.bBitMask &&
						0xff00 == surfacedata_.pixelformat.gBitMask &&
						0xff0000 == surfacedata_.pixelformat.rBitMask )
					{
						format = FORMAT_BGRA;
					} else if (	0xff == surfacedata_.pixelformat.rBitMask &&
								0xff00 == surfacedata_.pixelformat.gBitMask &&
								0xff0000 == surfacedata_.pixelformat.bBitMask )
					{
						format = FORMAT_RGBA;
					} else if (	0xffFF == surfacedata_.pixelformat.rBitMask &&
								0xffFF0000 == surfacedata_.pixelformat.gBitMask &&
								0x00 == surfacedata_.pixelformat.bBitMask &&
								0x00 == surfacedata_.pixelformat.alpahbitmask)
					{
						format = FORMAT_G16R16;
					} else if (	0x1F == surfacedata_.pixelformat.bBitMask &&
								0x3E0 == surfacedata_.pixelformat.gBitMask &&
								0x7C00 == surfacedata_.pixelformat.rBitMask )
					{
						format = FORMAT_X1R5G5B5;
					} else if (	0x1F == surfacedata_.pixelformat.bBitMask &&
								0x7E0 == surfacedata_.pixelformat.gBitMask &&
								0xF800 == surfacedata_.pixelformat.rBitMask )
					{
						format = FORMAT_R5G6B5;
					}
				}
			} else
			{
				if (0xFF==surfacedata_.pixelformat.rBitMask &&
					0x0==surfacedata_.pixelformat.gBitMask &&
					0x0==surfacedata_.pixelformat.bBitMask &&
					0x0==surfacedata_.pixelformat.alpahbitmask)
				{
					format = FORMAT_L8;
				} else if (	0xFFFF==surfacedata_.pixelformat.rBitMask &&
							0x0==surfacedata_.pixelformat.gBitMask &&
							0x0==surfacedata_.pixelformat.bBitMask &&
							0x0==surfacedata_.pixelformat.alpahbitmask)
				{
					format = FORMAT_L16;
				} else if (	0x0==surfacedata_.pixelformat.rBitMask &&
							0x0==surfacedata_.pixelformat.gBitMask &&
							0x0==surfacedata_.pixelformat.bBitMask &&
							0xFF==surfacedata_.pixelformat.alpahbitmask)
				{
					format = FORMAT_A8;
				} else if (	0xFF==surfacedata_.pixelformat.rBitMask &&
							0x0==surfacedata_.pixelformat.gBitMask &&
							0x0==surfacedata_.pixelformat.bBitMask &&
							0xFF00==surfacedata_.pixelformat.alpahbitmask)
				{
					format = FORMAT_A8L8;
				} else if (	0xFF==surfacedata_.pixelformat.rBitMask &&
							0xFF00==surfacedata_.pixelformat.gBitMask &&
							0x0==surfacedata_.pixelformat.bBitMask &&
							0x0==surfacedata_.pixelformat.alpahbitmask)
				{
					format = FORMAT_V8U8;
				} else if (	0xFF==surfacedata_.pixelformat.rBitMask &&
							0xFF00==surfacedata_.pixelformat.gBitMask &&
							0xFF0000==surfacedata_.pixelformat.bBitMask &&
							0xFF000000==surfacedata_.pixelformat.alpahbitmask)
				{
					format = FORMAT_Q8W8V8U8;
				} else if (	0xFFFF==surfacedata_.pixelformat.rBitMask &&
							0xFFFF0000==surfacedata_.pixelformat.gBitMask &&
							0x0==surfacedata_.pixelformat.bBitMask &&
							0x0==surfacedata_.pixelformat.alpahbitmask)
				{
					format = FORMAT_V16U16;
				}
			}
			return format;
		}

		int getNumImages()
		{
			if(!(surfacedata_.ddscaps.caps2 & DDS::DDSCAPS2_CUBEMAP))
				return 1;

			// We are a cube map, so work out how many sides we have
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
			case 0x74:
				format=FORMAT_R32G32B32A32F;
				break;
			case 0x71:
				format=FORMAT_R16G16B16A16F;
				break;
			case 0x70:
				format=FORMAT_G16R16F;
				break;
			case 0x73:
				format=FORMAT_G32R32F;
				break;
			case 0x6F:
				format=FORMAT_R16F;
				break;
			case 0x72:
				format=FORMAT_R32F;
				break;
			default:
				break;
			}
			return format;
		}


		boost::uint32_t ReadDword( byte * & pData )
		{
			boost::uint32_t value=Utils::read32_le(pData);
			pData+=4;
			return value;
		}

		/**
		* New function to read in a header in a portable way (no more pragmas needed)
		*/
		template<typename Source>
		bool ReadHeader( Source & src, DDS::DDSStruct & header)
		{
			const int headerSize=128;
			byte data[headerSize];
			byte * pData=data;
			if (headerSize!=io::read(src,reinterpret_cast<typename Source::char_type*>(pData), headerSize)) 
			{
				return false;
			}

			if (! (pData[0]=='D' && pData[1]=='D' && pData[2]=='S' && pData[3]==' ') )
			{
				return false;
			}
			pData+=4;

			header.size=ReadDword(pData);
			if (header.size!=124)
			{
				return false;
			}

			//convert the data
			header.flags=ReadDword(pData);
			header.height=ReadDword(pData);
			header.width=ReadDword(pData);
			header.sizeorpitch=ReadDword(pData);
			header.depth=ReadDword(pData);
			header.mipmapcount=ReadDword(pData);

			for (int i=0; i<11; ++i)
			{
				header.reserved[i]=ReadDword(pData);
			}
			
			//pixelfromat
			header.pixelformat.size=ReadDword(pData);
			header.pixelformat.flags=ReadDword(pData);
			header.pixelformat.fourCC=ReadDword(pData);
			header.pixelformat.RGBBitCount=ReadDword(pData);
			header.pixelformat.rBitMask=ReadDword(pData);
			header.pixelformat.gBitMask=ReadDword(pData);
			header.pixelformat.bBitMask=ReadDword(pData);
			header.pixelformat.alpahbitmask=ReadDword(pData);

			//caps
			header.ddscaps.caps1=ReadDword(pData);
			header.ddscaps.caps2=ReadDword(pData);
			header.ddscaps.reserved[0]=ReadDword(pData);
			header.ddscaps.reserved[1]=ReadDword(pData);
			header.reserved2=ReadDword(pData);

			return true;
		}

		LoaderImgData_t &imgdata_;
		bool headerdone_;
		DDS::DDSStruct surfacedata_;
	};

	filterptr MakeDDSFilter(LoaderImgData_t &imgdata)
	{
		return filterptr(new DDSFilter(imgdata));
	}

	DECLARE_TEXTURE_LOADER(TYPE_DDS,"DDS",MakeDDSFilter)
}
}
