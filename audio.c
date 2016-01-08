#include "moonbase.h"

int audio_initialized;

static struct {
	int	frequency, audio_channels, samples, sound_channels;
	Uint16	format;
} audio_options;

void audio_initialize( )
{
	SDL_AudioSpec desired;

	log_info( "Initializing audio.\n" );
	if ( !SDL_WasInit(SDL_INIT_AUDIO) ) {
		if ( SDL_InitSubSystem(SDL_INIT_AUDIO) ) {
			fatal( SDL_GetError() );
		}
	}
	memset( &desired, 0, sizeof(desired) );
	desired.freq = audio_options.frequency;
	desired.format = audio_options.format;
	desired.channels = audio_options.audio_channels;
	desired.samples = audio_options.samples;
	if ( Mix_OpenAudio(desired.freq, desired.format, desired.channels, desired.samples) ) {
		fatal( Mix_GetError() );
	}
	Mix_Init( MIX_INIT_OGG | MIX_INIT_MP3 );
	if ( Mix_AllocateChannels(audio_options.sound_channels) < audio_options.sound_channels ) {
		fatal( Mix_GetError() );
	}
	Mix_Resume( -1 );
	audio_initialized = 1;
}

void audio_shutdown( )
{
	log_info( "Shutting down audio.\n" );
	Mix_Pause( -1 );
	Mix_CloseAudio( );
	SDL_AudioQuit( );
	SDL_QuitSubSystem( SDL_INIT_AUDIO );
	audio_initialized = 0;
}

int audio_get_audio_channels( )
{
	return audio_options.audio_channels;	
}

Uint16 audio_get_format( )
{
	return audio_options.format;
}

int audio_get_frequency( )
{
	return audio_options.frequency;
}

int audio_get_samples( )
{
	return audio_options.samples;
}

int audio_get_sound_channels( )
{
	return audio_options.sound_channels;
}

void audio_set_audio_channels( int channels )
{
	audio_options.audio_channels = channels;		
}

void audio_set_format( Uint16 format )
{
	audio_options.format = format;
}

void audio_set_frequency( int frequency )
{
	audio_options.frequency = frequency;
}

void audio_set_samples( int samples )
{
	audio_options.samples = samples;
}

void audio_set_sound_channels( int channels )
{
	audio_options.sound_channels = channels;
	if ( audio_initialized ) {
		Mix_AllocateChannels( channels );
	}
}

int sound_play( ASSET *sound, int fade_ms, int loop )
{
	if ( fade_ms != 0 ) {
		return Mix_FadeInChannel( -1, asset_sound_handle(sound), loop ? -1 : 0, fade_ms );
	} 
	return Mix_PlayChannel( -1, asset_sound_handle(sound), loop ? -1 : 0 );
}

static int moonbase_audio_pause( lua_State *s )
{
	audio_pause( );
	return 0;
}

static int moonbase_audio_resume( lua_State *s )
{
	audio_resume( );
	return 0;
}

static int moonbase_audio_stop( lua_State *s )
{
	int fade_ms;

	fade_ms = luaL_optint( s, 1, 0 );
	audio_stop( fade_ms );
	return 0;
}

static int moonbase_audio_restart( lua_State *s )
{
	audio_restart( );
	return 0;
}

static int moonbase_audio_get_audio_channels( lua_State *s )
{
	lua_pushinteger( s, audio_get_audio_channels() );
	return 1;
}

static int moonbase_audio_get_format( lua_State *s )
{
	lua_pushunsigned( s, audio_get_format() );
	return 1;
}

static int moonbase_audio_get_frequency( lua_State *s )
{
	lua_pushinteger( s, audio_get_frequency() );
	return 1;
}

static int moonbase_audio_get_samples( lua_State *s )
{
	lua_pushinteger( s, audio_get_samples() );
	return 1;
}

static int moonbase_audio_get_sound_channels( lua_State *s )
{
	lua_pushinteger( s, audio_get_sound_channels() );
	return 1;
}

static int moonbase_audio_get_volume( lua_State *s )
{
	lua_pushnumber( s, audio_get_volume() );
	return 1;
}

static int moonbase_audio_set_audio_channels( lua_State *s )
{
	audio_set_audio_channels( luaL_checkinteger(s, 1) );
	return 0;
}

static int moonbase_audio_set_format( lua_State *s )
{
	audio_set_format( luaL_checkunsigned(s, 1) );
	return 0;
}

static int moonbase_audio_set_frequency( lua_State *s )
{
	audio_set_frequency( luaL_checkinteger(s, 1) );
	return 0;
}

