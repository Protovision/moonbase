/***********************************************************
 * luacom - Crappy lua helper functions for C 
 *
 * Use at your own risk. Author is not responsible for
 * injuries, deaths, or emotional traumas caused by software
 * that uses this code.
 **********************************************************/

#ifndef LUACOM_H
#define LUACOM_H

#include <stdarg.h>
#ifndef LUA_COMPAT_APIINTCASTS
#define LUA_COMPAT_APIINTCASTS
#endif
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

/***********************************************************
 *
 *	Format character sequence
 *
 *	i	integer
 *	u	unsigned integer
 *	n	double-precision number
 *	s	string
 *	f	function
 *	b	boolean
 *	p	userdata
 *
 **********************************************************/

/***********************************************************
 * luacom_write_array
 *
 * Writes to values to an array's indices
 *
 * s		= Lua state
 * index	= Location of array
 * format	= Format of values to be inserted
 * ...		= An even number of arguments, each 2
 *		arguments consists of an array index 
 *		followed by a value
 *
 **********************************************************/
void	luacom_write_array( lua_State *s, int index, const char *format, ... );

/***********************************************************
 * luacom_read_array
 *
 * Reads values from an array
 *
 * s		= Lua state
 * index	= Location of array
 * format	= Format of values to be read
 * ...		= An even number of arguments, each 2 
 *		arguments consists of an array index 
 *		followed by a pointer for the result
 *
 **********************************************************/
void	luacom_read_array( lua_State *s, int index, const char *format, ... );

/***********************************************************
 * luacom_write_table
 *
 * Writes to values to a table's fields
 *
 * s		= Lua state
 * index	= Location of table 
 * format	= Format of values to be inserted
 * ...		= An even number of arguments, each 2
 *		arguments consists of a field name
 *		followed by a value
 *
 **********************************************************/
void	luacom_write_table( lua_State *s, int index, const char *format, ... );

/***********************************************************
 * luacom_read_table
 *
 * Reads values from a table
 *
 * s		= Lua state
 * index	= Location of table
 * format	= Format of values to be read
 * ...		= An even number of arguments, each 2 
 *		arguments consists of a field name
 *		followed by a pointer for the result
 *
 **********************************************************/
void	luacom_read_table( lua_State *s, int index, const char *format, ... );

/***********************************************************
 * luacom_append_stack
 *
 * Appends values to a Lua stack
 *
 * s		= Lua state
 * format	= Format of values 
 * ...		= The values to append
 *
 **********************************************************/
void	luacom_append_stack( lua_State *s, const char *format, ... );

/***********************************************************
 * luacom_read_stack
 *
 * Reads values from a Lua stack
 *
 * s		= Lua state
 * format	= Format of values
 * ...		= An even number of arguments, each 2
 *		arguments consists of an index and a 
 *		pointer for the result
 *
 **********************************************************/
void	luacom_read_stack( lua_State *s, const char *format, ... );

/***********************************************************
 * luacom_create_object 
 *
 * Create a userdata object with methods
 *
 * s		= Lua state
 * classname	= Name of userdata type
 * udata	= Pointer to userdata
 * udata_size	= Size of userdata
 * methods	= Methods that operate on the userdata
 		(use luaL_checkudata to retrieve the userdata)
 *
 **********************************************************/
void	luacom_create_object( lua_State *s, const char *classname, void *udata, size_t udata_size, const luaL_Reg *methods );

/***********************************************************
 * luacom_get_global_field 
 *
 * Retreive a nested value from a global lua table
 *
 * s		= Lua state
 * global	= Name of the global table
 * ...		= Names of nested values. Each value
 *		is a member of the table name of the 
 *		previous argument
 *
 **********************************************************/
int	luacom_get_global_field( lua_State *s, const char *global, ... );

#endif

