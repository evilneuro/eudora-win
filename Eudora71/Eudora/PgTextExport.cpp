//	PaigeTextExport.cpp
//	
//	Paige text exporting that better handles embedded images and
//	emoticons, while leaving the parent class PaigeExportFilter,
//	which was previously used for exporting, untouched.
//
//	Copyright (c) 2004 by QUALCOMM, Incorporated
/* Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. */



#include "stdafx.h"

#include "PgTextExport.h"
#include "pgOSUtl.h"

#include "DebugNewHelpers.h"


PaigeTextExportFilter::PaigeTextExportFilter()
{
	//	We want to specially handle embedded objects, so we set the flags:
	//	EXPORT_TEXT_FORMATS_FLAG		- to stop on each format change (in case
	//									  the new format is for an embedded image)
	//	EXPORT_EMBEDDED_OBJECTS_FEATURE	- pass us information for each embedded image
	feature_bits |= EXPORT_TEXT_FORMATS_FLAG | EXPORT_EMBEDDED_OBJECTS_FEATURE;
}


PaigeTextExportFilter::~PaigeTextExportFilter()
{

}


pg_boolean PaigeTextExportFilter::pgWriteNextBlock()
{
	pg_boolean		bResult = true;
	
	//	Because we set the EXPORT_EMBEDDED_OBJECTS_FEATURE feauture_bits flag,
	//	we expect embedded objects to:
	//	* Be marked as such
	//	* Have the embed_ref stashed in translator.data_param1
	//	* Have exactly 2 bytes of text - the "[]" dummy embed text - I'm not
	//	  sure if the exactly 2 bytes requirement breaks down with unicode.
	if ( (translator.data_type == export_embed_type) &&
		 translator.data_param1 &&
		 (translator.bytes_transferred == 2) )
	{		
		//	Now that we've recognized the data as an embedded image we
		//	no longer care about outputting the "[]" dummy embed text.
		//	We'd like to output the alt_string if possible, which for
		//	emoticons is the trigger, or nothing if we can't get that.
		//	Outputting the "[]" dummy embed text wouldn't help anything.
		
		//	Get the embed ref and pointer
		embed_ref		refEmbed = translator.data_param1;
		pg_embed_ptr	pEmbed = reinterpret_cast<pg_embed_ptr>( UseMemory(refEmbed) );

		if (pEmbed)
		{
			if (pEmbed->data)
			{
				//	Get the pg_url_image ref and pointer
				memory_ref			refImage = reinterpret_cast<memory_ref>( pEmbed->data );
				pg_url_image_ptr	pImage = reinterpret_cast<pg_url_image_ptr>( UseMemory(refImage) );
				
				if (pImage)
				{
					//	Output the alt_string if we have it
					if (pImage->alt_string && *pImage->alt_string)
					{
						long		nBytes = pgCStrLength(pImage->alt_string);
						pg_error	nResult = io_proc(pImage->alt_string, io_data_direct, &filepos, &nBytes, filemap);
						
						bResult = (nResult == NO_ERROR);
					}

					//	Unlock the embed_ref
					UnuseMemory(refImage);
				}
			}

			//	Unlock the pg_url_image memory_ref
			UnuseMemory(refEmbed);
		}
	}
	else
	{
		//	Alert developer if we failed one of the additional tests for an
		//	embedded image.
		ASSERT(translator.data_type != export_embed_type);
		
		//	Output everything other than embedded images as normal
		bResult = PaigeExportFilter::pgWriteNextBlock();
	}

	return bResult;
}
