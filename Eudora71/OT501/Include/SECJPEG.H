// Stringray Software Extension Classes
// Copyright (C) 1995 Stingray Software Inc,
// All rights reserved.
//
// SEC JPEG file format header
//
// This source code is only intended as a supplement to the
// Stingray Extension Classes 
// See the Stingray Extension Classes help files for 
// detailed information regarding using SEC classes.
//
// CHANGELOG: 
//
//    AAB	7/03/95	Started
//    AAB	2/01/95	update to JPEG v.6
//
//
//

#ifndef __SECJPEG_H__
#define __SECJPEG_H__

//
// SEC Extension DLL
// Initialize declaration context
//

#ifdef _SECDLL
	#undef AFXAPP_DATA
	#define AFXAPP_DATA AFXAPI_DATA
	#undef AFX_DATA
	#define AFX_DATA    SEC_DATAEXT
#endif //_SECDLL

#include "secimage.h"
#include "jpeglib.h"
#include "jinclude.h"
//AAB #include "jpegint.h"
#include "jmemsys.h"
#include "jdhuff.h"
#include "jchuff.h"
#include "jdct.h"


//typedefs needed here for class prototype fxns.
// from jquant1.cpp
#define ODITHER_SIZE  16	/* dimension of dither matrix */
/* NB: if ODITHER_SIZE is not a power of 2, ODITHER_MASK uses will break */
#define ODITHER_CELLS (ODITHER_SIZE*ODITHER_SIZE)	/* # cells in matrix */
#define ODITHER_MASK  (ODITHER_SIZE-1) /* mask for wrapping around counters */

typedef int ODITHER_MATRIX[ODITHER_SIZE][ODITHER_SIZE];
typedef int (*ODITHER_MATRIX_PTR)[ODITHER_SIZE];


// jchuff.cpp
/* Expanded entropy encoder object for Huffman encoding.
 *
 * The savable_state subrecord contains fields that change within an MCU,
 * but must not be updated permanently until we complete the MCU.
 */
typedef struct {
  INT32 put_buffer;		/* current bit-accumulation buffer */
  int put_bits;			/* # of bits now in it */
  int last_dc_val[MAX_COMPS_IN_SCAN]; /* last DC coef for each component */
} comp_savable_state;


/* Working state while writing an MCU.
 * This struct contains all the fields that are needed by subroutines.
 */

typedef struct {
  JOCTET * next_output_byte;	/* => next byte to write in buffer */
  size_t free_in_buffer;	/* # of byte spaces remaining in buffer */
  comp_savable_state cur;		/* Current bit buffer & DC state */
  j_compress_ptr cinfo;		/* dump_buffer needs access to this */
} comp_working_state;

//jcmarker.cpp
typedef enum {			/* JPEG marker codes */
  M_SOF0  = 0xc0,
  M_SOF1  = 0xc1,
  M_SOF2  = 0xc2,
  M_SOF3  = 0xc3,
  
  M_SOF5  = 0xc5,
  M_SOF6  = 0xc6,
  M_SOF7  = 0xc7,
  
  M_JPG   = 0xc8,
  M_SOF9  = 0xc9,
  M_SOF10 = 0xca,
  M_SOF11 = 0xcb,
  
  M_SOF13 = 0xcd,
  M_SOF14 = 0xce,
  M_SOF15 = 0xcf,
  
  M_DHT   = 0xc4,
  
  M_DAC   = 0xcc,
  
  M_RST0  = 0xd0,
  M_RST1  = 0xd1,
  M_RST2  = 0xd2,
  M_RST3  = 0xd3,
  M_RST4  = 0xd4,
  M_RST5  = 0xd5,
  M_RST6  = 0xd6,
  M_RST7  = 0xd7,
  
  M_SOI   = 0xd8,
  M_EOI   = 0xd9,
  M_SOS   = 0xda,
  M_DQT   = 0xdb,
  M_DNL   = 0xdc,
  M_DRI   = 0xdd,
  M_DHP   = 0xde,
  M_EXP   = 0xdf,
  
  M_APP0  = 0xe0,
  M_APP1  = 0xe1,
  M_APP2  = 0xe2,
  M_APP3  = 0xe3,
  M_APP4  = 0xe4,
  M_APP5  = 0xe5,
  M_APP6  = 0xe6,
  M_APP7  = 0xe7,
  M_APP8  = 0xe8,
  M_APP9  = 0xe9,
  M_APP10 = 0xea,
  M_APP11 = 0xeb,
  M_APP12 = 0xec,
  M_APP13 = 0xed,
  M_APP14 = 0xee,
  M_APP15 = 0xef,
  
  M_JPG0  = 0xf0,
  M_JPG13 = 0xfd,
  M_COM   = 0xfe,
  
  M_TEM   = 0x01,
  
  M_ERROR = 0x100
} COMP_JPEG_MARKER;

//jdmarker.cpp


