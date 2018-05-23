/* Import-Export module. Copyright 1995-96 by DataPak Software, Inc.  This software is a
part of the total PAIGE library.

This source file contains all the member functions for the HTML import class (derived
from the PaigeImportFilter class. */

#include "stdafx.h"

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
#include "text2html.h" // QUALCOMM == Kusuma
#include "html2text.h" // QUALCOMM == Kusuma
#include "HTMLUtils.h"

// support for HTML 4.x "OBJECT" tag
#include "ObjectSpec.h"
#include "PgEmbeddedObject.h"
#include "PgCntrItem.h"

#include "DebugNewHelpers.h"


//QUALCOMM Begin - Kusuma (mimic v3.14)
struct format_stack {
	style_info			style;
	font_info			font;
};
typedef struct format_stack format_stack;
typedef format_stack PG_FAR *format_stack_ptr;
//QUALCOMM End - Kusuma (mimic v3.14)

// To silence compiler warning: unreferenced function
#pragma warning(disable : 4505)

static pg_boolean equal_string (pg_char_ptr str1, pg_char_ptr str2);
static pg_boolean all_hex (pg_char_ptr hex_str, short str_size);
static void param_to_lowercase (pg_char_ptr param);
static long table_command (pg_char_ptr table, pg_char_ptr argument, pg_short_t arg_size);
static pg_boolean is_hex_char (pg_char the_char);
static long string_to_hex (pg_char_ptr string);
static void set_style (long style_index, style_info_ptr style, font_info_ptr font, par_info_ptr par);
static void to_lower (pg_char_ptr the_char);
static tag_attribute_ptr find_parameter (tag_attribute_ptr params, long num_params, short wanted_code);
static long numeric_value (pg_char_ptr param);
static long decimal_value_percent (pg_char_ptr param, long max_for_percent);
static long image_type (pg_char_ptr filename);
static pg_boolean test_extension (pg_char_ptr file_ext, pg_char_ptr match);
static void translate_single_file (pg_char_ptr URL, pg_char_ptr filename);


/* PaigeImportFilter constructor. This sets up the various members common to all filters.
No error checking or file verification occurs just yet. */


PaigeHTMLImportFilter::PaigeHTMLImportFilter ()
	:	html2text(FALSE)
{
   file_type = pg_html_type;  // Default type for this class
   feature_bits = IMPORT_TEXT_FEATURE | IMPORT_TEXT_FORMATS_FEATURE
               | IMPORT_PAR_FORMATS_FEATURE | IMPORT_PAGE_INFO_FEATURE
               | IMPORT_EMBEDDED_OBJECTS_FEATURE
               | IMPORT_HYPERTEXT_FEATURE
               | IMPORT_SERIAL_SETUP
               | IMPORT_PARS_EXTERNAL
               | IMPORT_EXTEND_OUTPUT
               | IMPORT_NO_LEADING_SPACES;
   
   pgFillBlock(&default_font, sizeof(font_info), 0);
   pgFillBlock(html_styles, sizeof(long) * NUM_HTML_STYLES, 0);
   
   point_levels[0] = 0x00080000;
   point_levels[1] = 0x000A0000;
   point_levels[2] = 0x000C0000;
   point_levels[3] = 0x000E0000;
   point_levels[4] = 0x00120000;
   point_levels[5] = 0x00180000;
   point_levels[6] = 0x00240000;
   
   current_point_level = base_font_index = DEF_POINT_LEVEL;
   font_tag_enable = FALSE;
   deflist_level = 0;
   cr_enable = 0;
   list_level = 0;
   blockquote_level = 0;
   target_link_style = 0;
   //title_ptr = NULL;
   inside_title = FALSE;
   //QUALCOMM Begin - Kusuma (mimic v3.14) added style-stack-ref
   param_ref = attribute_ref = anchor_ref = table_text = tables_ref = style_stack_ref = MEM_NULL;
   linebreak_char = 0x0D;
   doing_row = code_listing = FALSE;
   doing_table = 0;

   embed_last_action = FALSE;

   bk_color.red = bk_color.green = bk_color.blue = 0xCCCC;
   bk_color.alpha = 0;
   
   list_indent_value = DEFLIST_INDENT_VALUE;
}


/* PaigeImportFilter destructor. This removes common items. */

PaigeHTMLImportFilter::~PaigeHTMLImportFilter ()
{
   ::DisposeNonNilMemory(table_text);
   ::DisposeNonNilMemory(tables_ref);
   //QUALCOMM Begin - Kusuma (mimic v3.14)
   ::DisposeNonNilMemory(style_stack_ref);
   //QUALCOMM End - Kusuma (mimic v3.14)

   FreeStyleSheetDefs();
}


void PaigeHTMLImportFilter::FreeStyleSheetDefs()
{
   while ( !style_sheet_defs.IsEmpty() )
      delete style_sheet_defs.RemoveHead();
}


void PaigeHTMLImportFilter::SetBaseURLString(const char * szBaseURLString)
{
	strncpy( (char *) base_url_string, szBaseURLString, sizeof(base_url_string) );
	base_url_string[sizeof(base_url_string)-1] = '\0';
}


/* pgPrepareImport sets up the necessary extra buffers to begin importing. */

pg_error PaigeHTMLImportFilter::pgPrepareImport (void)
{
   font_info_ptr  first_font;
   rectangle      bounds;

//QUALCOMM Begin - Kusuma (mimic v3.14)
   FreeStyleSheetDefs();

   //def_text_color = cur_text_color = paige_globals->def_style.fg_color;
   current_style.fg_color = paige_globals->def_style.fg_color;
//QUALCOMM End - Kusuma (mimic v3.14)

   param_ref = ::MemoryAlloc(paige_globals->mem_globals, sizeof(tag_attribute), 0, 6);
   //QUALCOMM Begin - Kusuma (mimic v3.14)
   style_stack_ref = ::MemoryAlloc(paige_globals->mem_globals, sizeof(format_stack), 0, 2);
   //QUALCOMM End - Kusuma (mimic v3.14)
   attribute_ref_size = 256;
   attribute_ref = ::MemoryAllocClear(paige_globals->mem_globals, sizeof(pg_char), attribute_ref_size, 128);
   attribute_ptr = (pg_char_ptr)UseMemory(attribute_ref);
   attribute_index = 0;
   
   if (default_font.name[0] == 0) {

#ifdef MAC_PLATFORM
      pgBlockMove("Times", &default_font.name[1], 5);
      default_font.name[0] = 5;
#else
//      pgBlockMove("Times New Roman", &default_font.name[1], 15);
//      default_font.name[0] = 15;
		pgBlockMove( &paige_globals->def_font, &default_font, sizeof(font_info) );
#endif

   }
   
   import_pg_rec->procs.font_proc(import_pg_rec, &default_font);

   first_font = (font_info_ptr)UseMemory(import_pg_rec->fonts);
   pgBlockMove(&default_font, first_font, sizeof(font_info));
   UnuseMemory(import_pg_rec->fonts);

   current_par = translator.par_format;
   translator.font_changed = TRUE;
   
   CreateHTMLStyles(import_pg, &default_font.name[1], html_styles);
   GetStylesheet(html_styles[body_stylesheet], TRUE);
   //QUALCOMM Begin === Kusuma
   //The body style has to be set to the current Message Font size
   current_style.point = point_levels[2];
   //QUALCOMM End === Kusuma

   saved_style = current_style;
   saved_font = current_font;
   
   // Hyperlinks were not being imported with the proper style when the imported
   // was transferred from the "shell" reference to the original Paige reference.
   // The problem there was twofold - the hyperlink styles weren't always being
   // setup in the original Paige reference. Even if they were being setup - the
   // IDs of the styles didn't necessarily match up between the "shell" reference
   // and the original Paige reference. The solution is to initialize ourselves
   // with the style IDs from the destination Paige reference.
   if (original_pg)
   {
      // import_pg is a "shell" Paige reference, so initialize ourselves with
      // the style IDs from original_pg, the destination Paige reference.
      pgInitDefaultTarget(original_pg, &default_target);
      pgInitDefaultSource(original_pg, &default_source);
   }
   else
   {
      // No "shell" Paige reference is being used - import_pg is the destination
      // Paige reference, so initialize ourselves with the style IDs from it.
      pgInitDefaultTarget(import_pg, &default_target);
      pgInitDefaultSource(import_pg, &default_source);
   }

   image_callback = (embed_callback)paige_globals->embed_callback_proc;
   
   if (import_bits & IMPORT_BKCOLOR_FLAG)
      pgSetPageColor(import_pg, &bk_color);
   
   if (import_bits & IMPORT_BROWSER_PAGE_FLAG)
      import_pg_rec->doc_info.attributes &= (~(V_REPEAT_BIT | BOTTOM_FIXED_BIT));

   GetMemoryRecord(original_pg_rec->wrap_area, 1, &bounds);
   page_width = bounds.bot_right.h - bounds.top_left.h;

   if (original_pg_rec->doc_info.attributes & USE_MARGINS_BIT)
      page_width -= (original_pg_rec->doc_info.margins.top_left.h + original_pg_rec->doc_info.margins.bot_right.h);
 
   page_width -= (import_pg_rec->globals->def_par.indents.left_indent
               + import_pg_rec->globals->def_par.indents.right_indent + 4);

   return      NO_ERROR;
}


/* pgImportDone disposes all structs created for pgPrepareImport(). */

pg_error PaigeHTMLImportFilter::pgImportDone (void)
{
   ::DisposeNonNilMemory(param_ref);
   ::DisposeNonNilMemory(anchor_ref);

   if (attribute_ref)
      UnuseAndDispose(attribute_ref);

   FreeStyleSheetDefs();

   return      NO_ERROR;
}


/* ImportCleanup fixes all the table entries. */

void PaigeHTMLImportFilter::pgImportCleanup ()
{
   if (tables_ref) {
      select_pair_ptr         table_offsets;
      long              num_tables;
      
      num_tables = GetMemorySize(tables_ref);
      table_offsets = (select_pair_ptr)UseMemory(tables_ref);

      while (num_tables) {
         
         pgMeasureMaxColumns(import_pg_rec, table_offsets);

         ++table_offsets;
         --num_tables;
      }
      
      UnuseAndDispose(tables_ref);
      tables_ref = MEM_NULL;
   }
}

/* pgVerifySignature returns NO_ERROR if this is an RTF file. */

pg_error PaigeHTMLImportFilter::pgVerifySignature ()
{
   return   ::pgVerifyHTML((pg_file_unit)filemap, io_proc, file_begin);
}


/* pgReadNextBlock is the major entry point that reads the next block of text, sets up paragraph
and text formats and returns something for the base class to insert into the pg_ref. */

