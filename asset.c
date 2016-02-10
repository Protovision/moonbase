#include "moonbase.h"

static struct mht *asset_table;

void asset_free_entry( void *k, void *v )
{
	struct asset *ent;

	SDL_free( k );
	ent = (struct asset*)v;
	switch ( ent->type ) {
	case ASSET_FONT:
		TTF_CloseFont( ent->handle );
		break;
	case ASSET_IMAGE:
		SDL_DestroyTexture( ent->handle );
		break;
	case ASSET_SOUND:
		Mix_FreeChunk( ent->handle );
		break;
	}
	SDL_free( ent );
}

void asset_initialize( )
{
	asset_table = mht_strk_new( 128, asset_free_entry );
	if ( asset_table == NULL ) {
		fatal( "Failed to create asset table" );
	}
}

void asset_shutdown( )
{
	mht_free( asset_table );
}

void *asset_create( const char *path, void *handle, int asset_type )
{
	char buf[32];
	char *key;
	struct asset *ent;

	if ( path != NULL ) {
		key = SDL_strdup( path );
	} else {
		SDL_snprintf( buf, 32, "@%p", handle );
		key = SDL_strdup( buf );
	}
	ent = (struct asset*)SDL_malloc( sizeof(struct asset) );
	ent->type = asset_type;
	ent->handle = handle;
	ent->refcount = 1;
	mht_set( asset_table, key, ent, 1 );
	return key;
}

struct asset *asset_find( void *key )
{
	struct asset *ent;

	if ( mht_get(asset_table, key, (void**)&ent) ) return NULL;
	return ent;
}

void *asset_acquire( void *key )
{
	struct asset *ent;
	
	ent = asset_find( key );
	if ( ent == NULL ) return NULL;
	++asset_find( key )->refcount;
	return key;
}

void *asset_release( void *key )
{
	struct asset *ent;

	ent = asset_find( key );
	if ( --ent->refcount <= 0 ) {
		mht_del( asset_table, key );
		return NULL;
	}
	return key;
}

SDL_Texture *asset_image_handle( void *key )
{
	return (SDL_Texture*)asset_find(key)->handle;
}

TTF_Font *asset_font_handle( void *key )
{
	return (TTF_Font*)asset_find(key)->handle;
}

Mix_Chunk *asset_sound_handle( void *key )
{
	return (Mix_Chunk*)asset_find(key)->handle;
}





