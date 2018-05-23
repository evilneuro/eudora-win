// PgEmbeddedImage.cpp
//
// Routines for embedding "url-images". These inline graphics are a
// specialization of an ordinary embedded image object. Originally, Paige only
// used "url-images" for images created by importing an html file, but sanity
// dictates that they be used exclusively for embedded image support in
// Eudora. (Take my word for it.)
//
// Copyright (c) 1997-2000 by QUALCOMM, Incorporated
/* Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. */

//

// windows
#include "stdafx.h"

#include "afxinet.h"
#include "resource.h"
#include "process.h"

// QuickTime
#include "qtwrapper.h"

#include "eudora.h"

// standard Paige includes
#include "Paige.h"
#include "pgTraps.h"
#include "pgOSUtl.h"
#include "pgUtils.h"
#include "pgTxrCPP.h"
#include "defprocs.h"
#include "pgErrors.h"
#include "pgDefStl.h"
#include "pgHTMDef.h"
#include "pgEmbed.h"
#include "pgdeftbl.h"
#include "machine.h"
#include "pgTables.h"
#include "pgHText.h"
#include "pgFrame.h"

// editor protocols
#include "qcprotocol.h"

// Eudora utilities
//#include "guiutils.h"
#include "fileutil.h"
#include "HTMLUtils.h"
#include "qcgraphics.h"

// mime storage objects
#include "MimeStorage.h"

// this module's public face.
#include "PgEmbeddedImage.h"

// plain ol' butt-ugly spaghetti
#include "PgStuffBucket.h"


#include "DebugNewHelpers.h"


///////////////////////////////////////////////////////////////////////////////
// private parts

static PG_PASCAL (long) eCallback( paige_rec_ptr pg, pg_embed_ptr embed_ptr,
        long embed_type, short command, long user_refcon, long param1, long param2 );

static long image_type( pg_char_ptr filename );
static pg_boolean test_extension( pg_char_ptr file_ext, pg_char_ptr match );
static void make_new_part( QCMessage* message, pg_url_image_ptr pUrlImage,
                                        long type, char* cid );
static void make_new_part_from_file(QCMessage* message, char* filepath, char* cid, bool bIsEmoticon);
static void clean_filepath( char* in, char* out );
static void make_filepath_url( const char* in, char* out );
static bool is_filepath_url( char* filepath );
static void make_it_schmookie( QCMessage* message, pg_url_image_ptr pUrlImage,
                                                        char* newCid );

static HBITMAP bitmap_from_url_image( pg_url_image_ptr pUrlImage );
static bool url_image_from_bitmap( HBITMAP hbm, pg_url_image_ptr pUrlImage );
static void altstring_schmookerdoodle( QCMessage* pM,
                                       pg_url_image_ptr pUrlImage );

static int fetch_url_schmookie( FetchContext** pfq, char* url, char* filePath, HWND hNotify, BOOL & bEudoraTempFile );
static unsigned int hash_string(const char* pszStr);
static bool find_me( pg_embed_ptr pe, long* index, long* position );

static bool g_qtInitialized = false;
static const char* local_file_spec = "file://";
static unsigned long g_uniqueId = 0;


///////////////////////////////////////////////////////////////////////////////
// Initialization

// PgMultimediaInit - this was intended to be called once to set up the Paige
// embed stuff, and initialize quicktime, but for some reason initializing the
// embed stuff is working better when called by every Paige instance, and we've
// decided not to initlialize the qt stuff until the last moment.

void PgMultimediaInit( pg_globals_ptr pGlob )
{
    pgInitEmbedProcs( pGlob, eCallback, NULL );
}


// PgMultimediaShutdown - terminates the quicktime libraries. this is called
// once during the programs lifetime, in ExitInstance (eudora.cpp).

void PgMultimediaShutdown()
{
    if ( g_qtInitialized ) {
        QTW_Shutdown();
    }
}


// init_quicktime - this is a one-time initialization of the quicktime
// libraries. it is called, first thing, in a few places, which allows us to
// do away with initializing quicktime in Eudora's InitInstance method.
//
// Note: failing this step should not cause any code to behave badly, Eudora
// must keep running, sans images.

bool init_quicktime()
{
    if ( !g_qtInitialized ) {
        g_qtInitialized = QTW_Startup();
    }

    return g_qtInitialized;
}


///////////////////////////////////////////////////////////////////////////////
// Images

// PgInsertImageFromFile - creates and inserts an url_image ref from a disk-
// based file. does not require quicktime, but will handle many more formats
// if quicktime is present.

void PgInsertImageFromFile( pg_ref pg, const char* pFullPathName ) 
{
    char pathNameUrl[MAX_PATH] = "";
    make_filepath_url( pFullPathName, pathNameUrl );
    embed_ref er = PgNewUrlImageRef( pg, pathNameUrl, 0, 0, NULL,
                                     NULL, eCallback );

    pgInsertEmbedRef( pg, er, CURRENT_POSITION, 0, eCallback,
                      0, draw_none );
}


// PgInsertEmoticonImageFromFile - creates and inserts an emoticon url_image
// ref from a disk-based file. Does not require quicktime, but will handle
// many more formats if quicktime is present.