pg_boolean PaigeHTMLImportFilter::pgReadNextBlock (void)
{
   pg_char           next_char;
   pg_boolean        transfer_text = FALSE;
   pg_error       	 result = NO_ERROR;

   translator.bytes_transferred = 0;
   translator.flags = 0;

   if (current_style.embed_object) {   
      
      ProcessEmbed();
      return   TRUE;
   }
   
   translator.format = current_style;
   translator.font = current_font;

   while ( (result = pgGetImportChar(&next_char)) == NO_ERROR ) {
     
      if (next_char == TAG_COMMAND_CHAR) {
         long        	command_id;
         pg_boolean     end_tag;

         command_id = ReadHTMLTag(&end_tag);
         
         if (command_id) {
            tag_attribute_ptr       params;
            long                 num_params;
            short                table_id, tag;
            
            if (command_id == -1 && translator.bytes_transferred)
               break;

            table_id = pgHiWord(command_id);
            tag = pgLoWord(command_id);
            
            if (end_tag)
               tag = static_cast<short>(-tag);
            
            params = (tag_attribute_ptr)UseMemory(param_ref);
            num_params = GetMemorySize(param_ref);
            
            if (code_listing) {
               
               if (table_id == CONTROL_COMMAND)
                  if (tag == -code_listing_command)
                     transfer_text = DoControlTag(tag, params, num_params);
            }
            else {

               if (!this->PreProcessTag(table_id, tag, params, num_params)) {
               
	               switch (table_id) {
	                  
	                  case CONTROL_COMMAND:
	                     transfer_text |= DoControlTag(tag, params, num_params);
	                     break;

	                  case STYLE_COMMAND:
	                     transfer_text |= DoStyleTag(tag, params, num_params);
	                     break;

	                  case DATA_COMMAND:
	                     transfer_text |= DoDataTag(tag, params, num_params);
	                     break;

	                  case TABLE_DATA_COMMAND:
	                     transfer_text |= DoTableTag(tag, params, num_params);
	                     break;
	               }
	           }
            }

            UnuseMemory(param_ref);
         }        
      }
      else
      if (next_char == '&' && !code_listing) {
         pg_char        translation[32];
         short       	trans_ctr = 0;
         
		 pgGetImportChar(&next_char);
         
         embed_last_action = FALSE;
         translation[0] = next_char;
         ++trans_ctr;

         while (trans_ctr < 31) {
            
            translation[trans_ctr] = 0;
            
			// Heuristic here.  If we see "&nbsp" without the trailing semi-colon,
			// then just act likes it's there.  Seems like a common mistake to forget
			// the trailing semi-colon, and browsers seem to account for this.
			if (trans_ctr == 4 && strnicmp((char*)translation, "nbsp", 4) == 0 &&
				pgNextImportChar() != ';')
			{
				translation[trans_ctr] = ';';
				break;
			}

            if (pgNextImportChar() == '<')
            	break;

            if (pgGetImportChar(&translation[trans_ctr]))
               break;
           
            if (translation[trans_ctr] == ';')
               break;
            if (translation[trans_ctr] < 0x21)
           	   break;

            ++trans_ctr;
         }
        
         if (translation[trans_ctr] != ';') {
        	 short		out_index;
        	 
			 // QUALCOMM Begin == Kusuma
			 // Don't read in title, because we don't use the information and for certain
			 // invalid messages the title tags caused Paige to fail a memory allocation
			 // which caused Eudora to quit. We also don't want the title to show up
			 // so skip outputting when inside a title tag.
			 //
			 // If & did not precede one of the special chars like nbsp etc.
			 // then include it as part of the text.
			 //if (!title_ptr)
			 if (!inside_title)
				OutputCharacter('&');
			 // QUALCOMM End == Kusuma

        	 for (out_index = 0; out_index <= trans_ctr; ++out_index) {
        	    
        	    if (!translation[out_index])
        	    	break;

			    // Don't read in title, because we don't use the information and for certain
			    // invalid messages the title tags caused Paige to fail a memory allocation
			    // which caused Eudora to quit. We also don't want the title to show up
			    // so skip outputting when inside a title tag.
	         	//if (title_ptr) {
			    //
			    //	title_ptr = (pg_char_ptr)AppendMemory(original_pg_rec->doc_info.title, 1, FALSE);
			    //	*title_ptr = translation[out_index];
	         	//}
	         	//else
				if (!inside_title)
	            	OutputCharacter(translation[out_index]);
	         }
         }
         else {
        
	         translation[trans_ctr] = 0;
	        
	         next_char = TranslateCharacter(translation);
	        
			 //QUALCOMM == Kusuma
			 //Also check for the escape character itself. Japanese trnaslator s/w like
			 //Shift JIS add escode characters to encode the Japanese characters and hence
			 //shouldn't be changed to spaces
	         //if (next_char < ' ')

			 //	We allow these low characters when coming in from other sources like pasting
			 // non-HTML. Allow them also when importing HTML for consistency and so that
			 // we preserve messages as close to their original form as possible. If anyone
			 // ever uncomments this to exclude low characters again, be sure to maintain
			 // the Japanese exclusion.
			 //if ((next_char < ' ') && (next_char != 0x1B))
	         //   next_char = ' ';

			 // Don't read in title, because we don't use the information and for certain
			 // invalid messages the title tags caused Paige to fail a memory allocation
			 // which caused Eudora to quit. We also don't want the title to show up
			 // so skip outputting when inside a title tag.
	         //if (title_ptr) {
			 //  
			 //   title_ptr = (pg_char_ptr)AppendMemory(original_pg_rec->doc_info.title, 1, FALSE);
			 //   *title_ptr = next_char;
	         //}
	         //else
			 if (!inside_title)
	            OutputCharacter(next_char);
	     }
      }
      else {
         
		 // Don't read in title, because we don't use the information and for certain
		 // invalid messages the title tags caused Paige to fail a memory allocation
		 // which caused Eudora to quit. We also don't want the title to show up
		 // so skip outputting when inside a title tag.
         //if (title_ptr) {
         //     
         //     if (next_char >= ' ') {
         //     
		 //	  	title_ptr = (pg_char_ptr)AppendMemory(original_pg_rec->doc_info.title, 1, FALSE);
		 //	  	*title_ptr = next_char;
		 //	  }
         // }
         //else
         //if (next_char >= ' ' || cr_enable) {
         if ( !inside_title && (next_char >= ' ' || cr_enable) ) {
            
            if (doing_table && !doing_row)
               next_char = 0;
            else
            if (next_char == 0x0A) {
               
               if (last_char_in == linebreak_char)
                  next_char = 0;
               else
                  next_char = linebreak_char;
            }
            else
            if (next_char == 0x0D)
               next_char = linebreak_char;

			//QUALCOMM == Kusuma
			//Check for the escape code.
            //if (next_char < ' ' && !cr_enable)
			if (next_char < ' ' && !cr_enable && next_char != 0x1B)
               next_char = (pg_char)' ';

            if (next_char) {
               
               if (cr_enable)
                  last_char_out = 0x21;   // Fake this so everything is literal
               
               if (OutputCharacter(next_char))
                  break;
            }
         }
         else
         if (next_char == 0x0D || next_char == 0x0A)
		 {	 
            if (!doing_table && !doing_row) {

				next_char = (pg_char)' ';

				if (OutputCharacter(next_char))
				   break;
			}
		 }
		 //QUALCOMM Begin - Kusuma
		 //Need to preserve the Esc char for Japanese Shift JIS stuff
		 else if (next_char == 0x1B)
			 if (OutputCharacter(next_char))
				 break;
		 //QUALCOMM End - Kusuma
      }

      if (transfer_text && !code_listing)
         break;
   }

//QUALCOMM Begin - Kusuma (mimic v3.14)   
   //current_style.fg_color = cur_text_color;
//QUALCOMM End - Kusuma (mimic v3.14)   

   if (translator.bytes_transferred)
      translator.format_changed = translator.font_changed = TRUE;
   
   if (current_par.table.table_columns)
      current_par.table.unique_id = table_unique_id;
   else
      current_par.table.unique_id = 0;

   bytes_read = (filepos - file_begin) - buffer_size + buffer_index;
   io_result = result;

   if (result == NO_ERROR || translator.bytes_transferred)
      return   TRUE;
   
   return   FALSE;
}


/* ReadHTMLTag reads in the HTML tag, and if recognized returns the table type (hiword)
and the tag ID code (loword), or zero if not recognized.  If tag is known then any
attributes are built in param_ref memory_ref;  if end_tag is set to TRUE then the
tab is an END-tag value.  In every case, the file skips to the first char following the >
bracket (also truncates trailing spaces). */

long PaigeHTMLImportFilter::ReadHTMLTag (pg_boolean PG_FAR *end_tag)
{
   pg_char_ptr    	command, unknown_args;
   pg_short_t     	arg_size;
   pg_char        	next_char;
   pg_boolean     	possible_params = FALSE;
   long        		item_id, end_attributes;

// First we prcesses the whole tag so it is parsed into a single buffer:

   attribute_index = 0;
   *end_tag = FALSE;
   
   if (code_listing) {
      
      PushOutputState();
      OutputCharacter('<');
   }
   
   next_char = pgNextImportChar();

   if (next_char == '!') {
	     pg_char		previous_char, terminator;

         // A comment, ignore.
         
         pgGetImportChar(&next_char);

         previous_char = 0;
		 terminator = pgNextImportChar();

         while (pgGetImportChar(&next_char) == NO_ERROR) {
            
            if (code_listing)
               OutputCharacter(next_char);

            if (next_char == ENDTAG_COMMAND_CHAR) {
               	
               	if (previous_char == '-' || terminator != '-')
                  	break;
            }
			
			previous_char = next_char;
         }
         
         return   0;
   }

   if (next_char >= 'A' && next_char <= 'Z')
       next_char += 0x20;
   
   if (next_char != 0x2F && (next_char < 'a' || next_char > 'z')) {
   		
   		OutputCharacter('<');
   		return	0;
   }
   
   while (pgGetImportChar(&next_char) == NO_ERROR) {
      
      if (code_listing)
         OutputCharacter(next_char);

      // We used to convert everything to lower case here. Most of the time that
      // worked, because when we're interpreting tags and attribute values the
      // constant strings that we're looking for are all lower case. However
      // it turned out that for embedded images if the img src CID was in
      // upper case, then our ability to find the corresponding image was
      // broken by converting everything to lower case here.
      //
      // We now convert to lower case as needed in table_command.

      if (next_char == ENDTAG_COMMAND_CHAR)
         break;
      
      if (next_char <= ' ')
         InsertTokenChar((pg_char)' ', TRUE);
      else
      if (next_char == '=') {
         
         InsertTokenChar((pg_char)' ', TRUE);
         InsertTokenChar(next_char, FALSE);
         InsertTokenChar((pg_char)' ', TRUE);
      }
      else
      if ( (next_char == '"') || (next_char == '\'') ) {
         // Parse either single or double quoted attributes
         pg_char     quote_char = next_char;
         
         InsertTokenChar((pg_char)' ', TRUE);
         InsertTokenChar(quote_char, FALSE);

         while (pgGetImportChar(&next_char) == NO_ERROR) {
            // Don't stop until we hit the closing quotation
            //if (next_char < (pg_char)' ')
            //   break;

            InsertTokenChar(next_char, FALSE);
            
            if (next_char == quote_char)
               break;
         }
         
         InsertTokenChar((pg_char)' ', TRUE);
      }
      else
         InsertTokenChar(next_char, FALSE);
   }

   InsertTokenChar((pg_char)' ', TRUE);
   end_attributes = attribute_index;
   InsertTokenChar((pg_char)'\0', FALSE);

// Line is parsed, so read the tag and possible attributes.
 
   attribute_index = 0;
   command = GetNextToken(&arg_size);
   SetMemorySize(param_ref, 0);
 
   if (command != NULL && arg_size) {
      
      if (*command == NTAG_COMMAND_CHAR) {
         
         *end_tag = TRUE;
         ++command;
         --arg_size;
         
         possible_params = FALSE;
      }
      else
         possible_params = (pg_boolean)(attribute_ptr[attribute_index] != '\0');
 
      if ((item_id = table_command(control_commands, command, arg_size)) != 0) {
         
         if (possible_params)
            ProcessAttributes(control_attributes, CONTROL_COMMAND);

         return   (item_id | ((long)CONTROL_COMMAND << 16));
      }

      if ((item_id = table_command(html_style_commands, command, arg_size)) != 0) {

         if (possible_params)
            ProcessAttributes(style_attributes, STYLE_COMMAND);

         return   (item_id | ((long)STYLE_COMMAND << 16));
      }

      if ((item_id = table_command(data_commands, command, arg_size)) != 0) {

         if (possible_params)
            ProcessAttributes(data_attributes, DATA_COMMAND);

         return   (item_id | ((long)DATA_COMMAND << 16));
      }

      if ((item_id = table_command(table_data_commands, command, arg_size)) != 0) {

         if (possible_params)
            ProcessAttributes(data_attributes, TABLE_DATA_COMMAND);

         return   (item_id | ((long)TABLE_DATA_COMMAND << 16));
      }
      
   // Unimplemented tag:

      unknown_args = attribute_ptr;
      unknown_args += attribute_index;
      
      if (UnimplementedTag(command, unknown_args, *end_tag)) {

         translator.format_changed |= !pgEqualStruct(&translator.format, &current_style, SIGNIFICANT_STYLE_SIZE);
         translator.font_changed |= !pgEqualStruct(&translator.font, &current_font, SIGNIFICANT_FONT_SIZE);

         if (!translator.bytes_transferred) {

            translator.format = current_style;
            translator.font = current_font;
         }
      
         return   -1;
      }
	  else
	  if ((item_id = table_command(errorlog_tags, command, arg_size)) != 0) {
		   long			offset = ImportedOffset();
		   
		   switch (item_id) {

				case uns_applet:
					InsertErrorLog(APPLET_ERROR, offset, offset + 1, NULL, FALSE);
					break;
					
				case uns_form:
					InsertErrorLog(FORMS_ERROR, offset, offset + 1, NULL, FALSE);
					break;

				case uns_frame:
					InsertErrorLog(FRAME_ERROR, offset, offset + 1, NULL, FALSE);
					break;
		   }
      }
   }

   return      0;
}


/* GetNextToken returns a pointer to the next token in attribute_ptr[attribute_index],
also setting the token_size to the number of chars. Or, if no more tokens, NULL is returned.
Also note that if the token is surrounded in "quotes" then the quotes are NOT part of the
result. */

pg_char_ptr PaigeHTMLImportFilter::GetNextToken (pg_short_t PG_FAR *token_size)
{
   pg_char_ptr       token = NULL;
   pg_char           next_char;
   pg_char           quote_char;

   *token_size = 0;

   if (attribute_ptr[attribute_index] != '\0') {
   
      token = &attribute_ptr[attribute_index];
      
      if ( (attribute_ptr[attribute_index] == '"') || (attribute_ptr[attribute_index] == '\'') ) {
         
         quote_char = attribute_ptr[attribute_index]; // Remember leading quote char
         
         ++token;       // Skip leading quote
         ++attribute_index;

         for (;;) {
            
            next_char = attribute_ptr[attribute_index];
            
			if (next_char == '\0')
               break;

            ++attribute_index;
            
            if (next_char == quote_char)
               break;

            *token_size += 1;
         }
         
         if (attribute_ptr[attribute_index] == ' ')
            ++attribute_index;
      }
      else {
      
         for (;;) {
            
            next_char = attribute_ptr[attribute_index];
            ++attribute_index;
            
            if (next_char == ' ')
               break;

			if (next_char == '\0')
				break;
            
            *token_size += 1;
         }
      }
   }
   
   return   token;   
}


