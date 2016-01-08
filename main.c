#include "moonbase.h"

void quit( int sig )
{
	log_info( "Preparing to quit\n" );
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
	log_info( "Starting initialization\n" );
	base_initialize( argc, argv );
	audio_initialize( );
	video_initialize( );
	game_initialize( );

	signal( SIGTERM, quit );
	signal( SIGINT, quit );
}

void frame( )
{
	Uint32 start, elapsed, delay;

	start = SDL_GetTicks( );

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
	log_info( "Running game loop\n" );
	for ( ;; ) {
		frame( );
	}

	return 0;
}
