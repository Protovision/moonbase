#include "moonbase.h"

void image_draw( const struct rectangle *dst, void *image )
{
	SDL_Texture *texture;
	struct rectangle real_dst;

	texture = asset_image_handle( image );
	memcpy( &real_dst, dst, sizeof(real_dst) );
	if ( dst->w == 0 && dst->h == 0 ) {
		SDL_QueryTexture( texture, NULL, NULL, &real_dst.w, &real_dst.h );
	}
	SDL_RenderCopy( video_renderer, texture, NULL, (SDL_Rect*)&real_dst );
}

void image_draw_background( void *image )
{
	SDL_RenderCopy( video_renderer, asset_image_handle(image), NULL, NULL );
}

void image_draw_clip( const struct rectangle *dst, void *image, const struct rectangle *src )
{
	struct rectangle real_dst;

	memcpy( &real_dst, dst, sizeof(real_dst) );
	if ( dst->w == 0 && dst->h == 0 ) {
		real_dst.w = src->w;
		real_dst.h = src->h;
	}
	SDL_RenderCopy( video_renderer, asset_image_handle(image), (const SDL_Rect*)src, (SDL_Rect*)&real_dst );
}

void image_get_size( void *image, struct size *size )
{
	Uint32 format;
	int access;

	SDL_QueryTexture( asset_image_handle(image), &format, &access, &size->w, &size->h );
}

float image_get_alpha( void *image )
{
	Uint8 alpha;

	SDL_GetTextureAlphaMod( asset_image_handle(image), &alpha );
	return ( alpha / 255.0f );
}

void image_set_alpha( void *image, float alpha )
{
	Uint8 alpha_int;

	alpha_int = (Uint8)( 255.0f * alpha );
	SDL_SetTextureAlphaMod( asset_image_handle(image), alpha_int );
}

static int moonbase_image_draw( lua_State *s )
{
	void *image;
	struct rectangle r;

	image = *(void**)luaL_checkudata( s, 1, "moonbase_image" );
	luacom_read_array( s, 2, "ii", 1, &r.x, 2, &r.y );
	lua_len( s, 2 );
	if ( lua_tointeger(s, -1) == 4 ) {
		luacom_read_array( s, 2, "ii", 3, &r.w, 4, &r.h );
	} else {
		r.w = 0;
		r.h = 0;
	}
	image_draw( &r, image );
	return 0;
}

static int moonbase_image_draw_background( lua_State *s )
{
	void *image;

	image = *(void**)luaL_checkudata( s, 1, "moonbase_image" );
	image_draw_background( image );
	return 0;
}

static int moonbase_image_draw_clip( lua_State *s )
{
	void *image;
	struct rectangle src, dst;

	image = *(void**)luaL_checkudata( s, 1, "moonbase_image" );
	luacom_read_array( s, 2, "ii", 1, &dst.x, 2, &dst.y );
	lua_len( s, 2 );
	if ( lua_tointeger(s, -1) == 4 ) {
		luacom_read_array( s, 2, "ii", 3, &dst.w, 4, &dst.h );
	} else {
		dst.w = 0;
		dst.h = 0;
	}
	luacom_read_array( s, 3, "ii", 1, &src.x, 2, &src.y );
	lua_len( s, 3 );
	if ( lua_tointeger(s, -1) == 4 ) {
		luacom_read_array( s, 3, "ii", 3, &src.w, 4, &src.h );
	} else {
		src.w = 0;
		src.h = 0;
	}
	image_draw_clip( &dst, image, &src );
	return 0;
}

static int moonbase_image_get_size( lua_State *s )
{
	void *image;
	struct size size;

	image = *(void**)luaL_checkudata( s, 1, "moonbase_image" );
	image_get_size( image, &size );
	lua_createtable( s, 0, 2 );
	luacom_write_array( s, -1, "ii", 1, size.w, 2, size.h );
	return 1;
}

static int moonbase_image_get_alpha( lua_State *s )
{
	void *image;
	float alpha;

	image = *(void**)luaL_checkudata( s, 1, "moonbase_image" );
	alpha = image_get_alpha( image );
	lua_pushnumber( s, alpha );
	return 1;
}

static int moonbase_image_set_alpha( lua_State *s )
{
	void *image;
	float alpha;

	image = *(void**)luaL_checkudata( s, 1, "moonbase_image" );
	alpha = luaL_checknumber( s, 2 );
	image_set_alpha( image, alpha );
	return 0;
}

static int moonbase_image_gc( lua_State *s )
{
	void *image;

	image = *(void**)luaL_checkudata( s, 1, "moonbase_image" );
	asset_release( image );
	return 0;
}

luaL_Reg moonbase_image_methods[] = {
	{ "draw", moonbase_image_draw },
	{ "drawBackground", moonbase_image_draw_background },
	{ "drawClip", moonbase_image_draw_clip },
	{ "getSize", moonbase_image_get_size },
	{ "getAlpha", moonbase_image_get_alpha },
	{ "setAlpha", moonbase_image_set_alpha },
	{ "__gc", moonbase_image_gc },
	{ NULL, NULL }
};

int moonbase_image_initialize( lua_State *s )
{
	luaL_newlib( s, moonbase_image_methods );
	return 1;
}