void PaigeHTMLImportFilter::CalculateLeftIndent(long nReduceBy)
{
   current_par.indents.left_indent = (deflist_level + list_level + blockquote_level - nReduceBy) * list_indent_value;
}


/* DoControlTag handles all the tags in the control tag table. */

pg_boolean PaigeHTMLImportFilter::DoControlTag (short tag, tag_attribute_ptr params, long num_params)
{
   tag_attribute_ptr       param_ptr;
   color_value             link_color;
   long                	   value, param_index, border_info;

   switch (tag) {
   
	  //QUALCOMM Begin == Kusuma
	  case x_tab_command:
		  OutputCharacter('\t');
		  pg_char next_char;

		  //Suck up all the nbsp's within the x-tab tags if there were no
		  //previous errors. This is how code must be written....
		  //comments interspersed with code :-)
		  while (pgGetImportChar(&next_char) == NO_ERROR)
		  {
			if (next_char == ENDTAG_COMMAND_CHAR)
				break;
		  }
		  break;
	  case -x_tab_command:
		  break;
	  //QUALCOMM End == Kusuma
      case -title_command:
		 // Don't read in title, because we don't use the information and for certain
		 // invalid messages the title tags caused Paige to fail a memory allocation
		 // which caused Eudora to quit. We also don't want the title to show up
		 // so skip outputting when inside a title tag.
		 //if (title_ptr) {
		 //	
		 //	AppendMemory(original_pg_rec->doc_info.title, 1, TRUE);
		 //	UnuseMemory(original_pg_rec->doc_info.title);
		 //	title_ptr = NULL;
		 //}
		 inside_title = FALSE;
         
         break;

      case -parbreak_command:
      case -div_command:
         if (doing_table)
            break;

         current_par.justification = justify_left;
         
         if (doing_table)
		 {
            BreakLine(TRUE, FALSE);
         }
         else
         {
            if (embed_last_action)
            {
               // Pasting HTML from MS Word that looks like this: "<p>some text <img></p>"
               // ended up applying the embed style to the 0x0D that ApplyInputParagraph
               // inserts if there isn't already a line break. (To reproduce, type
               // a few characters in Word, insert an image from a file, then copy).
               // We avoid this by checking to see if we just inserted an image and output
               // the line break (if there isn't already one - hence the first FALSE)
               // before calling ApplyInputParagraph (which won't output another line
               // break because it sees the line break that we did).
               BreakLine(FALSE, FALSE);
               embed_last_action = false;
            }

            // Dump the pending text before we call ApplyInputParagraph so that it sees
            // the line break we may have just outputted (and anything else done earlier).
            DumpPendingText();
            ApplyInputParagraph(&current_par, -1, FALSE);
		 }
         break;

      case -code_listing_command:
         if (!code_listing)
            break;

         PopOutputState();
         cr_enable = code_listing = FALSE;

         BreakLine(TRUE, FALSE);
         DumpPendingText();
//QUALCOMM Begin - Kusuma (mimic v3.14)
         //GetStylesheet(html_styles[body_stylesheet]);
		 PopStyle();
//QUALCOMM End - Kusuma (mimic v3.14)
         current_style.point = point_levels[base_font_index - 1];
         break;
         
      case -ordered_list_command:
      case -unordered_list_command:
      case -directory_command:
      case -menu_command:
         DumpPendingText();
		 
         list_level -= 1;
         
         if (list_level == 0)
         	current_par.html_style = current_par.html_numbers = 0;
         
         CalculateLeftIndent();
         current_par.html_bullet = list_level;
 
         if (!doing_table)
            ApplyInputParagraph(&current_par, -1, FALSE);

         // We now handle gaps between list and non-list areas on display rather than
         // with large (i.e. double) breaks. Always pass FALSE for second parameter.
         BreakLine(FALSE, FALSE);

         break;

      case -deflist_command:
		 DumpPendingText();

         deflist_level -= 1;
         
         if (deflist_level == 0) {
            current_par.html_style = current_par.html_numbers = 0;
            CalculateLeftIndent();
         }
         
         if (!doing_table)
            ApplyInputParagraph(&current_par, -1, FALSE);

         // We now handle gaps between indented and non-indented areas on display rather than
         // with large (i.e. double) breaks. Always pass FALSE for second parameter.
         BreakLine(FALSE, FALSE);

         break;

      case base_url_command:
         break;

      case basefont_command:
         if ((param_ptr = find_parameter(params, num_params, size_attribute)) != NULL) {
            
            if (param_ptr->param[0] == '-')
               value = base_font_index - numeric_value(&param_ptr->param[1]);
            else
            if (param_ptr->param[0] == '+')
               value = base_font_index + numeric_value(&param_ptr->param[1]);
            else
               value = numeric_value(param_ptr->param);

            if (value == 0)
               value = 1;
            else
            if (value > 7)
               value = 7;
            
            base_font_index = current_point_level = value;
            current_style.point = point_levels[base_font_index - 1];
         }

         break;

      case body_command:
         for (param_index = 0; param_index < num_params; ++param_index) {
            
            param_ptr = &params[param_index];
            
            switch (param_ptr->code) {
               
               case bk_image_attribute:
               {
               	   long offset = ImportedOffset();
               	   
               	   InsertErrorLog(IMAGE_BACKGROUND_ERROR, offset, offset + 1, NULL, FALSE);
               	   break;
               }

               case bk_color_attribute:
                  if (import_bits & IMPORT_BKCOLOR_FLAG) {
                  
                     TranslateColor(param_ptr->param, &import_pg_rec->bk_color);
                     
                     if (original_pg) {
                        paige_rec_ptr     pg_rec;
                        
                        pg_rec = (paige_rec_ptr)UseMemory(original_pg);
                        pg_rec->bk_color = import_pg_rec->bk_color;
                        UnuseMemory(original_pg);
                     }
                  }
                  break;
               
               case current_link_color_attribute:
                  TranslateColor(param_ptr->param, &link_color);
                  translator.hyperlink.state2_style = pgNewHyperlinkStyle(import_pg, link_color.red, link_color.green, link_color.blue, X_UNDERLINE_BIT, FALSE);
                  break;
                  
               case link_color_attribute:
                  TranslateColor(param_ptr->param, &link_color);
                  translator.hyperlink.state1_style = pgNewHyperlinkStyle(import_pg, link_color.red, link_color.green, link_color.blue, X_UNDERLINE_BIT, FALSE);
                  break;
                  
               case text_color_attribute:
                  TranslateColor(param_ptr->param, &def_text_color);
                  current_style.fg_color = cur_text_color = def_text_color;
                  break;
               
               case seen_link_color_attribute:
                  TranslateColor(param_ptr->param, &link_color);
                  translator.hyperlink.state3_style = pgNewHyperlinkStyle(import_pg, link_color.red, link_color.green, link_color.blue, X_UNDERLINE_BIT, FALSE);
                  break;
            }
         }
         
         break;

      case linebreak_command:
         BreakLine(TRUE, FALSE);
         return   TRUE;

      case deflist_command:
         BreakLine(FALSE, FALSE);
         DumpPendingText();

         deflist_level += 1;
         current_par.html_style = html_definition_list;
         
         if (!doing_table)
            ApplyInputParagraph(&current_par, -1, FALSE);

         break;

      case heading_command:
         break;

      case linerule_command:
         DumpPendingText();

         if (find_parameter(params, num_params, noshade_attribute))
            border_info = 0;
         else
            border_info = 0x80000000;
         
         if ((param_ptr = find_parameter(params, num_params, size_attribute)) == NULL)
            border_info |= 0x02000000;
         else {
            
            value = numeric_value(param_ptr->param);
            
            if (value == 0)
               value = 2;
            else
            if (value > 7)
               value = 7;
            
            border_info |= (value << 24);
         }
         
         current_par.table.border_info = border_info;
         
         if (!doing_table) {
         
            ApplyInputParagraph(&current_par, -1, FALSE);
            current_par.table.border_info = 0;
            ApplyInputParagraph(&current_par, -1, TRUE);
         }

         break;
      
      case listing_command:
         if (list_level) {
            
            DumpPendingText();
            CalculateLeftIndent();
            current_par.class_info |= BULLETED_LINE;
            current_par.html_bullet = list_level;

            if (!doing_table)
               ApplyInputParagraph(&current_par, -1, FALSE);
            else
               BreakLine(FALSE, FALSE);
         }
         break;
      
      case code_listing_command:
         cr_enable = code_listing = TRUE;
         
         if (!doing_table) {
            
            DumpPendingText();
            ApplyInputParagraph(&current_par, -1, FALSE);
         }
         else
            BreakLine(TRUE, FALSE);

//QUALCOMM Begin - Kusuma (mimic v3.14)
		 PushStyle();
         GetStylesheet(html_styles[code_stylesheet], TRUE);
//QUALCOMM End - Kusuma (mimic v3.14)

         break;


      case parbreak_command:
         // Look for class attribute for style sheet name
         param_ptr = find_parameter(params, num_params, class_attribute);

         if (param_ptr)
         {
            PaigeHTMLStyleSheetDef *   pStyleSheetDef = NULL;
            POSITION                   pos = style_sheet_defs.GetHeadPosition();
            bool                       bSkipBreakLine = false;

            // Look for corresponding style sheet definition that applies to par breaks
            // and tells us not to use line breaks.
            while (pos)
            {
               pStyleSheetDef = reinterpret_cast<PaigeHTMLStyleSheetDef *>( style_sheet_defs.GetNext(pos) );

               if ( pStyleSheetDef && pStyleSheetDef->IsForParBreaks() && pStyleSheetDef->ShouldSkipBreakLine() )
               {
                  // Create a string from param and length, so that we have a NULL terminated string for comparison
                  CString     sName(reinterpret_cast<LPCTSTR >(param_ptr->param), param_ptr->param_size);

                  if ( pStyleSheetDef->IsForStyleSheetNamed(sName) )
				  {
					  bSkipBreakLine = true;
					  break;
				  }
               }
            }

            // Output line breaks unless a style sheet matched the par break class and overrode this behavior
            if (!bSkipBreakLine)
               BreakLine(FALSE, TRUE);
         }
         else
         {
            // No class attribute, so no style sheet overrides the normal par break behavior - output line breaks
            BreakLine(FALSE, TRUE);
         }
		 // fall through...

      case div_command:
         if ((param_ptr = find_parameter(params, num_params, data_align_attribute)) != NULL) {
            
            DumpPendingText();
            current_par.justification = (short)TranslateAlignment(param_ptr->param);
            
            if (!doing_table)
               ApplyInputParagraph(&current_par, -1, FALSE);
         }

         break;

      case title_command:
		 // Don't read in title, because we don't use the information and for certain
		 // invalid messages the title tags caused Paige to fail a memory allocation
		 // which caused Eudora to quit. We also don't want the title to show up
		 // so skip outputting when inside a title tag.
      	 //if (!original_pg_rec->doc_info.title)
         //{
         //   original_pg_rec->doc_info.title = MemoryAllocID(import_pg_rec->globals->mem_globals,
      	 //					sizeof(pg_char), 0, 64, original_pg_rec->mem_id);
   
         //   title_ptr = (pg_char_ptr)UseMemory(original_pg_rec->doc_info.title);
         //}
		 inside_title = TRUE;
         break;

      case unordered_list_command:
      case directory_command:
      case menu_command:
      case ordered_list_command:
         // We now handle gaps between list and non-list areas on display rather than
         // with large (i.e. double) breaks. Always pass FALSE for second parameter.
         if (list_level == 0)
            BreakLine(FALSE, FALSE);
 
         DumpPendingText();

         list_level += 1;
         CalculateLeftIndent();
 
         if (tag == unordered_list_command)
            current_par.html_style = html_unordered_list;
         else
         if (tag == directory_command)
            current_par.html_style = html_directory;
         else
         if (tag == menu_command)
            current_par.html_style = html_menu;
         else
            current_par.html_style = html_ordered_list;
         
         if (!doing_table) {
         	
            ApplyInputParagraph(&current_par, -1, FALSE);
         }
         
         break;

      case wordbreak_command:
         break;
   }
   
   return   FALSE;
}


/* DoStyleTag handles all the style type tags. */

