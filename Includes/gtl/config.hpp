// A config setup file for GTL
// Sets up defines for:
// - Filters
// - PhysFS support
//
// Also setup autolinking under VC7.x

#ifndef GTL_CONFIG_HPP
#define GTL_CONFIG_HPP

#ifdef GTL_BUILD
	#define GTL_BITMAP_FILTER
	#define GTL_DDS_FILTER
	#define GTL_TGA_FILTER
	#define GTL_JPG_FILTER
	#define GTL_PNG_FILTER
#endif

// Autolinking options
#if !defined(GTL_BUILD) && !defined(GTL_NO_AUTOLINK)
#pragma message("GTL: AutoLinking")
	#if defined(_MSC_VER) && !defined(__MWERKS__) && !defined(__EDG_VERSION__)
		#define OGLWFW_MSVC _MSC_VER
	#endif

	#if defined(OGLWFW_MSVC) \
	|| defined(__BORLANDC__) \
	|| (defined(__MWERKS__) && defined(_WIN32) && (__MWERKS__ >= 0x3000)) \
	|| (defined(__ICL) && defined(_MSC_EXTENSIONS) && (_MSC_VER >= 1200))

		#if defined(_MSC_VER) || defined(__MWERKS__) 
			#if defined(_DEBUG)
				#if defined(GTL_PHYSFS_SUPPORT)
					#if defined(_DLL)
						#pragma comment(lib,"GTL_mt_dll_PhysFS_d.lib")
						#pragma message("GTL: Multi-Thread DLL Debug Runtime With PhysFS Support")
					#elif defined(_MT)
						#pragma comment(lib,"GTL_mt_PhysFS_d.lib")
						#pragma message("GTL: Multi-Thread Debug Runtime With PhysFS Support")
					#else
						#pragma comment(lib,"GTL_PhysFS_d.lib")
						#pragma message("GTL: Single Thread Debug Runtime With PhysFS Support")
					#endif
				#else
					#if defined(_DLL)
						#pragma comment(lib,"GTL_mt_dll_d.lib")
						#pragma message("GTL: Multi-Thread DLL Debug Runtime")
					#elif defined(_MT)
						#pragma comment(lib,"GTL_mt_d.lib")
						#pragma message("GTL: Multi-Thread Debug Runtime")
					#else
						#pragma comment(lib,"GTL_d.lib")
						#pragma message("GTL: Single Thread Debug Runtime")
					#endif
				#endif
			#else
				#if defined(GTL_PHYSFS_SUPPORT)
					#if defined(_DLL)
						#pragma comment(lib,"GTL_mt_dll_PhysFS.lib")
						#pragma message("GTL: Multi-Thread DLL Runtime With PhysFS Support")
					#elif defined(_MT)
						#pragma comment(lib,"GTL_mt_PhysFS.lib")
						#pragma message("GTL: Multi-Thread Runtime With PhysFS Support")
					#else
						#pragma comment(lib,"GTL_PhysFS.lib")
						#pragma message("GTL: Single Thread Runtime With PhysFS Support")
					#endif
				#else
					#if defined(_DLL)
						#pragma comment(lib,"GTL_mt_dll.lib")
						#pragma message("GTL: Multi-Thread DLL Runtime")
					#elif defined(_MT)
						#pragma comment(lib,"GTL_mt.lib")
						#pragma message("GTL: Multi-Thread Runtime")
					#else
						#pragma comment(lib,"GTL.lib")
						#pragma message("GTL: Single Thread Runtime")
					#endif
				#endif
			#endif
		#endif
	#endif
#endif // end !defined(GTL_BUILD)


#endif