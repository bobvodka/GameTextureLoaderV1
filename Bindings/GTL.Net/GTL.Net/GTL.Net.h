// GTL.Net.h

#pragma once

using namespace System;

#include "../../../Includes/gtl/GameTextureLoader.hpp"

namespace GTLNet {

	public enum class ImgFormat
	{
		FORMAT_NONE = 0,
		FORMAT_RGB,
		FORMAT_BGR,
		FORMAT_RGBA,
		FORMAT_BGRA,
		FORMAT_ABGR,
		FORMAT_DXT1,
		FORMAT_DXT2,
		FORMAT_DXT3,
		FORMAT_DXT4,
		FORMAT_DXT5,
		FORMAT_3DC,
	};

	public enum class FileTypes {
		TYPE_BMP = 1,
		TYPE_JPG,
		TYPE_TGA,
		TYPE_PNG,
		TYPE_DDS
	};

	public ref class Image {
	public:
		~Image() {
			GameTextureLoader::FreeTexture(image);
		}
		!Image() {
			this->~Image();
		}

		property int Height {
			int get() {
				return image->getHeight();
			}
		}
		property int Width {
			int get() {
				return image->getWidth();
			}
		}
		property int Depth {
			int get() {
				return image->getDepth();
			}
		}
		property int ColorDepth {
			int get() {
				return image->getColourDepth();
			}
		}
		property ImgFormat ImageFormat {
			ImgFormat get() {
				return safe_cast<ImgFormat>(image->getFormat());
			}
		}
		property int ImageCount {
			int get() {
				return image->getNumImages();
			}
		}
		property int MipMapCount {
			int get() {
				return image->getNumMipMaps();
			}
		}
		IntPtr getData(int mipmaplvl, int image)
		{
			return static_cast<IntPtr>(this->image->getDataPtr(mipmaplvl,image));
		}
		IntPtr getData(int mipmaplvl)
		{
			return getData(mipmaplvl,0);
		}
		IntPtr getData()
		{
			return getData(0,0);
		}
	public:
		static Image^ LoadTexture(System::String^ fileName) {
			IntPtr ptr = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(fileName);
			char* fileNamePtr = static_cast<char*>(ptr.ToPointer());
			Image^ img = gcnew Image();

			img->image = GameTextureLoader::LoadTexture(std::string(fileNamePtr));

			System::Runtime::InteropServices::Marshal::FreeHGlobal(ptr);
			return img;
		}

		static Image^ LoadTexture(System::String^ fileName, FileTypes type) {
			IntPtr ptr = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(fileName);
			char* fileNamePtr = static_cast<char*>(ptr.ToPointer());
			Image^ img = gcnew Image();

			img->image = GameTextureLoader::LoadTexture(std::string(fileNamePtr, safe_cast<GameTextureLoader::FileTypes>(type)));

			System::Runtime::InteropServices::Marshal::FreeHGlobal(ptr);
			return img;
		}
	private:
		GameTextureLoader::Image* image;
	};
}
