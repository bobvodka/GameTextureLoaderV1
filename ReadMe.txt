========================================================================
    The Game Texture Loader
========================================================================

Sections
- Description
- Installation
- TODOs
- Version History
- Notes
- Contact Infomation

Description:

The Game Texture Loader (GTL) is designed to be a simple system for
the loading of common game format textures via a small number of
functions.

As of this version if supports the following files;
- BMP
- JPG
- PNG
- TGA
- DDS (DXTn compressed images are indentified)

TGA, PNG and JPG are decompressed as required to form a bitmap image,
DDS files on the other hand have no processing done to them, allowing
you to upload DDS compressed images directly to the graphics API of
your choice or even just access the compressed data directly if required.

It currently supports loading from a given file name, from a PhysFS 
file handle or custom reading functions.

========================================================================
Installation
========================================================================

Visual Studio.Net 2003
-----------------------

Load the solution provided in the MSVS.Net folder and rebuild the 
solution.
The libs will be copied to the 'libs' directory.

Visual Studio 2005
-------------------

Load the solution provided in the VS05 folder and rebuild the solution.
As VS05 can compile both 32bit and 64bit the solution as targets for both
32bit and 64bit system. 
All 32bit libraries are copied to 'libs\win32' and all 64bit libraries
are copied to 'libs\x64'.

Visual Studio Express Edition
------------------------------

The Express Edition can also use the same solution as the full visual 
studio. However, the EE lacks support for 64bit targets, so while 64bit
will appear as a target option you probably wont get any output from it
other than errors.

The 32bit libraries are copied to 'libs\win32'.

Once they are compiled to use insure that the 'Include' folder and the
correct 'lib' folders are in the compiler's paths. 

Build Options
--------------

If you want autolinking to work correctly on your projects then the 
linker MUST know where to find the libraries it requires. If for some 
reason you dont want to use the autolinker system then define 
GTL_NO_AUTOLINK to disable.

When compiling it is possible to leave some formats out of the final
library. This is done by changing the config.hpp file and removing
the relivent define for the image type. 

For example, commenting out "#define GTL_BITMAP_FILTER" would disable
bitmap loading support.

PhysFS support can also be controlled in the same way.

This library makes a fair amount of use of the Boost C++ library, so
if you dont have at least version 1.33 installed and compiled it wont
compile or work. 

========================================================================
TODOs
========================================================================

- C interface
- SDL compatible interface?
- work out which zlib functions are required by libpng but not used
  by physfs and come up with a tricky hack to get it all working.

========================================================================
Version History
========================================================================

Version 2.0.1
- Total rejig of the interface, so it now lives in a class
- Load functions now return pointers to be a tad more DLL friendly
- Added 'safe' load functions which return boost::shared_ptr

Version 1.0.1
??/12/05
- Fixed typo in the 2nd PhysFS support function

Version 1.0.0
01/10/05
- Inital version
- Support included for;
-- BMP
-- JPG
-- PNG
-- TGA
-- DDS
- Reading supported from
-- Named file
-- PhysFS handle
-- Custom read/seek functions (untested but should work)

========================================================================
Notes
========================================================================

Might be some endian issues on none Intel systems.

========================================================================
Contact Infomation
========================================================================

Send any general questions, bug reports etc to me (Rob Jones):
  rob [at] phantom-web.co.uk