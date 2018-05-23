/* gzio.c -- IO on .gz files
 * Copyright (C) 1995 Jean-loup Gailly.
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* $Id: gzio.c,v 1.8 1995/05/03 17:27:09 jloup Exp $ */

#include <stdio.h>
#include <malloc.h>

#include "stdafx.h"

#ifdef _SECDLL
#undef AFXAPI_DATA
#define AFXAPI_DATA __based(__segname("_DATA"))
#endif //_SECDLL
#include "compeng.h"
#include "zutil.h"

struct internal_state {int dummy;}; /* for buggy compilers */

#define Z_BUFSIZE 4096

#define ALLOC(size) malloc(size)
#define TRYFREE(p) {if (p) free(p);}

#define GZ_MAGIC_1 0x1f
#define GZ_MAGIC_2 0x8b

/* gzip flag byte */
#define ASCII_FLAG   0x01 /* bit 0 set: file probably ascii text */
#define HEAD_CRC     0x02 /* bit 1 set: header CRC present */
#define EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define COMMENT      0x10 /* bit 4 set: file comment present */
#define RESERVED     0xE0 /* bits 5..7: reserved */

#ifndef SEEK_CUR
#  define SEEK_CUR 1
#endif


 /* ===========================================================================
 * Cleanup then free the given gz_stream. Return a zlib error code.
 */
int SECComp::destroy (gz_stream *s)
{
    int err = Z_OK;

    if (!s) return Z_STREAM_ERROR;

    TRYFREE(s->inbuf);
    TRYFREE(s->outbuf);
    TRYFREE(s->path);
    TRYFREE(s->msg);

    if (s->stream.state != NULL) {
       if (s->mode == 'w') {
           err = deflateEnd(&(s->stream));
       } else if (s->mode == 'r') {
           err = inflateEnd(&(s->stream));
       }
    }

    if (s->pFile != NULL) {
		s->pFile->CFile::Close();
        err = Z_ERRNO;
    }
    if (s->z_err < 0) err = s->z_err;
    TRYFREE(s);
    return err;
}

