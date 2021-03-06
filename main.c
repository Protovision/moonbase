#include "moonbase.h"

void quit( int sig )
{
	if ( sig != -1 ) {
		game_shutdown( );
	}
	video_shutdown( );
	audio_shutdown( );
	base_shutdown( );
	exit( 0 );
}

void initialize( int argc, char *argv[] )
{
	base_initialize( argc, argv );
	audio_initialize( );
	video_initialize( );
	game_initialize( );
	signal( SIGINT, quit );
	signal( SIGTERM, quit );
}

void frame( )
{
	Uint32 start, elapsed, delay;

	start = SDL_GetTicks( );

	if ( base_resume_time && start >= base_resume_time ) {
		base_resume_time = 0;
		lua_settop( base_game_state, 0 );
		lua_resume( base_game_state, base_engine_state, 0 );
		return;
	}

	game_input( );
	game_update( );
	video_render( );

	delay = 1000 / base_fps;
	elapsed = SDL_GetTicks() - start;

	if ( elapsed < delay ) {
		SDL_Delay( delay - elapsed );
	}
}

int main( int argc, char *argv[] )
{
	initialize( argc, argv );
	for ( ;; ) {
		frame( );
	}

	return 0;
}
