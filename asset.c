#include "moonbase.h"

static struct lh_table *asset_table;

void asset_free_entry( struct lh_entry *e )
{
	asset_entry *ent;

	SDL_free( e->k );
	ent = (asset_entry*)e->v;
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
	asset_table = lh_kchar_table_new( 64, "asset_table", asset_free_entry );
	if ( asset_table == NULL ) {
		fatal( "Failed to create asset table" );
	}
}

void asset_shutdown( )
{
	lh_table_free( asset_table );
}

ASSET *asset_create( void *handle, int asset_type, const char *key )
{
	char buf[32];
	char *kptr;
	asset_entry *ent;

	if ( key != NULL ) {
		kptr = SDL_strdup( key );
	} else {
		SDL_snprintf( buf, 32, "@%p", handle );
		kptr = SDL_strdup( buf );
	}
	ent = SDL_malloc( sizeof(asset_entry) );
	ent->type = asset_type;
	ent->handle = handle;
	ent->refcount = 1;
	lh_table_insert( asset_table, kptr, ent );
	return lh_table_lookup_entry( asset_table, kptr );
}

ASSET *asset_find( const char *key )
{
	return lh_table_lookup_entry( asset_table, key );
}

ASSET *asset_acquire( ASSET *asset )
{
	asset_entry *ent;

	ent = (asset_entry*)asset->v;
	++ent->refcount;
	return asset;
}

ASSET *asset_release( ASSET *asset )
{
	asset_entry *ent;

	ent = (asset_entry*)asset->v;
	if ( --ent->refcount <= 0 ) {
		lh_table_delete_entry( asset_table, asset );
		return NULL;
	}
	return asset;
}