pg_boolean PaigeHTMLImportFilter::DoStyleTag (short tag, tag_attribute_ptr params, long num_params)
{
   pg_boolean        result = FALSE;
   tag_attribute_ptr param_ptr;
   long          	 param_index, old_point;
   long		 		 some_text_imported;
   
   some_text_imported = bytes_imported + translator.bytes_transferred;

   switch (tag) {
      
      case -bold_command:
      case -strong_command:
         current_style.styles[bold_var] = 0;
		 //QUALCOMM Begin
		 //saved_style.styles[bold_var] = 0;
		 //QUALCOMM End
         break;
         
      case -italic_command:
      case -emphasis_style_command:
         current_style.styles[italic_var] = 0;
		 //QUALCOMM Begin
		 //saved_style.styles[italic_var] = 0;
		 //QUALCOMM End
         break;
      
      case -font_command:
      	 if (!font_tag_enable)
      	 	break;

//QUALCOMM Begin - Kusuma (mimic v3.14)
         //current_point_level = saved_point_index;
         //current_style = saved_style;
         //current_font = saved_font;
         //cur_text_color = def_text_color;
         //current_style.point = point_levels[saved_point_index - 1];
		 font_tag_enable -= 1;
		 PopStyle(true);
//QUALCOMM End - Kusuma (mimic v3.14)
         return   TRUE;

      case -asis_command:
         DumpPendingText();
         BreakLine(TRUE, FALSE);
         cr_enable = FALSE;
         GetStylesheet(html_styles[body_stylesheet]);
         current_style.point = point_levels[base_font_index - 1];
		 PopStyle();
         translator.format = current_style;
         translator.format_changed = TRUE;
         DumpPendingText();

         break;

      case -nobreak_command:
         current_style.class_bits &= (~CANNOT_BREAK);
         break;

      case -subscript_command:
      case -superscript_command:
//QUALCOMM Begin - Kusuma (mimic v3.14)
         //current_style.styles[subscript_var] = current_style.styles[superscript_var] = 0;
		  PopStyle();
//QUALCOMM End - Kusuma (mimic v3.14)
         break;

      case -strikeout_command:
      case -strikeout2_command:
         current_style.styles[strikeout_var] = 0;
 		 //QUALCOMM Begin
		 //saved_style.styles[strikeout_var] = 0;
		 //QUALCOMM End
         break;

      case -blink_command:
         current_style.styles[blink_var] = 0;
 		 //QUALCOMM Begin
		 //saved_style.styles[blink_var] = 0;
		 //QUALCOMM End
         break;

      case -underline_command:
         current_style.styles[underline_var] = 0;
 		 //QUALCOMM Begin
		 //saved_style.styles[underline_var] = 0;
		 //QUALCOMM End

         break;

      case -blockquote_style_command:
      case -code_def_style_command:
      case -heading1_command:
      case -heading2_command:
      case -heading3_command:
      case -heading4_command:
      case -heading5_command:
      case -heading6_command:
      case -teletype_style_command:
         DumpPendingText();
//QUALCOMM Begin - Kusuma (mimic v3.14)
         //GetStylesheet(html_styles[body_stylesheet]);
         //current_style.point = point_levels[base_font_index - 1];
		 PopStyle();
//QUALCOMM End - Kusuma (mimic v3.14)
         current_par.justification = 0;
         translator.format = current_style;
         
		 //QUALCOMM Begin - Kusuma
		 if (tag != -teletype_style_command)
		 //QUALCOMM End - Kusuma
         BreakLine(FALSE, TRUE);
         DumpPendingText();

         if (tag == -blockquote_style_command)
         {
            blockquote_level--;

			// Probably not necessary, but let's be sure
            if (blockquote_level < 0)
               blockquote_level = 0;

            CalculateLeftIndent();
         }
		          
		 //QUALCOMM Begin - Kusuma
		 //Added check for closing tt tag
         if (!doing_table && tag != -teletype_style_command)
            ApplyInputParagraph(&current_par, -1, FALSE);

         break;

      case -code_var_style_command:
      case -address_style_command:
      case -cite_style_command:
      case -code_style_command:
         DumpPendingText();
//QUALCOMM Begin - Kusuma (mimic v3.14)
         //GetStylesheet(html_styles[body_stylesheet]);
         //current_style.point = point_levels[base_font_index - 1];
		 PopStyle();
//QUALCOMM End - Kusuma (mimic v3.14)
         translator.format = current_style;
         break;

      case -center_command:
         if (linebreak_char != 0x0D)
            break;
            
         DumpPendingText();
         current_par.justification = translator.par_format.justification = justify_left;
         
         if (!doing_table)
            ApplyInputParagraph(&current_par, -1, FALSE);
         break;
         
      case address_style_command:
         DumpPendingText();
//QUALCOMM Begin - Kusuma (mimic v3.14)
		 PushStyle();
         //GetStylesheet(html_styles[address_stylesheet]);
         GetStylesheet(html_styles[address_stylesheet], FALSE);
//QUALCOMM End - Kusuma (mimic v3.14)
         translator.format = current_style;
         break;

      case bold_command:
      case strong_command:
         current_style.styles[bold_var] = -1;
 		 //QUALCOMM Begin
		 //saved_style.styles[bold_var] = -1;
		 //QUALCOMM End

         break;

      case larger_size_command:
      case -smaller_size_command:
         if (current_point_level < 7) {
            
            ++current_point_level;
            current_style.point = point_levels[current_point_level - 1];
         }
         break;

      case blink_command:
         current_style.styles[blink_var] = -1;
 		 //QUALCOMM Begin
		 //saved_style.styles[blink_var] = -1;
		 //QUALCOMM End
         break;
         
      case blockquote_style_command:
          //QUALCOMM Begin == Kusuma
		  translator.format = current_style;
		  //QUALCOMM End == Kusuma

         GetStylesheet(html_styles[blockquote_stylesheet]);
		 
		 //QUALCOMM Begin == Kusuma
		 current_style.point = translator.format.point;
		 current_style.fg_color = translator.format.fg_color;
		 current_style.bk_color = translator.format.bk_color;
		 current_style.font_index = translator.format.font_index;
		 pgBlockMove(translator.format.styles, current_style.styles, MAX_STYLES * sizeof(short));
		 translator.format = current_style;
		 //QUALCOMM End == Kusuma

         DumpPendingText();

         blockquote_level++;
         CalculateLeftIndent();

         if (!doing_table)
            ApplyInputParagraph(&current_par, -1, FALSE);
         break;
      
      case center_command:
         if (linebreak_char != 0x0D)
            break;
            
         DumpPendingText();
         current_par.justification = justify_center;
         
         if (!doing_table)
            ApplyInputParagraph(&current_par, -1, FALSE);
         break;

      case cite_style_command:
         DumpPendingText();
//QUALCOMM Begin - Kusuma (mimic v3.14)
		 PopStyle();
         //old_point = current_style.point;
         //GetStylesheet(html_styles[cite_stylesheet]);
         //current_style.point = old_point;
//QUALCOMM Begin - Kusuma (mimic v3.14)
         translator.format = current_style;
         break;

      case code_style_command:
         DumpPendingText();
//QUALCOMM Begin - Kusuma (mimic v3.14)
		 PushStyle();
         //GetStylesheet(html_styles[code_stylesheet]);
         GetStylesheet(html_styles[code_stylesheet], FALSE);
//QUALCOMM End Kusuma (mimic v3.14)
         translator.format = current_style;
         break;

      case deflist_indented_command:
         DumpPendingText();
		 CalculateLeftIndent();
         
         if (!doing_table)
            ApplyInputParagraph(&current_par, -1, FALSE);
         else
            BreakLine(FALSE, FALSE);
         
         break;

      case code_def_style_command:
//QUALCOMM Begin - Kusuma (mimic v3.14)
		  DumpPendingText();
		  PushStyle();
		  //GetStylesheet(html_styles[code_def_stylesheet]);
		  GetStylesheet(html_styles[code_def_stylesheet], FALSE);
//QUALCOMM End - Kusuma (mimic v3.14)
         break;

      case deflist_item_command:
         DumpPendingText();
		 CalculateLeftIndent(1);

         current_par.html_style &= 0x0000FFFF;

         if (!doing_table)
            ApplyInputParagraph(&current_par, -1, FALSE);
         else
            BreakLine(FALSE, FALSE);

         break;

      case font_command:
//QUALCOMM Begin - Kusuma (mimic v3.14)
		 //saved_style = current_style;
         //saved_font = current_font;
         //saved_point_index = current_point_level;
         //font_tag_enable = TRUE;
		 PushStyle();
		 font_tag_enable += 1;
//QUALCOMM End - Kusuma (mimic v3.14)

         for (param_index = 0, param_ptr = params; param_index < num_params; ++param_ptr, ++param_index) {
            
            switch (param_ptr->code) {

               case fontcolor_attribute:
                  TranslateColor(param_ptr->param, &cur_text_color);
				  //if (font_tag_enable)
					//saved_style.fg_color = current_style.fg_color;
//QUALCOMM Begin - Kusuma (mimic v3.14)
                  current_style.fg_color = cur_text_color;
//QUALCOMM End - Kusuma (mimic v3.14)
                  break;

               case fontface_attribute:
					 //saved_style = current_style;
					 //saved_font = current_font;
					 //saved_point_index = current_point_level;
                  TranslateFont(param_ptr->param);
                  break;

               case fontsize_attribute:

				  //if (font_tag_enable) 
					  //saved_style.point = current_style.point;

                  if (param_ptr->param[0] == '-')
                     current_point_level = base_font_index - numeric_value(&param_ptr->param[1]);
                  else
                  if (param_ptr->param[0] == '+')
                     current_point_level = base_font_index + numeric_value(&param_ptr->param[1]);
                  else
                     current_point_level = numeric_value(param_ptr->param);

                  if (current_point_level == 0)
                     current_point_level = 1;
                  else
                  if (static_cast<unsigned long>(current_point_level) > 7)
                     current_point_level = 7;

                  current_style.point = point_levels[current_point_level - 1];
                  break;
            }
         }

		 //font_tag_enable = TRUE;

         break;

      case heading1_command:
      	 if (some_text_imported)
         	BreakLine(TRUE, FALSE);

         DumpPendingText();
//QUALCOMM Begin - Kusuma (mimic v3.14)
		 PushStyle();
//QUALCOMM End - Kusuma (mimic v3.14)
         GetStylesheet(html_styles[heading1_stylesheet]);
         break;

      case heading2_command:
      	 if (some_text_imported)
         	BreakLine(TRUE, FALSE);

         DumpPendingText();
//QUALCOMM Begin - Kusuma (mimic v3.14)
		 PushStyle();
//QUALCOMM End - Kusuma (mimic v3.14)
         GetStylesheet(html_styles[heading2_stylesheet]);
         break;

      case heading3_command:
      	 if (some_text_imported)
         	BreakLine(TRUE, FALSE);
         	
         DumpPendingText();
//QUALCOMM Begin - Kusuma (mimic v3.14)
		 PushStyle();
//QUALCOMM End - Kusuma (mimic v3.14)
         GetStylesheet(html_styles[heading3_stylesheet]);
         break;

      case heading4_command:
      	 if (some_text_imported)
         	BreakLine(TRUE, FALSE);
         	
         DumpPendingText();
//QUALCOMM Begin - Kusuma (mimic v3.14)
		 PushStyle();
//QUALCOMM End - Kusuma (mimic v3.14)
         GetStylesheet(html_styles[heading4_stylesheet]);
         break;

      case heading5_command:
      	 if (some_text_imported)
         	BreakLine(TRUE, FALSE);

         DumpPendingText();
//QUALCOMM Begin - Kusuma (mimic v3.14)
		 PushStyle();
//QUALCOMM End - Kusuma (mimic v3.14)
         GetStylesheet(html_styles[heading5_stylesheet]);
         break;

      case heading6_command:
      	 if (some_text_imported)
         	BreakLine(TRUE, FALSE);
         	
         DumpPendingText();
//QUALCOMM Begin - Kusuma (mimic v3.14)
		 PushStyle();
//QUALCOMM End - Kusuma (mimic v3.14)
         GetStylesheet(html_styles[heading6_stylesheet]);
         break;

      case italic_command:
      case emphasis_style_command:
         current_style.styles[italic_var] = -1;
 		 //QUALCOMM Begin
		 //saved_style.styles[italic_var] = -1;
		 //QUALCOMM End
         break;

      case code_var_style_command:
         old_point = current_style.point;
//QUALCOMM Begin - Kusuma (mimic v3.14)
		 PushStyle();
         //GetStylesheet(html_styles[code_var_stylesheet]);
         GetStylesheet(html_styles[code_var_stylesheet], FALSE);
//QUALCOMM End - Kusuma (mimic v3.14)
         current_style.point = old_point;
         break;
         
      case asis_command:
         cr_enable = TRUE;
         BreakLine(TRUE, FALSE);
         DumpPendingText();
		 GetStylesheet(html_styles[pre_tag_stylesheet]);
         return   TRUE;

      case nobreak_command:
         current_style.class_bits |= CANNOT_BREAK;
         break;

      case strikeout_command:
      case strikeout2_command:
         current_style.styles[strikeout_var] = -1;
 		 //QUALCOMM Begin
		 //saved_style.styles[strikeout_var] = -1;
		 //QUALCOMM End
         break;
         
      case smaller_size_command:
      case -larger_size_command:
         if (current_point_level > 1) {
            
            --current_point_level;
            current_style.point = point_levels[current_point_level - 1];
         }
         break;

      case subscript_command:
//QUALCOMM Begin - Kusuma (mimic v3.14)
		 PushStyle();
         current_style.styles[subscript_var] = 50;
         current_style.styles[superscript_var] = 0;
         current_style.shift_verb = percent_of_style;
         //current_style.styles[subscript_var] = 3;
         //current_style.styles[superscript_var] = 0;
//QUALCOMM End - Kusuma (mimic v3.14)
         break;

      case superscript_command:
//QUALCOMM Begin - Kusuma (mimic v3.14)
      	 PushStyle();
         current_style.styles[superscript_var] = 50;
         current_style.styles[subscript_var] = 0;
         current_style.shift_verb = percent_of_style;
         //current_style.styles[superscript_var] = 3;
         //current_style.styles[subscript_var] = 0;
//QUALCOMM End - Kusuma (mimic v3.14)
         break;

      case teletype_style_command:
//QUALCOMM Begin - Kusuma (mimic v3.14)
		 PushStyle();
         GetStylesheet(html_styles[teletype_stylesheet], TRUE);
//         GetStylesheet(html_styles[teletype_stylesheet], FALSE);
//QUALCOMM End - Kusuma (mimic v3.14)
         break;
         
      case underline_command:
         current_style.styles[underline_var] = -1;
 		 //QUALCOMM Begin
		 //saved_style.styles[underline_var] = -1;
		 //QUALCOMM End
         break;
   }
   
   if (!doing_table)
      if ((param_ptr = find_parameter(params, num_params, style_align_attribute)) != NULL) {
         
         DumpPendingText();
         current_par.justification = (short)TranslateAlignment(param_ptr->param);
         ApplyInputParagraph(&current_par, -1, FALSE);
      }

   translator.format_changed |= !pgEqualStruct(&translator.format, &current_style, SIGNIFICANT_STYLE_SIZE);
   translator.font_changed |= !pgEqualStruct(&translator.font, &current_font, SIGNIFICANT_FONT_SIZE);

   if (!translator.bytes_transferred) {
   
      translator.format = current_style;
      translator.font = current_font;
   }
   else
      result |= (translator.format_changed | translator.font_changed);

   return   result;
}


