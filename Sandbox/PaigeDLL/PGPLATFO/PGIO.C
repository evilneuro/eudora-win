/* 
	Paige Universal I/O
	
	File:		pgIO.c

	by T. R. Shaw

	Copyright й 1995 OITC, Inc.
	All rights reserved.
		
	pgIO.c - Universal I/O routines

	This software has been privately developed and falls within 
	DFARS 252.227-7013(c)(1) and associated regulations and it or its
	derivatives may not be used outside of the License agreement with 
	T. R. Shaw and OITC.  Copyright may not be removed.
	
	oitc@iu.net

	Revision History
	03/27/95	1.0b1	TRS - Initial beta for external customer Paige release
	03/18/96	1.4 GC - Moved pgScrapMemoryRead/Write
*/

#include "pgIO.h"
#include "pgosutl.h"
#include "defprocs.h"


/* Names indicate functions in this file */


#ifdef MAC_PLATFORM

#include <Aliases.h>
#include <Files.h>

#pragma segment pgbasic4


/* Create a file descriptor */

PG_C (pg_file_desc_ref) pgFileSpec2FileDescriptor(const pgm_globals_ptr mem_globals, const pg_file_desc_ptr spec)
{
	pg_file_desc_ref	ref;

	ref = MemoryAlloc (mem_globals, sizeof(FSSpec), 1, 0);
	*((FSSpecPtr)UseMemory(ref)) = *spec;
	UnuseMemory(ref);
	
	return ref;
}


/* Create a file descriptor */

PG_C (pg_file_desc_ref) pgCreateFileDescriptor(const pgm_globals_ptr mem_globals, const pg_file_name_ptr file_name)
{
	FSSpec				spec;
	
	pgFailError(mem_globals, FSMakeFSSpec(0, 0, file_name, &spec));
	
	return pgFileSpec2FileDescriptor(mem_globals, &spec);
}


#ifdef NO_C_INLINE

/* Dispose of a file descriptor */

PG_C (void) pgDisposeFileDescriptor(pg_file_desc_ref ref)
{
	DisposeNonNilFailedMemory(ref);
}

#endif


/* Create a file and open it */

PG_C (pg_error) pgCreateFile(pg_file_desc_ref ref, short perm, pg_file_unit_ptr ref_num)
{
	long		type = GetGlobalsFromRef(ref)->fileType;
	long		creator = GetGlobalsFromRef(ref)->creator;
	pg_error	err;
	
	if (perm & PG_BINARY)
	{
		if (!type) 
			type = PG_BINARY_TYPE;
	}
	else type = 'TEXT';
	
	if (!creator) 
		creator = PG_IO_CREATOR;

	if ((err = FSpCreate((FSSpecPtr)UseMemory(ref), creator, type, smSystemScript)) == dupFNErr)
	{
		if (perm & PG_EXCL)
			err = NO_ERROR;
		if (!err)
			err = pgOpenFile(ref, perm, ref_num);
	}
	else
	if (err == NO_ERROR)
		err = pgOpenFile(ref, perm, ref_num);

	return err;
}


/* Open a file */

PG_C (pg_error) pgOpenFile(pg_file_desc_ref ref, short perm, pg_file_unit_ptr ref_num)
{
	FInfo		fndrInfo;
	FSSpecPtr	spec;
	pg_error	err;
	Boolean		wasAliased;
	Boolean		targetIsFolder;
		
	spec = UseMemory(ref);
	
	err = ResolveAliasFile(spec, TRUE, &targetIsFolder, &wasAliased);
	if (targetIsFolder)
		err = fnfErr;
	if (!err)
	{
		if (!(err = FSpGetFInfo(spec, &fndrInfo)))
		{
			if (!(perm & PG_BINARY))
				fndrInfo.fdType = 'TEXT';
			
			if (!(err = FSpSetFInfo(spec, &fndrInfo)))
			{
				SignedByte permission;
				
				switch (perm & PG_RDWR)
				{
					case PG_RDONLY:
						permission = fsRdPerm;
						break;
	
					case PG_WRONLY:
						permission = fsWrPerm;
						break;
	
					case PG_RDWR:
						permission = fsRdWrPerm;
						break;

					default:
						permission = fsCurPerm;
						break;
				}
				
				if (!(err = FSpOpenDF(spec, permission, ref_num)))
				{
					if (err == fnfErr && (perm & PG_CREAT))
						err = pgCreateFile(ref, perm, ref_num);
					
					if (!err)
					{
						if (perm & PG_APPEND)
							err = SetFPos(*ref_num, fsFromLEOF, 0);
						if (perm & PG_TRUNC)
							err = SetEOF(*ref_num, 0);
					}
				}
			}
		}
	}
	
	UnuseMemory(ref);
	
	return err;
}


