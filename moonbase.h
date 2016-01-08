#ifndef BASE_H
#define BASE_H

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"
#include "unzip.h"
#include "luacom.h"
#include <talloc.h>
#include <json-c/linkhash.h>

#define MAX_STRING	32000
#define MAX_PATHNAME	256

typedef SDL_Point	Point;
typedef SDL_Rect	Rectangle;
typedef struct {
	int	w;
	int	h;
} Size;

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define hton32(X)	(SDL_Swap32(X))
#else
#define hton32(X)	(X)
#endif

char	*vstr( const char *fmt, ... );
char	*joinpath( const char *base, const char *p );

extern char log_critical_buffer[ 4096 ];

#define log_verbose( f, ... )	((void)SDL_LogVerbose( SDL_LOG_CATEGORY_APPLICATION, (f), ##__VA_ARGS__ ))
#define log_debug( f, ... )	((void)SDL_LogDebug( SDL_LOG_CATEGORY_APPLICATION, (f), ##__VA_ARGS__ ))
#define log_info( f, ... )	((void)SDL_LogInfo( SDL_LOG_CATEGORY_APPLICATION, (f), ##__VA_ARGS__ ))
#define log_warning( f, ... )	((void)SDL_LogWarn( SDL_LOG_CATEGORY_APPLICATION, (f), ##__VA_ARGS__ ))
#define log_error( f, ... )	((void)SDL_LogError( SDL_LOG_CATEGORY_ERROR, (f), ##__VA_ARGS__ ))
#define fatal( f, ... )	do { \
	SDL_snprintf( log_critical_buffer, 4096, (f), ##__VA_ARGS__ ); \
	SDL_LogCritical( SDL_LOG_CATEGORY_ERROR, "%s", log_critical_buffer ); \
	SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "Critical", log_critical_buffer, NULL ); \
	quit( -1 ); \
} while ( 0 )

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

void	base_initialize( int argc, char *argv[] );
void	base_shutdown( );

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

typedef struct lh_entry	ASSET;

enum {
	ASSET_FONT,
	ASSET_IMAGE,
	ASSET_SOUND
};

typedef struct {
	int	type;
	void	*handle;
	int	refcount;
} asset_entry;

void	asset_initialize( );
void	asset_shutdown( );
ASSET	*asset_create( void *handle, int asset_type, const char *key );
ASSET	*asset_find( const char *key );
ASSET	*asset_acquire( ASSET *asset );
ASSET	*asset_release( ASSET *asset );

#define		asset_get_handle( A )	(((asset_entry*)((A)->v))->handle)
#define		asset_image_handle( A )	((SDL_Texture*)asset_get_handle((A)))
#define		asset_font_handle( A )	((TTF_Font*)asset_get_handle((A)))
#define		asset_sound_handle( A )	((Mix_Chunk*)asset_get_handle((A)))

/***********************************************************
 * archive.c 
 **********************************************************/

void	archive_initialize( );
void	archive_shutdown( );
int	archive_contains( const char *file );
int	archive_load_script( const char *file );
ASSET	*archive_load_font( const char *file, int size );
ASSET	*archive_load_image( const char *file );
ASSET	*archive_load_sound( const char *file );

/***********************************************************
 * audio.c 
 **********************************************************/

void	audio_initialize( );
void	audio_shutdown( );

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

int	sound_play( ASSET *sound, int fade_ms, int loop );

#define	channel_get_volume(C)	((float)(Mix_Volume((C),-1)/MIX_MAX_VOLUME))
#define	channel_set_volume(C,V)	((void)(Mix_Volume((C),MIX_MAX_VOLUME*(V))))
#define	channel_stop(C,F)	((void)(((F) != 0.0) ? Mix_FadeOutChannel((C),(F)) : Mix_HaltChannel((C))))
#define	channel_pause(C)	((void)(Mix_Pause((C))))
#define	channel_resume(C)	((void)(Mix_Resume((C))))

/***********************************************************
 * video.c 
 **********************************************************/

typedef struct {
	Uint32	format;
	Size	size;
	int	refresh_rate;
	void	*opaque;
} VideoMode;

typedef struct {
	Point		location;
	Size		size;
	//double	dpi[3];
	VideoMode	*modes;
	int		num_modes;
} VideoDisplay;

extern VideoDisplay	*video_displays;
extern int		video_num_displays;
extern char		**video_drivers;
extern int		video_num_drivers;

extern SDL_Renderer	*video_renderer;
extern SDL_Window	*video_window;

void	video_initialize( );
void	video_shutdown( );
void	video_start_window( );
void	video_stop_window( );

#define video_restart( )		do { video_stop_window( ); video_start_window( ); } while ( 0 )
#define	video_clear( )			((void)(SDL_RenderClear(video_renderer)))
#define	video_draw_line(P,Q)		((void)(SDL_RenderDrawLine(video_renderer,(P)->x,(P)->y,(Q)->x,(Q)->y)))
#define	video_draw_rectangle(R)		((void)(SDL_RenderDrawRect(video_renderer,(R))))
#define	video_fill_rectangle(R)		((void)(SDL_RenderFillRect(video_renderer,(R))))
#define video_render( )			((void)(SDL_RenderPresent(video_renderer)))
#define video_show_messagebox(M)	((void)(SDL_ShowSimpleMessageBox(0,"",(M),NULL)))

int			video_is_fullscreen( );
int			video_is_input_grabbed( );
double			video_get_brightness( );
const VideoDisplay	*video_get_display( );
const char		*video_get_driver( );
const Size		*video_get_logical_size( );
const VideoMode		*video_get_mode( );
const Point		*video_get_position( );
const Size		*video_get_size( );
const char		*video_get_title( );

void	video_set_brightness( double brightness );
void	video_set_draw_color( const char *color );
void	video_set_driver( const char *driver_name );
void	video_set_fullscreen( int fullscreen );
void	video_set_input_grabbed( int input_grabbed );
void	video_set_logical_size( const Size *size );
void	video_set_mode( const VideoMode *mode );
void	video_set_position( const Point *position );
void	video_set_size( const Size *size );
void	video_set_title( const char *title );

/***********************************************************
* font.c 
**********************************************************/

void	font_get_render_size( ASSET *font, const char *text, Size *size );
ASSET	*font_render_text( ASSET *font, const char *text, const char *color );

/***********************************************************
 * image.c 
 **********************************************************/

void	image_draw( const Rectangle *destination, ASSET *image );
void	image_draw_background( ASSET *image );
void	image_draw_clip( const Rectangle *destination, ASSET *image, const Rectangle *source );
void	image_get_size( ASSET *image, Size *size );
float	image_get_alpha( ASSET *image );
void	image_set_alpha( ASSET *image, float alpha );

/***********************************************************
 * storage.c 
 **********************************************************/

FILE	*storage_open( const char *path, const char *mode );
int	storage_rename( const char *oldpath, const char *newpath );
int	storage_remove( const char *path );


#endif