/* PushStyle saves the current style on an internal "stack" */

void PaigeHTMLImportFilter::PushStyle (void)
{
	format_stack_ptr		stack;
	
	stack = (format_stack_ptr)AppendMemory(style_stack_ref, 1, FALSE);
	stack->style = current_style;
	stack->style.maintenance = current_point_level;
	stack->font = current_font;
	UnuseMemory(style_stack_ref);
}

/* PopStyle restores the last PushStyle() values. */

//QUALCOMM Begin - Beckley (fix bug with </font> restoring too much)
//void PaigeHTMLImportFilter::PopStyle ()
void PaigeHTMLImportFilter::PopStyle (pg_boolean bFontTag /*= false*/)
{
	format_stack_ptr		stack;
	long					stack_size = GetMemorySize(style_stack_ref);
	
	if (stack_size > 0) {
		
		--stack_size;
		stack = (format_stack_ptr)UseMemoryRecord(style_stack_ref, stack_size, 0, TRUE);
		if (!bFontTag)
			current_style = stack->style;
		else
		{
			short SavedStyles[MAX_STYLES];
			memcpy(SavedStyles, current_style.styles, sizeof(SavedStyles));
			current_style = stack->style;
			memcpy(current_style.styles, SavedStyles, sizeof(SavedStyles));
		}
		current_font = stack->font;
		current_point_level = current_style.maintenance;
		current_style.maintenance = 0;
		UnuseMemory(style_stack_ref);
		
		SetMemorySize(style_stack_ref, stack_size);		
	}
}
//QUALCOMM End - Beckley (fix bug with </font> restoring too much)


/* DoDataTag handles all the data tags (like link and image). */
pg_boolean PaigeHTMLImportFilter::DoDataTag (short tag, tag_attribute_ptr params, long num_params)
{
   tag_attribute_ptr       param_ptr;
   
   switch (tag) {

      case -hyperlink_command:

         if (anchor_ref) {
         
            ProcessHyperlink();
            DisposeMemory(anchor_ref);
            anchor_ref = MEM_NULL;

            return   TRUE;
         }

         break;

      case hyperlink_command:
         if (!(import_bits & IMPORT_HYPERTEXT_FLAG))
            break;

         if (anchor_ref)
         	 DisposeMemory(anchor_ref);

         anchor_ref = MemoryDuplicate(param_ref);
         link_range.begin = link_range.end = bytes_imported + translator.bytes_transferred;
         break;

	  case param_command:
		 {
#if 0
		 // the "param" tag must immediately follow its containing "object" tag,
		 // so if we don't have an embed in "current_style," something's wrong.
		 memory_ref embed;

		 if ( embed = current_style.embed_object ) {
			 pg_embed_ptr pe = (pg_embed_ptr) UseMemory( embed );
			 HtmlObjectSpec* pSpec = (HtmlObjectSpec*) pe->user_data;

			 if ( pSpec ) {
				tag_attribute_ptr prms = params;

				// parse out all the attributes
				for ( int i = 0; i < num_params; i++, prms++ )
					pSpec->SetAttribute( prms->code, (char*)prms->param );
			 }
		 }
#endif
		 }
	     break;

	  case -object_command:
		  if ( !translator.bytes_transferred ) {
				ProcessEmbed();
				return TRUE;
		  }
		 break;

	  case object_command:
			{
		    // all we do here is allocate and fill out an "ObjectSpec." We don't
			// bind to the ActiveX control until paige tells us to draw.
			HtmlObjectSpec* pSpec = DEBUG_NEW_NOTHROW HtmlObjectSpec;

			if ( pSpec ) {
				tag_attribute_ptr prms = params;

				// parse out all the attributes
				for ( int i = 0; i < num_params; i++, prms++ )
					pSpec->SetAttribute( prms->code, (char*)prms->param );

				// if there width & height aren't specified, we'll have to go
				// ahead and bind now to get the object's size.
				PgCntrItem* pItem = 0;
				if ( !pSpec->width || !pSpec->height )
					pItem = PgBindToObject( import_pg_rec, pSpec );

				// create our associated embed ref
				memory_ref embed = (long) pgNewEmbedRef(
								import_pg_rec->globals->mem_globals,
								embed_user_box,
								0,
								0,
								ALIGN_CBASELINE_FLAG,
								0,
								0,
								FALSE );

				if ( embed ) {
					// init the embed and prepare to import -- the embed's type
					// is "user_box," which is just a blank rectangle
					pg_embed_ptr pe = (pg_embed_ptr) UseMemory( embed );
					pe->width = pSpec->width;
					pe->height = pSpec->height;
					pe->user_data = (long) pSpec;
					pe->user_refcon = (long) pItem;
					UnuseMemory( embed );

					pgInitEmbedStyleInfo(
								import_pg_rec,
								0,
								embed,
								0,
								ActiveXCallbackProc,
								0,
								&current_style,
								NULL,
								NULL,
								NULL,
								FALSE );

					// BOG: need to perform ascent/descent mangling here; right
					// now, we're just baseline-aligned at zero.

//					if ( !translator.bytes_transferred )
//						ProcessEmbed();
				}
				else {
					delete pSpec;
					
					if ( pItem )
						pItem->Delete();
				}

				return TRUE;
			}
			}
	     break;

      case image_command:

         if ((param_ptr = find_parameter(params, num_params, image_url_attribute)) != NULL) {
            pg_url_image      image_record;
            pg_char           szImageURL[512];
            memory_ref        embed, image_ref;
            pg_frame		  frame;
            long              param_size, image_descent, original_descent, original_ascent;
			long			  current_offset = ImportedOffset();
			long			  embed_flags = 0;

            pgFillBlock(&image_record, sizeof(pg_url_image), 0);
            pgFillBlock(&frame, sizeof(pg_frame), 0);

            resolve_URL( param_ptr->param, szImageURL, sizeof(szImageURL) );

            // Assume that we have a full URL
            bool              bFullURL = true;

            if ( *base_url_string && !IsURLScheme(reinterpret_cast<char *>(param_ptr->param)) )
            {
               // It's not a recognized URL beginning and we have the ability to translate a
               // relative URL to a full URL.
               bFullURL = false;
               
               char        cURLStart = *param_ptr->param;

               if ( ((cURLStart >= 'a') && (cURLStart <= 'z')) ||
                    ((cURLStart >= 'A') && (cURLStart <= 'Z')) )
               {
                  if ( (param_ptr->param[1] == ':') &&
                       ((param_ptr->param[2] == '\\') || (param_ptr->param[2] == '/')) )
                  {
                     // For whatever reason there's no file://, but it starts like
                     // a Windows full path so it's a full URL.
                     bFullURL = true;
                  }
               }
            }
            
            if ( bFullURL ||
                 ( (strlen(reinterpret_cast<char *>(base_url_string)) +
                    strlen(reinterpret_cast<char *>(szImageURL))) > sizeof(image_record.URL)) )
            {
               // Full URL or no room available to combine URL with base
               strncpy( reinterpret_cast<char *>(image_record.URL), reinterpret_cast<char *>(szImageURL), sizeof(image_record.URL) );
            }
			else
            {
               // Relative URL with base URL and room available to combine the two
               strcpy( reinterpret_cast<char *>(image_record.URL), reinterpret_cast<char *>(base_url_string) );

               // I'm not sure that this would ever happen, but check for slash at start of relative path
               // and skip over it so that we don't get a full path with a double slash.
               const char *		pStartRelativePath = reinterpret_cast<char *>(szImageURL);
               if ( (*pStartRelativePath == '\\') || (*pStartRelativePath == '/') )
                  pStartRelativePath++;

               strcat(reinterpret_cast<char *>(image_record.URL), pStartRelativePath);
            }

            // Terminate the string for edge cases
            image_record.URL[sizeof(image_record.URL)-1] = '\0';


            image_record.source_type = (short)image_type(image_record.URL);

            if ((param_ptr = find_parameter(params, num_params, image_width_attribute)) != NULL)
               image_record.source_width = (short)decimal_value_percent(param_ptr->param, page_width);
            if ((param_ptr = find_parameter(params, num_params, image_height_attribute)) != NULL)
               image_record.source_height = (short)numeric_value(param_ptr->param);

            if ((param_ptr = find_parameter(params, num_params, alt_image_attribute)) != NULL) {
               
               if ((param_size = param_ptr->param_size) > 63)
                  param_size = 63;
               
               pgBlockMove(param_ptr->param, image_record.alt_string, param_size);
            }

         	if (find_parameter(params, num_params, ismap_attribute))
				InsertErrorLog(IMAGE_MAP_ERROR, current_offset, current_offset + 2, NULL, FALSE);

            image_ref = pgNewImageRecord(import_pg, &image_record, image_callback, -1);
            GetMemoryRecord(image_ref, 0, &image_record);

            original_ascent = current_style.ascent;
            image_descent = original_descent = (long)current_style.descent;
            image_descent <<= 16;
            
            embed = (long)pgNewEmbedRef(import_pg_rec->globals->mem_globals,
                        embed_url_image, (void PG_FAR *)image_ref, 0, ALIGN_CBASELINE_FLAG, -image_descent, 0, FALSE);
			
			if (!doing_table && (import_bits & IMPORT_XFRAMES_FLAG))
         		if ((param_ptr = find_parameter(params, num_params, data_align_attribute)) != NULL) {

				// Set up a frame just incase this is an "exclusion-wrap" image:

				translate_single_file(image_record.URL, frame.name);
				frame.bounds.bot_right.h = image_record.source_width;
				frame.bounds.bot_right.v = image_record.source_height;
				pgOffsetRect(&frame.bounds, 4, 4);
				frame.wrap = frame.bounds;
				pgInsetRect(&frame.wrap, -4, -4);
				frame.position = current_offset;
				frame.flags = FRAME_ATTACHED_TO_PAR | FRAME_NO_BACKGROUND;
				frame.shading = pgRGBToLong(&import_pg_rec->bk_color);

				frame.type = frame_embed;
				
				switch (TranslateAlignment(param_ptr->param)) {

					case justify_left:
						frame.flags |= FRAME_ALIGN_LEFT;
						frame.data = (generic_var)embed;
						break;
						
					case justify_center:
						frame.flags |= FRAME_ALIGN_CENTER;
						frame.data = (generic_var)embed;
						break;

					case justify_right:
						frame.flags |= FRAME_ALIGN_RIGHT;
						frame.data = (generic_var)embed;
						break;

					case justify_middle:
					case justify_absmiddle:
					case justify_baseline:
						embed_flags = ALIGN_CBASELINE_FLAG;
						break;
						
					case justify_absbottom:
						embed_flags = ALIGN_BBASELINE_FLAG;
						break;
				}
			}

			// Check to see if image is an emoticon image as indicated by the
			// special parameter eudora=emoticon.
			if ((param_ptr = find_parameter(params, num_params, eudora_attribute)) != NULL) {
				if ( strnicmp(reinterpret_cast<char *>(param_ptr->param), "emoticon", param_ptr->param_size) == 0 )
					embed_flags |= EMBED_EMOTICON_FLAG;
			}

			if (frame.data)
				InsertFrame(&frame);
			else {
				pg_embed_ptr		embed_ptr;
				
				embed_ptr = (pg_embed_ptr)UseMemory(embed);
				embed_ptr->type |= embed_flags;
				UnuseMemory(embed);

           	 	pgInitEmbedStyleInfo(import_pg_rec, 0, embed, 0, image_callback, 0, &current_style, NULL, NULL, NULL, FALSE);

	            if (image_record.source_height) {
	               
	               current_style.ascent = image_record.source_height;
	               current_style.ascent = (short)original_descent;
	               
	               if (original_ascent > current_style.ascent)
	                  current_style.ascent = (short)original_ascent;
	            }
	            
	            if (!translator.bytes_transferred)
	               ProcessEmbed();
	        }
         }

         return      TRUE;
   }

   return   FALSE;
}


