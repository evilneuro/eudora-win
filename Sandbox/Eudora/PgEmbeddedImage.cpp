// PgEmbeddedImage.cpp
//
// Routines for embedding "url-images". These inline graphics are a
// specialization of an ordinary embedded image object. Originally, Paige only
// used "url-images" for images created by importing an html file, but sanity
// dictates that they be used exclusively for embedded image support in
// Eudora. (Take my word for it.)

// windows
#include "stdafx.h"
#include "afxinet.h"
#include "resource.h"

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
#include "qcgraphics.h"

// mime storage objects
#include "MimeStorage.h"

// this module's public face.
#include "PgEmbeddedImage.h"

// plain ol' butt-ugly spaghetti
#include "PgStuffBucket.h"



#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////////////
// private parts

static PG_PASCAL (long) eCallback( paige_rec_ptr pg, pg_embed_ptr embed_ptr,
        long embed_type, short command, long user_refcon, long param1, long param2 );

static long image_type( pg_char_ptr filename );
static void resolve_URL( pg_char_ptr source_URL, pg_char_ptr target_URL );
static pg_short_t translate_hex( pg_char hex_char );
static pg_boolean test_extension( pg_char_ptr file_ext, pg_char_ptr match );
static void make_new_part( QCMessage* message, pg_url_image_ptr pUrlImage,
                                        long type, char* cid );
static void make_new_part_from_file( QCMessage* message, char* filepath, char* cid );
static void clean_filepath( char* in, char* out );
static void make_filepath_url( const char* in, char* out );
static bool is_filepath_url( char* filepath );
static void make_it_schmookie( QCMessage* message, pg_url_image_ptr pUrlImage,
                                                        char* newCid );

static bool ShouldDisplayWithStingray( const char* fullPath );
static HBITMAP bitmap_from_url_image( pg_url_image_ptr pUrlImage );
static bool url_image_from_bitmap( HBITMAP hbm, pg_url_image_ptr pUrlImage );
static bool init_quicktime();
static void altstring_schmookerdoodle( QCMessage* pM,
                                       pg_url_image_ptr pUrlImage );

static bool fetch_url_schmookie( char* url, char* filePath );


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

static bool init_quicktime()
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
                    if ( PgLoadUrlImage( pg, image ) )
                        return 0;
                }
            }

            break;

        case EMBED_LOAD_IMAGE:
            if ( !image->image_data ) {
                if ( PgLoadUrlImage( pg, image ) )
                    return 0;
            }

                break;

        case EMBED_SAVE_IMAGE:
            if ( PgSaveUrlImage( pg, image ) )
                return 0;

            break;
        }
    }

    // If not handled above then call the default:
    return pgDefaultEmbedCallback( pg, embed_ptr, embed_type, command,
                                   user_refcon, param1, param2 );
}


bool PgLoadUrlImage( paige_rec_ptr pg, pg_url_image_ptr pUrlImage )
{
    bool fRet = false;

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

        if ( pSB && pSB->kind == PgStuffBucket::kMessage ) {
            QCMessage* pMsg = pSB->pMessage;
            CString srcURI = pUrlImage->URL;
            char tempfilePath[MAX_PATH] = "";

            pMsg->GetPartAsFile( srcURI, tempfilePath, sizeof(tempfilePath) );
            clean_filepath( (char*) tempfilePath, filePath );
        }

        if ( !(*filePath) ) {
            // ok, we failed to find it in our "mime store", so we let wininet
            // have a poke. this gets us http, ftp, gopher, and "file://".
            fetch_url_schmookie( (char*)pUrlImage->URL, filePath );
        }

        if (ShouldDisplayWithStingray(filePath)) {
            QCMetaFileInfo mfi;
            if (MetafileFromImage( filePath, &mfi )) {
                if ( !pUrlImage->source_width )
                    pUrlImage->source_width = (short)mfi.width;
                if ( !pUrlImage->source_height )
                    pUrlImage->source_height = (short)mfi.height;

                pUrlImage->type_and_flags = embed_meta_file;
                pUrlImage->image_data = (long)mfi.hm;
                fRet = true;
            }
        }

        if ( !fRet && init_quicktime()) {
// QTRW
            // convert image to a metafile format, and hand it to paige
            HMETAFILE hMeta;
            int cx = pUrlImage->source_width;
            int cy = pUrlImage->source_height;

            QTW_MetafileFromQtImage( filePath, &hMeta, &cx, &cy );

            if ( hMeta ) {
                pUrlImage->source_width = (short)cx;
                pUrlImage->source_height = (short)cy;
                pUrlImage->type_and_flags = embed_meta_file;
                pUrlImage->image_data = (long)hMeta;
                fRet = true;
            }
        }
    }

    return fRet;
}

// UGLY: boy, the last-minute hacks are starting to pile up. this routine needs
// to be reworked a little, as it is getting a bit ugly. we also need to
// decide on the type/subtype "stuff" for sure before we go too much further.