bool PgInsertEmoticonImageFromFile(pg_ref pg, const char * szEmoticonImageFullPathName, const char * szEmoticonTrigger, long nPosition) 
{
    bool	bEmoticonInsertionSucceeded = false;
	
	char	szPathNameUrl[MAX_PATH] = "";
    make_filepath_url(szEmoticonImageFullPathName, szPathNameUrl);

	// Create the URL image with the emoticon trigger as the alt string
    embed_ref	er = PgNewUrlImageRef( pg, szPathNameUrl, 0, 0, szEmoticonTrigger,
									   NULL, eCallback );

	ASSERT(er);
	if (er)
	{
		pg_embed_ptr	pEmbed = reinterpret_cast<pg_embed_ptr>( UseMemory(er) );

		if (pEmbed)
		{
			// Mark the embedded image as an emoticon so that when we save/export
			// HMTL we recognize it and save it as the original trigger text.
			pEmbed->type |= EMBED_EMOTICON_FLAG;

			UnuseMemory(er);
		}

		// Save the selection (unless told to use current position) and set the
		// selection to be the insertion position, because pgInsertEmbedRef will
		// delete the current selection and we don't want that (we specifically
		// do our own deletion when we do want to replace the selection).
		select_pair		currentSelection = { 0, 0 };

		if (nPosition != CURRENT_POSITION)
		{
			pgGetSelection( pg, &currentSelection.begin, &currentSelection.end );

			pgSetSelection(pg, nPosition, nPosition, 0, FALSE);
		}
		
		// pgInsertEmbedRef doesn't return success or failure. It really returns
		// whether or not any drawing was done. Since we used draw_none it will
		// always return 0, so we just ignore its return result.
		pgInsertEmbedRef(pg, er, nPosition, 0, eCallback, 0, draw_none);

		if (nPosition != CURRENT_POSITION)
			pgSetSelection( pg, currentSelection.begin, currentSelection.end, 0, FALSE);
		
		// We got this far, assume success.
		bEmoticonInsertionSucceeded = true;
	}

	return bEmoticonInsertionSucceeded;
}


// PgInsertImageFromBitmap - creates and inserts an url_image embed ref from
// a windows bitmap. does not require quicktime.

void PgInsertImageFromBitmap( pg_ref pg, HBITMAP hbm )
{
    // create a metafile-based url_image from the bitmap
    pg_url_image urlImage;
    bool copasetic = url_image_from_bitmap( hbm, &urlImage );

    // turn our url_image into an embed ref and insert the bastard
    if ( copasetic ) {
        // TODO: need to change this routine to either create the embed from
        // various parameters, or from a supplied url_image
        embed_ref er = PgNewUrlImageRef( pg, NULL, 0, 0, NULL,
                                         &urlImage, eCallback );

        pgInsertEmbedRef( pg, er, CURRENT_POSITION, 0, eCallback,
                          0, draw_none );
    }
}



static PG_PASCAL (long) eCallback( paige_rec_ptr pg,
                        pg_embed_ptr embed_ptr, long embed_type, short command,
                                   long user_refcon, long param1, long param2 ) {

    if ((embed_type == embed_url_image) /* g_qtInitialized*/ ) {
        pg_url_image_ptr image;
        image = (pg_url_image_ptr) param1;

        switch( command ) {
        case EMBED_PREPARE_IMAGE:
            if ( !(image->source_width && image->source_height) ) {
                if ( !image->image_data ) {
					// Don't allow threaded fetch, because we're being called with a temporary embed_ptr
                    if ( PgLoadUrlImage( pg, image, embed_ptr, false ) )
                        return 0;
					else
						image->loader_result = 0xFFFF;
                }
            }

            break;

        case EMBED_LOAD_IMAGE:
            if ( !image->image_data ) {
				// Allow threaded fetch if necessary
                if ( PgLoadUrlImage( pg, image, embed_ptr, true ) )
                    return 0;
				else
					image->loader_result = 0xFFFF;
            }

                break;

        case EMBED_SAVE_IMAGE:
            if ( PgSaveUrlImage(pg, image, embed_ptr) )
                return 0;

            break;

		case EMBED_DRAW:
			{
				CDC* pDC = CDC::FromHandle((HDC)pg->port.machine_ref);
				CSafetyPalette Palette;
				Palette.Select(pDC->GetSafeHdc());
				Palette.Realize(pDC->GetSafeHdc());
				long result = pgDefaultEmbedCallback( pg, embed_ptr, embed_type, command,
													  user_refcon, param1, param2 );
				Palette.Unselect();
				return result;
			}
			break;

		case EMBED_DESTROY:
			{
				FetchContext* pfc = (FetchContext*) embed_ptr->user_refcon;

				if ( pfc ) {
					// Signal the thread that it should terminate itself and delete its FetchContext
					pfc->terminate();

					// this deletes itself, the pointer is wild
					embed_ptr->user_refcon = (long) (pfc = 0);
				}
			}
			break;
		}
    }


    // If not handled above then call the default:
    return pgDefaultEmbedCallback( pg, embed_ptr, embed_type, command,
                                   user_refcon, param1, param2 );
}


