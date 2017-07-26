// PNG Decoding Filter

#include "lpng128/png.h"

#include <boost/iostreams/categories.hpp> // input_filter_tag
#include <boost/iostreams/operations.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>

#include <boost/shared_ptr.hpp>

#include <gtl/FilterBase.hpp>
#include "../internals/utils.hpp"

namespace GameTextureLoader { namespace Filters {
	namespace io = boost::iostreams;

	namespace PNG
	{
		// PNG Access functions
		//////////////////////////////////////////////////////////////////////////////

		void read_function(png_structp png_ptr,
			png_bytep data,
			png_size_t length) 
		{
				streambuf_t* file = reinterpret_cast<streambuf_t*>(png_get_io_ptr(png_ptr));

				if(io::read(*file,reinterpret_cast</*io::filtering_istream*/streambuf_t::char_type*>(data),std::streamsize(length)) != std::streamsize(length))
					png_error(png_ptr, "Read error");
			}	

			//////////////////////////////////////////////////////////////////////////////

			void warning_function(png_structp png_ptr, png_const_charp error) {
				// no warnings
			}

			//////////////////////////////////////////////////////////////////////////////

			void error_function(png_structp png_ptr, png_const_charp warning) {
				// copied from libpng's pngerror.cpp, but without the fprintf
				jmp_buf jmpbuf;
				memcpy(jmpbuf, png_ptr->jmpbuf, sizeof(jmp_buf));
				longjmp(jmpbuf, 1);
			}

	}

	class PNGFilter : public FilterBase
	{
	public:
		typedef char                       char_type;
		typedef io::multichar_input_filter_tag  category;
		
		PNGFilter(LoaderImgData_t &imgdata) : imgdata_(imgdata),headerdone_(false),png_ptr_(NULL),info_ptr_(NULL),row_pointers_(NULL),rowoffset_(0),currentrow_(0),rowlenght_(0)
		{
		}

		PNGFilter(PNGFilter const& lhs) : imgdata_(lhs.imgdata_), headerdone_(lhs.headerdone_),png_ptr_(lhs.png_ptr_),info_ptr_(lhs.info_ptr_)
			,row_pointers_(lhs.row_pointers_),rowoffset_(lhs.rowoffset_),currentrow_(lhs.currentrow_),rowlenght_(lhs.rowlenght_)
		{
		}
		~PNGFilter()
		{
		}

