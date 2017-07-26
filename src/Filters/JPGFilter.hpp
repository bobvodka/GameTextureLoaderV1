// Stream filter object for decoding jpg images
// uses libjpg-6b
#include <boost/iostreams/categories.hpp> // input_filter_tag
#include <boost/iostreams/operations.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>

#include <boost/shared_ptr.hpp>

#include <gtl/FilterBase.hpp>
#include "../internals/utils.hpp"

#include <cstdio>  // needed by jpeglib.h
#include <csetjmp>

extern "C" {  // stupid JPEG library
		#include "jpeg-6b\jpeglib.h"
	}

namespace GameTextureLoader { 	namespace Filters {
	
	namespace io = boost::iostreams;
	
	namespace JPG 
	{

		struct InternalStruct 
		{
			struct {
				jpeg_error_mgr mgr;
				jmp_buf setjmp_buffer;
			} error_mgr;

			streambuf_t * file;
			byte *buffer;
		};

		static const int JPEG_BUFFER_SIZE = 4096;

		void init_source(j_decompress_ptr cinfo) {
			// no initialization required
		}

		//////////////////////////////////////////////////////////////////////////////

		boolean fill_input_buffer(j_decompress_ptr cinfo) {
			// more or less copied from jdatasrc.c

			JPG::InternalStruct* is = (JPG::InternalStruct*)(cinfo->client_data);

			std :: streamsize nbytes = io::read(*(is->file),reinterpret_cast<streambuf_t::char_type*>(is->buffer),JPEG_BUFFER_SIZE);
			if (nbytes == -1) 
			{
				/* Insert a fake EOI marker */
				is->buffer[0] = (JOCTET)0xFF;
				is->buffer[1] = (JOCTET)JPEG_EOI;
				nbytes = 2;
			}
//			std::cout << "Filling buffer with " << nbytes << " bytes\n";
			cinfo->src->bytes_in_buffer = nbytes;
			cinfo->src->next_input_byte = is->buffer;
			return TRUE;
		}

		//////////////////////////////////////////////////////////////////////////////

		void skip_input_data(j_decompress_ptr cinfo, long num_bytes) {
			if (num_bytes > 0) {
				while (num_bytes > (long)cinfo->src->bytes_in_buffer) {
					num_bytes -= (long)cinfo->src->bytes_in_buffer;
					fill_input_buffer(cinfo);
				}
				cinfo->src->next_input_byte += (size_t)num_bytes;
				cinfo->src->bytes_in_buffer -= (size_t)num_bytes;
			}
		}

		//////////////////////////////////////////////////////////////////////////////

		void term_source(j_decompress_ptr cinfo) {
			// nothing to do here...
		}

		//////////////////////////////////////////////////////////////////////////////

		void error_exit(j_common_ptr cinfo) {
			JPG::InternalStruct* is = (JPG::InternalStruct*)(cinfo->client_data);
			longjmp(is->error_mgr.setjmp_buffer, 1);
		}

		//////////////////////////////////////////////////////////////////////////////

		void emit_message(j_common_ptr /*cinfo*/, int /*msg_level*/) {
			// ignore error messages
		}
	}

	class JPGFilter : public FilterBase
	{
		
	public:
		typedef char                       char_type;
		typedef io::multichar_input_filter_tag  category;