/* Close a file */

PG_C (pg_error) pgCloseFile(pg_file_unit ref_num)
{
	return FSClose(ref_num);
}


/* Delete a file */

PG_C (pg_error) pgDeleteFile(pg_file_desc_ref ref)
{
	pg_error	err;
	
	err = FSpDelete((FSSpecPtr)UseMemory(ref));
	UnuseMemory(ref);
	
	return err;
}


/* Rename a file */

PG_C (pg_error) pgRenameFile(pg_file_desc_ref ref, const pg_file_name_ptr new_file_name)
{
	pg_error	err;
	
	err = FSpRename((FSSpecPtr)UseMemory(ref), new_file_name);
	UnuseMemory(ref);

	return err;
}


/* Create and open a temporary file */

PG_C (pg_file_desc_ref) pgOpenTempFile(const pgm_globals_ptr mem_globals, pg_file_unit_ptr temp_ref_num)
{
	FSSpec				spec;
	pg_error			err;
	pg_file_desc_ref	temp_ref = MEM_NULL;
	
	pgFailError(mem_globals, FindFolder(kOnSystemDisk, kTemporaryFolderType, kCreateFolder, &spec.vRefNum, &spec.parID));
	
	pgBlockMove("\pPaigeTmp", spec.name, 10);
	spec.name[++spec.name[0]] = '.';
	spec.name[++spec.name[0]] = 'A';
	
	while ((err = FSpCreate(&spec, PG_IO_CREATOR, PG_TEMP_TYPE, smSystemScript)) != NO_ERROR)
		spec.name[spec.name[0]]++;
	
	pgFailError(mem_globals, err);
	
	err = FSpOpenDF(&spec, fsRdWrPerm, temp_ref_num);
		
	pgFailError(mem_globals, err);

	temp_ref = MemoryAlloc (mem_globals, sizeof(FSSpec), 1, 0);
	*((FSSpecPtr)UseMemory(temp_ref)) = spec;
	UnuseMemory(temp_ref);
	
	return temp_ref;
}
		

#ifdef NO_C_INLINE

PG_C (pg_error) pgGetFileEOF(pg_file_unit ref_num, long PG_FAR *offset_result)
{
	return  GetEOF(ref_num, offset_result);
}


PG_C (pg_error) pgSetFileEOF(pg_file_unit ref_num, long offset)
{
	return  SetEOF(ref_num, offset);
}


PG_C (pg_error) pgGetFilePos(pg_file_unit ref_num, long PG_FAR *offset_result)
{
	return  GetFPos(ref_num, offset_result);
}


PG_C (pg_error) pgSetFilePos(pg_file_unit ref_num, long offset)
{
    return SetFPos(ref_num, fsFromStart, offset);
}


PG_C (pg_error) pgReadFileBytes(pg_file_unit ref_num, long PG_FAR *byte_size, void PG_FAR *buffer)
{
    return FSRead(ref_num, byte_size, buffer);
}


PG_C (pg_error) pgWriteFileBytes(pg_file_unit ref_num, long PG_FAR *byte_size, void PG_FAR *buffer)
{
    return FSWrite(ref_num, byte_size, buffer);
}

#endif


#endif

#ifdef WINDOWS_PLATFORM

#include <stdio.h>


/* Create a file descriptor */

PG_C (pg_file_desc_ref) pgFileSpec2FileDescriptor(const pgm_globals_ptr mem_globals, const pg_file_desc_ptr spec)
{
	return CString2Memory (mem_globals, (pg_c_string_ptr)spec);
}


/* Create a file descriptor */

PG_C (pg_file_desc_ref) pgCreateFileDescriptor(const pgm_globals_ptr mem_globals, const pg_file_name_ptr file_name)
{	
	return CString2Memory (mem_globals, file_name);
}


#ifdef NO_C_INLINE

/* Dispose of a file descriptor */

PG_C (void) pgDisposeFileDescriptor(pg_file_desc_ref ref)
{
	DisposeNonNilFailedMemory(ref);
}

