#include "moonbase.h"

VideoDisplay		*video_displays;
int			video_num_displays;

char			**video_drivers;
int			video_num_drivers;

SDL_Renderer	*video_renderer;
SDL_Window 	*video_window;

static struct {
	SDL_DisplayMode	mode;
	char		*title, *driver;
	Point		position;
	Size		size;
	Size		logical_size;
	int		grab_input, fullscreen, set_mode;
	double		brightness;
	SDL_Color	draw_color;
} video_options;


static void video_discover_drivers( )
{
	int i;

	video_num_drivers = SDL_GetNumVideoDrivers( );
	video_drivers = (char**)SDL_calloc( video_num_drivers, sizeof(char*) );
	for ( i = 0; i < video_num_drivers; ++i ) {
		video_drivers[ i ] = SDL_strdup( SDL_GetVideoDriver(i) );
	}
}

static void video_discover_displays( )
{
	int i, j, num_modes;
	Rectangle bounds;

	video_num_displays = SDL_GetNumVideoDisplays( );
	video_displays = (VideoDisplay*)SDL_calloc( video_num_displays, sizeof(VideoDisplay) );
	for ( i = 0; i < video_num_displays; ++i ) {
		SDL_GetDisplayBounds( i, (SDL_Rect*)&video_displays[i].location );
		//SDL_GetDisplayDPI( i, &video_displays[i].dpi[0], &video_displays[i].dpi[1], &video_displays[i].dpi[2] );
		video_displays[i].num_modes = SDL_GetNumDisplayModes( i );
		video_displays[i].modes = (VideoMode*)SDL_calloc( video_displays[i].num_modes, sizeof(VideoMode) );
		for ( j = 0; j < video_displays[i].num_modes; ++j ) {
			SDL_GetDisplayMode( i, j, (SDL_DisplayMode*)&video_displays[i].modes[j] );	
		}
	}
}

static void video_initialize_default_configuration( )
{
	video_options.mode.driverdata = NULL;
	video_options.set_mode = 0;
	video_options.title = SDL_strdup( " " );
	video_options.driver = SDL_strdup( SDL_GetVideoDriver(0) );
	video_options.position.x = SDL_WINDOWPOS_UNDEFINED;
	video_options.position.y = SDL_WINDOWPOS_UNDEFINED;
	video_options.size.w = 640;
	video_options.size.h = 480;
	video_options.logical_size.w = 640;
	video_options.logical_size.h = 480;
	video_options.grab_input = 0;
	video_options.fullscreen = 0;
	video_options.brightness = 1;
	video_renderer = NULL;
	video_window = NULL;
}

void video_initialize( )
{
	if ( !SDL_WasInit(SDL_INIT_VIDEO) ) {
		if ( SDL_InitSubSystem(SDL_INIT_VIDEO) ) {
			fatal( "%s", SDL_GetError() );
		}
	}
	video_initialize_default_configuration( );
	video_discover_drivers( );
	video_discover_displays( );
}

void video_stop_window( )
{
	if ( video_renderer != NULL ) {
		SDL_DestroyRenderer( video_renderer );
		video_renderer = NULL;
	}
	if ( video_window != NULL ) {
		SDL_DestroyWindow( video_window );
		video_window = NULL;
	}
	SDL_VideoQuit( );
}

