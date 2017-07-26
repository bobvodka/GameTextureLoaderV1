// Main GameTextureLoader source file
// Pulls it all together
#pragma warning(push)	//disable some warnings for this cpp FileDevice
#pragma warning(disable : 4996)	// disable complains about not using the secure CRT in 
								// boost and the string header

#include <string>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/detail/ios.hpp>
#pragma warning(pop)

#include <algorithm>
#include <iostream>

#include <gtl/GameTextureLoader.hpp>
#include <gtl/GTLUserFunctions.hpp>
#include "internals/internal.hpp"


#include "Filters/FilterRegister.hpp"
#include "Filters/FilterProxy.hpp"
#include "Filters/filters.hpp"

#include "Devices/DeviceBase.hpp"
#include <gtl/DeviceProxy.hpp>
#include "Devices/FileDevice.hpp"
#include "Devices/CustomReadDevice.hpp"

#include "internals/utils.hpp"

#ifdef GTL_PHYSFS_SUPPORT
#include "Devices/DevicePhysFS.hpp"
#endif


namespace GameTextureLoader
{
	namespace io = boost::iostreams;

	//////////////////////////////////////////////////////////////////////////
	//	GTL Image class functions
	//////////////////////////////////////////////////////////////////////////

	unsigned char * Image::getDataPtr(int imgnumber, int mipmaplvl) {return getDataPtrImpl(imgnumber,mipmaplvl);};
	int Image::getWidth(int mipmaplvl){return getWidthImpl(mipmaplvl);};
	int Image::getHeight(int mipmaplvl){return getHeightImpl(mipmaplvl);};
	int Image::getDepth(int mipmaplvl){return getWidthImpl(mipmaplvl);};
	int Image::getSize(int mipmaplvl){return getSizeImpl(mipmaplvl);};
	int Image::getNumMipMaps() { return getNumMipMapsImpl();};
	int Image::getNumImages(){return getNumImagesImpl();};
	int Image::getColourDepth(){return getColourDepthImpl();};
	ImgFormat Image::getFormat(){return getFormatImpl();};

	
	// Does the image loading
	void LoadData(io::filtering_istream &texturein, LoaderImgData_t &headerdata, ImageData_t &realdata)
	{
		io::filtering_istream::char_type databuffer[128];
		int dataoffset = 0;
		std::streamsize s;
		int height = 0;
		while(( s = io::read(texturein,databuffer,128)) != -1)
		{
			// On the first run of a filter it needs to fill out information structure so we can
			// reserve the right amount of memory.
			if(!realdata)
			{
				realdata.reset(new unsigned char[headerdata.size]);
				memset(realdata.get(),0,headerdata.size);
			}

			if(s > 0 && dataoffset < headerdata.size)
				// copy data from the read buffer into the final buffer
				std::copy(databuffer,databuffer + s, realdata.get() + dataoffset);
			dataoffset += s;
		}

		if(headerdata.xflip)
		{
			// fix mirrored data
			FlipX(realdata,headerdata);
		}
		if(headerdata.yflip)
		{
			// invert image
			FlipY(realdata,headerdata);
		}
	}

	Image* LoadData(io::filtering_istream &texturein, LoaderImgData_t &headerdata)
	{
		ImageData_t realdata;
		LoadData(texturein,headerdata,realdata);
		return new ImageImpl(headerdata.height,headerdata.width,headerdata.depth,headerdata.colourdepth,headerdata.format,headerdata.numImages,headerdata.numMipMaps,realdata);
	}

	Image* LoadData(io::filtering_istream &texturein, LoaderImgData_t &headerdata, LoadCallBack_t userfunc)
	{
		ImageData_t realdata;
		LoadData(texturein,headerdata,realdata);
		Imageinfo info;
		CreateImageInfo(headerdata,info);
		userfunc(realdata,info);
		return new ImageImpl(info.height,info.width,info.depth,info.colourdepth,info.format,info.numImages,info.numMipMaps,realdata);
	}

	template<typename T>
	FilterCreator_t GetTextureFilterCreator(T const &val)
	{
		Filters::FilterRegister reg;
		FilterCreator_t creator = reg.getCreator(val);
		return creator;
	}

