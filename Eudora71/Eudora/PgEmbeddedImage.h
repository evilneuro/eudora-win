#ifndef _PG_EMBEDDED_IMAGE_H_
#define _PG_EMBEDDED_IMAGE_H_


#include "paige.h"
#include "pgEmbed.h"


// custom windows message handled by PaigeEdtView
extern const UINT umsgInvalidateImage;

// state info for background loading of image data
class FetchContext
{
public:
	unsigned uid;
	char* url;
	char* filePath;
	unsigned long getfile_thread;
	BOOL bTerminate;		// Signals that the thread is should exit and delete the fetch context itself
	HWND hNotify;
	BOOL bThreadDone;		// Signals that the thread is done, and fetch_url_schmookie should delete the fetch context
	BOOL bEudoraTempFile;
	CCriticalSection criticalSection;	// Used during thread destruction

public:
	FetchContext();
	~FetchContext();

	void terminate();

	static UINT getfile_thread_proc(LPVOID pParam);
};

#ifdef __cplusplus
extern "C" {
#endif
// put C++ stuff outside of this block

	bool init_quicktime();
	
    void PgMultimediaInit( pg_globals_ptr pGlob );
    void PgMultimediaShutdown();
    void PgInsertImageFromFile( pg_ref pg, const char* pFullPathName );
	bool PgInsertEmoticonImageFromFile(pg_ref pg, const char * szEmoticonImageFullPathName, const char * szEmoticonTrigger, long nPosition = CURRENT_POSITION);
    bool PgLoadUrlImage( paige_rec_ptr pg, pg_url_image_ptr pUrlImage, pg_embed_ptr embed, bool bAllowThreadedFetch );
    bool PgSaveUrlImage(paige_rec_ptr pg, pg_url_image_ptr pUrlImage, pg_embed_ptr pEmbed);
    embed_ref PgNewUrlImageRef( pg_ref pg, const char* pUrl, short width,
				short height, const char* pAltString,
				pg_url_image_ptr pUrlImage,
				embed_callback callback );

    void PgInsertImageFromBitmap( pg_ref pg, HBITMAP hbm );
    unsigned PgValidateImageFile( const char* fullPath, BOOL bDisplayFileErrors = TRUE );
    bool PgGetOfnFilterString( char** );

#ifdef __cplusplus
}
#endif

#endif    // _PG_EMBEDDED_IMAGE_H_