// jcphuff.cpp
typedef struct {
  struct jpeg_entropy_encoder pub; /* public fields */

  /* Mode flag: TRUE for optimization, FALSE for actual data output */
  boolean gather_statistics;

  /* Bit-level coding status.
   * next_output_byte/free_in_buffer are local copies of cinfo->dest fields.
   */ 
  boolean aab_filler;
  JOCTET * next_output_byte;	/* => next byte to write in buffer */
  size_t free_in_buffer;	/* # of byte spaces remaining in buffer */
  INT32 put_buffer;		/* current bit-accumulation buffer */
  int put_bits;			/* # of bits now in it */
  j_compress_ptr cinfo;		/* link to cinfo (needed for dump_buffer) */

  /* Coding status for DC components */
  int last_dc_val[MAX_COMPS_IN_SCAN]; /* last DC coef for each component */

  /* Coding status for AC components */
  int ac_tbl_no;		/* the table number of the single component */
  unsigned int EOBRUN;		/* run length of EOBs */
  unsigned int BE;		/* # of buffered correction bits before MCU */
  char * bit_buffer;		/* buffer for correction bits (1 per char) */
  /* packing correction bits tightly would save some space but cost time... */

  unsigned int restarts_to_go;	/* MCUs left in this restart interval */
  int next_restart_num;		/* next restart number to write (0-7) */

  /* Pointers to derived tables (these workspaces have image lifespan).
   * Since any one scan codes only DC or only AC, we only need one set
   * of tables, not one for DC and one for AC.
   */
  c_derived_tbl * derived_tbls[NUM_HUFF_TBLS];

  /* Statistics tables for optimization; again, one set is enough */
  long * count_ptrs[NUM_HUFF_TBLS];
} cp_phuff_entropy_encoder;
typedef cp_phuff_entropy_encoder * cp_phuff_entropy_ptr;

// jquant2.cpp
typedef struct {
  /* The bounds of the box (inclusive); expressed as histogram indexes */
  int c0min, c0max;
  int c1min, c1max;
  int c2min, c2max;
  /* The volume (actually 2-norm) of the box */
  INT32 volume;
  /* The number of nonzero histogram cells within this box */
  long colorcount;
} box;

typedef box * boxptr;


