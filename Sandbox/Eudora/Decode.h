// Decoder
// Base class for decoders
//

#ifndef _DECODER_H_
#define _DECODER_H_

class Decoder
{
public:
	Decoder() {}

	// Decode
	// The main engine for decoders
	// In:		the input buffer
	// InLen:	the number of bytes to be converted
	// Out:		the output buffer (assumed to be large enough to handle output)
	// OutLen:	the number of bytes actually copied to Out
	// Returns the number of errors encountered
	virtual int Decode(const char *In, LONG InLen, char* Out, LONG& OutLen) = 0;

	// Init
	// Initializes the decoder
	// Returns the number of errors encountered
	virtual int Init() { return (0); }

	// Added by JOK so we avoid having decoder write to the file.
	// Decoders should operate on memory buffers only!!
	// Actually only used by IMAP. Should be overridden.
	// Binhex and uudecode should use this to allow a caller to fetch the
	// embedded filename after decoding is complete.
	// 
	virtual LPCTSTR GetEmbeddedFilename () { return NULL; }
};

#endif // #ifndef _DECODER_H_
