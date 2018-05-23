// SpellPaige.cpp
//
// Copyright (c) 1997-2000 by QUALCOMM, Incorporated
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

#include "resource.h"
#include "utils.h"
#include "rs.h"
#include "guiutils.h"

// For all text functions
#include "pgtraps.h"
#include "pgscrap.h"

#include "PaigeEdtView.h"
#include "Spell.h"
#include "spellpaige.h"
#include "SpellDialog.h"
#include "PaigeStyle.h"
#include "pghtext.h"
#include "CompMessageFrame.h"
#include "QCCommandActions.h"
#include "QCCommandDirector.h"



extern QCCommandDirector	g_theSpellingDirector;


// Paige
//#include "defprocs.h"
//#include "pgutils.h"
//#include "paige.h"

#include "DebugNewHelpers.h"

CSpellPaige::CSpellPaige(BOOL hasQuotedText /*= FALSE*/) :
	CSpell(), m_bRedrawMisspelledWordsImmediately(true), m_bIsRedrawNeeded(false),
	m_nSpellDialogRefCount(0)
{
    m_pView = NULL;
    m_paigeRef = NULL;
        
    m_HasQuotedText = hasQuotedText;
    m_prevWord[0] = 0; 
    m_pSpellDialog = NULL;
}


// this must always be capable of being called multiple times, since
// this object may get re-initialized.
void CSpellPaige::Init( CPaigeEdtView* pView )
{
    m_pView = pView;
    m_paigeRef = pView->m_paigeRef;
}


CSpellPaige::~CSpellPaige()
{
	Cleanup();
}

 
// ============================================= 

int CSpellPaige::ErrMsg(int ret)
{       
	CString Cause;
	UINT CauseID = IDS_SPELL_BASE_ERROR;

	if (ret <= SpellCheck_TOO_MANY_SESSIONS_ERR && ret >= SpellCheck_BAD_BLOCK_ID_ERR)
		CauseID += -ret;

	Cause.LoadString(CauseID);
	ErrorDialog(IDS_SPELL_ERR_FORMAT, (const char*) Cause);
	
	return (ret);
}
 
void CSpellPaige::Cleanup()
{
	//	Bad things happen if we delete the dialog while it's open.
	//	Let's not do that.
	if ( (m_pSpellDialog != NULL) && (m_nSpellDialogRefCount == 0) &&
		 !m_pSpellDialog->IsDialogOpen() )
	{
		delete m_pSpellDialog;
		m_pSpellDialog = NULL;
	}
}

int CSpellPaige::Check(CPaigeEdtView* pView, int LinesFromTop /*= 0*/, BOOL autoCheck /*=FALSE*/)
{
	m_pView = pView;
	m_paigeRef = pView->m_paigeRef;
	return ReallyCheck(LinesFromTop, autoCheck);
}


int CSpellPaige::Check( select_pair* pTextRange /*=NULL*/, int LinesFromTop /*= 0*/,
			BOOL autoCheck /*=FALSE*/ )
{
	if (!m_pView || !m_paigeRef)
	{
		ASSERT(0);
		return -1;
	}

    return ReallyCheck( LinesFromTop, autoCheck, pTextRange, false );
}



