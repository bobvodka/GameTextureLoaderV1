// Bitmapfilter.hpp
// Defines an input filter for a bitmap file type
//
// Uses boost1.33's iostreams library

#include <boost/iostreams/categories.hpp> // input_filter_tag
#include <boost/iostreams/operations.hpp>

#include <gtl/FilterBase.hpp>
#include "../internals/utils.hpp"

namespace GameTextureLoader { namespace Filters
{
	namespace io = boost::iostreams;

	class bitmapfilter : public FilterBase
	{

		struct Header {
			bool os2;

			int file_size;
			int data_offset;
			int width;
			int height;
			int bpp;
			int compression;

			int pitch;  // number of bytes in each scanline
			int image_size;
		};


	public:
		typedef char                       char_type;
		typedef io::multichar_input_filter_tag  category;

		bitmapfilter(LoaderImgData_t &imgdata) : headerdone_(false),imgdata_(imgdata)
		{
		}
		~bitmapfilter()
		{
		}
		bitmapfilter(bitmapfilter const &lhs) : headerdone_(lhs.headerdone_), imgdata_(lhs.imgdata_)
		{
		}

		virtual std::streamsize read(streambuf_t *src,char * s, std::streamsize n)
		{
			if(!headerdone_)
			{
				Header bitmapheader;
				// Read in the header
				if(!ReadHeader(*src,bitmapheader))
					return -1;
				if(!ReadInfoHeader(*src,bitmapheader)/* &&
				!ReadPalette(src,bitmapheader)*/)
					return -1;
				// Extract infomation from bitmap header to imgdata struct
				imgdata_.height = bitmapheader.height;
				imgdata_.width = bitmapheader.width;
				if(bitmapheader.image_size != 0)
					imgdata_.size = bitmapheader.image_size;
				else
				{
					imgdata_.size = bitmapheader.width * bitmapheader.height * (bitmapheader.bpp/8);
				}
				imgdata_.colourdepth = bitmapheader.bpp;
				imgdata_.depth = 0;	// not a 3D image
				imgdata_.numImages = 1;	// Only one image stored
				imgdata_.numMipMaps = 0; // no mipmaps
				if(bitmapheader.os2 == true)
					imgdata_.format = FORMAT_RGB;
				else
					imgdata_.format = FORMAT_BGR;
				imgdata_.yflip = true;
				headerdone_ = true;
			}

			// Now read in the data proper in chunks
			//std::cout << "Normal Data Copy..." << std::endl;
			return io::read(*src,s,n);
		}

	private:
		// Main header reading functions
		template<typename Source>
			bool ReadHeader(Source& src,Header &bitmapheader)
		{
			byte header[14];
			if(!io::read(src,reinterpret_cast<Source::char_type*>(header),14))
				return false;
			if (header[0] != 'B' || header[1] != 'M')
				return false;

			bitmapheader.file_size   = Utils::read32_le(header + 2);
			bitmapheader.data_offset = Utils::read32_le(header + 10);

			return true;
		};

		template<typename Source>
			bool ReadInfoHeader(Source& src,Header &bitmapheader)
		{
			const int HEADER_READ_SIZE = 24;

			// read the only part of the header we need
			byte header[HEADER_READ_SIZE];
			if (!io::read(src,reinterpret_cast<Source::char_type*>(header), HEADER_READ_SIZE)) 
			{
				return false;
			}

			int size = Utils::read32_le(header + 0);
			int width;
			int height;
			int planes;
			int bpp;
			int compression;
			int image_size;

			if (size < 40) {  // assume OS/2 bitmap
				if (size < 12) {
					return false;
				}

				bitmapheader.os2 = true;
				width  = Utils::read16_le(header + 4);
				height = Utils::read16_le(header + 6);
				planes = Utils::read16_le(header + 8);
				bpp    = Utils::read16_le(header + 10);
				compression = 0;
				image_size = 0;

			} else {

				bitmapheader.os2 = false;
				width       = Utils::read32_le(header + 4);
				height      = Utils::read32_le(header + 8);
				planes      = Utils::read16_le(header + 12);
				bpp         = Utils::read16_le(header + 14);
				compression = Utils::read32_le(header + 16);
				image_size  = Utils::read32_le(header + 20);

			}

			// sanity check the info header
			if (planes != 1) {
				return false;
			}

			// adjust image_size
			// (if compression == 0 or 3, manually calculate image size)
			int line_size = 0;
			if (compression == 0 || compression == 3) {
				line_size = (width * bpp + 7) / 8;
				line_size = (line_size + 3) / 4 * 4;  // 32-bit-aligned
				image_size = line_size * height;
			}

			bitmapheader.width       = width;
			bitmapheader.height      = height;
			bitmapheader.bpp         = bpp;
			bitmapheader.compression = compression;
			bitmapheader.pitch       = line_size;
			bitmapheader.image_size  = image_size;

			// jump forward (backward in the OS/2 case :) to the palette data
			io::seek(src,size - HEADER_READ_SIZE,std::ios_base::cur);
			return true;
		};
	private:
		LoaderImgData_t &imgdata_;
		bool headerdone_;
	};

	filterptr MakeBitmapFilter(LoaderImgData_t &imgdata)
	{
		return filterptr(new bitmapfilter(imgdata));
	}

	DECLARE_TEXTURE_LOADER(TYPE_BMP, "BMP",MakeBitmapFilter)
}
}