bool PgLoadUrlImage( paige_rec_ptr pg, pg_url_image_ptr pUrlImage, pg_embed_ptr embed,bool bAllowThreadedFetch )
{
    bool fRet = false;
	BOOL bEudoraTempFile = FALSE;

    // The Way It Is:
    //
    // if the url is cid, we get a file path from the mime store, otherwise we
    // use the wininet 'OpenURL' routine to get the file---if the file is not
    // local we suck it over the wire first. once we have a full local path to
    // the file, we use either quicktime or stringray to convert the image into
    // metafile format, which Paige supports natively.

    if ( pUrlImage && *pUrlImage->URL ) {
        char filePath[MAX_PATH] = "";
        PgStuffBucket* pSB = (PgStuffBucket*) pg->user_refcon;
        char* purl = (char*) pUrlImage->URL;

        if ( is_filepath_url(purl) )
		{
            // Handle both file:// prefix and file:/// prefix
			const char *	szAfterLocalFileSpec = purl + strlen(local_file_spec);

			if (*szAfterLocalFileSpec == '/')
				szAfterLocalFileSpec++;
			
			strcpy(filePath, szAfterLocalFileSpec);
		}
		else if ( pSB && (pSB->kind == PgStuffBucket::kMessage) && (pSB->subkind != PgStuffBucket::kPreviewMessage) )
		{
			//	Here we resolve the URI and clean up the resulting file path.
			//
			//	Preview messages won't hit this code (see subkind check above),
			//	because they already have their URI's resolved earlier
			//	(of necessity - multiple concentrated messages wouldn't have
			//	a *single* pointer to a loaded message available, and preview
			//	display no longer keeps any messages loaded anyway).
			//
			//	For any view type other than a preview message we have a
			//	pointer to the loaded message in pSB->pMessage. We resolve
			//	the CID on demand so that saving a message can avoid problems
			//	like creating a redundant new CID for saved embeded images.
			QCMessage* pMsg = pSB->pMessage;
			if (pMsg)
			{
				CString srcURI = pUrlImage->URL;
				char tempfilePath[MAX_PATH] = "";
				
				pMsg->GetPartAsFile( srcURI, tempfilePath, sizeof(tempfilePath) );
				clean_filepath( (char*) tempfilePath, filePath );
			}
		}

        if ( !(*filePath) )
        {
            if (bAllowThreadedFetch)
			{
				// ok, we failed to find it in our "mime store", so we let wininet
				// have a poke. this gets us http, ftp, gopher, etc.
				if ( !is_filepath_url( purl ) )
					unescape_url( purl );

				HWND hNotify = NULL;
				
				if (pSB && pSB->pWndOwner)
					hNotify = pSB->pWndOwner->GetSafeHwnd();

				if ( fetch_url_schmookie( (FetchContext**)&embed->user_refcon,
						purl, filePath, hNotify, bEudoraTempFile ) != -1 ) {

					fRet = true;   // keep trying
				}
			}
			else
			{
				// We're not allowed to do a threaded fetch now (because we're
				// being called during EMBED_PREPARE_IMAGE), but we *do* want
				// to keep on trying (so that we get called later when we
				// will allow the threaded fetch).
				fRet = true;
			}
        }

        if ( *filePath )
        {
			fRet = false;

            if (CanHandleImageInternally(filePath, FALSE))
            {
                QCMetaFileInfo mfi;
				int TransparencySysColor = COLOR_3DFACE;
				if (pSB && pSB->kind != PgStuffBucket::kAd)
					TransparencySysColor = COLOR_WINDOW;
                if (MetafileFromImage( filePath, &mfi, TransparencySysColor))
                {
                    if ( !pUrlImage->source_width )
                        pUrlImage->source_width = (short)mfi.width;
                    if ( !pUrlImage->source_height )
                        pUrlImage->source_height = (short)mfi.height;

                    pUrlImage->type_and_flags = embed_meta_file;
                    pUrlImage->image_data = (long)mfi.hm;
                    fRet = true;
                }
            }

            if ( !fRet && init_quicktime())
            {
                // convert image to a metafile format, and hand it to paige
                HMETAFILE hMeta = NULL;
                int cx = pUrlImage->source_width;
                int cy = pUrlImage->source_height;

                QTW_MetafileFromQtImage( filePath, &hMeta, &cx, &cy );

                if ( hMeta )
                {
                    pUrlImage->source_width = (short)cx;
                    pUrlImage->source_height = (short)cy;
                    pUrlImage->type_and_flags = embed_meta_file;
                    pUrlImage->image_data = (long)hMeta;
                    fRet = true;
                }
            }

			// Did the original source fail to specify the dimensions?
			if ( (embed->height == 0) || (embed->width == 0) )
			{
				// Update the embedded image dimensions
				embed->uu.pict_data.pict_frame.top_left.h = 0;
				embed->uu.pict_data.pict_frame.top_left.v = 0;

				embed->width = pUrlImage->source_width;
				embed->uu.pict_data.pict_frame.bot_right.h = pUrlImage->source_width;

				embed->height = pUrlImage->source_height;
				embed->uu.pict_data.pict_frame.bot_right.v = pUrlImage->source_height;

				// Search for ourselves since that seems to be the only way to get our
				// index position (needed for pgInvalEmbedRef).
				long			pos = 0;
				bool			bFound = false;
				select_pair		sel;
				embed_ref		er;
				pg_embed_ptr	embed_ptr;

				sel.begin = 0;
				sel.end = pg->t_length;

				for ( long li =  1; !bFound && (er = pgGetIndEmbed( pg->myself, &sel, li, &pos, 0 )); li++ )
				{
					embed_ptr = (pg_embed_ptr) UseMemory(er);
					
					if (embed_ptr)
					{
						if (embed->style == embed_ptr->style)
							bFound = true;

						UnuseMemory( er );
					}
				}

				if (bFound)
				{
					// Call pgInvalEmbedRef so that the embedded style gets updated with
					// our updated dimensions.
					pgInvalEmbedRef( pg->myself, pos, embed, draw_none );

					// Make the window erase and redraw to handle any text that used to
					// be drawn in the space we now occupy
					if (pSB && pSB->pWndOwner)
						pSB->pWndOwner->Invalidate();
				}
			}

			if (bEudoraTempFile)
			{
				// Caching didn't work, so we used a temp file. Now that we've loaded
				// the temp file we can delete it. A more permanent file will later
				// be created in the embedded images directory.
				::DeleteFile(filePath);

				// Also delete the original name with the .tmp extension - we kept it
				// around until now because GetTempFileName might rely on the existence
				// of the original file to make sure the name is unique.
				if ( ChangeFileNameExtension(filePath, "tmp", MAX_PATH) )
					::DeleteFile(filePath);
			}
        }
    }

    return fRet;
}

// UGLY: boy, the last-minute hacks are starting to pile up. this routine needs
// to be reworked a little, as it is getting a bit ugly. we also need to
// decide on the type/subtype "stuff" for sure before we go too much further.

