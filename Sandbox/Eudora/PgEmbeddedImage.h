#ifndef _PG_EMBEDDED_IMAGE_H_
#define _PG_EMBEDDED_IMAGE_H_

#ifdef __cplusplus
extern "C" {
#endif
// put C++ stuff outside of this block

    void PgMultimediaInit( pg_globals_ptr pGlob );
    void PgMultimediaShutdown();
    void PgInsertImageFromFile( pg_ref pg, const char* pFullPathName );
    bool PgLoadUrlImage( paige_rec_ptr pg, pg_url_image_ptr pUrlImage );
    bool PgSaveUrlImage( paige_rec_ptr pg, pg_url_image_ptr pUrlImage );
    embed_ref PgNewUrlImageRef( pg_ref pg, const char* pUrl, short width,
				short height, const char* pAltString,
				pg_url_image_ptr pUrlImage,
				embed_callback callback );

    void PgInsertImageFromBitmap( pg_ref pg, HBITMAP hbm );
    unsigned PgValidateImageFile( const char* fullPath );
    bool PgGetOfnFilterString( char** );

#ifdef __cplusplus
}
#endif

#endif    // _PG_EMBEDDED_IMAGE_H_