/* DoTableTag handles all the table tags. This gets called only on the SECOND PASS. */

pg_boolean PaigeHTMLImportFilter::DoTableTag (short tag, tag_attribute_ptr params, long num_params)
{
   select_pair_ptr         table_offset;
   tag_attribute_ptr	   param_ptr;
   long              	   possible_span;
   pg_table                temp_table;

   switch (tag) {

      case -table_row_command:
      	 if (doing_table > 1)
      	 	break;
 
      	 doing_row = FALSE;
         break;

      case -table_begin_command:
      	 if (!doing_table)
      	 	break;
      	 
      	 if ((doing_table -= 1) > 0)
      	 	break;
 
         EndTableRow();
         doing_row = FALSE;
         linebreak_char = 0x0D;

         table_offset = (select_pair_ptr)UseMemoryRecord(tables_ref,
                                 GetMemorySize(tables_ref) - 1, 0, TRUE);
         table_offset->end = ImportedOffset();
         UnuseMemory(tables_ref);

         pgFillBlock(current_par.tabs, sizeof(tab_stop) * TAB_ARRAY_SIZE, 0);
         current_par.num_tabs = 0;
         pgFillBlock(&current_par.table, sizeof(pg_table), 0);
         current_par.table.unique_id = 0;
         ApplyInputParagraph(&current_par, -1, FALSE);
         BreakLine(TRUE, FALSE);

         break;

      case table_dcell_command:
      case table_hcell_command:
      	 if (doing_table > 1) {
      	 
      	    if (column_ctr)
      	    	OutputCharacter(0x20);
      	    	
      	 	break;
		 }
		 
         if (!doing_row)
            DoTableTag(table_row_command, NULL, 0);
		 
		 if (column_ctr == TAB_ARRAY_SIZE)
		 	break;

         pgFillBlock(&temp_table, sizeof(pg_table), 0);
         possible_span = SetTableAttributes (params, num_params, &temp_table);

         if (column_ctr)
            OutputCharacter(0x09);

         current_par.tabs[column_ctr].position = temp_table.table_column_width;
         current_par.tabs[column_ctr].tab_type = temp_table.unique_id + 1;
         current_par.tabs[column_ctr].tab_type |= (possible_span << 24);
		 current_par.tabs[column_ctr].ref_con = temp_table.border_shading;
		 
         ++column_ctr;

         if (tag == table_hcell_command)
            return   DoStyleTag(bold_command, params, num_params);

         return   DoStyleTag(-bold_command, params, num_params);

      case table_begin_command:
         if (!parallel_buffer && !temp_buffer_ref) {

            BreakLine(TRUE, FALSE);
            DumpPendingText();


         	if ((param_ptr = find_parameter(params, num_params, data_align_attribute)) != NULL) {
         		long		alignment;
         		
         		alignment = TranslateAlignment(param_ptr->param);
         		
         		if (alignment)
         			if (alignment != justify_center && alignment != justify_right) {
						long		error_offset = ImportedOffset();

						InsertErrorLog(IMAGE_WRAP_ERROR, error_offset, error_offset + 2, NULL, FALSE);
				}
			}

            table_unique_id = pgUniqueID(import_pg);
            ProcessPreTable(params, num_params);
            linebreak_char = 0x0A;
            doing_table = 1;
            doing_row = FALSE;
            table_offset = (select_pair_ptr)AppendMemory(tables_ref, 1, TRUE);
            table_offset->begin = table_offset->end = ImportedOffset();
            UnuseMemory(tables_ref);
            
            row_begin = bytes_imported + translator.bytes_transferred;
         }
         else {
         	
         	++doing_table;
         	table_offset = (select_pair_ptr)UseMemoryRecord(tables_ref, GetMemorySize(tables_ref) - 1, 0, TRUE);
         	InsertErrorLog(TABLE_IN_TABLE_ERROR, table_offset->begin, ImportedOffset(), NULL, FALSE);
         	UnuseMemory(tables_ref);
         }
         
         break;

      case table_row_command:
         if (doing_table > 1) {
         	
         	 OutputCharacter(0x0A);
         	 break;
         }
         
         if (column_ctr)
            EndTableRow();
            
         doing_row = FALSE;
         
         temp_table = master_table;
         table_def_align = left_tab;
         
         if (params)
            SetTableAttributes (params, num_params, &temp_table);
         
         if (temp_table.unique_id & 0xFF)
            table_def_align = static_cast<short>(temp_table.unique_id & 0xFF);

         if (!temp_table.border_info)
            temp_table.border_info = master_table.border_info;
         if (!temp_table.border_shading)
            temp_table.border_shading = master_table.border_shading;
         if (!temp_table.cell_h_extra)
            temp_table.cell_h_extra = master_table.cell_h_extra;
         if (!temp_table.border_spacing)
            temp_table.border_spacing = master_table.border_spacing;

         temp_table.table_column_width = 0;
         temp_table.table_columns = master_table.table_columns;
         SetDefaultTable(&temp_table);
         current_par.table.unique_id = table_unique_id;
         
         doing_row = TRUE;
         break;
   }

   return   FALSE;
}

/* ProcessPreTable causes the import to make one pass through a table to learn enough
about it to produce the columns and cells. */

void PaigeHTMLImportFilter::ProcessPreTable (tag_attribute_ptr params, long num_params)
{
   memory_ref        old_params_ref;
   pg_char           next_char;
   
   old_params_ref = param_ref;
   param_ref = ::MemoryAlloc(paige_globals->mem_globals, sizeof(tag_attribute), 0, 6);
   
   if (!tables_ref)
      tables_ref = MemoryAlloc(paige_globals->mem_globals, sizeof(select_pair), 0, 2);
   
   parallel_buffer = MemoryAlloc(paige_globals->mem_globals, sizeof(pg_char), 0, 256);
   last_physical_read = file_begin + bytes_read;
   pgFillBlock(&master_table, sizeof(pg_table), 0);
   column_ctr = 0;
   table_def_align = left_tab;

   SetTableAttributes(params, num_params, &master_table);
   master_table.unique_id = 0;
   master_table.table_column_width = 0;
   doing_table = 1;

   while (pgGetImportChar(&next_char) == NO_ERROR) {
      
      if (next_char == TAG_COMMAND_CHAR) {
         long        command_id;
         pg_boolean     end_tag;

         command_id = ReadHTMLTag(&end_tag);
         
         if (command_id) {
            tag_attribute_ptr       params;
            long                 	num_params;
            short                	table_id, tag;
            
            table_id = pgHiWord(command_id);
            tag = pgLoWord(command_id);
            
            if (end_tag)
               tag = static_cast<short>(-tag);
            
            if (table_id == TABLE_DATA_COMMAND) {
               
               if (tag == -table_begin_command) {
                  
                  if ((--doing_table) == 0)
                  break;
			   }
			   else
			   if (tag == table_begin_command)
			   	   ++doing_table;

			   if (doing_table == 1) {
			   
	               params = (tag_attribute_ptr)UseMemory(param_ref);
	               num_params = GetMemorySize(param_ref);
	               DoPreTableTag(tag, params, num_params);
	               UnuseMemory(param_ref);
	           }
            }
         }        
      }
   }
   
   if (master_table.table_columns == 0)
      master_table.table_columns = 1;
   
   ImportTemporaryBuffer(MEM_NULL);
   DisposeMemory(param_ref);
   param_ref = old_params_ref;
   
   SetDefaultTable(&master_table);
}

/* ImportedOffset returns the current position in the import paige object. */

long PaigeHTMLImportFilter::ImportedOffset (void)
{
   pg_import_ptr     import_ptr;
   long           offset;
   
   import_ptr = (pg_import_ptr)UseMemory(import_pg_rec->import_control);
   offset = import_ptr->t_length;
   UnuseMemory(import_pg_rec->import_control);
   
   return   (offset + translator.bytes_transferred);
}

/* DoPreTableTag processes all table tags for PRE-PROCESSING (pass 1). All we check for in
this case is overall params like number of columns, etc. */

pg_boolean PaigeHTMLImportFilter::DoPreTableTag (short tag, tag_attribute_ptr params, long num_params)
{
    long			possible_span;
	pg_table		temp_table;

   switch (tag) {

      case -table_row_command:
         column_ctr = 0;
         break;
         
      case table_dcell_command:
      case table_hcell_command:
         ++column_ctr;
         
         if ((possible_span = SetTableAttributes (params, num_params, &temp_table)) > 1)
         	column_ctr += (possible_span - 1);
         
         if (column_ctr > master_table.table_columns)
            master_table.table_columns = column_ctr;
         break;

      case table_row_command:
         column_ctr = 0;
         break;
   }

   return   FALSE;
}

/* SetDefaultTable sets up the current par format to reflect the default table settings. */

void PaigeHTMLImportFilter::SetDefaultTable (pg_table_ptr table)
{
   pg_short_t        index;
   long           def_width = 0;
   
   pgBlockMove(table, &current_par.table, sizeof(pg_table));
   pgFillBlock(current_par.tabs, sizeof(tab_stop) * TAB_ARRAY_SIZE, 0);
   current_par.num_tabs = (pg_short_t)table->table_columns;
   
   if (table->table_columns)
      def_width = table->table_column_width / table->table_columns;

   for (index = 0; index < current_par.num_tabs; ++index) {
      
      current_par.tabs[index].tab_type = table_def_align;
      current_par.tabs[index].position = def_width;
      current_par.tabs[index].leader = table->border_info;
      current_par.tabs[index].ref_con = table->border_shading;
   }
   
   column_ctr = 0;
}

/* DumpPendingText forces any text not yet returned to the host importer to be exported
to the import_pg_rec. */

void PaigeHTMLImportFilter::DumpPendingText (void)
{
   if (translator.bytes_transferred) {

//QUALCOMM Begin - Kusuma (mimic v3.14)
      //current_style.fg_color = cur_text_color;
//QUALCOMM End - Kusuma (mimic v3.14)
      translator.format_changed = translator.font_changed = TRUE;

      if (current_par.table.table_columns)
         current_par.table.unique_id = table_unique_id;
      else
         current_par.table.unique_id = 0;
      
      ProcessImportData();
   }
}


/* SetTableAttributes checks the parameter list and sets corresponding attributes in
the attributes parameter. Note that attributes is not necessarily the whole table -- the
caller might use a temp pg_table to set, say, cell or attributes. If there is a "colspan"
attribute that value is returned as the function result. */

long PaigeHTMLImportFilter::SetTableAttributes (tag_attribute_ptr params, long num_params, pg_table_ptr attributes)
{ 
   tag_attribute_ptr    argument;
   long              value, colspan;
   
   attributes->cell_h_extra = 2;
   colspan = 0;

   if ((argument = find_parameter(params, num_params, bk_table_attribute)) != NULL)
      attributes->border_shading = TranslateColor(argument->param, NULL);
   else
      attributes->border_shading = 0;

   if ((argument = find_parameter(params, num_params, border_attribute)) != NULL) {
      
      if (argument->param[0] == 0)
         value = 1;
      else
         value = numeric_value(argument->param);

      if (value != 0) {
         
         value += 2;
         
         if (value > 7)
         	value = 7;

         attributes->border_info = value | (value << 8) | (value << 16) | (value << 24);
         attributes->border_info |= PG_BORDER_ALL3D;
      }
      else
         attributes->border_info = 0;
   }
   else
      attributes->border_info = 0;

   if ((argument = find_parameter(params, num_params, cellspace_attribute)) != NULL)
      attributes->cell_h_extra = numeric_value(argument->param);
   else
   if ((argument = find_parameter(params, num_params, cellpad_attribute)) != NULL)
      attributes->cell_h_extra = numeric_value(argument->param);

   if ((argument = find_parameter(params, num_params, image_width_attribute)) != NULL) {
      pg_boolean     percent = FALSE;

      if ((value = (long)argument->param_size) > 0)
         if (argument->param[value - 1] == '%') {
            
            argument->param[value - 1] = 0;
            argument->param_size -= 1;
            percent = TRUE;
         }
      
      attributes->table_column_width = numeric_value(argument->param);
      
      if (percent)
         attributes->table_column_width = (page_width * attributes->table_column_width) / 100;
   }
   else
      attributes->table_column_width = 0;

   if ((argument = find_parameter(params, num_params, image_height_attribute)) != NULL)
      attributes->table_cell_height = numeric_value(argument->param);
   else
      attributes->table_cell_height = 0;

   /* "Unique ID" is used for the alignment param. */

   if ((argument = find_parameter(params, num_params, data_align_attribute)) != NULL)
      attributes->unique_id = TranslateAlignment(argument->param);
   else
      attributes->unique_id = 0;
   
   if ((argument = find_parameter(params, num_params, vertical_align_attribute)) != NULL)
      attributes->unique_id |= (TranslateAlignment(argument->param) << 16);
   
   if ((argument = find_parameter(params, num_params, column_span_attribute)) != NULL) {
   
      colspan = numeric_value(argument->param);
      
      if (colspan < 2)
         colspan = 0;
   }
   
   return   colspan;
}