bool PgSaveUrlImage(paige_rec_ptr pg, pg_url_image_ptr pUrlImage, pg_embed_ptr pEmbed)
{
    bool fRet = false;
    PgStuffBucket* pSB = (PgStuffBucket*) pg->user_refcon;

	if ( !pSB || !pSB->pMessage || (pSB->subkind == PgStuffBucket::kPreviewMessage) )
	{
		ASSERT(0);
		return false;
	}

    QCMessage* pM = pSB->pMessage;
    char *pUri = (char*) pUrlImage->URL;

    if ( pSB->bOleExport == true ) {
        // if we're doing a copy/drag, we just want to zero out the uri. it's
        // nice to be able to copy/paste uris and all that, but it raises too
        // many issues in the short term (gotta ship!)
        pgFillBlock( pUrlImage->URL, sizeof( pUrlImage->URL ), 0 );
        pgFillBlock( pUrlImage->alt_string, sizeof(pUrlImage->alt_string), 0 );
        fRet = true;
    }
    else {
        switch ( pSB->kind ) {
        case PgStuffBucket::kMessage:
        {
            char cid[MAX_PATH] = "";

            // no uri at all: image was pasted into document, create new part.
            if ( !(*pUri) ) {
                make_new_part( pM, pUrlImage, embed_jpeg_pict, cid );

                // if it's a ReadMessage, convert the cid to file path (barf)
                if ( pSB->subkind == PgStuffBucket::kReadMessage ) {
                    pM->GetPartAsFile( cid, cid, sizeof(cid), false );
                    make_filepath_url( (const char*) cid, cid );
                }
            }

            // uri is a "file://": create new part; except if this is a
            // ReadMessage in edit mode, in which case we just leave it alone
            // (let's not kid ourselves that this really "works").
            else if ( is_filepath_url( pUri ) )
			{
                if ( pSB->subkind == PgStuffBucket::kReadMessage )
				{
                    altstring_schmookerdoodle( pM, pUrlImage );
                    fRet = true;
                    break;      // man this is gettin' gross!
                }
                else
				{
                    bool	bIsEmoticon = ( (pEmbed->type & EMBED_EMOTICON_FLAG) != 0 );

					make_new_part_from_file(pM, pUri, cid, bIsEmoticon);
				}
            }

            // uri is a cid: create new part if the cid doesn't exist
            // BOG: another sick hack for edit mode ReadMessages
            else if ( pSB->subkind == PgStuffBucket::kReadMessage ) {
                if ( !SUCCEEDED(pM->GetPartAsFile( pUri,
                                                   cid, sizeof(cid), false )) ) {

                    char hackcid[MAX_PATH] = "";
                    make_it_schmookie( pM, pUrlImage, hackcid );
                    pM->GetPartAsFile( hackcid, cid, sizeof(cid), false );
                    make_filepath_url( (const char*) cid, cid );
                }
            }
            else
                make_it_schmookie( pM, pUrlImage, cid );

            if ( *cid ) {
                strcpy( pUri, cid );
                altstring_schmookerdoodle( pM, pUrlImage );
                fRet = true;
            }
            else
                assert( 0 );
        }
        break;

        case PgStuffBucket::kDocument:
            // if the uri is empty, it means the image came from the clipboard.
            // we must create a new disk based file, deriving the url from the
            // file's path.
            break;
        }
    }

    // if this is an ole operation (copy/paste or drag/drop), then we cache
    // the data in any additional formats we care to provide. right now this
    // is just bitmaps; if we decide to go hog-wild, we'll need to consider
    // delayed rendering as this could get *very* expensive.

    // we only do this for explicitly selected embeds
    long begin, end;
    pgGetSelection( pg->doc_pg, &begin, &end );
    
    if ( pSB->bOleExport && ((end - begin) == 2) ) {
        COleDataSource* pDataSource = pSB->pDS;

        if ( pDataSource ) {
            STGMEDIUM stg;
            HBITMAP hbm = bitmap_from_url_image( pUrlImage );

            if ( hbm ) {
				//	The real key here is that stg.pUnkForRelease is set to NULL
				//	(fixes bug where crash occurred when bitmap was later released),
				//	but it doesn't hurt to set the entire thing to NULL.
				memset( &stg, 0, sizeof(STGMEDIUM) );
				
                stg.tymed = TYMED_GDI;
                stg.hBitmap = hbm;
                pDataSource->CacheData( CF_BITMAP, &stg );
            }
        }
    }

    return fRet;
}


// altstring_schmookerdoodle - just a little crust. takes a message object
// and an url_image, and patches-up the alt_string to the name of the file
// associated with the mime part. If I meant for anyone to use this but me, I
// would have given the darn thing a real name. (word up).

static void altstring_schmookerdoodle( QCMessage* pM,
                                       pg_url_image_ptr pUrlImage )
{
    if ( pUrlImage && !*pUrlImage->alt_string ) {
		const char* purl = (const char*) pUrlImage->URL;
		char fpath[MAX_PATH] = "";

		if ( !is_filepath_url( (char*)purl ) )
			pM->GetPartAsFile( purl, fpath, sizeof(fpath), false );
		else
			strcpy( fpath, purl );

		assert( *fpath );

		if ( *fpath ) {
			int fnamLen;
			char* fname = strrchr( fpath, '\\' );
			assert( fname );

			if ( fname )
				fname++;
			else
				fname = fpath;

			if ( (fnamLen = strlen( fname )) > 63 )
				fnamLen = 63;

			char* pas = (char*) pUrlImage->alt_string;
			size_t asSize = sizeof(pUrlImage->alt_string);
			pgFillBlock( pas, asSize, 0 );
			pgBlockMove( fname, pas, fnamLen );
		}
	}
}


