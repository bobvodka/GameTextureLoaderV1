// Source file to get the required filters into play


#include <gtl/GameTextureLoader.hpp>
#include "../internals/internal.hpp"
#include "FilterRegister.hpp"

#if defined(GTL_BITMAP_FILTER) && !defined(GTL_NO_BITMAP_FILTER)
	#include "BitmapFilter.hpp"
#endif
#if defined(GTL_JPG_FILTER) && !defined(GTL_NO_JPG_FILTER)
	#include "JPGFilter.hpp"
#endif
#if defined(GTL_PNG_FILTER) && !defined(GTL_NO_PNG_FILTER)
	#include "PNGFilter.hpp"
#endif
#if defined(GTL_TGA_FILTER) && !defined(GTL_NO_TGA_FILTER)
	#include "TGAFilter.hpp"
#endif
#if defined(GTL_DDS_FILTER) && !defined(GTL_NO_DDS_FILTER)
	#include "DDSFilter.hpp"
#endif