/* ===========================================================================
     Opens a gzip (.gz) file for reading or writing. The mode parameter
   is as in fopen ("rb" or "wb"). The file is given either by file descritor
   or path name (if fd == -1).
     gz_open return NULL if the file could not be opened or if there was
   insufficient memory to allocate the (de)compression state; errno
   can be checked to distinguish the two cases (if errno is zero, the
   zlib error is Z_MEM_ERROR).
*/
// gzFile SECComp::gz_open (TCHAR *path, TCHAR *mode, int fd)
gzFile SECComp::gz_open (TCHAR *path, TCHAR *mode, int fd, CFile *pFile)
{
    int err;
    int level = Z_DEFAULT_COMPRESSION; /* compression level */
    TCHAR *p = mode;
    gz_stream *s = (gz_stream *)ALLOC(sizeof(gz_stream));
    UINT nOpenFlags = 0;
	fd;	//unused

    if (!s) return Z_NULL;

    s->stream.zalloc = (alloc_func)0;
    s->stream.zfree = (comp_free_func)0;
    s->stream.next_in = s->inbuf = Z_NULL;
    s->stream.next_out = s->outbuf = Z_NULL;
    s->stream.avail_in = s->stream.avail_out = 0;
    // s->pFile = new CFile();

    s->z_err = Z_OK;
    s->z_eof = 0;
    s->crc = crc32(0L, Z_NULL, 0);
    s->msg = NULL;
    s->transparent = 0;

    s->path = (TCHAR*)ALLOC(_tcsclen(path)+1);
    if (s->path == NULL) {
        return destroy(s), (gzFile)Z_NULL;
    }
    _tcscpy(s->path, path); /* do this early for debugging */

    s->mode = '\0';
    do {
        if (*p == 'r') 
	{
		s->mode = 'r';
		nOpenFlags |= CFile::modeRead;
	}
        if (*p == 'w') 
	{
		s->mode = 'w';
		nOpenFlags |= CFile::modeWrite;
	}
        if (*p >= '1' && *p <= '9') level = *p - '0';
    } while (*p++);

	if (pFile)
    	s->pFile = pFile;
    else
    	s->pFile = new CFile(path, nOpenFlags);

    if (s->mode == '\0') return destroy(s), (gzFile)Z_NULL;
    
    if (s->mode == 'w') {
        err = deflateInit2(&(s->stream), level,
                           DEFLATED, -MAX_WBITS, DEF_MEM_LEVEL, 0);
        /* windowBits is passed < 0 to suppress zlib header */

        s->stream.next_out = s->outbuf = (Byte*)ALLOC(Z_BUFSIZE);

        if (err != Z_OK || s->outbuf == Z_NULL) {
            return destroy(s), (gzFile)Z_NULL;
        }
    } else {
        err = inflateInit2(&(s->stream), -MAX_WBITS);
        s->stream.next_in  = s->inbuf = (Byte*)ALLOC(Z_BUFSIZE);

        if (err != Z_OK || s->inbuf == Z_NULL) {
            return destroy(s), (gzFile)Z_NULL;
        }
    }
    s->stream.avail_out = Z_BUFSIZE;

    errno = 0;
    //AAB s->file = fd < 0 ? FOPEN(path, mode) : fdopen(fd, mode);
    // s->pFile->Open(path, nOpenFlags);

    if (s->pFile == NULL) {
        return destroy(s), (gzFile)Z_NULL;
    }
    if (s->mode == 'w') {
        /* Write a very simple .gz header:
         */
	TCHAR header[256];
        //AAB _ftprintf(s->file, "%c%c%c%c%c%c%c%c%c%c", GZ_MAGIC_1, GZ_MAGIC_2,
        //AAB       DEFLATED, 0 /*flags*/, 0,0,0,0 /*time*/, 0 /*xflags*/, OS_CODE);
        _stprintf(header, _T("%c%c%c%c%c%c%c%c%c%c"), GZ_MAGIC_1, GZ_MAGIC_2,
               DEFLATED, 0 /*flags*/, 0,0,0,0 /*time*/, 0 /*xflags*/, OS_CODE);
		s->pFile->CFile::Write(header, 10 /*strlen(header)+1*/);
    } else {
        /* Check and skip the header:
         */
        Byte c1 = 0, c2 = 0;
        Byte method = 0;
        Byte flags = 0;
        Byte xflags = 0;
        Byte time[4];
        Byte osCode;
        int c;

        // s->stream.avail_in = fread(s->inbuf, 1, 2, s->file);
		s->stream.avail_in = s->pFile->CFile::Read(s->inbuf, 2);
        if (s->stream.avail_in != 2 || s->inbuf[0] != GZ_MAGIC_1
            || s->inbuf[1] != GZ_MAGIC_2) {
            s->transparent = 1;
            return (gzFile)s;
        }
        s->stream.avail_in = 0;
        //AAB _ftscanf(s->file,"%c%c%4c%c%c", &method, &flags, time, &xflags, &osCode);
		//AAB char strmethod[10];
		//AAB s->pFile->CFile::Read(strmethod, 8);
		s->pFile->CFile::Read(&method, 1);
		s->pFile->CFile::Read(&flags, 1);
		s->pFile->CFile::Read(&time[0], 4);
		s->pFile->CFile::Read(&xflags, 1);
		s->pFile->CFile::Read(&osCode, 1);
        //AAB _stscanf(strmethod,"%c%c%4c%c%c", &method, &flags, time, &xflags, &osCode);


        if (method != DEFLATED || (s->pFile->GetLength() == s->pFile->GetPosition()) || (flags & RESERVED) != 0) {
            s->z_err = Z_DATA_ERROR;
            return (gzFile)s;
        }
        if ((flags & EXTRA_FIELD) != 0) { /* skip the extra field */
            long len;
	    //AAB char lenbuf[3];
		//AAB s->pFile->CFile::Read(lenbuf, 2);
		s->pFile->CFile::Read(&c1, 1);
		s->pFile->CFile::Read(&c2, 1);
            //AAB _ftscanf(s->file, "%c%c", &c1, &c2);
            //AAB _stscanf(lenbuf, "%c%c", &c1, &c2);

            len = c1 + ((long)c2<<8);
            //AAB fseek(s->file, len, SEEK_CUR);
	    s->pFile->Seek(len, CFile::current);
        }
        if ((flags & ORIG_NAME) != 0) { /* skip the original file name */
            //AAB while ((c = _gettc(s->file)) != 0 && c != EOF) ;
			while ((s->pFile->CFile::Read(&c, 1)) && c != EOF );
        }
        if ((flags & COMMENT) != 0) {   /* skip the .gz file comment */
            //AAB while ((c = _gettc(s->file)) != 0 && c != EOF) ;
			while ((s->pFile->CFile::Read(&c, 1)) && c != EOF );
        }
        if ((flags & HEAD_CRC) != 0) {  /* skip the header crc */
	    char headers[3];
		s->pFile->CFile::Read(headers, 2); 
		s->pFile->CFile::Read(&c1, 1);
		s->pFile->CFile::Read(&c2, 1);
	    //AAB _stscanf(headers, "%c%c", &c1, &c2);

            // _ftscanf(s->file, "%c%c", &c1, &c2);
        }
        //AAB if (feof(s->file)) {
	if (s->pFile->GetLength() == s->pFile->GetPosition()) {
            s->z_err = Z_DATA_ERROR;
        }
    }
    return (gzFile)s;
}

