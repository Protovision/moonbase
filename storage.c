#include "moonbase.h"

FILE *storage_open( const char *path, const char *mode )
{
	return fopen( joinpath(base_data_path, path), mode );
}

int storage_rename( const char *oldpath, const char *newpath )
{
	const char *abs_oldpath, *abs_newpath;

	abs_oldpath = joinpath( base_data_path, oldpath );
	abs_newpath = joinpath( base_data_path, newpath );
	return rename( abs_oldpath, abs_newpath ) == 0;
}

int storage_remove( const char *path )
{
	return remove( joinpath(base_data_path, path) ) == 0;
}

static int moonbase_storage_open( lua_State *s )
{
	const char *relative_path, *mode;
	char *absolute_path;

	relative_path = luaL_checkstring( s, 1 );
	mode = luaL_checkstring( s, 2 );
	absolute_path = joinpath( base_data_path, relative_path );
	lua_getglobal( s, "io" );
	lua_getfield( s, -1, "open" );
	lua_pushstring( s, absolute_path );
	lua_pushstring( s, mode );
	lua_call( s, 2, 1 );
	return 1;
}

static int moonbase_storage_rename( lua_State *s )
{
	storage_rename( luaL_checkstring(s, 1), luaL_checkstring(s, 2) );	
	return 0;
}

static int moonbase_storage_remove( lua_State *s )
{
	storage_remove( luaL_checkstring(s, 1) );
	return 0;
}

static luaL_Reg moonbase_storage_methods[] = {
	{ "open", moonbase_storage_open },
	{ "rename", moonbase_storage_rename },
	{ "remove", moonbase_storage_remove }
};

int moonbase_storage_initialize( lua_State *s )
{
	luaL_newlib( s, moonbase_storage_methods );
	return 1;
}