	template<typename T>
	Image* LoadTexture(Devices::DeviceBase &dev, T const &ext)
	{
		LoaderImgData_t headerdata;
		io::filtering_istream texturein;
		FilterCreator_t creator = GetTextureFilterCreator(ext);
		Filters::filterptr filter = creator(headerdata);
		texturein.push(Filters::FilterProxy(filter));
		texturein.push(Devices::DeviceProxy(&dev));
		return LoadData(texturein,headerdata);
	}

	template<typename T>
	Image* LoadTexture(Devices::DeviceBase &dev, T const &ext, LoadCallBack_t userfunc)
	{
		LoaderImgData_t headerdata;
		io::filtering_istream texturein;
		FilterCreator_t creator = GetTextureFilterCreator(ext);
		Filters::filterptr filter = creator(headerdata);
		texturein.push(Filters::FilterProxy(filter));
		texturein.push(Devices::DeviceProxy(&dev));
		return LoadData(texturein,headerdata,userfunc);
	}

	Image* LoadTexture(std::string const &filename)
	{
		std::string ext = ExtractFileExtension(filename);
	
		try
		{
			Devices::FileDevice dev(filename);
			return LoadTexture(dev,ext);
		}
		catch (BOOST_IOSTREAMS_FAILURE &)
		{
			
		}
		return new ImageImpl();
		
	}

	Image* LoadTexture(std::string const &filename, LoadCallBack_t userfunc)
	{
		std::string ext = ExtractFileExtension(filename);

		try
		{
			Devices::FileDevice dev(filename);
			return LoadTexture(dev,ext,userfunc);
		}
		catch (BOOST_IOSTREAMS_FAILURE &)
		{

		}
		return new ImageImpl();

	}


	Image* LoadTexture(std::string const &filename, FileTypes val)
	{
		try
		{
			Devices::FileDevice dev(filename);
			return LoadTexture(dev,val);
		}
		catch (BOOST_IOSTREAMS_FAILURE &) 
		{
		}
		return new ImageImpl();
	}

	Image* LoadTexture(std::string const &filename, FileTypes val, LoadCallBack_t userfunc)
	{
		try
		{
			Devices::FileDevice dev(filename);
			return LoadTexture(dev,val,userfunc);
		}
		catch (BOOST_IOSTREAMS_FAILURE &) 
		{
		}
		return new ImageImpl();
	}

#ifdef GTL_PHYSFS_SUPPORT
	Image* LoadTexture(PHYSFS_File* file, FileTypes val)
	{
		Devices::PhysFSDevice dev(file);
		return LoadTexture(dev,val);
	}

	Image* LoadTexture(PHYSFS_File* file, FileTypes val, LoadCallBack_t userfunc)
	{
		Devices::PhysFSDevice dev(file);
		return LoadTexture(dev,val,userfunc);
	}

	Image* LoadTexture(PHYSFS_File *file, std::string const &filename)
	{
		std::string ext = ExtractFileExtension(filename);
		Devices::PhysFSDevice dev(file);
		return LoadTexture(dev,ext);
	}

	Image* LoadTexture(PHYSFS_File *file, std::string const &filename, LoadCallBack_t userfunc)
	{
		std::string ext = ExtractFileExtension(filename);
		Devices::PhysFSDevice dev(file);
		return LoadTexture(dev,ext,userfunc);
	}
#endif

	Image* LoadTexture(ReadFunc_t reader, SeekFunc_t seeker, FileTypes val)
	{
		Devices::CustomReadDevice dev(reader,seeker);
		return LoadTexture(dev, val);
	}

	Image* LoadTexture(ReadFunc_t reader, SeekFunc_t seeker, FileTypes val, LoadCallBack_t callback)
	{
		Devices::CustomReadDevice dev(reader,seeker);
		return LoadTexture(dev, val,callback);
	}

	void FreeTexture(Image* img)
	{
		delete img;
	}

	int RegisterFilter(std::string const &ext, FilterCreator_t filterCreator)
	{
		Filters::FilterRegister reg;
		return reg.RegisterRunTimeFilter(ext,filterCreator);
	}

	void UnRegisterFilter(int filterID)
	{
		Filters::FilterRegister reg;
		reg.UnRegisterRunTimeFilter(filterID);
	}



}