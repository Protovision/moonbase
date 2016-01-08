#include "moonbase.h"
#include "unzip.h"

static unzFile		archive_handle;

void archive_initialize( )
{
	archive_handle = unzOpen( base_game_path );
	if ( archive_handle == NULL ) {
		fatal( "Failed to open zip file: %s\n", base_game_path );
	}
}

void archive_shutdown( )
{
	unzClose( archive_handle );
}

int archive_contains( const char *filename )
{
	return ( unzLocateFile(archive_handle, filename, 1) == UNZ_OK );
}

void archive_load_data( const char *file, void **ptr, size_t *size )
{
	unz_file_info info;
	char *data, *data_end;
	int got;

	if ( unzLocateFile(archive_handle, file, 1) != UNZ_OK ) {
		fatal( "Failed to locate archived file: %s\n", file );
	}
	unzOpenCurrentFile( archive_handle );
	unzGetCurrentFileInfo( archive_handle, &info, NULL, 0, NULL, 0, NULL, 0 );
	data = (char*)SDL_malloc( info.uncompressed_size+1 );
	*ptr = data;
	*size = info.uncompressed_size;
	data_end = data + info.uncompressed_size;
	*data_end = 0;

	while ((got = unzReadCurrentFile(archive_handle, data, data_end - data)) > 0) {
		data = data + got;
	}
	unzCloseCurrentFile( archive_handle );
}

int archive_load_script( const char *file )
{
	char *script;
	size_t size;
	int top, new_top;

	top = lua_gettop( base_engine_state );
	archive_load_data( file, (void**)&script, &size );
	if ( script == NULL ) {
		fatal( "Failed to find file in archive:\n%s\n", file );
	}
	if ( luaL_dostring(base_engine_state, script) ) {
		fatal( "%s:\n%s\n", file, lua_tostring(base_engine_state, -1) );
	}
	SDL_free( script );
	new_top = lua_gettop( base_engine_state );
	return new_top - top;
}

static int archive_close_file( SDL_RWops *ops )
{
	SDL_free( ops->hidden.mem.base );
	SDL_FreeRW( ops );
	return 0;
}

static SDL_RWops *archive_open_file( const char *filename )
{
	void *data;
	size_t size;
	SDL_RWops *ops;

	archive_load_data( filename, &data, &size );
	ops = SDL_RWFromMem( data, size );
	if ( ops ) ops->close = archive_close_file;
	return ops;
}

ASSET *archive_load_font( const char *filename, int size )
{
	ASSET *asset;
	TTF_Font *font;
	char buf[264];

	asset = asset_find( filename );
	if ( asset != NULL ) {
		return asset_acquire( asset );
	}
	font = TTF_OpenFontRW( archive_open_file(filename), 1, size );
	if ( font == NULL ) {
		fatal( "Failed to load font %s:\n%s\n", filename, TTF_GetError() );
	}
	SDL_snprintf( buf, 256, "%s:%d", filename, size );
	return asset_create( font, ASSET_FONT, buf );
}

ASSET *archive_load_image( const char *filename )
{
	ASSET *asset;
	SDL_Texture *texture;
	SDL_Surface *surface;

	asset = asset_find( filename );
	if ( asset != NULL ) {
		return asset_acquire( asset );
	}
	surface = IMG_Load_RW( archive_open_file(filename), 1 );
	if ( surface == NULL ) {
		fatal( IMG_GetError() );
	}
	texture = SDL_CreateTextureFromSurface( video_renderer, surface );
	if ( texture == NULL ) {
		fatal( SDL_GetError() );
	}
	SDL_FreeSurface( surface );
	SDL_SetTextureBlendMode( texture, SDL_BLENDMODE_BLEND );
	return asset_create( texture, ASSET_IMAGE, filename );
}

ASSET *archive_load_sound( const char *filename )
{
	ASSET *asset;
	Mix_Chunk *sound;

	asset = asset_find( filename );
	if ( asset != NULL ) {
		return asset_acquire( asset );
	}
	sound = Mix_LoadWAV_RW( archive_open_file(filename), 1 );
	if ( sound == NULL ) {
		fatal( Mix_GetError() );
	}
	return asset_create( sound, ASSET_SOUND, filename );
}

static int moonbase_archive_font( lua_State *s )
{
	ASSET *font;
	extern luaL_Reg moonbase_font_methods[];

	font = archive_load_font( luaL_checkstring(s, 1), luaL_checkinteger(s, 2) );
	luacom_create_object( s, "moonbase_font", &font, sizeof(font), moonbase_font_methods );
	return 1;
}

static int moonbase_archive_image( lua_State *s )
{
	ASSET *image;
	extern luaL_Reg moonbase_image_methods[];

	image = archive_load_image( luaL_checkstring(s, 1) );
	luacom_create_object( s, "moonbase_image", &image, sizeof(image), moonbase_image_methods );
	return 1;
}

static int moonbase_archive_sound( lua_State *s )
{
	ASSET *sound;
	extern luaL_Reg moonbase_sound_methods[];

	sound = archive_load_sound( luaL_checkstring(s, 1) );
	luacom_create_object( s, "moonbase_sound", &sound, sizeof(sound), moonbase_sound_methods );
	return 1;
}

static luaL_Reg moonarchive_handle_methods[] = {
	{ "font", moonbase_archive_font },
	{ "image", moonbase_archive_image },
	{ "sound", moonbase_archive_sound }
};

int moonbase_archive_initialize( lua_State *s )
{
	luaL_newlib( s, moonarchive_handle_methods );
	return 1;
}

