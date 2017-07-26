// Bitmapfilter.hpp
// Defines an input filter for a bitmap file type
//
// Uses boost1.33's iostreams library

#include <boost/iostreams/categories.hpp> // input_filter_tag
#include <boost/iostreams/operations.hpp>

#include <boost/scoped_array.hpp>

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
			int palette_size;	//size of the palette in bytes
		};


	public:
		typedef char                       char_type;
		typedef io::multichar_input_filter_tag  category;

		bitmapfilter(LoaderImgData_t &imgdata) : headerdone_(false),imgdata_(imgdata),rowoffset_(0)
		{
		}
		~bitmapfilter()
		{
		}
		bitmapfilter(bitmapfilter const &lhs) : headerdone_(lhs.headerdone_), imgdata_(lhs.imgdata_),rowoffset_(lhs.rowoffset_)
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
				if(!ReadInfoHeader(*src,bitmapheader))
					return -1;

				// Extract information from bitmap header to imgdata struct
				imgdata_.height = bitmapheader.height;
				imgdata_.width = bitmapheader.width;
				imgdata_.size = bitmapheader.width * bitmapheader.height * 3;	//everything is converted to BGR
				imgdata_.colourdepth = 24;
				imgdata_.depth = 0;	// not a 3D image
				imgdata_.numImages = 1;	// Only one image stored
				imgdata_.numMipMaps = 1; // no mipmaps
				
				SetFlips(imgdata_,true,false);
				imgdata_.yflip = true;				

				// Create a buffer to hold the bmp data
				boost::scoped_array<byte> tmpbuffer(new byte[bitmapheader.image_size]);						// temp buffer
				databuffer_.reset(new byte[imgdata_.size]);	
				// read whole file in
				if (bitmapheader.image_size!=io::read(*src,reinterpret_cast<streambuf_t::char_type*>(tmpbuffer.get()),bitmapheader.image_size))
					return -1;				

				//apply appropriate conversion
				switch (bitmapheader.bpp)
				{
				case 32:
					Bpp32toRGB( tmpbuffer.get(), databuffer_.get(), bitmapheader);
					break;
				case 24:
					Bpp24toRGB( tmpbuffer.get(), databuffer_.get(), bitmapheader);
					break;
				case 16:
					Bpp16toRGB( tmpbuffer.get(), databuffer_.get(), bitmapheader);
					break;
				case 8:
					Bpp8toRGB( tmpbuffer.get(), databuffer_.get(), bitmapheader);
					break;
				case 4:
					Bpp4toRGB( tmpbuffer.get(), databuffer_.get(), bitmapheader);
					break;
				case 1:
					Bpp1toRGB( tmpbuffer.get(), databuffer_.get(), bitmapheader);
					break;
				default:
					return -1;
					break;
				};
				
				imgdata_.format = FORMAT_BGR;	//even the os2 v1 bitmaps seem to be BGR
				headerdone_ = true;				
			}
			
			int copyBytes=n;
			if (copyBytes > imgdata_.size- rowoffset_)
				copyBytes=imgdata_.size- rowoffset_;

			if (0==copyBytes)
				return -1;

			memcpy( s, databuffer_.get()+rowoffset_, copyBytes);
			rowoffset_+=copyBytes;

			return copyBytes;
		}
	private:
		void Bpp32toRGB( byte * src, byte * dst, Header & header)
		{
			for (int height=0; height<header.height; ++height)
			{
				byte * srcRow=src + height*header.pitch;
				byte * dstRow=dst+ height * header.width * 3;
				for (int width=0; width<header.width; ++width)
				{
					dstRow[0]=srcRow[0];
					dstRow[1]=srcRow[1];
					dstRow[2]=srcRow[2];
					srcRow+=4;
					dstRow+=3;
				}
			}
		}

		void Bpp24toRGB( byte * src, byte * dst, Header & header)
		{
			for (int height=0; height<header.height; ++height)
			{
				byte * srcRow=src + height*header.pitch;
				byte * dstRow=dst+ height * header.width * 3;
				for (int width=0; width<header.width; ++width)
				{
					dstRow[0]=srcRow[0];
					dstRow[1]=srcRow[1];
					dstRow[2]=srcRow[2];
					srcRow+=3;
					dstRow+=3;
				}
			}
		}

		void Bpp16toRGB( byte * src, byte * dst, Header & header)
		{	
			for (int height=0; height<header.height; ++height)
			{
				byte * srcRow=src + height*header.pitch;
				byte * dstRow=dst+ height * header.width * 3;
				for (int width=0; width<header.width; ++width)
				{
					const byte blue=srcRow[0]&0x1F;
					const byte green=(srcRow[0]>>5) + ((srcRow[1]&0x03)<<3);
					const byte red=(srcRow[1]>>2)&0x1F;

					dstRow[0]=(byte)((double)blue*(255.0/31.0) );
					dstRow[1]=(byte)((double)green*(255.0/31.0) );
					dstRow[2]=(byte)((double)red*(255.0/31.0) );
					srcRow+=2;
					dstRow+=3;
				}
			}
		}

		void Bpp8toRGB( byte * src, byte * dst, Header & header)
		{
			byte * color=NULL;
			for (int height=0; height<header.height; ++height)
			{
				byte * srcRow=src + height*header.pitch;
				byte * dstRow=dst+ height * header.width * 3;
				for (int width=0; width<header.width; ++width)
				{
					color= palette_ + 4*(int)srcRow[width];
					dstRow[0]=color[0];
					dstRow[1]=color[1];
					dstRow[2]=color[2];
					dstRow+=3;
				}
			}
		}

		void Bpp4toRGB( byte * src, byte * dst, Header & header)
		{
			byte * color=NULL;
			for (int height=0; height<header.height; ++height)
			{
				byte * srcRow=src + height*header.pitch;
				byte * dstRow=dst+ height * header.width * 3;
				for (int width=0; width<header.width; ++width)
				{
					const int palettePos=4*((srcRow[width/2]>>((1-width%2)*4))&0x0F);
					color= palette_+palettePos;
					dstRow[0]=color[0];
					dstRow[1]=color[1];
					dstRow[2]=color[2];
					dstRow+=3;
				}
			}
		}

		void Bpp1toRGB( byte * src, byte * dst, Header & header)
		{
			if (0==header.palette_size)
			{	//fake a palette
				palette_[0]=0;
				palette_[1]=0;
				palette_[2]=0;
				palette_[3]=0;
				palette_[4]=255;
				palette_[5]=255;
				palette_[6]=255;
				palette_[7]=255;
			}			

			byte * color=NULL;
			for (int height=0; height<header.height; ++height)
			{
				byte * srcRow=src + height*header.pitch;
				byte * dstRow=dst+ height * header.width * 3;
				for (int width=0; width<header.width; ++width)
				{
					color= palette_+4* (int)((srcRow[width/8]>>(7-width%8))&1);

					dstRow[0]=color[0];
					dstRow[1]=color[1];
					dstRow[2]=color[2];
					if (height==300)
					{
						dstRow[0]=0;
						dstRow[1]=255;
						dstRow[2]=0;
					}
					dstRow+=3;
				}
			}
		}

		// Main header reading functions
		template<typename Source>
			bool ReadHeader(Source& src,Header &bitmapheader)
		{
			byte header[14];
			if(!io::read(src,reinterpret_cast<typename Source::char_type*>(header),14))
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
			byte header[HEADER_READ_SIZE];

			//first read the size of the header
			if (!io::read(src,reinterpret_cast<typename Source::char_type*>(header), 4)) 
			{
				return false;
			}
			int size = Utils::read32_le(header);
			int readHeaderBytes=4;	//4 bytes already read

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

				//read the 8 bytes we need				
				if (!io::read(src,reinterpret_cast<typename Source::char_type*>(header+4), 8)) 
				{
					return false;
				}
				readHeaderBytes+=8;

				bitmapheader.os2 = true;
				width  = Utils::read16_le(header + 4);
				height = Utils::read16_le(header + 6);
				planes = Utils::read16_le(header + 8);
				bpp    = Utils::read16_le(header + 10);
				compression = 0;
				image_size = 0;

			} else {
				//read the 20 bytes we need
				if (20!=io::read(src,reinterpret_cast<typename Source::char_type*>(header+4), 20)) 
				{
					return false;
				}
				readHeaderBytes+=20;

				bitmapheader.os2 = false;
				width       = Utils::read32_le(header + 4);
				height      = Utils::read32_le(header + 8);
				planes      = Utils::read16_le(header + 12);
				bpp         = Utils::read16_le(header + 14);
				compression = Utils::read32_le(header + 16);
				image_size  = Utils::read32_le(header + 20);

			}

			// sanity check the info header
			if (planes != 1 ) {
				return false;
			}

			if (!(1==bpp || 4==bpp || 8==bpp || 16==bpp || 24==bpp || 32==bpp))
			{
				return false;
			}

			int line_size = 0;

			if (0!=compression)
				return false;	//just plain RGB data supported

			line_size = ((width * bpp + 31)/32)*4;	//32 bit aligned
			image_size = line_size * height;

			bitmapheader.width       = width;
			bitmapheader.height      = height;
			bitmapheader.bpp         = bpp;
			bitmapheader.compression = compression;
			bitmapheader.pitch       = line_size;
			bitmapheader.image_size  = image_size;

			// skip the rest of the header to get to the palette data (not using seeking, to be able to use non seekable sources in the future)
			//io::seek(src,size - HEADER_READ_SIZE,std::ios_base::cur);
			while (readHeaderBytes<size)
			{
				int toRead= size-readHeaderBytes;
				if (toRead>HEADER_READ_SIZE)
					toRead=HEADER_READ_SIZE;

				if (toRead!=io::read(src,reinterpret_cast<typename Source::char_type*>(header), toRead) )
					return false;
				readHeaderBytes+=toRead;
			}

			//calculate palette size			
			bitmapheader.palette_size=bitmapheader.data_offset-size-14;

			//read the palette
			if (bitmapheader.palette_size>0 && bitmapheader.palette_size<=1024)
			{
				if (!io::read(src,reinterpret_cast<typename Source::char_type*>(palette_), bitmapheader.palette_size)) 
				{
					return false;
				}
			}

			//convert to BGRX palette if necessary
			if (bitmapheader.os2)
			{
				byte tmp[1024];
				memcpy(tmp, palette_, 1024);
				for (int i=0; i<256; ++i)
				{
					palette_[i*4+0]=tmp[i*3+0];
					palette_[i*4+1]=tmp[i*3+1];
					palette_[i*4+2]=tmp[i*3+2];
					palette_[i*4+3]=0;
				}
			}

			return true;
		};
	private:
		bool headerdone_;
		LoaderImgData_t &imgdata_;
		boost::scoped_array<byte> databuffer_;
		int rowoffset_;
		byte palette_[1024];
	};

	filterptr MakeBitmapFilter(LoaderImgData_t &imgdata)
	{
		return filterptr(new bitmapfilter(imgdata));
	}

	DECLARE_TEXTURE_LOADER(TYPE_BMP, "BMP",MakeBitmapFilter)
}
}
