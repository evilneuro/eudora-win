#if !defined EngineHIncluded
#define EngineHIncluded

#include "Platform.h"
#if enginePlatform == enginePlatformWindowsIntel16
#include "sepream.h"
#include "setypes.h"
#else
#include "EnginePreamble.h"
#include "EngineTypes.h"
#endif

beginEngineNamespace

// miscellaneous
engineAPI int32 getEngineMajorVersion();
engineAPI int32 getEngineMinorVersion();
engineAPI int32 getEngineBuildNumber();
engineAPI int32 getEngineBuildType();
engineAPI int32 getEngineDebugLevel();
engineAPI int32 getEngineReleaseYear();
engineAPI int32 getEngineReleaseMonth();
engineAPI int32 getEngineReleaseDay();
engineAPI int32 getEngineReleaseHour();
engineAPI int32 getEngineReleaseMinute();

engineAPI const char* getEngineVersionString();
engineAPI const char* getEngineFormatString(engineFormat format);
engineAPI const char* getEngineFormatStringLong(engineFormat format);
engineAPI const char* getEngineErrorString(engineError error);
engineAPI const char* getEngineErrorStringLong(engineError error);

// archive reader
engineAPI archiveReader newArchiveReader();
#define noArchiveReader ((archiveReader)0)
engineAPI void deleteArchiveReader(archiveReader);

engineAPI bool openArchive(const char* archiveName, archiveReader); // opens archive for reading
engineAPI bool classifyArchive(archiveReader); // determines archive type and position
engineAPI bool scanArchive(archiveReader); // verifies the archive and prepares for expansion
engineAPI bool decodeArchive(archiveReader); // decodes the archive
engineAPI bool closeArchive(archiveReader); // closes archive

// formats that can be recognized
// a format may have a classifier but not a reader, or have a reader but not a writer
// initially all formats that are present in the build are enabled
// formats may or may not be enabled depending on whether their respecitve code is present in the build
// formats are always disabled when requested
engineAPI bool canClassify(engineFormat format, archiveReader);
engineAPI bool willClassify(engineFormat format, archiveReader);
engineAPI bool doClassify(engineFormat format, archiveReader); // returns true if enabled
engineAPI void doNotClassify(engineFormat format, archiveReader);
engineAPI bool doClassifyAll(archiveReader); // returns true if all enabled
engineAPI void doNotClassifyAny(archiveReader);

// formats that can be decoded
engineAPI bool canRead(engineFormat format, archiveReader);
engineAPI bool willRead(engineFormat format, archiveReader);
engineAPI bool doRead(engineFormat format, archiveReader);
engineAPI void doNotRead(engineFormat format, archiveReader);
engineAPI bool doReadAll(archiveReader);
engineAPI void doNotReadAny(archiveReader);

engineAPI char getRealPathChar(archiveReader);
engineAPI void setRealPathChar(char c, archiveReader);
engineAPI char getErsatzPathChar(archiveReader);
engineAPI void setErsatzPathChar(char c, archiveReader);

engineAPI engineTextOut getReaderTextOut(archiveReader);
engineAPI void setReaderTextOut(engineTextOut textOut, archiveReader);
engineAPI engineMacBinaryOut getReaderMacBinaryOut(archiveReader);
engineAPI void setReaderMacBinaryOut(engineMacBinaryOut macBinaryOut, archiveReader);
engineAPI engineOnConflict getReaderOnConflict(archiveReader);
engineAPI void setReaderOnConflict(engineOnConflict onConflict, archiveReader);
engineAPI const char* getReaderDestination(archiveReader);
engineAPI void setReaderDestination(const char* folder, archiveReader);

engineAPI void setReaderUserData(void* userData, archiveReader);
engineAPI void* getReaderUserData(archiveReader);
engineAPI const char* getReaderArchiveName(archiveReader);
engineAPI const char* getReaderFileName(archiveReader);
engineAPI void setReaderFileName(const char* fileName, archiveReader);

engineAPI void setReaderArchivePosition(uint32 position, archiveReader);
engineAPI uint32 getReaderArchivePosition(archiveReader);
engineAPI void setClassifierSpan(size_t span, archiveReader);
engineAPI size_t getClassifierSpan(archiveReader);
engineAPI engineFormat getReaderArchiveFormat(archiveReader);

engineAPI bool setReaderPassword(const char* password, archiveReader);
engineAPI void deleteReaderPassword(archiveReader);
engineAPI bool setReaderKey(const char* key, size_t keySize, archiveReader);
engineAPI bool setReaderKeySize(size_t keySize, archiveReader);
engineAPI void deleteReaderKey(archiveReader);

