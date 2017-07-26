// GTL Lua : A lua binding for GTL
//
// Exposes the GTL Image object as a table with functions
// - Width, Height, Depth, Size, MipMap count, Image Count, Colour depth and Format are 
//	 named table elements which are read only
// - data pointer is returned as 'userdata' via function
// - 'Formats' are exposed via a read-only table in the GTL "namespace"
// - new Images are created via GTL:newImage()

#include "stdafx.h"
#include "GTL Lua.h"

#include <lua.hpp>
#include "../../../Includes/gtl/GameTextureLoader.hpp"

const char GTLLUA[] = "gtl.image";

struct imgHolder
{
	GameTextureLoader::Image *img;
};

// Meta-table functions
static int l_getData(lua_State *L)
{
	imgHolder * imgh = (imgHolder*)luaL_checkudata(L,1,GTLLUA);
	luaL_argcheck(L,imgh->img != NULL, 1, "NULL image pointer detected");

	int lvl = 0;
	int image = 0;
	const int numargs = lua_gettop(L);
	if(numargs == 3)	// we have all the paramets
	{
		lvl = luaL_checkint(L,-2);
		int maxlvl = imgh->img->getNumMipMaps();
		luaL_argcheck(L, maxlvl >= lvl && lvl >= 0,-2,"Requested Mipmap level is greater than the number of available levels");
		image = luaL_checkint(L,-1);
		int maximg = imgh->img->getNumImages();
		luaL_argcheck(L, maximg >= image && image >= 0,-1,"Requested Image is greater than the number of available images");
	}
	else if (numargs == 2)	// only img and mipmap parameter
	{
		lvl = luaL_checkint(L,-1);
		int maxlvl = imgh->img->getNumMipMaps();
		luaL_argcheck(L, maxlvl >= lvl && lvl >= 0,-1,"Requested Mipmap level is greater than the number of available levels");
	}
	lua_pushlightuserdata(L,imgh->img->getDataPtr(lvl,image));
	return 1;
}

static int l_getColourDepth(lua_State *L)
{
	imgHolder * imgh = (imgHolder*)luaL_checkudata(L,1,GTLLUA);
	luaL_argcheck(L,imgh->img != NULL, -1, "NULL image pointer detected");

	lua_pushinteger(L,imgh->img->getColourDepth());
	return 1;
}

static int l_getNumMipMaps(lua_State *L)
{
	imgHolder * imgh = (imgHolder*)luaL_checkudata(L,1,GTLLUA);
	luaL_argcheck(L,imgh->img != NULL, -1, "NULL image pointer detected");

	lua_pushinteger(L,imgh->img->getNumMipMaps());
	return 1;
}

static int l_getNumImages(lua_State *L)
{
	imgHolder * imgh = (imgHolder*)luaL_checkudata(L,1,GTLLUA);
	luaL_argcheck(L,imgh->img != NULL, -1, "NULL image pointer detected");

	lua_pushinteger(L,imgh->img->getNumImages());
	return 1;
}

static int l_getFormat(lua_State *L)
{
	imgHolder * imgh = (imgHolder*)luaL_checkudata(L,1,GTLLUA);
	luaL_argcheck(L,imgh->img != NULL, -1, "NULL image pointer detected");

	lua_pushinteger(L,imgh->img->getFormat());
	return 1;
}


static int l_getSize(lua_State *L)
{
	imgHolder * imgh = (imgHolder*)luaL_checkudata(L,1,GTLLUA);
	luaL_argcheck(L,imgh->img != NULL, -1, "NULL image pointer detected");

	int lvl = 0;
	if(lua_gettop(L) > 1)
	{
		lvl = luaL_checkint(L,-1);
		int n = imgh->img->getNumMipMaps();
		luaL_argcheck(L, n >= lvl && lvl >= 0,-1,"Requested Mipmap level is greater than the number of available levels");

	}
	lua_pushinteger(L,imgh->img->getSize(lvl));
	return 1;
}

