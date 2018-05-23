/*
 * jerror.h
 *
 * Copyright (C) 1994-1995, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file defines the error and message codes for the JPEG library.
 * Edit this file to add new codes, or to translate the message strings to
 * some other language.
 * A set of error-reporting macros are defined too.  Some applications using
 * the JPEG library may wish to include this file to get the error codes
 * and/or the macros.
 */

/*
 * To define the enum list of message codes, include this file without
 * defining macro JMESSAGE.  To create a message string table, include it
 * again with a suitable JMESSAGE definition (see jerror.c for an example).
 */
#ifndef JMESSAGE
#ifndef JERROR_H
/* First time through, define the enum list */
#define JMAKE_ENUM_LIST
#else
/* Repeated inclusions of this file are no-ops unless JMESSAGE is defined */
#define JMESSAGE(code,string)
#endif /* JERROR_H */
#endif /* JMESSAGE */

#ifdef JMAKE_ENUM_LIST

typedef enum {

#define JMESSAGE(code,string)	code ,

#endif /* JMAKE_ENUM_LIST */

JMESSAGE(JMSG_NOMESSAGE, _T("Bogus message code %d")) /* Must be first entry! */

/* For maintenance convenience, list is alphabetical by message code name */
JMESSAGE(JERR_ARITH_NOTIMPL,
	 _T("Sorry, there are legal restrictions on arithmetic coding"))
JMESSAGE(JERR_BAD_ALIGN_TYPE, _T("ALIGN_TYPE is wrong, please fix"))
JMESSAGE(JERR_BAD_ALLOC_CHUNK, _T("MAX_ALLOC_CHUNK is wrong, please fix"))
JMESSAGE(JERR_BAD_BUFFER_MODE, _T("Bogus buffer control mode"))
JMESSAGE(JERR_BAD_COMPONENT_ID, _T("Invalid component ID %d in SOS"))
JMESSAGE(JERR_BAD_DCTSIZE, _T("IDCT output block size %d not supported"))
JMESSAGE(JERR_BAD_IN_COLORSPACE, _T("Bogus input colorspace"))
JMESSAGE(JERR_BAD_J_COLORSPACE, _T("Bogus JPEG colorspace"))
JMESSAGE(JERR_BAD_LENGTH, _T("Bogus marker length"))
JMESSAGE(JERR_BAD_MCU_SIZE, _T("Sampling factors too large for interleaved scan"))
JMESSAGE(JERR_BAD_POOL_ID, _T("Invalid memory pool code %d"))
JMESSAGE(JERR_BAD_PRECISION, _T("Unsupported JPEG data precision %d"))
JMESSAGE(JERR_BAD_PROGRESSION,
	 _T("Invalid progressive parameters Ss=%d Se=%d Ah=%d Al=%d"))
JMESSAGE(JERR_BAD_PROG_SCRIPT,
	 _T("Invalid progressive parameters at scan script entry %d"))
JMESSAGE(JERR_BAD_SAMPLING, _T("Bogus sampling factors"))
JMESSAGE(JERR_BAD_SCAN_SCRIPT, _T("Invalid scan script at entry %d"))
JMESSAGE(JERR_BAD_STATE, _T("Improper call to JPEG library in state %d"))
JMESSAGE(JERR_BAD_VIRTUAL_ACCESS, _T("Bogus virtual array access"))
JMESSAGE(JERR_BUFFER_SIZE, _T("Buffer passed to JPEG library is too small"))
JMESSAGE(JERR_CANT_SUSPEND, _T("Suspension not allowed here"))
JMESSAGE(JERR_CCIR601_NOTIMPL, _T("CCIR601 sampling not implemented yet"))
JMESSAGE(JERR_COMPONENT_COUNT, _T("Too many color components: %d, max %d"))
JMESSAGE(JERR_CONVERSION_NOTIMPL, _T("Unsupported color conversion request"))
JMESSAGE(JERR_DAC_INDEX, _T("Bogus DAC index %d"))
JMESSAGE(JERR_DAC_VALUE, _T("Bogus DAC value 0x%x"))
JMESSAGE(JERR_DHT_COUNTS, _T("Bogus DHT counts"))
JMESSAGE(JERR_DHT_INDEX, _T("Bogus DHT index %d"))
JMESSAGE(JERR_DQT_INDEX, _T("Bogus DQT index %d"))
JMESSAGE(JERR_EMPTY_IMAGE, _T("Empty JPEG image (DNL not supported)"))
JMESSAGE(JERR_EMS_READ, _T("Read from EMS failed"))
JMESSAGE(JERR_EMS_WRITE, _T("Write to EMS failed"))
JMESSAGE(JERR_EOI_EXPECTED, _T("Didn't expect more than one scan"))
JMESSAGE(JERR_FILE_READ, _T("Input file read error"))
JMESSAGE(JERR_FILE_WRITE, _T("Output file write error --- out of disk space?"))
JMESSAGE(JERR_FRACT_SAMPLE_NOTIMPL, _T("Fractional sampling not implemented yet"))
JMESSAGE(JERR_HUFF_CLEN_OVERFLOW, _T("Huffman code size table overflow"))
JMESSAGE(JERR_HUFF_MISSING_CODE, _T("Missing Huffman code table entry"))
JMESSAGE(JERR_IMAGE_TOO_BIG, _T("Maximum supported image dimension is %u pixels"))
JMESSAGE(JERR_INPUT_EMPTY, _T("Empty input file"))
JMESSAGE(JERR_INPUT_EOF, _T("Premature end of input file"))
JMESSAGE(JERR_MISMATCHED_QUANT_TABLE,
	 _T("Cannot transcode due to multiple use of quantization table %d"))
JMESSAGE(JERR_MISSING_DATA, _T("Scan script does not transmit all data"))
JMESSAGE(JERR_MODE_CHANGE, _T("Invalid color quantization mode change"))
JMESSAGE(JERR_NOTIMPL, _T("Not implemented yet"))
JMESSAGE(JERR_NOT_COMPILED, _T("Requested feature was omitted at compile time"))
JMESSAGE(JERR_NO_BACKING_STORE, _T("Backing store not supported"))
JMESSAGE(JERR_NO_HUFF_TABLE, _T("Huffman table 0x%02x was not defined"))
JMESSAGE(JERR_NO_IMAGE, _T("JPEG datastream contains no image"))
JMESSAGE(JERR_NO_QUANT_TABLE, _T("Quantization table 0x%02x was not defined"))
JMESSAGE(JERR_NO_SOI, _T("Not a JPEG file: starts with 0x%02x 0x%02x"))
JMESSAGE(JERR_OUT_OF_MEMORY, _T("Insufficient memory (case %d)"))
JMESSAGE(JERR_QUANT_COMPONENTS,
	 _T("Cannot quantize more than %d color components"))
JMESSAGE(JERR_QUANT_FEW_COLORS, _T("Cannot quantize to fewer than %d colors"))
JMESSAGE(JERR_QUANT_MANY_COLORS, _T("Cannot quantize to more than %d colors"))
JMESSAGE(JERR_SOF_DUPLICATE, _T("Invalid JPEG file structure: two SOF markers"))
JMESSAGE(JERR_SOF_NO_SOS, _T("Invalid JPEG file structure: missing SOS marker"))
JMESSAGE(JERR_SOF_UNSUPPORTED, _T("Unsupported JPEG process: SOF type 0x%02x"))
JMESSAGE(JERR_SOI_DUPLICATE, _T("Invalid JPEG file structure: two SOI markers"))
JMESSAGE(JERR_SOS_NO_SOF, _T("Invalid JPEG file structure: SOS before SOF"))
JMESSAGE(JERR_TFILE_CREATE, _T("Failed to create temporary file %s"))
JMESSAGE(JERR_TFILE_READ, _T("Read failed on temporary file"))
JMESSAGE(JERR_TFILE_SEEK, _T("Seek failed on temporary file"))
JMESSAGE(JERR_TFILE_WRITE,
	 _T("Write failed on temporary file --- out of disk space?"))
JMESSAGE(JERR_TOO_LITTLE_DATA, _T("Application transferred too few scanlines"))
JMESSAGE(JERR_UNKNOWN_MARKER, _T("Unsupported marker type 0x%02x"))
JMESSAGE(JERR_VIRTUAL_BUG, _T("Virtual array controller messed up"))
JMESSAGE(JERR_WIDTH_OVERFLOW, _T("Image too wide for this implementation"))
JMESSAGE(JERR_XMS_READ, _T("Read from XMS failed"))
JMESSAGE(JERR_XMS_WRITE, _T("Write to XMS failed"))
JMESSAGE(JMSG_COPYRIGHT, JCOPYRIGHT)
JMESSAGE(JMSG_VERSION, JVERSION)
JMESSAGE(JTRC_16BIT_TABLES,
	 _T("Caution: quantization tables are too coarse for baseline JPEG"))
JMESSAGE(JTRC_ADOBE,
	 _T("Adobe APP14 marker: version %d, flags 0x%04x 0x%04x, transform %d"))
JMESSAGE(JTRC_APP0, _T("Unknown APP0 marker (not JFIF), length %u"))
JMESSAGE(JTRC_APP14, _T("Unknown APP14 marker (not Adobe), length %u"))
JMESSAGE(JTRC_DAC, _T("Define Arithmetic Table 0x%02x: 0x%02x"))
JMESSAGE(JTRC_DHT, _T("Define Huffman Table 0x%02x"))
JMESSAGE(JTRC_DQT, _T("Define Quantization Table %d  precision %d"))
JMESSAGE(JTRC_DRI, _T("Define Restart Interval %u"))
JMESSAGE(JTRC_EMS_CLOSE, _T("Freed EMS handle %u"))
JMESSAGE(JTRC_EMS_OPEN, _T("Obtained EMS handle %u"))
JMESSAGE(JTRC_EOI, _T("End Of Image"))
JMESSAGE(JTRC_HUFFBITS, _T("        %3d %3d %3d %3d %3d %3d %3d %3d"))
JMESSAGE(JTRC_JFIF, _T("JFIF APP0 marker, density %dx%d  %d"))
JMESSAGE(JTRC_JFIF_BADTHUMBNAILSIZE,
	 _T("Warning: thumbnail image size does not match data length %u"))
JMESSAGE(JTRC_JFIF_MINOR, _T("Unknown JFIF minor revision number %d.%02d"))
JMESSAGE(JTRC_JFIF_THUMBNAIL, _T("    with %d x %d thumbnail image"))
JMESSAGE(JTRC_MISC_MARKER, _T("Skipping marker 0x%02x, length %u"))
JMESSAGE(JTRC_PARMLESS_MARKER, _T("Unexpected marker 0x%02x"))
JMESSAGE(JTRC_QUANTVALS, _T("        %4u %4u %4u %4u %4u %4u %4u %4u"))
JMESSAGE(JTRC_QUANT_3_NCOLORS, _T("Quantizing to %d = %d*%d*%d colors"))
JMESSAGE(JTRC_QUANT_NCOLORS, _T("Quantizing to %d colors"))
JMESSAGE(JTRC_QUANT_SELECTED, _T("Selected %d colors for quantization"))
JMESSAGE(JTRC_RECOVERY_ACTION, _T("At marker 0x%02x, recovery action %d"))
JMESSAGE(JTRC_RST, _T("RST%d"))
JMESSAGE(JTRC_SMOOTH_NOTIMPL,
	 _T("Smoothing not supported with nonstandard sampling ratios"))
JMESSAGE(JTRC_SOF, _T("Start Of Frame 0x%02x: width=%u, height=%u, components=%d"))
JMESSAGE(JTRC_SOF_COMPONENT, _T("    Component %d: %dhx%dv q=%d"))
JMESSAGE(JTRC_SOI, _T("Start of Image"))
JMESSAGE(JTRC_SOS, _T("Start Of Scan: %d components"))
JMESSAGE(JTRC_SOS_COMPONENT, _T("    Component %d: dc=%d ac=%d"))
JMESSAGE(JTRC_SOS_PARAMS, _T("  Ss=%d, Se=%d, Ah=%d, Al=%d"))
JMESSAGE(JTRC_TFILE_CLOSE, _T("Closed temporary file %s"))
JMESSAGE(JTRC_TFILE_OPEN, _T("Opened temporary file %s"))
JMESSAGE(JTRC_UNKNOWN_IDS,
	 _T("Unrecognized component IDs %d %d %d, assuming YCbCr"))
JMESSAGE(JTRC_XMS_CLOSE, _T("Freed XMS handle %u"))
JMESSAGE(JTRC_XMS_OPEN, _T("Obtained XMS handle %u"))
JMESSAGE(JWRN_ADOBE_XFORM, _T("Unknown Adobe color transform code %d"))
JMESSAGE(JWRN_BOGUS_PROGRESSION,
	 _T("Inconsistent progression sequence for component %d coefficient %d"))
JMESSAGE(JWRN_EXTRANEOUS_DATA,
	 _T("Corrupt JPEG data: %u extraneous bytes before marker 0x%02x"))
JMESSAGE(JWRN_HIT_MARKER, _T("Corrupt JPEG data: premature end of data segment"))
JMESSAGE(JWRN_HUFF_BAD_CODE, _T("Corrupt JPEG data: bad Huffman code"))
JMESSAGE(JWRN_JFIF_MAJOR, _T("Warning: unknown JFIF revision number %d.%02d"))
JMESSAGE(JWRN_JPEG_EOF, _T("Premature end of JPEG file"))
JMESSAGE(JWRN_MUST_RESYNC,
	 _T("Corrupt JPEG data: found marker 0x%02x instead of RST%d"))
JMESSAGE(JWRN_NOT_SEQUENTIAL, _T("Invalid SOS parameters for sequential JPEG"))
JMESSAGE(JWRN_TOO_MUCH_DATA, _T("Application transferred too many scanlines"))

#ifdef JMAKE_ENUM_LIST

  JMSG_LASTMSGCODE
} J_MESSAGE_CODE;

