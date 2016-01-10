#include "moonbase.h"

void game_initialize( )
{
	extern int moonbase_initialize( lua_State * );

	moonbase_initialize( base_engine_state );
	if ( archive_contains(base_config_script) ) {
		archive_load_script( base_config_script );
	}
	video_start_window( );
	archive_load_script( base_main_script );
	if ( luacom_get_global_field(base_engine_state, "moonbase", "event", "initialize", NULL) ) {
		lua_call( base_engine_state, 0, 0 );
	}
	lua_getglobal( base_engine_state, "moonbase" );
	base_game_state = lua_newthread( base_engine_state );
	lua_setfield( base_engine_state, -2, "gameThread_" );
	lua_getglobal( base_game_state, "main" );
	lua_resume( base_game_state, base_engine_state, 0 );
}

void game_input( )
{
	SDL_Event e;

	while ( SDL_PollEvent(&e) ) {
		if ( e.type == SDL_QUIT ) {
			quit( 0 );
		}
		if ( !luacom_get_global_field(base_engine_state, "moonbase", "event", "input", NULL) ) {
			lua_settop( base_engine_state, 0 );
			return;
		}

		lua_createtable( base_engine_state, 0, 5 );
		switch( e.type ) {
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			luacom_write_table( base_engine_state, 2, "ssb",
				"type", "keyboard",
				"key", SDL_GetKeyName(e.key.keysym.sym),
				"pressed", (e.type == SDL_KEYDOWN)
			);
			break;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			luacom_write_table( base_engine_state, 2, "siiib",
				"type", "mouse",
				"button", (e.button.button == SDL_BUTTON_LEFT) ? 1 : (
					(e.button.button == SDL_BUTTON_MIDDLE) ? 2 : 3 ),
				"x", e.button.x,
				"y", e.button.y,
				"pressed", e.type == SDL_MOUSEBUTTONDOWN
			);
			break;
		case SDL_TEXTEDITING:
			luacom_write_table( base_engine_state, 2, "ssii",
				"type", "text edit",
				"text", e.edit.text,
				"start", e.edit.start,
				"length", e.edit.length
			);
			break;
		case SDL_TEXTINPUT:
			luacom_write_table( base_engine_state, 2, "ss",
				"type", "text input",
				"text", e.text.text
			);
			break;
		case SDL_MOUSEWHEEL:
			luacom_write_table( base_engine_state, 2, "ti",
				"type", "wheel",
				"direction", (e.wheel.y ? 1 : 2)
			);
			break;
		/*case SDL_MOUSEMOTION:
			luacom_write_table( base_engine_state, 2, "sii",
				"type", "motion",
				"xrel", e.motion.xrel,
				"yrel", e.motion.yrel
			);
			break;
		*/
		default:
			break;
		}
		lua_call( base_engine_state, 1, 0 );
		lua_settop( base_engine_state, 0 );
	}
}

void game_update( )
{
	if ( luacom_get_global_field(base_engine_state, "moonbase", "event", "update", NULL) ) {
		lua_pushinteger( base_engine_state, SDL_GetTicks() );
		lua_call( base_engine_state, 1, 0 );
	}
	lua_settop( base_engine_state, 0 );
}

void game_shutdown( )
{
	if ( luacom_get_global_field(base_engine_state, "moonbase", "event", "shutdown", NULL) ) {
		lua_call( base_engine_state, 0, 0 );
	}
	video_stop_window( );
	lua_settop( base_engine_state, 0 );
}
