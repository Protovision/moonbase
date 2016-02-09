#ifndef BASE_H
#define BASE_H

#include <signal.h>
#ifndef LUA_COMPAT_APIINTCASTS
#define LUA_COMPAT_APIINTCASTS
#endif
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#if defined(__APPLE__)
#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>
#include <SDL2_mixer/SDL_mixer.h>
#include <SDL2_ttf/SDL_ttf.h>
#elif defined(__linux__)
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include "SDL_ttf.h"
#endif
#include "unzip.h"
#include "talloc.h"
#include "mht.h"

#include "luacom.h"

#define MAX_STRING	32000
#define MAX_PATHNAME	256

struct point { int x, y; };
struct rectangle { int x, y, w, h; };
struct size { int w, h; };

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define hton32(X)	(SDL_Swap32(X))
#else
#define hton32(X)	(X)
#endif

/***********************************************************
 * log.c 
 **********************************************************/

enum {
	LOG_CRITICAL = 1,
	LOG_ERROR,
	LOG_WARN,
	LOG_INFO,
	LOG_DEBUG,
	LOG_VERBOSE
};

int	log_get_verbosity( );
void	log_set_verbosity( int level );
void	log_increment_verbosity( );
void	log_decrement_Verbosity( );
void	log_printf( int level, const char *fmt, ... );

extern char fatal_buffer[ 4096 ];
#define fatal( f, ... )	do { \
	SDL_snprintf( fatal_buffer, 4096, (f), ##__VA_ARGS__ ); \
	log_printf( LOG_CRITICAL, "%s", fatal_buffer ); \
	SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "Critical", fatal_buffer, NULL ); \
	quit( -1 ); \
} while ( 0 )

// main.c
void	quit( int sig );

/***********************************************************
 * base.c 
 **********************************************************/

extern int		base_pool_size;
extern char		*base_game_path;
extern char		*base_data_path;
extern char		*base_main_script;
extern char		*base_config_script;
extern TALLOC_CTX	*base_pool;
extern lua_State	*base_engine_state;
extern lua_State	*base_game_state;
extern unzFile		base_archive;
extern int		base_fps;
extern Uint32		base_resume_time;

void	base_initialize( int argc, char *argv[] );
void	base_shutdown( );

char	*vstr( const char *fmt, ... );
char	*joinpath( const char *base, const char *p );


/***********************************************************
 * game.c 
 **********************************************************/

void	game_initialize( );
void	game_shutdown( );
void	game_input( );
void	game_update( );

/***********************************************************
 * asset.c 
 **********************************************************/

enum {
	ASSET_FONT,
	ASSET_IMAGE,
	ASSET_SOUND
};

struct asset {
	int	type;
	void	*handle;
	int	refcount;
};
struct asset	*asset_find( void *key );

void		asset_initialize( );
void		asset_shutdown( );
void		*asset_create( const char *path, void *handle, int asset_type );
void		*asset_acquire( void *asset );
void		*asset_release( void *asset );
SDL_Texture	*asset_image_handle( void *asset );
TTF_Font	*asset_font_handle( void *asset );
Mix_Chunk	*asset_sound_handle( void *asset );

/***********************************************************
 * archive.c 
 **********************************************************/

void	archive_initialize( );
void	archive_shutdown( );
int	archive_contains( const char *file );
int	archive_load_script( const char *file );
void	*archive_load_font( const char *file, int size );
void	*archive_load_image( const char *file );
void	*archive_load_sound( const char *file );

/***********************************************************
 * audio.c 
 **********************************************************/

void	audio_initialize( );
void	audio_shutdown( );
void	audio_start_mixer( );
void	audio_stop_mixer( );

#define audio_restart( )	do { audio_shutdown( ); audio_initialize( ); } while ( 0 )
#define	audio_pause( )		((void)(Mix_Pause(-1)))
#define	audio_stop(F)		((void)(((F) != 0) ? Mix_FadeOutChannel(-1,(F)) : Mix_HaltChannel(-1)))
#define	audio_resume( )		((void)(Mix_Resume(-1)))
#define	audio_get_volume( )	((float)(Mix_Volume(-1,-1)/MIX_MAX_VOLUME))
#define	audio_set_volume(V)	((void)(Mix_Volume(-1,MIX_MAX_VOLUME*(V))))