#undef JMAKE_ENUM_LIST
#endif /* JMAKE_ENUM_LIST */

/* Zap JMESSAGE macro so that future re-inclusions do nothing by default */
#undef JMESSAGE


#ifndef JERROR_H
#define JERROR_H

/* Macros to simplify using the error and trace message stuff */
/* The first parameter is either type of cinfo pointer */

/* Fatal errors (print message and exit) */
#define ERREXIT(cinfo,code)  \
  ((cinfo)->err->msg_code = (code), \
   (this->*(cinfo)->err->error_exit) ((j_common_ptr) (cinfo)))
#define ERREXIT1(cinfo,code,p1)  \
  ((cinfo)->err->msg_code = (code), \
   (cinfo)->err->msg_parm.i[0] = (p1), \
   (this->*(cinfo)->err->error_exit) ((j_common_ptr) (cinfo)))
#define ERREXIT2(cinfo,code,p1,p2)  \
  ((cinfo)->err->msg_code = (code), \
   (cinfo)->err->msg_parm.i[0] = (p1), \
   (cinfo)->err->msg_parm.i[1] = (p2), \
   (this->*(cinfo)->err->error_exit) ((j_common_ptr) (cinfo)))
#define ERREXIT3(cinfo,code,p1,p2,p3)  \
  ((cinfo)->err->msg_code = (code), \
   (cinfo)->err->msg_parm.i[0] = (p1), \
   (cinfo)->err->msg_parm.i[1] = (p2), \
   (cinfo)->err->msg_parm.i[2] = (p3), \
   (this->*(cinfo)->err->error_exit) ((j_common_ptr) (cinfo)))
