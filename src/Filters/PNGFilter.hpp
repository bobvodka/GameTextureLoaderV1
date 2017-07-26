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
				png_read_png(png_ptr_, info_ptr_, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_EXPAND, NULL);

				if (!png_get_rows(png_ptr_, info_ptr_)) {
					png_destroy_read_struct(&png_ptr_, &info_ptr_, NULL);
					return -1;
				}

				int bit_depth = png_get_bit_depth(png_ptr_, info_ptr_);
				int num_channels = png_get_channels(png_ptr_, info_ptr_);

				imgdata_.width = png_get_image_width(png_ptr_, info_ptr_);
				imgdata_.height = png_get_image_height(png_ptr_, info_ptr_);
				imgdata_.colourdepth = bit_depth * num_channels;
				imgdata_.depth = 0;
				imgdata_.numImages = 1;
				imgdata_.numMipMaps = 0;
				imgdata_.size = imgdata_.width * imgdata_.height * num_channels;
				rowlenght_ = imgdata_.width * num_channels;
				row_pointers_ = png_get_rows(png_ptr_, info_ptr_);

				if (bit_depth == 8 && num_channels == 4)
					imgdata_.format = FORMAT_RGBA;
				else  if (bit_depth == 8 && num_channels == 3)
					imgdata_.format = FORMAT_RGB;
				else
				{
					png_destroy_read_struct(&png_ptr_, &info_ptr_, NULL);					
					imgdata_.format = FORMAT_NONE;
					return -1;	// we dont deal with palletted images
				}

				headerdone_ = true;
			}


			// read upto 'n' bytes from each row (raw_pointers_[rowoffset_];) , stiching together rows into the buffer
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
		}

	protected:
	private:
		LoaderImgData_t &imgdata_;
		bool headerdone_;
		png_structp png_ptr_;
		png_infop info_ptr_;
		png_bytepp row_pointers_;

		int rowoffset_;
		int rowlenght_;
		int currentrow_;
	};

	filterptr MakePNGFilter(LoaderImgData_t &imgdata)
	{
		return filterptr(new PNGFilter(imgdata));
	}

	DECLARE_TEXTURE_LOADER(TYPE_PNG, "PNG",MakePNGFilter)
}
}