int CSpellPaige::ReallyCheck( int LinesFromTop, BOOL autoCheck,
                              select_pair* pTextRange /*= NULL*/, bool bInteractive /*= true*/ )
{
    int result;

	//SDSpellTRACE("ReallyCheck %d.%d\n",pTextRange?pTextRange->begin:-1,pTextRange?pTextRange->end:-1);
    if ( m_pSpellDialog == NULL )
    {
		m_pSpellDialog = DEBUG_NEW CSpellDialog( this );

		// We just had to create the dialog - the ref count should be 0.
		ASSERT(m_nSpellDialogRefCount == 0);
		m_nSpellDialogRefCount = 0;
    }

    if( ( m_pSpellDialog == NULL ) || m_pSpellDialog->GetLastError() ) 
    {
        // the dialog could not be successfully created.
        return -1;
    }

	// Make sure we don't delete the dialog (due to focus changing or whatever)
	// while we're working here.
	m_nSpellDialogRefCount++;

    m_prevWord[0] = 0; 
    m_bWasHighlighted = TRUE;
    pgGetSelection(m_paigeRef, &m_BegChar, &m_EndChar);

    int nOldCaretPos = m_BegChar;
    if (m_BegChar == m_EndChar)
    {
        m_bWasHighlighted = FALSE;

        if ( pTextRange ) {
            m_BegChar = pTextRange->begin;
            m_EndChar = pTextRange->end;
        }
        else {
            m_BegChar = 0;
            m_EndChar = pgTextSize(m_paigeRef);
        }
    }   

    m_CurPos = m_BegChar;
    
    m_DidTopScroll = FALSE;
    m_TopVisibleLine = 3 - LinesFromTop;

    m_pSpellDialog->SetMisspellings(FALSE);

    // Do the checking
    BOOL bDoCoolSpell = FALSE;
    BOOL bFirstPass = TRUE;

 redo:
	// Let's be safe, ok?
	//ASSERT(m_BegChar>=0);	// uncomment to check fix below
	//ASSERT(m_EndChar<=pgTextSize(m_paigeRef));	// uncomment to check fix below
	m_BegChar = max(0L,m_BegChar);
	m_EndChar = min(m_EndChar,pgTextSize(m_paigeRef));
	m_BegChar = min(m_BegChar,m_EndChar);
   
    if ( autoCheck && bFirstPass ) {
	bDoCoolSpell = TRUE;
    }
    else {
        if ( (GetIniShort(IDS_INI_CRAIG) ||
	      GetIniShort(IDS_INI_INTERACTIVE_SPELL_CHECK) == FALSE) && !bInteractive ) {

            bDoCoolSpell = TRUE;
	}

        if ( ShiftDown() && bInteractive )
            bDoCoolSpell = !bDoCoolSpell;
    }

    if ( bDoCoolSpell ) 
        result = m_pSpellDialog->DoCoolSpell( autoCheck );
    else
        result = m_pSpellDialog->DoModalSpell( autoCheck );
  
    // Return style back
    int result2;
    if( ( result == IDCANCEL ) && ( ( result2 = m_pSpellDialog->GetLastError() ) != 0 ) ) {		
		// We're done with the dialog now
		m_nSpellDialogRefCount--;

        Cleanup();
		
        return (ErrMsg(result2));
    }
    else {
	// BOG: if we're doing an autocheck-on-queue/send, then the first time through
	// is a invisible check, with a warning of any misspellings. if the user decides
	// to spellcheck the doc for real, we go 'round again in interactive mode.

	if ( autoCheck && bFirstPass && (result == FALSE) ) {
#if 0
// So, Irwin don't want to see the warning dlg anymore? Fine wi' me! We'll just cut
// to the chase, and bring the spell dlg up right away.
	    int iRet = WarnTwoOptionsCancelDialog( 0, IDS_SPELL_ON_QUEUE_WARN_MISSPELLINGS,
						   IDS_YES_BUTTON, IDS_NO_BUTTON );

	    if ( iRet == IDC_WARN_BUTTON1 ) {
		m_CurPos = m_BegChar;
		bDoCoolSpell = bFirstPass = FALSE;
		goto redo;
	    }
	    else if ( iRet == IDC_WARN_BUTTON2 )
		result = TRUE;
	    else
		result = IDCANCEL;
#endif
// Hack for Irwin -- head for the second pass
	    m_CurPos = m_BegChar;
	    bDoCoolSpell = bFirstPass = FALSE;
	    MessageBeep( MB_ICONEXCLAMATION );
	    goto redo;
	}

	// BOG: no longer display this message when doing CoolSpell, as we are now doing
	// CoolSpell on the fly. the old way is dead.

        if ( !bDoCoolSpell && (m_pSpellDialog->FoundMisspelling() == FALSE) && !autoCheck ) 
		{
		   /*Commenting this code since we will now have spell check in headers too.
		   If we show this message here and show same message in header, User will be annoyed.
		   So Now CompMessageFrame will decide to show this message only if there is 
		   no misspellings in Body and Header.
          ::MessageBox( m_pView->m_hWnd, (LPCTSTR)CRString(IDS_SPELL_NO_MISSPELLINGS),
			  (LPCTSTR)CRString(IDS_EUDORA), MB_OK);*/
			result = NO_MISSPELLINGS;
        }
    }                                   

// BOG: keep the spell dialog around for CoolSpell; recreating for each invocation
// is way too expensive.
//    delete m_pSpellDialog;
//    m_pSpellDialog = NULL;

    if (m_bWasHighlighted)
        pgSetSelection(m_paigeRef, m_BegChar, m_EndChar, 0, TRUE);
    else
        pgSetSelection(m_paigeRef, nOldCaretPos, nOldCaretPos, 0, TRUE);

	// We're done with the dialog now
	m_nSpellDialogRefCount--;

    return (result);
}  