int	audio_get_audio_channels( );
Uint16	audio_get_format( );
int	audio_get_frequency( );
int	audio_get_samples( );
int	audio_get_sound_channels( );

void	audio_set_audio_channels( int channels );
void	audio_set_format( Uint16 format );
void	audio_set_frequency( int frequency );
void	audio_set_samples( int samples );
void	audio_set_sound_channels( int channels );

float	channel_get_volume( int channel );
void	channel_stop( int channel, int fade );
void	channel_pause( int channel );
void	channel_resume( int channel );

int	sound_play( void *sound, int fade_ms, int loop );

#define	channel_get_volume(C)	((float)(Mix_Volume((C),-1)/MIX_MAX_VOLUME))
#define	channel_set_volume(C,V)	((void)(Mix_Volume((C),MIX_MAX_VOLUME*(V))))
#define	channel_stop(C,F)	((void)(((F) != 0.0) ? Mix_FadeOutChannel((C),(F)) : Mix_HaltChannel((C))))
#define	channel_pause(C)	((void)(Mix_Pause((C))))
#define	channel_resume(C)	((void)(Mix_Resume((C))))

/***********************************************************
 * video.c 
 **********************************************************/

struct video_mode {
	Uint32		format;
	struct size	size;
	int		refresh_rate;
	void		*opaque;
};

struct video_display {
	struct point		location;
	struct size		size;
	struct video_mode	*modes;
	int			num_modes;
};

extern struct video_display	*video_displays;
extern int			video_num_displays;
extern char			**video_drivers;
extern int			video_num_drivers;

extern SDL_Renderer	*video_renderer;
extern SDL_Window	*video_window;

void	video_initialize( );
void	video_shutdown( );
void	video_start_window( );
void	video_stop_window( );

#define video_restart( )		do { video_stop_window( ); video_start_window( ); } while ( 0 )
#define	video_clear( )			((void)(SDL_RenderClear(video_renderer)))
#define	video_draw_line(P,Q)		((void)(SDL_RenderDrawLine(video_renderer,(P)->x,(P)->y,(Q)->x,(Q)->y)))
#define	video_draw_rectangle(R)		((void)(SDL_RenderDrawRect(video_renderer,(const SDL_Rect*)(R))))
#define	video_fill_rectangle(R)		((void)(SDL_RenderFillRect(video_renderer,(const SDL_Rect*)(R))))
#define video_render( )			((void)(SDL_RenderPresent(video_renderer)))
#define video_show_messagebox(M)	((void)(SDL_ShowSimpleMessageBox(0,"",(M),NULL)))

int				video_is_fullscreen( );
int				video_is_input_grabbed( );
double				video_get_brightness( );
const struct video_display	*video_get_display( );
const char			*video_get_driver( );
const struct size		*video_get_logical_size( );
const struct video_mode		*video_get_mode( );
const struct point		*video_get_position( );
const struct size		*video_get_size( );
const char			*video_get_title( );

void	video_set_brightness( double brightness );
void	video_set_draw_color( const char *color );
void	video_set_driver( const char *driver_name );
void	video_set_fullscreen( int fullscreen );
void	video_set_input_grabbed( int input_grabbed );
void	video_set_logical_size( const struct size *size );
void	video_set_mode( const struct video_mode *mode );
void	video_set_position( const struct point *position );
void	video_set_size( const struct size *size );
void	video_set_title( const char *title );

/***********************************************************
* font.c 
**********************************************************/

void	font_get_render_size( void *font, const char *text, struct size *size );
void	*font_render_text( void *font, const char *text, const char *color );

/***********************************************************
 * image.c 
 **********************************************************/

void	image_draw( const struct rectangle *destination, void *image );
void	image_draw_background( void *image );
void	image_draw_clip( const struct rectangle *destination, void *image, const struct rectangle *source );
void	image_get_size( void *image, struct size *size );
float	image_get_alpha( void *image );
void	image_set_alpha( void *image, float alpha );

/***********************************************************
 * storage.c 
 **********************************************************/

FILE	*storage_open( const char *path, const char *mode );
int	storage_rename( const char *oldpath, const char *newpath );
int	storage_remove( const char *path );


#endif