#define ERREXIT4(cinfo,code,p1,p2,p3,p4)  \
  ((cinfo)->err->msg_code = (code), \
   (cinfo)->err->msg_parm.i[0] = (p1), \
   (cinfo)->err->msg_parm.i[1] = (p2), \
   (cinfo)->err->msg_parm.i[2] = (p3), \
   (cinfo)->err->msg_parm.i[3] = (p4), \
   (this->*(cinfo)->err->error_exit) ((j_common_ptr) (cinfo)))
#define ERREXITS(cinfo,code,str)  \
  ((cinfo)->err->msg_code = (code), \
   strncpy((cinfo)->err->msg_parm.s, (str), JMSG_STR_PARM_MAX), \
   (this->*(cinfo)->err->error_exit) ((j_common_ptr) (cinfo)))

#define MAKESTMT(stuff)		do { stuff } while (0)

/* Nonfatal errors (we can keep going, but the data is probably corrupt) */
#define WARNMS(cinfo,code)  \
  ((cinfo)->err->msg_code = (code), \
   (this->*(cinfo)->err->emit_message) ((j_common_ptr) (cinfo), -1))
#define WARNMS1(cinfo,code,p1)  \
  ((cinfo)->err->msg_code = (code), \
   (cinfo)->err->msg_parm.i[0] = (p1), \
   (this->*(cinfo)->err->emit_message) ((j_common_ptr) (cinfo), -1))
