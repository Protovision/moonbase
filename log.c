#include "moonbase.h"

char fatal_buffer[ 4096 ];

static int log_priority_to_level( SDL_LogPriority p )
{
	static int levels[] = {
		0,
		LOG_VERBOSE,
		LOG_DEBUG,
		LOG_INFO,
		LOG_WARN,
		LOG_ERROR,
		LOG_CRITICAL
	};
	return levels[ p ];
}

static int log_level_to_priority( int level )
{
	static int priorities[] = {
		0,
		SDL_LOG_PRIORITY_CRITICAL,
		SDL_LOG_PRIORITY_ERROR,
		SDL_LOG_PRIORITY_WARN,
		SDL_LOG_PRIORITY_INFO,
		SDL_LOG_PRIORITY_DEBUG,
		SDL_LOG_PRIORITY_VERBOSE
	};
	return priorities[ level ];
}

int log_get_verbosity( )
{
	return log_priority_to_level( SDL_LogGetPriority(SDL_LOG_CATEGORY_APPLICATION) );
}

void log_set_verbosity( int level )
{
	SDL_LogSetPriority( SDL_LOG_CATEGORY_APPLICATION, log_level_to_priority(level) );	
}

void log_increment_verbosity( )
{
	int level;

	level = log_get_verbosity( );
	if ( level == 6 ) {
		return;
	}
	log_set_verbosity( ++level );
}

void log_decrement_verbosity( )
{
	int level;

	level = log_get_verbosity( );
	if ( level == 1 ) {
		return;
	}
	log_set_verbosity( --level );
}

void log_printf( int level, const char *fmt, ... )
{
	va_list v;

	va_start( v, fmt );
	SDL_LogMessageV(
		SDL_LOG_CATEGORY_APPLICATION,
		log_level_to_priority(level), 
		fmt,
		v
	);
	va_end( v );
}

static int moonbase_log_critical( lua_State *s )
{
	log_printf( LOG_CRITICAL, "%s", luaL_checkstring(s, 1) );
	return 0;
}

static int moonbase_log_error( lua_State *s )
{
	log_printf( LOG_ERROR, "%s", luaL_checkstring(s, 1) );
	return 0;
}

static int moonbase_log_warn( lua_State *s )
{
	log_printf( LOG_WARN, "%s", luaL_checkstring(s, 1) );
	return 0;
}

static int moonbase_log_info( lua_State *s )
{
	log_printf( LOG_INFO , "%s", luaL_checkstring(s, 1) );
	return 0;
}

static int moonbase_log_debug( lua_State *s )
{
	log_printf( LOG_DEBUG, "%s", luaL_checkstring(s, 1) );
	return 0;
}

static int moonbase_log_verbose( lua_State *s )
{
	log_printf( LOG_VERBOSE, "%s", luaL_checkstring(s, 1) );
	return 0;
}

static luaL_Reg moonbase_log_methods[] = {
	{ "critical", moonbase_log_critical },
	{ "error", moonbase_log_error },
	{ "warn", moonbase_log_warn },
	{ "info", moonbase_log_info },
	{ "debug", moonbase_log_debug },
	{ "verbose", moonbase_log_verbose },
	{ NULL, NULL }
};

int moonbase_log_initialize( lua_State *s )
{
	luaL_newlib( s, moonbase_log_methods );
	return 1;
}