static int l_getWidth(lua_State *L)
{
	imgHolder * imgh = (imgHolder*)luaL_checkudata(L,1,GTLLUA);
	luaL_argcheck(L,imgh->img != NULL, -1, "NULL image pointer detected");

	int lvl = 0;
	if(lua_gettop(L) > 1)
	{
		lvl = luaL_checkint(L,-1);
		int n = imgh->img->getNumMipMaps();
		luaL_argcheck(L, n >= lvl && lvl >= 0,-1,"Requested Mipmap level is greater than the number of available levels");

	}
	lua_pushinteger(L,imgh->img->getWidth(lvl));
	return 1;
}

static int l_getHeight(lua_State *L)
{
	imgHolder * imgh = (imgHolder*)luaL_checkudata(L,1,GTLLUA);
	luaL_argcheck(L,imgh->img != NULL, -1, "NULL image pointer detected");

	int lvl = 0;
	if(lua_gettop(L) > 1)
	{
		lvl = luaL_checkint(L,-1);
		int n = imgh->img->getNumMipMaps();
		luaL_argcheck(L, n >= lvl && lvl >= 0,-1,"Requested Mipmap level is greater than the number of available levels");

	}
	lua_pushinteger(L,imgh->img->getHeight(lvl));
	return 1;
}

static int l_getDepth(lua_State *L)
{
	imgHolder * imgh = (imgHolder*)luaL_checkudata(L,1,GTLLUA);
	luaL_argcheck(L,imgh->img != NULL, -1, "NULL image pointer detected");

	int lvl = 0;
	if(lua_gettop(L) > 1)
	{
		lvl = luaL_checkint(L,-1);
		int n = imgh->img->getNumMipMaps();
		luaL_argcheck(L, n >= lvl && lvl >= 0,-1,"Requested Mipmap level is greater than the number of available levels");

	}
	lua_pushinteger(L,imgh->img->getDepth(lvl));
	return 1;
}

static int l_getDimensions(lua_State *L)
{
	imgHolder * imgh = (imgHolder*)luaL_checkudata(L,1,GTLLUA);
	luaL_argcheck(L,imgh->img != NULL, -1, "NULL image pointer detected");

	int lvl = 0;
	if(lua_gettop(L) > 1)
	{
		lvl = luaL_checkint(L,-1);
		int n = imgh->img->getNumMipMaps();
		luaL_argcheck(L, n >= lvl && lvl >= 0,-1,"Requested Mipmap level is greater than the number of available levels");

	}
	
	// extract all the dimensions
	lua_pushinteger(L,imgh->img->getWidth(lvl));
	lua_pushinteger(L,imgh->img->getHeight(lvl));
	lua_pushinteger(L,imgh->img->getDepth(lvl));
	
	return 3;	// 3 values returned
}

static int l_toString(lua_State *L)
{
	imgHolder * imgh = (imgHolder*)luaL_checkudata(L,1,GTLLUA);
	lua_pushstring(L,"GLT.Image");
	return 1;
}


// Table based functions
static int l_newTexture(lua_State *L)
{
	const char * filename = luaL_checkstring(L,1);
	imgHolder * ptr = (imgHolder*)lua_newuserdata(L,sizeof(imgHolder));
	GameTextureLoader::Image * img = GameTextureLoader::LoadTexture(std::string(filename));
	ptr->img = img;	// This is all a bit hacky :|

	// Register meta-table
	luaL_getmetatable(L,GTLLUA);
	lua_setmetatable(L,-2);

	return 1;	// return userdata which is already on the stack
}

static int l_freeTexture(lua_State *L)
{
	imgHolder * imgh = (imgHolder*)luaL_checkudata(L,1,GTLLUA);
	GameTextureLoader::FreeTexture(imgh->img);
	imgh->img = NULL;
	return 0;	
}

static int l_setOrigin(lua_State *L)
{
	int origin = luaL_checkint(L,-1);
	luaL_argcheck(L, 3 >= origin && origin >= 0,-1,"Origin Value incorrect");
	GameTextureLoader::SetOrigin((GameTextureLoader::ImgOrigin)origin);
	return 0;
}

static const struct luaL_Reg gtlLib[] = {
	{"newTexture", l_newTexture},
	{"freeTexture",l_freeTexture},
	{"setOrigin",l_setOrigin},
	{NULL,NULL}
};