/* EndTableRow terminates a table row, returning TRUE if anything happened. */

void PaigeHTMLImportFilter::EndTableRow (void)
{
   pg_short_t        index;
   long              span, num_columns;

   doing_row = FALSE;

   if (!column_ctr)
      return;
   
   
   num_columns = master_table.table_columns;

   for (index = 0; index < (pg_short_t)master_table.table_columns; ++index) {
      
      if ((span = current_par.tabs[index].tab_type >> 24) > 1)
         num_columns -= (span - 1);
   }

   while (column_ctr < num_columns) {
      
      OutputCharacter(0x09);
      column_ctr += 1;
   }
   
   OutputCharacter(0x0D);
   DumpPendingText();
   
   ApplyInputParagraph(&current_par, row_begin, FALSE);
   row_begin = bytes_imported + translator.bytes_transferred;

   for (index = 0; index < TAB_ARRAY_SIZE; ++index)
      current_par.tabs[index].tab_type &= TAB_TYPE_MASK;
   
   column_ctr = 0;
}


/* TranslateAlignment returns an alignment value based upon the param string. */

long PaigeHTMLImportFilter::TranslateAlignment (pg_char_ptr param)
{
   long        result = 0;
   long        param_value;
   
   param_to_lowercase(param);

   if ((param_value = table_command(data_values, param, (pg_short_t)pgCStrLength(param))) > 0) {
      
      switch (param_value) {

         case absbottom_value:
            result = justify_absbottom;
            break;
            
         case absmiddle_value:
            result = justify_absmiddle;
            break;
            
         case baseline_value:
            result = justify_baseline;
            break;
            
         case center_value:
            result = justify_center;
            break;
            
         case left_value:
            result = justify_left;
            break;

         case middle_value:
            result = justify_middle;
            break;

         case right_value:
            result = justify_right;
            break;

         case top_value:
            result = justify_top;
            break;
      }
   }

   return   result;
}

/* TranslateColor takes a color string param and sets a color_value. If color is null then
a long-color ref is returned (RBG). */
//QUALCOMM Begin - Kusuma (mimic v3.14)
//long PaigeHTMLImportFilter::TranslateColor (pg_char_ptr param, color_value_ptr color)
long PaigeHTMLImportFilter::TranslateColor (pg_char_ptr param, color_value_ptr color, pg_boolean black_OK)
//QUALCOMM End - Kusuma (mimic v3.14)
{
   long        value, red, green, blue;
//QUALCOMM Begin - Kusuma (mimic v3.14)
   short		error_color = FALSE;
//QUALCOMM End - Kusuma (mimic v3.14)
   
   value = 0;

   if (*param == '#') 
      value = string_to_hex(&param[1]);
   
   else {
      long      param_value;
      short		param_size;
      
      param_to_lowercase(param);
	  param_size = (short)pgCStrLength(param);
	  
      if ((param_value = table_command(data_color_values, param, (pg_short_t)param_size)) > 0)
         value = data_colors[param_value - 1];
      else {
		 long		offset;
		 
		 if (all_hex(param, param_size))
		 	value = string_to_hex(param);
		 else {
		 
  	     	offset = ImportedOffset();
         	InsertErrorLog(UNSUPPORTED_COLOR, offset, offset + 1, param, FALSE);
//QUALCOMM Begin - Kusuma (mimic v3.14)
         	error_color = TRUE;
//QUALCOMM End - Kusuma (mimic v3.14)
         }
      }
   }
   
   if (color) {

      if ((red = (value >> 8) & 0xFF00) != 0)
         red |= 0x00FF;
      if ((blue = (value << 8) & 0xFF00) != 0)
         blue |= 0x00FF;
      if ((green = (value & 0xFF00)) != 0)
         green |= 0x00FF;
   
      color->red = (pg_short_t)red;
      color->green = (pg_short_t)green;
      color->blue = (pg_short_t)blue;
   }
   else {
      
      red = value & 0xFF0000;
      blue = value & 0x00FF;
      value &= 0x00FF00;
      value |= ((red >> 16) | (blue << 16));
   }

//QUALCOMM Begin - Kusuma (mimic v3.14)
   if (!black_OK && !value && !error_color)
   		value = 0x00010101;
//QUALCOMM End - Kusuma (mimic v3.14)

   return   value;
}

/* TranslateFont translates the font face attribute. */

void PaigeHTMLImportFilter::TranslateFont (pg_char_ptr param)
{
   font_info         font;
   long				 offset = ImportedOffset();
   short          	 index, font_out;
   
   index = 0;

   while (param[index] == ' ' || param[index] == ',')
      ++index;
   
   pgFillBlock(&font, sizeof(font_info), 0);
   font_out = 0;
   
   while (param[index]) {
      
      if (param[index] == ',')
         break;
      
      ++font_out;
      font.name[font_out] = param[index];
      ++index;

      if (font_out >= (FONT_SIZE - 2))
         break;
   }
   
   font.name[0] = (pg_char)font_out;

   if (pgIsRealFont(paige_globals, &font, FALSE)) {
      
      pgBlockMove(&font, &current_font, sizeof(font_info));
      import_pg_rec->procs.font_proc(import_pg_rec, &current_font);
   }
   else if (font.name[0])
      InsertFontError(&font.name[1], offset);
}


/* TranslateCharacter returns the HTML translation for special chars. */

pg_char PaigeHTMLImportFilter::TranslateCharacter (pg_char_ptr param)
{
   pg_boolean  found = FALSE;
   pg_char     result = ' ';
   long		   offset;

   if (*param == '#') {
   
      long     nResult = numeric_value(&param[1]);
      
      if (nResult <= 0xFF) {
         result = (pg_char) nResult;

         if (nResult >= 0x80) {
            offset = ImportedOffset();
            InsertErrorLog(HARD_CODED_CHAR_WARNING, offset, offset + 1, NULL, FALSE);
         }
      }
      else {
         // More than one byte - look for CP1252 equivalent
         for (short i = 0; i < kLengthCharEntityArr; i++) {
            if (character_entity_arr[i].entityValue == nResult) {
               result = character_entity_arr[i].platformChar;
               found = TRUE;
               break;
            }
         }

         if (!found) {
      	     offset = ImportedOffset();
             InsertErrorLog(HARD_CODED_CHAR_WARNING, offset, offset + 1, NULL, FALSE);
         }
      }
   }
   else {
      while (*param == ' ')
         ++param;

      for (short i = 0; i < kLengthCharEntityArr; i++) {
         if ( strcmp(character_entity_arr[i].entityName, (const char *) param) == 0 ) {
            result = character_entity_arr[i].platformChar;
            found = TRUE;
            break;
         }
      }

      if (!found) {
         offset = ImportedOffset();
         InsertErrorLog(UNSUPPORTED_SYMBOL, offset, offset + 1, param, FALSE);
      }
   }
   
   return   result;
}


/* ProcessHyperlink processes a link from the HTML parameters. This only gets called following the
ending tag for an anchor "a"-tag. */

void PaigeHTMLImportFilter::ProcessHyperlink (void)
{
   tag_attribute_ptr    params, argument;
   long              num_params;

   link_range.end = bytes_imported + translator.bytes_transferred;
   
   num_params = GetMemorySize(anchor_ref);
   params = (tag_attribute_ptr)UseMemory(anchor_ref);

   if ((argument = find_parameter(params, num_params, link_name_attribute)) != NULL) {
      // A target link
      
      translator.hyperlink_target_changed = TRUE;
      translator.hyperlink_target = default_target;
      translator.hyperlink_target.applied_range = link_range;
      translator.hyperlink_target.type = HYPERLINK_NORMAL;
      translator.hyperlink_target.active_style = translator.hyperlink_target.state1_style = target_link_style;

      if (argument->param_size)
         pgInsertURL(import_pg_rec, &translator.hyperlink_target, argument->param);
   }
   else {   // assume source link if no "name" attribute
      
      translator.hyperlink_changed = TRUE;
      translator.hyperlink = default_source;
      translator.hyperlink.applied_range = link_range;
      translator.hyperlink.active_style = translator.hyperlink.state1_style;
      translator.hyperlink.target_id = 0;
      
      translator.hyperlink.type = HYPERLINK_NORMAL | UNRESOLVED_LINK;

		// BOG: eudora specific hyperlink support. It is a bad bummer that this
		// stuff is ending up here, but the extensibility of the Paige txr classes
		// is not very well thought out, and getting this stuff into our own
		// derived classes was just too unwieldy; tough stuff. Searching on "BOG:"
		// in the \eudora\eudora\ and \eudora\pgheaders\ directories will help one
		// find other Paige modules that have been modified.
		if ((argument = find_parameter(params, num_params, eudora_attribute)) != NULL) {
			if (argument->param_size) {
				char* prm = (char*) argument->param;
				int prmSize = argument->param_size;

				if ( strnicmp( prm, "autourl", prmSize ) == 0 )
					translator.hyperlink.type |= HYPERLINK_EUDORA_AUTOURL;
				else if ( strnicmp( prm, "attachment", prmSize ) == 0 )
					translator.hyperlink.type |= HYPERLINK_EUDORA_ATTACHMENT;
				else if ( strnicmp( prm, "plugin", prmSize ) == 0 )
					translator.hyperlink.type |= HYPERLINK_EUDORA_PLUGIN;
				else if ( strnicmp( prm, "eudorainfo", prmSize ) == 0 )
					translator.hyperlink.type |= HYPERLINK_EUDORA_INFO;
			}
		}
		// END: eudora specific hyperlink support.
      
      if ((argument = find_parameter(params, num_params, link_url_attribute)) != NULL) {

         if (argument->param_size) {
			  //QUALCOMM == Ben  - Made the size 1024 instead of 256
            //pg_char translated_URL[1024];
 			  CString	translated_URL;
			  char*		tempStr;

            
			if (argument->param[0] == '#')
			{
				translated_URL = argument->param;
				translator.hyperlink.target_id = -1;
			}
			else
			{
// BOG: we had to hack this to stop it from stripping cgi stuff off of the link.
// Silly paige boys! We use our own "resolve_URL" routine later.
//            resolve_URL(argument->param, translated_URL);
//QUALCOMM Begin == Kusuma
				//Need to prepend the base_url stuff if it exists
				if (*base_url_string != NULL)
				{
					if (!IsURLScheme((const char*)argument->param))
					{
/*						int remaining;
						strcpy((char*)translated_URL, (const char*)base_url_string);
						remaining = 1023 - strlen((char*)translated_URL);
						if (remaining > 0)
							strncat((char*)translated_URL, (const char*)argument->param, remaining);*/
						translated_URL = (const char*)base_url_string;
						translated_URL += (const char*)argument->param;
					}
					else
						//strcpy( (char*)translated_URL, (const char*)argument->param );
						translated_URL = (const char*)argument->param;
				}
				else
//QUALCOMM Begin == Kusuma
				//strcpy( (char*)translated_URL, (const char*)argument->param );
					translated_URL = argument->param;

				//See if it is a pgp plugin line, in which case we don;t want to decode it
				//cos it strips out the <xxxx:xxxx> part from the URL cos it's interpreted as a tag
				BOOL bDecode = TRUE;

				if (translator.hyperlink.type & HYPERLINK_EUDORA_PLUGIN)
				{
					int index = translated_URL.Find(".ems <");
					if ( index != -1)
					{
						//Just to make sure it is the ems plugin line, check for : and > also
						if ((translated_URL.GetAt(index+10) == '.') && (translated_URL.GetAt(index+15) == '>'))
							bDecode = FALSE;
					}
				}
				
				//Decode the URL which is in HTML form eg: &amp; to & etc
				//Skip attachment URLs since we don't html-ize them.
				if ((bDecode == TRUE) && !(translator.hyperlink.type & HYPERLINK_EUDORA_ATTACHMENT))
					translated_URL = html2text.Html2Text(translated_URL);

				//Strip all the newlines caused by tags due to multiline URLs
				if (translated_URL.Find("\r\n") != -1)
				{
					char* tempPos = 0;
					tempStr = translated_URL.LockBuffer();
					while (tempPos = strstr(tempStr, "\r\n"))
					{
						strcpy(tempPos, tempPos+2);
						tempStr = tempPos;
					}
					translated_URL.ReleaseBuffer();
				}
			}

//QUALCOMM End == Kusuma

			pgInsertURL(import_pg_rec, &translator.hyperlink, (unsigned char *)((LPCTSTR)translated_URL));
// END HACK
			//pgInsertURL(import_pg_rec, &translator.hyperlink, translated_URL);
         }
      }
   }

   UnuseMemory(anchor_ref);
}

/* ProcessEmbed sets up the final things for importing an embedded object. */

void PaigeHTMLImportFilter::ProcessEmbed (void)
{
   translator.format = current_style;
   
   OutputCharacter(DUMMY_LEFT_EMBED);
   OutputCharacter(DUMMY_RIGHT_EMBED);

// Zero-out embed_ref stuff from possible previous embedded insertion.

   current_style.procs = paige_globals->def_style.procs;
   current_style.embed_entry = current_style.embed_style_refcon
         = current_style.embed_refcon = current_style.embed_id = 0;
   current_style.embed_object = MEM_NULL;
   current_style.char_bytes = 0;
   current_style.class_bits = 0;
   
   embed_last_action = TRUE;
}