static int moonbase_audio_set_samples( lua_State *s )
{
	audio_set_samples( luaL_checkinteger(s, 1) );
	return 0;
}

static int moonbase_audio_set_sound_channels( lua_State *s )
{
	audio_set_sound_channels( luaL_checkinteger(s, 1) );
	return 0;
}

static int moonbase_audio_set_volume( lua_State *s )
{
	audio_set_volume( luaL_checknumber(s, 1) );
	return 0;
}

static luaL_Reg moonbase_audio_methods[] = {
	/* Operations */
	{ "pause", moonbase_audio_pause },
	{ "resume", moonbase_audio_resume },
	{ "stop", moonbase_audio_stop },
	{ "restart", moonbase_audio_restart },

	/* Accessors */
	{ "getAudioChannels", moonbase_audio_get_audio_channels },
	{ "getFormat", moonbase_audio_get_format },
	{ "getFrequency", moonbase_audio_get_frequency },
	{ "getSamples", moonbase_audio_get_samples },
	{ "getSoundChannels", moonbase_audio_get_sound_channels },
	{ "getVolume", moonbase_audio_get_volume },

	/* Mutators */
	{ "setAudioChannels", moonbase_audio_set_audio_channels },
	{ "setFormat", moonbase_audio_set_format },
	{ "setFrequency", moonbase_audio_set_frequency },
	{ "setSamples", moonbase_audio_set_samples },
	{ "setSoundChannels", moonbase_audio_set_sound_channels },
	{ "setVolume", moonbase_audio_set_volume }
};

int moonbase_audio_initialize( lua_State *s )
{
	luaL_newlib( s, moonbase_audio_methods );
	return 1;
}

static int moonbase_channel_pause( lua_State *s )
{
	int channel;

	channel = *(int*)luaL_checkudata( s, 1, "moonbase_channel" );
	channel_pause( channel );
	return 0;
}

static int moonbase_channel_resume( lua_State *s )
{
	int channel;

	channel = *(int*)luaL_checkudata( s, 1, "moonbase_channel" );
	channel_resume( channel );
	return 0;
}

static int moonbase_channel_stop( lua_State *s )
{
	int channel, fade_ms;

	channel = *(int*)luaL_checkudata( s, 1, "moonbase_channel" );
	fade_ms = luaL_optint( s, 2, 0 );
	channel_stop( channel, fade_ms );
	return 0;
}

static int moonbase_channel_get_volume( lua_State *s )
{
	int channel;
	
	channel = *(int*)luaL_checkudata( s, 1, "moonbase_channel" );
	lua_pushnumber( s, channel_get_volume(channel) );
	return 1;
}

static int moonbase_channel_set_volume( lua_State *s )
{
	int channel;
	float volume;

	channel = *(int*)luaL_checkudata( s, 1, "moonbase_channel" );
	volume = luaL_checknumber( s, 2 );
	channel_set_volume( channel, volume );
	return 0;
}

static luaL_Reg moonbase_channel_methods[] = {
	{ "pause", moonbase_channel_pause },
	{ "resume", moonbase_channel_resume },
	{ "stop", moonbase_channel_stop },
	{ "getVolume", moonbase_channel_get_volume },
	{ "setVolume", moonbase_channel_set_volume }
};

static int moonbase_sound_play( lua_State *s )
{
	ASSET *sound;
	int fade_ms, channel;

	sound = *(ASSET**)luaL_checkudata( s, 1, "moonbase_sound" );
	fade_ms = luaL_optint( s, 2, 0 );
	channel = sound_play( sound, fade_ms, 0 );
	luacom_create_object( s, "moonbase_channel", &channel, sizeof(channel), moonbase_channel_methods );
	return 1;
}

static int moonbase_sound_loop( lua_State *s )
{
	ASSET *sound;
	int channel, fade_ms;

	sound = *(ASSET**)luaL_checkudata( s, 1, "moonbase_sound" );
	fade_ms = luaL_optint( s, 2, 0 );
	channel = sound_play( sound, fade_ms, 1 );
	luacom_create_object( s, "moonbase_channel", &channel, sizeof(channel), moonbase_channel_methods );
	return 1;
}

static int moonbase_sound_gc( lua_State *s )
{
	ASSET *sound;

	sound = *(ASSET**)luaL_checkudata( s, 1, "moonbase_sound" );
	asset_release( sound );
	return 0;
}

luaL_Reg moonbase_sound_methods[] = {
	{ "play", moonbase_sound_play },
	{ "loop", moonbase_sound_loop },
	{ "__gc", moonbase_sound_gc }
};