bool PgSaveUrlImage( paige_rec_ptr pg, pg_url_image_ptr pUrlImage )
{
    bool fRet = false;
    PgStuffBucket* pSB = (PgStuffBucket*) pg->user_refcon;
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
                    pM->GetPartAsFile( cid, cid, sizeof(cid) );
                    make_filepath_url( (const char*) cid, cid );
                }
            }

            // uri is a "file://": create new part; except if this is a
            // ReadMessage in edit mode, in which case we just leave it alone
            // (let's not kid ourselves that this really "works").
            else if ( is_filepath_url( pUri ) ) {
                if ( pSB->subkind == PgStuffBucket::kReadMessage ) {
                    altstring_schmookerdoodle( pM, pUrlImage );
                    fRet = true;
                    break;      // man this is gettin' gross!
                }
                else
                    make_new_part_from_file( pM, pUri, cid );
            }

            // uri is a cid: create new part if the cid doesn't exist
            // BOG: another sick hack for edit mode ReadMessages
            else if ( pSB->subkind == PgStuffBucket::kReadMessage ) {
                if ( !SUCCEEDED(pM->GetPartAsFile( pUri,
                                                   cid, sizeof(cid) )) ) {

                    char hackcid[MAX_PATH] = "";
                    make_it_schmookie( pM, pUrlImage, hackcid );
                    pM->GetPartAsFile( hackcid, cid, sizeof(cid) );
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
    const char* purl = (const char*) pUrlImage->URL;
    char fpath[MAX_PATH] = "";

    if ( !is_filepath_url( (char*)purl ) )
        pM->GetPartAsFile( purl, fpath, sizeof(fpath) );
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

    char* ext = "jpg";
    QCMimePart* pMP = NULL;
    message->AddPart( ext, &pMP );

    if ( pMP ) {
        pMP->Close();   // we'll do all the writing

        // create a new mime part & get the actual file
        CString csCID;
        char fullPath[MAX_PATH] = "";
        pMP->GetCID( csCID );
        message->GetPartAsFile( csCID, fullPath, sizeof(fullPath) );
        assert( *fullPath );

        // convert, in two steps, the metafile to a jpeg file
        HBITMAP hbm = bitmap_from_url_image( pUrlImage );
        CBitmap* pBitmap = CBitmap::FromHandle( hbm );
        bool fCreated = CreateJpegFromBitmap( pBitmap, fullPath );
        assert( fCreated );
        ::DeleteObject( hbm );

        // cid gets written as part of html message
        strcpy( cid, csCID );
        delete pMP;
    }

    // BOG: this is in lieu of error handling; for now.
    ASSERT( strlen( cid ) );
}


static void make_new_part_from_file( QCMessage* message, char* filepath,
                                     char* cid )
{
    char path[MAX_PATH] = "";
    QCMimePart* pMP = NULL;
    clean_filepath( filepath, path );

    // BOG: here's a sick hack; i need to pass this a CString only to satisfy
    // overloaded calling convention. AddPart(char*...) expects a file
    // extension while the CString version expects a full path!
    CString sickHack = path;
    message->AddPart( sickHack, &pMP );

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
    char* path = in;     // set this up so failure returns the "in"

    if ( strnicmp( in, local_file_spec, strlen(local_file_spec) ) == 0 ) {
        path = strstr( in, ":" );
        path += 3;
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
            char* pBuf = new char[kBufSize];
            ASSERT( pBuf );
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
                message->AddPart( ext, &pNewMP );

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

    long        param_size, image_descent, original_descent, original_ascent;
    long        embed_flags = 0;


    if ( pUrlImage ) {
        image_record = *pUrlImage;
    }
    else {
        pgFillBlock( &image_record, sizeof(pg_url_image), 0 );
        resolve_URL( (pg_char_ptr) pUrl, image_record.URL );
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


static void resolve_URL( pg_char_ptr source_URL, pg_char_ptr target_URL )
{
    pg_char_ptr       output, input;
    long           output_size;

    output_size = 0;
    input = source_URL;
    output = target_URL;
   
    while (*input) {
      
//              if (*input == '?')
//                      break;
      
        if (*input == '%') {
         
            ++input;
         
            if (!*input)
                break;

            *output = (pg_char)translate_hex(*input++);
            *output <<= 4;

            if (!*input)
                break;

            *output |= translate_hex(*input++);
        }
        else
            *output++ = *input++;

        output_size += 1;

        if (output_size == 127)
            break;
    }
   
    *output = 0;
}


static pg_short_t translate_hex (pg_char hex_char)
{
    pg_short_t        result;
   
    result = (pg_short_t)hex_char;
   
    if (result > 'F')
        result -= 0x20;
    if (result > '9')
        result -= 7;
   
    return (pg_short_t) (result - 0x30);
}


static pg_boolean test_extension (pg_char_ptr file_ext, pg_char_ptr match)
{
    short       index;
   
    for (index = 0; index < 3; ++index)    
        if (file_ext[index] != match[index])
            return   FALSE;
   
    return   TRUE;
}

bool ShouldDisplayWithStingray(const char* fullPath)
{
    // This whole routine is only necessary until QUICKTIME3 
    // is incorporated into the product

    // Display with Quicktime if it can display it
    // BOG: this next statement assumes Quicktime 3.0+
    if ( g_qtInitialized ) {
        return false;
    }

    // If extension is neither jpg nor gif 
    // then do not display with Stingray
    if (!fullPath) return false;
    size_t len = strlen(fullPath);
    if (len < 5) return false;
    const char* s = strrchr(fullPath, '.');

    if (!s)
        return false;

    s++;
    const bool isJpeg = (stricmp(s, "jpg" ) == 0) || (stricmp(s, "jpeg" ) == 0);
    const bool isBmp = (stricmp(s, "bmp" ) == 0);

    if ( !(isJpeg || isBmp) )
        return false;

    // Quicktime is not available and this is a jpg or bmp file
    // Have Stingray display it
    return true;
}

unsigned PgValidateImageFile( const char* fullPath )
{
    unsigned uRet;

    if ( ShouldDisplayWithStingray( fullPath ) ) {
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

    if ( g_qtInitialized ) {
        bRet = QTW_GetOfnFilterString( ppFS );
    }
    else {
        // without QT, we can only handle jpeg and bmp
        *ppFS = "Image Files|*.jpeg;*.jpg;*.bmp|All Files|*.*||";
        bRet = true;
    }

    return bRet;
}


// url_image_to_bitmap - creates a bitmap compatible with the current display
// settings from a metafile-based "url_image". the bitmap is a copy; the
// url_image is not modified; caller must dispose of the bitmap.
//
// GOTTAFIX: the lack of EH here is completely nuts.

static HBITMAP bitmap_from_url_image( pg_url_image_ptr pUrlImage )
{
    // create a display-compatible memory dc & bitmap
    HDC hdcOffScreen = ::CreateCompatibleDC( NULL );
    assert( hdcOffScreen );

    // here's a little freebie: if you try to use a memory dc to create a
    // compatible bitmap, you will not be happy with the results ;-)
    HBITMAP hbm = ::CreateCompatibleBitmap( ::GetDC( NULL ),
                                            pUrlImage->source_width,
                                            pUrlImage->source_height );

    assert( hbm );
    HBITMAP hbmOld = (HBITMAP) ::SelectObject( hdcOffScreen, hbm );

    // play the metafile on the offscreen bitmap
    HMETAFILE hMF = (HMETAFILE) pUrlImage->image_data;
    ::PlayMetaFile( hdcOffScreen, hMF );

    // select out the bitmap & cleanup
    ::SelectObject( hdcOffScreen, hbmOld );
    ::DeleteDC( hdcOffScreen );

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

static bool fetch_url_schmookie( char* url, char* filePath )
{
    bool bRet = false;

    // first check to see if the url has a cache entry. if so we'll just
    // pass back the path to the file in the cache. don't ever save one
    // of these file paths; you can't count on them hanging around.

    for ( int pass = 1; pass <= 2 && !bRet; pass++ ) {

        DWORD ceiSize = 0;
        LPINTERNET_CACHE_ENTRY_INFO pei;

        if ( !GetUrlCacheEntryInfo( url, NULL, &ceiSize ) ) {
            if ( GetLastError() == ERROR_INSUFFICIENT_BUFFER ) {
                pei = (LPINTERNET_CACHE_ENTRY_INFO) new char[ceiSize];

                if ( GetUrlCacheEntryInfo( url, pei, &ceiSize ) ) {
                    strcpy( filePath, pei->lpszLocalFileName );
                    bRet = true;
                }

                delete [] pei;
            }
            else if ( pass == 1 ) {
                // initializing session NULL causes the return from
                // AfxGetAppName() to be used for the user-agent name.

                CInternetSession session( NULL );
                CStdioFile* pFile = NULL;
                bool bLocal = is_filepath_url( url );

                try {
                    pFile = session.OpenURL( url, 1, INTERNET_FLAG_TRANSFER_BINARY );

                    if ( pFile ) {
                        if ( bLocal ) {
                            // url is of the form "file://"; no need to fetch
                            strcpy( filePath, pFile->GetFilePath() );
                            bRet = true;
                        }
                        else {
                            // fetch network file, we get a local path from the
                            // cache on the second "pass".
                            char buf[10240];
                            while ( pFile->Read( buf, sizeof(buf) ) > 0 )
                                ;
                        }

                        pFile->Close();
                    }
                }
                catch ( CInternetException* pEx )
                {
                    // TODO: need to do something other than report the
                    // error---a broken link image would be good.
                    pEx->ReportError();
                    pEx->Delete();
                }

                delete pFile;
                session.Close();
            }
        }
    }

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////
// The End