		JPGFilter(LoaderImgData_t &imgdata) : headerdone(false),imgdata_(imgdata),buffer_(new byte[JPG::JPEG_BUFFER_SIZE]),mgr_(new jpeg_source_mgr),
			cinfo_(new jpeg_decompress_struct),linebuffer_(NULL),finished_(false),numBytesRead_(0),linebufferoffset_(0),is_(new JPG::InternalStruct)
		{
			mgr_->bytes_in_buffer = 0;
			mgr_->next_input_byte = NULL;
			mgr_->init_source       = JPG::init_source;
			mgr_->fill_input_buffer = JPG::fill_input_buffer;
			mgr_->skip_input_data   = JPG::skip_input_data;
			mgr_->resync_to_restart = jpeg_resync_to_restart;  // use default
			mgr_->term_source       = JPG::term_source;

			jpeg_create_decompress(cinfo_.get());

			is_->buffer = buffer_.get();

			cinfo_->client_data = is_.get();

			cinfo_->err = jpeg_std_error(&is_->error_mgr.mgr);
			is_->error_mgr.mgr.error_exit = JPG::error_exit;

			if (setjmp(is_->error_mgr.setjmp_buffer)) 
			{
				jpeg_destroy_decompress(cinfo_.get());
				return; // -1;	// throw?
			}

			cinfo_.get()->src = mgr_.get();

		};
		~JPGFilter()
		{
		};
		JPGFilter(JPGFilter const &lhs) : headerdone(lhs.headerdone), imgdata_(lhs.imgdata_),buffer_(lhs.buffer_),mgr_(lhs.mgr_),
			cinfo_(lhs.cinfo_),linebuffer_(lhs.linebuffer_),finished_(lhs.finished_),numBytesRead_(lhs.numBytesRead_),linebufferoffset_(lhs.linebufferoffset_)
			,is_(lhs.is_)
		{
		};

		virtual std::streamsize read(streambuf_t *src,char * s, std::streamsize n)
		{
			is_->file = src;

			if(!headerdone)
			{
				jpeg_read_header(cinfo_.get(), TRUE);
				jpeg_start_decompress(cinfo_.get());

				if (cinfo_->output_components != 1 && cinfo_->output_components != 3) {
					jpeg_finish_decompress(cinfo_.get());
					jpeg_destroy_decompress(cinfo_.get());
					return -1;
				}
				row_stride_ = cinfo_->output_width * cinfo_->output_components;
				linebuffer_ = (*cinfo_->mem->alloc_sarray)(
					(j_common_ptr)cinfo_.get(),
					JPOOL_IMAGE,
					row_stride_,
					1);
				
				imgdata_.width = cinfo_->output_width;
				imgdata_.height = cinfo_->output_height;
				imgdata_.numImages = 1;
				imgdata_.numMipMaps = 0;
				imgdata_.size = cinfo_->output_width * cinfo_->output_height * cinfo_->output_components;
				imgdata_.colourdepth = cinfo_->output_components * 8;
				if(cinfo_->output_components == 3)
					imgdata_.format = FORMAT_RGB;
				else
					imgdata_.format = FORMAT_NONE;	// fix this for grayscale images
				imgdata_.depth = 0;
				headerdone = true;
			}

			while(cinfo_->output_scanline < cinfo_->output_height)
			{
				if(linebufferoffset_ + n < row_stride_)
				{
					byte * bufferptr = reinterpret_cast<byte*>(*linebuffer_) + linebufferoffset_;
					std::copy(bufferptr,bufferptr+n,s);
					linebufferoffset_ += n;
					return n;
				}
				else
				{
					byte * bufferptr = reinterpret_cast<byte*>(*linebuffer_) + linebufferoffset_;
					int amountleft = row_stride_ - linebufferoffset_;
					if(amountleft > 0)
						std::copy(bufferptr,bufferptr+n,s);
					linebufferoffset_ = 0;
                    int num_rows = jpeg_read_scanlines(cinfo_.get(),linebuffer_,1);
					if(num_rows == 0)
						return -1;
					
					if(amountleft == n)
						return n;
					else
					{
						n -= amountleft;
						s += amountleft;
					}

				}
			}

			jpeg_finish_decompress(cinfo_.get());
			jpeg_destroy_decompress(cinfo_.get());
			
			return -1; 
		}
			
	protected:
	private:

		LoaderImgData_t &imgdata_;
		bool headerdone;
		boost::shared_array<byte> buffer_;
		boost::shared_ptr<jpeg_source_mgr> mgr_;
		boost::shared_ptr<jpeg_decompress_struct> cinfo_;
		JSAMPARRAY linebuffer_;

		bool finished_;
		std::streamsize numBytesRead_;

		int linebufferoffset_;
		int row_stride_;

		boost::shared_ptr<JPG::InternalStruct> is_;
	};

	filterptr MakeJPGFilter(LoaderImgData_t &imgdata)
	{
		return filterptr(new JPGFilter(imgdata));
	}

	DECLARE_TEXTURE_LOADER(TYPE_JPG, "JPG",MakeJPGFilter)
	}
}