#endif


// create a file
PG_C (pg_error) pgCreateFile(pg_file_desc_ref ref, short perm, pg_file_unit_ptr ref_num)
{
	pg_error	err = NO_ERROR;
	
	if ((*ref_num = _lcreat((LPCSTR)UseMemory(ref), 0)) == HFILE_ERROR)
	{
		err = (pg_error)HFILE_ERROR;
	}
	UnuseMemory(ref);
	
	return err;
}


// open a file
PG_C (pg_error) pgOpenFile(pg_file_desc_ref ref, short perm, pg_file_unit_ptr ref_num)
{
	pg_error	err = NO_ERROR;
	
	if ((*ref_num = _lopen((LPCSTR)UseMemory(ref), perm)) == HFILE_ERROR)
	{
		err = (pg_error)HFILE_ERROR;
	}
	UnuseMemory(ref);

	return err;
}


// close a file
PG_C (pg_error) pgCloseFile(pg_file_unit ref_num)
{
	pg_error	err = NO_ERROR;
	
	if (_lclose(ref_num) == HFILE_ERROR)
	{
		err = (pg_error)HFILE_ERROR;
	}
	return err;
}


// delete a file
PG_C (pg_error) pgDeleteFile(pg_file_desc_ref ref)
{
	pg_error	err = NO_ERROR;
	
	if (remove((LPCSTR)UseMemory(ref)) == HFILE_ERROR)
	{
		err = (pg_error)HFILE_ERROR;
	}
	UnuseMemory(ref);

	return err;
}


// rename a file
PG_C (pg_error) pgRenameFile(pg_file_desc_ref ref, const pg_file_name_ptr new_file_name)
{
	pg_error	err = NO_ERROR;
	
	if (rename((LPCSTR)UseMemory(ref), (LPCSTR)new_file_name) == HFILE_ERROR)
	{
		err = (pg_error)HFILE_ERROR;
	}
	UnuseMemory(ref);

	return err;
}


// open temporary a file
PG_C (pg_file_desc_ref) pgOpenTempFile(const pgm_globals_ptr mem_globals, pg_file_unit_ptr temp_ref_num)
{
	pg_file_name_ptr	p;
	long				len;
	pg_error			err = NO_ERROR;
	short				cnt = 200;
	pg_file_desc_ref	temp_ref = MEM_NULL;
	
	p = (pg_file_name_ptr)tmpnam(NULL);
	while ((*temp_ref_num = _lopen((LPCSTR)p, 0 /*ееее*/)) == HFILE_ERROR)
	{
		if (!--cnt)
			break;
	}
	if (*temp_ref_num == HFILE_ERROR)
	{
		err = (pg_error)HFILE_ERROR;
	}
	
	pgFailError(mem_globals, err);

	len = pgCStrLength(p) + 1;
	temp_ref = MemoryAlloc (mem_globals, sizeof(pg_char), len, 0);
	pgBlockMove(p, (pg_bits8_ptr)UseMemory(temp_ref), len * sizeof(pg_char));
	UnuseMemory(temp_ref);
	
	return temp_ref;
}


#ifdef NO_C_INLINE

PG_C (pg_error) pgGetFileEOF(pg_file_unit ref_num, long PG_FAR *offset_result)
{
	if ((*offset_result = _llseek(ref_num, 0L, SEEK_END)) == EOF)
		return EOF;
    return NO_ERROR;
}




PG_C (pg_error) pgGetFilePos(pg_file_unit ref_num, long PG_FAR *offset_result)
{
	if ((*offset_result = _llseek(ref_num, 0L, SEEK_CUR)) == EOF)
		return EOF;
    return NO_ERROR;
}


PG_C (pg_error) pgSetFilePos(pg_file_unit ref_num, long offset)
{
	if (_llseek(ref_num, offset, SEEK_SET) == EOF)
		return EOF;
    return NO_ERROR;
}


#endif

PG_C (pg_error) pgSetFileEOF(pg_file_unit ref_num, long offset)
{
	pg_error err;

	err = pgSetFilePos(ref_num, offset);
	if (err == NO_ERROR)
		err = _lwrite(ref_num, (void PG_FAR *)&err, 0);

    return err;
}


