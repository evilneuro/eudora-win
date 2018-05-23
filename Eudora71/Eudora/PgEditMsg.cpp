// PgEditMsg.cpp -- handles all the "EDIT" messages
//
// Copyright (c) 2000 by QUALCOMM, Incorporated
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

//

#include "stdafx.h"

#include "Paige.h"
#include "pgTraps.h"
#include "pgUtils.h"
#include "pgShapes.h" 
#include "pgEmbed.h"
#include "machine.h"
#include "defprocs.h"
#include "pgOSUtl.h"
#include "pgEdit.h"
#include "pgFiles.h"
#include "pgSubref.h"
#include "pgExceps.h"
#include "pgScrap.h"

#include "paigeedtview.h"
#include "pgstyleutils.h"


#include "DebugNewHelpers.h"


// BOG: the em_linescroll implementation is mostly unchanged from the original
// Paige source. it probably only works well for plain text documents, as
// it won't correctly handle variable char heights. an alternate implementation
// would be to pump "OnVScroll" calls---one for each line.

BOOL CPaigeEdtView::EditWindowProc( UINT message, WPARAM wParam, LPARAM lParam, LPLONG response )
{
	BOOL handled = TRUE;   // unless we hit "default," we handled it
	long line_begin, line_end, textsize;
	rectangle vis_rect;

	// this is the usual winproc return value
	*response = 0;

	switch (message)
	{
	case EM_SETSEL:
		pgSetSelection(m_paigeRef, (long)wParam, (long)lParam, 0, TRUE);
		break;

	case EM_GETSEL:
		{
		LPDWORD outPtr;
		select_pair curSel;
		pgGetSelection(m_paigeRef, &curSel.begin, &curSel.end);

		if ( outPtr = (LPDWORD)wParam )
			*outPtr = curSel.begin;
		if ( outPtr = (LPDWORD)lParam )
			*outPtr = curSel.end;
		}
		break;


	case EM_HIDESELECTION:
		if ( wParam )   // nonzero == hide the selection
			pgSetHiliteStates(m_paigeRef, deactivate_verb, no_change_verb, TRUE);
		else
			pgSetHiliteStates(m_paigeRef, activate_verb, no_change_verb, TRUE); 

		break;

	case EM_GETRECT:
		pgAreaBounds(m_paigeRef, MEM_NULL, &vis_rect);
		RectangleToRect(&vis_rect, NULL, (Rect PG_FAR *)lParam);
		break;

	case EM_LINESCROLL:
		{
		rectangle char_rect;
		long h_scroll, v_scroll;

		pgCharacterRect(m_paigeRef, 0, FALSE, FALSE, &char_rect);
		v_scroll = char_rect.bot_right.v - char_rect.top_left.v;
		v_scroll *= (long)lParam;
		h_scroll = 0;   // we don't do horizontal scrolling

		pgScrollPixels(m_paigeRef, h_scroll, v_scroll, best_way);
		UpdateScrollBars();
		}
		break;

	case EM_SCROLL:
		OnVScroll( wParam, 0, NULL );
		break;

	case EM_GETMODIFY:
		*response = GetDocument()->IsModified();
		break;

	case EM_SETMODIFY:
		GetDocument()->SetModifiedFlag( wParam );
		break;

	case EM_GETLINECOUNT:
		*response = pgNumLines(m_paigeRef);
		break;

	case EM_LINEINDEX:
		pgLineNumToOffset(m_paigeRef, (long)wParam, response, &line_end);
		break;

	case EM_LINELENGTH:
		pgFindLine(m_paigeRef, (long)wParam, &line_begin, &line_end);
		*response = line_end - line_begin;
		break;

	case WM_GETFONT:
		{
		style_info styleInfo, styleMask;
		pgGetStyleInfo(m_paigeRef, NULL, FALSE, &styleInfo, &styleMask);
		*response = (long)styleInfo.machine_var;
		}
		break;

	case WM_SETFONT:
		{
		LOGFONT lf;
		::GetObject((HANDLE)wParam, sizeof(LOGFONT), &lf);

		font_info fontInfo, fontMask;
		style_info styleInfo, styleMask;

		pgFillBlock(&fontMask, sizeof(font_info), -1);
        PgConvertLogFont( m_paigeRef, PgGlobalsPtr(), &lf, &fontInfo,
				&styleInfo, &styleMask );
		pgSetStyleAndFont(m_paigeRef, NULL, &styleInfo, &styleMask,
				&fontInfo, &fontMask, best_way);
		}
		break;

	case WM_GETTEXTLENGTH:
		*response = (long)pgTextSize(m_paigeRef);
		break;

	case EM_GETSELTEXT:
	case WM_GETTEXT:
		{
		pg_ref pg;

		if (message == EM_GETSELTEXT) {
			pg = pgCopy(m_paigeRef, 0);
			wParam = pgTextSize(pg) + 1;
		}
		else
			pg = m_paigeRef;

		if (wParam > 0) {
			paige_rec_ptr pg_rec;
			text_block_ptr block;
			pg_char_ptr user_ptr;
			long max_size, actual_size, block_size;

			pg_rec = (paige_rec_ptr) UseMemory(pg);
			block = (text_block_ptr) UseMemory(pg_rec->t_blocks);
			user_ptr = (pg_char_ptr)lParam;
			max_size = (long)wParam;
			max_size -= sizeof(char);

			for (actual_size = 0; actual_size < max_size; ++block) {
				block_size = block->end - block->begin;

				if ((actual_size + block_size) > max_size)
					block_size = max_size - actual_size;

				pgBlockMove(UseMemory(block->text), user_ptr, block_size * sizeof(pg_char));
				UnuseMemory(block->text);

				actual_size += block_size;
				user_ptr += block_size;

				if (block->end == pg_rec->t_length)
					break;
			}

			*user_ptr = 0;
			*response = actual_size;
			UnuseMemory(pg_rec->t_blocks);
			UnuseMemory(pg);

			if (pg != m_paigeRef)
				pgDispose(pg);
		}
		}

		break;

	case WM_SETTEXT:
		{
		// wm_settext is like em_replacesel, except that existing text is
		// always replaced---so smoke all text then just fall through
		select_pair doc;
		doc.begin = 0;
		doc.end = pgTextSize( m_paigeRef );
		pgDelete(m_paigeRef, &doc, best_way);
		}

	case EM_REPLACESEL:
		if (!lParam)
			textsize = 0;
		else
			textsize = lstrlen((LPSTR)lParam);

		if (textsize == 0) {
			if (pgNumSelections(m_paigeRef) > 0) {
				PrepareUndo(undo_delete, MEM_NULL, TRUE);
				pgDelete(m_paigeRef, NULL, best_way);
			}
		}
		else {
			SetSelectedText( (LPCSTR)lParam, false );
		}

		UpdateScrollBars();
		break;

	case EM_GETLINE:
		{
		memory_ref ref;
		select_pair line_bounds;
		short PG_FAR *maxsize;

		pgLineNumToOffset(m_paigeRef, (long)wParam, &line_bounds.begin, &line_bounds.end);

		if ((ref = pgCopyText(m_paigeRef, &line_bounds, all_data)) != MEM_NULL) {
			maxsize = (short PG_FAR *)lParam;
			*response = GetMemorySize(ref);

			if (*response > (long)*maxsize)
				*response = (long)*maxsize;

			if (*response) {
				pgBlockMove(UseMemory(ref), maxsize, *response * sizeof(pg_char));
				UnuseMemory(ref);
			}

			DisposeMemory(ref);
		}
		}
		break;
         
	case EM_CANUNDO:
		*response = (m_undoStack.GetCount() != 0);
		break;

	case EM_FMTLINES:
		break;

	case EM_LINEFROMCHAR:
		line_begin = (long)wParam;

		if (line_begin < 0)
			pgGetSelection(m_paigeRef, &line_begin, NULL);

		*response = pgOffsetToLineNum(m_paigeRef, line_begin, FALSE);
		break;

	case EM_SETTABSTOPS:
		break;

	case EM_SETPASSWORDCHAR:
		break;

	case EM_GETFIRSTVISIBLELINE:
		break;

	case EM_SETREADONLY:
		SetReadOnly( wParam != 0 );
		*response = TRUE;
		break;

	case EM_SETWORDBREAKPROC:
	case EM_GETWORDBREAKPROC:
		break;
      
	case WM_UNDO:
	case EM_UNDO:
		OnEditUndo();
		*response = TRUE;
		break;

	default:
		handled = FALSE;
		break;
	}

	return handled;
}

