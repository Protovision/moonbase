#include "moonbase.h"

int		base_pool_size;
char		*base_game_path;
char		*base_data_path;
char		*base_config_script;
char		*base_main_script;
TALLOC_CTX	*base_pool;
lua_State	*base_engine_state;
lua_State	*base_game_state;
int		base_fps;
Uint32		base_resume_time;

static void *base_engine_state_allocator( void *ud, void *ptr, size_t osize, size_t nsize )
{
	if ( nsize == 0 ) {
		talloc_free( ptr );
		return NULL;
	}
	return talloc_realloc_size( base_pool, ptr, nsize );
}

static int base_archive_searcher( lua_State *s )
{
	archive_load_script( luaL_checkstring(s, 1) );
	lua_pushnil( s );
	return 1;
}

static void base_help( const char *argv0 )
{
	printf(
		"Moonbase\n"
		"An engine for Lua-driven applications.\n"
		"Usage: %s [options] [game] \n\n"
		"game   Path of game archive to load (Default: game.zip)\n"
		"-c     Specify script to load before window creation (Default: config.lua)\n"
		"-m     Specify script to load after window creation (Default: main.lua)\n"
		"-d     Path for game to save data (Default value platform dependent)\n"
		"-p     Have lua use a memory pool; specify size of pool in megabytes\n"
		"-l     Set log level from 1-6, higher is more verbose. (Default: 4)\n",
		argv0 );
}

void base_initialize( int argc, char *argv[] )
{
	int i, ch;
	char *p;

	base_pool_size = 0;
	base_pool = NULL;
	base_game_path = NULL;
	base_data_path = NULL;
	base_config_script = NULL;
	base_main_script = NULL;
	base_fps = 30;
	base_resume_time = 0;

	SDL_Init( 0 );
	log_set_verbosity( LOG_VERBOSE );

	for ( i = 1; i < argc; ++i ) {
		if ( argv[i][0] != '-' ) {
			if ( base_game_path != NULL ) {
				SDL_free( base_game_path );
			}
			base_game_path = SDL_strdup( argv[i] );
			continue;
		}
		ch = argv[i][0];
		if ( ch == '-' ) {
			ch = argv[i][1];
		}
		switch ( ch ) {
		case 'c':
			base_config_script = SDL_strdup( argv[++i] );
			break;
		case 'm':
			base_main_script = SDL_strdup( argv[++i] );
			break;
		case 'p':
			base_pool_size = SDL_atoi( argv[++i] );
			break;
		case 'd':
			base_data_path = SDL_strdup( argv[++i] );
			break;
		case 'l':
			if ( SDL_isdigit(argv[i+1][0]) ) {
				log_set_verbosity( SDL_atoi(argv[++i]) );
			} else {
				log_increment_verbosity( );
			}
			break;
		case 'h':
			base_help( argv[0] );
			exit( 0 );
		default:
			break;
		}
	}

	if ( base_config_script == NULL ) {
		base_config_script = SDL_strdup( "config.lua" );
	}
	if ( base_main_script == NULL ) {
		base_main_script = SDL_strdup( "main.lua" );
	}
	if ( base_game_path == NULL ) {
		p = SDL_GetBasePath( );
		base_game_path = SDL_strdup( vstr("%sgame.zip", p, "game.zip") );
		SDL_free( p );
	}
	if ( base_data_path == NULL ) {
		base_data_path = SDL_GetPrefPath( "swoope", "moonbase" );
	}

	TTF_Init( );
	IMG_Init( IMG_INIT_JPG|IMG_INIT_PNG );
	asset_initialize( );
	archive_initialize( );

	if ( base_pool_size != 0 ) {
		base_pool = talloc_pool( NULL, base_pool_size * (1024*1024) );
		if ( base_pool == NULL ) {
			fatal( "Failed to allocate lua memory pool\n" );
		}
		base_engine_state = lua_newstate( base_engine_state_allocator, NULL );
	} else {
		base_engine_state = luaL_newstate( );
	}

	if ( base_engine_state == NULL ) {
		fatal( "Failed to initialize Lua\n" );
	}

	luaL_openlibs( base_engine_state );
	luacom_get_global_field( base_engine_state, "package", "searchers", NULL );
	lua_pushcfunction( base_engine_state, base_archive_searcher );
	lua_pushinteger( base_engine_state, 5 );
	lua_settable( base_engine_state, -3 );
	lua_settop( base_engine_state, 0 );
}