#define WARNMS2(cinfo,code,p1,p2)  \
  ((cinfo)->err->msg_code = (code), \
   (cinfo)->err->msg_parm.i[0] = (p1), \
   (cinfo)->err->msg_parm.i[1] = (p2), \
   (this->*(cinfo)->err->emit_message) ((j_common_ptr) (cinfo), -1))

/* Informational/debugging messages */
#define TRACEMS(cinfo,lvl,code)  \
  ((cinfo)->err->msg_code = (code), \
   (this->*(cinfo)->err->emit_message) ((j_common_ptr) (cinfo), (lvl)))
#define TRACEMS1(cinfo,lvl,code,p1)  \
  ((cinfo)->err->msg_code = (code), \
   (cinfo)->err->msg_parm.i[0] = (p1), \
   (this->*(cinfo)->err->emit_message) ((j_common_ptr) (cinfo), (lvl)))
#define TRACEMS2(cinfo,lvl,code,p1,p2)  \
  ((cinfo)->err->msg_code = (code), \
   (cinfo)->err->msg_parm.i[0] = (p1), \
   (cinfo)->err->msg_parm.i[1] = (p2), \
   (this->*(cinfo)->err->emit_message) ((j_common_ptr) (cinfo), (lvl)))
#define TRACEMS3(cinfo,lvl,code,p1,p2,p3)  \
  MAKESTMT(int * _mp = (cinfo)->err->msg_parm.i; \
	   _mp[0] = (p1); _mp[1] = (p2); _mp[2] = (p3); \
	   (cinfo)->err->msg_code = (code); \
	   (this->*(cinfo)->err->emit_message) ((j_common_ptr) (cinfo), (lvl)); )