PG_C (pg_error) pgWriteFileBytes(pg_file_unit ref_num, long PG_FAR *byte_size, void PG_FAR *buffer)
{
	long		byte_count = *byte_size;
	pg_error	err = NO_ERROR;
	
	byte_count = _hwrite(ref_num, buffer, byte_count);
	if (*byte_size != byte_count)
	{
		return (pg_error)HFILE_ERROR;
	}

	*byte_size = byte_count;

    return err;
}


PG_C (pg_error) pgReadFileBytes(pg_file_unit ref_num, long PG_FAR *byte_size, void PG_FAR *buffer)
{
	long	byte_count = *byte_size;
	
	byte_count = _hread(ref_num, buffer, byte_count);
	*byte_size = byte_count;
	if (byte_count <= 0)
	{
		if (!byte_count)
			return EOF_ERR;
		else return (pg_error)HFILE_ERROR;
	}
    return NO_ERROR;
}


#endif


PG_C (pg_error) pgReadFileData(pg_file_unit ref_num, long byte_size, void PG_FAR *buffer)
{
	long	byte_count = byte_size;
	
	return pgReadFileBytes(ref_num, &byte_count, buffer);
}


PG_C (pg_error) pgWriteFileData(pg_file_unit ref_num, long byte_size, const void PG_FAR *buffer)
{
	long	byte_count = byte_size;
	
	return pgWriteFileBytes(ref_num, &byte_count, (void PG_FAR *)buffer);
}


PG_C (void) pgSetTypeCreator(const pgm_globals_ptr mem_globals, long creator, long fileType)
{
    mem_globals->creator = creator;
    mem_globals->fileType = fileType;
}



/* This is a "fake" file I/O proc that sends the data to a memory_ref instead of
a file.	*/

PG_PASCAL (pg_error) pgScrapMemoryWrite (void PG_FAR *data, short verb, long PG_FAR *position,
		long PG_FAR *data_size, file_ref filemap)
{
	pg_bits8_ptr		new_data, source_data;
	long				ref_size;

	if (verb == io_set_fpos)
		return	NO_ERROR;
	
	if (verb == io_set_eof) {

		SetMemorySize(filemap, *position);
		return	NO_ERROR;
	}

	if (verb == io_data_indirect)
		source_data = UseMemory((memory_ref) data);
	else
		source_data = (pg_bits8_ptr) data;
	
	ref_size = GetMemorySize(filemap);

	if (ref_size > *position) {
		
		if ((*position + *data_size) > ref_size)
			SetMemorySize(filemap, *position + *data_size);

		new_data = UseMemoryRecord(filemap, *position, USE_ALL_RECS, TRUE);
	}
	else
		new_data = AppendMemory(filemap, *data_size, FALSE);

	pgBlockMove(source_data, new_data, *data_size);
	UnuseMemory(filemap);
	
	if (verb == io_data_indirect)
		UnuseMemory((memory_ref)data);

	*position += *data_size;

	return	NO_ERROR;
}



/* pgScrapMemoryRead is a "fake" file I/O proc that reads the data into a memory_ref instead of
a file.  */

PG_PASCAL (pg_error) pgScrapMemoryRead (void PG_FAR *data, short verb, long PG_FAR *position,
		long PG_FAR *data_size, file_ref filemap)
{
	long PG_FAR		*ptr_to_long;

	if (verb == io_set_fpos)
		return	NO_ERROR;
	
	if (verb == io_file_unit) {
		
		ptr_to_long = (long PG_FAR *)data;
		*ptr_to_long = (long)filemap;
		
		*((pg_file_unit PG_FAR *)data) = (pg_file_unit)filemap;
		return	NO_ERROR;
	}
	else
	if (verb == io_get_eof) {
		
		ptr_to_long = (long PG_FAR *)data;
		*ptr_to_long = GetMemorySize(filemap);
	}
	else {
		pg_bits8_ptr	the_data, target_data;
		
		the_data = UseMemory(filemap);
		the_data += *position;
		
		if (verb == io_data_indirect) {
			
			SetMemorySize((memory_ref) data, *data_size);
			target_data = UseMemory((memory_ref) data);
		}
		else
			target_data = data;

		pgBlockMove(the_data, target_data, *data_size);
		UnuseMemory(filemap);
		
		if (verb == io_data_indirect)
			UnuseMemory((memory_ref)data);

		*position += *data_size;
	}
	
	return	NO_ERROR;
}