void video_start_window( )
{
	if ( SDL_VideoInit(video_options.driver) ) {
		fatal( "%s", SDL_GetError() );
	}
	video_window = SDL_CreateWindow(
		video_options.title,
		video_options.position.x,
		video_options.position.y,
		video_options.size.w,
		video_options.size.h,
		SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI );
	if ( video_window == NULL ) {
		fatal( "%s", SDL_GetError() );
	}
	video_renderer = SDL_CreateRenderer( video_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
	if ( video_renderer == NULL ) {
		fatal( "%s", SDL_GetError() );
	}
	SDL_SetRenderDrawBlendMode( video_renderer, SDL_BLENDMODE_BLEND );
	SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" );
	SDL_RenderSetLogicalSize(
		video_renderer,
		video_options.logical_size.w,
		video_options.logical_size.h );
	SDL_SetWindowGrab( video_window, video_options.grab_input );
	if ( video_options.set_mode ) {
		SDL_SetWindowDisplayMode( video_window, &video_options.mode );
	} else {
		SDL_SetWindowDisplayMode( video_window, NULL );
	}
	SDL_SetWindowFullscreen( video_window, video_options.fullscreen );
}

void video_shutdown( )
{
	int i, j;

	SDL_QuitSubSystem( SDL_INIT_VIDEO );
	if ( video_options.driver != NULL ) {
		SDL_free( video_options.driver );
		video_options.driver = NULL;
	}
	if ( video_options.title != NULL ) {
		SDL_free( video_options.title );
		video_options.title = NULL;
	}
	if ( video_drivers != NULL ) {
		for ( i = 0; i < video_num_drivers; ++i ) {
			SDL_free( video_drivers[i] );
		}
		SDL_free( video_drivers );
		video_drivers = NULL;
	}
	if ( video_displays != NULL ) {
		for ( i = 0; i < video_num_displays; ++i ) {
			SDL_free( video_displays[i].modes );
		}
		SDL_free( video_displays );
		video_displays = NULL;
	}
}

int video_is_fullscreen( )
{
	return ( SDL_GetWindowFlags(video_window) & SDL_WINDOW_FULLSCREEN );
}

int video_is_input_grabbed( )
{
	return SDL_GetWindowGrab( video_window );	
}

double video_get_brightness( )
{
	return SDL_GetWindowBrightness( video_window );
}

const VideoDisplay *video_get_display( )
{
	return &video_displays[ SDL_GetWindowDisplayIndex(video_window) ];
}

const char *video_get_driver( )
{
	return SDL_GetCurrentVideoDriver( );
}

const Size *video_get_logical_size( )
{
	static Size size;

	SDL_RenderGetLogicalSize( video_renderer, &size.w, &size.h );
	return &size;
}

const VideoMode *video_get_mode( )
{
	static VideoMode mode;

	SDL_GetWindowDisplayMode( video_window, (SDL_DisplayMode*)&mode );
	return &mode;
}

const Point *video_get_position( )
{
	static Point point;

	SDL_GetWindowPosition( video_window, &point.x, &point.y );
	return &point;
}

const Size *video_get_size( )
{
	static Size size;

	SDL_GetWindowSize( video_window, &size.w, &size.h );
	return &size;
}

const char *video_get_title( )
{
	return SDL_GetWindowTitle( video_window );
}

void video_set_brightness( double brightness )
{
	video_options.brightness = brightness;
	if ( video_window != NULL ) {
		SDL_SetWindowBrightness( video_window, brightness );
	}
}

void video_set_draw_color( const char *color )
{
	unsigned int c;

	sscanf( color, "%x", &c );
	c = hton32 ( c );
	video_options.draw_color = *(SDL_Color*)&c;
	if ( video_renderer != NULL ) {
		SDL_SetRenderDrawColor( video_renderer,
			video_options.draw_color.r,
			video_options.draw_color.g,
			video_options.draw_color.b,
			video_options.draw_color.a );
	}
}

void video_set_driver( const char *driver_name )
{
	if ( video_options.driver != NULL ) {
		SDL_free( video_options.driver );
	}
	video_options.driver = SDL_strdup( driver_name );
}

void video_set_fullscreen( int fullscreen )
{
	video_options.fullscreen = ( fullscreen ? SDL_WINDOW_FULLSCREEN : 0 );
	if ( video_window != NULL ) {
		SDL_SetWindowFullscreen( video_window, video_options.fullscreen );
	}
}

void video_set_input_grabbed( int input_grabbed )
{
	video_options.grab_input = input_grabbed;
	if ( video_window != NULL ) {
		SDL_SetWindowGrab( video_window, input_grabbed );
	}
}

void video_set_logical_size( const Size *size )
{
	video_options.logical_size = *size;
	if ( video_renderer != NULL ) {
		SDL_RenderSetLogicalSize( video_renderer, size->w, size->h );
	}
}

void video_set_mode( const VideoMode *mode )
{
	video_options.mode = *(SDL_DisplayMode*)mode;
	video_options.set_mode = 1;
	if ( video_window != NULL ) {
		SDL_SetWindowDisplayMode( video_window, &video_options.mode );
	}
}

void video_set_position( const Point *position )
{
	video_options.position = *position;
	if ( video_window != NULL ) {
		SDL_SetWindowPosition( video_window, position->x, position->y );
	}
}

void video_set_size( const Size *size )
{
	video_options.size = *size;
	if ( video_window != NULL ) {
		SDL_SetWindowSize( video_window, size->w, size->h );
	}
}

void video_set_title( const char *title )
{
	if ( video_options.title != NULL ) {
		SDL_free( video_options.title );
	}
	video_options.title = SDL_strdup( title );
	if ( video_window != NULL ) {
		SDL_SetWindowTitle( video_window, title );
	}
}

static int moonbase_video_clear( lua_State *s )
{
	video_clear( );
	return 0;
}

static int moonbase_video_draw_line( lua_State *s )
{
	Point p1, p2;

	luacom_read_array( s, 1, "ii", 1, &p1.x, 2, &p1.y );
	luacom_read_array( s, 2, "ii", 1, &p2.x, 2, &p2.y );
	video_draw_line( &p1, &p2 );
	return 0;
}

static int moonbase_video_draw_rect( lua_State *s )
{
	Rectangle r;

	luacom_read_array( s, 1, "iiii", 1, &r.x, 2, &r.y, 3, &r.w, 4, &r.h );
	video_draw_rectangle( &r );
	return 0;
}

static int moonbase_video_fill_rect( lua_State *s )
{
	Rectangle r;

	luacom_read_array( s, 1, "iiii", 1, &r.x, 2, &r.y, 3, &r.w, 4, &r.h );
	video_fill_rectangle( &r );
	return 0;
}

static int moonbase_video_message_box( lua_State *s )
{
	const char *message;

	message = luaL_checkstring( s, 1 );
	video_show_messagebox( message );
	return 0;
}

static int moonbase_video_restart( lua_State *s )
{
	video_stop_window( );
	video_start_window( );
	return 0;
}

static int moonbase_video_is_fullscreen( lua_State *s )
{
	lua_pushboolean( s, video_is_fullscreen() );
	return 1;
}

static int moonbase_video_is_input_grabbed( lua_State *s )
{
	lua_pushboolean( s, video_is_input_grabbed() );
	return 1;
}

static int moonbase_video_get_brightness( lua_State *s )
{
	lua_pushnumber( s, video_get_brightness() );
	return 1;
}

static int moonbase_video_get_display( lua_State *s )
{
	const VideoDisplay *display;

	display = video_get_display( );
	luacom_get_global_field( s, "moonbase", "video", "displays", NULL );
	lua_rawgeti( s, -1, display - video_displays + 1 );
	return 1;	
}

static int moonbase_video_get_driver( lua_State *s )
{
	const char *driver_name;

	driver_name = video_get_driver( );
	lua_pushstring( s, driver_name );
	return 1;
}

static int moonbase_video_get_mode( lua_State *s )
{
	const VideoMode *mode;

	mode = video_get_mode( );
	lua_createtable( s, 0, 4 );
	luacom_write_table( s, 1, "uip",
		"format", mode->format,
		"refresh_rate", mode->refresh_rate,
		"opaque", mode->opaque );
	lua_createtable( s, 0, 2 );
	luacom_write_array( s, -1, "ii", mode->size.w, mode->size.h );
	lua_setfield( s, -2, "size" );
	return 1;
}

static int moonbase_video_get_position( lua_State *s )
{
	const Point *point;

	point = video_get_position(  );
	lua_createtable( s, 0, 2 );
	luacom_write_array( s, -1, "ii", 1, point->x, 2, point->y );
	return 1;
}

static int moonbase_video_get_size( lua_State *s )
{
	const Size *size;

	size = video_get_size(  );
	lua_createtable( s, 0, 2 );
	luacom_write_array( s, -1, "ii", 1, size->w, 2, size->h );
	return 1;
}

static int moonbase_video_get_title( lua_State *s )
{
	const char *title;

	title = video_get_title( );
	lua_pushstring( s, title );
	return 1;
}

static int moonbase_video_set_brightness( lua_State *s )
{
	video_set_brightness( luaL_checknumber(s, 1) );
	return 0;
}

static int moonbase_video_set_draw_color( lua_State *s )
{
	video_set_draw_color( luaL_checkstring(s, 1) );
	return 0;
}

static int moonbase_video_set_driver( lua_State *s )
{
	video_set_driver( luaL_checkstring(s, 1) );
	return 0;
}

static int moonbase_video_set_fullscreen( lua_State *s )
{
	video_set_fullscreen( luaL_checkint(s, 1) );
	return 0;
}

static int moonbase_video_set_input_grabbed( lua_State *s )
{
	video_set_input_grabbed( luaL_checkint(s, 1) );
	return 0;
}

static int moonbase_video_set_mode( lua_State *s )
{
	VideoMode mode;

	luacom_read_table( s, 1, "uip",
		"format", &mode.format,
		"refreshRate", &mode.refresh_rate,
		"opaque", &mode.opaque );
	lua_getfield( s, 1, "size" );
	luacom_read_array( s, -1, "ii", 1, &mode.size.w, 2, &mode.size.h );
	video_set_mode( &mode );
	return 0;
}

static int moonbase_video_set_position( lua_State *s )
{
	Point position;

	luacom_read_array( s, 1, "ii", 1, &position.x, 2, &position.y );
	video_set_position( &position );
	return 0;
}

static int moonbase_video_set_size( lua_State *s )
{
	Size size;

	luacom_read_array( s, 1, "ii", 1, &size.w, 2, &size.h );
	video_set_size( &size );
	return 0;
}

static int moonbase_video_set_title( lua_State *s )
{
	video_set_title( luaL_checkstring(s, 1) );
	return 0;
}

static luaL_Reg moonbase_video_methods[] = {
	/* Operations */
	{ "clear", moonbase_video_clear },
	{ "drawLine", moonbase_video_draw_line },
	{ "drawRect", moonbase_video_draw_rect }, 
	{ "fillRect", moonbase_video_fill_rect },
	{ "messageBox", moonbase_video_message_box },
	{ "restart", moonbase_video_restart },

	/* Accessors */
	{ "isFullscreen", moonbase_video_is_fullscreen },
	{ "isInputGrabbed", moonbase_video_is_input_grabbed },
	{ "getBrightness", moonbase_video_get_brightness },
	{ "getDisplay", moonbase_video_get_display },
	{ "getDriver", moonbase_video_get_driver },
	{ "getMode", moonbase_video_get_mode },
	{ "getPosition", moonbase_video_get_position },
	{ "getSize", moonbase_video_get_size },
	{ "getTitle", moonbase_video_get_title },

	/* Mutators */
	{ "setBrightness", moonbase_video_set_brightness },
	{ "setDrawColor", moonbase_video_set_draw_color },
	{ "setDriver", moonbase_video_set_driver },
	{ "setFullscreen", moonbase_video_set_fullscreen },
	{ "setInputGrabbed", moonbase_video_set_input_grabbed },
	{ "setMode", moonbase_video_set_mode },
	{ "setPosition", moonbase_video_set_position },
	{ "setSize", moonbase_video_set_size },
	{ "setTitle", moonbase_video_set_title },

	{ NULL, NULL }
};

static void moonbase_video_push_drivers_table( lua_State *s )
{
	int i;

	lua_createtable( s, 0, video_num_drivers );
	for ( i = 0; i < video_num_drivers; ++i ) {
		lua_pushstring( s, video_drivers[i] );
		lua_rawseti( s, -2, i+1 );
	}
}

static void moonbase_video_push_displays_table( lua_State *s )
{
	int i, j, n;
	VideoDisplay *disp;
	VideoMode *mode;

	lua_createtable( s, 0, video_num_displays );
	for ( i = 0; i < video_num_displays; ++i ) {
		disp = video_displays + i;
		lua_createtable( s, 0, 4 );
		lua_createtable( s, 0, 2 );
		luacom_write_array( s, -1, "ii",
			1, disp->location.x,
			2, disp->location.y );
		lua_setfield( s, -2, "location" );
		lua_createtable( s, 0, 2 );
		luacom_write_array( s, -1, "ii",
			1, disp->size.w,
			2, disp->size.h );
		lua_setfield( s, -2, "size" );
		/*lua_createtable( s, 0, 3 );
		luacom_write_array( s, -1, "nnn",
			1, disp->dpi[0],
			2, disp->dpi[1],
			3, disp->dpi[3] );
		lua_setfield( s, -2, "dpi" );*/
		lua_createtable( s, 0, disp->num_modes );
		for ( j = 0; j < disp->num_modes; ++j ) {
			mode = disp->modes + j;
			lua_createtable( s, 0, 4 );
			luacom_write_table( s, -1, "uip",
				"format", mode->format,
				"refreshRate", mode->refresh_rate,
				"opaque", mode->opaque );
			lua_createtable( s, 0, 2 );
			luacom_write_array( s, -1, "ii", 1, mode->size.w, 2, mode->size.h );
			lua_setfield( s, -2, "size" );
			lua_rawseti( s, -2, j+1 );
		}
		lua_setfield( s, -2, "modes" );
		lua_rawseti( s, -2, i+1 );
	}
}

int moonbase_video_initialize( lua_State *s )
{
	lua_newtable( s );
	luaL_setfuncs( s, moonbase_video_methods, 0 );
	moonbase_video_push_drivers_table( s );
	lua_setfield( s, -2, "drivers" );
	moonbase_video_push_displays_table( s );
	lua_setfield( s, -2, "displays" );
	return 1;
}