engineAPI uint32 getReaderItems(archiveReader);
engineAPI bool readerHasMultipleRoots(archiveReader);
engineAPI const char* getReaderRootName(archiveReader);
engineAPI uint32 getReaderCompSize(archiveReader);
engineAPI uint32 getReaderUncompSize(archiveReader);

engineAPI void setReaderPositionList(uint32 positionList[], archiveReader);
#define enginePositionListEnd ((uint32)-1)

engineAPI engineError getReaderError(archiveReader);
engineAPI int32 getReaderExtError(archiveReader);

// callbacks
// all callbacks are optional--do not use set...() for those callbacks that you do not need
// in all callbacks having bool return type, false means "abort processing and return to caller",
// true means "continue processing"

typedef bool (*readerError)(engineError error, archiveReader);
// called when a non-fatal or recoverable error occurs
typedef void (*readerProgressScanBegin)(archiveReader);
// called before scan of an archive begin
typedef bool (*readerProgressScanStep)(archiveReader);
// periodically called during scan of an archive
typedef void (*readerProgressScanEnd)(archiveReader);
// called when scan of an archive is complete
typedef void (*readerProgressSizeBegin)(uint32 size, archiveReader);
// called to initialize progress size bar
typedef bool (*readerProgressSizeMove)(uint32 size, archiveReader);
// called to update progress size bar
typedef void (*readerProgressSizeEnd)(archiveReader);
// called when decoding/encoding of an archive is complete
typedef void (*readerProgressFilesBegin)(int32 files, archiveReader);
// called when the total number of files is known
typedef bool (*readerProgressFilesDone)(int32 files, archiveReader);
// called after each file is processed
typedef void (*readerProgressFilesEnd)(archiveReader);
// called when decoding/encoding of an archive is complete
typedef bool (*readerArchiveNext)(archiveReader);
// called to obtain a name of a subsequent archive segment
typedef bool (*readerArchiveInfo)(archiveReader);
// called when archive information is available
typedef bool (*readerArchiveDecodeBegin)(archiveReader);
// called before decoding of an archive begins
typedef void (*readerArchiveDecodeEnd)(archiveReader);
// called when decoding of an archive is complete
typedef bool (*readerFileInfo)(uint32 position, archiveReader);
// called when file information is available
typedef bool (*readerFileDecodeBegin)(uint32 position, archiveReader);
// called before decoding of a file begins
typedef void (*readerFileDecodeEnd)(archiveReader);
// called when decoding of a file is complete
typedef void (*readerFileNewName)(archiveReader);
// called when archive format does not contain any file name
typedef void (*readerFileChangedName)(const char* fileName, archiveReader);
// called when output fileName has been changed by the engine
typedef bool (*readerFileDelete)(archiveReader);
// called to ask whether an incomplete or damaged file has to be deleted

// Enable reader callbacks by calling these functions
engineAPI void setReaderError(readerError, archiveReader);
engineAPI void setReaderProgressScanBegin(readerProgressScanBegin, archiveReader);
engineAPI void setReaderProgressScanStep(readerProgressScanStep, archiveReader);
engineAPI void setReaderProgressScanEnd(readerProgressScanEnd, archiveReader);
engineAPI void setReaderProgressSizeBegin(readerProgressSizeBegin, archiveReader);
engineAPI void setReaderProgressSizeMove(readerProgressSizeMove, archiveReader);
engineAPI void setReaderProgressSizeEnd(readerProgressSizeEnd, archiveReader);
engineAPI void setReaderProgressFilesBegin(readerProgressFilesBegin, archiveReader);
engineAPI void setReaderProgressFilesDone(readerProgressFilesDone, archiveReader);
engineAPI void setReaderProgressFilesEnd(readerProgressFilesEnd, archiveReader);
engineAPI void setReaderArchiveNext(readerArchiveNext, archiveReader);
engineAPI void setReaderArchiveInfo(readerArchiveInfo, archiveReader);
engineAPI void setReaderArchiveDecodeBegin(readerArchiveDecodeBegin, archiveReader);
engineAPI void setReaderArchiveDecodeEnd(readerArchiveDecodeEnd, archiveReader);
engineAPI void setReaderFileInfo(readerFileInfo, archiveReader);
engineAPI void setReaderFileDecodeBegin(readerFileDecodeBegin, archiveReader);
engineAPI void setReaderFileDecodeEnd(readerFileDecodeEnd, archiveReader);
engineAPI void setReaderFileNewName(readerFileNewName, archiveReader);
engineAPI void setReaderFileChangedName(readerFileChangedName, archiveReader);
engineAPI void setReaderFileDelete(readerFileDelete, archiveReader);