void CSpellPaige::myHilightWord( BOOL bMark /* = FALSE */ )
{
    if ( !bMark ) {
        // Keep the hilite even though the message window does not have focus.
        pgSetHiliteStates( m_paigeRef, activate_verb, activate_verb, TRUE );

        // Select and hilite the misspelled word.
        pgSetSelection( m_paigeRef, m_theSelection.begin, m_theSelection.end, 0, TRUE );
        pgPaginateNow( m_paigeRef, CURRENT_POSITION, FALSE );
        pgScrollToView( m_paigeRef, CURRENT_POSITION, 32, 32, TRUE, best_way );
    }
    else {
        // Don't bother hilighting and scrolling, just mark it as misspelled
        CPaigeStyle style( m_paigeRef );
        style.ApplyMisspelled( TRUE, &m_theSelection, m_bRedrawMisspelledWordsImmediately );
		if (!m_bIsRedrawNeeded)
			m_bIsRedrawNeeded = !m_bRedrawMisspelledWordsImmediately;
    }
}

void CSpellPaige::mySetRedraw( BOOL bRedraw /* = TRUE */)
{
	m_bRedrawMisspelledWordsImmediately = (bRedraw != FALSE);
}

void CSpellPaige::my_ReplaceBlockWord(UCHAR* CurWord, const UCHAR* repWord)
{
    // clear any spelling-related styles
    select_pair sel;
    pgGetSelection( m_paigeRef, &sel.begin, &sel.end );
    CPaigeStyle style( m_paigeRef );
    style.ApplyMisspelled( FALSE, &sel );

    int OldLen,NewLen,AdjLen;

//      To delete a word I reselect the word to remove with trailing white space
//      and replace that selection with a null string.

    OldLen = strlen((const char*)CurWord);  
    NewLen = strlen((const char*)repWord);
    AdjLen = NewLen - OldLen;

    //
    // Adjust the selection to include the following white space
    // if we are deleting the selection.
    //
    if ((NewLen == 0) && isspace((int)(unsigned char)m_termChar) )
    {
        select_pair sel;
        pgGetSelection(m_paigeRef, &sel.begin, &sel.end);
        sel.end += 1;
        AdjLen -= 1;
        pgSetSelection(m_paigeRef, sel.begin, sel.end, 0, FALSE);
    }   
    //
    // We're adding or removing characters so adjust the indices
    //
    if (AdjLen!=0)                      
    {
        m_CurPos += AdjLen; 
        m_EndChar += AdjLen;
    }
    //
    // Deleting cannot be simulated by inserting a NULL string.
    // Therefore, check explicitly for a delete operation.
    //
    if ( *repWord == '\0' )
        pgCut(m_paigeRef,NULL,best_way);
    else
        pgInsert( m_paigeRef, (pg_byte_ptr)repWord, NewLen, CURRENT_POSITION, 
                  data_insert_mode, 0, best_way );

    m_pView->Invalidate();
    m_pView->UpdateWindow();
    m_pView->UpdateChangeState( CPaigeEdtView::kAddChange );
} 

// @PUBLIC_FUNCTION: BlockIsDoubledWord
// This function returns TRUE if the current word in a block is a duplicate of
// the word preceding it. Otherwise, FALSE is returned.
//
// Return value: TRUE if the current word is doubled; FALSE if it's not
// doubled.
// @END
//
int CSpellPaige::BlockIsDoubledWord(const char* curWord)
{
	if (strcmp((char*)m_prevWord, curWord) == 0)
	{
		// Ensure that we report this only once.
		if ( !((strlen((char*)curWord)==1) && ( ispunct((int)(unsigned char)*curWord) || isspace((int)(unsigned char)*curWord) ) ) )
		{
			m_prevWord[0] = 0;
			return (TRUE);
		}
	}

	// Save the current word so we can detect if the next word is
	// a repetition of it. However, if the current word terminates
	// a sentence or clause, artificially prevent the doubled-word
	// test from succeeding.
	//
	if (!isspace((int)(unsigned char)m_termChar)  && ispunct((int)(unsigned char)m_termChar))
	{
		// The word ended in punctuation, so disable the doubled-
		// word test next pass.
		m_prevWord[0] = 0;
	}
	else
	{
		// No terminating punctuation, so enable the doubled-word
		// test next pass.
		if ( strlen(curWord) < MAX_WORD_SIZE )
			strcpy((char*)m_prevWord, curWord);
		else
		{
			strncpy((char*)m_prevWord, curWord, MAX_WORD_SIZE-1);
			m_prevWord[MAX_WORD_SIZE-1] = NULL;
		}

	}

	return (FALSE);
}    
      