// make_new_part - for saving images that exist in paige, but not as part
// of a message. right now we only handle jpegs.
//
// BOG: if yer wondering why we don't call make_new_part_from_file here, it's
// because we want the files that we create to be managed by the mime
// storage stuff. (just in case your one of those insufferable bozos that
// can't help "correcting" other people's code ;-)

static void make_new_part( QCMessage* message, pg_url_image_ptr pUrlImage,
                           long type, char* cid )
{
    // BOG: We only need to handle "embed_jpeg_pict" for now, but with
    // QT3.0 we will have to support quite a bit more.
    ASSERT( type == embed_jpeg_pict );

    // convert, in two steps, the metafile to a jpeg file
    HBITMAP hbm = bitmap_from_url_image( pUrlImage );
	ASSERT(hbm);
	if (hbm)
	{
		CBitmap * pBitmap = CBitmap::FromHandle( hbm );
		ASSERT(pBitmap);

		if (pBitmap)
		{
			// Now that we know that at least part of the conversion has worked
			// go ahead and create the part.
			char* ext = "jpg";
			QCMimePart* pMP = NULL;

			message->NewPart( ext, &pMP );

			if (pMP)
			{
				pMP->Close();   // we'll do all the writing
				
				// create a new mime part & get the actual file
				CString csCID;
				char fullPath[MAX_PATH] = "";
				pMP->GetCID( csCID );

				if ( !csCID.IsEmpty() )
				{
					message->GetPartAsFile( csCID, fullPath, sizeof(fullPath), false );
					ASSERT( *fullPath );
					
					if (*fullPath)
					{
						bool fCreated = CreateJpegFromBitmap( pBitmap, fullPath );
						ASSERT( fCreated );
						if (fCreated)
						{
							// cid gets written as part of html message
							strcpy( cid, csCID );
						}
						else
						{
							// JPEG creation failed! Delete the file and remove the part.
							::DeleteFile(fullPath);
							message->RemovePart(csCID);
						}
					}
				}

				delete pMP;
			}
		}
		
		::DeleteObject( hbm );
	}
}


static void make_new_part_from_file( QCMessage* message, char* filepath,
                                     char* cid, bool bIsEmoticon )
{
    char path[MAX_PATH] = "";
    QCMimePart* pMP = NULL;
    clean_filepath( filepath, path );

    message->AddPart(path, &pMP, bIsEmoticon);

    if ( pMP ) {
        CString csCID;
        pMP->GetCID( csCID );
        strcpy( cid, csCID );
        delete pMP;
    }

    // BOG: this is in lieu of error handling; for now.
    ASSERT( strlen( cid ) );
}


static void clean_filepath( char* in, char* out )
{
    char*	path = in;     // set this up so failure returns the "in"
	size_t	nLocalFileSpecLen = strlen(local_file_spec);

    if ( strnicmp(in, local_file_spec, nLocalFileSpecLen) == 0 ) {
        // Handle both file:// prefix and file:/// prefix
        path = in + nLocalFileSpecLen;

        if (*path == '/')
            path++;
    }

    strcpy( out, path );
}


static void make_filepath_url( const char* in, char* out )
{
    CString path = "";
    if ( strnicmp( in, local_file_spec, strlen(local_file_spec) ) )
        path += local_file_spec;

    path += in;
    strcpy( out, path );
}


static bool is_filepath_url( char* filepath )
{
    return (strnicmp( filepath, local_file_spec, strlen(local_file_spec) ) == 0);
}


static void make_it_schmookie( QCMessage* message,
                               pg_url_image_ptr pUrlImage, char* newCid )
{
    QCMimePart* pMP = NULL;
    char* cid = (char*) pUrlImage->URL;
    message->GetPart( cid, &pMP );

    if ( !pMP ) 
    {
        make_new_part( message, pUrlImage, embed_jpeg_pict, newCid );
    }
    else 
    {

        // the theory here is based on the fact that when one does a
        // Reply/Forward/Redirect on a message the mime parts end up without
        // cids. we make new parts, and copy the data; the cid-less parts get
        // dropped on the floor when the message is sent. Hack? Yes.

        if ( message->IsRealCID( cid ) ) 
        {
            const INT32 kBufSize = (10 * 1024);
			char* pBuf = DEBUG_NEW_NOTHROW char[kBufSize];

			if ( pBuf )
            {
                char* ext = "jpg";
                char* dot = NULL;

                pBuf[ 0 ] = '\0';
                message->GetPartAsFile( cid, pBuf, kBufSize );
                dot = strrchr( pBuf, '.' );
                if ( dot != NULL )
                    ext = ++dot; //skip '.'

                QCMimePart* pNewMP = NULL;
                message->NewPart( ext, &pNewMP );

                INT32 nBytesRead = 0;
                INT32 nBytesWritten = 0;

                do 
                {
                    if ( SUCCEEDED(pMP->Read( pBuf, kBufSize, &nBytesRead )) ) 
                    {
                        if ( FAILED(pNewMP->Write( pBuf, nBytesRead,
                                                   &nBytesWritten )) ) 
                        {
                            break;
                        }

                        assert( nBytesRead == nBytesWritten );
                    }
                    else 
                    {
                        assert( 0 );
                        break;
                    }
                } while ( nBytesRead == kBufSize );

                pNewMP->Close();

                CString csCID;
                pNewMP->GetCID( csCID );
                strcpy( newCid, csCID );

                delete pNewMP;
                delete[] pBuf;
            }
        }
        else 
        {
            strcpy( newCid, cid );
        }

        pMP->Close();
        delete pMP;
    }
}


// PgNewUrlImageRef - mostly lifted from some private Paige code, this routine
// knows how to set-up/create an embed ref for a url_image. the Paige dudes
// seemed to think that these would only ever be created from html source,
// but they were really wrong!

