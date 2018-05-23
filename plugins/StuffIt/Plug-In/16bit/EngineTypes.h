#if !defined EngineTypesHIncluded
#define EngineTypesHIncluded

#include <stdlib.h>

#include "Platform.h"
#if enginePlatform == enginePlatformWindowsIntel16
#include "sepream.h"
#else
#include "EnginePreamble.h"
#endif

typedef unsigned char uchar;
typedef char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef long int32;
typedef unsigned long uint32;

beginEngineNamespace

typedef void* archiveReader;
typedef void* archiveWriter;

typedef enum { // text conversion type
    engineTextOutSame = 0, // no change
    engineTextOutAuto = 1, // if a file contains text
    engineTextOutLf = 2, // use LF
    engineTextOutCr = 4, // use CR
    engineTextOutUnix = engineTextOutLf, // Unix
    engineTextOutMac = engineTextOutCr, // MacOS
    engineTextOutDos = engineTextOutLf | engineTextOutCr // MS-DOS, Windows, OS/2
} engineTextOut;

typedef enum {
    engineMacBinaryOutOff = 0, // never
    engineMacBinaryOutOn, // always
    engineMacBinaryOutAuto // if resource fork is present
} engineMacBinaryOut;

typedef enum {
    engineCompLevelAuto = -1, // automatic
    engineCompLevelStuffItNone = 0, // no compression
    engineCompLevelStuffItDeluxe = 13, // StuffIt Deluxe compression
    engineCompLevelZipNone = 0, // no compression
    engineCompLevelZipBase = 0, // use as ...Base + n, n = 1..9
    engineCompLevelZipOne = 1, // 1..9, increasing compression, decreasing speed
    engineCompLevelZipMin = 1,
    engineCompLevelZipTwo = 2,
    engineCompLevelZipThree = 3,
    engineCompLevelZipFour = 4,
    engineCompLevelZipFive = 5,
    engineCompLevelZipSix = 6,
    engineCompLevelZipDefault = 6,
    engineCompLevelZipSeven = 7,
    engineCompLevelZipEight = 8,
    engineCompLevelZipNine = 9,
    engineCompLevelZipMax = 9,
} engineCompLevel;

typedef enum {
    engineOnConflictOverwrite = 0,
    engineOnConflictAutoRename
} engineOnConflict;

// engineFormat is used to refer to file formats supported or recognized by the engine
typedef enum {
    engineFormatUnknown = 0, // special cases
    engineFormatEmpty,
    engineFormatEof,
    engineFormatStuffIt, // Macintosh
    engineFormatFirst = engineFormatStuffIt,
    engineFormatCompactPro,
    engineFormatZip, // DOS/Windows
    engineFormatArc,
    engineFormatArj,
    engineFormatLha,
    engineFormatHa,
    engineFormatRar,
    engineFormatGzip, // Unix
    engineFormatUnixCompress,
    engineFormatScoCompress,
    engineFormatUnixPack,
    engineFormatUnixCompact,
    engineFormatFreeze,
    engineFormatUuencode, // encoded
    engineFormatBinHex,
    engineFormatBtoa,
    engineFormatMime,
    engineFormatUnixTar, // miscellaneous
    engineFormatMacBinary,
    engineFormatMacBinaryInfo,
    engineFormatMacBinaryData,
    engineFormatMacBinaryRsrc,
    engineFormatStuffItSegment, // Aladdin
    engineFormatStuffItSegmentN,
    engineFormatPrivateFile,
    engineFormatLast = engineFormatPrivateFile
} engineFormat;
// engineFormatMacBinaryInfo, engineFormatMacBinaryData, and engineFormatMacBinaryRsrc are used only in callbacks
// engineFormatStuffItSegment refers to the first segment, engineFormatStuffItSegmentN to any subsequent segment

// error is used to transmit error information at engine "C" call level
typedef enum {
    noEngineError = 0,
    engineErrorAborted, // aborted by caller
    engineErrorNoMemory, // out of memory
    engineErrorFormatNotSupported, // file format not supported
    engineErrorMethodNotSupported, // compression method or file format variation not supported
    engineErrorInputOpen, // error opening input file
    engineErrorSeekError, // error seeking in a file
    engineErrorReadError, // error reading input file
    engineErrorEof, // error reading a file
    engineErrorOutputOpen, // error opening output file
    engineErrorWriteError, // error writing output file
    engineErrorCloseError, // error closing a file
    engineErrorDelete, // error deleting a file
    engineErrorGetDirError, // _getcwd(), GetCurrentDirectory() engineError
    engineErrorSetDirError, // _chdir(), SetCurrentDirectory() engineError
    engineErrorSetInfo, // error setting file info (attributes, timestamp)
    engineErrorGetInfo, // error getting file info (size, attributes, timestamp)
    engineErrorFolderCreate, // error creating output folder
    engineErrorDestFolderCreate, // error creating destination folder
    engineErrorDosReserved, // name is reserved by DOS (such as CON)
    engineErrorInvalidFormat, // invalid file format
    engineErrorInvalidFileName, // for formats that restrict fileNames
    engineErrorInvalidData, // corrupted input file
    engineErrorSegmented, // segmented files not supported
    engineErrorEncrypted, // encrypted files not supported
    engineErrorCollision, // unresolved fileName collision
    engineErrorNumberOfFiles, // for those encoders that require a fixed number of inputs
    engineErrorIsAFolder, // for those encoders that do not accept folders as input
    engineErrorPathnameTooLong, // pathname exceeds OS limit
    engineErrorSegmentOutOfSequence, // segment out of sequence
    engineErrorInvalidSegmentSize, // segment size too small
    engineErrorIncorrectPassword, // incorrect password
    engineErrorNoOpenArchive, // operation attempted on an archive that is not open
    engineErrorLast = engineErrorNoOpenArchive
} engineError;

endEngineNamespace

#endif // EngineTypesHIncluded