static const struct luaL_Reg gtlLib_meta[] = {
	{"getData", l_getData},
	{"getDimensions",l_getDimensions},
	{"getColourDepth",l_getColourDepth},
	{"getColorDepth",l_getColourDepth},	// for our American friends...
	{"getNumMipMaps",l_getNumMipMaps},
	{"getNumImages",l_getNumImages},
	{"getSize",l_getSize},
	{"getWidth",l_getWidth},
	{"getHeight",l_getHeight},
	{"getDepth",l_getDepth},
	{"getFormat",l_getFormat},
	{"freeTexture",l_freeTexture},
	{"__gc",l_freeTexture},
	{"__tostring",l_toString},
	{NULL,NULL}
};

void setTableField(lua_State *L, const char * key, int value)
{
	lua_pushinteger(L,value);
	lua_setfield(L,-2,key);
}

void register_ImgFormat(lua_State *L)
{
	lua_newtable(L);
	int value = 0;
	setTableField(L,"none",value); value++;
	setTableField(L,"rgb",value); value++;
	setTableField(L,"bgr",value); value++;
	setTableField(L,"rgba",value); value++;
	setTableField(L,"bgra",value); value++;
	setTableField(L,"abgr",value); value++;
	setTableField(L,"dxt1",value); value++;
	setTableField(L,"dxt2",value); value++;
	setTableField(L,"dxt3",value); value++;
	setTableField(L,"dxt4",value); value++;
	setTableField(L,"dxt5",value); value++;
	setTableField(L,"3dc",value); value++;
	setTableField(L,"rgba32b",value); value++;
	setTableField(L,"rgba16f",value); value++;
	setTableField(L,"gr16f",value); value++;
	setTableField(L,"gr32f",value); value++;
	setTableField(L,"r16f",value); value++;
	setTableField(L,"r32f",value); value++;
	setTableField(L,"r5g6b5",value); value++;
	setTableField(L,"x1r5g5b5",value); value++;
	setTableField(L,"a1r5g5b5",value); value++;
	setTableField(L,"l8",value); value++;
	setTableField(L,"a8l8",value); value++;
	setTableField(L,"l16",value); value++;
	setTableField(L,"a8",value); value++;
	setTableField(L,"g16r16",value); value++;
	setTableField(L,"v8u8",value); value++;
	setTableField(L,"v16u16",value); value++;
	setTableField(L,"q8w8v8u8",value); value++;
	setTableField(L,"rgba16",value); value++;
	setTableField(L,"rgb16",value); value++;
	setTableField(L,"a16",value); value++;
	setTableField(L,"a16l16",value); value++;

	
	// Register the table in the GTL table
	lua_setfield(L,-2,"format");

}

void register_ImgOrigin(lua_State *L)
{
	lua_newtable(L);
	int value = 0;
	setTableField(L,"top left",value); value++;
	setTableField(L,"top right",value); value++;
	setTableField(L,"bottom left",value); value++;
	setTableField(L,"bottom right",value); value++;

	// Register the table in the GTL table
	lua_setfield(L,-2,"origin");
}

void register_Type(lua_State *L)
{
	lua_newtable(L);
	int value = 0;
	setTableField(L,"bmp",value); value++;
	setTableField(L,"jpg",value); value++;
	setTableField(L,"tga",value); value++;
	setTableField(L,"png",value); value++;
	setTableField(L,"dds",value); value++;

	// Register the table in the GTL table
	lua_setfield(L,-2,"type");
}

// Does the registering
GTLLUA_API int luaopen_gtl(lua_State *L)
{
	// Create a meta-table to identify GTL::Image types
	luaL_newmetatable(L, GTLLUA);
	// Duplicate the meta table
	lua_pushvalue(L,-1);
	// Set the meta table's __index to the meta table
	lua_setfield(L,-2,"__index");

	// Now register the 'per instance' functions
	luaL_register(L,NULL,gtlLib_meta);
	// Finally register the lib's only stand alone function
	luaL_register(L,"GTL",gtlLib);
	// Need to create three 'sub tables' in the GTL namespace which contain
	// the data for image type, colour depth data and origin data
	register_ImgFormat(L);
	register_ImgOrigin(L);
	register_Type(L);
	return 1;
}