int CSpellPaige::my_CheckBlock(long options, UCHAR* errWord, UCHAR* repWord) 
{
	int result;
	//
	// Initialize constants for plain text excerpt detection
	//
	BOOL bIgnoreOriginal= GetIniShort(IDS_INI_IGNORE_ORIGINAL);
	const char *xQuote	= GetIniString(IDS_INI_QUOTE_PREFIX);
	const char *xReply	= GetIniString(IDS_INI_REPLY_PREFIX);
	int xReplyLen = strlen(xReply);			
	int xQuoteLen = strlen(xQuote);
	int maxLen = max(xReplyLen,xQuoteLen);
	CPaigeStyle style(m_paigeRef);
	pg_hyperlink hyperlink;

	while (m_CurPos < m_EndChar ) //m_CurRow <= m_EndRow)
	{ 
		//
		//  Skip quoted lines 
		//
		if (!m_bWasHighlighted && bIgnoreOriginal )
		{
			//
			// Offset of current line
			//
			select_pair sel;
			pgFindPar(m_paigeRef, m_CurPos, &sel.begin,&sel.end);
			int nCacheEnd = sel.end;
			//
			// HTML Excerpts
			//
			if ( style.IsExcerpt(&sel) )
			{
				m_CurPos=nCacheEnd+1;
				continue;
			}
			//
			// Get the minimum number of characters necessary from Paige
			// for the prefix comparison.
			//
			int nLen = min((int)(sel.end-sel.begin), maxLen);
			sel.end = sel.begin + nLen;

			if ((xReplyLen && !m_pView->StrPgCmp(xReply,sel)) ||
				(xQuoteLen && !m_pView->StrPgCmp(xQuote,sel)) )
			{
				m_CurPos = nCacheEnd+1;
				continue;
			}	
		}

		// Ignore the URLs
		if ( pgGetHyperlinkSourceInfo(m_paigeRef, m_CurPos, 0, FALSE, &hyperlink) )
		{
			if (hyperlink.type & HYPERLINK_EUDORA_AUTOURL)
			{
				m_CurPos = hyperlink.applied_range.end + 1;

				//Make sure that similar words before and after the URL are not reported as doubled word
				m_prevWord[0] = 0;

				if (m_CurPos >= m_EndChar )
					return (SpellCheck_END_OF_BLOCK_RSLT);
			}
		}

		//
		// Find the next word
		//
		pgFindWord(m_paigeRef, m_CurPos, &m_theSelection.begin,&m_theSelection.end,FALSE,FALSE);
		m_CurPos = m_theSelection.end+1;
	
		if ( m_theSelection.begin != m_theSelection.end )
		{
			long		length = 0;
			
			m_pView->GetPgText( reinterpret_cast<char *>(errWord), SpellCheck_MAX_WORD_SZ, m_theSelection, FALSE, &length );

			// If word starts with single quote - remove it
			if ( (length > 0) && (static_cast<char>(*errWord) == '\'') )
			{
				memcpy(errWord, errWord+1, length-1);
				m_theSelection.begin++;
				length--;
				errWord[length] = 0;
			}

			// If word ends with single quote - remove it.
			// (SpellCheck_SPLIT_CONTRACTED_WORDS_OPT partially handles an ending single
			// quote, but misses cases like "familiess'" and "companiess'".)
			if ( (length > 0) && (static_cast<char>(errWord[length-1]) == '\'') )
			{
				length--;
				errWord[length] = 0;
				m_theSelection.end--;
			}

			//
			// Store the terminating character.  Need this for double word detection
			//
			text_ref	ref_for_text;
			pg_byte_ptr	pText;
			m_termChar = NULL;
			if ( pText = pgExamineText(m_paigeRef, m_theSelection.end, &ref_for_text, &length))
			{
				m_termChar = *(char*)pText;
				UnuseMemory(ref_for_text);
			}

			// Hack check for dashes, spaces, etc.
			if ( (strlen((char*)errWord)==1) && ( ispunct((int)(unsigned char)errWord[0]) || isspace((int)(unsigned char)errWord[0] ) ))
				result = SpellCheck_OK_RSLT; 
			else
				result = m_pSpellDialog->my_SpellCheck_CheckWord( options, errWord, repWord, SpellCheck_MAX_WORD_SZ );

			//SDSpellTRACE("Check: %s %d\n",errWord,result);

			// Check for doubled words.
			if (m_pSpellDialog->GetOptionsMask() & SpellCheck_REPORT_DOUBLED_WORD_OPT)
			{
				if (BlockIsDoubledWord((char *)errWord))
					result |= SpellCheck_DOUBLED_WORD_RSLT;
			}
	
			// Exit early if the client must respond to this word.
			if (result & SpellCheck_CHANGE_WORD_RSLT)
			{
				myHilightWord();
				my_ReplaceBlockWord(errWord, repWord);
			}
			else if (result != SpellCheck_OK_RSLT)
			{
				m_pSpellDialog->SetMisspellings( TRUE );
				return (result);
			}
		}
			
	}
	
	return (SpellCheck_END_OF_BLOCK_RSLT);
}