// disable reader callbacks by calling these functions
engineAPI void resetReaderError(archiveReader);
engineAPI void resetReaderProgressScanBegin(archiveReader);
engineAPI void resetReaderProgressScanStep(archiveReader);
engineAPI void resetReaderProgressScanEnd(archiveReader);
engineAPI void resetReaderProgressSizeBegin(archiveReader);
engineAPI void resetReaderProgressSizeMove(archiveReader);
engineAPI void resetReaderProgressSizeEnd(archiveReader);
engineAPI void resetReaderProgressFilesBegin(archiveReader);
engineAPI void resetReaderProgressFilesDone(archiveReader);
engineAPI void resetReaderProgressFilesEnd(archiveReader);
engineAPI void resetReaderArchiveNext(archiveReader);
engineAPI void resetReaderArchiveInfo(archiveReader);
engineAPI void resetReaderArchiveDecodeBegin(archiveReader);
engineAPI void resetReaderArchiveDecodeEnd(archiveReader);
engineAPI void resetReaderFileInfo(archiveReader);
engineAPI void resetReaderFileDecodeBegin(archiveReader);
engineAPI void resetReaderFileDecodeEnd(archiveReader);
engineAPI void resetReaderFileNewName(archiveReader);
engineAPI void resetReaderFileChangedName(archiveReader);
engineAPI void resetReaderFileDelete(archiveReader);

engineAPI void resetAllReaderCallbacks(archiveReader);

// archive writer
engineAPI archiveWriter newArchiveWriter();
#define noArchiveWriter ((archiveWriter)0)
engineAPI void deleteArchiveWriter(archiveWriter);

engineAPI bool createArchive(const char* fileList[], archiveWriter); // creates the archive
#define engineFileListEnd ((const char*)0)

// formats that can be created
engineAPI bool canWrite(engineFormat format, archiveWriter);
engineAPI bool willWrite(engineFormat format, archiveWriter);
engineAPI bool doWrite(engineFormat format, archiveWriter);
engineAPI void doNotWrite(engineFormat format, archiveWriter);
engineAPI bool doWriteAll(archiveWriter);
engineAPI void doNotWriteAny(archiveWriter);

engineAPI engineTextOut getWriterTextOut(archiveWriter);
engineAPI void setWriterTextOut(engineTextOut textOut, archiveWriter);
engineAPI engineCompLevel getWriterCompLevel(archiveWriter);
engineAPI void setWriterCompLevel(engineCompLevel compLevel, archiveWriter);
engineAPI engineOnConflict getWriterOnConflict(archiveWriter);
engineAPI void setWriterOnConflict(engineOnConflict onConflict, archiveWriter);

engineAPI bool setWriterPassword(const char* password, archiveReader);
engineAPI void deleteWriterPassword(archiveReader);
engineAPI bool setWriterKey(const char* key, size_t keySize, archiveReader);
engineAPI bool setWriterKeySize(size_t keySize, archiveReader);
engineAPI void deleteWriterKey(archiveReader);

engineAPI void setWriterUserData(void* userData, archiveWriter);
engineAPI void* getWriterUserData(archiveWriter);
engineAPI const char* getWriterArchiveName(archiveWriter);
engineAPI const char* getWriterFileName(archiveWriter);
engineAPI void setWriterFileName(const char* fileName, archiveWriter);

engineAPI void setWriterArchiveFormat(engineFormat format, archiveWriter);
engineAPI void setWriterArchiveName(const char* archiveName, archiveWriter);

engineAPI void setWriterMacType(uint32 macType, archiveWriter);
engineAPI void setWriterMacCreator(uint32 macCreator, archiveWriter);

engineAPI engineError getWriterError(archiveWriter);
engineAPI int32 getWriterExtError(archiveWriter);