embed_ref PgNewUrlImageRef( pg_ref pg, const char* pUrl, short width,
                            short height, const char* pAltString,
                            pg_url_image_ptr pUrlImage,
                            embed_callback callback )
{
    pg_url_image        image_record;
    memory_ref          embed, image_ref;
    style_info          current_style, style_mask;
        
    paige_rec_ptr       pgRec = (paige_rec_ptr) UseMemory( pg );

    long param_size, image_descent, original_descent, original_ascent;

    if ( pUrlImage ) {
        image_record = *pUrlImage;
    }
    else {
        pgFillBlock( &image_record, sizeof(pg_url_image), 0 );
        resolve_URL( (pg_char_ptr) pUrl, image_record.URL, sizeof(image_record.URL) );
        image_record.source_type = (short)image_type( image_record.URL );
        image_record.source_width = width;
        image_record.source_height = height;

        if ( pAltString != NULL ) {
            if ( (param_size = strlen( pAltString )) > 63 )
                param_size = 63;

            pgBlockMove( pAltString, image_record.alt_string, param_size );
        }
    }

    // since Paige uses the URL as the condition for their reference counting
    // scheme, we must coble one together if we don't allready have one.
    // BUG?: do we need a file extension for this?
    if ( !(*image_record.URL) ) {
        PgStuffBucket* pSB = (PgStuffBucket*) pgRec->user_refcon;
        pSB->UniqueIDString( (char*)image_record.URL );
    }

    image_ref = pgNewImageRecord( pg, &image_record, callback, -1 );
    pgGetStyleInfo( pg, NULL, FALSE, &current_style, &style_mask );
    original_ascent = current_style.ascent;
    image_descent = original_descent = (long)current_style.descent;
    image_descent <<= 16;
      
    embed = (long) pgNewEmbedRef( pgRec->globals->mem_globals,
                                  embed_url_image, (void PG_FAR *)image_ref,
                                  0, ALIGN_CBASELINE_FLAG, -image_descent,
                                  0, FALSE);

    UnuseMemory( pg );
    return embed;
}


static long image_type (pg_char_ptr filename)
{
    long        type = embed_gif_pict;
    short       str_size;
   
    str_size = (short)pgCStrLength(filename);
   
    while (str_size > 0) {
      
        --str_size;
      
        if (filename[str_size] == '.')
            break;
    }
   
    if (filename[str_size] == '.') {
   
        if (test_extension(&filename[str_size + 1], (pg_char_ptr)"jpg")
            || test_extension(&filename[str_size + 1], (pg_char_ptr)"jpe"))
            type = embed_jpeg_pict;
        else
            if (test_extension(&filename[str_size + 1], (pg_char_ptr)"pic"))
                type = embed_mac_pict;
    }
   
    return      type;
}


static pg_boolean test_extension (pg_char_ptr file_ext, pg_char_ptr match)
{
    short       index;
   
    for (index = 0; index < 3; ++index)    
        if (file_ext[index] != match[index])
            return   FALSE;
   
    return   TRUE;
}


unsigned PgValidateImageFile( const char* fullPath, BOOL bDisplayFileErrors )
{
    unsigned uRet;

	if (strlen(fullPath) >= MAX_PATH)
		return IDS_ERR_IMAGEOPEN;

    if ( CanHandleImageInternally( fullPath, bDisplayFileErrors ) ) {
        uRet = 0;
    }
    else {
        // currently, the errcode param for this routine is not implimented;
        // all failures will generate "Unable to open image file" messages.
        if ( init_quicktime() && QTW_IsValidImageFile( fullPath, NULL ) ) {
            uRet = 0;
        }
        else {
            uRet = IDS_ERR_IMAGEOPEN;
        }
    }

    return uRet;
}


bool PgGetOfnFilterString( char** ppFS )
{
    bool bRet = false;

	// Attempt to init QT before continuing. That way we use QT if it's available
	// (otherwise we would only use QT if the user had previously viewed an image
	// that we couldn't handle internally, which is just plain non-intuitive).
	init_quicktime();

    if ( g_qtInitialized ) {
        bRet = QTW_GetOfnFilterString( ppFS );
    }
    else {
        // without QT, we can only handle bmp, jpeg, and png
        *ppFS = "Image Files|*.bmp;*.jpg;*.jpeg;*.jpe;*.png|All Files|*.*||";
        bRet = true;
    }

    return bRet;
}


// url_image_to_bitmap - creates a bitmap compatible with the current display
// settings from a metafile-based "url_image". the bitmap is a copy; the
// url_image is not modified; caller must dispose of the bitmap.

static HBITMAP bitmap_from_url_image( pg_url_image_ptr pUrlImage )
{
	HBITMAP		hbm = NULL;
	
	if (pUrlImage && pUrlImage->image_data)
	{		
		// create a display-compatible memory dc & bitmap
		HDC hdcOffScreen = ::CreateCompatibleDC( NULL );
		assert( hdcOffScreen );

		if (hdcOffScreen)
		{
			// Ben sez: here's a little freebie: if you try to use a memory dc to create a
			// compatible bitmap, you will not be happy with the results ;-)
			hbm = ::CreateCompatibleBitmap( ::GetDC( NULL ),
											pUrlImage->source_width,
											pUrlImage->source_height );
			assert( hbm );

			if (hbm)
			{
				HBITMAP hbmOld = (HBITMAP) ::SelectObject( hdcOffScreen, hbm );

				// play the metafile on the offscreen bitmap
				// Possibly for robustness we shouldn't always assume that image_data
				// will be a HMETAFILE, but perhaps in Eudora's use it always is?
				HMETAFILE hMF = (HMETAFILE) pUrlImage->image_data;

				::PlayMetaFile( hdcOffScreen, hMF );

				// select out the bitmap & cleanup
				::SelectObject( hdcOffScreen, hbmOld );
			}

			::DeleteDC( hdcOffScreen );
		}
	}

    return hbm;     // schmookie!
}