/* BreakLine terminates a line with a CR.  If large_break is TRUE then the paragraph format
is set up so there is LINE_BREAK_EXTRA amount. The result is TRUE if one or more CRs were
output. If unconditional is TRUE then the first CR is output regardless of a previous CR
or not. */

pg_boolean PaigeHTMLImportFilter::BreakLine (pg_boolean unconditional, pg_boolean large_break)
{
   pg_boolean        result = FALSE;
   short          cr_count = 0;
   
   if (!last_char_out || last_char_out == linebreak_char) {
      
      cr_count = 1;
      
      if (last_char2_out == linebreak_char)
         cr_count += 1;
   }
   
   if (large_break) {
      
      if (unconditional || cr_count == 0) {

         OutputCharacter(linebreak_char);
         OutputCharacter(linebreak_char);

         result = TRUE;
      }
      else
      if (cr_count == 1) {
      
         OutputCharacter(linebreak_char);
         result = TRUE;
      }
   }
   else {
   
      if (!cr_count || unconditional)
         if (OutputCharacter(linebreak_char) == NO_ERROR) {
         
            result = TRUE;
      }
   }

   return   result;
}


/* ProcessAttributes scans the input stream until it sees an end-tag char, processing
all the attributes in the process. The command_type indicates the type of tag we are
dealing with. */

void PaigeHTMLImportFilter::ProcessAttributes (pg_char_ptr attribute_table, short command_type)
{
   pg_char_ptr          command;
   tag_attribute_ptr    params;
   pg_short_t           arg_size;
   long              item_id;

   while ((command = GetNextToken(&arg_size)) != NULL) {
      
      if ((item_id = table_command(attribute_table, command, arg_size)) != 0) {
         
         params = (tag_attribute_ptr)AppendMemory(param_ref, 1, TRUE);
         params->code = (short)item_id;
         
         if (attribute_ptr[attribute_index] == '=') {
            
            GetNextToken(&arg_size);      // Slip '=' sign
            
            if ((command = GetNextToken(&arg_size)) != NULL) {
               
               if (arg_size >= kMaxAttributeSize)
                  arg_size = kMaxAttributeSize -1;
               
               pgBlockMove(command, params->param, (long)arg_size);
               params->param_size = (short)arg_size;
            }
         }
         
         UnuseMemory(param_ref);
      }
      else  // Attribute unknown so skip possible '=' attribute.
      if (attribute_ptr[attribute_index] == '=') {
         
         GetNextToken(&arg_size);   // empties '=' token
         GetNextToken(&arg_size);   // empties item that follows
      }
   }
}


/* InsertTokenChar places a char into the attribute buffer, incrementing the index
(and expanding the memory_ref if necessary). If parse_spaces is FALSE the char is
inserted unconditionally, otherwise if the char <= space it is inserted as a space ONLY
if the previous char is not a space and index > 0. */

void PaigeHTMLImportFilter::InsertTokenChar (pg_char the_char, pg_boolean parse_spaces)
{
   pg_char        use_char;
   
   use_char = the_char;

   if (the_char <= (pg_char)' ' && parse_spaces) {
      long        look_back;
      
      if (!(look_back = attribute_index))
         return;
      
      --look_back;
      
      if (attribute_ptr[look_back] == (pg_char)' ')
         return;
      
      use_char = ' ';
   }
   
   attribute_ptr[attribute_index] = use_char;
   ++attribute_index;
   
   if (attribute_index == attribute_ref_size) {
      
      UnuseMemory(attribute_ref);
      attribute_ref_size += 128;
      SetMemorySize(attribute_ref, attribute_ref_size);
      attribute_ptr = (pg_char_ptr)UseMemory(attribute_ref);
   }
   
   attribute_ptr[attribute_index] = 0;
}

/* UnimplementedTag gets called by the importer whenever there is a tag that isn't processed.
Upon entry, tag is the tag string(without the <brackets> nor the </end tag) terminated with a space.
Paramlist is a parsed string of all the attributes (space-delineated for each token),
terminated with a carriage return. If end_tag is TRUE then the tag is an end-tag.
If the function returns TRUE then something was processed (for example the current style format
has been changed). */

pg_boolean PaigeHTMLImportFilter::UnimplementedTag (pg_char_ptr tag, pg_char_ptr paramlist, pg_boolean end_tag)
{
   return   FALSE;
}

/* PreProcessTag gets called before formal processing is done on the SUPPORTED tag. The
purpose of this virtual function is to allow an override to handle attributes that
are not supported. */

pg_boolean PaigeHTMLImportFilter::PreProcessTag (short tag_type, short tag, tag_attribute_ptr params, long num_params)
{
   return   FALSE;
}

/* InsertFontError inserts an error for a font not existing. The error is inserted only once
for each font name. */

void PaigeHTMLImportFilter::InsertFontError (pg_char_ptr fontface, long offset)
{
	pg_errlog_ptr			errors;
	long					num_errors;
	
	num_errors = GetMemorySize(error_log);
	errors = (pg_errlog_ptr)UseMemory(error_log);
	
	while (num_errors) {
		
		if (errors->code == UNSUPPORTED_FONT)
			if (equal_string(errors->message, fontface))
				break;

		++errors;
		--num_errors;
	}
	
	UnuseMemory(error_log);
	
	if (!num_errors)
		InsertErrorLog(UNSUPPORTED_FONT, offset, offset + 1, fontface, FALSE);
}

/* GetStylesheet set up the current style, font and par per the style ID. */
//QUALCOMM Begin - Kusuma (mimic v3.14)
//void PaigeHTMLImportFilter::GetStylesheet (long style_id)
//{
//	pgGetNamedStyleInfo(import_pg, style_id, &current_style, &current_font, NULL);
//}
void PaigeHTMLImportFilter::GetStylesheet (long style_id, pg_boolean set_whole_style)
{
	if (!set_whole_style)
		pgGetNamedStyleInfo(import_pg, style_id, NULL, &current_font, NULL);
	else {
		color_value		text_color;
	
		text_color = current_style.fg_color;
		pgGetNamedStyleInfo(import_pg, style_id, &current_style, &current_font, NULL);
		current_style.fg_color = text_color;
	}
}
//QUALCOMM End - Kusuma (mimic v3.14)



/***************************** Local Support ******************************/

/* equal_string returns TRUE if the two strings match. */

static pg_boolean equal_string (pg_char_ptr str1, pg_char_ptr str2)
{
	register pg_char_ptr		input;
	register pg_char_ptr		output;
	
	input = str1;
	output = str2;
	
	for (;;) {
		
		if (*input != *output)
			break;
		
		if (*input++ == 0)
			return	TRUE;

		++output;
	}
	
	return	FALSE;	
}


static void param_to_lowercase (pg_char_ptr param)
{
   register pg_char_ptr arg_ptr;
   pg_char              the_char;

   arg_ptr = param;

   while (*arg_ptr) {
      
      the_char = *arg_ptr;
      
      if (the_char >= 'A' && the_char <= 'Z')
         the_char += 0x20;
      
      *arg_ptr++ = the_char;
   }
}


/* This functions scans a table of terms and returns the argument number if found,
or zero if not found. */

static long table_command (pg_char_ptr table, pg_char_ptr argument, pg_short_t arg_size)
{
   register pg_char_ptr    table_ptr;
   register pg_char_ptr    arg_ptr;
   register pg_short_t     arg_ctr;
   long                    table_item;
   pg_char                 arg_char;
   
   if (!arg_size)
      return   FALSE;

   table_ptr = table;
   table_item = 1;

   while (*table_ptr) {
      
      arg_ptr = argument;

	  // We used to convert everything to lower case in ReadHTMLTag.
	  // Most of the time that worked, because the tag names and attribute
	  // values that we're provided in the table parameter are all lower
	  // case. However it turned out that for embedded images if the
	  // img src CID was in upper case, then our ability to find the
	  // corresponding image was broken by converting everything to lower 
	  // case in ReadHTMLTag.
	  //
	  // So now we convert the case here immediately before doing the
	  // character comparison.
	  arg_char = *arg_ptr;

	  if (arg_char >= 'A' && arg_char <= 'Z')
		  arg_char += 0x20;

      if (*table_ptr > arg_char)
         return   FALSE;
      
      if (*table_ptr == arg_char) {
      
         arg_ctr = arg_size;
   
         for (arg_ctr = arg_size; arg_ctr; --arg_ctr) {

            // Convert the case here immediately before doing the character comparison
            // (see above comment).
            arg_char = *arg_ptr++;

            if (arg_char >= 'A' && arg_char <= 'Z')
                arg_char += 0x20;

            if (*table_ptr++ != arg_char)
               break;

            if (*table_ptr == ' ') {
               
               if (arg_ctr == 1)
                  return   table_item;
               
               break;
            }
         }
      }

      for (;;)
         if (*table_ptr++ == ' ')
            break;

      table_item += 1;
   }

   return      0;
}


/* is_hex_char returns TRUE if the_char is hexidecimal ascii. */

static pg_boolean is_hex_char (pg_char the_char)
{
   if (the_char >= '0' && the_char <= '9')
      return   TRUE;
   if (the_char >= 'a' && the_char <= 'f')
      return   TRUE;
   if (the_char >= 'A' && the_char <= 'F')
      return   TRUE;

   return      FALSE;
}


/* string_to_hex returns the numeric value of a string. */

static long string_to_hex (pg_char_ptr string)
{
   register pg_char_ptr string_ptr;
   long              result = 0;
   
   string_ptr = string;

   while (*string_ptr > 0) {
      pg_char        the_char;

      the_char = *string_ptr++;
      
      result <<= 4;
      result |= (long)translate_hex(the_char);
   }

   return      result;
}


static void to_lower (pg_char_ptr the_char)
{
   if (*the_char >= 'A' && *the_char <= 'Z')
      *the_char += 0x20;
}

/* find_parameter searches through the params looking for wanted_code. If found the
pointer is returned. */

static tag_attribute_ptr find_parameter (tag_attribute_ptr params, long num_params, short wanted_code)
{
   register tag_attribute_ptr param_ptr;
   long                 index;
   
   param_ptr = params;
   
   for (index = 0; index < num_params; ++index, ++param_ptr)
      if (param_ptr->code == wanted_code)
         return   param_ptr;
   
   
   return   NULL;
}


/* numeric_value returns the numeric value of *param - converting both hex and decimal. */

static long numeric_value (pg_char_ptr param)
{
   long     value;
   short    index;

   value = 0;
   index = 0;
   
   if (param[index] == '-')
      ++index;
   
   if ( (param[index] == 'x') || (param[index] == 'x') ) {
      // Skip past the 'X'
      ++index;

#define Hex2Nyb(c) (c<='9'?c-'0':(c>='a'?c-'a'+10:c-'A'+10))
      while (param[index]) {
         if (param[index] < '0' || param[index] > '9')
         	break;
 
         value *= 16;
         value += Hex2Nyb(param[index]);
         ++index;
      }
   }
   else {
      while (param[index]) {
         short    digit;
      
         if (param[index] < '0' || param[index] > '9')
         	break;
 
         digit = (short)param[index];
         ++index;
      
         digit &= 0x000F;
         value *= 10;
         value += digit;
      }
   }
   
   if (*param == '-')
      value = -value;
   
   return   value;
}

/* decimal_value returns the decimal numeric value of *param with possible percent. */

static long decimal_value_percent (pg_char_ptr param, long max_for_percent)
{
   long     value;
   short    index;

   value = 0;
   index = 0;
   
   if (param[index] == '-')
      ++index;
   
   while (param[index]) {
      short    digit;
      
      digit = (short)param[index];
      ++index;
      
      if (digit == '%') {
         value = (max_for_percent * value) / 100;
         break;
      }

      digit &= 0x000F;
      value *= 10;
      value += digit;
   }
   
   if (*param == '-')
      value = -value;
   
   return   value;
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

/* test_extension returns TRUE if file_ext and match are the same 3 characters. */

static pg_boolean test_extension (pg_char_ptr file_ext, pg_char_ptr match)
{
   short       index;
   
   for (index = 0; index < 3; ++index)    
      if (file_ext[index] != match[index])
         return   FALSE;
   
   return   TRUE;
}



/* all_hex returns TRUE if all the chars in hex_str are hex. */

static pg_boolean all_hex (pg_char_ptr hex_str, short str_size)
{
	short		index;
	
	for (index = 0; index < str_size; ++index)
		if (!is_hex_char(hex_str[index]))
			return	FALSE;
	
	return	TRUE;
}

/* translate_single_file strips out the ending file name in URL, if any. Max output
is 31 chars + null terminator. */

static void translate_single_file (pg_char_ptr URL, pg_char_ptr filename)
{
	long		input_size, output_size, index;
	
	input_size = index = pgCStrLength(URL);
	
	while (index) {
		
		--index;
		
		if (URL[index] == 0x2F)
			break;
	}
	
	if (URL[index] == 0x2F)
		++index;
	
	output_size = input_size - index;
	
	if (output_size > 31)
		output_size = 31;
	
	if (output_size)
		pgBlockMove(&URL[index], filename, output_size);
	
	filename[output_size] = 0;
}