void CSpellPaige::MarkWordNotMisspelled(CPaigeEdtView * pView, const char * in_szWord)
{
	if (!pView || !pView->m_paigeRef)
		return;
	
	m_pView = pView;
	m_paigeRef = pView->m_paigeRef;
	
	style_walk		walker;
	paige_rec_ptr	pgr = reinterpret_cast<paige_rec_ptr>( UseMemory(m_paigeRef) );

	if (pgr)
	{		
		CPaigeStyle		style(m_paigeRef);
		
		//	Walk the mispelled words and see if any match the word we were passed
		pgPrepareStyleWalk(pgr, 0, &walker, FALSE);	// Ignore paragraph styles

		//	Keep track of the location of any words that we find that need to
		//	be marked not mispelled
		vector<select_pair>		markNotMispelledList;
		select_pair				sel;
		
		do
		{
			if ( style.IsMisspelled(walker.cur_style) )
			{
				pgFindWord(m_paigeRef, walker.current_offset, &sel.begin, &sel.end, TRUE, FALSE);

				//	If the word matches, add it to the list to mark not mispelled
				if ( m_pView->StrPgCmp(in_szWord, sel) == 0 )
					markNotMispelledList.push_back(sel);
			}
		} while ( pgWalkNextStyle(&walker) );
		
		//	We're done walking, free the memory
		pgPrepareStyleWalk(pgr, 0, NULL, FALSE); 

		if ( !markNotMispelledList.empty() )
		{
			vector<select_pair>::iterator	i;

			//	Run through the list and mark the words not mispelled
			for (i = markNotMispelledList.begin(); i != markNotMispelledList.end(); i++)
				style.ApplyMisspelled(FALSE, &(*i), false);

			//	Make sure we update eventually
			m_pView->Invalidate();
		}

		UnuseMemory(m_paigeRef);
	}
}


#define FIRST_COMMAND_ID 100

