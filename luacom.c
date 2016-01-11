#include "luacom.h"

#define _LUACOM_PUSH_NEXT_TYPE( type )	\
	do {	\
		switch ( type ) {	\
			case 'i':	\
				lua_pushinteger( s, va_arg(v, int) );	\
				break;	\
			case 'u':	\
				lua_pushunsigned( s, va_arg(v, unsigned int) );	\
				break;	\
			case 'n':	\
				lua_pushnumber( s, va_arg(v, double) );	\
				break;	\
			case 's':	\
				lua_pushstring( s, va_arg(v, const char*) );	\
				break;	\
			case 'f':	\
				lua_pushcfunction( s, va_arg(v, lua_CFunction) );	\
				break;	\
			case 'b':	\
				lua_pushboolean( s, va_arg(v, int) );	\
				break;	\
			case 'p': \
				lua_pushlightuserdata( s, va_arg(v, void*) ); \
				break; \
			default:	\
				break;	\
			}	\
	} while ( 0 )

void luacom_write_array( lua_State *s, int index, const char *format, ... )
{
	int i, n;
	va_list v;

	luaL_checktype( s, index, LUA_TTABLE );
	if ( index < 0 ) {
		index = index - 2;
	}
	va_start( v, format );
	for ( i = 0; format[i] != 0; ++i ) {
		lua_pushinteger( s, va_arg(v, int) );
		_LUACOM_PUSH_NEXT_TYPE( format[i] );
		lua_settable( s, index );
	}
	va_end( v );
}

void luacom_write_table( lua_State *s, int index, const char *format, ... )
{
	int i;
	const char *field;
	va_list v;

	luaL_checktype( s, index, LUA_TTABLE );
	if ( index < 0 ) {
		index = index - 2;
	}
	va_start( v, format );
	for ( i = 0; format[i] != 0; ++i ) {
		lua_pushstring( s, va_arg(v, const char*) );
		_LUACOM_PUSH_NEXT_TYPE( format[i] );
		lua_settable( s, index );
	}
	va_end( v );
}

void luacom_append_stack( lua_State *s, const char *format, ... )
{
	int i;
	va_list v;

	va_start( v, format );
	for ( i = 0; format[i] != 0; ++i ) {
		_LUACOM_PUSH_NEXT_TYPE( format[i] );
	}
}

#define _LUACOM_READ_NEXT_TYPE( _type, _index )	\
	do {	\
		switch ( _type ) {	\
		case 'i':	\
			*va_arg( v, int* ) = (int)luaL_checkint( s, _index );	\
			break;	\
		case 'u':	\
			*va_arg( v, unsigned int* ) = (unsigned int)luaL_checkunsigned( s, _index );	\
			break;	\
		case 'n':	\
			*va_arg( v, double* ) = (double)luaL_checknumber( s, _index );	\
			break;	\
		case 's':	\
			*va_arg( v, const char** ) = (const char*)luaL_checkstring( s, _index );	\
			break;	\
		case 'b':	\
			luaL_checktype( s, _index, LUA_TBOOLEAN );	\
			*va_arg( v, int* ) = (int)lua_toboolean( s, _index );	\
			break;	\
		case 'p': \
			luaL_checktype( s, _index, LUA_TLIGHTUSERDATA ); \
			*va_arg( v, const void** ) = lua_topointer( s, _index ); \
			break; \
		default:	\
			break;	\
		}	\
	} while ( 0 )

void luacom_read_array( lua_State *s, int index, const char *format, ... )
{
	int i, n;
	va_list v;

	luaL_checktype( s, index, LUA_TTABLE );
	if ( index < 0 ) {
		index = index - 1;
	}
	va_start( v, format );
	for ( i = 0; format[i] != 0; ++i ) {
		lua_pushinteger( s, va_arg(v, int) );
		lua_gettable( s, index );
		_LUACOM_READ_NEXT_TYPE( format[i], -1 );
		lua_pop( s, 1 );
	}
	va_end( v );
}

void luacom_read_table( lua_State *s, int index, const char *format, ... )
{
	int i, n;
	va_list v;

	luaL_checktype( s, index, LUA_TTABLE );
	if ( index < 0 ) {
		index = index - 1;
	}
	va_start( v, format );
	for ( i = 0; format[i] != 0; ++i ) {
		lua_pushstring( s, va_arg(v, const char*) );
		lua_gettable( s, index );
		_LUACOM_READ_NEXT_TYPE( format[i], -1 );
		lua_pop( s, 1 );
	}
	va_end( v );
}

void luacom_read_stack( lua_State *s, const char *format, ... )
{
	int i;
	va_list v;

	va_start( v, format );
	for ( i = 0; format[i] != 0; ++i ) {
		_LUACOM_READ_NEXT_TYPE( format[i], va_arg(v, int) );
	}
	va_end( v );
}

void luacom_create_object( lua_State *s, const char *classname, void *data, size_t data_size, const luaL_Reg *methods )
{
	void *userdata;

	userdata = lua_newuserdata( s, data_size );
	memcpy( userdata, data, data_size );
	luaL_newmetatable( s, classname );
	luaL_setfuncs( s, methods, 0 );
	lua_pushvalue( s, -1 );
	lua_setfield( s, -2, "__index" );
	lua_setmetatable( s, -2 );
}

int luacom_get_global_field( lua_State *s, const char *global, ... )
{
	int top, i;
	va_list v;
	const char *field;

	va_start( v, global );
	lua_getglobal( s, global );
	top = lua_gettop( s );
	for ( ;; ) {
		field = va_arg( v, const char* );
		if ( field == NULL ) break;
		lua_pushstring( s, field );
		lua_rawget( s, -2 );
		if ( lua_isnil(s, -1) ) {
			lua_settop( s, top - 1 );
			va_end( v );
			return 0;
		}
		lua_remove( s, top ); 
	}
	va_end( v );
	return 1;
}