// url_image_from_bitmap - fills out a pg_url_image structure from a supplied
// bitmap, including converting the bitmap to a metafile (this due to the
// fact that we use metafiles for all images in the editor)

static bool url_image_from_bitmap( HBITMAP hbm, pg_url_image_ptr pUrlImage )
{
    bool bRet = false;
    
    // create a memory metafile dc
    HDC hdcMetaFile = ::CreateMetaFile( NULL );
    assert( hdcMetaFile );
    
    // create a display-compatible memory dc for the source
    HDC hdcOffScreen = ::CreateCompatibleDC( NULL );
    assert( hdcOffScreen );
    
    if ( hdcMetaFile && hdcOffScreen ) {
        BITMAP bmp;
        ::GetObject( hbm, sizeof(BITMAP), &bmp );
        HBITMAP hbmOld = (HBITMAP) ::SelectObject( hdcOffScreen, hbm );

        // microsmoosh says anisotropic is 'da kine
        ::SetMapMode( hdcMetaFile, MM_ANISOTROPIC );
        
        // blit 'dem bits!
        ::BitBlt( hdcMetaFile,
                  0,0,
                  bmp.bmWidth,
                  bmp.bmHeight,
                  hdcOffScreen,
                  0,0,
                  SRCCOPY );

        ::SelectObject( hdcOffScreen, hbmOld );
        HMETAFILE hmf = ::CloseMetaFile( hdcMetaFile );
        assert( hmf );

        // fill out the url_image structure. note: since this image comes from
        // a memory-based data source, it has no associated url, and since we
        // don't do png like we should, its type is allways jpeg
        pgFillBlock( pUrlImage, sizeof(pg_url_image), 0 );
        pUrlImage->source_type =
            (short)image_type( (unsigned char*) "dummy.jpg" );

        pUrlImage->source_width = (short) bmp.bmWidth;
        pUrlImage->source_height = (short) bmp.bmHeight;
        pUrlImage->type_and_flags = embed_meta_file;
        pUrlImage->image_data = (unsigned long) hmf;
        pgBlockMove( "Emacs!", pUrlImage->alt_string, 6 );      
        bRet = true;
    }

    if ( hdcMetaFile )
        ::DeleteDC( hdcMetaFile );

    if ( hdcOffScreen )
        ::DeleteDC( hdcOffScreen );

    return bRet;     // dookie!
}

// duplicate a string using "new" operator
char* dupe_string( const char* str )
{
	char* ps = DEBUG_NEW_NOTHROW char[strlen( str ) + 1];

	if ( ps )
		strcpy( ps, str );

	return ps;
}

// return the local filepath for this url.
static int fetch_url_schmookie( FetchContext** pfq, char* url, char* filePath, HWND hNotify, BOOL & bEudoraTempFile )
{
    int iRet = 0;
	unsigned uid = hash_string( url );
	FetchContext* pfc = *pfq;

	if ( pfc ) {
		// Check to see if the thread is done (32-bit variable, so checking it is atomic)
		if ( pfc->bThreadDone ) {
			if ( pfc->filePath ) {
				strcpy( filePath, pfc->filePath );
				bEudoraTempFile = pfc->bEudoraTempFile;
				iRet = 1;
			}
			else
				iRet = -1;

			delete pfc;

			*pfq = 0;
		}
	}
	else {
		*pfq = pfc = DEBUG_NEW_NOTHROW FetchContext;

		if ( pfc ) {
			pfc->uid = uid;
			pfc->url = dupe_string( url );
			pfc->hNotify = hNotify;

			// Start up the thread to download the image
			AfxBeginThread(FetchContext::getfile_thread_proc, pfc);
		}
	}

    return iRet;
}


unsigned int hash_string(const char* pszStr)
{
	unsigned int ulSum = 0;
	const unsigned int kKRHashPrime = 2147483629;

// algorithm: KRHash---derived from Karp & Rabin, Harvard Center for Research
// in Computing Technology Tech. Report TR-31-81. The constant prime number,
// kKRHashPrime, happens to be the largest prime number that will fit in
// 31 bits, except for 2^31-1 itself.

	for (; *pszStr; pszStr++)
	{
		for (int nBit = 0x80; nBit != 0; nBit >>= 1)
		{
			ulSum += ulSum;
			if (ulSum >= kKRHashPrime)
				ulSum -= kKRHashPrime;
			if ((*pszStr) & nBit)
				++ulSum;
			if (ulSum>= kKRHashPrime)
				ulSum -= kKRHashPrime;
		}
	}

	return (ulSum + 1);
}


////////////////////////////////////////////////////////////////////////////////
// FetchContext -- the inet context used for caching ads
#define ods(s) OutputDebugString(s"\n")

FetchContext::FetchContext()
{
	uid = 0;
	url = 0;
	filePath = 0;
	hNotify = 0;
	getfile_thread = 0;
	bTerminate = FALSE;
	bThreadDone = FALSE;
	bEudoraTempFile = FALSE;
}

FetchContext::~FetchContext()
{
	delete [] url;
	delete [] filePath;
}

void FetchContext::terminate()
{
	CSingleLock		singleLock(&criticalSection);
	
	if ( singleLock.Lock() ) {
		// If the thread isn't already done then tell it to terminate itself and delete this (its FetchContext)
		if (!bThreadDone)
			bTerminate = TRUE;

		singleLock.Unlock();
	}

	// The thread is done, self-destruct
	if (bThreadDone)
		delete this;
}