		virtual std::streamsize read(streambuf_t *src,char * s, std::streamsize n)
		{
			if(!headerdone_)
			{
				byte sig[8];
				io::read(*src,reinterpret_cast</*Source*/streambuf_t::char_type*>(sig),8);
				if (png_sig_cmp(sig, 0, 8))
					return -1;

				png_ptr_ = png_create_read_struct(
					PNG_LIBPNG_VER_STRING,
					NULL, NULL, NULL);
				if (!png_ptr_) {
					return -1;
				}

				info_ptr_ = png_create_info_struct(png_ptr_);
				if (!info_ptr_) {
					png_destroy_read_struct(&png_ptr_, NULL, NULL);
					return -1;
				}

				// the PNG error function calls longjmp(png_ptr->jmpbuf)
				if (setjmp(png_jmpbuf(png_ptr_))) {
					png_destroy_read_struct(&png_ptr_, &info_ptr_, NULL);
					return -1;
				}

				// set the error function
				png_set_error_fn(png_ptr_, 0, PNG::error_function, PNG::warning_function);

				// read the image
				png_set_read_fn(png_ptr_, src, PNG::read_function);
				png_set_sig_bytes(png_ptr_, 8);  // we already read 8 bytes for the sig
				// always give us 8-bit samples (strip 16-bit and expand <8-bit)
				// The below allows us to convert paletted images to RGB(A) images BUT it costs us 1 and 2 channel grey scale images
				//png_read_png(png_ptr_, info_ptr_, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_EXPAND, NULL);
				
				// So instead we are going to deal with things on our own
				// First we need to read in the header
				png_read_info(png_ptr_, info_ptr_);

				// Next we obtain the information we require
				int bit_depth;
				int colourtype;
				png_uint_32 width, height;
				if(!png_get_IHDR(png_ptr_,info_ptr_,&width, &height,&bit_depth, &colourtype, NULL, NULL, NULL))
				{
					png_destroy_read_struct(&png_ptr_, &info_ptr_, NULL);
					return -1;
				}
				
				// all images are always expanded to 8bit per pixel
				if(bit_depth < 8)
				{
					png_set_packing(png_ptr_);
					bit_depth = 8;
				}

				if(PNG_COLOR_TYPE_PALETTE == colourtype)
				{
					png_set_packing(png_ptr_);		// expand 1,2 & 4bit depths to 8bit
					png_set_expand(png_ptr_);	// convert paletted colours to full RGB colours
					if(png_get_valid(png_ptr_, info_ptr_, PNG_INFO_tRNS))
					{
						png_set_tRNS_to_alpha(png_ptr_);
						png_color_16 *image_background;

						// If the file has a valid 'background' then render pixels to it
						if (png_get_bKGD(png_ptr_, info_ptr_, &image_background))
							png_set_background(png_ptr_, image_background,PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);

						imgdata_.format = FORMAT_RGBA;
					}
					else
						imgdata_.format = FORMAT_RGB;
				}
				else
				{
					switch(bit_depth)
					{
					case 8:
						switch(colourtype)
						{
						case PNG_COLOR_TYPE_RGB_ALPHA: imgdata_.format = FORMAT_RGBA; break;
						case PNG_COLOR_TYPE_RGB: imgdata_.format = FORMAT_RGB; break;
						case PNG_COLOR_TYPE_GRAY_ALPHA: imgdata_.format = FORMAT_A8L8; break;
						case PNG_COLOR_TYPE_GRAY: imgdata_.format = FORMAT_L8; break;
						default: imgdata_.format = FORMAT_NONE; break;
						}
						break;
					case 16:
						switch(colourtype)
						{
						case PNG_COLOR_TYPE_RGB_ALPHA: imgdata_.format = FORMAT_RGBA16; break;
						case PNG_COLOR_TYPE_RGB: imgdata_.format = FORMAT_RGB16; break;
						case PNG_COLOR_TYPE_GRAY_ALPHA: imgdata_.format = FORMAT_A16L16; break;
						case PNG_COLOR_TYPE_GRAY: imgdata_.format = FORMAT_L16; break;
						default: imgdata_.format = FORMAT_NONE; break;
						}
						break;
					default:
						imgdata_.format = FORMAT_NONE;
					}
				}
		
				// update the transformations
				png_read_update_info(png_ptr_, info_ptr_);

				// setup imgdata structure
				int num_channels = png_get_channels(png_ptr_, info_ptr_);
				imgdata_.width = width;
				imgdata_.height = height;
				imgdata_.colourdepth = bit_depth * num_channels;
				imgdata_.depth = 0;
				imgdata_.numImages = 1;
				imgdata_.numMipMaps = 1;
				rowlenght_ = png_get_rowbytes(png_ptr_, info_ptr_);
				imgdata_.size = imgdata_.width * imgdata_.height * rowlenght_;
				

				// reserve space for the row
				row_pointers_ = new png_bytep[imgdata_.height];
				for (int row = 0; row < imgdata_.height; row++)
				{
					row_pointers_[row] = reinterpret_cast<png_bytep>(png_malloc(png_ptr_, rowlenght_));
				}
				png_read_image(png_ptr_, row_pointers_);	// finally load in the image

				SetFlips(imgdata_,false,false);
				headerdone_ = true;
				rowoffset_=0;
			}

			if (!png_ptr_)
				return -1;

			int toread=n;
			while (toread>0)
			{
				if (rowlenght_==rowoffset_)
				{
					rowoffset_=0;
					++currentrow_;
					
					if (currentrow_ >= imgdata_.height)
					{
						png_destroy_read_struct(&png_ptr_, &info_ptr_, NULL);
						delete [] row_pointers_;
						break;
					}
				}
				
				byte * bufferptr = reinterpret_cast<byte*>(row_pointers_[currentrow_]) + rowoffset_;
				int tocopy=rowlenght_-rowoffset_;
				if (tocopy>toread)
					tocopy=toread;

				memcpy(s, bufferptr, tocopy);
				s+=tocopy;
				toread-=tocopy;
				rowoffset_+=tocopy;
			}

			return n-toread;


/*
			// read up to 'n' bytes from each row (raw_pointers_[rowoffset_];) , stitching together rows into the buffer
			// as required.
			while (currentrow_ < imgdata_.height)
			{
				if(rowoffset_ + n < rowlenght_)
				{
					byte * sourceptr = reinterpret_cast<byte*>(row_pointers_[currentrow_]) + rowoffset_;
					std::copy(sourceptr , sourceptr  + n, s);
					rowoffset_ += n;
					return n;
				}
				else
				{
					int amountleft = rowlenght_ - rowoffset_;
					if(amountleft > 0)
					{
						byte * sourceptr = reinterpret_cast<byte*>(row_pointers_[currentrow_]) + rowoffset_;
						std::copy(sourceptr, sourceptr + amountleft, s);
					}
						
					rowoffset_ = 0;
					++currentrow_;
					if(amountleft == n)
						return n;
					else
					{
						n -= amountleft;
						s += amountleft;
					}
				}
			}
			

			return -1;
*/
		}

	protected:
	private:
		LoaderImgData_t &imgdata_;
		bool headerdone_;
		png_structp png_ptr_;
		png_infop info_ptr_;
		png_bytep *row_pointers_;

		int rowoffset_;
		int currentrow_;
		int rowlenght_;
	};

	filterptr MakePNGFilter(LoaderImgData_t &imgdata)
	{
		return filterptr(new PNGFilter(imgdata));
	}

	DECLARE_TEXTURE_LOADER(TYPE_PNG, "PNG",MakePNGFilter)
}
}
