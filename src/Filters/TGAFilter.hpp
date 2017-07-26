// TGA decoding filter
// Decodes both raw and RLE TGA files, however DOESNT deal with paletted files

#include <boost/iostreams/categories.hpp> // input_filter_tag
#include <boost/iostreams/operations.hpp>

#include <boost/shared_array.hpp>
#include <boost/scoped_array.hpp>
#include <vector>

#include <gtl/FilterBase.hpp>
#include "../internals/utils.hpp"

#include <fstream>

namespace GameTextureLoader { namespace Filters
{
	
	namespace io = boost::iostreams;

	class TGAFilter : public FilterBase
	{
	public:
		typedef char                       char_type;
		typedef io::multichar_input_filter_tag  category;

		TGAFilter(LoaderImgData_t &imgdata) : imgdata_(imgdata),headerdone_(false),RLE_(false),currentrow_(0),rowoffset_(0),rowlenght_(0)
		{
		}
		TGAFilter(TGAFilter const &lhs) : imgdata_(lhs.imgdata_),headerdone_(lhs.headerdone_),databuffer_(lhs.databuffer_),RLE_(lhs.RLE_)
			,currentrow_(lhs.currentrow_),rowoffset_(lhs.rowoffset_),rowlenght_(lhs.rowlenght_)
		{
		}
		~TGAFilter()
		{
		}

		virtual std::streamsize read(streambuf_t *src,char * s, std::streamsize n)
		{
			if(!headerdone_)
			{
				if(!readHeader(*src))
					return -1;
				headerdone_ = true;
				databuffer_.reset(new byte[imgdata_.size]);
				if(RLE_)
				{
					if(!readRLEData(*src))
						return -1;
				}
				else
				{
					io::read(*src, reinterpret_cast<streambuf_t::char_type*>(databuffer_.get()),imgdata_.size);
				}

				// OK, so the data has a blue tint to it, so that means we need to swap B and R around
				if(imgdata_.format == FORMAT_RGBA)
				{
					for (int i = 0, size = imgdata_.size; i < size; i+= 4)
					{
						std::swap(databuffer_[i],databuffer_[i+2]);
					}
				}
			}
			
			while (rowoffset_ < imgdata_.size)
			{
				if(rowoffset_ + n > imgdata_.size)
				{
					int amountleft = imgdata_.size - rowoffset_;
					byte * sourceptr = reinterpret_cast<byte*>(&databuffer_[rowoffset_]);
					std::copy(sourceptr, sourceptr + amountleft ,s);
					rowoffset_+=amountleft;
					return amountleft;
				}
				else
				{
					byte * sourceptr = reinterpret_cast<byte*>(&databuffer_[rowoffset_]);
					std::copy(sourceptr, sourceptr+n,s);
					rowoffset_ += n;
					return n;
				}
			}

			return -1;

		}

	protected:
	private:

		template<typename Source>
			bool readHeader(Source& src)
		{
			const int TGAHEADERSIZE = 18;	// TGA header is 18 bytes
			byte header[TGAHEADERSIZE];
			if(io::read(src,reinterpret_cast<typename Source::char_type*>(header),TGAHEADERSIZE) != TGAHEADERSIZE)
				return false;

			// decode header
			int id_length        = header[0];
			int cm_type          = header[1];
			if(cm_type != 0)
				return false;	// palletted images not supported

			int image_type       = header[2];

			if(image_type == 0 || image_type == 1 || image_type == 9)
				return false;	// no image data we can deal with

			if(image_type == 10)
				RLE_ = true;

			if(image_type == 11 || image_type == 32 || image_type == 33)
				return false;	// currently no support for properly compressed images, need to work on this.

			//int cm_first         = read16_le(header + 3);
//			int cm_length        = Utils::read16_le(header + 5);
//			int cm_entry_size    = header[7];  // in bits
			//int x_origin         = read16_le(header + 8);
			//int y_origin         = read16_le(header + 10);
			int width            = Utils::read16_le(header + 12);
			int height           = Utils::read16_le(header + 14);
			int pixel_depth      = header[16];
			int image_descriptor = header[17];

			bool mirrored = (image_descriptor & (1 << 4)) != 0;  // left-to-right?
			bool flipped  = (image_descriptor & (1 << 5)) == 0;  // bottom-to-top?

			imgdata_.width = width;
			imgdata_.height = height;
			imgdata_.colourdepth = pixel_depth;
			imgdata_.size = width * height * (pixel_depth/8);
			imgdata_.depth = 0;
			imgdata_.numMipMaps = 1;
			rowlenght_ = width * (pixel_depth/8);
			SetFlips(imgdata_,flipped,mirrored);
			//imgdata_.xflip = mirrored;
			//imgdata_.yflip = flipped;
			
			
			switch(pixel_depth)
			{
			case 32:
				imgdata_.format = /*FORMAT_BGRA*/ FORMAT_RGBA;
				imgdata_.numImages = 1;
				break;
			case 24:
				imgdata_.format = FORMAT_BGR;
				imgdata_.numImages = 1;
				break;
//			case 16:
//				imgdata_.format = FORMAT_BGR;
//				imgdata_.numImages = 1;
//				break;
			default:
				imgdata_.format = FORMAT_NONE;
				imgdata_.numImages = 0;
				return false;
			}

			// read the id stuff, not using seeking to be able to use non seekable sources in the future
			//io::seek(src,id_length,std::ios_base::cur);
			while (id_length)
			{
				int toRead= id_length>TGAHEADERSIZE ? TGAHEADERSIZE : id_length;
				if (toRead!=io::read(src,reinterpret_cast<typename Source::char_type*>(header), toRead) )
					return false;
				id_length-=toRead;
			}
			return true;
		}

		template<typename Source>
			bool readRLEData(Source& src)
		{
			int pixelsize = imgdata_.colourdepth/8;

			signed int currentbyte = 0;
			byte chunkheader = 0;

			while(currentbyte < imgdata_.size )
			{
				if(io::read(src,reinterpret_cast<typename Source::char_type*>(&chunkheader),1) == -1)
					return false;

				if(chunkheader < 128)
				{
					chunkheader++;
					if(io::read(src,reinterpret_cast<typename Source::char_type*>(&databuffer_[currentbyte]),chunkheader * pixelsize) == -1)
						return false;

					currentbyte += chunkheader * pixelsize;
				}
				else
				{
					chunkheader -= 127;
					unsigned int dataoffset = currentbyte;
					if(io::read(src,reinterpret_cast<typename Source::char_type*>(&databuffer_[currentbyte]),pixelsize) == -1)
						return false;

					currentbyte += pixelsize;

					for(int i = 1; i < chunkheader; ++i)
					{
						for(int elementCounter = 0; elementCounter < pixelsize; ++elementCounter)
							databuffer_[currentbyte + elementCounter] = databuffer_[dataoffset + elementCounter];

						currentbyte += pixelsize;
					}
				}


			}
			return true;
		}
	
		LoaderImgData_t &imgdata_;
		bool headerdone_;
		boost::shared_array<byte> databuffer_;
		bool RLE_;
		int currentrow_;
		int rowoffset_;
		int rowlenght_;
		
		
	};

	filterptr MakeTGAFilter(LoaderImgData_t &imgdata)
	{
		return filterptr(new TGAFilter(imgdata));
	}

	DECLARE_TEXTURE_LOADER(TYPE_TGA, "TGA",MakeTGAFilter)
}
}