void base_shutdown( )
{
	lua_close( base_engine_state );
	if ( base_pool != NULL ) {
		talloc_free( base_pool );
	}
	archive_shutdown( );
	asset_shutdown( );
	IMG_Quit( );
	TTF_Quit( );
	SDL_free( base_game_path );
	SDL_free( base_data_path );
	SDL_free( base_config_script );
	SDL_free( base_main_script );
}

static int moonbase_quit( lua_State *s )
{
	quit( 0 );
	return 0;
}

static int moonbase_get_fps( lua_State *s )
{
	lua_pushinteger( s, base_fps );
	return 1;
}

static int moonbase_set_fps( lua_State *s )
{
	base_fps = luaL_checkinteger( s, 1 );
	return 0;
}

static int moonbase_yield( lua_State *s )
{
	if ( lua_gettop(s) == 1 ) {
		base_resume_time = SDL_GetTicks( ) + luaL_checkunsigned( s, 1 );
	} else {
		base_resume_time = 0;
	}
	lua_yield( base_game_state, 0 );
	return lua_gettop( base_game_state );
}

static int moonbase_resume( lua_State *s )
{
	int nargs;

	base_resume_time = 0;
	nargs = lua_gettop( base_engine_state );
	lua_xmove( base_engine_state, base_game_state, nargs );
	lua_resume( base_game_state, base_engine_state, nargs );
	return lua_gettop( base_game_state );
}

static int moonbase_get_ticks( lua_State *s )
{
	lua_pushunsigned( s, SDL_GetTicks() );
	return 1;
}

static luaL_Reg moonbase_methods[] = {
	{ "getFps", moonbase_get_fps },
	{ "setFps", moonbase_set_fps },
	{ "yield", moonbase_yield },
	{ "resume", moonbase_resume },
	{ "getTicks", moonbase_get_ticks },
	{ "quit", moonbase_quit },
	{ NULL, NULL }
};

int moonbase_dummy_function( lua_State *s )
{
	return 0;
}

void moonbase_initialize( lua_State *s )
{
	extern int \
	moonbase_audio_initialize( lua_State * ),
	moonbase_video_initialize( lua_State * ),
	moonbase_archive_initialize( lua_State * ),
	moonbase_storage_initialize( lua_State * ),
	moonbase_text_initialize( lua_State * ),
	moonbase_log_initialize( lua_State * );

	lua_newtable( s );
	luaL_setfuncs( s, moonbase_methods, 0 );
	moonbase_text_initialize( s );
	lua_setfield( s, 1, "text" );
	moonbase_log_initialize( s );
	lua_setfield( s, 1, "log" );
	moonbase_audio_initialize( s );
	lua_setfield( s, 1, "audio" );
	moonbase_video_initialize( s );
	lua_setfield( s, 1, "video" );
	moonbase_archive_initialize( s );
	lua_setfield( s, 1, "archive" );
	moonbase_storage_initialize( s );
	lua_setfield( s, 1, "storage" );
	lua_newtable( s );
	lua_pushcfunction( s, moonbase_dummy_function );
	lua_pushvalue( s, -1 );
	lua_pushvalue( s, -1 );
	lua_pushvalue( s, -1 );
	lua_setfield( s, 2, "initialize" );
	lua_setfield( s, 2, "input" );
	lua_setfield( s, 2, "update" );
	lua_setfield( s, 2, "shutdown" );
	lua_setfield( s, 1, "event" );
	lua_pushcfunction( s, moonbase_dummy_function );
	lua_setfield( s, 1, "main" );
	lua_setglobal( s, "moonbase" );
}

char	*vstr(const char *fmt, ...)
{
	va_list v;
	char *buf;

	static int index = 0;
	static char strings[2][MAX_STRING];
		
	buf = strings[index & 1];
	++index;
	
	va_start(v, fmt);
	vsnprintf(buf, MAX_STRING, fmt, v);
	va_end(v);

	return buf;
}

char	*joinpath(const char *base, const char *p)
{
	char last;
	char *s, *path;

	static int index = 0;
	static char buf[2][MAX_PATHNAME];

	path = buf[index & 1];
	++index;

	last = 0;

#ifdef _WINDOWS
	sprintf(path, "%s\\", base);
#else
	sprintf(path, "%s/", base);
#endif

	for (s = path + strlen(path); *p; ++p, ++s) {
		if (last == '.' && *p == '.') {
			--s;
			continue;
		}
#ifdef _WINDOWS
		if (*p == '/') last = *s = '\\';
		else
#endif
		last = *s = *p;
	}

	*s = 0;
	return path;
}