int CSpellPaige::Popup(CPaigeEdtView* pView, POINT& point, select_pair& sel)
{
	m_pView = pView;
	m_paigeRef = pView->m_paigeRef;

	// Create the spell dialog dynamically if necessary
	if ( m_pSpellDialog == NULL )
	{
		m_pSpellDialog = DEBUG_NEW CSpellDialog( this );

		// We just had to create the dialog - the ref count should be 0.
		ASSERT(m_nSpellDialogRefCount == 0);
		m_nSpellDialogRefCount = 0;
	}

    if ( m_pSpellDialog == NULL )
    {
        // the dialog could not be successfully created.
        return -1;
    }

	// Make sure we don't delete the dialog (due to focus changing or whatever)
	// while we're working here.
	m_nSpellDialogRefCount++;

	//
	// Convert client Paige coords to screen
	//
	POINT pt;
	pt.x = point.x;
	pt.y = point.y;
	m_pView->ClientToScreen(&pt);
	//
	// Hilight the misspelled word and copy it into a buffer
	//
	char probWord[256];
	m_pView->GetPgText(probWord, sizeof(probWord), sel );
	//
	// Let the spell checker put up a popup of suggestions
	//

	//First check if it is a doubled word.
	BOOL	doubledWord = FALSE;
	int		nLength = sel.end - sel.begin;

	if (m_pSpellDialog->GetOptionsMask() & SpellCheck_REPORT_DOUBLED_WORD_OPT)
	{
		if (sel.begin != 0)
		{
			select_pair doubledSel;
			pgFindWord(m_paigeRef, sel.begin - 1, &doubledSel.begin, &doubledSel.end, TRUE, FALSE); 

			//If words are of the same length, possible double words
			if (nLength == (doubledSel.end - doubledSel.begin))
			{
				if ( (sel.begin != doubledSel.begin) && ( m_pView->StrPgCmp(probWord,doubledSel) == 0 ) )
					doubledWord = TRUE;
			}
		}
	}

	// If word starts with single quote - remove it so that we can offer the
	// correct suggested words, but leave the selection alone to keep the
	// delete, replace, etc. logic simpler (otherwise we'll do the wrong
	// thing with the single quote both during the operation and when
	// undoing).
	if ( (nLength > 0) && (*probWord == '\'') )
	{
		memcpy(probWord, probWord+1, nLength-1);
		nLength--;
		probWord[nLength] = 0;
	}

	int ret;
	if ( ret=m_pSpellDialog->Popup(probWord,point, doubledWord) )
	{
		CPaigeStyle style(m_paigeRef);
		style.ApplyMisspelled(FALSE, NULL, false);

		//	Make sure we update eventually
		m_pView->Invalidate();
		
		if ( (ret == COMMAND_ID_IGNOREALL) || (ret == COMMAND_ID_ADD) )
		{
			//
			// Add this word to the proper spelling dictionary
			//
			if ( ret == COMMAND_ID_IGNOREALL )
				m_pSpellDialog->IgnoreAll((unsigned char*)probWord);
			else
				m_pSpellDialog->AddToLex((unsigned char*)probWord);

			if (ret == COMMAND_ID_ADD)
			{
				//	Broadcast the add to all CCompMessageFrames, including our own
				g_theSpellingDirector.NotifyClients(NULL, CA_SPELLING_ADD_WORD, probWord);
			}
			else
			{
				//	Just mark the word not misspelled here. Unfortunately there's no
				//	easy was to make the subject header also reflect this change
				//	since it doesn't currently support ignore.
				MarkWordNotMisspelled(pView, probWord);
			}

			// Update immediately
			m_pView->UpdateWindow();
		}
		else if (ret == COMMAND_ID_DELETE_WORD)
		{
			long charInfo = pgCharType(m_paigeRef, sel.end, BLANK_BIT | TAB_BIT | PAR_SEL_BIT);

			if ( charInfo & (BLANK_BIT | TAB_BIT | PAR_SEL_BIT) )
			{
				sel.end += 1;
				pgSetSelection(m_paigeRef, sel.begin, sel.end, 0, FALSE);
			}

            m_pView->CleanURLonPosteriorDelete(sel.begin);

            m_pView->PrepareUndo(undo_delete);

			pgDelete(m_paigeRef, &sel, best_way);
			m_pView->UpdateChangeState( CPaigeEdtView::kAddChange );
		}
		else 
		{
            m_pView->PrepareUndo(undo_delete, (long *)&sel, TRUE);

			pgDelete(m_paigeRef, &sel, best_way);
			
			long	nStringLength = strlen(probWord);
			long	insert_ref[2];

			insert_ref[0] = sel.begin;
			insert_ref[1] = nStringLength;

			// Store undo information
			m_pView->PrepareUndo(undo_spelling_replace, insert_ref);
			
			//
			// Replace the Word
			//		
			pgInsert( m_paigeRef, (pg_byte_ptr)probWord, nStringLength, CURRENT_POSITION, 
							data_insert_mode, 0, best_way );

			m_pView->UpdateChangeState( CPaigeEdtView::kAddChange );
		}
	}

	// We're done with the dialog now
	m_nSpellDialogRefCount--;

	return 0;
}
