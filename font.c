#include "moonbase.h"

void font_get_render_size( ASSET *font, const char *text, Size *size )
{
	int result;

	result = TTF_SizeText( asset_font_handle(font), text, &size->w, &size->h );
	if ( result == -1 ) {
		fatal( "%s", TTF_GetError() );
	}
}

ASSET *font_render_text( ASSET *font, const char *text, const char *color )
{
	ASSET *asset;
	SDL_Rect rectangle;
	SDL_Texture *texture;
	SDL_Surface *surface;
	SDL_Color sdl_color;
	unsigned int c;

	sscanf( color, "%x", &c );
	c = hton32( c );
	sdl_color = *(SDL_Color*)&c;
	surface = TTF_RenderText_Blended( asset_font_handle(font), text, sdl_color );
	if ( surface == NULL ) {
		fatal( "%s", TTF_GetError() );
	}
	texture = SDL_CreateTextureFromSurface( video_renderer, surface );
	if ( texture == NULL ) {
		fatal( "%s", SDL_GetError() );
	}
	SDL_FreeSurface( surface );
	return asset_create( texture, ASSET_IMAGE, NULL );
}

static int moonbase_font_render( lua_State *s )
{
	ASSET *font, *image;
	const char *text, *color;
	extern luaL_Reg moonbase_image_methods[];

	font = *(ASSET**)luaL_checkudata( s, 1, "moonbase_font" );
	text = luaL_checkstring( s, 2 );
	color = luaL_checkstring( s, 3 );
	image = font_render_text( font, text, color );
	luacom_create_object( s, "moonbase_image", &image, sizeof(image), moonbase_image_methods );
	return 1;
}

static int moonbase_font_get_render_size( lua_State *s )
{
	ASSET *font;
	const char *text;
	Size size;

	font = *(ASSET**)luaL_checkudata( s, 1, "moonbase_font" );
	text = luaL_checkstring( s, 2 );
	font_get_render_size( font, text, &size );
	lua_createtable( s, 0, 2 );
	luacom_write_table( s, -1, "ii", 1, size.w, 2, size.h );
	return 1;
}

static int moonbase_font_gc( lua_State *s )
{
	ASSET *font;

	font = *(ASSET**)luaL_checkudata( s, 1, "moonbase_font" );
	asset_release( font );
	return 0;
}

luaL_Reg moonbase_font_methods[] = {
	{ "render", moonbase_font_render },
	{ "getRenderSize", moonbase_font_get_render_size },
	{ "__gc", moonbase_font_gc },
	{ NULL, NULL }
};

int moonbase_font_initialize( lua_State *s )
{
	luaL_newlib( s, moonbase_font_methods );
	return 1;
}

static int moonbase_text_is_active( lua_State *s )
{
	lua_pushboolean( s, SDL_IsTextInputActive() );
	return 1;
}

static int moonbase_text_start( lua_State *s )
{
	SDL_StartTextInput( );
	return 0;
}

static int moonbase_text_stop( lua_State *s )
{
	SDL_StopTextInput( );
	return 0;
}

static int moonbase_text_set_rect( lua_State *s )
{
	Rectangle r;

	luacom_read_array( s, 1, "iiii",
		1, &r.x, 
		2, &r.y,
		3, &r.w,
		4, &r.h );
	SDL_SetTextInputRect( &r );
	return 0;
}

static luaL_Reg moonbase_text_methods[] = {
	{ "isActive", moonbase_text_is_active },
	{ "start", moonbase_text_start },
	{ "stop", moonbase_text_stop },
	{ "setRect", moonbase_text_set_rect },
	{ NULL, NULL }
};

int moonbase_text_initialize( lua_State *s )
{
	luaL_newlib( s, moonbase_text_methods );
	return 1;
}