// callbacks
typedef bool (*writerError)(engineError error, archiveWriter);
// called when a non-fatal or recoverable error occurs
typedef void (*writerProgressSizeBegin)(uint32 size, archiveWriter);
// called to initialize progress size bar
typedef bool (*writerProgressSizeMove)(uint32 size, archiveWriter);
// called to update progress size bar
typedef void (*writerProgressSizeEnd)(archiveWriter);
// called when decoding/encoding of an archive is complete
typedef void (*writerProgressFilesBegin)(int32 files, archiveWriter);
// called when the total number of files is known
typedef bool (*writerProgressFilesDone)(int32 files, archiveWriter);
// called after each file is processed
typedef void (*writerProgressFilesEnd)(archiveWriter);
// called when decoding/encoding of an archive is complete
typedef bool (*writerArchiveCreateBegin)(archiveWriter);
// called before a new archive is created
typedef void (*writerArchiveChangedName)(const char* archiveName, archiveWriter);
// called before the archive is renamed
typedef void (*writerArchiveCreateEnd)(archiveWriter);
// called after the archive is created
typedef void (*writerArchiveSize)(archiveWriter);
// called to obtain the size of a new archive segment
typedef bool (*writerFileScan)(archiveWriter);
// called when a file is scanned for inclusion in an archive
typedef bool (*writerFileEncodeBegin)(archiveWriter);
// called before a file is added to an archive
typedef void (*writerFileEncodeEnd)(archiveWriter);
// called after a file has been added to an archive
typedef bool (*writerFileDelete)(archiveWriter);
// called to ask whether an incomplete or damaged file has to be deleted

// enable writer callbacks by calling these functions
engineAPI void setWriterError(writerError, archiveWriter);
engineAPI void setWriterProgressSizeBegin(writerProgressSizeBegin, archiveWriter);
engineAPI void setWriterProgressSizeMove(writerProgressSizeMove, archiveWriter);
engineAPI void setWriterProgressSizeEnd(writerProgressSizeEnd, archiveWriter);
engineAPI void setWriterProgressFilesBegin(writerProgressFilesBegin, archiveWriter);
engineAPI void setWriterProgressFilesDone(writerProgressFilesDone, archiveWriter);
engineAPI void setWriterProgressFilesEnd(writerProgressFilesEnd, archiveWriter);
engineAPI void setWriterArchiveCreateBegin(writerArchiveCreateBegin, archiveWriter);
engineAPI void setWriterArchiveChangedName(writerArchiveChangedName, archiveWriter);
engineAPI void setWriterArchiveCreateEnd(writerArchiveCreateEnd, archiveWriter);
engineAPI void setWriterArchiveSize(writerArchiveSize, archiveWriter);
engineAPI void setWriterFileScan(writerFileScan, archiveWriter);
engineAPI void setWriterFileEncodeBegin(writerFileEncodeBegin, archiveWriter);
engineAPI void setWriterFileEncodeEnd(writerFileEncodeEnd, archiveWriter);
engineAPI void setWriterFileDelete(writerFileDelete, archiveWriter);

// disable writer callbacks by calling these functions
engineAPI void resetWriterError(archiveWriter);
engineAPI void resetWriterProgressSizeBegin(archiveWriter);
engineAPI void resetWriterProgressSizeMove(archiveWriter);
engineAPI void resetWriterProgressSizeEnd(archiveWriter);
engineAPI void resetWriterProgressFilesBegin(archiveWriter);
engineAPI void resetWriterProgressFilesDone(archiveWriter);
engineAPI void resetWriterProgressFilesEnd(archiveWriter);
engineAPI void resetWriterArchiveCreateBegin(archiveWriter);
engineAPI void resetWriterArchiveChangedName(archiveWriter);
engineAPI void resetWriterArchiveCreateEnd(archiveWriter);
engineAPI void resetWriterArchiveSize(archiveWriter);
engineAPI void resetWriterFileScan(archiveWriter);
engineAPI void resetWriterFileEncodeBegin(archiveWriter);
engineAPI void resetWriterFileEncodeEnd(archiveWriter);
engineAPI void resetWriterFileDelete(archiveWriter);

engineAPI void resetAllWriterCallbacks(archiveWriter);

// linkage to C++ and Java classes
// these are used by superclasses, do not call from subclasses
engineAPI void setJavaWriter(void* javaWriter, archiveWriter);
engineAPI void* getJavaWriter(archiveWriter);
engineAPI void setCppWriter(void* cppWriter, archiveWriter);
engineAPI void* getCppWriter(archiveWriter);

engineAPI void setJavaReader(void* javaReader, archiveReader);
engineAPI void* getJavaReader(archiveReader);
engineAPI void setCppReader(void* cppReader, archiveReader);
engineAPI void* getCppReader(archiveReader);

endEngineNamespace

#endif // EngineHIncluded

