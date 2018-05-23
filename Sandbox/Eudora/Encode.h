// Encoder
// Base class for encoders
//

#ifndef _ENCODER_H_
#define _ENCODER_H_

class Encoder
{
public:
	Encoder() {}

	// Encode
	// The main engine for encoders
	// In:		the input buffer
	// InLen:	the number of bytes to be converted
	// Out:		the output buffer (assumed to be large enough to handle output)
	// OutLen:	the number of bytes actually copied to Out
	// Returns the number of errors encountered
	virtual int Encode(const char* In, LONG InLen, char* Out, LONG& OutLen) = 0;

	// Init
	// Initializes the encoder
	// Out:		the output buffer (assumed to be large enough to handle output)
	// OutLen:	the number of bytes actually copied to Out
	// Returns the number of errors encountered
	virtual int Init(char* /*Out*/, LONG& /*OutLen*/, int /*flag*/) { return (0); }

	// Done
	// Cleans up after the encoding
	// Out:		the output buffer (assumed to be large enough to handle output)
	// OutLen:	the number of bytes actually copied to Out
	// Returns the number of errors encountered
	virtual int Done(char* /*Out*/, LONG& /*OutLen*/) { return (0); }
};

#endif // #ifndef _ENCODER_H_