class SECJpeg: public SECImage
{

private:
	void SECoutput_init(j_decompress_ptr cinfo);

// Overrides
protected:
	virtual BOOL DoLoadImage(CFile* pFile);
	virtual BOOL DoSaveImage(CFile* pFile);

//jccoefct.cpp
void jinit_c_coef_controller (j_compress_ptr cinfo, boolean need_full_buffer);
void coefct_start_iMCU_row (j_compress_ptr cinfo);
void coefct_start_pass_coef (j_compress_ptr cinfo, J_BUF_MODE pass_mode);
boolean compress_data (j_compress_ptr cinfo, JSAMPIMAGE input_buf);
boolean compress_first_pass (j_compress_ptr cinfo, JSAMPIMAGE input_buf);
boolean coefct_compress_output (j_compress_ptr cinfo, JSAMPIMAGE input_buf);

//jcdctmgr.cpp
void start_pass_fdctmgr (j_compress_ptr cinfo);
void forward_DCT (j_compress_ptr cinfo, jpeg_component_info * compptr,
	JSAMPARRAY sample_data, JBLOCKROW coef_blocks,
	JDIMENSION start_row, JDIMENSION start_col,
	JDIMENSION num_blocks);
void forward_DCT_float (j_compress_ptr cinfo, jpeg_component_info * compptr,
	JSAMPARRAY sample_data, JBLOCKROW coef_blocks,
	JDIMENSION start_row, JDIMENSION start_col,
	JDIMENSION num_blocks);
void jinit_forward_dct (j_compress_ptr cinfo);

//jcinit.cpp
void jinit_compress_master (j_compress_ptr cinfo);

//jcmainct.cpp
void start_pass_main (j_compress_ptr cinfo, J_BUF_MODE pass_mode);
void process_data_simple_main (j_compress_ptr cinfo,
	JSAMPARRAY input_buf, JDIMENSION *in_row_ctr,
	JDIMENSION in_rows_avail);
void process_data_buffer_main (j_compress_ptr cinfo,
	JSAMPARRAY input_buf, JDIMENSION *in_row_ctr,
	JDIMENSION in_rows_avail);
void jinit_c_main_controller (j_compress_ptr cinfo, boolean need_full_buffer);

//jcmaster.cpp
void initial_setup (j_compress_ptr cinfo);
void validate_script (j_compress_ptr cinfo);
void select_scan_parameters (j_compress_ptr cinfo);
void per_scan_setup (j_compress_ptr cinfo);
void prepare_for_pass (j_compress_ptr cinfo);
void pass_startup (j_compress_ptr cinfo);
void finish_pass_master (j_compress_ptr cinfo);
void jinit_c_master_control (j_compress_ptr cinfo, boolean transcode_only);


//jcparam.cpp 
void jpeg_add_quant_table (j_compress_ptr cinfo, int which_tbl,
	const unsigned int *basic_table,
	int scale_factor, boolean force_baseline);
void jpeg_set_linear_quality (j_compress_ptr cinfo, int scale_factor,
	boolean force_baseline);
int jpeg_quality_scaling (int quality);
void jpeg_set_quality (j_compress_ptr cinfo, int quality, boolean force_baseline);
void add_huff_table (j_compress_ptr cinfo,
	JHUFF_TBL **htblptr, const UINT8 *bits, const UINT8 *val);
void std_huff_tables (j_compress_ptr cinfo);
void jpeg_set_defaults (j_compress_ptr cinfo);
void jpeg_default_colorspace (j_compress_ptr cinfo);
void jpeg_set_colorspace (j_compress_ptr cinfo, J_COLOR_SPACE colorspace);
jpeg_scan_info* fill_a_scan (jpeg_scan_info * scanptr, int ci,
	int Ss, int Se, int Ah, int Al);
jpeg_scan_info* fill_scans (jpeg_scan_info * scanptr, int ncomps,	
	int Ss, int Se, int Ah, int Al);
jpeg_scan_info* fill_dc_scans (jpeg_scan_info * scanptr, int ncomps, int Ah, int Al);
void jpeg_simple_progression (j_compress_ptr cinfo);

// jcprepct.cpp
void start_pass_prep (j_compress_ptr cinfo, J_BUF_MODE pass_mode);
void expand_bottom_edge (JSAMPARRAY image_data, JDIMENSION num_cols,
	int input_rows, int output_rows);
void pre_process_data (j_compress_ptr cinfo,
	JSAMPARRAY input_buf, JDIMENSION *in_row_ctr,
	JDIMENSION in_rows_avail,
	JSAMPIMAGE output_buf, JDIMENSION *out_row_group_ctr,
	JDIMENSION out_row_groups_avail);
void pre_process_context (j_compress_ptr cinfo,
	JSAMPARRAY input_buf, JDIMENSION *in_row_ctr,
	JDIMENSION in_rows_avail,
	JSAMPIMAGE output_buf, JDIMENSION *out_row_group_ctr,
	JDIMENSION out_row_groups_avail);
void create_context_buffer (j_compress_ptr cinfo);
void jinit_c_prep_controller (j_compress_ptr cinfo, boolean need_full_buffer);

//jctrans.cpp
void jpeg_write_coefficients (j_compress_ptr cinfo, jvirt_barray_ptr * coef_arrays);
void jpeg_copy_critical_parameters (j_decompress_ptr srcinfo,
	j_compress_ptr dstinfo);
void transencode_master_selection (j_compress_ptr cinfo,
	jvirt_barray_ptr * coef_arrays);
void start_iMCU_row (j_compress_ptr cinfo);
void start_pass_coef (j_compress_ptr cinfo, J_BUF_MODE pass_mode);
boolean compress_output (j_compress_ptr cinfo, JSAMPIMAGE input_buf);
void transencode_coef_controller (j_compress_ptr cinfo,
	jvirt_barray_ptr * coef_arrays);

// jdapistd.cpp
boolean jpeg_start_decompress (j_decompress_ptr cinfo);
boolean output_pass_setup (j_decompress_ptr cinfo);
JDIMENSION jpeg_read_scanlines (j_decompress_ptr cinfo, JSAMPARRAY scanlines,
	JDIMENSION max_lines);
JDIMENSION jpeg_read_raw_data (j_decompress_ptr cinfo, JSAMPIMAGE data,
	JDIMENSION max_lines);
boolean jpeg_start_output (j_decompress_ptr cinfo, int scan_number);
boolean jpeg_finish_output (j_decompress_ptr cinfo);

// jdatasrc.cpp
void init_source (j_decompress_ptr cinfo);
boolean fill_input_buffer (j_decompress_ptr cinfo);
void skip_input_data (j_decompress_ptr cinfo, long num_bytes);
void term_source (j_decompress_ptr cinfo);
void jpeg_stdio_src (j_decompress_ptr cinfo, CFile * infile);

// jdcolor.cpp 
void build_ycc_rgb_table (j_decompress_ptr cinfo);
void ycc_rgb_convert (j_decompress_ptr cinfo,
	JSAMPIMAGE input_buf, JDIMENSION input_row,
	JSAMPARRAY output_buf, int num_rows);
void null_convert (j_decompress_ptr cinfo,
	JSAMPIMAGE input_buf, JDIMENSION input_row,
	JSAMPARRAY output_buf, int num_rows);
void grayscale_convert (j_decompress_ptr cinfo,
	JSAMPIMAGE input_buf, JDIMENSION input_row,
	JSAMPARRAY output_buf, int num_rows);
void ycck_cmyk_convert (j_decompress_ptr cinfo,
	JSAMPIMAGE input_buf, JDIMENSION input_row,
	JSAMPARRAY output_buf, int num_rows);
void start_pass_dcolor (j_decompress_ptr cinfo);
void jinit_color_deconverter (j_decompress_ptr cinfo);

// jdhuff.cpp
void start_pass_huff_decoder (j_decompress_ptr cinfo);
void jpeg_make_d_derived_tbl (j_decompress_ptr cinfo, JHUFF_TBL * htbl,
	d_derived_tbl **pdtbl);
boolean jpeg_fill_bit_buffer (bitread_working_state * state,
	register bit_buf_type get_buffer, register int bits_left,
	int nbits);
int jpeg_huff_decode (bitread_working_state * state,
	register bit_buf_type get_buffer, register int bits_left,
	d_derived_tbl * htbl, int min_bits);
boolean process_restart (j_decompress_ptr cinfo);
boolean decode_mcu (j_decompress_ptr cinfo, JBLOCKROW *MCU_data);
void jinit_huff_decoder (j_decompress_ptr cinfo);

// jdmainct.cpp
void alloc_funny_pointers (j_decompress_ptr cinfo);
void make_funny_pointers (j_decompress_ptr cinfo);
void set_wraparound_pointers (j_decompress_ptr cinfo);
void set_bottom_pointers (j_decompress_ptr cinfo);
void start_pass_main (j_decompress_ptr cinfo, J_BUF_MODE pass_mode);
void process_data_simple_main (j_decompress_ptr cinfo,
	JSAMPARRAY output_buf, JDIMENSION *out_row_ctr,
	JDIMENSION out_rows_avail);
void process_data_context_main (j_decompress_ptr cinfo,
	JSAMPARRAY output_buf, JDIMENSION *out_row_ctr,
	JDIMENSION out_rows_avail);
void process_data_crank_post (j_decompress_ptr cinfo,
	JSAMPARRAY output_buf, JDIMENSION *out_row_ctr,
	JDIMENSION out_rows_avail);
void jinit_d_main_controller (j_decompress_ptr cinfo, boolean need_full_buffer);

//jdmaster.cpp
boolean use_merged_upsample (j_decompress_ptr cinfo);
void jpeg_calc_output_dimensions (j_decompress_ptr cinfo);
void prepare_range_limit_table (j_decompress_ptr cinfo);
void master_selection (j_decompress_ptr cinfo);
void prepare_for_output_pass (j_decompress_ptr cinfo);
void finish_output_pass (j_decompress_ptr cinfo);
void jpeg_new_colormap (j_decompress_ptr cinfo);
void jinit_master_decompress (j_decompress_ptr cinfo);

//jdphuff.cpp
void start_pass_phuff_decoder (j_decompress_ptr cinfo);
boolean progressive_process_restart (j_decompress_ptr cinfo);
boolean decode_mcu_DC_first (j_decompress_ptr cinfo, JBLOCKROW *MCU_data);
boolean decode_mcu_AC_first (j_decompress_ptr cinfo, JBLOCKROW *MCU_data);
boolean decode_mcu_DC_refine (j_decompress_ptr cinfo, JBLOCKROW *MCU_data);
boolean decode_mcu_AC_refine (j_decompress_ptr cinfo, JBLOCKROW *MCU_data);
void jinit_phuff_decoder (j_decompress_ptr cinfo);


//jdsample.cpp
void start_pass_upsample (j_decompress_ptr cinfo);
void sep_upsample (j_decompress_ptr cinfo,
	JSAMPIMAGE input_buf, JDIMENSION *in_row_group_ctr,
	JDIMENSION in_row_groups_avail,
	JSAMPARRAY output_buf, JDIMENSION *out_row_ctr,
	JDIMENSION out_rows_avail);
void fullsize_upsample (j_decompress_ptr cinfo, jpeg_component_info * compptr,
	JSAMPARRAY input_data, JSAMPARRAY *output_data_ptr);
void noop_upsample (j_decompress_ptr cinfo, jpeg_component_info * compptr,
	JSAMPARRAY input_data, JSAMPARRAY *output_data_ptr);
void int_upsample (j_decompress_ptr cinfo, jpeg_component_info * compptr,
	JSAMPARRAY input_data, JSAMPARRAY *ouput_data_ptr);
void h2v1_upsample (j_decompress_ptr cinfo, jpeg_component_info * compptr,
	JSAMPARRAY input_data, JSAMPARRAY *output_data_ptr);
void h2v2_upsample (j_decompress_ptr cinfo, jpeg_component_info * compptr,
	JSAMPARRAY input_data, JSAMPARRAY *output_data_ptr);
void h2v1_fancy_upsample (j_decompress_ptr cinfo, jpeg_component_info * compptr,
	JSAMPARRAY input_data, JSAMPARRAY *ouput_data_ptr);
void h2v2_fancy_upsample (j_decompress_ptr cinfo, jpeg_component_info * compptr,
	JSAMPARRAY input_data, JSAMPARRAY *output_data_ptr);
void jinit_upsampler (j_decompress_ptr cinfo);

// jerror.cpp
void error_exit (j_common_ptr cinfo);
void output_message (j_common_ptr cinfo);
void emit_message (j_common_ptr cinfo, int msg_level);
void format_message (j_common_ptr cinfo, TCHAR * buffer);
void reset_error_mgr (j_common_ptr cinfo);
struct jpeg_error_mgr * jpeg_std_error (struct jpeg_error_mgr * err);

// jfdctfst.cpp
void jpeg_fdct_ifast (DCTELEM * data);
void jpeg_idct_float (j_decompress_ptr cinfo, jpeg_component_info * compptr,
	JCOEFPTR coef_block,
	JSAMPARRAY output_buf, JDIMENSION output_col);

// jidctint.cpp
void jpeg_idct_islow (j_decompress_ptr cinfo, jpeg_component_info * compptr,
	JCOEFPTR coef_block,
	JSAMPARRAY output_buf, JDIMENSION output_col);

//jquant1.cpp
int select_ncolors (j_decompress_ptr cinfo, int Ncolors[]);
int output_value (j_decompress_ptr cinfo, int ci, int j, int maxj);
int largest_input_value (j_decompress_ptr cinfo, int ci, int j, int maxj);
void create_colormap (j_decompress_ptr cinfo);
void create_colorindex (j_decompress_ptr cinfo);
ODITHER_MATRIX_PTR make_odither_array (j_decompress_ptr cinfo, int ncolors);
void create_odither_tables (j_decompress_ptr cinfo);
void color_quantize (j_decompress_ptr cinfo, JSAMPARRAY input_buf,
	JSAMPARRAY output_buf, int num_rows);
void color_quantize3 (j_decompress_ptr cinfo, JSAMPARRAY input_buf,
	JSAMPARRAY output_buf, int num_rows);
void quantize_ord_dither (j_decompress_ptr cinfo, JSAMPARRAY input_buf,
	JSAMPARRAY output_buf, int num_rows);
void quantize3_ord_dither (j_decompress_ptr cinfo, JSAMPARRAY input_buf,
	JSAMPARRAY output_buf, int num_rows);
void quantize_fs_dither (j_decompress_ptr cinfo, JSAMPARRAY input_buf,
	JSAMPARRAY output_buf, int num_rows);
void alloc_fs_workspace (j_decompress_ptr cinfo);
void start_pass_1_quant (j_decompress_ptr cinfo, boolean is_pre_scan);
void finish_pass_1_quant (j_decompress_ptr cinfo);
void new_color_map_1_quant (j_decompress_ptr cinfo);
void jinit_1pass_quantizer (j_decompress_ptr cinfo);

//jutils.cpp
long jdiv_round_up (long a, long b);
long jround_up (long a, long b);
void jcopy_sample_rows (JSAMPARRAY input_array, int source_row,
	JSAMPARRAY output_array, int dest_row,
	int num_rows, JDIMENSION num_cols);
void jcopy_block_row (JBLOCKROW input_row, JBLOCKROW output_row,
	JDIMENSION num_blocks);
void jzero_far (void FAR * target, size_t bytestozero);

//jmemansi.cpp
void * jpeg_get_small (j_common_ptr cinfo, size_t sizeofobject);
void jpeg_free_small (j_common_ptr cinfo, void * object, size_t sizeofobject);
#ifdef WIN32
void  * jpeg_get_large (j_common_ptr cinfo, long sizeofobject);
void jpeg_free_large (j_common_ptr cinfo, void FAR * object, size_t sizeofobject);
#else
void huge * jpeg_get_large (j_common_ptr cinfo, long sizeofobject);
void jpeg_free_large (j_common_ptr cinfo, void huge * object, size_t sizeofobject);
#endif
long jpeg_mem_available (j_common_ptr cinfo, long min_bytes_needed,
	long max_bytes_needed, long already_allocated);
void read_backing_store (j_common_ptr cinfo, backing_store_ptr info,
#ifdef WIN32
	void *buffer_address,
#else
	void huge *buffer_address,
#endif
	long file_offset, long byte_count);
void write_backing_store (j_common_ptr cinfo, backing_store_ptr info,
	void FAR * buffer_address,
	long file_offset, long byte_count);
void close_backing_store (j_common_ptr cinfo, backing_store_ptr info);
void jpeg_open_backing_store (j_common_ptr cinfo, backing_store_ptr info,
	long total_bytes_needed);
long jpeg_mem_init (j_common_ptr cinfo);
void jpeg_mem_term (j_common_ptr cinfo);

//jmemnobs.cpp
// void * jpeg_get_small (j_common_ptr cinfo, size_t sizeofobject);
// void jpeg_free_small (j_common_ptr cinfo, void * object, size_t sizeofobject);
// void FAR * jpeg_get_large (j_common_ptr cinfo, size_t sizeofobject);
// void jpeg_free_large (j_common_ptr cinfo, void FAR * object, size_t sizeofobject);
// g jpeg_mem_available (j_common_ptr cinfo, long min_bytes_needed,
//ong max_bytes_needed, long already_allocated);
//id jpeg_open_backing_store (j_common_ptr cinfo, backing_store_ptr info,
//ong total_bytes_needed);
//long jpeg_mem_init (j_common_ptr cinfo);
// void jpeg_mem_term (j_common_ptr cinfo);

// jcapimin.cpp
void jpeg_create_compress (j_compress_ptr cinfo);
void jpeg_destroy_compress (j_compress_ptr cinfo);
void jpeg_abort_compress (j_compress_ptr cinfo);
void jpeg_suppress_tables (j_compress_ptr cinfo, boolean suppress);
void jpeg_finish_compress (j_compress_ptr cinfo);
void jpeg_write_marker (j_compress_ptr cinfo, int marker,
	const JOCTET *dataptr, unsigned int datalen);
void jpeg_write_tables (j_compress_ptr cinfo);

//jcapistd.cpp
void jpeg_start_compress (j_compress_ptr cinfo, boolean write_all_tables);
JDIMENSION jpeg_write_scanlines (j_compress_ptr cinfo, JSAMPARRAY scanlines,
	JDIMENSION num_lines);
JDIMENSION jpeg_write_raw_data (j_compress_ptr cinfo, JSAMPIMAGE data,
	JDIMENSION num_lines);
	

// jccolor.cpp
void rgb_ycc_start (j_compress_ptr cinfo);
void rgb_ycc_convert (j_compress_ptr cinfo,
	JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
	JDIMENSION output_row, int num_rows);
void rgb_gray_convert (j_compress_ptr cinfo,
	JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
	JDIMENSION output_row, int num_rows);
void cmyk_ycck_convert (j_compress_ptr cinfo,
	JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
	JDIMENSION output_row, int num_rows);
void grayscale_convert (j_compress_ptr cinfo,
	JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
	JDIMENSION output_row, int num_rows);
void null_convert (j_compress_ptr cinfo,
	JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
	JDIMENSION output_row, int num_rows);
void null_method (j_compress_ptr cinfo);
void jinit_color_converter (j_compress_ptr cinfo);

// jchuff.cpp
void start_pass_huff (j_compress_ptr cinfo, boolean gather_statistics);
void jpeg_make_c_derived_tbl (j_compress_ptr cinfo, JHUFF_TBL * htbl,
	c_derived_tbl **pdtbl);
boolean dump_buffer (comp_working_state * state);
boolean emit_bits (comp_working_state * state, unsigned int code, int size);
boolean flush_bits (comp_working_state * state);
boolean encode_one_block (comp_working_state * state, JCOEFPTR block, int last_dc_val,
	c_derived_tbl  *dctbl, c_derived_tbl *actbl);
boolean emit_restart (comp_working_state * state, int restart_num);
boolean encode_mcu_huff (j_compress_ptr cinfo, JBLOCKROW *MCU_data);
void finish_pass_huff (j_compress_ptr cinfo);
void htest_one_block (JCOEFPTR block, int last_dc_val,
	long dc_counts[], long ac_counts[]);
boolean encode_mcu_gather (j_compress_ptr cinfo, JBLOCKROW *MCU_data);
void jpeg_gen_optimal_table (j_compress_ptr cinfo, JHUFF_TBL * htbl, long freq[]);
void finish_pass_gather (j_compress_ptr cinfo);
void jinit_huff_encoder (j_compress_ptr cinfo);

// jcmarker.cpp
void emit_byte (j_compress_ptr cinfo, int val);
void emit_marker (j_compress_ptr cinfo, COMP_JPEG_MARKER mark);
void emit_2bytes (j_compress_ptr cinfo, int value);
int emit_dqt (j_compress_ptr cinfo, int index);
void emit_dht (j_compress_ptr cinfo, int index, boolean is_ac);
void emit_dac (j_compress_ptr cinfo);
void emit_dri (j_compress_ptr cinfo);
void emit_sof (j_compress_ptr cinfo, COMP_JPEG_MARKER code);
void emit_sos (j_compress_ptr cinfo);
void emit_jfif_app0 (j_compress_ptr cinfo);
void emit_adobe_app14 (j_compress_ptr cinfo);
void write_any_marker (j_compress_ptr cinfo, int marker,
	const JOCTET *dataptr, unsigned int datalen);
void write_file_header (j_compress_ptr cinfo);
void write_frame_header (j_compress_ptr cinfo);
void write_scan_header (j_compress_ptr cinfo);
void write_file_trailer (j_compress_ptr cinfo);
void write_tables_only (j_compress_ptr cinfo);
void jinit_marker_writer (j_compress_ptr cinfo);

// jcomapi.cpp
void jpeg_abort (j_common_ptr cinfo);
void jpeg_destroy (j_common_ptr cinfo);
JQUANT_TBL* jpeg_alloc_quant_table (j_common_ptr cinfo);
JHUFF_TBL* jpeg_alloc_huff_table (j_common_ptr cinfo);

//out8
// jcphuff.cpp 
void start_pass_phuff (j_compress_ptr cinfo, boolean gather_statistics);
void dump_buffer (cp_phuff_entropy_ptr entropy);
void emit_bits (cp_phuff_entropy_ptr entropy, unsigned int code, int size);
void flush_bits (cp_phuff_entropy_ptr entropy);
void emit_symbol (cp_phuff_entropy_ptr entropy, int tbl_no, int symbol);
void emit_buffered_bits (cp_phuff_entropy_ptr entropy, char * bufstart,
	unsigned int nbits);
void emit_eobrun (cp_phuff_entropy_ptr entropy);
void emit_restart (cp_phuff_entropy_ptr entropy, int restart_num);
boolean encode_mcu_DC_first (j_compress_ptr cinfo, JBLOCKROW *MCU_data);
boolean encode_mcu_AC_first (j_compress_ptr cinfo, JBLOCKROW *MCU_data);
boolean encode_mcu_DC_refine (j_compress_ptr cinfo, JBLOCKROW *MCU_data);
boolean encode_mcu_AC_refine (j_compress_ptr cinfo, JBLOCKROW *MCU_data);
void finish_pass_phuff (j_compress_ptr cinfo);
void finish_pass_gather_phuff (j_compress_ptr cinfo);
void jinit_phuff_encoder (j_compress_ptr cinfo);

// jcsample.cpp
void start_pass_downsample (j_compress_ptr cinfo);
void expand_right_edge (JSAMPARRAY image_data, int num_rows,
	JDIMENSION input_cols, JDIMENSION output_cols);
void sep_downsample (j_compress_ptr cinfo,
	JSAMPIMAGE input_buf, JDIMENSION in_row_index,
	JSAMPIMAGE output_buf, JDIMENSION out_row_group_index);
void int_downsample (j_compress_ptr cinfo, jpeg_component_info * compptr,
	JSAMPARRAY input_data, JSAMPARRAY output_data);
void fullsize_downsample (j_compress_ptr cinfo, jpeg_component_info * compptr,
	JSAMPARRAY input_data, JSAMPARRAY output_data);
void h2v1_downsample (j_compress_ptr cinfo, jpeg_component_info * compptr,
	JSAMPARRAY input_data, JSAMPARRAY output_data);
void h2v2_downsample (j_compress_ptr cinfo, jpeg_component_info * compptr,
	JSAMPARRAY input_data, JSAMPARRAY output_data);
void h2v2_smooth_downsample (j_compress_ptr cinfo, jpeg_component_info * compptr,
	JSAMPARRAY input_data, JSAMPARRAY output_data);
void fullsize_smooth_downsample (j_compress_ptr cinfo, jpeg_component_info *compptr,
	JSAMPARRAY input_data, JSAMPARRAY output_data);
void jinit_downsampler (j_compress_ptr cinfo);

// jdapimin.cpp
void jpeg_create_decompress (j_decompress_ptr cinfo);
void jpeg_destroy_decompress (j_decompress_ptr cinfo);
void jpeg_abort_decompress (j_decompress_ptr cinfo);
void jpeg_set_marker_processor (j_decompress_ptr cinfo, int marker_code,
	jpeg_marker_parser_method routine);
void default_decompress_parms (j_decompress_ptr cinfo);
int jpeg_read_header (j_decompress_ptr cinfo, boolean require_image);
int jpeg_consume_input (j_decompress_ptr cinfo);
boolean jpeg_input_complete (j_decompress_ptr cinfo);
boolean jpeg_has_multiple_scans (j_decompress_ptr cinfo);
boolean jpeg_finish_decompress (j_decompress_ptr cinfo);

// jdatadst.cpp
void init_destination (j_compress_ptr cinfo);
boolean empty_output_buffer (j_compress_ptr cinfo);
void term_destination (j_compress_ptr cinfo);
void jpeg_stdio_dest (j_compress_ptr cinfo, CFile * outfile);

//jdcoefct.cpp
void start_iMCU_row (j_decompress_ptr cinfo);
void jdcoefct_start_input_pass (j_decompress_ptr cinfo);
void start_output_pass (j_decompress_ptr cinfo);
int decompress_onepass (j_decompress_ptr cinfo, JSAMPIMAGE output_buf);
int dummy_consume_data (j_decompress_ptr cinfo);
int consume_data (j_decompress_ptr cinfo);
int decompress_data (j_decompress_ptr cinfo, JSAMPIMAGE output_buf);
boolean smoothing_ok (j_decompress_ptr cinfo);
int decompress_smooth_data (j_decompress_ptr cinfo, JSAMPIMAGE output_buf);
void jinit_d_coef_controller (j_decompress_ptr cinfo, boolean need_full_buffer);

//jddctmgr.cpp
void start_pass (j_decompress_ptr cinfo);
void jinit_inverse_dct (j_decompress_ptr cinfo);

//jdinput.cpp
void initial_setup (j_decompress_ptr cinfo);
void per_scan_setup (j_decompress_ptr cinfo);
void latch_quant_tables (j_decompress_ptr cinfo);
void start_input_pass (j_decompress_ptr cinfo);
void finish_input_pass (j_decompress_ptr cinfo);
int consume_markers (j_decompress_ptr cinfo);
void reset_input_controller (j_decompress_ptr cinfo);
void jinit_input_controller (j_decompress_ptr cinfo);

//jdmarker.cpp
boolean get_soi (j_decompress_ptr cinfo);
boolean get_sof (j_decompress_ptr cinfo, boolean is_prog, boolean is_arith);
boolean get_sos (j_decompress_ptr cinfo);
boolean get_app0 (j_decompress_ptr cinfo);
boolean get_app14 (j_decompress_ptr cinfo);
boolean get_dac (j_decompress_ptr cinfo);
boolean get_dht (j_decompress_ptr cinfo);
boolean get_dqt (j_decompress_ptr cinfo);
boolean get_dri (j_decompress_ptr cinfo);
boolean skip_variable (j_decompress_ptr cinfo);
boolean next_marker (j_decompress_ptr cinfo);
boolean first_marker (j_decompress_ptr cinfo);
int read_markers (j_decompress_ptr cinfo);
boolean read_restart_marker (j_decompress_ptr cinfo);
boolean jpeg_resync_to_restart (j_decompress_ptr cinfo, int desired);
void reset_marker_reader (j_decompress_ptr cinfo);
void jinit_marker_reader (j_decompress_ptr cinfo);

// jdmerge.cpp
void merge_build_ycc_rgb_table (j_decompress_ptr cinfo);
void start_pass_merged_upsample (j_decompress_ptr cinfo);
void merged_2v_upsample (j_decompress_ptr cinfo,
	JSAMPIMAGE input_buf, JDIMENSION *in_row_group_ctr,
	JDIMENSION in_row_groups_avail,
	JSAMPARRAY output_buf, JDIMENSION *out_row_ctr,
	JDIMENSION out_rows_avail);
void merged_1v_upsample (j_decompress_ptr cinfo,
	JSAMPIMAGE input_buf, JDIMENSION *in_row_group_ctr,
	JDIMENSION in_row_groups_avail,
	JSAMPARRAY output_buf, JDIMENSION *out_row_ctr,
	JDIMENSION out_rows_avail);
void h2v1_merged_upsample (j_decompress_ptr cinfo,
	JSAMPIMAGE input_buf, JDIMENSION in_row_group_ctr,
	JSAMPARRAY output_buf);
void h2v2_merged_upsample (j_decompress_ptr cinfo,
	JSAMPIMAGE input_buf, JDIMENSION in_row_group_ctr,
	JSAMPARRAY output_buf);
void jinit_merged_upsampler (j_decompress_ptr cinfo);

// jdpostct.cpp
void start_pass_dpost (j_decompress_ptr cinfo, J_BUF_MODE pass_mode);
void post_process_1pass (j_decompress_ptr cinfo,
	JSAMPIMAGE input_buf, JDIMENSION *in_row_group_ctr,
	JDIMENSION in_row_groups_avail,
	JSAMPARRAY output_buf, JDIMENSION *out_row_ctr,
	JDIMENSION out_rows_avail);
void post_process_prepass (j_decompress_ptr cinfo,
	JSAMPIMAGE input_buf, JDIMENSION *in_row_group_ctr,
	JDIMENSION in_row_groups_avail,
	JSAMPARRAY output_buf, JDIMENSION *out_row_ctr,
	JDIMENSION out_rows_avail);
void post_process_2pass (j_decompress_ptr cinfo,
	JSAMPIMAGE input_buf, JDIMENSION *in_row_group_ctr,
	JDIMENSION in_row_groups_avail,
	JSAMPARRAY output_buf, JDIMENSION *out_row_ctr,
	JDIMENSION out_rows_avail);
void jinit_d_post_controller (j_decompress_ptr cinfo, boolean need_full_buffer);


//jdtrans.cpp
jvirt_barray_ptr * jpeg_read_coefficients (j_decompress_ptr cinfo);
void transdecode_master_selection (j_decompress_ptr cinfo);

//jfdctflt.cpp
void jpeg_fdct_float (FAST_FLOAT * data);
void jpeg_fdct_islow (DCTELEM * data);

//jidctfst.cpp
void jpeg_idct_ifast (j_decompress_ptr cinfo, jpeg_component_info * compptr,
	JCOEFPTR coef_block,
	JSAMPARRAY output_buf, JDIMENSION output_col);

//jidctred.cpp
void jpeg_idct_4x4 (j_decompress_ptr cinfo, jpeg_component_info * compptr,
	JCOEFPTR coef_block,
	JSAMPARRAY output_buf, JDIMENSION output_col);
void jpeg_idct_2x2 (j_decompress_ptr cinfo, jpeg_component_info * compptr,
	JCOEFPTR coef_block,
	JSAMPARRAY output_buf, JDIMENSION output_col);
void jpeg_idct_1x1 (j_decompress_ptr cinfo, jpeg_component_info * compptr,
	JCOEFPTR coef_block,
	JSAMPARRAY output_buf, JDIMENSION output_col);


//jquant2.cpp
void prescan_quantize (j_decompress_ptr cinfo, JSAMPARRAY input_buf,
	JSAMPARRAY output_buf, int num_rows);
boxptr find_biggest_color_pop (boxptr boxlist, int numboxes);
boxptr find_biggest_volume (boxptr boxlist, int numboxes);
void update_box (j_decompress_ptr cinfo, boxptr boxp);
int median_cut (j_decompress_ptr cinfo, boxptr boxlist, int numboxes,
	int desired_colors);
void compute_color (j_decompress_ptr cinfo, boxptr boxp, int icolor);
void select_colors (j_decompress_ptr cinfo, int desired_colors);
int find_nearby_colors (j_decompress_ptr cinfo, int minc0, int minc1, int minc2,
	JSAMPLE colorlist[]);
void find_best_colors (j_decompress_ptr cinfo, int minc0, int minc1, int minc2,
	int numcolors, JSAMPLE colorlist[], JSAMPLE bestcolor[]);
void fill_inverse_cmap (j_decompress_ptr cinfo, int c0, int c1, int c2);
void pass2_no_dither (j_decompress_ptr cinfo,
	JSAMPARRAY input_buf, JSAMPARRAY output_buf, int num_rows);
void pass2_fs_dither (j_decompress_ptr cinfo,
	JSAMPARRAY input_buf, JSAMPARRAY output_buf, int num_rows);
void init_error_limit (j_decompress_ptr cinfo);
void finish_pass1 (j_decompress_ptr cinfo);
void finish_pass2 (j_decompress_ptr cinfo);
void start_pass_2_quant (j_decompress_ptr cinfo, boolean is_pre_scan);
void new_color_map_2_quant (j_decompress_ptr cinfo);
void jinit_2pass_quantizer (j_decompress_ptr cinfo);

//jmemmgr.cpp
void print_mem_stats (j_common_ptr cinfo, int pool_id);
void out_of_memory (j_common_ptr cinfo, int which);
void * alloc_small (j_common_ptr cinfo, int pool_id, size_t sizeofobject);
#ifdef WIN32
void * alloc_large (j_common_ptr cinfo, int pool_id, long sizeofobject);
#else
void huge * alloc_large (j_common_ptr cinfo, int pool_id, long sizeofobject);
#endif
JSAMPARRAY alloc_sarray (j_common_ptr cinfo, int pool_id,
	JDIMENSION samplesperrow, JDIMENSION numrows);
JBLOCKARRAY alloc_barray (j_common_ptr cinfo, int pool_id,
	JDIMENSION samplesperrow, JDIMENSION numrows);
jvirt_sarray_ptr request_virt_sarray (j_common_ptr cinfo, int pool_id, boolean pre_zero,
	JDIMENSION samplesperrow, JDIMENSION numrows,
	JDIMENSION maxaccess);
jvirt_barray_ptr request_virt_barray (j_common_ptr cinfo, int pool_id, boolean pre_zero,
	JDIMENSION blocksperrow, JDIMENSION numrows,
	JDIMENSION maxaccess);	
void realize_virt_arrays (j_common_ptr cinfo);
void do_sarray_io (j_common_ptr cinfo, jvirt_sarray_ptr ptr, boolean writing);
void do_barray_io (j_common_ptr cinfo, jvirt_barray_ptr ptr, boolean writing);
JSAMPARRAY access_virt_sarray (j_common_ptr cinfo, jvirt_sarray_ptr ptr,
	JDIMENSION start_row, JDIMENSION num_rows,
	boolean writable);
JBLOCKARRAY access_virt_barray (j_common_ptr cinfo, jvirt_barray_ptr ptr,
	JDIMENSION start_row, JDIMENSION num_rows,
	boolean writable);
void free_pool (j_common_ptr cinfo, int pool_id);
void self_destruct (j_common_ptr cinfo);
void jinit_memory_mgr (j_common_ptr cinfo);




protected:
  DECLARE_SERIAL(SECJpeg)

public:
  virtual void Serialize(CArchive& ar);
#ifndef WIN32
  BOOL SECQuantizeImage();
#endif

  // Construction                                 
  SECJpeg();
  SECJpeg(LPCTSTR lpszFileName);
  ~SECJpeg();

  // Destruction

  
  // Operations


  // Implementation 
private:
#ifdef WIN32
  LPBYTE m_lpArrayPtr;  // iterator that fills up m_lpSrcBits
  short *cache;
#else
  BYTE huge *m_lpArrayPtr;
  short huge *cache;
#endif

public:
BOOL m_bQuantize;  	// flag to invoke color dithering for images loaded into
					// systems with a different number of colors.
LPBYTE m_lpQuant;
long	m_nQuality;	//Quality factor; big factors in compression come into play with this setting
long	m_nSmoothing;
BOOL	m_bOptimizeEntropyCoding;
BOOL	m_bArithmeticCoding;	// use arithmetic or Huffman scheme

#ifdef _DEBUG
	void AssertValid() const;
#endif


};

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif // __SECJPEG_H__