#define TRACEMS4(cinfo,lvl,code,p1,p2,p3,p4)  \
  MAKESTMT(int * _mp = (cinfo)->err->msg_parm.i; \
	   _mp[0] = (p1); _mp[1] = (p2); _mp[2] = (p3); _mp[3] = (p4); \
	   (cinfo)->err->msg_code = (code); \
	   (this->*(cinfo)->err->emit_message) ((j_common_ptr) (cinfo), (lvl)); )
#define TRACEMS8(cinfo,lvl,code,p1,p2,p3,p4,p5,p6,p7,p8)  \
  MAKESTMT(int * _mp = (cinfo)->err->msg_parm.i; \
	   _mp[0] = (p1); _mp[1] = (p2); _mp[2] = (p3); _mp[3] = (p4); \
	   _mp[4] = (p5); _mp[5] = (p6); _mp[6] = (p7); _mp[7] = (p8); \
	   (cinfo)->err->msg_code = (code); \
	   (this->*(cinfo)->err->emit_message) ((j_common_ptr) (cinfo), (lvl)); )
#define TRACEMSS(cinfo,lvl,code,str)  \
  ((cinfo)->err->msg_code = (code), \
   strncpy((cinfo)->err->msg_parm.s, (str), JMSG_STR_PARM_MAX), \
   (this->*(cinfo)->err->emit_message) ((j_common_ptr) (cinfo), (lvl)))

#endif /* JERROR_H */