// custom windows message handled by PaigeEdtView
const UINT umsgInvalidateImage = RegisterWindowMessage( "umsgInvalidateImage" );
UINT FetchContext::getfile_thread_proc(LPVOID pParam)	// static
{
	FetchContext* pctx;   // image fetching state info
	if ( !(pctx = (FetchContext*) pParam) )
		return (unsigned) -1;

    // first check to see if the url has a cache entry. if so we'll just
    // pass back the path to the file in the cache. don't ever save one
    // of these file paths; you can't count on them hanging around.

// Getting rid of the portions that grab the resource off the network because
// it's happening on the main thread.  When we get the code to grab stuff
// in a background thread, this will need to be rewritten to handle it.

	bool bImageRetrieved = false;
	char szTempFileName[MAX_PATH];
	*szTempFileName = '\0';

    for ( int pass = 1; pass <= 2 && !bImageRetrieved && !pctx->bTerminate; pass++ ) {

        DWORD ceiSize = 0;
        LPINTERNET_CACHE_ENTRY_INFO pei;

        if ( !GetUrlCacheEntryInfo( pctx->url, NULL, &ceiSize ) ) {
			DWORD dwError = GetLastError();
            if ( dwError == ERROR_INSUFFICIENT_BUFFER ) {
				pei = (LPINTERNET_CACHE_ENTRY_INFO) DEBUG_NEW char[ceiSize];

                if ( GetUrlCacheEntryInfo( pctx->url, pei, &ceiSize ) ) {
					pctx->filePath = dupe_string( pei->lpszLocalFileName );
                    bImageRetrieved = true;

					// Getting the cache entry worked - so we no longer need the temp file
					if (*szTempFileName) {
						// Delete the temp file
						::DeleteFile(szTempFileName);

						// Also delete the original name with the .tmp extension - we kept it
						// around until now because GetTempFileName might rely on the existence
						// of the original file to make sure the name is unique.
						if ( ChangeFileNameExtension(szTempFileName, "tmp", MAX_PATH) )
							::DeleteFile(szTempFileName);

						*szTempFileName = '\0';
					}
                }

                delete [] pei;
            }
            else if ( pass == 1 ) {
                // initializing session NULL causes the return from
                // AfxGetAppName() to be used for the user-agent name.

                CInternetSession session( NULL );
                CStdioFile* pFile = NULL;
                bool bLocal = is_filepath_url( pctx->url );

                try {
                    pFile = session.OpenURL( pctx->url, 1, INTERNET_FLAG_TRANSFER_BINARY );

                    if ( pFile ) {
                        if ( bLocal ) {
                            // url is of the form "file://"; no need to fetch
							pctx->filePath = dupe_string( pFile->GetFilePath() );
                            bImageRetrieved = true;
                        }
                        else {
							// Fetch network file. We'll attempt to get the file from the automatically maintained cache
							// in the next pass.
							char * buf = DEBUG_NEW_NOTHROW char[10240];

							if (buf) {
								// If possible we'll save everything we fetch into a temporary file.
								// If getting the information from the cache works we can just delete it. However
								// I've seen cases where the automatic cache didn't cache (I don't know why) - even
								// manually attempting to force it to cache via CreateUrlCacheEntry and
								// CommitUrlCacheEntry didn't work.
								// Oh well - the temp file approach isn't much more work.
								BOOL	bUseTempFile = FALSE;

								if ( (!TempDir.IsEmpty()) && ::GetTempFileName(TempDir, "eud", 0, szTempFileName) ) {
									// ::GetTempFileName will always give us a ".tmp" extension.
									// Change the extension to match the type of file that we're downloading
									char *	szFileExt = strrchr(pctx->url, '.');

									if (szFileExt) {
										// Skip past the period
										szFileExt++;
										ChangeFileNameExtension(szTempFileName, szFileExt, MAX_PATH);
									}

									bUseTempFile = TRUE;
								}

								UINT	nCount;
								CFile	tempFile;
								
								// Open the temp file
								if (bUseTempFile)
									bUseTempFile = tempFile.Open(szTempFileName, CFile::modeCreate | CFile::modeWrite);
								
								while (!pctx->bTerminate)
								{
									nCount = pFile->Read( buf, 10240 );
									
									if (nCount == 0)
										break;
									
									// Write what we've downloaded to the temp file
									if (bUseTempFile)
										tempFile.Write(buf, nCount);
								}
								
								// tempFile will close on its own when it falls out of scope
								
								delete [] buf;
							}
                        }

                        pFile->Close();
                    }
                }
                catch ( CInternetException* pEx )
                {
                    // TODO: need to do something other than report the
                    // error---a broken link image would be good.
//                    pEx->ReportError();
                    pEx->Delete();
                }
				catch ( CFileException* e )
				{
					e->Delete();
				}

                delete pFile;
                session.Close();
            }
            else if ( (pass == 2) && *szTempFileName ) {
				// Getting the cache entry failed and we have a temp file - so use it
				pctx->filePath = dupe_string(szTempFileName);

				// Remember that we're using a temp file so that we know to delete it after
				// we load it.
				pctx->bEudoraTempFile = TRUE;

				bImageRetrieved = true;
			}
        }
    }

	CSingleLock		singleLock(&pctx->criticalSection);

	// Access setting bThreadDone
	if ( singleLock.Lock() ) {
		// If we haven't been told to terminate, then mark ourselves done
		if (!pctx->bTerminate)
			pctx->bThreadDone = TRUE;

		singleLock.Unlock();
	}
	
	if (pctx->bTerminate) {
		// We were told to terminate, so delete our information.
		delete pctx;
	}
	else if (bImageRetrieved) {
		// We weren't told to terminate and we retrieved our image so post a message
		// so that the image will be updated.
		if (bImageRetrieved)
			PostMessage( pctx->hNotify, umsgInvalidateImage, pctx->uid, 0 );
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// The End

