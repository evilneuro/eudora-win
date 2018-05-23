// qtwrapper.h - this is a wrapper api for anything that makes calls into
// the Quicktime API. Quicktime conflicts with Paige (Bad Paige boys!), so
// we need to isolate any Quicktime stuff here. Absolutely no Paige references
// can be made in this module, and no QT or Paige stuff can exist in the
// interface to this module.

#ifndef _QTWRAPPER_H_
#define _QTWRAPPER_H_

extern "C" {
    bool QTW_Startup();
    void QTW_Shutdown();
    bool QTW_MetafileFromQtImage( char* filePath, HMETAFILE *phMeta,
				  int *pCx, int *pCy );
    bool QTW_DibFromQtImage( char* filePath, HBITMAP* phDib, RECT* pRect );
    bool QTW_GetOfnFilterString( char** s );
    bool QTW_IsValidImageFile( const char* pFileName, unsigned** ppErrCode );
}


#endif   // _QTWRAPPER_H_