/* ===========================================================================
     Opens a gzip (.gz) file for reading or writing.
*/
gzFile SECComp::gzopen (TCHAR *path, TCHAR *mode)
{
    return gz_open (path, mode, -1);
}

/* ===========================================================================
     Associate a gzFile with the file descriptor fd.
*/
gzFile SECComp::gzdopen (int fd, TCHAR *mode)
{
    TCHAR name[20];
    _stprintf(name, _T("<fd:%d>"), fd); /* for debugging */

    return gz_open (name, mode, fd);
}

/* ===========================================================================
     Reads the given number of uncompressed bytes from the compressed file.
   gzread returns the number of bytes actually read (0 for end of file).
*/
DWORD SECComp::gzread (gzFile file, void FAR *buf, long len)
{
    gz_stream *s = (gz_stream*)file;

    if (s == NULL || s->mode != 'r') return (DWORD)Z_STREAM_ERROR;

    if (s->transparent) {
        int n = 0;
        BYTE *b = (BYTE *)buf;
        /* Copy the first two (non-magic) bytes if not done already */
        while (s->stream.avail_in > 0 && len > 0) {
            *b++ = *s->stream.next_in++;
            s->stream.avail_in--;
            len--; n++;
        }
        if (len == 0) return n; 
        // long test = (long)fread(b, 1, len, s->file);
#ifdef WIN32
		long test = s->pFile->CFile::Read(b, (UINT)len);
        return n + test; 
#else
		CFile *pCheck = new CFile();
		pCheck = (s->pFile);
		long test = pCheck->ReadHuge(b, len);		
		return n+test;
#endif
     
    }
    if (s->z_err == Z_DATA_ERROR) return 0; //AAB: Changed from -1 /* bad .gz file */
    if (s->z_err == Z_STREAM_END) return 0;  /* don't read crc as data */

    s->stream.next_out = buf;
    s->stream.avail_out = len;

    while (s->stream.avail_out != 0) {

        if (s->stream.avail_in == 0 && !s->z_eof) {

            errno = 0;
            s->stream.avail_in =
                // fread(s->inbuf, 1, Z_BUFSIZE, s->file);
                s->pFile->CFile::Read( s->inbuf, Z_BUFSIZE);
            if (s->stream.avail_in == 0) {
                s->z_eof = 1;
            } else if (s->stream.avail_in == (uInt)EOF) {
                s->stream.avail_in = 0;
                s->z_eof = 1;
                s->z_err = Z_ERRNO;
                break;
            }
            s->stream.next_in = s->inbuf;
        }
        s->z_err = inflate(&(s->stream), Z_NO_FLUSH);

        if (s->z_err == Z_STREAM_END ||
            s->z_err != Z_OK  || s->z_eof) break;
    }
    len -= s->stream.avail_out;
    s->crc = crc32(s->crc, buf, (WORD)len);
    return (DWORD)len;
}

/* ===========================================================================
     Writes the given number of uncompressed bytes into the compressed file.
   gzwrite returns the number of bytes actually written (0 in case of error).
*/
long SECComp::gzwrite (gzFile file, BYTE *buf, long len)
{
    gz_stream *s = (gz_stream*)file;

    if (s == NULL || s->mode != 'w') return Z_STREAM_ERROR;

    s->stream.next_in = buf;
    s->stream.avail_in = len;

    while (s->stream.avail_in != 0) {

        if (s->stream.avail_out == 0) {

            s->stream.next_out = s->outbuf;
            //AAB if (fwrite(s->outbuf, 1, Z_BUFSIZE, s->file) != Z_BUFSIZE) {
			s->pFile->CFile::Write((BYTE *)s->outbuf, Z_BUFSIZE ); 

            s->stream.avail_out = Z_BUFSIZE;
        }
        s->z_err = deflate(&(s->stream), Z_NO_FLUSH);
        if (s->z_err != Z_OK) break;
    }
    s->crc = crc32(s->crc, buf, (WORD)len);

    return (long)(len - s->stream.avail_in);
}

/* ===========================================================================
     Flushes all pending output into the compressed file. The parameter
   flush is as in the deflate() function.
     gzflush should be called only when strictly necessary because it can
   degrade compression.
*/
int SECComp::gzflush (gzFile file, int flush)
{
    long len;
    int done = 0;
    gz_stream *s = (gz_stream*)file;

    if (s == NULL || s->mode != 'w') return Z_STREAM_ERROR;

    s->stream.avail_in = 0; /* should be zero already anyway */

    for (;;) {
        len = Z_BUFSIZE - s->stream.avail_out;

        if (len != 0) {
            //AAB if (fwrite(s->outbuf, 1, len, s->file) != len) {
			s->pFile->CFile::Write(s->outbuf, len);

            s->stream.next_out = s->outbuf;
            s->stream.avail_out = Z_BUFSIZE;
        }
        if (done) break;
        s->z_err = deflate(&(s->stream), flush);

        /* deflate has finished flushing only when it hasn't used up
         * all the available space in the output buffer: 
         */
        done = (s->stream.avail_out != 0 || s->z_err == Z_STREAM_END);
 
        if (s->z_err != Z_OK && s->z_err != Z_STREAM_END) break;
    }
    //AAB fflush(s->file);
    s->pFile->Flush();
    return  s->z_err == Z_STREAM_END ? Z_OK : s->z_err;
}

/* ===========================================================================
   Outputs a long in LSB order to the given file
*/
void SECComp::putLong (CFile *pFile, DWORD x)
{
    int n;
    for (n = 0; n < 4; n++) {
        // _fputtc((int)(x & 0xff), file);
	char cOutchar;
	cOutchar = (char)(x&0xff);
	pFile->CFile::Write(&cOutchar, 1);
        x >>= 8;
    }
}

/* ===========================================================================
   Reads a long in LSB order from the given buffer
*/
DWORD SECComp::getLong (BYTE *buf)
{
    DWORD x = 0;
    BYTE *p = buf+4;

    do {
        x <<= 8;
        x |= *--p; 
    } while (p != buf);
    return x;
}

/* ===========================================================================
     Flushes all pending output if necessary, closes the compressed file
   and deallocates all the (de)compression state.
*/
int SECComp::gzclose (gzFile file)
{
    WORD n;
    int err;
    gz_stream *s = (gz_stream*)file;

    if (s == NULL) return Z_STREAM_ERROR;

    if (s->mode == 'w') {
        err = gzflush (file, Z_FINISH);
        //AAB if (err != Z_OK) return destroy(file);
        if (err != Z_OK) return destroy(s);

        putLong (s->pFile, s->crc);
        putLong (s->pFile, s->stream.total_in);

    } else if (s->mode == 'r' && s->z_err == Z_STREAM_END) {

        /* slide CRC and original size if they are at the end of inbuf */
        if ((n = (WORD)s->stream.avail_in) < 8  && !s->z_eof) {
            Byte *p = s->inbuf;
	    Bytef *q = s->stream.next_in;
            while (n--) { *p++ = *q++; };

            n = (WORD)s->stream.avail_in;
            // n += fread(p, 1, 8, s->file);
            n = (WORD)((WORD)n + (WORD)s->pFile->CFile::Read( p, 8));
            s->stream.next_in = s->inbuf;
        }
        /* check CRC and original size */
        if (n < 8 ||
            getLong(s->stream.next_in) != s->crc ||
            getLong(s->stream.next_in + 4) != s->stream.total_out) {

            s->z_err = Z_DATA_ERROR;
        }
    }
    //AAB return destroy(file);
    return destroy(s);
}

/* ===========================================================================
     Returns the error message for the last error which occured on the
   given compressed file. errnum is set to zlib error number. If an
   error occured in the file system and not in the compression library,
   errnum is set to Z_ERRNO and the application may consult errno
   to get the exact error code.
*/
TCHAR*  gzerror (gzFile file, int *errnum)
{
    TCHAR *m;
    gz_stream *s = (gz_stream*)file;

    if (s == NULL) {
        *errnum = Z_STREAM_ERROR;
        return z_errmsg[1-Z_STREAM_ERROR];
    }
    *errnum = s->z_err;
    if (*errnum == Z_OK) return _T("");

    m =  *errnum == Z_ERRNO ? zstrerror(errno) : s->stream.msg;

    if (m == NULL || *m == '\0') m = z_errmsg[1-s->z_err];

    TRYFREE(s->msg);
    s->msg = (TCHAR*)ALLOC(_tcsclen(s->path) + _tcsclen(m) + 3);
    _tcscpy(s->msg, s->path);
    _tcscat(s->msg, _T(": "));
    _tcscat(s->msg, m);
    return s->msg;
}
