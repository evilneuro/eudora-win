// PaigeEdtView.cpp : implementation of the CPaigeEdtView class
//

/*
ID_EDIT_TEXT_FONT
ID_BLKFMT_BULLETTED_LIST
ID_EDIT_INSERT_LINK
*/

#include "stdafx.h"

#include <afxodlgs.h>		// For Paste Special

#include "resource.h"
#include "guiutils.h"
#include "utils.h"

#include "rs.h"

//#include "PaigeEdtDoc.h"

// For all text functions
#include "pgtraps.h"
#include "pgscrap.h"

// For Import, Export of files in Text, RTF, Native Paige
// and HTML formats
#include "pgTxrCPP.h"

// paige utility routines
#include "pgutils.h"

// high-level style interface
#include "pghlevel.h"

// Paige html definitions
#include "pghtmdef.h"

// Paige hyperlinks
#include "pghtext.h"

// machine specific
#include "machine.h"
#include "pgosutl.h"

// Import/Export == included in PaigeEdtView.h
#include "Paige_io.h"

//#include "pgtables.h"

// eudora font info
#include "font.h"
#include "FaceNameDialog.h"

// line glitter functions
#include "defprocs.h"
#include "pgdefpar.h"
#include "pggrafx.h"

#include "spell.h"
#include "PaigeEdtView.h"

#include "PgStyleUtils.h"
#include "spellpaige.h"
#include "paige.h"

#include "QCRecipientDirector.h"
//#include "TableDlgs.h"

#include "address.h"
#include "PaigePreviewDC.h"

#include "PaigeStyle.h"
#include "pgStyleUtils.h"
#include "msgutils.h"
#include "COLORLIS.H"
#include "SafetyPal.h"
#include "pgStyleUtils.h"
#include "PgEmbeddedImage.h"
#include "InsertLinkDialog.h"
#include "Text2Html.h"
#include "mainfrm.h"
#include "QCSharewareManager.h"

#include "wininet.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

void FindWord(pg_ref pg, long offset, long PG_FAR *first_byte,
              long PG_FAR *last_byte, pg_boolean left_side, pg_boolean smart_select);
long FindNextWhitespace(pg_ref pg, long offset, long end);
long SkipWhiteSpaces(pg_ref pg, long offset, long end);
long FindValidURLStart(pg_ref pg, long offset, long end, char &startChar);
long FindValidURLEnd(pg_ref pg, long offset, long end, long begin, char startChar);
//theStr is a IN-OUT parameter. If the scheme is www. or ftp. then 
//http:// or ftp:// are appended to the beginning of the string
BOOL ValidURLScheme(char * theStr);
bool InDifferentWord(pg_ref paigeRef, select_pair_ptr oldSel);
bool GetWordContainingSelection(pg_ref paigeRef,select_pair_ptr theWord);
bool SelPairIntersects(select_pair_ptr sel1, select_pair_ptr sel2);


static  graf_device m_device;


// OG
#define IDS_PG_BORDER "10,5,2,0"

#define MOUSE_DOWN 0x1
#define MOUSE_TRIPLE_CLICKED 0x2
#define MOUSE_DRAG_SELECT 0x4
#define MOUSE_DRAG_DROP 0x8

#define AST_INTERVAL 100    // auto spell check timer interval (mSec)

enum {TRIPLE_CLICK_TIMER = 100, DRAG_DROP_SCROLL_TIMER, DRAG_SELECT_TIMER, AUTO_SPELL_TIMER };
BOOL g_bInteractiveSpellCheck = FALSE;

extern QCRecipientDirector g_theRecipientDirector;

extern QCRecipientDirector g_theRecipientDirector;
extern bool g_qtInitialized;
extern bool g_bOverwriteMode;


/////////////////////////////////////////////////////////////////////////////
// CPaigeEdtView

IMPLEMENT_DYNCREATE(CPaigeEdtView, CView)

    BEGIN_MESSAGE_MAP(CPaigeEdtView, CView)
    //{{AFX_MSG_MAP(CPaigeEdtView)
    ON_WM_SETCURSOR()
    ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_WM_SIZE()
    ON_WM_LBUTTONDOWN()
    ON_WM_RBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_WM_CHAR()
    ON_WM_KEYDOWN()
    ON_WM_LBUTTONDBLCLK()
    ON_WM_KILLFOCUS()
    ON_WM_SETFOCUS()
//      ON_MESSAGE(WM_GETFONT,OnGetFont)
//      ON_MESSAGE(WM_SETFONT,OnSetFont)
    ON_MESSAGE(WM_SYSCOLORCHANGE,OnSysColorChange)
    ON_WM_VSCROLL()
    ON_WM_HSCROLL()
    ON_WM_MOUSEWHEEL()
    ON_COMMAND(ID_EDIT_TEXT_FONT, OnFormatFont)
    ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_FONT, OnUpdateFormatFont)
    ON_WM_ERASEBKGND()
    ON_WM_TIMER()
    ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
    ON_COMMAND(ID_EDIT_CUT, OnEditCut)
    ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
    ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
    ON_COMMAND(ID_EDIT_PASTE_SPECIAL, OnEditPasteSpecial)
    ON_COMMAND(WM_UNDO, OnEditUndo)
    ON_COMMAND(WM_CUT, OnEditCut)
    ON_COMMAND(WM_COPY, OnEditCopy)
    ON_COMMAND(WM_PASTE, OnEditPaste)
    ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
    ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
    ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
    ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
    ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE_SPECIAL, OnUpdateEditPaste)
    ON_UPDATE_COMMAND_UI(WM_UNDO, OnUpdateEditUndo)
    ON_UPDATE_COMMAND_UI(WM_CUT, OnUpdateEditCut)
    ON_UPDATE_COMMAND_UI(WM_COPY, OnUpdateEditCopy)
    ON_UPDATE_COMMAND_UI(WM_PASTE, OnUpdateEditPaste)
    ON_COMMAND(ID_EDIT_PASTEASQUOTATION, OnEditPasteAsQuotation)
    ON_UPDATE_COMMAND_UI(ID_EDIT_PASTEASQUOTATION, OnUpdateEditPaste)
    ON_COMMAND(ID_EDIT_TEXT_BOLD, OnTextBold)
    ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_BOLD, OnUpdateTextBold)
    ON_COMMAND(ID_EDIT_TEXT_ITALIC, OnTextItalic)
    ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_ITALIC, OnUpdateTextItalic)
    ON_COMMAND(ID_EDIT_TEXT_UNDERLINE, OnTextUnderline)
    ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_UNDERLINE, OnUpdateTextUnderline)
    ON_COMMAND(ID_EDIT_TEXT_PLAIN, OnTextPlain)
    ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_PLAIN, OnUpdateTextPlain)
    ON_COMMAND(ID_EDIT_TEXT_INDENT_IN, OnIndentIn)
    ON_COMMAND(ID_EDIT_TEXT_INDENT_OUT, OnIndentOut)
    ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_INDENT_OUT, OnUpdateIndentOut)
    ON_COMMAND(ID_EDIT_TEXT_CENTER, OnParaCenter)
    ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_CENTER, OnUpdateParaCenter)
    ON_COMMAND(ID_EDIT_TEXT_LEFT, OnParaLeft)
    ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_LEFT, OnUpdateParaLeft)
    ON_COMMAND(ID_EDIT_TEXT_RIGHT, OnParaRight)
    ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_RIGHT, OnUpdateParaRight)
    ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
    ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
    ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
    ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ALL, OnUpdateEditSelectAll)
    //ON_COMMAND(ID_EDIT_TABLE_INSERTTABLE, OnEditInsertTable)
    //ON_UPDATE_COMMAND_UI(ID_EDIT_TABLE_INSERTTABLE, OnUpdateEditInsertTable)
    //ON_COMMAND(ID_EDIT_TABLE_DELETETABLE, OnEditDeleteTable)
    //ON_UPDATE_COMMAND_UI(ID_EDIT_TABLE_DELETETABLE, OnUpdateEditDeleteTable)
    ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_INDENT_IN, OnUpdateIndentIn)
    ON_COMMAND( ID_SPECIAL_ADDASRECIPIENT, OnSpecialAddAsRecipient )
    ON_UPDATE_COMMAND_UI( ID_SPECIAL_ADDASRECIPIENT, OnUpdateAddAsRecipient )
    ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
    ON_UPDATE_COMMAND_UI(ID_EDIT_INSERT, OnUpdateEditStyle)
    ON_COMMAND(ID_EDIT_INSERT_PICTURE, OnInsertPicture)
    ON_UPDATE_COMMAND_UI(ID_EDIT_INSERT_PICTURE, OnUpdateInsertPicture)
    ON_COMMAND(ID_EDIT_INSERT_HR, OnEditInsertHR)
    ON_UPDATE_COMMAND_UI(ID_EDIT_INSERT_HR, OnUpdateEditInsertHR)
    ON_COMMAND(ID_EDIT_WRAPSELECTION, OnEditWrapSelection)
    ON_UPDATE_COMMAND_UI(ID_EDIT_WRAPSELECTION, OnUpdateNeedSelEdit)
    ON_COMMAND(ID_EDIT_TEXT_TT, OnBlkfmtFixedwidth)
    ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_TT, OnUpdateBlkfmtFixedwidth)
    ON_COMMAND(ID_FILE_OPEN_ATTACHMENT, OnOpenAttachment)
    ON_UPDATE_COMMAND_UI(ID_FILE_OPEN_ATTACHMENT, OnUpdateOpenAttachment)
    ON_COMMAND(ID_EDIT_TEXT_ADD_QUOTE, OnAddQuote)
    ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_ADD_QUOTE, OnUpdateAddQuote)
    ON_COMMAND(ID_EDIT_TEXT_REMOVE_QUOTE, OnRemoveQuote)
    ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_REMOVE_QUOTE, OnUpdateRemoveQuote)
    //}}AFX_MSG_MAP

    ON_CBN_SELENDOK( IDC_FONT_COMBO, OnFontChange )
    ON_UPDATE_COMMAND_UI( IDC_FONT_COMBO, OnUpdateFontCombo )
    ON_COMMAND( ID_EDIT_TEXT_CLEAR, OnClearFormatting )
    ON_UPDATE_COMMAND_UI( ID_EDIT_TEXT_CLEAR, OnUpdateClearFormatting )
    ON_COMMAND_RANGE(ID_EDIT_TEXT_SIZE_VERY_SMALL, ID_EDIT_TEXT_SIZE_HUMONGOUS, OnTextSizeChange)
    ON_UPDATE_COMMAND_UI_RANGE(ID_EDIT_TEXT_SIZE_VERY_SMALL, ID_EDIT_TEXT_SIZE_HUMONGOUS, OnUpdateEditStyle)
    ON_COMMAND_RANGE(ID_COLOR0, ID_COLOR16, OnColorChange)
    ON_UPDATE_COMMAND_UI_RANGE(ID_COLOR0, ID_COLOR16, OnUpdateEditStyle)
    ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_SIZE, OnUpdateEditStyle)
    ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_COLOR, OnUpdateEditStyle)
    ON_COMMAND(ID_EDIT_INSERT_LINK, OnEditInsertLink)
    ON_UPDATE_COMMAND_UI(ID_EDIT_INSERT_LINK, OnUpdateInsertLink)
    ON_COMMAND(ID_BLKFMT_BULLETTED_LIST, OnBlkfmtBullettedList)
    ON_UPDATE_COMMAND_UI(ID_BLKFMT_BULLETTED_LIST, OnUpdateBlkfmtBullettedList)
        
    //}}AFX_MSG_MAP

    END_MESSAGE_MAP()

    PG_PASCAL (void) myPostPaginateProc(paige_rec_ptr pg, text_block_ptr block,
                                        smart_update_ptr update_info, long lineshift_begin, long lineshift_end,
                                        short action_taken_verb)
{
    pgPostPaginateProc(pg, block,update_info, lineshift_begin, lineshift_end,
                       action_taken_verb);

    if ( (action_taken_verb == paginated_line_shift) ||
         (action_taken_verb == paginated_full_block) )
    {
        PgStuffBucket* pSB = (PgStuffBucket*) pg->user_refcon;

        if((pSB == NULL) || !pSB->pWndOwner->IsKindOf(RUNTIME_CLASS(CPaigeEdtView)))
        {
            ASSERT(TRUE);
            return;
        }
        //((CPaigeEdtView*)pSB->pWndOwner)->UpdateScrollBars();
    }
}

/////////////////////////////////////////////////////////////////////////////
// CPaigeEdtView construction/destruction

CPaigeEdtView::CPaigeEdtView()
{
    m_paigeRef = NULL;
    m_styleEx = NULL;
    m_mouseState = 0;
    m_pgModifiers = 0;
    m_mousePt = 0;
    m_bDontDeactivateHighlight=FALSE;
    //undoCount = redoCount = 0;
    m_fRO = false;
    m_bAllowStyled = true;

    m_chngCount = m_ccLastSaved = 0;
    m_bIncludeHeaderFooters     = GetIniShort(IDS_INI_PRINT_HEADERS);
    //memset( &m_thePrintInfo, 0, sizeof( m_thePrintInfo ) );
    m_thePrintInfo.m_printPgRef = 0;
    m_bFixedWidth = FALSE;
    bHasBeenInitiallyUpdated = FALSE;
    m_prevPoint.x = m_prevPoint.y = 0;
    m_pDirtyPair = NULL;
    last_typed = 0;
    m_checkdastuff = false;
}

CPaigeEdtView::~CPaigeEdtView()
{       

    // delete the header font
    if(m_thePrintInfo.m_pHeaderFont)
        delete m_thePrintInfo.m_pHeaderFont;
    m_thePrintInfo.m_pHeaderFont = NULL;

    // delete the pen
    if(m_thePrintInfo.m_pPen)
        delete m_thePrintInfo.m_pPen;
    m_thePrintInfo.m_pPen = NULL;
    delete m_styleEx;
}

BOOL CPaigeEdtView::PreCreateWindow(CREATESTRUCT& cs)
{
    //  the CREATESTRUCT cs

    return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CPaigeEdtView drawing

void CPaigeEdtView::OnDraw(CDC* pDC)
{
    pgErasePageArea( m_paigeRef, MEM_NULL );
    pgDisplay( m_paigeRef, NULL, MEM_NULL, MEM_NULL, NULL, bits_emulate_or );
}

/*
LRESULT CPaigeEdtView::OnGetFont(WPARAM wParam, LPARAM lParam)
{
        long                                    style_index;
        style_info                              style;
        par_info                                par;
        font_info                               font;
        if ( !(style_index = pgGetNamedStyleIndex (m_paigeRef, body_style)) )
                return NULL;

        pgGetNamedStyleInfo(m_paigeRef, style_index, &style, &font, &par);      

        return  (long)&MessageFont.m_hObject;
        

}

LRESULT CPaigeEdtView::OnSetFont(WPARAM wParam, LPARAM lParam)
{
        short styleID;
        if ( !(styleID = pgGetNamedStyleIndex (m_paigeRef, body_style)) )
                return TRUE;

        // Set the body style to have the MessageFont
        LOGFONT lf;
        font_info fontInfo;
        style_info styleInfo;
        style_info styleInfoMask;
        par_info parInfo;
        par_info parMask;
        if( GetIniShort( IDS_INI_USE_PROPORTIONAL_AS_DEFAULT ) )
                MessageFont.GetLogFont( &lf );
        else
                MessageFixedFont.GetLogFont( &lf );
        
        PgConvertLogFont( m_paigeRef, PgGlobalsPtr(), &lf, &fontInfo, &styleInfo, &styleInfoMask );
        pgChangeStyle(m_paigeRef, styleID, &styleInfo, &fontInfo, best_way);

        return TRUE;
}

*/
LRESULT CPaigeEdtView::OnSysColorChange(WPARAM wParam, LPARAM lParam)
{
    style_info styleInfo;
    //
    // Reset the style sheet for the body
    //  
    long styleID;
    if ( !(styleID = pgGetNamedStyleIndex (m_paigeRef, body_style)) )
        return TRUE;

    pgFillBlock( &styleInfo, sizeof(style_info), 0 );
    pgGetStyle(m_paigeRef, (short)styleID, &styleInfo);

    // set default paige text color
    DWORD fgColor = GetSysColor( COLOR_WINDOWTEXT );
    pgOSToPgColor( &fgColor, &styleInfo.fg_color );

    DWORD bkColor = GetSysColor( COLOR_WINDOW );
    pgOSToPgColor( &bkColor, &styleInfo.bk_color );
        
    pgChangeStyle(m_paigeRef, (short)styleID, &styleInfo, NULL /*&fontInfo*/, best_way);
    pgSetDefaultStyle(m_paigeRef, &styleInfo, best_way);
    //
    // Modify the pgRec
    //
    paige_rec_ptr pgRecPtr = (paige_rec_ptr)UseMemory(m_paigeRef);
    pgOSToPgColor( &bkColor, &(pgRecPtr->bk_color));
    UnuseMemory(m_paigeRef);
    // 
    // Modify the Globals.  This happens for every open paige view
    // which is bad. Maybe later, centratlize this call by intercepting
    // it in Mainfrm and doing it once for all open paige views.
    //
    styleInfo = PgGlobalsPtr()->def_style;
    pgOSToPgColor( &fgColor, &styleInfo.fg_color );
    pgOSToPgColor( &bkColor, &styleInfo.bk_color );
        
    pgOSToPgColor( &bkColor, &(PgGlobalsPtr()->def_bk_color));
    memcpy( &(PgGlobalsPtr())->def_style, &styleInfo, sizeof(style_info) );

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CPaigeEdtView printing

BOOL CPaigeEdtView::OnPreparePrinting(CPrintInfo* pInfo)
{
    //If a selection exists, enable the Selection radio button in the
    //Print dialog
	if ( pgNumSelections(m_paigeRef) && !(pInfo->m_bPreview) )
    {
        pInfo->m_pPD->m_pd.Flags &= ~PD_NOSELECTION;
    }

    CDC *pDC =  GetDC( );
    UINT        uOldMappingMode;   
        
    // save the mapping mode
    uOldMappingMode = pDC->SetMapMode( MM_TEXT );

    // get the x & y resolutions
    m_thePrintInfo.m_ulXScreenRes = pDC->GetDeviceCaps( LOGPIXELSX );
    m_thePrintInfo.m_ulYScreenRes = pDC->GetDeviceCaps( LOGPIXELSY );

    pDC->SetMapMode(uOldMappingMode);
    ReleaseDC(pDC);

        
    return DoPreparePrinting(pInfo);
}

BOOL CPaigeEdtView::QuietPrint()
{
    // get default print info
    CPrintInfo printInfo;
    BOOL bError = FALSE;

    ASSERT(printInfo.m_pPD != NULL);    // must be set


    printInfo.m_bDirect = TRUE;

    if (OnPreparePrinting(&printInfo))
    {
        // hDC must be set (did you remember to call DoPreparePrinting?)
        ASSERT(printInfo.m_pPD->m_pd.hDC != NULL);

                // set up document info and start the document printing process
        CString strTitle;
        CDocument* pDoc = GetDocument();
        if (pDoc != NULL)
            strTitle = pDoc->GetTitle();
        else
            GetParentFrame()->GetWindowText(strTitle);
        
        if (strTitle.GetLength() > 31)
            strTitle.ReleaseBuffer(31);
                
        DOCINFO docInfo;
        memset(&docInfo, 0, sizeof(DOCINFO));
        docInfo.cbSize = sizeof(DOCINFO);
        docInfo.lpszDocName = strTitle;
                
        CString strPortName;
                
        docInfo.lpszOutput = NULL;
        strPortName = printInfo.m_pPD->GetPortName();
                
        // setup the printing DC
        CDC dcPrint;
        dcPrint.Attach(printInfo.m_pPD->m_pd.hDC);  // attach printer dc
        dcPrint.m_bPrinting = TRUE;
        OnBeginPrinting(&dcPrint, &printInfo);
                
                // start document printing process
        if (dcPrint.StartDoc(&docInfo) == SP_ERROR)
        {
            // enable main window before proceeding
            AfxGetMainWnd()->EnableWindow(TRUE);

                        // cleanup and show error message
            OnEndPrinting(&dcPrint, &printInfo);
            dcPrint.Detach();   // will be cleaned up by CPrintInfo destructor
            AfxMessageBox(AFX_IDP_FAILED_TO_START_PRINT);
            return FALSE;
        }

        // Guarantee values are in the valid range
        UINT nEndPage = printInfo.GetToPage();
        UINT nStartPage = printInfo.GetFromPage();

        if (nEndPage < printInfo.GetMinPage())
            nEndPage = printInfo.GetMinPage();
        if (nEndPage > printInfo.GetMaxPage())
            nEndPage = printInfo.GetMaxPage();

        if (nStartPage < printInfo.GetMinPage())
            nStartPage = printInfo.GetMinPage();
        if (nStartPage > printInfo.GetMaxPage())
            nStartPage = printInfo.GetMaxPage();

        int nStep = (nEndPage >= nStartPage) ? 1 : -1;
        nEndPage = (nEndPage == 0xffff) ? 0xffff : nEndPage + nStep;


        for (printInfo.m_nCurPage = nStartPage;
             printInfo.m_nCurPage != nEndPage; printInfo.m_nCurPage += nStep)
        {
            OnPrepareDC(&dcPrint, &printInfo);

            // check for end of print
            if (!printInfo.m_bContinuePrinting)
                break;

            // set up drawing rect to entire page (in logical coordinates)
            printInfo.m_rectDraw.SetRect(0, 0,
                                         dcPrint.GetDeviceCaps(HORZRES),
                                         dcPrint.GetDeviceCaps(VERTRES));
            dcPrint.DPtoLP(&printInfo.m_rectDraw);

            // attempt to start the current page
            if (dcPrint.StartPage() < 0)
            {
                bError = TRUE;
                break;
            }

            // must call OnPrepareDC on newer versions of Windows because
            // StartPage now resets the device attributes.
            OnPrepareDC(&dcPrint, &printInfo);

            ASSERT(printInfo.m_bContinuePrinting);

            // page successfully started, so now render the page
            OnPrint(&dcPrint, &printInfo);
            if (dcPrint.EndPage() < 0 )
            {
                bError = TRUE;
                break;
            }
        }

        // cleanup document printing process
        if (!bError)
            dcPrint.EndDoc();
        else
            dcPrint.AbortDoc();

        OnEndPrinting(&dcPrint, &printInfo);    // clean up after printing
                
        dcPrint.Detach();   // will be cleaned up by CPrintInfo destructor

    }
    return !bError;
}

//OnBeginPrinting calcualtes all of the rectangles for printing and max num of pages
void CPaigeEdtView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
    TEXTMETRIC  theTM;
    CFont*              pOldFont;
    ULONG               ulXRes;
    ULONG               ulYRes;
    ULONG               ulPageWidthPixels;
    ULONG               ulPageHeightPixels;
    ULONG               ulOneHalfInchX;
    ULONG               ulOneHalfInchY;
    ULONG               ulThreeQuartersInchX;
    ULONG               ulThreeQuartersInchY;
    rectangle   page_rect;

    SetRedraw( FALSE );

    // call the base class
    CView::OnBeginPrinting(pDC,pInfo); 

    // NOTE:  Paige prints the document with the current font of the document (wsywig).  Unfortunately, Eudora
    // has seperate fonts for messages and for printing which means the font must be changed in the doc to the printer
    // font and then switched back....yuk :0

    HDC hDC = 0;
    hDC = pDC->GetSafeHdc();

    //Set the print device for the original pg_ref only if we are printing the whole doc
    //For printing a selection, we copy the highlighted text into a temp pg_ref and print that.
    if (! (pInfo->m_pPD->m_pd.Flags & PD_SELECTION) )
        pgSetPrintDevice(m_paigeRef, (generic_var)hDC);

    //Incase we print the selection, this is the temp pg_ref we will use
    m_thePrintInfo.m_printPgRef = 0;

    CString msgFont;
    if( GetIniShort( IDS_INI_USE_PROPORTIONAL_AS_DEFAULT ) )
        msgFont = GetIniString(IDS_INI_MESSAGE_FONT);
    else
        msgFont = GetIniString(IDS_INI_MESSAGE_FIXED_FONT);
    FontSizeMapType& FontSizeMap = GetFontSizeMap();
    int msgFontSize = FontSizeMap[(int)GetIniShort(IDS_INI_MESSAGE_FONT_SIZE)][2];

    //Set the font to the printer font and size
    CString strName = GetIniString(IDS_INI_PRINTER_FONT);
    int fontSize = FontSizeMap[(int)GetIniShort(IDS_INI_PF_SIZE)][2];

    BOOL fontsDiff = (stricmp(msgFont, strName) == 0) ? FALSE : TRUE;
    BOOL sizesDiff = msgFontSize == fontSize ? FALSE : TRUE;

    pgGetSelection(m_paigeRef, &m_thePrintInfo.m_currSelection.begin, &m_thePrintInfo.m_currSelection.end);

    if (fontsDiff || sizesDiff)
    {
        long styleID;
        styleID = pgGetNamedStyleIndex (m_paigeRef, body_style);
        ASSERT(styleID);

        style_info printerStyle;

        pgFillBlock( &printerStyle, sizeof(style_info), 0 );

        pgGetStyle(m_paigeRef, (short)styleID, &printerStyle);
        printerStyle.point = fontSize;

        font_info printerFont;
        if (fontsDiff)
        {
            //font_info printerFont;
            style_info styleInfo, styleInfoMask;
                        
            pgFillBlock( &printerFont, sizeof(font_info), 0 );
                        
            extern CFontInfo PrinterFont;
            LOGFONT lf;
            PrinterFont.GetLogFont(&lf);
            PgConvertLogFont(m_paigeRef, PgGlobalsPtr(), &lf, &printerFont, &styleInfo, &styleInfoMask );

            if ( lf.lfWeight == FW_BOLD )
                printerStyle.styles[bold_var] = -1;
            if ( lf.lfItalic )
                printerStyle.styles[italic_var] = -1;
            if ( lf.lfUnderline )
                printerStyle.styles[underline_var] = -1;
            if ( lf.lfStrikeOut )
                printerStyle.styles[strikeout_var] = -1;
        }

        if (sizesDiff)
        {
            //Change the relative point sizes first
            select_pair sel;
            int currSize, newSize;
            int msgBase   = GetIniShort(IDS_INI_MESSAGE_FONT_SIZE);
            int printBase = GetIniShort(IDS_INI_PF_SIZE);
			long start    = 0;
			long end      = 0;

            if (pInfo->m_pPD->m_pd.Flags & PD_SELECTION)
            {
                start = m_thePrintInfo.m_currSelection.begin;
                end   = m_thePrintInfo.m_currSelection.end;
            }

            style_walk walker;
            paige_rec_ptr pgRecPtr = (paige_rec_ptr)UseMemory(m_paigeRef);
            pgPrepareStyleWalk(pgRecPtr, start, &walker, false);

			if ( !end )
				end = walker.t_length;

			for ( bool bDoneWalking = false;
					!bDoneWalking && (walker.current_offset < end); /*noop*/ )
			{
				sel.begin = walker.current_offset;
                currSize = walker.cur_style->point;

                for ( ;; )
				{
					if ( pgWalkNextStyle( &walker ) )
					{
						if ( walker.cur_style->point != currSize )
						{
							// we got a style change with a different point size. as long as the
							// current offset isn't past the end of a selection, it ends this
							// run of same-sized text---otherwise just do the selection.

							sel.end = (walker.current_offset < end) ? walker.current_offset : end;
							break;
						}
					}
					else
					{
						// there are no styles left to walk, "end" points at end of document
						// or end of selection---it will *never* exceed t_length.
						assert( end <= walker.t_length );

						sel.end = end;
						bDoneWalking = true;
						break;
					}
				}

                m_thePrintInfo.m_offsetsQueue.push(sel);

				// Find the font index and set the relative point size for the new index.
				// The order may seem a little strange, but it's a way to prefer
				// that matches occur closer toward the default font size.

				if (currSize == FontSizeMap[msgBase][2])
					newSize = FontSizeMap[printBase][2];
				else if (currSize == FontSizeMap[msgBase][1])
					newSize = FontSizeMap[printBase][1];
				else if (currSize == FontSizeMap[msgBase][0])
					newSize = FontSizeMap[printBase][0];
				else if (currSize == FontSizeMap[msgBase][3])
					newSize = FontSizeMap[printBase][3];
				else if (currSize == FontSizeMap[msgBase][4])
					newSize = FontSizeMap[printBase][4];
				else if (currSize == FontSizeMap[msgBase][5])
					newSize = FontSizeMap[printBase][5];
				else if (currSize == FontSizeMap[msgBase][6])
					newSize = FontSizeMap[printBase][6];
				else
				{
					// This happens with the strange case where specifying 11 point really
					// generates 12 point, so we don't wind up finding a match in the font
					// size map.  Just assume the default size for lack of a better choice.

					ASSERT(0);
					newSize = FontSizeMap[printBase][2];
				}

                m_thePrintInfo.m_fontSizeQueue.push(newSize);
            }

            pgPrepareStyleWalk(pgRecPtr, 0, NULL, false);
            UnuseMemory(m_paigeRef);

            //Set up the undo stack
            int count = m_thePrintInfo.m_fontSizeQueue.size();
            for (int i = 0; i < count; i++)
            {
                sel = m_thePrintInfo.m_offsetsQueue.front();
                m_thePrintInfo.m_offsetsQueue.pop();
                pgSetSelection(m_paigeRef, sel.begin, sel.end, 0, FALSE);
                m_thePrintInfo.m_printUndoStack.push( pgPrepareUndo(m_paigeRef, undo_format, NULL) );
                pgSetPointSize( m_paigeRef, (short)(m_thePrintInfo.m_fontSizeQueue.front() >> 16), &sel, FALSE );
                m_thePrintInfo.m_fontSizeQueue.pop();
            }
        }

        pgSetSelection(m_paigeRef, m_thePrintInfo.m_currSelection.begin, m_thePrintInfo.m_currSelection.end, 0, FALSE);
        m_thePrintInfo.m_printUndoStack.push(pgPrepareUndo(m_paigeRef, undo_format, (void PG_FAR *)MEM_NULL));
        if (fontsDiff)
        {
            pgChangeStyle(m_paigeRef, (short)styleID, &printerStyle, &printerFont, draw_none);
        }
        else
        {
            //Only the point size is different
            pgChangeStyle(m_paigeRef, (short)styleID, &printerStyle, MEM_NULL, draw_none);
        }
    }

    if (pInfo->m_pPD->m_pd.Flags & PD_SELECTION)
    {
        //Copy the text into a temporary pg_ref for printing
        m_thePrintInfo.m_printPgRef = pgCopy(m_paigeRef, &(m_thePrintInfo.m_currSelection));
        //Set the print device for the temporary pg_ref
        pgSetPrintDevice(m_thePrintInfo.m_printPgRef, (generic_var)hDC);
    }
        
    // save the mapping mode
    m_thePrintInfo.m_uOldMappingMode = pDC->SetMapMode( MM_TEXT );

    // get the x & y resolutions
    ulXRes = pDC->GetDeviceCaps( LOGPIXELSX );
    ulYRes = pDC->GetDeviceCaps( LOGPIXELSY );

    // get the size of the page in pixels
    ulPageWidthPixels = pDC->GetDeviceCaps( HORZRES );
    ulPageHeightPixels = pDC->GetDeviceCaps( VERTRES );
        
    // save some handy values
    ulOneHalfInchX = ( ulXRes / 2 );
    ulOneHalfInchY = ( ulYRes / 2 );
    ulThreeQuartersInchX = ( ulXRes * 3UL ) / 4;
    ulThreeQuartersInchY = ( ulYRes * 3UL ) / 4;
                                         
    // get the body rect
    m_thePrintInfo.m_theBodyRect.left = ( int ) ulThreeQuartersInchX;
    m_thePrintInfo.m_theBodyRect.right = ( int ) ( ulPageWidthPixels - ulThreeQuartersInchX );
        
    if ( !m_bIncludeHeaderFooters || (pInfo->m_pPD->m_pd.Flags & PD_SELECTION) )
    {
        m_thePrintInfo.m_theBodyRect.top = ( int ) ulThreeQuartersInchY;
        m_thePrintInfo.m_theBodyRect.bottom = ( int ) ( ulPageHeightPixels - ulThreeQuartersInchY );
    }
    else
    {
        // create the header font 
        m_thePrintInfo.m_pHeaderFont = new CFontInfo(FW_BOLD);
                
        if ( m_thePrintInfo.m_pHeaderFont != NULL )
        {
            // Lets get the user demanded font info
            int hdrFontSize = fontSize >> 16;
            m_thePrintInfo.m_pHeaderFont->CreateFont(strName, hdrFontSize, pDC);
        }
        
        // create the pen
        m_thePrintInfo.m_lPenWidth = ulYRes / 64; // 1/64 inch in pixels 
        m_thePrintInfo.m_pPen = new CPen( PS_SOLID, ( int ) ( m_thePrintInfo.m_lPenWidth ), RGB( 0, 0, 0 ) );

        if ( m_thePrintInfo.m_pHeaderFont != NULL )
        {
            // select the font
            pOldFont = pDC->SelectObject( m_thePrintInfo.m_pHeaderFont );
        }
        
        // get the metrics
        pDC->GetTextMetrics( &theTM );
                
        // get the position for the header text in pixels
        m_thePrintInfo.m_HeaderTextRect.top = ( int ) ulOneHalfInchY;
        m_thePrintInfo.m_HeaderTextRect.bottom = ( int )(       m_thePrintInfo.m_HeaderTextRect.top + 
                                                                theTM.tmHeight + 
                                                                theTM.tmExternalLeading );
        m_thePrintInfo.m_HeaderTextRect.left = ( int ) ulOneHalfInchX;
        m_thePrintInfo.m_HeaderTextRect.right = ( int ) ( ulPageWidthPixels - ulOneHalfInchX );

        // get the position for the footer text
        m_thePrintInfo.m_FooterTextRect.bottom = ( int )( ulPageHeightPixels - ulOneHalfInchY );
        m_thePrintInfo.m_FooterTextRect.top =   ( int )(        m_thePrintInfo.m_FooterTextRect.bottom - 
                                                                theTM.tmHeight );
        m_thePrintInfo.m_FooterTextRect.left = ( int ) m_thePrintInfo.m_HeaderTextRect.left;
        m_thePrintInfo.m_FooterTextRect.right = ( int ) m_thePrintInfo.m_HeaderTextRect.right;

                
        // reset the font
        if ( m_thePrintInfo.m_pHeaderFont != NULL )
        {
            // select the font
            pDC->SelectObject( pOldFont );
        }
                                        
        // set the top margin in pixels
        m_thePrintInfo.m_theBodyRect.top = ( int )(     m_thePrintInfo.m_HeaderTextRect.bottom +
                                                        ( 2 * m_thePrintInfo.m_lPenWidth ) + 
                                                        ( ulOneHalfInchY / 2 ) );

        m_thePrintInfo.m_theBodyRect.bottom = ( int ) ( ulPageHeightPixels - m_thePrintInfo.m_theBodyRect.top );
    }

    ////////////////////////////////////////////////////////////////
        // Need to calculate the body rect in the current resolution...



        double bDub = ((double)((double)m_thePrintInfo.m_theBodyRect.left / (double)ulXRes));

        //Convert to the screen resolution...for Paige
        m_thePrintInfo.m_theBodyRect.left = (int) ((bDub)* (double)m_thePrintInfo.m_ulXScreenRes);
        
        bDub = ((double)((double)m_thePrintInfo.m_theBodyRect.right / (double)ulXRes));
        m_thePrintInfo.m_theBodyRect.right = (int)(bDub  * (double)m_thePrintInfo.m_ulXScreenRes);
        
        bDub = ((double)((double)m_thePrintInfo.m_theBodyRect.top / (double)ulYRes));
        m_thePrintInfo.m_theBodyRect.top = (int)(bDub * (double)m_thePrintInfo.m_ulYScreenRes);
        
        bDub = ((double)((double)m_thePrintInfo.m_theBodyRect.bottom / (double)ulYRes));
        m_thePrintInfo.m_theBodyRect.bottom = (int)(bDub * (double)m_thePrintInfo.m_ulYScreenRes);

        //Now convert the rect to a page_rect and use this calculate the number of pages
        RectToRectangle(&m_thePrintInfo.m_theBodyRect, &page_rect);
        
        //Get and then set the number of pages
        if(!m_bFixedWidth)
        {
            shape_ref newArea;
            newArea = pgRectToShape(PgMemGlobalsPtr(), &page_rect);
        
            m_thePrintInfo.visArea = pgRectToShape(PgMemGlobalsPtr() , NULL);
            m_thePrintInfo.pageArea = pgRectToShape(PgMemGlobalsPtr() , NULL);

            pgGetAreas (m_paigeRef, m_thePrintInfo.visArea, m_thePrintInfo.pageArea, MEM_NULL);

            if (pInfo->m_pPD->m_pd.Flags & PD_SELECTION)
                pgSetAreas(m_thePrintInfo.m_printPgRef, newArea, newArea, MEM_NULL);
            else
                pgSetAreas (m_paigeRef, newArea, newArea, MEM_NULL);

            pgDisposeShape(newArea);
        }

        SHORT sPages = 0;

        if (pInfo->m_pPD->m_pd.Flags & PD_SELECTION)
            sPages = pgNumPages(m_thePrintInfo.m_printPgRef, &page_rect);
        else
            sPages = pgNumPages(m_paigeRef, &page_rect);
        
        pInfo->SetMaxPage(sPages);
}

void CPaigeEdtView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
    CView::OnEndPrinting(pDC, pInfo);

    // sanity check the dc---if bad, we're just gonna bail & pray. otoh, in preview
    // mode the dc *is* supposed to be null, and does not cause bad stuff to happen
    // if we keep going. this printing code is NASTY!!

    if ( !pDC->m_hDC && !pInfo->m_bPreview )
        return;

    // delete the header font
    delete m_thePrintInfo.m_pHeaderFont;
    m_thePrintInfo.m_pHeaderFont = NULL;

    // delete the pen
    delete m_thePrintInfo.m_pPen;
    m_thePrintInfo.m_pPen = NULL;

    // reset the mapping mode
    pDC->SetMapMode( m_thePrintInfo.m_uOldMappingMode );

    int count = m_thePrintInfo.m_printUndoStack.size();
    if (count)
    {
        //Delete the font sizes undo stack
        for (int i = 0; i < count; i++)
        {
            pgUndo(m_paigeRef, (undo_ref)m_thePrintInfo.m_printUndoStack.top() , TRUE, best_way);
            m_thePrintInfo.m_printUndoStack.pop();
        }

        ASSERT(m_thePrintInfo.m_printUndoStack.size() == 0 );
        ASSERT(m_thePrintInfo.m_offsetsQueue.size() == 0 );
        ASSERT(m_thePrintInfo.m_fontSizeQueue.size() == 0 );
        //pgUndo(m_paigeRef, *m_thePrintInfo.m_pNewUndo , TRUE, best_way);
        //delete m_thePrintInfo.m_pNewUndo;
        //m_thePrintInfo.m_pNewUndo = NULL;
    }

    if(!m_bFixedWidth)
    {
        pgSetAreas (m_paigeRef, m_thePrintInfo.visArea, m_thePrintInfo.pageArea, MEM_NULL);
        pgDisposeShape(m_thePrintInfo.visArea);
        pgDisposeShape(m_thePrintInfo.pageArea);
    }

    if (pInfo->m_pPD->m_pd.Flags & PD_SELECTION)
    {
        pgSetPrintDevice(m_thePrintInfo.m_printPgRef, (generic_var)NULL);
        pgDispose(m_thePrintInfo.m_printPgRef);
        m_thePrintInfo.m_printPgRef = 0;
    }
    else
        pgSetPrintDevice(m_paigeRef, (generic_var)NULL);

    pgSetSelection(m_paigeRef, m_thePrintInfo.m_currSelection.begin, m_thePrintInfo.m_currSelection.end, 0, FALSE);
    SetRedraw( TRUE );
    Invalidate();
}

void CPaigeEdtView::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{
    CRect       theRect;
    rectangle page_rect;
        
    //Store the rectange Paige will use to print into
    RectToRectangle(&m_thePrintInfo.m_theBodyRect, &page_rect);
        

    pDC->SetMapMode( MM_TEXT );

    // print the page header and footer if necessary
    if (m_bIncludeHeaderFooters && !(pInfo->m_pPD->m_pd.Flags & PD_SELECTION))
    {
        DoPageHeader(pDC, pInfo);
        DoPageFooter(pDC, pInfo);
    }

        
    HDC hDC = 0;
    hDC = pDC->GetSafeHdc();

    //Init the graf_device
    pgInitDevice( PgGlobalsPtr(), MEM_NULL, (generic_var)hDC, &m_device);
        
    if(pInfo->m_bPreview)
    {
        //Paige scales if we are in scaled preview mode.  We must adjust the resolution
        //We need to change the resolution in the graf_device first!
        CPaigePreviewDC *pPreviewDC = (CPaigePreviewDC*)pDC;
        double nNum = pPreviewDC->GetScaleNum();
        double nDen = pPreviewDC->GetScaleDen();
        double dScaleFactor = 1.0;
        short sHiResolution = pgHiWord(m_device.resolution);
        short sLoResolution = pgLoWord(m_device.resolution);    
                
        dScaleFactor = nDen/nNum;
        sHiResolution = (short)((double)sHiResolution / dScaleFactor);
        sLoResolution = (short)((double)sLoResolution / dScaleFactor);

        long lNewResolution = sHiResolution << 16;
        lNewResolution |= sLoResolution;

        m_device.resolution = lNewResolution;
    }
        
    long lStartOffset;
    if (pInfo->m_pPD->m_pd.Flags & PD_SELECTION)
    {
        lStartOffset = pgFindPage(m_thePrintInfo.m_printPgRef, (short)pInfo->m_nCurPage, &page_rect);
        pgPrintToPage(m_thePrintInfo.m_printPgRef, &m_device, lStartOffset, &page_rect, best_way);
    }
    else
    {
        //Find the offset of the page to print
        lStartOffset = pgFindPage(m_paigeRef, (short)pInfo->m_nCurPage, &page_rect);
        //Print the page out
        pgPrintToPage(m_paigeRef, &m_device, lStartOffset, &page_rect, best_way);
    }
        
    pgCloseDevice( PgGlobalsPtr(), &m_device);
}       

void CPaigeEdtView::DoPageHeader(CDC* pDC, CPrintInfo* pInfo)
{
    char        temp[133];
    CFont       *pOldFont;
    CPen        *pOldPen;
    CRect       rcWhoCares;
    BOOL        bLayoutOnly = (pDC->GetClipBox( &rcWhoCares ) == NULLREGION);

    (pInfo);
    // this is better than the above line of code. If all you want to do is
    // test for a null-region, don't also modify the drawing rect!

    if ( bLayoutOnly ) 
    {
        return;
    }
 
    // Get the old font...and install our emboldend one
    if ( m_thePrintInfo.m_pHeaderFont != NULL )
    {
        pOldFont = ( CFont* ) pDC->SelectObject( m_thePrintInfo.m_pHeaderFont ) ;
    }
        
    if ( m_thePrintInfo.m_pPen )
    {
        // set the pen
        pOldPen = pDC->SelectObject( m_thePrintInfo.m_pPen );
    }

    // Develope the text for the header
    GetHeaderText(temp, sizeof(temp));  
                
    // Draw the text and line, but only if we're not just checking the layout
    pDC->DrawText(      temp, 
                        -1, 
                        &( m_thePrintInfo.m_HeaderTextRect ), 
                        DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER);

    pDC->MoveTo(        m_thePrintInfo.m_HeaderTextRect.left, 
                        ( int ) ( m_thePrintInfo.m_HeaderTextRect.bottom + m_thePrintInfo.m_lPenWidth ) );
    pDC->LineTo(        m_thePrintInfo.m_HeaderTextRect.right, 
                        ( int ) ( m_thePrintInfo.m_HeaderTextRect.bottom + m_thePrintInfo.m_lPenWidth ) );
}  

//      ========================================================================================
// DoPageFooter
void CPaigeEdtView::DoPageFooter(CDC* pDC, CPrintInfo* pInfo)
{
    char                temp[256];
    CDocument*  pDoc    = GetDocument();
    long                tempSize;
    CFont*              pOldFont;
    CPen*               pOldPen;
    CSize               size;
    CRect               rectPage;

    // this is better than the above line of code. If all you want to do is
    // test for a null-region, don't also modify the drawing rect!
    CRect rcWhoCares;
    BOOL bLayoutOnly = (pDC->GetClipBox( &rcWhoCares ) == NULLREGION);
        
    if ( bLayoutOnly ) 
    {
        return;
    }

    if ( m_thePrintInfo.m_pHeaderFont )
    {
        // Set the font for our operation here...
        pOldFont = pDC->SelectObject( m_thePrintInfo.m_pHeaderFont );
    }

    if ( m_thePrintInfo.m_pPen )
    {
        // set the pen
        pOldPen = pDC->SelectObject( m_thePrintInfo.m_pPen );
    }

    // Lets get the footer text and put it in the footer
    GetFooterText(temp, sizeof(temp));
    tempSize = strlen(temp);
        
    if (tempSize > 60)
    {
        strcpy(temp + 60, "...");
    }

    // Draw the text
    pDC->DrawText(      temp, 
                        -1, 
                        &( m_thePrintInfo.m_FooterTextRect ), 
                        DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER);

    // print the page #
    wsprintf(temp, "%d  ", pInfo->m_nCurPage);
        
    rectPage = m_thePrintInfo.m_FooterTextRect;
    size = pDC->GetTextExtent(temp, strlen(temp));
        
    rectPage.left = rectPage.right - size.cx;

    pDC->DrawText(temp, -1, &rectPage, DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER);

    // draw the line
    pDC->MoveTo(        m_thePrintInfo.m_FooterTextRect.left, 
                        ( int ) ( m_thePrintInfo.m_FooterTextRect.top - ( m_thePrintInfo.m_lPenWidth * 2 ) ) );
    pDC->LineTo(        m_thePrintInfo.m_FooterTextRect.right, 
                        ( int ) ( m_thePrintInfo.m_FooterTextRect.top - ( m_thePrintInfo.m_lPenWidth * 2 ) ) );

    // Restore the original font & pen
    if( m_thePrintInfo.m_pHeaderFont ) 
    {
        pDC->SelectObject( pOldFont );
    }

    if( m_thePrintInfo.m_pPen ) 
    {
        pDC->SelectObject( pOldPen );
    }
}


// GetHeaderText
void CPaigeEdtView::GetHeaderText(LPTSTR theText, int nSize)
{
    ASSERT(theText != NULL);

    CDocument* pDoc = GetDocument();
    if (pDoc)
    {
        strncpy(theText, pDoc->GetTitle(), nSize);
        theText[nSize - 1] = 0;
    }
    else
        *theText = 0;
}



// GetFooterText
void CPaigeEdtView::GetFooterText(LPTSTR theText, int nSize)
{
    _snprintf(theText, nSize, (LPCTSTR)CRString(IDS_PRINT_FOOTER), GetReturnAddress());
    theText[nSize - 1] = 0;
}



/////////////////////////////////////////////////////////////////////////////
// CPaigeEdtView diagnostics

#ifdef _DEBUG
void CPaigeEdtView::AssertValid() const
{
    CView::AssertValid();
}

void CPaigeEdtView::Dump(CDumpContext& dc) const
{
    CView::Dump(dc);
}

// OG
/*
  CPaigeEdtDoc* CPaigeEdtView::GetDocument() // non-debug version is inline
  {
  ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPaigeEdtDoc)));
  return (CPaigeEdtDoc*)m_pDocument;
  }
*/
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPaigeEdtView message handlers

int CPaigeEdtView::GetTotalHeight()
{
    return (pgTotalTextHeight(m_paigeRef, TRUE) + m_pgBorder.top + m_pgBorder.bottom);
}

int CPaigeEdtView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
/*      rectangle pgClientRect, emptyRect;
        shape_ref visArea, excludeArea;*/

    if (CView::OnCreate(lpCreateStruct) == -1)
        return -1;

        //Register the COleDropTarget object for OLE Drag n Drop
        //  Will be revoked when the view is destroyed
    m_dropTarget.Register(this);

    CRect clientRect;
    GetClientRect(&clientRect);

    // initialize our border space
    // that mirrors the RECT structure (i.e. "<left>,<top>,<right>,<bottom>").

//      CString rsrc;
//      rsrc.LoadString( IDS_PG_BORDER );

    sscanf( /*rsrc.GetBuffer(16)*/IDS_PG_BORDER, _T("%i,%i,%i,%i"), &m_pgBorder.left,
            &m_pgBorder.top, &m_pgBorder.right, &m_pgBorder.bottom );

//      rsrc.ReleaseBuffer();

    // initialize the spell checking method -- must do before "NewPaigeObject"
    g_bInteractiveSpellCheck = GetIniShort( IDS_INI_INTERACTIVE_SPELL_CHECK );

    // create an editor instance
    NewPaigeObject();

    // make sure that image/QT support is ready to rock
    PgMultimediaInit( PgGlobalsPtr() );

    SetPaigePalette();
/*
  pg_hooks hooks;
  pgGetHooks(m_paigeRef, &hooks);
  hooks.paginate_proc =         (post_paginate_proc)myPostPaginateProc;
  pgSetHooks(m_paigeRef, &hooks, FALSE);
*/
    return 0;
}

bool CPaigeEdtView::SetPaigePalette(void)
{
    CDC *pDC = GetDC();
    if (!pDC) return false;
    int rc = pDC->GetDeviceCaps(RASTERCAPS);
    if (rc & RC_PALETTE)
    {
        rc = pDC->GetDeviceCaps(SIZEPALETTE);
        if (256 == rc)
        {
            HPALETTE hPalette = m_Palette.GetPalette();
            pgSetDevicePalette(m_paigeRef, (generic_var)hPalette);
        }
    }
    ReleaseDC(pDC);
    return true;
}

bool CPaigeEdtView::NewPaigeObject()
{
    if ( !g_bInteractiveSpellCheck )
	KillTimer( AUTO_SPELL_TIMER );

    DeletePaigeObject();

    rectangle pgClientRect, emptyRect;
    shape_ref visArea, excludeArea;
        
    CRect clientRect;
    GetClientRect(&clientRect);
    clientRect.DeflateRect( m_pgBorder );

    // All Paige areas are represented as Shapes
    RectToRectangle(&clientRect,&pgClientRect);
    visArea = pgRectToShape(PgMemGlobalsPtr(), &pgClientRect);

    // set up an empty shape for "exclude area". with shared styles, if we
    // don't pass something now, we're screwed later.
    memset( &emptyRect, '\0', sizeof(rectangle) );
    excludeArea = pgRectToShape( PgMemGlobalsPtr(), &emptyRect );

    //Set both page area and visible area the same
    m_paigeRef = pgNew( PgGlobalsPtr(), (generic_var)GetSafeHwnd(),
                        visArea, visArea, excludeArea, /*0*/NO_HIDDEN_TEXT_BIT );

    //Inform Paige to use def page background color to do the erasing.
    pg_doc_info docInfo;
    pgGetDocInfo(m_paigeRef, &docInfo);
    docInfo.attributes |= COLOR_VIS_BIT;
    pgSetDocInfo(m_paigeRef, &docInfo, FALSE, best_way);

    // set up the Eudora/Paige glue bucket
    PgStuffBucket* pSB = new PgStuffBucket;
    pSB->kind = PgStuffBucket::kDocument;
    pSB->pWndOwner = this;
    paige_rec_ptr pPg = (paige_rec_ptr) UseMemory( m_paigeRef );
    pPg->user_refcon = (long) pSB;
    UnuseMemory( m_paigeRef );

    // Paige maintains a copy of the shape, so we can dispose what we created
    pgDisposeShape(visArea);
    pgDisposeShape( excludeArea );

    // Set the body style to have the MessageFont
    LOGFONT lf;
    font_info fontInfo;
    style_info styleInfo;
    style_info styleInfoMask;
    par_info parInfo;
    par_info parMask;
    GetMessageFont().GetLogFont( &lf );

    PgConvertLogFont( m_paigeRef, PgGlobalsPtr(), &lf, &fontInfo, &styleInfo, &styleInfoMask );
    pgGetParInfo( m_paigeRef, NULL, false, &parInfo, &parMask );

    //Set the tab space depending on the font and the tabstop value specified by the user.
    int avgWidth;
    avgWidth = GetMessageFont().CharWidth();
    parInfo.def_tab_space = avgWidth * GetIniShort(IDS_INI_TAB_STOP);

    long style_id = pgNewNamedStyle( m_paigeRef, body_style, &styleInfo, &fontInfo, &parInfo );

    //This is a hack to prevent the body_style from getting deleted. When paige fixes
    //up style runs, it deletes all styles which are unused, so we increment the 
    //used_ctr here to prevent it from being whacked.
    ASSERT(style_id);

    paige_rec_ptr pg;
    pg = (paige_rec_ptr) UseMemory(m_paigeRef);
        
    register style_info_ptr             styles;
    register pg_short_t                 style_ctr, style_qty;
        
    styles = (style_info_ptr)UseMemory(pg->t_formats);
    style_qty = (pg_short_t)GetMemorySize(pg->t_formats);
        
    for (style_ctr = 0; style_ctr < style_qty; ++styles, ++style_ctr)
    {
        if (styles->style_sheet_id == style_id) 
        {
            styles->used_ctr++;
            break;
        }
    }
        
    UnuseMemory(pg->t_formats);
    UnuseMemory(m_paigeRef);

    // smack it full of HTML styles
    CreateHTMLStyles( m_paigeRef, /*PgGlobalsPtr()->def_font.name*/ (unsigned char *)lf.lfFaceName, NULL );

    delete m_styleEx;
    m_styleEx = new CPaigeStyle(m_paigeRef);

    // Shareware: In reduced feature mode, you cannot spell check
    if (UsingFullFeatureSet())
    {
        // re-init the spelling object's paige reference
        if ( !g_bInteractiveSpellCheck ) {
            m_spell.Init( this );
            SetTimer( AUTO_SPELL_TIMER, AST_INTERVAL, NULL );
        }
    }

    return true;
}

void CPaigeEdtView::DeletePaigeObject()
{
    undo_ref *theRef;
    for (int i = m_undoStack.GetCount(); i ; i--)
    {
        theRef = m_undoStack.GetTail();
        pgDisposeUndo(*theRef);
        delete theRef;
        m_undoStack.RemoveTail();
    }
    for (i = m_redoStack.GetCount(); i; i--)
    {
        theRef = m_redoStack.GetTail();
        pgDisposeUndo(*theRef);
        delete theRef;
        m_redoStack.RemoveTail();
    }
    for (i = m_deleteUndoStack.GetCount(); i ; i--)
    {
        theRef = m_deleteUndoStack.GetTail();
        pgDisposeUndo(*theRef);
        delete theRef;
        m_deleteUndoStack.RemoveTail();
    }
    for (i = m_deleteRedoStack.GetCount(); i; i--)
    {
        theRef = m_deleteRedoStack.GetTail();
        pgDisposeUndo(*theRef);
        delete theRef;
        m_deleteRedoStack.RemoveTail();
    }

    if (m_paigeRef)
    {
        // gotsta smoke ma stuff!
        paige_rec_ptr pPg = (paige_rec_ptr) UseMemory( m_paigeRef );
        PgStuffBucket* pSB = (PgStuffBucket*) pPg->user_refcon;
        delete pSB;
        UnuseMemory( m_paigeRef );

        pgDispose(m_paigeRef);
    }
}

void CPaigeEdtView::OnDestroy() 
{
    // unselect the palette
    generic_var hPalette;
    hPalette = pgGetDevicePalette(m_paigeRef);
    if (hPalette)
        pgSetDevicePalette(m_paigeRef, (generic_var)0);

    DeletePaigeObject();

    CView::OnDestroy();
}

void CPaigeEdtView::OnSize(UINT nType, int cx, int cy) 
{       
    CView::OnSize(nType, cx, cy);
    UpdateScrollBars();

    //Get current caret position (client co_ords)
    CPoint curPoint = GetCaretPos();;
    //Fill Paige coord struct
    co_ordinate curCoOrd;
    CPointToPgPoint(&curPoint, &curCoOrd);

    //Scroll to caret only if it was visible before resizing
    short posBits = 0;
    posBits = pgPtInView(m_paigeRef, &curCoOrd, NULL);
    SetPgAreas(/*this*/);
    if (posBits & WITHIN_VIS_AREA) {
        //Reset page and visible areas after sizing
        if (pgTextSize(m_paigeRef)) ScrollToCursor();
    }
        
        
}       

void CPaigeEdtView::CPointToPgPoint(CPoint *cPoint, co_ordinate_ptr PgPoint)
{
    PgPoint->h = cPoint->x;
    PgPoint->v = cPoint->y;
}

void CPaigeEdtView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// Shareware: In reduced feature mode, you cannot right-click
	if (UsingFullFeatureSet())
	{
		// FULL FEATURE mode

		//
		// Put cooridnates in a format that Paige recognizes
		//
		co_ordinate pg_mouse;
		CPointToPgPoint(&point,&pg_mouse);
		//
		// This point rests in a word. Select that word.
		//
		select_pair wordSel, curSel;
		long offset = pgPtToChar(m_paigeRef, &pg_mouse, NULL);
		pgFindWord(m_paigeRef, offset, &wordSel.begin, &wordSel.end,TRUE,FALSE);
		pgGetSelection(m_paigeRef, &curSel.begin, &curSel.end);

		// 
		// Avoid treating this as a spelling click if
		// the click took place in a selection that includes stuff
		// besides the word
		//
		if (curSel.begin<offset && offset<=curSel.end &&
			(curSel.begin<wordSel.begin || wordSel.end<curSel.end))
			return;

		//
		// Misspelled?
		//
		if ( m_styleEx->IsMisspelled(&wordSel) )
		{
			pgSetSelection(m_paigeRef, wordSel.begin, wordSel.end, 0, TRUE);
			//
			// This dialog should really be off the lower right corner
			// of the selection.  Need to change.
			//
			POINT pt;
			pt.x = point.x;
			pt.y = point.y;
			ClientToScreen(&pt);
			m_spell.Popup(this, pt, wordSel);
		}           
	}

    return;
}


void CPaigeEdtView::OnLButtonDown(UINT nFlags, CPoint point) 
{
    {   // BOG: this is a "re-entrant" hack. if we get called again in the middle
	// of hyperlink processing, bad things can/will happen.
        paige_rec_ptr pgRec = (paige_rec_ptr) UseMemory( m_paigeRef );
        PgStuffBucket* pSB = (PgStuffBucket*) pgRec->user_refcon;
        UnuseMemory( m_paigeRef );

        if ( pSB->hlinkProcessing == true ) {
            assert( 0 );
            return;
        }
    }   // BOG: end re-entrant prophylactic

    
    {   // BOG: see if anything needs spell-checking
        CPaigeStyle objStyle( m_paigeRef );
        select_pair textRange;
        pgGetSelection( m_paigeRef, &textRange.begin, &textRange.end );

        select_pair curWord;
        pgFindWord( m_paigeRef, textRange.begin, &curWord.begin, &curWord.end, 1, 0 );

        co_ordinate newCoord;
        long newOffset;
        CPointToPgPoint( &point, &newCoord );
        newOffset = pgPtToChar( m_paigeRef, &newCoord, NULL );
        bool bInCurrentWord = !((newOffset < curWord.begin) || (newOffset > curWord.end));

        if ( !bInCurrentWord && objStyle.NeedsSpellCheck( &textRange ) )
            m_checkdastuff = true;
    }   // BOG: end spellcheck hack


    BOOL ptInSelection = false;
    BOOL bSelectLine = FALSE;
    paige_rec_ptr pgPtr = MEM_NULL;

    co_ordinate pgMousePt;
    CPointToPgPoint(&point, &pgMousePt);

    SetCapture();
    m_pgModifiers = 0; 
    m_mousePt = CPoint(0,0);
    if (nFlags & MK_SHIFT) m_pgModifiers |= EXTEND_MOD_BIT;
    //if (nFlags & MK_CONTROL) m_pgModifiers |= DIS_MOD_BIT;

    //Drag-n-Drop and Triple-click selection will occur only if the click
    //was in selected text
    if ( pgNumSelections( m_paigeRef ) ) {
        //Create an empty shape and use it to get the current hilite region
        shape_ref hiliteRgn = 0;
        if ( (hiliteRgn = pgRectToShape(PgMemGlobalsPtr(), NULL)) != 0 ) {
            if ( pgGetHiliteRgn( m_paigeRef, NULL, MEM_NULL, hiliteRgn ) ) {
                if ( pgPtInShape( hiliteRgn, &pgMousePt, NULL, NULL, NULL ) )
                    ptInSelection = true;
            }

            pgDisposeShape( hiliteRgn );
        }
    }

    switch(m_mouseState)
    {
        //case MOUSE_DOUBLE_CLICKED : 
    case MOUSE_TRIPLE_CLICKED : 
        KillTimer(TRIPLE_CLICK_TIMER);
        m_mouseState &= ~MOUSE_TRIPLE_CLICKED;
        //Select only if the click was in selected text.
        if (ptInSelection)
            m_pgModifiers |= PAR_MOD_BIT;
        m_mouseState |= MOUSE_DRAG_SELECT;
        break;
    default :
        m_mousePt = point;
        if (ptInSelection) 
            m_mouseState |= (MOUSE_DRAG_DROP | MOUSE_DOWN);
        else m_mouseState |= MOUSE_DRAG_SELECT;
        break;
    }

    //Delay the func in case it turns out to be a drag-drop operation, else the 
    //text is deselected and the caret will be positioned at the current 
    //mouse-click pos. Incase it is an ordinary mouse click, then
    //we process DragSelect with both mouse_down and mouse up in LButtonUp.
    if (!(m_mouseState & MOUSE_DRAG_DROP))
    {
        //If the mouse click is in the *selection region* in the left border, 
        //select the whole line, extending/deselecting previous selections, if any.                     
        if ((point.x < m_pgBorder.left) && (point.y > m_pgBorder.top)&& (nFlags & MK_CONTROL))
        {
            SelectionMarginClicked(nFlags, point);
            m_prevPoint.x = point.x;
            m_prevPoint.y = point.y;
        }
        else if ((point.x < m_pgBorder.left) && (point.y > m_pgBorder.top) && (m_prevPoint.x == 0) && (m_prevPoint.y == 0))
        {       
            //Bug 2341. If this if the first time the user clicks in the margin then we need to pretend like a 
            //MOUSE_DRAG_SELECT action has occured.  Find the begining of the next line and set point.y to be that and
            //drag select to that point.  This will allow the user to drag and select lines at a time as well as
            //just select one line.
            bSelectLine = TRUE;
            CRect aRect;
            GetClientRect(aRect);
            CPointToPgPoint(&point, &pgMousePt);
            pgDragSelect(m_paigeRef, &pgMousePt, mouse_down, m_pgModifiers, 0, FALSE);

            //Find the point of the next line...
            co_ordinate curCoOrd;
                        
            long offset, oldLineLeft, lineLeft, lineRight;
                        
            CPointToPgPoint(&point, &curCoOrd);


            offset = pgPtToChar(m_paigeRef, &curCoOrd, NULL);
                        
                        //Since Paige resets the anchor only on the next mouse_down
                        //to this offset [incase the user clicked with the Shift key
                        //down, Paige has to know from where to extend the selection]
                        //So we set the anchor if clicking in the margin or without the
                        //shift key down.
            pgPtr = (paige_rec_ptr) UseMemory(m_paigeRef);
            pgPtr->hilite_anchor = offset;
            UnuseMemory(m_paigeRef);

            pgFindLine(m_paigeRef, offset, &lineLeft, &lineRight);
            oldLineLeft = lineLeft;
                        
            //Move to next line
            while((lineLeft == oldLineLeft) && (point.y < aRect.bottom))
            {
                point.y +=2;
                CPointToPgPoint(&point, &curCoOrd);
                offset = pgPtToChar(m_paigeRef, &curCoOrd, NULL);
                pgFindLine(m_paigeRef, offset, &lineLeft, &lineRight);
                
                if(point.y > aRect.bottom)
                {
                    point.y = m_prevPoint.y + 20;
                    break;
                }
            }
            //Use the next point.
            CPointToPgPoint(&point, &pgMousePt);
            SetTimer(DRAG_SELECT_TIMER, 100, NULL);
            pgDragSelect(m_paigeRef, &pgMousePt, mouse_moved, m_pgModifiers, 0, FALSE);
        }
        //Let Paige take care of any other ordinary, mundane stuff.
        /*else*/ if ((m_mouseState & MOUSE_DRAG_SELECT) && !bSelectLine)
        {
            SetTimer(DRAG_SELECT_TIMER, 100, NULL);
                        
            pgDragSelect(m_paigeRef, &pgMousePt, mouse_down, m_pgModifiers, 0, FALSE);

            if ( ! (nFlags & MK_SHIFT) )
            {
                pgPtr = (paige_rec_ptr) UseMemory(m_paigeRef);
                co_ordinate curCoOrd;
                CPointToPgPoint(&point, &curCoOrd);
                long offset = pgPtToChar(m_paigeRef, &curCoOrd, NULL);
                pgPtr->hilite_anchor = offset;
                UnuseMemory(m_paigeRef);
            }
        }
    }

    // BOG: Mouse clicks cancel "always visible find selection" mode
    m_bDontDeactivateHighlight = FALSE;

    CView::OnLButtonDown(nFlags, point);
}

void CPaigeEdtView::SelectionMarginClicked(UINT nFlags, CPoint point)
{
    paige_rec_ptr pgPtr = MEM_NULL;
    pgPtr = (paige_rec_ptr) UseMemory(m_paigeRef);
    ASSERT(pgPtr != MEM_NULL);
    long anchor = pgPtr->hilite_anchor;

    //Control Click selects the entire document
    if (nFlags & MK_CONTROL)
    {
        pgSetSelection(m_paigeRef, 0, pgTextSize(m_paigeRef), 0, TRUE);
        pgPtr->hilite_anchor = 0;
        m_mouseState &= ~MOUSE_DRAG_SELECT;
    }
    else 
    {
        co_ordinate curCoOrd;
        long offset, selectLeft, selectRight, lineLeft, lineRight;
                
        pgGetSelection(m_paigeRef, &selectLeft, &selectRight);

        CPointToPgPoint(&point, &curCoOrd);
        offset = pgPtToChar(m_paigeRef, &curCoOrd, NULL);
        pgFindLine(m_paigeRef, offset, &lineLeft, &lineRight);
                
        if (nFlags & MK_SHIFT)
        {
            // The hilite-anchor can be 0 in the following 3 cases
                                //1. Anchor was at 0, text may or may not be selected
                                //2. The whole text has been selected (Deselect in this case)
                                //3. A line may have been previously selected with Ctrl+Click 
                                //   (Extend selection in any direction in this case)

            // Set the correct anchor if the previous selection was from a Ctrl+Click
            if (anchor == 0)
            {
                long tempLeft, tempRight;
                pgFindLine(m_paigeRef, selectLeft, &tempLeft, &tempRight);
                if ((selectLeft == tempLeft) && (selectRight == tempRight))
                    pgPtr->hilite_anchor = (lineLeft < selectLeft) ? selectRight : selectLeft;
            }
            // Extend selection OR deselect depending on the anchor position
            if (anchor == selectLeft)
            {
                if (lineLeft < selectRight)
                {
                    pgSetSelection(m_paigeRef, anchor, lineLeft, 0, true);
                    m_mouseState &= ~MOUSE_DRAG_SELECT;
                }
                                //else do the usual processing
            }
            else if (anchor == selectRight)
            {
                if (lineLeft > selectLeft)
                {
                    pgSetSelection(m_paigeRef, lineLeft, anchor, 0, true);
                    m_mouseState &= ~MOUSE_DRAG_SELECT;
                }
                                //else do the usual processing
            }

            /*OLD STUFF
              //If the caret/entire-current-selection is on the same line,
              //set the selection to the entire line
              //if ((selectLeft > lineLeft) && (selectRight < lineRight))
              //        pgSetSelection(m_paigeRef, lineLeft, lineRight, 0, TRUE);
              //if we are SHIFT clicking *below* any existing-selected-lines, we need
              //to extend the selection. Normal paige processing will take care of 
              //extending selection if we SHIFT click above the existing-selected-lines.
              //else 
              if (lineLeft < selectLeft )
              pgExtendSelection(m_paigeRef, lineLeft - selectLeft, EXTEND_MOD_BIT, TRUE);
              else if (lineRight > selectRight)
              pgExtendSelection(m_paigeRef, lineRight - selectRight, EXTEND_MOD_BIT, TRUE);*/
        }
        else 
        {
            //Ordinary click sets the selection to that line
            pgSetSelection(m_paigeRef, lineLeft, lineRight, 0, TRUE);
            pgPtr->hilite_anchor = 0;
            m_mouseState &= ~MOUSE_DRAG_SELECT;
        }
    }                           

    if (pgPtr) UnuseMemory(m_paigeRef);
}

void CPaigeEdtView::OnLButtonUp(UINT nFlags, CPoint point) 
{
    // BOG: this is a "re-entrant" hack. if we get called again in the middle
    // of hyperlink processing, bad things can/will happen.
    {
        paige_rec_ptr pgRec = (paige_rec_ptr) UseMemory( m_paigeRef );
        PgStuffBucket* pSB = (PgStuffBucket*) pgRec->user_refcon;
        UnuseMemory( m_paigeRef );

        if ( pSB->hlinkProcessing == true ) {
            assert( 0 );
            return;
        }
    }   // BOG: end re-entrant prophylactic

    co_ordinate pgMousePt;
    //Reset the previous point for next time.
    m_prevPoint.x = m_prevPoint.y = 0;

        //If the click was on the selected text, we delayed incase it was a
        //Drag-n-drop operation. If it wasn't, then this is where we call 
        //pgDragSelect with mouse-down verb
    if (m_mouseState & MOUSE_DOWN)
    {
        CPointToPgPoint(&m_mousePt, &pgMousePt);
        pgDragSelect(m_paigeRef, &pgMousePt, mouse_down, m_pgModifiers, 0, FALSE);
        m_mouseState &= ~(MOUSE_DOWN | MOUSE_DRAG_DROP);
        // Call DragSelect again with mouse_up verb for Paige to do some internal cleanup
        CPointToPgPoint(&point, &pgMousePt);
        pgDragSelect(m_paigeRef, &pgMousePt, mouse_up, m_pgModifiers, 0, FALSE);
    }

    if (m_mouseState & MOUSE_DRAG_SELECT) 
    {
        KillTimer(DRAG_SELECT_TIMER);
        // Call DragSelect again with mouse_up verb for Paige to do some internal cleanup
        CPointToPgPoint(&point, &pgMousePt);
        pgDragSelect(m_paigeRef, &pgMousePt, mouse_up, m_pgModifiers, 0, FALSE);
    }

    if (m_pgModifiers & PAR_MOD_BIT)
    {
        //If it was a triple click, set the hilite anchor 
        //to the beginning of the selection
        long selectLeft;
        pgGetSelection(m_paigeRef, &selectLeft, MEM_NULL);

        paige_rec_ptr pgPtr = (paige_rec_ptr) UseMemory(m_paigeRef);
        pgPtr->hilite_anchor = selectLeft;
        UnuseMemory(m_paigeRef);
                
    }

    m_pgModifiers = 0;
    m_mousePt = CPoint(0,0);
    m_mouseState &= ~MOUSE_DRAG_SELECT;
    ReleaseCapture();

    CView::OnLButtonUp(nFlags, point);

        // BOG: we want to do this after anything else is out of the way, even
        // calling the base CView version first. This will prevent long
        // operations, resulting from the user clicking a hyperlink, from mucking
        // up anything else (selections, focus changes, etc...)
        //
        // As an aside, I wonder if we shouldn't be calling CView first. If the
        // user clicks a window in the background, do we really want to go
        // through all of our own Paige-related processing before the default
        // Windows stuff occurs? Just a thought ;-)     
        
    paige_rec_ptr pgRec = (paige_rec_ptr) UseMemory( m_paigeRef );
    PgStuffBucket* pSB = (PgStuffBucket*) pgRec->user_refcon;
    UnuseMemory( m_paigeRef );

    if (pSB->hlinkEvent)
    {
        select_pair sel;
        pg_hyperlink autoLink;
        
        pgGetSelection(m_paigeRef, &sel.begin, &sel.end); 

        pgGetHyperlinkSourceInfo(m_paigeRef, sel.begin, pSB->hlinkEvent, TRUE, &autoLink);

        //If it's an AutoURL make sure it's not stale.
        if ((!m_fRO) && (autoLink.type & HYPERLINK_EUDORA_AUTOURL))
        {
            char buf[INTERNET_MAX_URL_LENGTH];
            GetPgText(buf, sizeof(buf), autoLink.applied_range, FALSE);
            if (ValidURLScheme(buf))
            {
                char temp[INTERNET_MAX_URL_LENGTH] = "";
                strncat(temp, EscapeURL(buf), INTERNET_MAX_URL_LENGTH-1);
                pgSetSourceURL( m_paigeRef, autoLink.applied_range.begin, 
                                (unsigned char *)temp );
            }
            else
            {
                pgDeleteHyperlinkSource(m_paigeRef, autoLink.applied_range.begin, TRUE);
                paige_rec_ptr pgRec = (paige_rec_ptr) UseMemory( m_paigeRef );
                PgStuffBucket* pSB = (PgStuffBucket*) pgRec->user_refcon;
                pSB->hlinkEvent = 0;
                UnuseMemory( m_paigeRef );
            }
        }               

        // Process the hyperlink now [whatever it is]
		// Alt + click on URL in edit mode will launch
        if (m_fRO || GetKeyState(VK_MENU) < 0)
            PgProcessHyperlinks( m_paigeRef );
    }

}

HCURSOR hCur = 0;
BOOL CPaigeEdtView::OnSetCursor( CWnd* pWnd, UINT nHitTest, UINT message )
{
    if ( nHitTest == HTCLIENT && hCur ) {
        ::SetCursor( hCur );
        return TRUE;
    }
    else if ( nHitTest == HTVSCROLL ) {
        ::SetCursor( ::LoadCursor( 0, IDC_ARROW ) );
        return TRUE;
    }

    return FALSE;
}

void CPaigeEdtView::OnMouseMove(UINT nFlags, CPoint point) 
{
    // BOG: this cursor tweaking stuff really all belongs in OnSetCursor, except
    // that it will need to be fiddled around a bit when that happens. It's
    // working real good right now, and we're almost Beta 1, so I'm gonna
    // leave it 'til later.
    co_ordinate mousePt;
    CPointToPgPoint( &point, &mousePt );
    short ptID = pgPtInView( m_paigeRef, &mousePt, NULL );

    if ( ptID & WITHIN_VIS_AREA ) {
        pg_short_t ptInSelection = 0;
        if ( pgNumSelections( m_paigeRef ) ) {
            shape_ref hiliteRgn = 0;
            if ( (hiliteRgn = pgRectToShape( PgMemGlobalsPtr(), NULL )) != 0 ) {
                if ( pgGetHiliteRgn( m_paigeRef, NULL, MEM_NULL, hiliteRgn ) ) {
                    if ( pgPtInShape( hiliteRgn, &mousePt, NULL, NULL, NULL ) )
                        ptInSelection = true;
                }

                pgDisposeShape( hiliteRgn );
            }
        }

        CMainFrame* pWnd = (CMainFrame*)AfxGetMainWnd();
        bool bSetStatusText = TRUE;
        int offset;
        if ((offset=pgPtInHyperlinkSource( m_paigeRef, &mousePt )) != -1 ) 
        {
			// Alt + click on URL in edit mode will launch
            if (m_fRO || GetKeyState(VK_MENU) < 0)
                hCur = ::QCLoadCursor( IDC_APP_LAUNCH );
            else if ( ptInSelection )
                hCur = ::LoadCursor( 0, IDC_ARROW );
            //
            // Do hyperlink check for status bar
            //
            char url[INTERNET_MAX_URL_LENGTH] = "";
            if (pgGetSourceURL(m_paigeRef, offset, (pg_char_ptr)url, INTERNET_MAX_URL_LENGTH))
            {
                pWnd->SetStatusBarText((const char*) url);
                bSetStatusText = FALSE;
            }
        }
        else if ( ptInSelection )
            hCur = ::LoadCursor( 0, IDC_ARROW );
        else if ( pgPtInEmbed( m_paigeRef, &mousePt, NULL, NULL, false ) )
            hCur = ::LoadCursor( 0, IDC_ARROW );
                
        else
            hCur = ::LoadCursor( 0, IDC_IBEAM );

        if ( bSetStatusText)
            pWnd->PostMessage(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
    }
    else if ( (point.x < m_pgBorder.left) && (point.y > m_pgBorder.top) ) {
        hCur = ::QCLoadCursor( IDC_POINTER_SELECT );
    }

////// END CURSOR TWEAKING ///////////////////////////

    if (m_mouseState & MOUSE_DRAG_DROP)
    {
        //By default, Drag Drop will start after 200 millisecs or after it has
        //moved the number of pixels specified in the RECT parameter of DoDragDrop
        //But we want it to start only if the cursor has moved some pixels <ignore time factor>
        if (abs(point.x - m_mousePt.x) > 6 ||
            abs(point.y - m_mousePt.y) > 6)
        {
            COleDataSource *pDataSource = new COleDataSource();
            //Set up the Data Source for Drag and Drop
            OnCopy(pDataSource);
            DROPEFFECT dropEffect = pDataSource->DoDragDrop(DROPEFFECT_COPY | DROPEFFECT_MOVE, NULL, NULL);
            //Let OnDrop take care of deleting selection on a intra-app move
                        
            //If dragged to a different target and it was a MOVE effect, then delete the current selection
            if ((dropEffect == DROPEFFECT_MOVE) && (!m_fRO))
            {
                if ( (m_mouseState & MOUSE_DRAG_DROP) && (pgNumSelections(m_paigeRef)) ) 
                {
                    /*if (offset > selectRight)
                      offset -= (selectRight - selectLeft);*/
                    long insertRef[2];
                    pgGetSelection(m_paigeRef, &insertRef[0], &insertRef[1]);
                    PrepareUndo(undo_drag, insertRef, FALSE);
                    // Delete the current selection since it's a MOVE operation
                    pgDelete(m_paigeRef, NULL, best_way);
                    /*verb = undo_drag_drop_move;*/
                }
            }

            //Cleanup, maybe this should be in OnDrop/OnDragLeave
            m_mouseState = 0;
            m_mousePt = CPoint(0,0);
        }
    }
    else if((m_mouseState & MOUSE_DRAG_SELECT) && (point.x < m_pgBorder.left) && (point.y > m_pgBorder.top) && (m_prevPoint.x == 0) && (m_prevPoint.y == 0))
    {
        m_prevPoint.x = point.x;
        m_prevPoint.y = point.y;                
    }   
    // Use the modifiers set in LButtonDown to make extended or discontinuous selections
    else if (m_mouseState & MOUSE_DRAG_SELECT)
    {
        m_mousePt = point;
        co_ordinate pgMousePt;
        CPointToPgPoint(&point, &pgMousePt);
        pgDragSelect(m_paigeRef, &pgMousePt, mouse_moved, m_pgModifiers, 0, FALSE);
    }

    CView::OnMouseMove(nFlags, point);
}


inline bool is_word_delimiter( unsigned uChar ) {
    return (uChar == VK_SPACE || uChar == VK_RETURN || uChar == VK_TAB );   // ||
//            (ispunct((int)uChar) > 0));
}

// seek_to_blank --- seeks to just before the next ocurring blank character in the
// document. if one cannot be found, either the beginning or the end of document will
// be returned, depending on the value of "toLeft".
inline long seek_to_blank( pg_ref pg, long offset, bool toLeft, select_pair* pSel ) {
    long li, maxText;

    if ( pSel ) {
	li = pSel->begin;
	maxText = pSel->end;
    }
    else {
	li = offset;
	maxText = pgTextSize( pg );
    }

    for ( ; toLeft ? (li > 0) : (li < maxText); toLeft ? li-- : li++ )
        if ( pgCharType( pg, toLeft ? li - 1 : li + 1, BLANK_BIT ) & BLANK_BIT )
            break;

    return li;
}

inline bool chartype_in_range( pg_ref pg, long mask_bits, long offset, long nChars, bool toLeft ) {
    long li = offset;
    const long maxText = nChars;

    for ( int i = 0; i < maxText; i++ )
        if ( pgCharType( pg, toLeft ? li - 1 : li, mask_bits ) & mask_bits )
            return true;

    return false;
}

// adjust_text_range -- adjusts a select_pair to make sure that the "begin"
// and "end" fields fall on word boundaries.
inline void adjust_text_range( pg_ref pg, select_pair_ptr pTextRange ) {
    long offset, dummy;
    offset = pTextRange->begin;
    pgFindWord( pg, offset, &pTextRange->begin, &dummy, true, false );
    offset = pTextRange->end;
    pgFindWord( pg, offset, &dummy, &pTextRange->end, false, false );
}


void CPaigeEdtView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    TCHAR uChar = (TCHAR) nChar;
    bool bSplitExcerpt = FALSE;
    bool bBegOfExcerpt = FALSE;
    bool bEndOfMsg = FALSE;
    select_pair sel;

	// How about we update this, huh?
	last_typed = GetTickCount();

    if( uChar == VK_ESCAPE )
        return;

    // Process the keystrokes only if the document is not read only.
    if ( m_fRO )
    {
        if (nChar == VK_SPACE)
        {
            if (ShiftDown())
            {
                // A Shift+Space on a read-only view translates into a
                // "toggle read status" command.
                SendMessage(WM_COMMAND, ID_MESSAGE_STATUS_TOGGLE, LPARAM(GetSafeHwnd()));
            }
            else
            {
                // Translate a Space keypress on a read-only view 
                // into a scroll down keypress.  This is superior to
                // simulating a WM_VSCROLL scroll bar event since it
                // logically moves the cursor with the view.

                int nScrollPos = GetScrollPos(SB_VERT);
                SendMessage(WM_KEYDOWN, VK_NEXT, 1);
                SendMessage(WM_KEYUP, VK_NEXT, 1);

                if (GetScrollPos(SB_VERT) == nScrollPos)
                    SendMessage(WM_COMMAND, ID_NEXT_MESSAGE);
            }
        }

        if (uChar == PgGlobalsPtr()->line_wrap_char)
        {
            //If the caret or the beginning of the selection is
            //in the link, then process it.
            pgGetSelection(m_paigeRef, &sel.begin, MEM_NULL);
            long hLinkID = pgGetSourceID (m_paigeRef, sel.begin);

            if ( !hLinkID )
                return;

            paige_rec_ptr pgPtr = (paige_rec_ptr) UseMemory(m_paigeRef);

            // simply stash the hyperlink's id for later processing
            PgStuffBucket* pSB = (PgStuffBucket*) pgPtr->user_refcon;
            pSB->hlinkEvent = hLinkID;
            UnuseMemory(m_paigeRef);
            PgProcessHyperlinks(m_paigeRef);
        }

        return;
    }

	// If the character is printable, the current word is going
	// to be changed, possibly into two words.  If that happens
	// we want to be sure to both words get checked.  Thus, we'll
	// mark the whole thing right now as needing to be checked
	// SD 3/10/99
	if (isprint(uChar) && !isalnum(uChar)) m_styleEx->ClearMisspellingAndRecheck();

    
	// Keep track of where we were, for benefit of speller SD 2/24/99
	long oldChangeCount = pgGetChangeCtr(m_paigeRef);
	select_pair oldSel;
	pgGetSelection(m_paigeRef,&oldSel.begin,&oldSel.end);

    // The arrow keys are processed in OnKeyDown
    // Only traps Chars with an Ascii code. The rest are trapped in KeyDown.
    //
    // Pre-process Space, Enter keys for "Is the AutoURL going to go insane?" check

    long length = 0;
    pg_hyperlink autoLink;
    BOOL URLdirty = FALSE;

    if ( (uChar == PgGlobalsPtr()->line_wrap_char) || ((nChar == VK_SPACE)) )
    {
        long linkID;

        pgGetSelection(m_paigeRef, &sel.begin, &sel.end);
        
        if ( linkID = pgGetSourceID(m_paigeRef, sel.begin) )
        {
            pgGetHyperlinkSourceInfo(m_paigeRef, sel.begin, linkID, TRUE, &autoLink);

            if (autoLink.type & HYPERLINK_EUDORA_AUTOURL)
            {
                if ((sel.begin > autoLink.applied_range.begin) && (sel.begin < autoLink.applied_range.end))
                {
                    if (autoLink.applied_range.begin > 0)
                    {           
                        long charInfo = pgCharType(m_paigeRef, autoLink.applied_range.begin - 1,
                                                   BLANK_BIT | TAB_BIT | PAR_SEL_BIT);

                        if ( charInfo & (BLANK_BIT | TAB_BIT | PAR_SEL_BIT) )
                            URLdirty = TRUE;
                    }
                    else URLdirty = TRUE;
                }
            }
        }
    }


    // BOG: hack for overwrite mode. doesn't appear there's any built-in
    // way to do this. so here's my solution. i'm so proud.
    bool bDontOverwrite = ((uChar == PgGlobalsPtr()->line_wrap_char) ||
                           (uChar == PgGlobalsPtr()->bs_char));

    if ( g_bOverwriteMode && !bDontOverwrite ) {
        long begin, end;
        pgGetSelection( m_paigeRef, &begin, &end );

        if ( begin == end ) {
            long chType = pgCharType( m_paigeRef, begin, PAR_SEL_BIT );

            if ( !(chType & PAR_SEL_BIT) && end != pgTextSize(m_paigeRef)) {
                //PrepareUndo( undo_fwd_delete );
                PrepareUndo( undo_fwd_delete, MEM_NULL, TRUE );
                unsigned char chFwdDel = (TCHAR) PgGlobalsPtr()->fwd_delete_char;
                pgInsert( m_paigeRef, (pg_byte_ptr)&chFwdDel, 1, CURRENT_POSITION,
                          key_insert_mode, 0, best_way );
            }
            else
                bDontOverwrite = TRUE;
        }
        else
            bDontOverwrite = TRUE;
    }


    while (nRepCnt--) 
    {
        pgGetSelection(m_paigeRef, &sel.begin, &sel.end);

        // Backspace
        if (uChar == PgGlobalsPtr()->bs_char) 
        {
            if (pgNumSelections(m_paigeRef))
                CleanURLonPosteriorDelete(sel.begin);

            PrepareUndo(undo_backspace);
        }
        else
        {
            // Ctrl + Backspace: already taken care of in OnKeyDOwn
            if ((GetKeyState(VK_CONTROL) < 0) && (uChar == 127)) 
                break;
            else
            {
                // Clear style (Plain Text)
                if ((nChar == VK_SPACE) && ( GetKeyState(VK_CONTROL)<0 ) )
                {
                    OnClearFormatting();

					// Fix for bug #5002
					// Get out of here right now because call to ClearMisspellingAndRecheck()
					// down below causes formatting not to be cleared.
                    return;
                }
                else
                {
                    if ( (nChar == VK_TAB) && (GetIniShort( IDS_INI_TAB_SPACES )) )
                    {
                        long numSpaces = (long)TabToSpaces();
                        if (numSpaces)
                            PrepareUndo(undo_insert, &numSpaces);

                        uChar = (TCHAR) VK_SPACE;

                        //Insert n-1 spaces, the last one is inserted below
                        for (int count = 1; count < numSpaces; count++)
                            pgInsert( m_paigeRef, (pg_byte_ptr)&uChar, 1, CURRENT_POSITION,
                                      key_insert_mode, 0, draw_none );
                    }
                    else
                    {
                        // NewLine: In an excerpt, must split the excerpt in two
                        // and thus will require a series of carriage returns inserted.
                        if (uChar == PgGlobalsPtr()->line_wrap_char )
                        {
                            if ( bSplitExcerpt = m_styleEx->IsExcerpt(&sel) )
                            {
                                select_pair tempSel;
                                if (sel.begin > 0)
                                    tempSel.begin = tempSel.end = sel.begin - 1;
                                if (( sel.begin == 0 ) || ( ! m_styleEx->IsExcerpt(&tempSel) ))
                                {
                                    length = 2;
                                    bBegOfExcerpt = TRUE;
                                }
                                else
                                {
                                    length = 4;

                                    // If next character is a newline then insert one less new line.
                                    if (pgCharType(m_paigeRef, sel.end,PAR_SEL_BIT))
                                        length--;                   
                                    if ( bEndOfMsg= (sel.end == pgTextSize(m_paigeRef)) )
                                        length--;
                                }

                                PrepareUndo(undo_insert,&length);
                            }
                            else
                                PrepareUndo(undo_typing);

                            // Bullet list processing.  The bullet list style gets turned off if the following 
                            // conditions are met:
                            // 1) Nothing is selected; 2) The caret is in a non excerpted, bulleted list;
                            // 3) the caret is the first and only character on the line; and 4) the line
                            //    is the last line of a bulleted list.  Whew!

                            if ( (sel.begin==sel.end) && m_styleEx->IsBullet(&sel) && !bSplitExcerpt )
                            {
                                if (( sel.begin>0 ) && 
                                    ( pgCharType(m_paigeRef, sel.begin-1, PAR_SEL_BIT) & PAR_SEL_BIT) &&
                                    (( pgCharType(m_paigeRef, sel.begin,   PAR_SEL_BIT) & PAR_SEL_BIT) ||
                                     ( sel.begin >= pgTextSize(m_paigeRef) ) ) )
                                {
                                    select_pair selLine;
                                    pgFindLine(m_paigeRef, sel.begin, &selLine.begin, &selLine.end);
                                    selLine.begin = selLine.end;

                                    if (( selLine.end >= pgTextSize(m_paigeRef)) || !m_styleEx->IsBullet(&selLine) )
                                    {
                                        m_styleEx->ApplyBullet(FALSE);
                                        pgGetSelection(m_paigeRef, &sel.begin, &sel.end);
                                        pgSetSelection(m_paigeRef, sel.begin, sel.end, 0, FALSE);
                                    }
                                }
                            }
                        }
                        else    // All other characters
                        {
                            if ( g_bOverwriteMode && !bDontOverwrite )
                                PrepareUndo(undo_overwrite);
                            else
                                PrepareUndo(undo_typing);
                        }
                    }
                }
            }
        }


#ifdef NEVER	// this is now done at the bottom of the fn SD 2/24/99
        //
        // Check for Misspelled word.  Unmark word because it has now been modified.
        //

		sel2.begin = max(0,sel.begin-1);
        sel2.end = sel.end;

		m_styleEx->ClearMisspellingAndRecheck(&sel2);
#endif

        if ( is_word_delimiter( nChar ) ) {
            long begin, end;
            pgGetSelection( m_paigeRef, &begin, &end );

            if ( begin == end ) {
                long chType = pgCharType( m_paigeRef, begin - 1, WORD_BREAK_BIT /*| WORD_SEL_BIT*/ );

                if ( !((chType & WORD_BREAK_BIT)) ) {
                    long beginUrl = seek_to_blank( m_paigeRef, end, true, NULL );
                    MakeAutoURLSpaghetti( beginUrl, end );

		    if ( !g_bInteractiveSpellCheck ) {
			QueueDirtyPair( beginUrl, end );
			m_checkdastuff = true;
		    }
                }
            }
        }

                
        // Insert the character
        if ( !bSplitExcerpt )
        {
            pgInsert( m_paigeRef, (pg_byte_ptr)&uChar, 1, CURRENT_POSITION, 
                      key_insert_mode, 0, best_way );
        }
        else
        {
            // Break the excerpt in two
            pgInsert( m_paigeRef, (pg_byte_ptr)&uChar, 1, CURRENT_POSITION, 
                      key_insert_mode, 0, best_way );

            style_info  sinfo;
            font_info   fontInfo;
            sinfo = PgGlobalsPtr()->def_style;
            fontInfo = PgGlobalsPtr()->def_font;

            for (int i=0; i< length-1; i++)
            {
                pgSetInsertionStyles(m_paigeRef, &sinfo,&fontInfo);
                pgInsert( m_paigeRef, (pg_byte_ptr)&uChar, 1, CURRENT_POSITION, 
                          key_insert_mode, 0, best_way );
            }

            //
            // In the "broken" region, clear excerpting, paragraph and font styles
            //
 
            select_pair sel2;
            if (!bBegOfExcerpt)
            {
                sel2.begin = sel.begin+1;
                sel2.end = sel.begin+4;
            }
            else
            {
                sel2.begin = sel.begin;
                sel2.end = sel.begin + 2;
            }
            par_info parInfo, parInfoMask;
            long styleID;

            if (GetIniShort(IDS_INI_USE_PROPORTIONAL_AS_DEFAULT))
                styleID = pgGetNamedStyleIndex(m_paigeRef, body_style);
            else
                styleID = pgGetNamedStyleIndex(m_paigeRef, teletype_style);
            if (styleID)
                pgGetNamedStyleInfo(m_paigeRef, styleID, NULL, NULL, &parInfo);
            else
                parInfo = PgGlobalsPtr()->def_par;

            pgFillBlock( &parInfoMask, sizeof(par_info), SET_MASK_BITS );
            pgSetParInfo( m_paigeRef, &sel2, &parInfo, &parInfoMask, best_way );

            // Reset Caret in proper position
            if ( bEndOfMsg )
            {
                pgSetSelection(m_paigeRef, sel2.end, sel2.end, 0, FALSE);
                ClearStyle();
                ClearFormat();  
            }

            if (sel.begin == 0)
                pgSetSelection(m_paigeRef, 0, 0, 0, TRUE);      
            else
                pgSetSelection(m_paigeRef, sel.begin+2, sel.begin+2, 0, TRUE);
            Invalidate();
            bSplitExcerpt=FALSE;

        }
    }
        
    //
    //  AutoURL Post-Processing
    //

    if (URLdirty)
    {
        select_pair tempSel;
        char buf[INTERNET_MAX_URL_LENGTH];
        BOOL firstURLStillGood = FALSE, secondURLStillGood = FALSE;
        long linkEnd = autoLink.applied_range.end + 1 + length;

        tempSel.begin = autoLink.applied_range.begin;
        tempSel.end = sel.begin;

        GetPgText(buf, sizeof(buf), tempSel, FALSE);
        firstURLStillGood = ValidURLScheme(buf);

        if (firstURLStillGood)
        {
            autoLink.applied_range.end = sel.begin;

            if ( URLIsValid(buf) )
            {
                char tempBuf[INTERNET_MAX_URL_LENGTH] = "";
                strncat(tempBuf, EscapeURL(buf), INTERNET_MAX_URL_LENGTH-1 );
                pgChangeHyperlinkSource(m_paigeRef, autoLink.applied_range.begin, 
                                        &autoLink.applied_range, (unsigned char*)tempBuf, NULL, NULL, 
                                        NULL, NULL, NULL, best_way);
            }
        }
                
        tempSel.begin = sel.begin + 1 + length;
        tempSel.end = linkEnd;
        GetPgText(buf, sizeof(buf), tempSel, FALSE);
        secondURLStillGood = ValidURLScheme(buf);

        if (secondURLStillGood)
        {
            if ( URLIsValid(buf) )
            {
                if (firstURLStillGood)
                {
                    paige_rec_ptr pgPtr = (paige_rec_ptr) UseMemory(m_paigeRef);
                    autoLink.unique_id = pgAssignLinkID(pgPtr->hyperlinks);
                    UnuseMemory(m_paigeRef);
                }
                else 
                    pgDeleteHyperlinkSource(m_paigeRef, autoLink.applied_range.begin, TRUE);

                autoLink.applied_range = tempSel;
                char tempBuf[INTERNET_MAX_URL_LENGTH] = "";
                strncpy(tempBuf, EscapeURL(buf), INTERNET_MAX_URL_LENGTH-1 );
                pgChangeHyperlinkSource(m_paigeRef, autoLink.applied_range.begin, 
                                        &autoLink.applied_range, (unsigned char *)tempBuf, NULL, NULL, 
                                        NULL, NULL, NULL, best_way);
            }
                
        }

        if (!firstURLStillGood && !secondURLStillGood)
            pgDeleteHyperlinkSource(m_paigeRef, sel.begin, TRUE);
    }

    // If new-line or linefeed char, may have to scroll
//    if ((uChar == PgGlobalsPtr()->line_wrap_char) || (uChar == PgGlobalsPtr()->soft_line_char))
//    {
//        ScrollToCursor();
//    }
//    else {    // Temp fix, should really scroll only during word-wrap
    ScrollToCursor();
//    }

    CView::OnChar(nChar, nRepCnt, nFlags);

	// if we've changed the doc, recheck the current word
	if (oldChangeCount!=pgGetChangeCtr(m_paigeRef)) 
		m_styleEx->ClearMisspellingAndRecheck();
	// if we've changed which word is current, go check stuff
	else if (!m_checkdastuff)
		m_checkdastuff = InDifferentWord(m_paigeRef,&oldSel);
}


void CPaigeEdtView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    TCHAR uChar;
    BOOL callPaige = FALSE;
    long selectLeft, selectRight, wordLeft, wordRight, lineRight, lineLeft, paraBegin, paraEnd;
    BOOL ControlDown, ShiftDown, gottaScroll = FALSE;
    co_ordinate curCoOrd;
    CPoint curPoint;
    long anchor, scrollOffset = CURRENT_POSITION;
	long oldChangeCount = pgGetChangeCtr(m_paigeRef);
	select_pair oldSel;

    // Map the virtual keycode to paige default values for non-ascii
    //     chars. Also check if the CONTROL and SHIFT keys are down.
    ControlDown = (GetKeyState(VK_CONTROL) < 0);
    ShiftDown = (GetKeyState(VK_SHIFT) < 0);

    //
    // Handle navigation keys for read-only views, *before* we do any 
    // Paige memory allocations and other nasty stuff.
    //
    if (m_fRO)
    {
        BOOL PlainArrows = GetIniShort(IDS_INI_PLAIN_ARROWS) != 0;
        BOOL ControlArrows = GetIniShort(IDS_INI_CONTROL_ARROWS) != 0;

        switch (nChar)
        {
        case VK_LEFT:
        case VK_UP:
            if ((PlainArrows && !ControlDown) ||
                (ControlArrows && ControlDown))
            {
                SendMessage(WM_COMMAND, ID_PREVIOUS_MESSAGE);
                return;
            }
            break;
        case VK_DOWN:
        case VK_RIGHT:
            // Check for special navigation keys
            if ((PlainArrows && !ControlDown) ||
                (ControlArrows && ControlDown))
            {
                SendMessage(WM_COMMAND, ID_NEXT_MESSAGE);
                return;
            }
            break;
        default:
            // fall through to normal Paige keystroke handling below...
            break;
        }
    }

	pgGetSelection(m_paigeRef,&oldSel.begin,&oldSel.end);	// keep track of old selection SD 2/22/99

   {   // BOG: see if anything needs spell-checking
        if ( !ShiftDown /* && !m_fRO */ ) {
            CPaigeStyle objStyle( m_paigeRef );

            select_pair textRange;
            pgGetSelection( m_paigeRef, &textRange.begin, &textRange.end );
            long offset = textRange.begin;

            if ( nChar == VK_LEFT || nChar == VK_RIGHT ) {
                bool bToLeft = (nChar == VK_LEFT);

                // for vk_right, we hack textrange to point to previous char
                if ( !bToLeft )
                    textRange.begin = textRange.end = (offset - nRepCnt < 0) ? 0 : offset - nRepCnt;

                if ( objStyle.NeedsSpellCheck( &textRange ) )
                    if ( chartype_in_range( m_paigeRef, BLANK_BIT, offset, nRepCnt, bToLeft ) )
                        m_checkdastuff = true;
            }
            else if ( nChar == VK_UP || nChar == VK_DOWN ) {

                // for up/down we hack textrange to point to previous char if the next char
                // is a blank; otherwise, word won't get checked.
                if ( chartype_in_range( m_paigeRef, BLANK_BIT, offset, 1, false ) )
                    textRange.begin = textRange.end = (offset - nRepCnt < 0) ? 0 : offset - nRepCnt;

                if ( objStyle.NeedsSpellCheck( &textRange ) )
                    m_checkdastuff = true;
            }
        }
    }   // BOG: end spellcheck hack

        
    paige_rec_ptr pgPtr  = NULL;
    pgPtr = (paige_rec_ptr) UseMemory(m_paigeRef);

    pgGetSelection(m_paigeRef, &selectLeft, &selectRight);

    if (pgNumSelections(m_paigeRef)) {
        //Get the original anchor which started the selection
        //pgPtr = (paige_rec_ptr) UseMemory(m_paigeRef);
        anchor = pgPtr->hilite_anchor;
    }
    else anchor = selectRight;  //just the plain ol' caret

    // Map the virtual keycode to paige default values for non-ascii chars
    switch(nChar)
    {
    case VK_LEFT : 
        // Custom FindWord, uses Paige's pgFindWord, left_side and smart_select MUST be TRUE
        if (ControlDown) {
            if (selectRight > anchor)
                FindWord(m_paigeRef, selectRight, &wordLeft, &wordRight, TRUE, TRUE);
            else
                FindWord(m_paigeRef, selectLeft, &wordLeft, &wordRight, TRUE, TRUE);
            scrollOffset = wordLeft;
        }

        if (ShiftDown) {
            //Select to end of word (CTRL+SHIFT) or one char (SHIFT)
            if (ControlDown) {
                if (selectRight > anchor) //DeSelect
                {
                    pgSetSelection(m_paigeRef, selectLeft, wordLeft, 0, TRUE);
                    pgPtr->hilite_anchor = selectLeft;
                }
                else
                    pgExtendSelection(m_paigeRef, wordLeft - selectLeft, EXTEND_MOD_BIT, TRUE);
            }
            else {                                      
                pgInsert(m_paigeRef, (pg_byte_ptr)&(PgGlobalsPtr()->left_arrow_char), 1, 
                         CURRENT_POSITION, key_insert_mode, EXTEND_MOD_BIT, best_way );
                if (selectRight > anchor)
                    scrollOffset = selectRight - 1;
                else 
                    scrollOffset = selectLeft - 1;
            }
        }
        else if (ControlDown)
            pgSetSelection(m_paigeRef, wordLeft, wordLeft, 0, TRUE);
        else {uChar = (TCHAR) PgGlobalsPtr()->left_arrow_char/*0x1C*/;  callPaige = TRUE;}
        gottaScroll = TRUE; break;
        
    case VK_RIGHT :             
        //Testing
        /*pgFindWord(m_paigeRef, selectRight, &wordLeft, &wordRight, FALSE, TRUE);
          pgFindWord(m_paigeRef, selectRight, &wordLeft, &wordRight, FALSE, FALSE);
          pgFindWord(m_paigeRef, selectRight, &wordLeft, &wordRight, TRUE, TRUE);
          pgFindWord(m_paigeRef, selectRight, &wordLeft, &wordRight, TRUE, FALSE);*/

        // Edited FindWord (uses Paige's pgFindWord)
        // Get word boundaries, Select trailing blanks also to make it look
        //       like we are navigating to beginning of next word instead of end of current word
        // left_side MUST be FALSE, smart_select MUST be TRUE for the function
        if (ControlDown) {
            if (selectLeft < anchor)
                FindWord(m_paigeRef, selectLeft, &wordLeft, &wordRight, FALSE, TRUE);
            else
                FindWord(m_paigeRef, selectRight, &wordLeft, &wordRight, FALSE, TRUE);
            scrollOffset = wordRight;
        }

        if (ShiftDown) {
            if (ControlDown) {                          
                if (selectLeft < anchor) //DeSelect
                {
                    pgSetSelection(m_paigeRef, selectRight, wordRight, 0, TRUE);                                        
                    pgPtr->hilite_anchor = selectRight;
                }
                else 
                    pgExtendSelection(m_paigeRef, wordRight - selectRight, EXTEND_MOD_BIT, TRUE);                                                                                               
            }
            else {                              
                pgInsert(m_paigeRef, (pg_byte_ptr)&(PgGlobalsPtr()->right_arrow_char), 1, 
                         CURRENT_POSITION, key_insert_mode, EXTEND_MOD_BIT, best_way );
                if (selectLeft < anchor)
                    scrollOffset = selectLeft + 1;
                else 
                    scrollOffset = selectRight + 1;
            }
        }
        else if (ControlDown) 
            pgSetSelection(m_paigeRef, wordRight, wordRight, 0, TRUE);
        else {  uChar = (TCHAR) PgGlobalsPtr()->right_arrow_char/*0x1D*/; callPaige = TRUE; }
        gottaScroll = TRUE; break;
        
    case VK_UP : 
        if (ControlDown) {
            if (selectRight > anchor) {
                pgFindPar(m_paigeRef, selectRight, &paraBegin, &paraEnd);
                if (paraBegin == selectRight)
                    pgFindPar(m_paigeRef, selectRight-1, &paraBegin, &paraEnd);
                if (paraBegin != 0) 
                    scrollOffset = paraBegin - 1;
            }
            else {
                if (selectLeft == 0) break;
                pgFindPar(m_paigeRef, selectLeft, &paraBegin, &paraEnd);
                                //If we are already at the beginning of a paragraph, go to the previous one
                if (paraBegin == selectLeft)
                    pgFindPar(m_paigeRef, selectLeft-1, &paraBegin, &paraEnd);
                scrollOffset = paraBegin;
            }
        }

        if (ShiftDown) {
            // Select to beginning of the paragraph
            if (ControlDown) {
                if (selectRight > anchor) {//DeSelect
                    if (selectLeft > paraBegin)
                        pgSetSelection(m_paigeRef, paraBegin, selectLeft, 0, TRUE);
                    else
                        pgSetSelection(m_paigeRef, selectLeft, paraBegin, 0, TRUE);
                                
                    pgPtr->hilite_anchor = selectLeft;
                }
                else
                    pgExtendSelection(m_paigeRef, paraBegin - selectLeft, EXTEND_MOD_BIT, TRUE);
            }
            else
            {
                pgInsert(m_paigeRef, (pg_byte_ptr)&(PgGlobalsPtr()->up_arrow_char), 1, 
                         CURRENT_POSITION, key_insert_mode, EXTEND_MOD_BIT, best_way );
                long selLeft, selRight;
                pgGetSelection(m_paigeRef, &selLeft, &selRight);
                                //If it's stuck in the middle of the first line, select to the beginning
                if ((selRight == selectRight) && (selLeft == selectLeft))
                {
                    if ( selLeft != 0 )
                    {
                        if (selLeft <= pgPtr->hilite_anchor)
                            pgExtendSelection(m_paigeRef, -selLeft, EXTEND_MOD_BIT, TRUE);
                    }
                    else
                    {
                        if ( selRight > pgPtr->hilite_anchor )
                            pgSetSelection(m_paigeRef, 0, 0, 0, TRUE);
                    }
                }
                if (selRight > anchor)
                    scrollOffset = selRight;
                else 
                    scrollOffset = selLeft;
            }
        }
        else if (ControlDown) {
            // Navigate to beginning of the Current paragraph
            pgSetSelection(m_paigeRef, paraBegin, paraBegin, 0, TRUE);
        }
        else { uChar = (TCHAR) PgGlobalsPtr()->up_arrow_char/*0x1E*/; callPaige = TRUE;}
        gottaScroll = TRUE; break;
        
    case VK_DOWN : 
        if (ControlDown) {
            if (selectLeft < anchor)
                pgFindPar(m_paigeRef, selectLeft, &paraBegin, &paraEnd);
            else
                pgFindPar(m_paigeRef, selectRight, &paraBegin, &paraEnd);
            scrollOffset = paraEnd;
        }

        if (ShiftDown) {
            // Select to the end of the current paragraph
            if (ControlDown) {
                if (selectLeft < anchor) { //DeSelect
                    if (paraEnd > selectRight)
                        pgSetSelection(m_paigeRef, selectRight, paraEnd, 0, TRUE);
                    else
                        pgSetSelection(m_paigeRef, paraEnd, selectRight, 0, TRUE);

                    pgPtr->hilite_anchor = selectRight;
                }
                else
                    pgExtendSelection(m_paigeRef, paraEnd - selectRight, EXTEND_MOD_BIT, TRUE);
            }
            else
            {
                pgInsert(m_paigeRef, (pg_byte_ptr)&(PgGlobalsPtr()->down_arrow_char), 1, 
                         CURRENT_POSITION, key_insert_mode, EXTEND_MOD_BIT, best_way );
                long selLeft, selRight;
                pgGetSelection(m_paigeRef, &selLeft, &selRight);
                if ((selRight == selectRight) && (selLeft == selectLeft))
                {
                    long textSize = pgTextSize(m_paigeRef);
                    if ( selRight != textSize )
                    {
                        if (selRight >= pgPtr->hilite_anchor)
                            pgExtendSelection(m_paigeRef, textSize, EXTEND_MOD_BIT, TRUE);
                    }
                    else
                    {
                        if ( selLeft < pgPtr->hilite_anchor )
                            pgSetSelection(m_paigeRef, textSize, textSize, 0, TRUE);
                    }                                   
                }
                if (selLeft < anchor)
                    scrollOffset = selLeft;
                else 
                    scrollOffset = selRight;
            }
        }
        else if (ControlDown) {
            // Navigate to the beginning of the next paragraph
            pgSetSelection(m_paigeRef, paraEnd, paraEnd, 0, TRUE);
        }
        else { uChar = (TCHAR) PgGlobalsPtr()->down_arrow_char/*0x1F*/; callPaige = TRUE; }
        gottaScroll = TRUE; break; 
        
    case VK_DELETE : 
        if (m_fRO) break;
        if (ControlDown) {
            select_pair selectPair;
            //Delete word to the Right
            FindWord(m_paigeRef, selectRight, &selectPair.begin, &selectPair.end, FALSE, TRUE);
            selectPair.begin = selectRight;
            if (pgNumSelections(m_paigeRef))
                CleanURLonPosteriorDelete(selectLeft);
            PrepareUndo(undo_delete, (long *)&selectPair);
            pgDelete(m_paigeRef, &selectPair, best_way);
        }
        else 
        {
            uChar = (TCHAR) PgGlobalsPtr()->fwd_delete_char;/*0x7F*/; 
            if (pgNumSelections(m_paigeRef))
            {
                CleanURLonPosteriorDelete(selectLeft);
                PrepareUndo(undo_delete);
            }
            else
                PrepareUndo(undo_fwd_delete);
            callPaige = TRUE; 
        }
        gottaScroll=TRUE;
        //UpdateScrollBars(/*GetScrollBarCtrl(SB_VERT)*/);
        break;

    case VK_BACK :
        if (m_fRO) break;
        //Trap Ctrl_BackSpace here, Backspace itself is trapped in OnChar
        if (ControlDown) {
            select_pair selectPair;
            //Delete word from the current position to the beginning of the current word
            FindWord(m_paigeRef, selectLeft, &selectPair.begin, &selectPair.end, TRUE, TRUE);
            selectPair.end = selectLeft;
            if (pgNumSelections(m_paigeRef))
                CleanURLonPosteriorDelete(selectLeft);
            PrepareUndo(undo_delete, (long *)&selectPair);
            pgDelete(m_paigeRef, &selectPair, best_way);
        }
        break;

    case VK_END : 
        if (ShiftDown)
        {
            if (ControlDown) {
                if (selectLeft < anchor)
                    pgSetSelection(m_paigeRef, selectRight, pgTextSize(m_paigeRef), 0, TRUE);
                else
                    pgSetSelection(m_paigeRef, selectLeft, pgTextSize(m_paigeRef), 0, TRUE);

                scrollOffset = pgTextSize(m_paigeRef);
            }
            else {
                pgSetSelection(m_paigeRef, selectRight, selectRight, 0, TRUE);
                pgSetCaretPosition(m_paigeRef, end_line_caret, TRUE);
                long currPos;
                pgGetSelection(m_paigeRef, &currPos, MEM_NULL);
                if (selectLeft < anchor)
                    pgSetSelection(m_paigeRef, selectRight, currPos, 0, TRUE);
                else
                    pgSetSelection(m_paigeRef, selectLeft, currPos, 0, TRUE);
                                //Reset the hilite anchor to the old one, since it's confused by now!!
                pgPtr->hilite_anchor = anchor;
            }
        }
        else if (ControlDown) {
            pgSetCaretPosition(m_paigeRef, doc_bottom_caret, TRUE);
            pgPtr->hilite_anchor = pgTextSize(m_paigeRef);
        }
        else {
            if ( ( pgNumSelections(m_paigeRef) ) /*|| ( selectRight != pgTextSize(m_paigeRef) ) */)
            {
                                //pgSetCaretPosition(m_paigeRef, end_line_caret, TRUE);
                pgFindLine(m_paigeRef, selectRight-1, &lineLeft, &lineRight);
                if (lineRight != pgTextSize(m_paigeRef))
                    --lineRight;
                pgSetSelection(m_paigeRef, lineRight, lineRight, 0, TRUE);
            }
            else if ( selectRight != pgTextSize(m_paigeRef) )
                pgSetCaretPosition(m_paigeRef, end_line_caret, TRUE);
            pgGetSelection(m_paigeRef, MEM_NULL, &selectRight);
            pgPtr->hilite_anchor = selectRight;
        }
        gottaScroll = TRUE;
        break;
        
    case VK_HOME :
        if (ShiftDown)
        {
            if (ControlDown) {
                if (selectRight > anchor)
                    pgSetSelection(m_paigeRef, 0, selectLeft, 0, TRUE);
                else 
                    pgSetSelection(m_paigeRef, 0, selectRight, 0, TRUE);
                scrollOffset = 0;
            }
            else {
                pgFindLine(m_paigeRef, selectLeft, &lineLeft, &lineRight);
                if (selectRight > anchor)
                    pgSetSelection(m_paigeRef, lineLeft, selectLeft, 0, TRUE);
                else
                {       //
                    // pgFindLine returns end-of-text values if caret is last character in msg.
                    //
                    if ( (lineLeft == selectLeft) && (selectLeft == pgTextSize(m_paigeRef)) )
                    {
                        select_pair sel;
                        pgSetCaretPosition(m_paigeRef, begin_line_caret, TRUE);
                        pgGetSelection(m_paigeRef, &sel.begin, &sel.end);
                        pgSetSelection(m_paigeRef, selectRight, sel.begin, 0, TRUE);
                    }
                    else
                        pgExtendSelection(m_paigeRef, lineLeft - selectLeft, EXTEND_MOD_BIT, TRUE);
                }
            }
        }
        else if (ControlDown) {
            pgSetCaretPosition(m_paigeRef, home_caret, TRUE);
            pgPtr->hilite_anchor = 0;
        }
        else 
        {
            if ( pgNumSelections(m_paigeRef) )
            {
                pgFindLine(m_paigeRef, selectLeft, &lineLeft, &lineRight);
                pgSetSelection(m_paigeRef, lineLeft, lineLeft, 0, TRUE);
            }
            else
            {
                pgSetCaretPosition(m_paigeRef, begin_line_caret, TRUE);
                pgGetSelection(m_paigeRef, &selectLeft, MEM_NULL);
                pgPtr->hilite_anchor = selectLeft;
            }
        }
        gottaScroll = TRUE;
        break;

    case VK_PRIOR :
    {
        long offset;

        if ( pgNumSelections(m_paigeRef) )
        {
            if (selectLeft == anchor)
                pgSetSelection(m_paigeRef, selectRight, selectRight, 0, TRUE);
            else
                pgSetSelection(m_paigeRef, selectLeft, selectLeft, 0, TRUE);
        }

        curPoint = GetCaretPos();
        //Fill Paige coord struct
        CPointToPgPoint(&curPoint, &curCoOrd);
                        
        // Safe to pass 0 for nPos since it's not gonna be used unless 1st arg is SB_THUMBTRACK
        OnVScroll(SB_PAGEUP, 0, GetScrollBarCtrl(SB_VERT));
                        
        offset = pgPtToChar(m_paigeRef, &curCoOrd, NULL);
                        
        // No more pages above, set cursor to the beginning of the text
        if (offset == selectLeft)
            offset = 0;

        if (ShiftDown)
        {
            if ( !pgPtr ) 
                pgPtr = (paige_rec_ptr) UseMemory(m_paigeRef);

            if (selectRight > anchor) //Deselecting
            {
                if (offset == selectRight)
                    offset = 0;
                pgSetSelection(m_paigeRef, selectLeft, offset, 0, TRUE);
                pgPtr->hilite_anchor = selectLeft;
            }
            else
            {
                pgSetSelection(m_paigeRef, selectRight, offset, 0, TRUE);
                pgPtr->hilite_anchor = selectRight;
            }
        }               
        else
            pgSetSelection(m_paigeRef, offset, offset, 0, TRUE);
                        
        //SetPgAreas();

        break;
    }
    case VK_NEXT :
    {
        long offset;

        if ( pgNumSelections(m_paigeRef) )
        {
                                //Might cause a flicker but this is the only way to get a caret.
            if (selectLeft == anchor)
                pgSetSelection(m_paigeRef, selectRight, selectRight, 0, TRUE);
            else
                pgSetSelection(m_paigeRef, selectLeft, selectLeft, 0, TRUE);
        }

        curPoint = GetCaretPos();
        //Fill Paige coord struct
        CPointToPgPoint(&curPoint, &curCoOrd);

        OnVScroll(SB_PAGEDOWN, 0, GetScrollBarCtrl(SB_VERT));
                        
        offset = pgPtToChar(m_paigeRef, &curCoOrd, NULL);
                        
        // No more pages below, set cursor to the end of the text
        if (offset == selectRight)
            offset = pgTextSize(m_paigeRef);

        if (ShiftDown)
        {
            if ( !pgPtr ) 
                pgPtr = (paige_rec_ptr) UseMemory(m_paigeRef);

            if (selectLeft < anchor) //Deselect
            {
                if (offset == selectLeft)
                    offset = pgTextSize(m_paigeRef);
                pgSetSelection(m_paigeRef, selectRight, offset, 0, TRUE);
                pgPtr->hilite_anchor = selectRight;
            }
            else
            {
                pgSetSelection(m_paigeRef, selectLeft, offset, 0 , TRUE);
                pgPtr->hilite_anchor = selectLeft;
            }
        }
        else
            pgSetSelection(m_paigeRef, offset, offset, 0, TRUE);

        //Make sure you don't see half of the cursor, 
        // incase cursor is on the first line in the vis-area
        //gottaScroll = TRUE;
        //SetPgAreas();

        break;
    }
    }

    if (callPaige == TRUE)
    {
        while (nRepCnt--)
            pgInsert( m_paigeRef, (pg_byte_ptr)&uChar, sizeof(unsigned char), 
                      CURRENT_POSITION, key_insert_mode, 0, best_way );
    }
        
 	// if we've changed the doc, recheck the current word
	if (oldChangeCount!=pgGetChangeCtr(m_paigeRef)) 
		m_styleEx->ClearMisspellingAndRecheck();
	// if we've changed which word is current, go check stuff
	else if (!m_checkdastuff)
		m_checkdastuff = InDifferentWord(m_paigeRef,&oldSel);
	
	if (gottaScroll) ScrollToCursor(scrollOffset);
        
    if (pgPtr) UnuseMemory(m_paigeRef);
    //CView::OnKeyDown(nChar, nRepCnt, nFlags);

    // BOG: Text selections resulting from find operations remain visible even when the
    // editor window loses focus, but this causes user selections to also behave this
    // way unless we employ some criteria for canceling out of this mode. Here it is.
    // I think this is a reasonably complete set---add more if ya think of 'em!

    if ( (uChar == VK_LEFT || uChar == VK_RIGHT || uChar == VK_UP || uChar == VK_DOWN) ||
         (!m_fRO && (uChar == VK_RETURN || uChar == VK_TAB || isascii(uChar))) ) {

        m_bDontDeactivateHighlight = FALSE;
    }
}

// Has the insertion point moved to a new word?
// This is really only meant to work if the document
// hasn't changed since oldSel was taken; if you call
// it after changing the document, phhhhttttt!!!! to you.  SD 2/24/99
bool InDifferentWord(pg_ref paigeRef, select_pair_ptr oldSel)
{
	select_pair newSel;
	bool result = false;

	// get current selection
	pgGetSelection(paigeRef,&newSel.begin,&newSel.end);

	//SDSpellTRACE("raw %d.%d %d.%d ",oldSel->begin,oldSel->end,newSel.begin,newSel.end);

	// we only do this if the selection is an insertion point
	// we can only be in a different word if the selection has changed
	if (newSel.begin==newSel.end && (newSel.begin!=oldSel->begin || newSel.end!=oldSel->end))
	{
		// ok, we have an insertion point and it's not in the same place as it was

		// expand the offsets to word boundaries
		pgFindWord(paigeRef, newSel.begin, &newSel.begin,&newSel.end,TRUE,FALSE);
		pgFindWord(paigeRef, oldSel->begin, &oldSel->begin,&oldSel->end,TRUE,FALSE);
		// TRACE("SD word %d.%d %d.%d ",oldSel->begin,oldSel->end,newSel.begin,newSel.end);

		// Are we in a new spot? 
		if (newSel.begin!=oldSel->begin)
			result = true;
	}

	//SDSpellTRACE(" %s\n",result?"diff":"same");

	return result;
}

// Get the offsets of a single word that either contains the selection or
// has the selection on its ends.  Returns true if there is such a single word,
// and fills in theWord with its coordinates.  If there is no such word, returns
// false and puts -1,-1 into theWord
bool GetWordContainingSelection(pg_ref paigeRef,select_pair_ptr theWord)
{
	select_pair beginWord, endWord;

	// get the selection
	pgGetSelection(paigeRef,&theWord->begin,&theWord->end);
	//SDSpellTRACE("GWCS %d.%d",theWord->begin,theWord->end);

	// and figure out the word it starts in
	pgFindWord(paigeRef,theWord->begin,&beginWord.begin,&beginWord.end,true,false);

	// if it's not an insertion point, figure out the word it ends in
	if (theWord->begin != theWord->end)
	{
		pgFindWord(paigeRef,theWord->end,&endWord.begin,&endWord.end,true,false);

		// if the words aren't the same, bail!
		if (endWord.begin != beginWord.begin)
		{
			theWord->begin = theWord->end = -1;
			//SDSpellTRACE(" false\n",theWord->begin,theWord->end);
			return false;
		}
	}

	// Ok, we found a word!
	*theWord = beginWord;
	//SDSpellTRACE(" %d.%d true\n",theWord->begin,theWord->end);
	return true;
}

// Do two selection pairs intersect?
bool SelPairIntersects(select_pair_ptr sel1, select_pair_ptr sel2)
{
	if (sel1->end <= sel2->begin) return false;
	if (sel1->begin >= sel2->end) return false;
	return true;
}

void CPaigeEdtView::ScrollToCursor(long offset)
{
    if (offset == CURRENT_POSITION)
    {
        CPoint curPoint;
        //Get current caret position (client co_ords)
        curPoint = GetCaretPos();

        long left;
        rectangle rect;
        pgGetSelection(m_paigeRef, &left, MEM_NULL);
        pgCharacterRect(m_paigeRef, left, FALSE, FALSE, &rect);
        CRect clientRect;
        GetClientRect(&clientRect);
        clientRect.DeflateRect(m_pgBorder);
        if (rect.bot_right.v > clientRect.bottom )
            curPoint.y = rect.bot_right.v;

        //Fill Paige coord struct
        co_ordinate curCoOrd;
        CPointToPgPoint(&curPoint, &curCoOrd);

        //Do we need to scroll?
        short posBits = 0;
        posBits = pgPtInView(m_paigeRef, &curCoOrd, NULL);
        if (!(posBits & WITHIN_VIS_AREA))
        {
            if (!pgNumSelections(m_paigeRef)) {
                pgPaginateNow(m_paigeRef, CURRENT_POSITION, FALSE);
                if (pgScrollToView(m_paigeRef, CURRENT_POSITION, 0, 0, TRUE, best_way)) {
                    UpdateScrollBars();
                }
            }
            else UpdateScrollBars();
        }
        else
            UpdateScrollBars();
    }
    else 
    {
        pgPaginateNow(m_paigeRef, offset, FALSE);
        if (pgScrollToView(m_paigeRef, offset, 0, 0, TRUE, best_way)) {
            UpdateScrollBars();
        }

    }
}

void CPaigeEdtView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
    // BOG: this is a "re-entrant" hack. if we get called again in the middle
    // of hyperlink processing, bad things can/will happen.
    {
        paige_rec_ptr pgRec = (paige_rec_ptr) UseMemory( m_paigeRef );
        PgStuffBucket* pSB = (PgStuffBucket*) pgRec->user_refcon;
        UnuseMemory( m_paigeRef );

        if ( pSB->hlinkProcessing == true ) {
            assert( 0 );
            return;
        }
    }   // BOG: end re-entrant prophylactic

    co_ordinate pgMousePt;
    CPointToPgPoint(&point, &pgMousePt);

    SetCapture();
    m_pgModifiers |= WORD_MOD_BIT;
    pgDragSelect(m_paigeRef, &pgMousePt, mouse_down, m_pgModifiers, 0, FALSE);

        //Set Timer to catch the third click, if any
    m_mouseState |= ( MOUSE_TRIPLE_CLICKED | MOUSE_DRAG_SELECT );
    if (!SetTimer(TRIPLE_CLICK_TIMER, GetDoubleClickTime(), NULL)) 
        m_mouseState &= ~MOUSE_TRIPLE_CLICKED;

    long selectLeft;
    pgGetSelection(m_paigeRef, &selectLeft, MEM_NULL);

    paige_rec_ptr pgPtr = (paige_rec_ptr) UseMemory( m_paigeRef );
    pgPtr->hilite_anchor = selectLeft;
    UnuseMemory(m_paigeRef);
        
    ReleaseCapture();
    CView::OnLButtonDblClk(nFlags, point);
}

void CPaigeEdtView::OnKillFocus(CWnd* pNewWnd) 
{
    // Deactivate the cursor when window loses focus
    if (!m_bDontDeactivateHighlight)
        pgSetHiliteStates(m_paigeRef, deactivate_verb, no_change_verb, TRUE);
    CView::OnKillFocus(pNewWnd);
}

void CPaigeEdtView::OnSetFocus(CWnd* pOldWnd) 
{
    // Activate the cursor when window gets focus
    pgSetHiliteStates(m_paigeRef, activate_verb, no_change_verb, TRUE); 
    CView::OnSetFocus(pOldWnd);
}

void CPaigeEdtView::OnEditCut() 
{
    OnEditCopy();
    PrepareUndo(undo_delete);
    // Delete the current selection since it's a CUT operation
    pgDelete(m_paigeRef, NULL, best_way);
    ScrollToCursor();
}

void CPaigeEdtView::OnEditCopy() 
{
    COleDataSource *clipBoardStuff = new COleDataSource();
    OnCopy(clipBoardStuff);
}

void CPaigeEdtView::OnCopy(COleDataSource *clipBoardStuff)
{
    // setup the stuff bucket so that any embeds get saved in whatever
    // alternate formats are available. note that embeds only get rendered in
    // additional formats if they are explicitly selected.

    paige_rec_ptr pgRec = (paige_rec_ptr) UseMemory( m_paigeRef );
    PgStuffBucket* pSB = (PgStuffBucket*) pgRec->user_refcon;
    UnuseMemory( m_paigeRef );
    assert( pSB->bOleExport == false );
    assert( pSB->pDS == NULL );

    if ( clipBoardStuff ) {
        pSB->bOleExport = true;
        pSB->pDS = clipBoardStuff;
        PgDataTranslator theTxr( m_paigeRef );
        theTxr.ExportData( clipBoardStuff, PgDataTranslator::kFmtText );

		// If Shift key is down, then only put plain text on clipboard
        if (GetKeyState(VK_SHIFT) >= 0)
        {
            theTxr.ExportData( clipBoardStuff, PgDataTranslator::kFmtRtf );
            theTxr.ExportData( clipBoardStuff, PgDataTranslator::kFmtNative );
            theTxr.ExportData( clipBoardStuff, PgDataTranslator::kFmtHtml );
        }
        clipBoardStuff->SetClipboard();
    }

    // make sure that these are "dead"
    pSB->bOleExport = false;
    pSB->pDS = NULL;
}


void CPaigeEdtView::OnEditPaste() 
{
    COleDataObject clipBoardData;
    VERIFY(clipBoardData.AttachClipboard());
    OnPaste(&clipBoardData, undo_ole_paste);
}

void CPaigeEdtView::OnEditPasteSpecial()
{
	COlePasteSpecialDialog dlg;

	dlg.AddFormat(CF_TEXT, TYMED_HGLOBAL, AFX_IDS_TEXT_FORMAT, FALSE, FALSE);
	dlg.AddFormat(PgDataTranslator::kFmtRtf, TYMED_HGLOBAL, AFX_IDS_RTF_FORMAT, FALSE, FALSE);
	dlg.AddFormat(PgDataTranslator::kFmtNative, TYMED_HGLOBAL, IDS_PAIGE_FORMAT, FALSE, FALSE);
	dlg.AddFormat(CF_DIB, TYMED_HGLOBAL, AFX_IDS_DIB_FORMAT, FALSE, FALSE);
	dlg.AddFormat(CF_BITMAP, TYMED_GDI, AFX_IDS_BITMAP_FORMAT, FALSE, FALSE);
	// HTML last because it's the most likely to be screwed up
	dlg.AddFormat(PgDataTranslator::kFmtHtml, TYMED_HGLOBAL, IDS_HTML_FORMAT, FALSE, FALSE);

	if (dlg.DoModal() == IDOK)
	{
		COleDataObject clipBoardData;
		CLIPFORMAT cf = dlg.m_ps.arrPasteEntries[dlg.m_ps.nSelectedIndex].fmtetc.cfFormat;

		VERIFY(clipBoardData.AttachClipboard());

		OnPaste(&clipBoardData, undo_ole_paste, cf);
	}
}

void CPaigeEdtView::OnPaste(COleDataObject *clipBoardData, short undoVerb /*= 0*/, PgDataTranslator::FormatType clipFormat /*= 0*/) 
{
    PgDataTranslator theTxr( m_paigeRef );

	if (clipFormat == CF_DIB)
		clipFormat = CF_BITMAP;
	else if (clipFormat == 0)
	{
		// Shift down indicates a plain paste, but not if it's Shift + Insert,
		// which is the ancient accelerator for doing paste.
		if ((GetKeyState(VK_SHIFT) < 0) && (GetKeyState(VK_INSERT) >= 0) &&
			clipBoardData->IsDataAvailable(CF_TEXT, NULL))
		{
			clipFormat = CF_TEXT;
		}
        
		else if ((clipBoardData->IsDataAvailable((short)PgDataTranslator::kFmtNative, NULL)) && (pgTextSize(m_paigeRef)))
			clipFormat = PgDataTranslator::kFmtNative;

//		else if (clipBoardData->IsDataAvailable((short)PgDataTranslator::kFmtHtml, NULL))
//			clipFormat = PgDataTranslator::kFmtHtml;
                
		else if (clipBoardData->IsDataAvailable((short)PgDataTranslator::kFmtRtf, NULL))
			clipFormat = PgDataTranslator::kFmtRtf;
        
		else if (clipBoardData->IsDataAvailable(CF_TEXT, NULL))
			clipFormat = CF_TEXT;
		else if ( clipBoardData->IsDataAvailable( CF_BITMAP, NULL ) )
			clipFormat = CF_BITMAP;
	}

    //Pasting over all text should not use Native format 'cos it crashes
    if (clipFormat == PgDataTranslator::kFmtNative)
    {
        long selRight, selLeft;

        pgGetSelection(m_paigeRef, &selLeft, &selRight);
        if ( (selLeft == 0) && (selRight == pgTextSize(m_paigeRef)) )
        {
            if (clipBoardData->IsDataAvailable((short)PgDataTranslator::kFmtRtf, NULL))
                clipFormat = PgDataTranslator::kFmtRtf;
            else 
                if (clipBoardData->IsDataAvailable(CF_TEXT, NULL))
                    clipFormat = CF_TEXT;
                else
                    clipFormat = 0;
        }
    }

    if (clipFormat) {
        // Paige implementation doesn't seem to handle selected text correctly, 
        //  so let's perform the deletion ourselves.
        if (pgNumSelections(m_paigeRef))
        {
            if (!(m_mouseState & MOUSE_DRAG_DROP))
            {
                PrepareUndo(undo_delete, MEM_NULL, TRUE);
                pgDelete(m_paigeRef, NULL, best_way);
            }
            switch(undoVerb)
            {
            case undo_ole_paste : 
                undoVerb = undo_ole_paste_delete; 
                break;
            case undo_drop : 
                //if (m_mouseState & MOUSE_DRAG_DROP) 
                undoVerb = undo_drag_drop_move ; 
                break;
            }
        }
        long insert_ref[2], first, last;
        pgGetSelection(m_paigeRef, &first, &last);
        insert_ref[0] = first;
        first = pgTextSize(m_paigeRef);

        // BOG: here's a hack to handle bitmaps off the clipboard. don't know
        // if we should be doing CF_DIB as well; i suspect that we should :)
        if ( clipFormat == CF_BITMAP ) {
            STGMEDIUM stg;
            clipBoardData->GetData( CF_BITMAP, &stg );
            PgInsertImageFromBitmap( m_paigeRef, (HBITMAP)stg.hBitmap );
            Invalidate();
        }
        else {
            theTxr.ImportData( clipBoardData, clipFormat );
        }

        last = pgTextSize(m_paigeRef);
        insert_ref[1] = last - first;
        PrepareUndo(undoVerb, insert_ref);


        // Any AutoURLs in the pasted text?
        if (clipFormat != PgDataTranslator::kFmtNative)
            MakeAutoURLSpaghetti(insert_ref[0], insert_ref[0] + insert_ref[1]);

        // spell check the new stuff
	if ( !g_bInteractiveSpellCheck ) {
	    select_pair tr;
	    tr.begin = insert_ref[0];
	    tr.end = tr.begin + insert_ref[1];
	    adjust_text_range( m_paigeRef, &tr );    // make sure begin/end fall on word boundaries
	    QueueDirtyPair( tr.begin, tr.end  );     // spelling happens later -- now is bad time
	    m_checkdastuff = true;                   // don't know 'bout this crap
	}

        //Need to make sure the hyperlink callbacks is set to our default
        if (clipFormat == PgDataTranslator::kFmtNative)
        {
            pg_hyperlink hyperlink;
            //If there are any hyperlinks, then make sure the callback is set to our callback
            if (pgGetHyperlinkSourceInfo(m_paigeRef, insert_ref[0], 0, true, &hyperlink))
                pgSetHyperlinkCallback( m_paigeRef, hlCallback, NULL );
        }

        ScrollToCursor();                                         
    }
} 

void CPaigeEdtView::OnEditPasteAsQuotation()
{
    COleDataObject clipBoardData;
    VERIFY(clipBoardData.AttachClipboard());
    PgDataTranslator theTxr( m_paigeRef );

    PgDataTranslator::FormatType clipFormat = 0;

    bool bShift = (GetKeyState(VK_SHIFT) < 0);
    if ( bShift && clipBoardData.IsDataAvailable(CF_TEXT, NULL))
        clipFormat = CF_TEXT; 

    else if ((clipBoardData.IsDataAvailable((short)PgDataTranslator::kFmtNative, NULL)) && (pgTextSize(m_paigeRef)))
        clipFormat = PgDataTranslator::kFmtNative;

//      else if (clipBoardData.IsDataAvailable((short)PgDataTranslator::kFmtHtml, NULL))
//              clipFormat = PgDataTranslator::kFmtHtml;
                
    else if (clipBoardData.IsDataAvailable((short)PgDataTranslator::kFmtRtf, NULL))
        clipFormat = PgDataTranslator::kFmtRtf;
        
    else if (clipBoardData.IsDataAvailable(CF_TEXT, NULL))
        clipFormat = CF_TEXT; 

    //
    // Save the starting offset;
    //
    select_pair sel;
    pgGetSelection(m_paigeRef, &sel.begin, &sel.end);
    long savedFirst = sel.begin;
    //
    //  If the caret is in column 1, then there is no need to
    //  prepend a lf.
    //
    long lineStart, lineEnd;
    pgFindLine(m_paigeRef, sel.begin, &lineStart, &lineEnd);
    bool bPrependBreak = (sel.begin != lineStart);

    // if Native translater must check to see if it is plain/rich       
    if ((clipFormat == PgDataTranslator::kFmtNative) ||
        (clipFormat == PgDataTranslator::kFmtRtf) )
    {
        if (pg_ref pg = pgNewShell(PgGlobalsPtr()))
        {
            PgDataTranslator theTempTxr( pg );
            theTempTxr.ImportData( &clipBoardData, clipFormat );
            if ( !PgHasStyledText(pg, PgGlobalsPtr(), NULL) )
                clipFormat = CF_TEXT;
            pgDispose(pg);                                      
        }
    }
        
    if ( bShift || (clipFormat == CF_TEXT &&
					!GetIniShort(IDS_INI_ALWAYS_EXCERPT_RESPONSE) &&
					!GetIniShort(IDS_INI_EXCERPT_FIXED_TEXT) &&
					!PgHasStyledText(m_paigeRef, PgGlobalsPtr())) )
    {
        HANDLE hText;
        char* QuotedText = NULL;
        if (OpenClipboard())
        {
            hText = GetClipboardData(CF_TEXT);
            if (hText)
            {
                char* text = (char*)GlobalLock(hText);
                if (text)
                    QuotedText = QuoteText(text, FALSE);
                GlobalUnlock(hText);
                if (QuotedText)
                {
                    long length = strlen(QuotedText);
                                //      PrepareUndo(undo_insert,&length);
                    select_pair tempSel;
                    pgGetSelection(m_paigeRef, &tempSel.begin, &tempSel.end);
                    BOOL result = SetTextAs(QuotedText, &tempSel, PgDataTranslator::kFmtText, FALSE);
                    //pgInsert( m_paigeRef, (pg_byte_ptr)QuotedText, length, CURRENT_POSITION, 
                    //          data_insert_mode, 0, best_way );
                    delete [] QuotedText;
                }
            }
            CloseClipboard();
        }
    }
    else
    {
        if (clipFormat) {
            //
            // Existing excerpt level of insertion point
            //
            int level = 1;
            par_info    mask, info;
            pgInitParMask(&info,0);
            pgInitParMask(&mask,0);
            pgGetParInfo( m_paigeRef, NULL, false, &info, &mask );
            if ( m_styleEx->IsExcerpt(&info) )
                level += info.user_data;
            //
            //  Paige implementation doesn't seem to handle selected text correctly, 
            //  so let's perform the deletion ourselves.
            //
            short verb = undo_ole_paste;
            if (pgNumSelections(m_paigeRef))
            {
                PrepareUndo(undo_delete, MEM_NULL, TRUE);
                pgDelete(m_paigeRef, NULL, best_way);
                verb = undo_ole_paste_delete; 
            }
            //
            // Insert the text at the caret
            //
            long insert_ref[2];
            pgGetSelection(m_paigeRef, &sel.begin, &sel.end);
            insert_ref[0] = sel.begin;
            insert_ref[1] = 0;
            char br = '\r';
            if ( bPrependBreak )
            {
                pgInsert( m_paigeRef, (pg_byte_ptr)&br, 1, CURRENT_POSITION, key_insert_mode, 0, best_way );
                insert_ref[1] += 1;
                savedFirst++;
            }
            sel.begin = pgTextSize(m_paigeRef);
            theTxr.ImportData( &clipBoardData, clipFormat );
            sel.end = pgTextSize(m_paigeRef);
            insert_ref[1] += sel.end - sel.begin;
            //
            // Get the ending offset
            //
            pgGetSelection(m_paigeRef, &sel.begin, &sel.end);
            long savedLast = sel.end;
            //
            // Was a carriage return the last character pasted?
            //
            text_ref    ref_for_text;
            long                length=1;
            pg_byte_ptr pText;
            pText = pgExamineText(m_paigeRef, savedLast-1, &ref_for_text, &length);
            if (pText && *pText != PgGlobalsPtr()->line_wrap_char )                     
            {
                pgInsert( m_paigeRef, (pg_byte_ptr)&br, 1, CURRENT_POSITION, key_insert_mode, 0, best_way );
                insert_ref[1] +=1;
                savedLast++;
                sel.begin++;
                sel.end++;
            }
            PrepareUndo(verb, insert_ref);
                
            //
            // Apply Excerpt style to paste
            //
            pgSetSelection(m_paigeRef, savedFirst, savedLast, 0, FALSE);
            m_styleEx->SetExcerpt(level);
            pgSetSelection(m_paigeRef, sel.begin, sel.end, 0, TRUE);
        }
    }
    ScrollToCursor();                                     
}

BOOL CPaigeEdtView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
    dwStyle |= WS_VSCROLL;
    return CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}

void CPaigeEdtView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
    switch ( nSBCode ) {
    case SB_BOTTOM:
        pgScroll( m_paigeRef, scroll_none, scroll_end, best_way );
        break;
    case SB_LINEDOWN:
        pgScroll( m_paigeRef, scroll_none, -scroll_unit, best_way );
        break;
    case SB_LINEUP:
        pgScroll( m_paigeRef, scroll_none, scroll_unit, best_way );
        break;
    case SB_PAGEDOWN:
        pgScroll( m_paigeRef, scroll_none, -scroll_page, best_way );
        break;
    case SB_PAGEUP:
        pgScroll( m_paigeRef, scroll_none, scroll_page, best_way );
        break;
    case SB_TOP:
        pgScroll( m_paigeRef, scroll_none, scroll_home, best_way );
        break;
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK: {
        short curH, curV, maxH, maxV;
        pgGetScrollValues( m_paigeRef, &curH, &curV, &maxH, &maxV );
        SCROLLINFO si;
        GetScrollInfo(SB_VERT,&si);
        int nPos1 = MapScrollPos(nPos,si.nMax-si.nPage-1,maxV);
                        
        pgSetScrollValues( m_paigeRef, curH, (short)nPos1,
                           (pg_boolean)(nSBCode == SB_THUMBPOSITION), best_way );
        break;
    }
    }

    UpdateScrollBars( /* pScrollBar */ );
    CView::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CPaigeEdtView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
    switch ( nSBCode ) {
    case SB_PAGEDOWN:
        pgScroll( m_paigeRef, -scroll_page, scroll_none, best_way );
        break;
    case SB_LINEDOWN:
        pgScroll( m_paigeRef, -scroll_unit, scroll_none, best_way );
        break;
    case SB_PAGEUP:
        pgScroll( m_paigeRef, scroll_page, scroll_none, best_way );
        break;
    case SB_LINEUP:
        pgScroll( m_paigeRef, scroll_unit, scroll_none, best_way );
        break;
    case SB_THUMBPOSITION: //Doc doesn't include SB_THUMBTRACK??  Not sure why.
    {
        short curH, curV, maxH, maxV;
        pgGetScrollValues( m_paigeRef, &curH, &curV, &maxH, &maxV );
        pgSetScrollValues( m_paigeRef, curH, (short)nPos,
                           (pg_boolean)(nSBCode == SB_THUMBPOSITION), best_way );
        break;
    }
    }

    UpdateScrollBars( /* pScrollBar */ );
    CView::OnHScroll(nSBCode, nPos, pScrollBar);
}

UINT _AfxGetMouseScrollLines(BOOL bForceFresh = FALSE);

UINT _AfxGetMouseScrollLines(BOOL bForceFresh /* = FALSE */)
{
    // if we've already got it and we're not refreshing,
    // return what we've already got

    static BOOL bGotScrollLines = FALSE;
    static UINT uCachedScrollLines = 0;

    if (!bForceFresh && bGotScrollLines)
        return uCachedScrollLines;

        // see if we can find the mouse window

    bGotScrollLines = TRUE;

    static UINT msgGetScrollLines = 0;
    static WORD nRegisteredMessage = 0;

    if (nRegisteredMessage == 0)
    {
        msgGetScrollLines = ::RegisterWindowMessage(MSH_SCROLL_LINES);
        if (msgGetScrollLines == 0)
            nRegisteredMessage = 1;     // couldn't register!  never try again
        else
            nRegisteredMessage = 2;     // it worked: use it
    }

    if (nRegisteredMessage == 2)
    {
        HWND hwMouseWheel = NULL;
        hwMouseWheel = FindWindow(MSH_WHEELMODULE_CLASS, MSH_WHEELMODULE_TITLE);
        if (hwMouseWheel && msgGetScrollLines)
        {
            uCachedScrollLines = (UINT)
                ::SendMessage(hwMouseWheel, msgGetScrollLines, 0, 0);
            return uCachedScrollLines;
        }
    }

    // couldn't use the window -- try system settings

    OSVERSIONINFO ver;
    memset(&ver, 0, sizeof(ver));
    ver.dwOSVersionInfoSize = sizeof(ver);

    uCachedScrollLines = 3; // reasonable default
    if (!::GetVersionEx(&ver))
        return uCachedScrollLines;

    if ((ver.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ||
         ver.dwPlatformId == VER_PLATFORM_WIN32_NT)
        && ver.dwMajorVersion < 4)
    {
        HKEY hKey;
        if (RegOpenKeyEx(HKEY_CURRENT_USER,  _T("Control Panel\\Desktop"),
                         0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
        {
            TCHAR szData[128];
            DWORD dwKeyDataType;
            DWORD dwDataBufSize = sizeof(szData)/sizeof(szData[0]);

            if (RegQueryValueEx(hKey, _T("WheelScrollLines"), NULL, &dwKeyDataType,
                                (LPBYTE) &szData, &dwDataBufSize) == ERROR_SUCCESS)
            {
                uCachedScrollLines = _tcstoul(szData, NULL, 10);
            }
            RegCloseKey(hKey);
        }
    }
    else if (ver.dwPlatformId == VER_PLATFORM_WIN32_NT && ver.dwMajorVersion >= 4)
    {
        ::SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &uCachedScrollLines, 0);
    }

    return uCachedScrollLines;
}

BOOL CPaigeEdtView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    UINT uWheelScrollLines = _AfxGetMouseScrollLines(TRUE);
    int nToScroll;
//      int nDisplacement;

    nToScroll = ::MulDiv(-zDelta, uWheelScrollLines, WHEEL_DELTA);
    WORD nScrollCode;
    if (nToScroll == -1 || uWheelScrollLines == WHEEL_PAGESCROLL)
    {
        nScrollCode = (WORD)(zDelta > 0? SB_PAGEUP : SB_PAGEDOWN);
        SendMessage(WM_VSCROLL, MAKEWPARAM(nScrollCode, 0));
    }
    else
    {
//              nDisplacement = nToScroll * <line height>;
//              nDisplacement = min(nDisplacement, <page height>);
        nScrollCode = (WORD)(zDelta > 0? SB_LINEUP : SB_LINEDOWN);
        if (nToScroll < 0)
            nToScroll = -nToScroll;
        for (int i = 0; i < nToScroll; i++)
            SendMessage(WM_VSCROLL, MAKEWPARAM(nScrollCode, 0));
    }

    return (1);
}

void CPaigeEdtView::UpdateScrollBars( bool bSetPage /* = false */ )
{
    short maxH=0;
    short maxV=0;
    short hValue=0;
    short vValue=0;

    if (bSetPage)
        bHasBeenInitiallyUpdated=true;

    if ( !bHasBeenInitiallyUpdated )
        return;

    SCROLLINFO si;
    si.cbSize = sizeof(si);
    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    si.nMin = 0;
    //
    // Page size represents the number of data units that can
    // fit in the client area of the owner window given its current 
    // size.  Let's make a pixel THE data unit.
    //
    RECT rect;
    GetClientRect(&rect);
    si.nPage = rect.bottom - rect.top; 
    //
    // Maximum size of scroll bar will be the total height of
    // the document in pixels.
    //
    si.nMax = pgTotalTextHeight(m_paigeRef, true);
    //
    // Translate current scroll position from Paige To Windows
    //
    pgGetScrollValues(m_paigeRef, &hValue, &vValue, &maxH, &maxV );
    int yPos = MapScrollPos(vValue, maxV, si.nMax - si.nPage - 1);
    //
    // Position cannot exceed maximum scroll position
    //
    si.nPos = min(yPos,si.nMax);
        
    if (si.nPage < static_cast<UINT>(si.nMax))
        ShowScrollBar(SB_VERT,TRUE);

    if ( si.nMax > 10000 )
    {
        si.nMax = si.nMax /100;
        si.nPage = si.nPage /100;
        si.nPos = si.nPos /100;
    }
        
    SetScrollInfo(SB_VERT, &si, TRUE);

}


// OnFormatFont:
// Handles the "Font..." menu command

void CPaigeEdtView::OnFormatFont() 
{
    LOGFONT lf;
    memset( &lf, '\0', sizeof(LOGFONT) );
    pgGetFontByLog( m_paigeRef, &lf );

    /*CFontDialog*/CFaceNameDialog theFont( &lf, CF_INITTOLOGFONTSTRUCT | CF_EFFECTS | CF_SCREENFONTS );

    // get font attributes for current selection from user
    int iRet = theFont.DoModal();

    if ( iRet == IDOK ) {
        if ( pgNumSelections(m_paigeRef) )
            PrepareUndo(undo_format); 
        // set the face name
        pgSetFontByName( m_paigeRef,
                         (const pg_font_name_ptr)(LPCSTR)theFont.GetFaceName(), NULL, FALSE );

        // set the point size. note that windows gives this to us in tenths of a
        // point, while paige wants it in points.
        //pgSetPointSize( m_paigeRef, (short)pgAbsoluteValue(theFont.GetSize()/10),
        //              NULL, FALSE );

        // set the text color
        COLORREF theColor = theFont.GetColor();
        pgSetTextColor( m_paigeRef, &theColor, NULL, FALSE );

        // set the style attributes
        long styleBits, setBits;
        styleBits = setBits = 0;

        if ( theFont.IsBold() ) {
            styleBits |= X_BOLD_BIT;
        }
        if ( theFont.IsItalic() ) {
            styleBits |= X_ITALIC_BIT;
        }
        if ( theFont.IsUnderline() ) {
            styleBits |= X_UNDERLINE_BIT;
        }
        if ( theFont.IsStrikeOut() ) {
            styleBits |= X_STRIKEOUT_BIT;
        }

        // before we set styles, check for "plain text". not sure what this means
        // within the context of Paige, but as does the sample code, so do i.
        if ( styleBits == X_PLAIN_TEXT ) {
            setBits = X_ALL_STYLES;
        }
        else {
            long fartBits;
            pgGetStyleBits( m_paigeRef, &setBits, &fartBits );
            setBits ^= styleBits;
        }

        // set dem style bits!
        pgSetStyleBits( m_paigeRef, styleBits, setBits, NULL, TRUE );
    }
}

void CPaigeEdtView::OnUpdateFormatFont(CCmdUI* pCmdUI)
{
    if ((m_fRO) || (!m_bAllowStyled))
        pCmdUI->Enable( false );
    else
        pCmdUI->Enable( true );
}

// OnFormatStyles:
// Handles the "Format:Named Styles..." menu command
//
// This uses the "dialog from hell"; just give it a paige ref, and it'll do the
// rest. The dialog allows for creating a named style sheet and adding it to
// the document, as well as applying it to the current selection.

void CPaigeEdtView::OnFormatStyles() 
{
/*
  CNamedStyleDialog theNSDialog( m_paigeRef );
  theNSDialog.DoModal();
*/
}


void CPaigeEdtView::OnTextBold()
{
    if ( pgNumSelections(m_paigeRef) )
        PrepareUndo(undo_format);
    pgSetStyleBits( m_paigeRef, X_BOLD_BIT, X_BOLD_BIT, NULL, best_way );
}

void CPaigeEdtView::OnUpdateTextBold( CCmdUI* pCmdUI )
{
    if ((m_fRO) || (!m_bAllowStyled))
        pCmdUI->Enable( false );
    else
    {
        pCmdUI->Enable( true );

        pCmdUI->SetCheck(GetStyleState( bold_var ));
    }
}


void CPaigeEdtView::OnTextItalic()
{
    if ( pgNumSelections(m_paigeRef) )
        PrepareUndo(undo_format);
    pgSetStyleBits( m_paigeRef, X_ITALIC_BIT, X_ITALIC_BIT, NULL, best_way );
}

void CPaigeEdtView::OnUpdateTextItalic( CCmdUI* pCmdUI )
{
    if ((m_fRO) || (!m_bAllowStyled))
        pCmdUI->Enable( false );
    else
    {
        pCmdUI->Enable( true );
        pCmdUI->SetCheck(GetStyleState( italic_var ));
    }
}


void CPaigeEdtView::OnTextUnderline()
{
    if ( pgNumSelections(m_paigeRef) )
        PrepareUndo(undo_format);
    pgSetStyleBits( m_paigeRef, X_UNDERLINE_BIT, X_UNDERLINE_BIT, NULL, best_way );
}

void CPaigeEdtView::OnUpdateTextUnderline( CCmdUI* pCmdUI )
{
    if ((m_fRO) || (!m_bAllowStyled))
        pCmdUI->Enable( false );
    else
    {
        pCmdUI->Enable( true );
        pCmdUI->SetCheck(GetStyleState( underline_var ));
    }   
        
}

void CPaigeEdtView::OnTextPlain()
{
    if ( pgNumSelections(m_paigeRef) )
        PrepareUndo(undo_format);
    pgSetStyleBits( m_paigeRef, X_PLAIN_TEXT, X_ALL_STYLES, NULL, best_way );
}

void CPaigeEdtView::OnUpdateTextPlain( CCmdUI* pCmdUI )
{
    bool bStyled=FALSE;

    if ((m_fRO) || (!m_bAllowStyled))
        pCmdUI->Enable( false );
    else
    {
        pCmdUI->Enable( true );
        long style_bits, consist_bits;
        pgGetStyleBits( m_paigeRef, &style_bits, &consist_bits);
                
        bStyled = !(style_bits == X_PLAIN_TEXT );

        /*      style_info info, mask;
                pgInitStyleMask(&info,0);
                pgInitStyleMask(&mask,0);
                pgGetStyleInfo(m_paigeRef, NULL, FALSE, &info, &mask);
                pg_globals_ptr  globals=pgGetGlobals(m_paigeRef);
                //
                // Character Styles: bold, italic, underline
                // Text color, size     and embedded images
                //
                long style_bits = PaigeToQDStyle(&info);
                bStyled = ( ( style_bits & (X_BOLD_BIT | X_ITALIC_BIT & X_UNDERLINE_BIT)) || 
                ( info.fg_color.red       != globals->def_style.fg_color.red    ) ||
                ( info.fg_color.green != globals->def_style.fg_color.green      ) ||
                ( info.fg_color.blue  != globals->def_style.fg_color.blue       ) ||
                ( info.fg_color.alpha != globals->def_style.fg_color.alpha      ) ||
                ( info.embed_id           != globals->def_style.embed_id                ) ||
                ( info.point              != globals->def_style.point                   ) );
                //
                // Font format: Name and size
                //
                if (!bStyled)
                {
                char szFontName[256]={NULL};
                pgGetFontByName (m_paigeRef, (unsigned char*) &szFontName);
                bStyled= stricmp((const char*)&szFontName,(const char*)globals->def_font.name);
                }
        */
    }
    if ( bStyled )
        pCmdUI->SetCheck( false );
    else
        pCmdUI->SetCheck( true );
        
}


void CPaigeEdtView::OnParaCenter()
{
    par_info parInfo, parInfoMask;
    long styleID;
    if ( styleID = pgGetNamedStyleIndex (m_paigeRef, body_style) )
        pgGetNamedStyleInfo(m_paigeRef, styleID, NULL, NULL, &parInfo);
    else
        parInfo = PgGlobalsPtr()->def_par;
    pgFillBlock( &parInfoMask, sizeof(par_info), 0);//SET_MASK_BITS );

    parInfo.justification = justify_center;
    parInfoMask.justification = -1;
    PrepareUndo(undo_format);
    pgSetParInfo( m_paigeRef, NULL, &parInfo, &parInfoMask, best_way );
}

void CPaigeEdtView::OnUpdateParaCenter( CCmdUI* pCmdUI )
{
    if ((m_fRO) || (!m_bAllowStyled))
        pCmdUI->Enable(false);
    else
    {
        pCmdUI->Enable();
        pCmdUI->SetCheck(GetJustifyState(justify_center));
    }
}


void CPaigeEdtView::OnParaLeft()
{
    par_info parInfo, parInfoMask;
    long styleID;
    if ( styleID = pgGetNamedStyleIndex (m_paigeRef, body_style) )
        pgGetNamedStyleInfo(m_paigeRef, styleID, NULL, NULL, &parInfo);
    else
        parInfo = PgGlobalsPtr()->def_par;
    pgFillBlock( &parInfoMask, sizeof(par_info), SET_MASK_BITS );

    parInfo.justification = justify_left;
    parInfoMask.justification = -1;
    PrepareUndo(undo_format);
    pgSetParInfo( m_paigeRef, NULL, &parInfo, &parInfoMask, best_way );
}

void CPaigeEdtView::OnUpdateParaLeft( CCmdUI* pCmdUI )
{
    if ((m_fRO) || (!m_bAllowStyled))
        pCmdUI->Enable(false);
    else
    {
        pCmdUI->Enable();
        pCmdUI->SetCheck(GetJustifyState(justify_left));
    }
}

void CPaigeEdtView::OnParaRight()
{
    par_info parInfo, parInfoMask;
    long styleID;
    if ( styleID = pgGetNamedStyleIndex (m_paigeRef, body_style) )
        pgGetNamedStyleInfo(m_paigeRef, styleID, NULL, NULL, &parInfo);
    else
        parInfo = PgGlobalsPtr()->def_par;
    pgFillBlock( &parInfoMask, sizeof(par_info), SET_MASK_BITS );

    parInfo.justification = justify_right;
    parInfoMask.justification = -1;
    PrepareUndo(undo_format);
    pgSetParInfo( m_paigeRef, NULL, &parInfo, &parInfoMask, best_way );
}

void CPaigeEdtView::OnUpdateParaRight( CCmdUI* pCmdUI )
{
    if ((m_fRO) || (!m_bAllowStyled))
        pCmdUI->Enable(false);
    else
    {
        pCmdUI->Enable();
        pCmdUI->SetCheck(GetJustifyState(justify_right));
    }
}

void CPaigeEdtView::OnIndentIn()
{
	// Shareware: In reduced feature mode, you cannot change margins
	if (UsingFullFeatureSet())
	{
		// FULL FEATURE mode
		pg_indents ind, msk;
		pgGetIndents( m_paigeRef, NULL, &ind, &msk, NULL, NULL );
		ind.left_indent += DEFLIST_INDENT_VALUE;
		msk.left_indent = -1;
		msk.right_indent = 0;
		msk.first_indent = 0;
		PrepareUndo(undo_format);
		pgSetIndents( m_paigeRef, NULL, &ind, &msk, best_way );
	}
}

void CPaigeEdtView::OnUpdateIndentIn(CCmdUI* pCmdUI) 
{
 	// Shareware: In reduced feature mode, you cannot change margins
	pCmdUI->Enable(!m_fRO && m_bAllowStyled && UsingFullFeatureSet());
}

void CPaigeEdtView::OnIndentOut()
{
 	// Shareware: In reduced feature mode, you cannot change margins
	if (UsingFullFeatureSet())
	{
		// FULL FEATURE mode
		pg_indents ind, msk;
		pgGetIndents( m_paigeRef, NULL, &ind, &msk, NULL, NULL );
		ind.left_indent -= DEFLIST_INDENT_VALUE;
		msk.left_indent = -1;
		msk.right_indent = 0;
		msk.first_indent = 0;
		PrepareUndo(undo_format);
		pgSetIndents( m_paigeRef, NULL, &ind, &msk, best_way );
	}
}

void CPaigeEdtView::OnUpdateIndentOut( CCmdUI* pCmdUI )
{
 	// Shareware: In reduced feature mode, you cannot change margins
	if (UsingFullFeatureSet())
	{
		// FULL FEATURE mode
		if ((m_fRO) || (!m_bAllowStyled))
			pCmdUI->Enable( false );
		else
		{
			int baseIndent = 0;
			pg_indents ind, msk;
			pgGetIndents( m_paigeRef, NULL, &ind, &msk, NULL, NULL );
			if ( m_styleEx->IsBullet(NULL) )
				baseIndent = DEFLIST_INDENT_VALUE; 
			if ( ind.left_indent > baseIndent )
				pCmdUI->Enable( true );
			else
				pCmdUI->Enable(false);
		}
	}
	else
	{
		// REDUCED FEATURE mode
		pCmdUI->Enable(FALSE);
	}
}


// Serialize:
// Loads/saves Paige contents to/from HTML files
//
// Once this code is in Eudora, it will need to be rewritten to use streams
// instead of just passing the file name to the SGML parser.

void CPaigeEdtView::Serialize(CArchive& ar) 
{
    PgDataTranslator theTxr( m_paigeRef );
        
    CFile *theFile = ar.GetFile();
    CString fileName = theFile->GetFileName();

    //I guess we can just find ".ext" but this is being safe
    int dot = fileName.ReverseFind('.');
    CString fileExt = fileName.Right(fileName.GetLength() - dot - 1);

    PgDataTranslator::FormatType format;

    if ( ( fileExt.CompareNoCase(CRString(IDS_HTM_EXTENSION)) == 0 ) ||
         ( fileExt.CompareNoCase(CRString(IDS_HTML_EXTENSION)) == 0) )
        format = PgDataTranslator::kFmtHtml;
    else if ( fileExt.CompareNoCase(CRString(IDS_RTF_EXTENSION)) == 0)
        format = PgDataTranslator::kFmtRtf;
    else
        format = PgDataTranslator::kFmtText;

    if ( ar.IsStoring() ) {
        // storing code
        theTxr.ExportData( theFile, format);
        SaveChangeState();
    }
    else {
        // loading code

        pgApplyNamedStyle(m_paigeRef,NULL,body_style,best_way);

        theTxr.ImportData( theFile, format);
        pgSetSelection(m_paigeRef, 0, 0, 0, TRUE);
        UpdateScrollBars(true); 
    }
        
    //theFile->Close();
}


BOOL CPaigeEdtView::OnEraseBkgnd(CDC* pDC) 
{
    DWORD windowColor;
    pgColorToOS(&(PgGlobalsPtr()->def_bk_color), &windowColor);

    CBrush theBrush;
    theBrush.CreateSolidBrush(windowColor);
    CBrush* pOldBrush = (CBrush*) pDC->SelectObject( &theBrush );

    CRect rect;
    pDC->GetClipBox( &rect );
    pDC->PatBlt( rect.left, rect.top, rect.Width(), rect.Height(),PATCOPY );
    pDC->SelectObject( pOldBrush );
    return true;
}

void FindWord(pg_ref pgRef, long offset, long PG_FAR *wordLeft, long PG_FAR *wordRight, 
              pg_boolean left_side, pg_boolean smart_select) 
{
    long charInfo=0;
    long textSize = pgTextSize(pgRef);

    //Find Word to the Right, breaks at tabs, carriage returns, most punctuations and new-words
    if (!left_side) {
        *wordLeft = *wordRight = offset;
        charInfo = pgCharType(pgRef, *wordRight, TAB_BIT);
        while ((*wordRight != textSize) && (charInfo & TAB_BIT))                        
            charInfo = pgCharType(pgRef, ++*wordRight, TAB_BIT);
        charInfo = pgCharType(pgRef, *wordRight, BLANK_BIT | TAB_BIT | PAR_SEL_BIT);
        while ((*wordRight != textSize) && (charInfo & BLANK_BIT) && !(charInfo & (TAB_BIT | PAR_SEL_BIT)))
            charInfo = pgCharType(pgRef, ++*wordRight, BLANK_BIT | TAB_BIT | PAR_SEL_BIT);

        if ((*wordRight == offset) && (*wordRight != textSize)) {                       
            pgFindWord(pgRef, offset, wordLeft, wordRight, left_side, smart_select);
            if ((*wordRight == offset) && (*wordRight != textSize))
                ++*wordRight;
            charInfo = pgCharType(pgRef, *wordRight, BLANK_BIT | PAR_SEL_BIT | TAB_BIT);
            while ((*wordRight != textSize) && (charInfo & BLANK_BIT) && !(charInfo & (PAR_SEL_BIT | TAB_BIT)))
                charInfo = pgCharType(pgRef, ++*wordRight, BLANK_BIT | PAR_SEL_BIT | TAB_BIT);                  
        }
    }
    else {
        //Find word to the Left, breaks at beginning of tabs, carriage-returns, most punctuations and new-words
        //Traverse blanks on immediate left, if any, so we don;t get stuck at the same word
        *wordRight = *wordLeft = offset;
        //Find all blanks
        if (*wordLeft != 0) 
            charInfo = pgCharType(pgRef, *wordLeft-1, BLANK_BIT | TAB_BIT | PAR_SEL_BIT);
        while ((charInfo & BLANK_BIT) && !(charInfo & (TAB_BIT | PAR_SEL_BIT)) && (--*wordLeft != 0))
            charInfo = pgCharType(pgRef, *wordLeft-1, BLANK_BIT | TAB_BIT | PAR_SEL_BIT);

        offset = *wordLeft;
        pgFindWord(pgRef, offset, wordLeft, wordRight, left_side, smart_select);
        if ((*wordLeft == offset) && (*wordLeft != 0)) {
            //Find all tabs
            charInfo = pgCharType(pgRef, *wordLeft-1, TAB_BIT);
            while ((charInfo & TAB_BIT) && (--*wordLeft != 0))
                charInfo = pgCharType(pgRef, *wordLeft-1, TAB_BIT);
            if ((*wordLeft == offset) && (*wordLeft != 0)) --*wordLeft; 
        }
    }
}

void CPaigeEdtView::SetPgAreas( /*CPaigeEdtView *view*/ )
{       
    rectangle pgClientRect;
//      shape_ref visArea;

    CRect clientRect;
    GetClientRect(&clientRect);

    // don't want to fill the *entire* client area
    clientRect.DeflateRect( m_pgBorder );

    // Get the client area dimensions
    RectToRectangle(&clientRect,&pgClientRect);
//      visArea = pgRectToShape(PgMemGlobalsPtr(), &pgClientRect);
        
    // Reset the page area and the visible area to the current size
//      pgSetAreas(m_paigeRef, visArea, visArea, MEM_NULL);
    pgSetAreaBounds(m_paigeRef, &pgClientRect, &pgClientRect);

    // Dispose of the shape bcos Paige maintains a copy
//      pgDisposeShape(visArea);

    pgAdjustScrollMax(m_paigeRef, best_way);
}


void CPaigeEdtView::OnTimer(UINT nIDEvent)
{
    switch(nIDEvent) {
        // TRIPLE_CLICK .... Time Up and no third click
    case TRIPLE_CLICK_TIMER : 
        m_mouseState &= ~MOUSE_TRIPLE_CLICKED;
        KillTimer(nIDEvent); break;
    case DRAG_SELECT_TIMER : //Fall thru
        // DRAG_DROP scrolling
    case DRAG_DROP_SCROLL_TIMER : 
    {
        CRect clientRect;
        GetClientRect(&clientRect);
        if (clientRect.bottom - m_mousePt.y  <= 10)
        {
            int LinesToScroll = (m_mousePt.y - clientRect.bottom) / 16 + 1;
            for (; LinesToScroll > 0; LinesToScroll--)
            {
                OnVScroll(SB_LINEDOWN, 0, GetScrollBarCtrl(SB_VERT));
                if (nIDEvent == DRAG_SELECT_TIMER)
                {
                    co_ordinate pgMousePt;
                    CPointToPgPoint(&m_mousePt, &pgMousePt);
                    pgDragSelect(m_paigeRef, &pgMousePt, mouse_moved, m_pgModifiers, 0, FALSE);
                }
            }
        }
        else if (m_mousePt.y - clientRect.top <= 10)
        {
            int LinesToScroll = (clientRect.top - m_mousePt.y) / 16 + 1;
            for (; LinesToScroll > 0; LinesToScroll--)
            {
                OnVScroll(SB_LINEUP, 0, GetScrollBarCtrl(SB_VERT));
                if (nIDEvent == DRAG_SELECT_TIMER)
                {
                    co_ordinate pgMousePt;
                    CPointToPgPoint(&m_mousePt, &pgMousePt);
                    pgDragSelect(m_paigeRef, &pgMousePt, mouse_moved, m_pgModifiers, 0, FALSE);
                }
            }
        }
        break;
    }
    case AUTO_SPELL_TIMER:
    {
        if (IsReadOnly())
            KillTimer( AUTO_SPELL_TIMER );
        else if ( ((GetTickCount() - last_typed) > 250) && m_checkdastuff ) {
            KillTimer( AUTO_SPELL_TIMER );

			//SDSpellTRACE("Spell begin\n");

			if ((g_bInteractiveSpellCheck=GetIniShort( IDS_INI_INTERACTIVE_SPELL_CHECK )))
			{
				m_checkdastuff = false;
				//SDSpellTRACE("Spell aborted\n");
				break;
			}

			HideCaret();

			select_pair textRange;
            while ( DequeueDirtyPair( &textRange.begin, &textRange.end ) )
                m_spell.Check( &textRange );

            // now do the style runs -- see if any garbage is layin' 'round!

			// find the current word
			select_pair curWord;
			GetWordContainingSelection(m_paigeRef,&curWord);

			// prepare to walk styles
            style_walk sw;
            paige_rec_ptr prp = (paige_rec_ptr) UseMemory( m_paigeRef );
            pgPrepareStyleWalk( prp, 0, &sw, false );
            UnuseMemory( m_paigeRef );

            // walk my ass off!
			for (;;) {
                style_info_ptr psi = sw.cur_style;
				
				//SDSpellTRACE("Walking: %d.%d %d\n",sw.current_offset,sw.next_style_run->offset-sw.current_offset,CPaigeStyle::NeedsSpellCheck( psi ));

                if ( CPaigeStyle::NeedsSpellCheck( psi ) ) {
                    textRange.begin = textRange.end = sw.current_offset;
                    textRange.end += sw.next_style_run->offset - textRange.begin;
					if ( SelPairIntersects(&textRange,&curWord) )
					{
						if ( textRange.begin<curWord.begin )
						{
							QueueDirtyPair( textRange.begin, curWord.begin );
						}
						if ( curWord.end < textRange.end )
						{
							QueueDirtyPair( curWord.end, textRange.end );
						}
					}
					else
					{
						QueueDirtyPair( textRange.begin, textRange.end );
					}
                }
				if (!pgWalkNextStyle(&sw)) break;
            }

            pgPrepareStyleWalk( prp, 0, NULL, false );    // tells paige we're done
            CPaigeStyle boogieMama( m_paigeRef );

            while ( DequeueDirtyPair( &textRange.begin, &textRange.end ) ) {
                boogieMama.SetNeedsSpellCheck( FALSE, &textRange );
                m_spell.Check( &textRange );
            }

            m_checkdastuff = false;
 			//SDSpellTRACE("Spell end\n");
           SetTimer( AUTO_SPELL_TIMER, AST_INTERVAL, NULL );
           ShowCaret();
        }
        break;
    }
    }
}


void CPaigeEdtView::OnInitialUpdate() 
{
    CView::OnInitialUpdate();
        
    pgSetSelection( m_paigeRef, 0, 0, 0, false );
}

short CPaigeEdtView::GetPaigeUndoVerb(short EudoraUndoVerb)
{
    //In case we happen to call with a valid Paige Undo verb
    short PaigeUndoVerb = EudoraUndoVerb;

    switch(EudoraUndoVerb)
    {
    case undo_drag : 
        PaigeUndoVerb = undo_delete; break;
    case -undo_drag :
        PaigeUndoVerb = -undo_delete; break;
                
    case undo_insert_hr : 
    case undo_insert_hr_withCR : 
        PaigeUndoVerb = undo_format; break;
    case -undo_insert_hr :
    case -undo_insert_hr_withCR : 
        PaigeUndoVerb = -undo_delete; break;

    case undo_drop :
    case undo_ole_paste :
    case undo_translation :
    case undo_ole_paste_delete :
    case undo_drag_drop_move :
    case undo_translation_replace :
        PaigeUndoVerb = undo_app_insert; break;
    case -undo_drop :
    case -undo_ole_paste :
    case -undo_translation :
    case -undo_ole_paste_delete :
    case -undo_drag_drop_move :
    case -undo_translation_replace :
        PaigeUndoVerb = -undo_app_insert; break;

    case undo_overwrite :
        PaigeUndoVerb = undo_typing; break;
    case -undo_overwrite :
        PaigeUndoVerb = -undo_typing; break;
    }

    return PaigeUndoVerb;
}


void CPaigeEdtView::PrepareUndo(short verb, long *insert_ref, BOOL useDeleteStack)
{
    undo_ref prevUndo, *newUndo = new undo_ref ;
    short originalVerb = verb;

    CUndoStack &theStack = useDeleteStack ? m_deleteUndoStack : m_undoStack;
        
    //For all verbs except the following three
    prevUndo = MEM_NULL; 

    //Need to force new undo refs ourselves?
    if ((verb == undo_typing) || /*(verb = undo_overwrite) ||*/
        (verb == undo_fwd_delete) || (verb == undo_backspace))
    {
        if (theStack.GetCount() > 0)
            if (pgUndoType(prevUndo = *theStack.GetTail()) == verb)
            {
                long selectLeft;
                pgGetSelection(m_paigeRef, &selectLeft, MEM_NULL);

                pg_undo_ptr undoPtr = (pg_undo_ptr)UseMemory(prevUndo);

                if ( ( (verb == undo_typing && undoPtr->verb == undo_typing) /*|| 
                                                                               (verb == undo_overwrite && undoPtr->verb == undo_overwrite)*/ ) 
                     &&
                     (!HasChanged() || undoPtr->verb != undoPtr->real_verb) )
                {
                    //If the last saved undo ref was for typing and the doc is currently
                    //in a saved state, then typing some more will add it to the same 
                    //undo ref, but we need to force a new one to be created

                    //Or if we performed an undo/redo, the real verb gets changed to paste
                    //and they Paige clears out the old undo info
                    UnuseMemory(prevUndo);
                    prevUndo = MEM_NULL; 
                }
                else if ((undoPtr->alt_range.end != selectLeft) || (pgNumSelections(m_paigeRef)))
                {
                    UnuseMemory(prevUndo);
                    prevUndo = MEM_NULL; 
                }
                else UnuseMemory(prevUndo);
            }
            else prevUndo = MEM_NULL;
    }

    //Now change all our self-defined verbs to what Paige can understand b'4 calling pgPrepareUndo
    if (abs(originalVerb) >= undo_drag)
        verb = CPaigeEdtView::GetPaigeUndoVerb(originalVerb);

    if (insert_ref)
        *newUndo = pgPrepareUndo(m_paigeRef, verb, (void PG_FAR *)insert_ref);
    else
        *newUndo = pgPrepareUndo(m_paigeRef, verb, (void PG_FAR *)prevUndo);

    //Now set the undo verbs back to what we understand
    if (abs(originalVerb) >= undo_drag)
    {
        pg_undo_ptr undoPtr = (pg_undo_ptr)UseMemory(*newUndo);
        undoPtr->verb = originalVerb;
        UnuseMemory(*newUndo);
    }

    if (!prevUndo || (*newUndo != prevUndo)) 
    {
        AddRefToTail(theStack, newUndo);
        //Enough to keep track of the Main undo stack
        if ( ! useDeleteStack )
            UpdateChangeState( kAddChange );
    }
    else 
        delete newUndo;

}

void CPaigeEdtView::OnEditUndo() 
{
    if (m_undoStack.GetCount() == 0)
    {
        // Why did this get enabled when there's nothing to undo?
        ASSERT(0);
        return;
    }

    undo_ref *undoRef = m_undoStack.GetTail();

    if (!undoRef || !*undoRef)
    {
        // Uh oh, something weird happened!
        ASSERT(0);
        return;
    }

    //Change the verbs to what Paige can understand b'4 calling pgUndo
    pg_undo_ptr refPtr = (pg_undo_ptr)UseMemory(*undoRef);

    undo_ref *redoRef = new undo_ref;
    undo_ref *undoDeleteRef, *redoDeleteRef;

    short originalVerb = refPtr->verb;

    if (abs(originalVerb) >= undo_drag)
        refPtr->verb = CPaigeEdtView::GetPaigeUndoVerb(originalVerb);

    UnuseMemory(*undoRef);

    //pgUndo returns a ref to be used for Redo-ing the operation
    if (abs(originalVerb) >= undo_ole_paste_delete)
    {
        *redoRef = pgUndo(m_paigeRef, *undoRef, TRUE, best_way);
        undoDeleteRef = m_deleteUndoStack.GetTail();
        redoDeleteRef = new undo_ref;
        *redoDeleteRef = pgUndo(m_paigeRef, *undoDeleteRef, TRUE, best_way);
    }
    else 
        *redoRef = pgUndo(m_paigeRef, *undoRef, TRUE, best_way);
        
    //Change the verb back to what we understand
    if (abs(originalVerb) >= undo_drag)
    {
        refPtr = (pg_undo_ptr)UseMemory(*redoRef);
        refPtr->verb = (short)-(originalVerb);
        UnuseMemory(*redoRef);
    }

    //TODO: Update Current Style Run somehow here
    if ((abs(originalVerb) == undo_format) || (abs(originalVerb) >= undo_ole_paste))
    {
        if (!pgNumSelections(m_paigeRef))
        {
            long selectLeft;
            pgGetSelection(m_paigeRef, &selectLeft, MEM_NULL);
            pgSetSelection(m_paigeRef, selectLeft, selectLeft, 0, TRUE);
        }
        //TODO: Update the toolbar buttons here for format changes
    }

    RemoveRef(m_undoStack);
    AddRefToTail(m_redoStack, redoRef);
    UpdateChangeState( kUndoChange );

    if (abs(originalVerb) >= undo_ole_paste_delete)
    {
        RemoveRef(m_deleteUndoStack);
        AddRefToTail(m_deleteRedoStack, redoDeleteRef);
        //UpdateChangeState( kUndoChange );
    }
    ScrollToCursor();
}

void CPaigeEdtView::OnUpdateEditUndo(CCmdUI* pCmdUI) 
{
    if (!m_fRO)
    {
        int undoCount = m_undoStack.GetCount();
        pCmdUI->Enable(undoCount);

        if (undoCount) 
        {
            switch(pgUndoType(*m_undoStack.GetTail())) 
            {
                //case undo_overwrite :
            case undo_typing : 
                pCmdUI->SetText(CRString(IDS_EDIT_UNDO_TYPING)); break;
            case undo_fwd_delete :
            case undo_backspace :
            case undo_delete : 
                pCmdUI->SetText(CRString(IDS_EDIT_UNDO_DELETE)); break;
            case undo_format :
                pCmdUI->SetText(CRString(IDS_EDIT_UNDO_FORMATTING)); break;
            case undo_ole_paste :
            case undo_ole_paste_delete :
                pCmdUI->SetText(CRString(IDS_EDIT_UNDO_PASTE)); break;
            case undo_drag :
            case undo_drop :
            case undo_drag_drop_move :
                pCmdUI->SetText(CRString(IDS_EDIT_UNDO_DRAG_DROP)); break;
            case undo_insert_hr :
            case undo_insert_hr_withCR :
                pCmdUI->SetText(CRString(IDS_EDIT_UNDO_HR)); break;
            case undo_none :
            default : 
                pCmdUI->SetText(CRString(IDS_EDIT_UNDO)); break;
            }
        }
        else pCmdUI->SetText(CRString(IDS_EDIT_UNDO));
    }
    else
    {
        pCmdUI->SetText(CRString(IDS_EDIT_UNDO));
        pCmdUI->Enable(FALSE);
    }
}

void CPaigeEdtView::OnEditRedo() 
{
    ASSERT(m_redoStack.GetCount());

    undo_ref *redoRef = m_redoStack.GetTail();
    undo_ref *undoRef = new undo_ref;

    undo_ref *redoDeleteRef, *undoDeleteRef;

    //Change the verb to what Paige can understand
    pg_undo_ptr refPtr = (pg_undo_ptr)UseMemory(*redoRef);

    short originalVerb = refPtr->verb;

    if (abs(originalVerb) >= undo_drag)
        refPtr->verb = CPaigeEdtView::GetPaigeUndoVerb(originalVerb);

    UnuseMemory(*redoRef);

    //Redo the deletes, if any, first
    if (abs(originalVerb) >= undo_ole_paste_delete)
    {
        redoDeleteRef = m_deleteRedoStack.GetTail();
        undoDeleteRef = new undo_ref;
        *undoDeleteRef = pgUndo(m_paigeRef, *redoDeleteRef, TRUE, best_way);
        //Pop the delete ref from the secondary redo stack and 
        //add the new one to the secondary undo stack
        RemoveRef(m_deleteRedoStack);
        AddRefToTail(m_deleteUndoStack, undoDeleteRef);
        //UpdateChangeState( kRedoChange );
    }

    //Redo the main action
    *undoRef = pgUndo(m_paigeRef, *redoRef, TRUE, best_way);

    //Change the verb from [-]undo_app_insert back to [-]paste/drag_drop
    if (abs(originalVerb) >= undo_drag)
    {
        refPtr = (pg_undo_ptr)UseMemory(*undoRef);
        refPtr->verb = (short)-(originalVerb);
        UnuseMemory(*undoRef);
    } 

    //TODO: Update Current Style Run somehow here
    if ((abs(originalVerb) == undo_format) || (abs(originalVerb) >= undo_ole_paste))
    {
        if (!pgNumSelections(m_paigeRef))
        {
            long selectLeft;
            pgGetSelection(m_paigeRef, &selectLeft, MEM_NULL);
            pgSetSelection(m_paigeRef, selectLeft, selectLeft, 0, TRUE);
        }
    }

    //Pop the redo ref from the Tail
    RemoveRef(m_redoStack);
    AddRefToTail(m_undoStack, undoRef);
    UpdateChangeState( kRedoChange );
}

void CPaigeEdtView::OnUpdateEditRedo(CCmdUI* pCmdUI) 
{
    if (!m_fRO)
    {
        int redoCount = m_redoStack.GetCount();
        pCmdUI->Enable(redoCount);
        if (redoCount) {
            switch(pgUndoType(*m_redoStack.GetTail())) 
            {
            case -undo_typing : 
                pCmdUI->SetText(CRString(IDS_EDIT_REDO_TYPING)); break;
            case -undo_fwd_delete :
            case -undo_backspace :
            case -undo_delete: 
                pCmdUI->SetText(CRString(IDS_EDIT_REDO_DELETE)); break;
            case -undo_format : 
                pCmdUI->SetText(CRString(IDS_EDIT_REDO_FORMATTING)); break;
            case -undo_ole_paste :
            case -undo_ole_paste_delete :
                pCmdUI->SetText(CRString(IDS_EDIT_REDO_PASTE)); break;
            case -undo_drag :
            case -undo_drop :
            case -undo_drag_drop_move :
                pCmdUI->SetText(CRString(IDS_EDIT_REDO_DRAG_DROP)); break;
            case -undo_insert_hr :
            case -undo_insert_hr_withCR :
                pCmdUI->SetText(CRString(IDS_EDIT_REDO_HR)); break;
            case undo_none :
            default : 
                pCmdUI->SetText(CRString(IDS_EDIT_REDO)); break;
            }
        }
        else pCmdUI->SetText(CRString(IDS_EDIT_REDO));
    }
    else
    {
        pCmdUI->SetText(CRString(IDS_EDIT_REDO));
        pCmdUI->Enable(FALSE);
    }
}



void CPaigeEdtView::AddRefToTail(CUndoStack &theStack, undo_ref *theRef) 
{
    if (theStack.GetCount() == UNDO_STACK_MAX)
    {
        undo_ref *staleRef = theStack.GetHead();
        RemoveRef(theStack, /*fromTail=*/FALSE);
    }
    theStack.AddTail(theRef);
}

void CPaigeEdtView::RemoveRef(CUndoStack &theRefStack, BOOL fromTail)
{
    undo_ref *theRef;
    if (fromTail) {
        theRef = theRefStack.GetTail();
        theRefStack.RemoveTail();       
    }
    else {
        theRef = theRefStack.GetHead();
        theRefStack.RemoveHead();
    }
    pgDisposeUndo(*theRef);
    delete theRef;
}

void CPaigeEdtView::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(pgNumSelections(m_paigeRef));        
}

void CPaigeEdtView::OnUpdateEditCut(CCmdUI* pCmdUI) 
{
    if (m_fRO)
        pCmdUI->Enable(FALSE);
    else
        pCmdUI->Enable(pgNumSelections(m_paigeRef));            
}

void CPaigeEdtView::OnUpdateEditPaste(CCmdUI* pCmdUI) 
{
    BOOL dataAvailable = FALSE;

    if (!m_fRO)
    {
        COleDataObject clipBoardData;
        VERIFY(clipBoardData.AttachClipboard());

        if ((clipBoardData.IsDataAvailable((short)PgDataTranslator::kFmtNative, NULL)) ||
            (clipBoardData.IsDataAvailable((short)PgDataTranslator::kFmtRtf, NULL)) ||
            (clipBoardData.IsDataAvailable(CF_TEXT, NULL)) ||
            (clipBoardData.IsDataAvailable(CF_BITMAP, NULL)) ||
            (clipBoardData.IsDataAvailable((short)PgDataTranslator::kFmtHtml, NULL)))
            dataAvailable = TRUE;
    }

    pCmdUI->Enable(dataAvailable);
}

DROPEFFECT CPaigeEdtView::OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) 
{
    //m_mouseState |= MOUSE_DRAG_DROP;

    DROPEFFECT dropEffect;
    dropEffect = OnDragOver(pDataObject, dwKeyState, point);
    if ((dropEffect == DROPEFFECT_MOVE) || (dropEffect == DROPEFFECT_COPY)) {
        //m_mouseState |= MOUSE_DRAG_DROP;
        SetTimer(DRAG_DROP_SCROLL_TIMER, 200, NULL);
    }
    return dropEffect;
    //return CView::OnDragEnter(pDataObject, dwKeyState, point);
}

void CPaigeEdtView::OnDragLeave() 
{
    // TODO: Add your specialized code here and/or call the base class
    KillTimer(DRAG_DROP_SCROLL_TIMER);
    //m_mouseState &= ~MOUSE_DRAG_DROP;
    m_mousePt = 0;
    CView::OnDragLeave();
}

DROPEFFECT CPaigeEdtView::OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) 
{
    if (((pDataObject->IsDataAvailable((short)PgDataTranslator::kFmtNative, NULL)) ||
         (pDataObject->IsDataAvailable((short)PgDataTranslator::kFmtRtf, NULL)) ||
         (pDataObject->IsDataAvailable(CF_TEXT, NULL)) ||
         (pDataObject->IsDataAvailable((short)PgDataTranslator::kFmtHtml, NULL)))
        &&
        (!m_fRO))
    {
        m_mousePt = point;
        if (dwKeyState & MK_CONTROL)
            return DROPEFFECT_COPY;
        else //if ((dwKeyState & MK_SHIFT) || (dwKeyState & MK_ALT)) 
            return DROPEFFECT_MOVE;
    }
    else return DROPEFFECT_NONE;
    //return CView::OnDragOver(pDataObject, dwKeyState, point);
}

BOOL CPaigeEdtView::OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point) 
{
    if (m_fRO) 
    { 
        OnDragLeave();
        return false;
    }

    //Kill the *AutoScroll* timer for drag and drop
    KillTimer(DRAG_DROP_SCROLL_TIMER); 

    long selectRight, selectLeft;
    pgGetSelection(m_paigeRef, &selectLeft, &selectRight);

    //Get current *CURSOR* position
    CPoint curPoint;
    ::GetCursorPos(&curPoint);
    ScreenToClient(&curPoint);
    //Fill Paige coord struct
    co_ordinate curCoOrd;
    CPointToPgPoint(&curPoint, &curCoOrd);
    long offset = pgPtToChar(m_paigeRef, &curCoOrd, NULL);
    short verb = undo_drop;

    //No op if drag and drop is within the same selection
    if ((m_mouseState & MOUSE_DRAG_DROP) && (offset >= selectLeft) && (offset <= selectRight))
        return FALSE;

    // source and target are the same, hence handle the deletion of text is previous location
    if ((dropEffect == DROPEFFECT_MOVE) && (!m_fRO)){
        if ( (m_mouseState & MOUSE_DRAG_DROP) && (pgNumSelections(m_paigeRef)) ) {
            if (offset > selectRight)
                offset -= (selectRight - selectLeft);
            PrepareUndo(undo_delete, MEM_NULL, TRUE);
            // Delete the current selection since it's a MOVE operation
            pgDelete(m_paigeRef, NULL, best_way);
            verb = undo_drag_drop_move;
        }
    }

    pgSetSelection(m_paigeRef, offset, offset, 0, TRUE);        
    OnPaste(pDataObject, verb);

    return TRUE;
    //return CView::OnDrop(pDataObject, dropEffect, point);
}

void CPaigeEdtView::UpdateChangeState( ucs_verb ucsVerb /* = kDocOnly */ )
{
    switch ( ucsVerb ) {

    case kAddChange:
    case kRedoChange:
        m_chngCount++;
        break;
    case kUndoChange:
        ASSERT( m_chngCount > 0 );
        m_chngCount--;
        break;
    case kDocOnly:
        break;

    default:
        // we have been passed a bad 'verb'
        ASSERT(0);
    }

    // Note that we could also do some type of check for the 4th "gigachange",
    // but the likelyhood of reaching it at all is only less remote than
    // the chance that it would happen in a development build.

    // this may look expensive, but it's all in-line
    CDocument *pDoc = GetDocument();
    pDoc->SetModifiedFlag( HasChanged() || pDoc->IsModified());
}


void CPaigeEdtView::OnEditSelectAll() 
{
    long textSize = pgTextSize(m_paigeRef);
    if (textSize)
        pgSetSelection(m_paigeRef, 0, textSize, 0, TRUE);
}

void CPaigeEdtView::OnUpdateEditSelectAll(CCmdUI* pCmdUI) 
{       
    long textSize = pgTextSize(m_paigeRef);

    if (textSize)
    {
        long selectLeft, selectRight;
        pgGetSelection(m_paigeRef, &selectLeft, &selectRight);
        pCmdUI->Enable(!((selectLeft == 0) && (selectRight == textSize)));
    }
    else pCmdUI->Enable(FALSE);
}

/*void CPaigeEdtView::OnEditInsertTable() 
{
        CNewTableDlg newTable;
        if (IDOK == newTable.DoModal())
        {
        pg_table pgTable;
        pgTable.table_columns = newTable.m_columns;
        long pixelWidth = InchesToPixels(newTable.m_columnWidth);
        pgTable.table_column_width = pixelWidth;
        pgTable.border_info = 0x0b0b0b0b;
        pgTable.cell_h_extra = 2;

        pgTable.table_cell_height = 0;
        pgTable.border_shading = 0;
        pgTable.border_spacing = 0;
        pgTable.cell_borders = 0; 
        pgTable.grid_borders = 0;
        pgTable.left_border_color = 0;
        pgTable.top_border_color = 0;
        pgTable.right_border_color = 0;
        pgTable.bottom_border_color = 0;
        
        //TEST CODE ONLY
        // TODO: Add your command handler code here
        //pg_table pgTable;
        //long someValue = 3;

    //Testing, use a dialog later!
/*      pgTable.table_columns = 3;
        pgTable.table_column_width = 100;
        pgTable.table_cell_height = 0;
        pgTable.border_info = 0; //To be [set] or not to be [set] :-???
        //pgTable.border_info = PG_BORDER_ALL3D; 
        pgTable.border_info = 0x0b0b0b0b;
        pgTable.border_shading = 0;
        pgTable.border_spacing = 0;
        pgTable.cell_borders = 0; //To set here or in tab_info :-????
        //pgTable.cell_borders = someValue | (someValue << 8) | (someValue << 16) | (someValue << 24);
        //pgTable.cell_borders |= 0x0b0b0b0b;//PG_BORDER_ALL3D;
        pgTable.grid_borders = 0;
        pgTable.grid_borders = PG_BORDER_ALLDOTTED; //This sucks!! doesn't work
        pgTable.cell_h_extra = 2;
        pgTable.left_border_color = 0;
        pgTable.top_border_color = 0;
        pgTable.right_border_color = 0;
        pgTable.bottom_border_color = 0;*\/

        pgInsertTable(m_paigeRef, CURRENT_POSITION, &pgTable, newTable.m_rows, best_way);
        //Invalidate();

        //If the user doesn't do this they are screwed!!
        TCHAR uChar = (TCHAR)PgGlobalsPtr()->line_wrap_char;
        pgInsert( m_paigeRef, (pg_byte_ptr)&uChar, 1, CURRENT_POSITION, 
                                                key_insert_mode, 0, best_way );
        }
        /*select_pair selection;
        pgGetSelection(m_paigeRef, &selection.begin, &selection.end);

        par_info info, mask;
        pgGetParInfoEx(m_paigeRef, MEM_NULL, false, &info, &mask, true);

        info.num_tabs = 3;
        for (int i=0; i<3; ++i) {
                info.tabs[i].tab_type = 1;
                info.tabs[i].position = 100;
                //info.tabs[i].leader = PG_BORDER_ALL3D;
                info.tabs[i].leader = 0x0b0b0b0b;
        }
        info.column_var = 300;
        info.table.table_columns = 3;
        info.table.border_info = 0x0b0b0b0b;
        //info.table.border_info = PG_BORDER_ALL3D;
        info.table.border_spacing = 0;
        info.table.cell_h_extra = 2;

        pgSetParInfoEx(m_paigeRef, &selection, &info, &mask, true, true);

        pgInsert( m_paigeRef, (pg_byte_ptr)&uChar, 1, CURRENT_POSITION, 
                                key_insert_mode, 0, best_way );*\/
}


void CPaigeEdtView::OnUpdateEditInsertTable(CCmdUI* pCmdUI) 
{
/*      if (m_fRO)
                pCmdUI->Enable(false);
        else
                pCmdUI->Enable(!pgIsTable(m_paigeRef, CURRENT_POSITION));       
*\/
        pCmdUI->Enable(false);
}

void CPaigeEdtView::OnEditDeleteTable() 
{
        // TODO: Add your command handler code here

        //Testing
        par_info info, mask;
        pgGetParInfoEx(m_paigeRef, MEM_NULL, false, &info, &mask, true);
}

void CPaigeEdtView::OnUpdateEditDeleteTable(CCmdUI* pCmdUI) 
{
/*      if (m_fRO)
                pCmdUI->Enable(false);
        else
                pCmdUI->Enable(pgIsTable(m_paigeRef, CURRENT_POSITION));
*\/
        pCmdUI->Enable(false);
}

long CPaigeEdtView::InchesToPixels(float inches)
{
        CDC *pDC = GetDC();
        long pixels = (long)(pDC->GetDeviceCaps(LOGPIXELSX) * inches);
        ReleaseDC(pDC);
        return pixels;
}
        
float CPaigeEdtView::PixelsToInches(long pixels)
{
        CDC *pDC = GetDC();
        float inches = ((float)pixels) / ((float)pDC->GetDeviceCaps(LOGPIXELSX));
        ReleaseDC(pDC);
        return inches;
}
*/
int CPaigeEdtView::CheckSpelling(BOOL autoCheck /*=FALSE*/)
{
    int ret = 0;

	// Shareware: In reduced feature mode, you cannot right-click
	if (UsingFullFeatureSet())
	{
		// FULL FEATURE mode

		MakeAutoURLSpaghetti(0, pgTextSize(m_paigeRef));
		ret = m_spell.Check(this,0,autoCheck);

	/*      int stCh, endCh;
			GetEditCtrl().GetSel(stCh, endCh);
			if (GetFocus() == &GetEditCtrl() && stCh != endCh)
			ret = Spell.Check(&GetEditCtrl());
			else
			{
			GetEditCtrl().SetSel(0, 0);
			CEdit *sub = (CEdit *)m_pHeader->GetDlgItem(IDC_HDRFLD_EDIT3);
			if (sub) 
			ret = Spell.Check(sub, 0, TRUE); 
			if (!ret)
			ret = Spell.Check(&GetEditCtrl(), 0, autoCheck);
			}
	*/
	}
    
	return ret;
}

BOOL CPaigeEdtView::DoFindFirst( 
    const CString&      szSearch, 
    BOOL                        bMatchCase, 
    BOOL                        bWholeWord, 
    BOOL                        bSelect )
{
    return Find(0,szSearch,bMatchCase,bWholeWord,bSelect);
}

BOOL CPaigeEdtView::DoFindNext( 
    const CString&      szSearch, 
    BOOL                        bMatchCase, 
    BOOL                        bWholeWord, 
    BOOL                        bSelect )
{

    long offset=0;
    select_pair sel;

    //
    // Begin search after the current selection
    //
    pgGetSelection(m_paigeRef, &sel.begin, &sel.end);
    offset = sel.end +1;
        
    return Find(offset,szSearch,bMatchCase,bWholeWord,bSelect);
}


BOOL CPaigeEdtView::Find( 
    long offset,
    const CString&      szSearch, 
    BOOL                        bMatchCase, 
    BOOL                        bWholeWord, 
    BOOL                        bSelect )
{
    select_pair sel;
    long lMax = pgTextSize(m_paigeRef);
    bool bFound=FALSE;
    int ret;

    //
    // Make a local copy of the search string and the target string,
    // so that we can call MakeLower() on them, if necessary.
    //
    CString tmpSearch = szSearch;
    if (!bMatchCase )
        tmpSearch.MakeLower();

    text_ref    ref_for_text;
    long                length;
    pg_byte_ptr pText;

    while ( offset <= lMax )
    {
        //
        // Get a block of text
        //
        pText = pgExamineText(m_paigeRef, offset, &ref_for_text, &length);
        if (NULL == pText)
        {
            //
            // Fell off the end of the message, so break out of this loop.
            //
            break;
        }
        ASSERT(length > 0);
        CString         tmpText( (char*) pText,length );
        if (!bMatchCase )
            tmpText.MakeLower();
        //
        // Search for string
        //
        if ( (ret = tmpText.Find(tmpSearch)) >= 0 )
        {
            //
            // We've found the string but it could be a Red Herring
            // if Whole Word is checked.   Verify this.
            //
            if ( !bWholeWord )
                bFound = TRUE;
            else
            {
                pgFindWord(m_paigeRef, offset+ret, &sel.begin,&sel.end,TRUE,FALSE);
                if ( StrPgCmp( tmpSearch,sel ) == 0 )
                    bFound =TRUE;
                else
                    offset = sel.end+1;
            }

            if (bFound)
            {
                        
                sel.begin = offset+ret;
                sel.end = sel.begin + tmpSearch.GetLength();
                                //
                                // Could be hidden text in Taboo headers
                                //
                style_info                      info, mask;
                pgInitStyleMask(&info,0);
                pgInitStyleMask(&mask,0);
                pgGetStyleInfo(m_paigeRef,&sel,FALSE,&info,&mask);
                if (info.styles[hidden_text_var] == 1)
                {
                    bFound = FALSE;
                    offset = sel.end+1;
                }
                else
                {
                    UnuseMemory(ref_for_text);
                    break;
                }
            }
        }
        else
        {
            // No match in this block
            offset += length+1;
        }
        UnuseMemory(ref_for_text);
    }

    if ( bFound && bSelect )
    {
// BOG: don't want to grab the focus, just turn on highlighting and make sure it
// stays on until the (l)user clicks in the client area or types something.
//        SetFocus();
        m_bDontDeactivateHighlight = TRUE;
        pgSetHiliteStates(m_paigeRef, activate_verb, no_change_verb, TRUE); 
        pgSetSelection(m_paigeRef, sel.begin, sel.end, 0, TRUE);
        pgScrollToView(m_paigeRef, CURRENT_POSITION, 32, 32, TRUE, best_way);
        Invalidate();
        UpdateScrollBars();
    }

    return bFound;      

}

char* CPaigeEdtView::GetPgText(char* pDstText, int nMaxLen, select_pair& sel, bool bSelection)
{
    if ( bSelection )
        pgSetSelection(m_paigeRef, sel.begin, sel.end, 0, TRUE);
    memory_ref  refText = pgCopyText(m_paigeRef, &sel, all_data);
    char* pText=(char*) UseMemory(refText);
    //
    // Append a terminating NULL
    //
    AppendMemory(refText,sizeof(pg_byte),TRUE);
    if ( strlen(pText) <= (size_t)nMaxLen )
        strcpy(pDstText,pText);
    else
    {
        strncpy(pDstText,pText,nMaxLen-1);
        pDstText[nMaxLen-1] = NULL;
    }
          
    UnuseAndDispose(refText);

    return pDstText;

}

int CPaigeEdtView::StrPgCmp(
    const char* pSrcText,
    select_pair& sel,
    BOOL                        bMatchCase , // = TRUE
    BOOL                        bWholeWord,  // = FALSE
    BOOL                        bSelect)         // = FALSE
{

    //
    // Compare length of strings first
    //
    long lSrcLen = strlen(pSrcText);
    if ( lSrcLen > (sel.end-sel.begin)  )
        return 1;
    else if ( lSrcLen < (sel.end-sel.begin) )
        return -1;
    //
    // Strings are same size, so examine text
    //
    text_ref    ref_for_text;
    long                length;
    pg_byte_ptr pText;
    pText = pgExamineText(m_paigeRef, sel.begin, &ref_for_text, &length);
    //
    // Just to be sure that selection doesn't cross text boundaries
    //
    length = min(lSrcLen,length);
    //
    // Comare the strings
    //
    int ret = strncmp(pSrcText, (char*) pText, lSrcLen);

    UnuseMemory(ref_for_text);
        
    return ret;

}

void CPaigeEdtView::OnSpecialAddAsRecipient()
{
    select_pair sel;
    char buf[1024];

    pgGetSelection(m_paigeRef, &sel.begin, &sel.end);
    if ( sel.end != sel.begin )
    {
        GetPgText(buf, sizeof(buf), sel, FALSE);
        if (  (*buf != '\0') && (strchr(buf,'\r') == NULL ) )
            g_theRecipientDirector.AddCommand( buf );
    }
                                
}

void CPaigeEdtView::OnUpdateAddAsRecipient(CCmdUI* pCmdUI)
{       
    select_pair sel;
    char buf[1024];

    pCmdUI->Enable( FALSE );
    pgGetSelection(m_paigeRef, &sel.begin, &sel.end);
    if ( sel.end != sel.begin )
    {
        GetPgText(buf, sizeof(buf), sel, FALSE);
        if (  (*buf != '\0') && (strchr(buf,'\r') == NULL ) )
            pCmdUI->Enable( TRUE );
    }
}


BOOL CPaigeEdtView::GetTextAs( pg_ref theRef, CString &text, PgDataTranslator::FormatType format)
{
    text.Empty();

    // do the deed
    memory_ref pgmRef;
    PgDataTranslator theTxr( theRef );
    theTxr.ExportData( &pgmRef, format);

    // convert the PGMM reference to a c-string, and jam it in the doc. note
    // that anything in the paige instance (headers, whatever) is gonna
    // get saved as part of the message.
    char *theStr = (char *) UseMemoryToCStr( pgmRef );
        
    if (format == PgDataTranslator::kFmtText)
    {
        ConvertNBSPsToSpaces(theStr);
    }

    text = theStr;

    // get out before someone gets hurt
    UnuseAndDispose( pgmRef );

    if ( text.IsEmpty() )
        return FALSE;

    return TRUE;
}

BOOL CPaigeEdtView::GetSelectedText( CString& szText )
{
    if (! pgNumSelections(m_paigeRef) )
        return ( FALSE );

    select_pair selection;
    pgGetSelection(m_paigeRef, &selection.begin, &selection.end);
    pg_ref selRef = pgCopy(m_paigeRef, &selection);

    // need to stash ptr to the "stuffbucket"
    paige_rec_ptr pRec = (paige_rec_ptr) UseMemory( m_paigeRef );
    paige_rec_ptr pRecCopy = (paige_rec_ptr) UseMemory( selRef );
    pRecCopy->user_refcon = pRec->user_refcon;
    UnuseMemory( m_paigeRef );
    UnuseMemory( selRef );

    BOOL result = GetTextAs(selRef, szText, PgDataTranslator::kFmtText);

    pgDispose( selRef );
    return result;
}

BOOL CPaigeEdtView::GetAllText( CString& szText )
{
    if (! pgTextSize(m_paigeRef) )
        return ( FALSE );

    BOOL result = GetTextAs( m_paigeRef, szText, PgDataTranslator::kFmtText );
        
    return result;
}


BOOL CPaigeEdtView::GetSelectedHTML( CString& szHTML )
{
    if (! pgNumSelections(m_paigeRef) )
        return ( FALSE );

    select_pair selection;
    pgGetSelection(m_paigeRef, &selection.begin, &selection.end);
    pg_ref selRef = pgCopy(m_paigeRef, &selection);

    // need to stash ptr to the "stuffbucket"
    paige_rec_ptr pRec = (paige_rec_ptr) UseMemory( m_paigeRef );
    paige_rec_ptr pRecCopy = (paige_rec_ptr) UseMemory( selRef );
    pRecCopy->user_refcon = pRec->user_refcon;
    UnuseMemory( m_paigeRef );
    UnuseMemory( selRef );

    BOOL result = GetTextAs(selRef, szHTML, PgDataTranslator::kFmtHtml);

    pgDispose( selRef );
    return result;
}

BOOL CPaigeEdtView::GetAllHTML( CString& szText )
{
    if (! pgTextSize(m_paigeRef) )
        return ( FALSE );

    BOOL result = GetTextAs( m_paigeRef, szText, PgDataTranslator::kFmtHtml );

    return result;
}

BOOL CPaigeEdtView::GetMessageAsHTML( CString& szText, BOOL IncludeHeaders )
{
    //Implemented in derived classes
    ASSERT(0);
    return (FALSE);
}

BOOL CPaigeEdtView::GetMessageAsText( CString& szText, BOOL IncludeHeaders )
{
    //Implemented in derived classes
    ASSERT(0);
    return (FALSE);
}

//
//EXPECTS a selection pointer if text is to be replaced ELSE 
//inserts text at the current position or at the left selection position.
//
BOOL CPaigeEdtView::SetTextAs( 
    const char* text, 
    select_pair_ptr sel,
    PgDataTranslator::FormatType format,
    bool bSign)
{
    short verb = undo_translation;
    //
    // Paste=Delete+Insert.  Delete the current selection
    //
    if ((sel) && (sel->begin < sel->end))
    {
        PrepareUndo(undo_delete, (long *)sel, TRUE);
        pgDelete(m_paigeRef, sel, best_way);
        verb = undo_translation_replace;
    }
    //
    // Init the translator
    //
    memory_ref strRef = CString2Memory (PgMemGlobalsPtr(), (LPCTSTR)text);
    PgDataTranslator theTxr(m_paigeRef);
    //
    // Save beginning offset of selection
    //
    long first, last;
    pgGetSelection(m_paigeRef, &first, &last);
    //
    // Insert text and caculate how much text was inserted.
    //
    long oldSize, newSize;
    oldSize = pgTextSize(m_paigeRef);
    theTxr.ImportData(&strRef, format);
    newSize = pgTextSize(m_paigeRef);
    //
    // Store undo information
    //
    long insert_ref[2];
    insert_ref[0] = first;
    insert_ref[1] = newSize - oldSize;
    PrepareUndo(verb, insert_ref);

    DisposeMemory(strRef);

    //
    // Sign the inserted text
    //
    if (bSign)
    {
        pgGetSelection(m_paigeRef, &first,&last);
        pgSetSelection(m_paigeRef, insert_ref[0], last,0, FALSE);
        m_styleEx->ApplySigned(TRUE);
        pgSetSelection(m_paigeRef, first, last,0, TRUE);
    }

	//
	// Mark it for spelling
	//
	select_pair sel2 = {first,first + newSize - oldSize};
	m_styleEx->ClearMisspellingAndRecheck(&sel2);
	m_checkdastuff = true;
        
    ScrollToCursor();
        
    return ( TRUE );
}

BOOL CPaigeEdtView::SetSelectedText( const char* szText, bool bSign )
{
    select_pair sel;
    pgGetSelection(m_paigeRef, &sel.begin, &sel.end);

    BOOL result = SetTextAs(szText, &sel, PgDataTranslator::kFmtText, bSign);

    return result;
}

BOOL CPaigeEdtView::SetAllText( const char* szText, bool bSign )
{
    select_pair sel;

    sel.begin = 0;
    sel.end = pgTextSize(m_paigeRef);

    BOOL result = SetTextAs(szText, &sel, PgDataTranslator::kFmtText, bSign);

    return result;
}

BOOL CPaigeEdtView::SetSelectedHTML( const char* szHTML, bool bSign )
{
    select_pair sel;
    pgGetSelection(m_paigeRef, &sel.begin, &sel.end);

    BOOL result = SetTextAs(szHTML, &sel, PgDataTranslator::kFmtHtml, bSign);

    return result;
}

BOOL CPaigeEdtView::SetAllHTML( const char* szHTML, bool bSign )
{
    select_pair sel;

    sel.begin = 0;
    sel.end = pgTextSize(m_paigeRef);

    BOOL result = SetTextAs(szHTML, &sel, PgDataTranslator::kFmtHtml, bSign);

    return result;
}

BOOL CPaigeEdtView::PasteOnDisplayHTML( const char* szHTML,bool bSign )
{
    return SetSelectedHTML(szHTML, bSign);
}


BOOL CPaigeEdtView::ClearFormat()
{
    par_info parInfo, parInfoMask;
    long styleID;
    if ( styleID = pgGetNamedStyleIndex (m_paigeRef, body_style) )
        pgGetNamedStyleInfo(m_paigeRef, styleID, NULL, NULL, &parInfo);
    else
        parInfo = PgGlobalsPtr()->def_par;
    pgFillBlock( &parInfoMask, sizeof(par_info), SET_MASK_BITS );

    //
    // Don't clear excerpting
    //
    parInfoMask.user_data=0;
    parInfoMask.user_id = 0;
    parInfoMask.procs.line_glitter = 0;
    parInfoMask.left_extra =0;

    pgSetParInfo( m_paigeRef, NULL, &parInfo, &parInfoMask, best_way );
        
    return TRUE;
}

BOOL CPaigeEdtView::ClearStyle()
{
    style_info  mask, info;
    font_info   fontMask, fontInfo;
    pgFillBlock( &mask, sizeof(style_info), SET_MASK_BITS );
    pgFillBlock( &fontMask, sizeof(font_info), SET_MASK_BITS );

    long styleID;
    if ( (styleID = pgGetNamedStyleIndex (m_paigeRef, body_style) ) )
    {
        pgGetNamedStyleInfo(m_paigeRef, styleID, &info, &fontInfo, NULL);
    }
    else
    {
        info = PgGlobalsPtr()->def_style;
        fontInfo = PgGlobalsPtr()->def_font;
    }
    pgSetStyleInfo(m_paigeRef, NULL, &info, &mask, draw_none);
    pgSetFontInfo(m_paigeRef, NULL, &fontInfo, &fontMask, best_way);
    return TRUE;
}

BOOL CPaigeEdtView::ClearHyperlink()
{
    //
    // Get Selection range
    //
    select_pair sel;
    pgGetSelection(m_paigeRef, &sel.begin, &sel.end);

    pg_hyperlink hyperlink;
    long offset = sel.begin;
    while (( pgGetHyperlinkSourceInfo (m_paigeRef, offset, 0, true, &hyperlink) ) &&
           (hyperlink.applied_range.begin < sel.end))
    {
        if ( !((hyperlink.type & HYPERLINK_EUDORA_ATTACHMENT)   ||
               (hyperlink.type & HYPERLINK_EUDORA_PLUGIN          )     ||
               (hyperlink.type & HYPERLINK_EUDORA_AUTOURL   ) ))
            pgDeleteHyperlinkSource (m_paigeRef, hyperlink.applied_range.begin, true);
        offset = hyperlink.applied_range.end+1;
    }
        
    return TRUE;

}

void CPaigeEdtView::OnFontChange()
{
    CComboBox*                  pCombo;
    CString                             szString;
    QCChildToolBar*             pFormattingToolBar;

    CFrameWnd* Frame = (CFrameWnd*) GetParentFrame();
    QCProtocol* pProtocol = QCProtocol::QueryProtocol( QCP_FORMAT_TOOLBAR, Frame );

    if( pProtocol == NULL ) 
        return ;
        
    SetFocus();

    pFormattingToolBar = pProtocol->GetFormatToolbar();
    VERIFY( pCombo = ( CComboBox* )  pFormattingToolBar->GetDlgItem( IDC_FONT_COMBO ));
        
    if( pCombo->GetCurSel() < 0 )
        return;
        
    pCombo->GetLBText( pCombo->GetCurSel(), szString );

    if ( pgNumSelections(m_paigeRef) )
        PrepareUndo(undo_format);
    pgSetFontByName( m_paigeRef,
                     (const pg_font_name_ptr)(LPCSTR)szString, NULL, TRUE );


}

void CPaigeEdtView::OnUpdateFontCombo(CCmdUI* pCmdUI) 
{
    CComboBox*                  pCombo;
    QCChildToolBar*             pFormattingToolBar;

    CFrameWnd* Frame = (CFrameWnd*) GetParentFrame();
    QCProtocol* pProtocol = QCProtocol::QueryProtocol( QCP_FORMAT_TOOLBAR, Frame );

    if( pProtocol == NULL ) 
        return ;
        
    pFormattingToolBar = pProtocol->GetFormatToolbar();
    VERIFY( pCombo = ( CComboBox* )  pFormattingToolBar->GetDlgItem( IDC_FONT_COMBO ));
        
    if ( !pCombo->GetDroppedState() )
    {
        char szFontName[256]={NULL};
        pgGetFontByName (m_paigeRef, (unsigned char*) &szFontName)      ;
        if( *szFontName != NULL )
        {
            int i = pCombo->FindStringExact( -1, szFontName );
            if ( pCombo->GetCurSel() != i )
                pCombo->SetCurSel( i );
        }
    }
        
    pCmdUI->Enable( TRUE );
        
}



// Happy-crappy picture stuff...

void CPaigeEdtView::OnInsertPicture()
{
	// Shareware: In reduced feature mode, you cannot insert pictures
	if (UsingFullFeatureSet())
	{
		// FULL FEATURE mode
		char* filter = NULL;
		PgGetOfnFilterString( &filter );
		CFileDialog dlg( true, "jpg", NULL, 0, filter, this );

		if ( dlg.DoModal() == IDOK ) {
			unsigned er;
			if ( (er = PgValidateImageFile(dlg.GetPathName())) == 0 ) {
				//Need to call Undo here, maybe with app_insert verb
				PrepareUndo(undo_embed_insert);
				// Schmookie!
				PgInsertImageFromFile( m_paigeRef, dlg.GetPathName() );
				Invalidate();
			}
			else
				ErrorDialog( er );
		}
	}
}

void CPaigeEdtView::OnUpdateInsertPicture(CCmdUI* pCmdUI) 
{
	// Shareware: In reduced feature mode, you cannot insert pictures
    pCmdUI->Enable( !m_fRO && m_bAllowStyled && UsingFullFeatureSet() /*&& g_qtInitialized*/ );
}

void CPaigeEdtView::OnEditInsertHR()
{
	// Shareware: In reduced feature mode, you cannot insert horiz line
	if (UsingFullFeatureSet())
	{
		// FULL FEATURE mode
		select_pair sel;
		pgGetSelection(m_paigeRef, &sel.begin, &sel.end);
		short verb = undo_insert_hr;

		long lineLeft, lineRight;
		pgFindLine(m_paigeRef, sel.begin, &lineLeft, &lineRight);
		if (lineRight != lineLeft + 1)
		{
			TCHAR uChar = (TCHAR)PgGlobalsPtr()->line_wrap_char;
			long insert_Ref[2];
			insert_Ref[0] = lineLeft;
			insert_Ref[1] = 1;
			PrepareUndo(undo_app_insert, insert_Ref, TRUE);
			pgInsert( m_paigeRef, (pg_byte_ptr)&uChar, 1, lineLeft, 
					  key_insert_mode, 0, best_way );
			verb = undo_insert_hr_withCR;
		}
        
		sel.begin = lineLeft;
		sel.end = lineLeft;

		par_info info, mask;
		pgGetParInfoEx(m_paigeRef, &sel, FALSE, &info, &mask, FALSE);

		info.table.border_info |= PG_BORDER_LINERULE;

		PrepareUndo(verb);

		pgSetParInfoEx(m_paigeRef, &sel, &info, &mask, FALSE, best_way);

		//TEMP Uncomment after updating to new Paige srcs       
		/*      style_info styleInfo, styleMask;
				sel.end += 1;
				pgGetStyleInfo(m_paigeRef, &sel, FALSE, &styleInfo, &styleMask);
				styleInfo.styles[linerule_var] = 1;
        
				pgSetStyleInfo(m_paigeRef, &sel, &styleInfo, &styleMask, best_way);
		*/      
		sec_toolbar_menu_hack();
	}
}

void CPaigeEdtView::OnUpdateEditInsertHR( CCmdUI* pCmdUI )
{
	// Shareware: In reduced feature mode, you cannot insert horiz line
	if (UsingFullFeatureSet())
	{
		// FULL FEATURE mode
	    OnUpdateEditStyle( pCmdUI );
	}
	else
	{
		// REDUCED FEATURE mode
		pCmdUI->Enable(FALSE); // Disable in REDUCED FEATURE mode
	}
}

void CPaigeEdtView::OnUpdateEditStyle( CCmdUI* pCmdUI )
{
    // Not to be confused with OnUpdateEdible.
    if ((m_fRO) || (!m_bAllowStyled))
        pCmdUI->Enable( FALSE );
    else
        pCmdUI->Enable( TRUE );
}


void CPaigeEdtView::OnTextSizeChange( UINT uSizeID ) 
{
    if ( pgNumSelections( m_paigeRef ) ) {
        PrepareUndo( undo_format );
    }

    unsigned i = (uSizeID - ID_EDIT_TEXT_SIZE_VERY_SMALL);
    int baseFontSize = GetIniShort(IDS_INI_MESSAGE_FONT_SIZE);
    short size = (short)(GetFontSizeMap()[(int)baseFontSize][i] >> 16);
    pgSetPointSize( m_paigeRef, size, NULL, TRUE );
    sec_toolbar_menu_hack();
}


void CPaigeEdtView::OnColorChange( UINT uColorID ) 
{
    if ( pgNumSelections( m_paigeRef ) ) {
        PrepareUndo( undo_format );
    }

    COLORREF theColor =  CColorMenu::GetColor( uColorID );
    pgSetTextColor( m_paigeRef, &theColor, NULL, TRUE );
    sec_toolbar_menu_hack();
}


void CPaigeEdtView::OnClearFormatting() 
{
    if ( pgNumSelections(m_paigeRef) )
        PrepareUndo(undo_format);
    ClearStyle();
    ClearFormat();      
    ClearHyperlink();
}


void CPaigeEdtView::OnUpdateClearFormatting(CCmdUI* pCmdUI) 
{
    select_pair sel;

    pgGetSelection(m_paigeRef, &sel.begin, &sel.end);
	bool bExcerptCausedStyle;
    if ( PgHasStyledText(m_paigeRef, PgGlobalsPtr(), &sel, &bExcerptCausedStyle) && !bExcerptCausedStyle)
        pCmdUI->Enable( TRUE );
    else
        pCmdUI->Enable( FALSE );
        
}

void CPaigeEdtView::UpdateBlockFmt(
    CCmdUI* pCmdUI,
    unsigned char* szStylename) 
{
    if (m_fRO)
        pCmdUI->Enable( FALSE );
    else
        pCmdUI->Enable( TRUE );
        
    short styleid;
    pgGetStyleSheet(m_paigeRef, NULL, &styleid);
    long id = pgGetNamedStyleIndex(m_paigeRef,szStylename);
    if ( styleid == id )
        pCmdUI->SetRadio( TRUE );
    else
        pCmdUI->SetRadio( FALSE );
}



void CPaigeEdtView::OnBlkfmtBullettedList() 
{
 	// Shareware: In reduced feature mode, you cannot insert bullet lists
	if (UsingFullFeatureSet())
	{
		// FULL FEATURE mode
		if ( pgNumSelections(m_paigeRef) )
			PrepareUndo(undo_format);
		if ( !m_styleEx->IsBullet(NULL) )
			m_styleEx->ApplyBullet(TRUE);
		else
			m_styleEx->ApplyBullet(FALSE);
	}
}

void CPaigeEdtView::OnUpdateBlkfmtBullettedList(CCmdUI* pCmdUI) 
{
 	// Shareware: In reduced feature mode, you cannot insert bullet lists
	if (UsingFullFeatureSet())
	{
		// FULL FEATURE mode
		if ( m_styleEx->IsBullet(NULL) )
			pCmdUI->SetCheck( TRUE );
		else
			pCmdUI->SetCheck( FALSE );

		if (!m_bAllowStyled)
			pCmdUI->Enable(FALSE);
	}
	else
	{
		// REDUCED FEATURE mode
		pCmdUI->Enable(FALSE);
	}
}

/*

void CPaigeEdtView::OnBlkfmtBq() 
{
        if ( pgNumSelections(m_paigeRef) )
                PrepareUndo(undo_format);
        pgApplyNamedStyle (m_paigeRef, NULL, blockquote_style,  best_way);
}

void CPaigeEdtView::OnUpdateBlkfmtBQ(CCmdUI* pCmdUI) 
{
        UpdateBlockFmt( pCmdUI, blockquote_style );
}

void CPaigeEdtView::OnBlkfmtH1() 
{
        if ( pgNumSelections(m_paigeRef) )
                PrepareUndo(undo_format);
        pgApplyNamedStyle (m_paigeRef, NULL, heading1_style,  best_way);
}

void CPaigeEdtView::OnUpdateBlkfmtH1(CCmdUI* pCmdUI) 
{
        UpdateBlockFmt( pCmdUI, heading1_style );
}

void CPaigeEdtView::OnBlkfmtH2() 
{
        if ( pgNumSelections(m_paigeRef) )
                PrepareUndo(undo_format);
        pgApplyNamedStyle (m_paigeRef, NULL, heading2_style,  best_way);
}

void CPaigeEdtView::OnUpdateBlkfmtH2(CCmdUI* pCmdUI) 
{
        UpdateBlockFmt( pCmdUI, heading2_style );
}

void CPaigeEdtView::OnBlkfmtH3() 
{
        if ( pgNumSelections(m_paigeRef) )
                PrepareUndo(undo_format);
        pgApplyNamedStyle (m_paigeRef, NULL, heading3_style,  best_way);
}

void CPaigeEdtView::OnUpdateBlkfmtH3(CCmdUI* pCmdUI) 
{
        UpdateBlockFmt( pCmdUI, heading3_style );
}

void CPaigeEdtView::OnBlkfmtH4() 
{
        if ( pgNumSelections(m_paigeRef) )
                PrepareUndo(undo_format);
        pgApplyNamedStyle (m_paigeRef, NULL, heading4_style,  best_way);
}

void CPaigeEdtView::OnUpdateBlkfmtH4(CCmdUI* pCmdUI) 
{
        UpdateBlockFmt( pCmdUI, heading4_style );
}

void CPaigeEdtView::OnBlkfmtH5() 
{
        if ( pgNumSelections(m_paigeRef) )
                PrepareUndo(undo_format);
        pgApplyNamedStyle (m_paigeRef, NULL, heading5_style,  best_way);
}

void CPaigeEdtView::OnUpdateBlkfmtH5(CCmdUI* pCmdUI) 
{
        UpdateBlockFmt( pCmdUI,heading5_style );
}

void CPaigeEdtView::OnBlkfmtH6() 
{
        if ( pgNumSelections(m_paigeRef) )
                PrepareUndo(undo_format);
        pgApplyNamedStyle (m_paigeRef, NULL, heading6_style,  best_way);
}

void CPaigeEdtView::OnUpdateBlkfmtH6(CCmdUI* pCmdUI) 
{
        UpdateBlockFmt( pCmdUI,heading6_style );
}


void CPaigeEdtView::OnBlkfmtNormal() 
{
//      OnClearFormatting();
        if ( pgNumSelections(m_paigeRef) )
                PrepareUndo(undo_format);
        pgApplyNamedStyle (m_paigeRef, NULL, body_style,  best_way);

}

void CPaigeEdtView::OnUpdateBlkfmtNormal(CCmdUI* pCmdUI) 
{
        UpdateBlockFmt( pCmdUI,body_style );

}
*/
//
//  This is a helper function for the tri-state toolbar buttons.  
//  1 = style is consistently on.
//  0 = style is consistently off.
//  2 = style is both on and off in this selection.
//
int CPaigeEdtView::GetStyleState (int index)
{
    style_info                  info, mask;
    pgInitStyleMask(&info,0);
    pgInitStyleMask(&mask,0);

    //
    // Style is consistent throughout selection
    //
    pgGetStyleInfo(m_paigeRef, NULL, FALSE, &info, &mask);
    if ( (info.styles[index] != 0) && ( mask.styles[index] != 0 ) )
        return 1;
    //
    // Style is indeterminate
    // 
    info.styles[index] = -1; 
    pgGetStyleInfo(m_paigeRef, NULL, TRUE, &info, &mask);
    if ( info.styles[index] != 0 )
    {
        if ( mask.styles[index] != 0 )
            return 2;
    }
    //
    // Style is not present
    //
    return 0;

        
}


int CPaigeEdtView::GetJustifyState (int index)
{
    par_info                    info, mask;
    pgInitParMask(&info,0);
    pgInitParMask(&mask,0);

    //
    // Style is consistent throughout selection
    //
    pgGetParInfo(m_paigeRef, NULL, FALSE, &info, &mask);
    if ( (info.justification == index) && ( mask.justification != 0 ) )
        return 1;
    //
    // Style is indeterminate
    // 
    info.justification = (short) index; 
    pgGetParInfo(m_paigeRef, NULL, TRUE, &info, &mask);
    if ( info.justification == index )
    {
        if ( mask.justification != 0 )
            return 2;
    }
    //
    // Style is not present
    //
    return 0;
        
}

int CPaigeEdtView::MapScrollPos(int nPos, int sourceMax, int dstMax)
{
    int ret = 0;
        
    if (sourceMax)
    {
        float ratio = (float)(nPos)/(float)sourceMax;
        ret = (int)((ratio * (float)dstMax)+.5);
                
        ret = min(dstMax,ret);
    }   
    return ret;
}

void CPaigeEdtView::OnUpdateNeedSelEdit(CCmdUI* pCmdUI)
{
    if (m_fRO)
        pCmdUI->Enable(FALSE);
    else
        pCmdUI->Enable(pgNumSelections(m_paigeRef));            
}

void CPaigeEdtView::OnEditWrapSelection()
{
    BOOL DoUnwrap = ShiftDown();
    if( !m_fRO)
    {
        OnEditCopy();
        if (OpenClipboard())
        {
            HANDLE hText = GetClipboardData(CF_TEXT);
            if (hText)
            {
                char* text = (char*)GlobalLock(hText);
                if (DoUnwrap)
                    UnwrapText(text);
                else
                    text = WrapText(text);
                if (text)
                {
                    SetSelectedText(text,FALSE);
                    if (!DoUnwrap)
                        delete [] text;
                }
                GlobalUnlock(hText);
            }
        }
        CloseClipboard();
    }
}

//extern "C" PG_PASCAL (long) hlCallback( paige_rec_ptr pg, pg_hyperlink_ptr hypertext,
//                      short command, short modifiers, long position, pg_char_ptr URL );


void CPaigeEdtView::OnUpdateInsertLink(CCmdUI* pCmdUI) 
{
	// Shareware: In reduced feature mode, you cannot insert hyperlinks
	if (UsingFullFeatureSet())
	{
		// FULL FEATURE mode
		bool bEnable = false;

		if (m_bAllowStyled)
		{
			select_pair sel;
			pgGetSelection(m_paigeRef, &sel.begin, &sel.end);
			//
			// If caret/selection contains a hyperlink, then the functionality is enabled
			// unless that hyperlink is a special, Eudora generated URL.
			//
			pg_hyperlink hyperlink;
			if ( pgGetHyperlinkSourceInfo (m_paigeRef, sel.begin, 0, false, &hyperlink) ||
				 pgGetHyperlinkSourceInfo (m_paigeRef, sel.end, 0, false, &hyperlink) )
			{
				bEnable = !((hyperlink.type & HYPERLINK_EUDORA_ATTACHMENT)  ||
							(hyperlink.type & HYPERLINK_EUDORA_PLUGIN         )     ||
							(hyperlink.type & HYPERLINK_EUDORA_AUTOURL   ) );                       
			}
			else
				//
				// Not in a hyperlink so must have a selection range to be valid
				// 
				bEnable = (sel.begin != sel.end);
		}

		pCmdUI->Enable( bEnable );
	}
	else
	{
		// REDUCED FEATURE mode
		pCmdUI->Enable( FALSE );
	}
}

void CPaigeEdtView::OnEditInsertLink()
{
	// Shareware: In reduced feature mode, you cannot insert hyperlinks
	if (UsingFullFeatureSet())
	{
		// FULL FEATURE mode
		char pURL[INTERNET_MAX_URL_LENGTH];

		pURL[0] = 0;
		//
		// Editing or Creating a Hyperlink?
		//
		select_pair sel;
		pgGetSelection(m_paigeRef, &sel.begin, &sel.end);

		pg_hyperlink hyperlink;
		bool bEdit=FALSE;
		if ( (pgGetHyperlinkSourceInfo (m_paigeRef, sel.begin, 0, false, &hyperlink)) ||
			 (pgGetHyperlinkSourceInfo (m_paigeRef, sel.end  , 0, false, &hyperlink)))
		{
			sel.begin = min(sel.begin, hyperlink.applied_range.begin);
			sel.end   = max(sel.end, hyperlink.applied_range.end); 
			pgSetSelection(m_paigeRef, sel.begin, sel.end,0,true);
			//The URL source may have been stored in URL (char *) or 
			//cloaked in alt_URL (memory_ref) So use the function to get 
			//the URL.
			//pURL = hyperlink.URL;
			pgGetSourceURL(m_paigeRef, hyperlink.applied_range.begin, (unsigned char*)pURL, INTERNET_MAX_URL_LENGTH);
			bEdit = TRUE;
		}
		CInsertLinkDialog dlg((const char*)pURL);
		pgSetHiliteStates(m_paigeRef, activate_verb,activate_verb , FALSE);
		m_bDontDeactivateHighlight=TRUE;
		if ( dlg.DoModal() != IDOK)
			return;
		m_bDontDeactivateHighlight=FALSE;

		//Manually set the modified flag since there is no undo support for this.
		GetDocument()->SetModifiedFlag();
        
		//CString szURL = dlg.GetURL();
		CString szURL = EscapeURL( dlg.GetURL() );
		//pURL = new unsigned char[szURL.GetLength()+1];
		//strcpy(pURL, szURL);
		strncpy(pURL, szURL, INTERNET_MAX_URL_LENGTH-1);

		if (bEdit)
			pgChangeHyperlinkSource (m_paigeRef, hyperlink.applied_range.begin,     &sel,
									 (unsigned char*)pURL,NULL, hlCallback, 0, 0,0, best_way);
		else
		{
			paige_rec_ptr pgPtr = (paige_rec_ptr) UseMemory(m_paigeRef);
			pgSetHyperlinkSource (m_paigeRef, NULL,(unsigned char*) pURL,NULL, hlCallback,
								  HYPERLINK_EUDORA_URL,  pgAssignLinkID(pgPtr->hyperlinks),0,0,0, best_way);
			UnuseMemory(m_paigeRef);
		}
        
		//delete [] pURL;
	}
}


//theStr is an IN-OUT parameter. If the scheme is www. or ftp. then
// http:// and ftp:// are appended to the beginning of the string and
// this resultant string should be used to set the href
BOOL ValidURLScheme(char *theStr)
{
    char buffer[INTERNET_MAX_URL_LENGTH];

    if (strnicmp(theStr, "www.", 4) == 0)
    {
        strcpy(buffer, "http://");
        strncat(buffer, theStr, min(strlen(theStr), INTERNET_MAX_URL_LENGTH-8));
        strcpy(theStr, buffer);
        return TRUE;
    }
    else if (strnicmp(theStr, "ftp.", 4) == 0)
    {
        strcpy(buffer, "ftp://");
        strncat(buffer, theStr, min(strlen(theStr), INTERNET_MAX_URL_LENGTH-7));
        strcpy(theStr, buffer);
        return TRUE;
    }
        
    if (IsURLScheme(theStr))
        return TRUE;
    else 
        return FALSE;
}


BOOL CPaigeEdtView::MakeAutoURLSpaghetti(long start, long end)
{
    long linkID = 0;
    select_pair sel;
    char buf[INTERNET_MAX_URL_LENGTH], tempBuf[INTERNET_MAX_URL_LENGTH], startChar = ' ';
    pg_hyperlink autoLink;

    InitHtmlStrings();

    paige_rec_ptr pgPtr = (paige_rec_ptr) UseMemory(m_paigeRef);

    //Validate existing AutoURLs first since text may have been added 
    //making them invalid or stale.

    long offset = start;
    long charInfo;

    while (offset < end)
    {
        if ( pgGetHyperlinkSourceInfo(m_paigeRef, offset, 0, true, &autoLink) )
        {
            if (autoLink.applied_range.begin >= end) 
                break;

            if (autoLink.type & HYPERLINK_EUDORA_AUTOURL)
            {
                                //Check if the beginning is valid, if it is not, delete the link
                                //Any valid link will be found later
                if (offset > start)
                    charInfo = pgCharType(m_paigeRef, offset - 1, UPPER_CASE_BIT | LOWER_CASE_BIT);

                if ((offset > start) && ( charInfo & (UPPER_CASE_BIT | LOWER_CASE_BIT) ))
                    pgDeleteHyperlinkSource (m_paigeRef, autoLink.applied_range.begin, true);
                else
                {
                    startChar = ' ';
                                        
                    if (offset > start)
                    {
                        unsigned char buf[8];
                        pgCharByte(m_paigeRef, offset - 1, buf);
                        startChar = buf[0];
                        if (startChar != '"' || startChar != '<')
                            startChar = ' ';
                    }

                    //The URL beginning is still valid, now check if the end is valid
                    offset = FindValidURLEnd(m_paigeRef, offset, end, autoLink.applied_range.begin, startChar);
                    if (offset != autoLink.applied_range.end)
                    {
                        //Update the end offset for the URL
                        autoLink.applied_range.end = offset;
                    }
                    //Sanity check for URL
                    if (autoLink.applied_range.begin != autoLink.applied_range.end)
                    {
                        GetPgText(buf, sizeof(buf), autoLink.applied_range, FALSE);
                        if (ValidURLScheme(buf))
                        {
                            if (URLIsValid(buf))
                            {
                                tempBuf[0] = 0;
                                strncat(tempBuf, EscapeURL(buf), INTERNET_MAX_URL_LENGTH-1);
                                pgChangeHyperlinkSource(m_paigeRef, autoLink.applied_range.begin, 
                                                        &autoLink.applied_range, (unsigned char *)tempBuf, NULL, NULL, NULL, NULL, NULL, best_way);
                            }
                        }
                        else
                            pgDeleteHyperlinkSource(m_paigeRef, autoLink.applied_range.begin, TRUE);
                    }
                    else
                        pgDeleteHyperlinkSource(m_paigeRef, autoLink.applied_range.begin, TRUE);
                }
            }
        }
        else 
            break;
        offset = autoLink.applied_range.end+1;
    }

    //Parse the text for any new AutoURLs (new message / since last saved)
    sel.begin = start;
    startChar = ' ';

    while (sel.begin < end)
    {
        //Finds the beginning of the next word, skipping all spaces and tabs and newlines.
        sel.begin = FindValidURLStart(m_paigeRef, sel.begin, end, startChar);

        if (sel.begin == end)
            break;

        //If it is already a hyper link, don't mess with it, not even AutoURLs since they have been
        //processed in the loop above
        if ( linkID = pgGetSourceID(m_paigeRef, sel.begin) )
        {
            pgGetHyperlinkSourceInfo(m_paigeRef, sel.begin, linkID, TRUE, &autoLink);

            //End of the hyperlink is where our checking begins !!
            pgFindHyperlinkSourceByID(m_paigeRef, sel.begin, &sel.begin, linkID, FALSE);
                        
            //Finds the beginning of the next word, skipping all spaces and tabs and newlines.
            sel.begin = FindValidURLStart(m_paigeRef, sel.begin, end, startChar);
                        
            //Don't walk off the end
            if (sel.begin == end)
                break;
        }

        //Find the ending of a possible auto URL
        sel.end = FindValidURLEnd(m_paigeRef, sel.begin, end, sel.begin, startChar);

        if (sel.begin != sel.end)
        {
            if ( sel.end - sel.begin > sizeof(buf) )
                sel.end = sel.begin + sizeof(buf) - 1;

            GetPgText(buf, sizeof(buf), sel, FALSE);
                        
            if ( *buf && ValidURLScheme(buf) )
            {
                tempBuf[0] = 0;
                strncat(tempBuf, EscapeURL(buf), INTERNET_MAX_URL_LENGTH-1);
                                //Make the AutoURL
                pgSetHyperlinkSource (m_paigeRef, &sel,(unsigned char*)tempBuf, NULL, hlCallback,
                                      HYPERLINK_EUDORA_AUTOURL, pgAssignLinkID(pgPtr->hyperlinks),0,0,0, best_way);
            }
        }

        if (sel.end == end)
            break;
                
        sel.begin = sel.end;
    }
        
    UnuseMemory(m_paigeRef);
    return TRUE;
}

long FindNextWhitespace(pg_ref pg, long offset, long end)
{
    long dontCareAboutThis;
    long charInfo;
    char theChar;

    charInfo = pgCharType(pg, offset, BLANK_BIT | TAB_BIT | PAR_SEL_BIT);

    while ((offset < end) && !(charInfo & (BLANK_BIT | TAB_BIT | PAR_SEL_BIT)))
    {
        unsigned char buf[8];
        int byteCount = pgCharByte(pg, offset, buf);
        if (byteCount > 1 || (buf[0] & 0x80))
        {
            offset += byteCount;
            continue;
        }
        theChar = buf[0];
        if (theChar == '<' || theChar == '"')
            break;
                
        offset++;
        if (offset == end)
            break;
                
        charInfo = pgCharType(pg, offset, BLANK_BIT | TAB_BIT | PAR_SEL_BIT);
        if ( charInfo & (BLANK_BIT | TAB_BIT | PAR_SEL_BIT) )
            break;
                
        pgFindWord(pg, offset, &dontCareAboutThis, &offset, TRUE, FALSE);
                
        charInfo = pgCharType(pg, offset, BLANK_BIT | TAB_BIT | PAR_SEL_BIT);
    }

    return offset;
}

long SkipWhiteSpaces(pg_ref pg, long offset, long end)
{
    long charInfo;

    charInfo = pgCharType(pg, offset, BLANK_BIT | TAB_BIT | PAR_SEL_BIT);

    while ((offset < end) && (charInfo & (BLANK_BIT | TAB_BIT | PAR_SEL_BIT)))
    {
        offset++;
        if (offset == end)
            break;
        charInfo = pgCharType(pg, offset, BLANK_BIT | TAB_BIT | PAR_SEL_BIT);
    }

    return offset;
        
}

/*********************************************************
Skips all whitespaces and punctuations to find the 
beginning of a URL. The startChar is defaulted to a
space and is replaced with a valid URL start punctuation 
like < or ". This startChar should be used to determine 
what the valid end of the URL is, the closing punctuation 
> or " OR a whitespace
**********************************************************/
long FindValidURLStart(pg_ref pg, long offset, long end, char& startChar)
{
    //FOR REFERENCE PURPOSES
    //Paige considers the following as punctuation chars
    //       ! # $ % & * ( ) + - = : " ; ' < > ? , . /
    //Paige considers the following as lower case
    //       ~ @ ^ _ \ { } | \ [ ] 

    long charInfo;
    char theChar;
    BOOL notfound = TRUE;

    startChar = ' ';

    while ( notfound && (offset < end) )
    {
        charInfo = pgCharType(pg, offset, BLANK_BIT | TAB_BIT | PAR_SEL_BIT | 
                              PUNCT_NORMAL_BIT | UPPER_CASE_BIT | LOWER_CASE_BIT);

        unsigned char buf[8];
        int count = pgCharByte(pg, offset, buf);
        if (count > 1)
        {
            // Multibyte character definitely isn't something we're looking for
            offset += count;
            continue;
        }
        theChar = buf[0];
        if (theChar & 0x80)
        {
            // 8-bit characters aren't matches either
            offset++;
            continue;
        }

        // Skip if whitespace
        if ( charInfo & (BLANK_BIT | TAB_BIT | PAR_SEL_BIT) )
        {
            startChar = ' ';
            offset++;
            // Whitespace chars are also lower case, so don't process it again
            continue;
        }

        //Skip if punctuation
        if (charInfo & PUNCT_NORMAL_BIT)
        {
            // Remember the char if it is a valid URL enclosing char
            if ( (theChar == '"') || (theChar == '<') )
                startChar = theChar;
            offset++;
            continue;
        }

        if (charInfo & LOWER_CASE_BIT)
        {
            if (theChar != '~' && theChar != '@' && theChar != '^'
                && theChar != '_' && theChar != '`' && theChar != '{' 
                && theChar != '}' && theChar != '|' && theChar != '[' 
                && theChar != ']' && theChar != '\\')
                notfound = FALSE;
            else 
                offset++;

            continue;
        }

        if (charInfo & UPPER_CASE_BIT)
        {
            notfound = FALSE;
            continue;
        }

        // Maybe a digit or some other character that hasn't been covered by 
        // the cases above
        offset++;
    }

    return offset;
}

long FindValidURLEnd(pg_ref pg, long offset, long end, long begin, char delimiter)
{
    BOOL found = FALSE;
    unsigned char theChar[8];
    long temp = offset;

    if ( (delimiter == '"') || (delimiter == '<') )
    {
        //Make sure we check for the correct enclosing char
        if ( delimiter == '<' )
            delimiter = '>';

        int startCount = 1;

        while ( !found && (temp < end) )
        {
            pgCharByte(pg, temp, theChar);
                        
            //if we have a URL in <>, then it can have embedded <>
            if (*theChar == '<')
                startCount++;
                        
            if (*theChar == delimiter)
            {
                startCount--;
                                
                if (startCount == 0)
                    found = TRUE;
                else 
                    temp++;
            }
            else
                temp++;
        }

        if (found)
            return temp;
    }

    // Either we haven't found the matching " or > OR 
    // the URL delimiter is a space
    long whitespace = FindNextWhitespace(pg, offset, end);

    long charInfo;

    //Now traverse backwards to see if we have a punctuation 
    //at the end of the URL
    if (whitespace > offset)
        --whitespace;

    while (whitespace > offset)
    {
        pgCharByte(pg, whitespace, theChar);

        unsigned char c = *theChar;
        if ( c == '/' || c == '\\' )
            break;

        // If it is one of those lower case punctuation thingies, it's not valid
        if (c == '~' || c == '@' && c == '^'
            || c == '_' || c == '`' || c == '{' 
            || c == '}' || c == '|' || c == '[' 
            || c == ']' /*|| c == '\'*/)
        {
            --whitespace;
            continue;
        }


        charInfo = pgCharType(pg, whitespace, PUNCT_NORMAL_BIT);
                
        if ( ! (charInfo & PUNCT_NORMAL_BIT) )
            break;
        else 
            --whitespace;
    }

    return ++whitespace;
}

BOOL CPaigeEdtView::CleanURLonPosteriorDelete(long selectBegin)
{
    long linkID;
    if (linkID = pgGetSourceID(m_paigeRef, selectBegin))
    {
        pg_hyperlink pgLink;

        pgGetHyperlinkSourceInfo(m_paigeRef, selectBegin, linkID, TRUE, &pgLink);

        if ( (selectBegin > pgLink.applied_range.begin) && 
             (selectBegin < pgLink.applied_range.end) )
        {
            pgLink.applied_range.end = selectBegin;
//                      pgDeleteHyperlinkSource(m_paigeRef, pgLink.applied_range.begin, TRUE);
//                      pgSetHyperlinkSource (m_paigeRef, &pgLink.applied_range, pgLink.URL, NULL,
//                              hlCallback, HYPERLINK_EUDORA_AUTOURL, pgLink.unique_id,0,0,0, best_way);
            pgSetHyperlinkSourceInfo(m_paigeRef, pgLink.applied_range.begin, 
                                     pgLink.unique_id, &pgLink);
            return TRUE;
        }
    }
    return FALSE;
}

void CPaigeEdtView::OnBlkfmtFixedwidth() 
{
    bool bOn = true;

    if ( pgNumSelections(m_paigeRef) )
        PrepareUndo(undo_format);

    style_info                  info, mask;
    pgInitStyleMask(&info,0);
    pgInitStyleMask(&mask,0);
    long id = pgGetNamedStyleIndex(m_paigeRef,teletype_style);
    //
    // Style is consistent throughout selection
    //
    pgGetStyleInfo(m_paigeRef, NULL, FALSE, &info, &mask);
    if ( (abs(info.style_sheet_id) == id) && ( mask.style_sheet_id != 0 ) )
        bOn = false;
    else
        //
        // Style is indeterminate
        // 
        if ( mask.style_sheet_id == 0 )
            bOn=true;
        else
                                //
                                // Style is not present
                                //
            bOn=true;
    if (bOn)
        pgApplyNamedStyle (m_paigeRef, NULL, teletype_style,  best_way);
    else
    {
        pgApplyNamedStyle (m_paigeRef, NULL, body_style,  best_way);    
    }
}

void CPaigeEdtView::OnUpdateBlkfmtFixedwidth(CCmdUI* pCmdUI) 
{
    if ((m_fRO) || (!m_bAllowStyled))
        pCmdUI->Enable( false );
    else
    {
        pCmdUI->Enable( true );

        style_info                      info, mask;
        pgInitStyleMask(&info,0);
        pgInitStyleMask(&mask,0);
        long id = pgGetNamedStyleIndex(m_paigeRef,teletype_style);

        //
        // Style is consistent throughout selection
        //
        pgGetStyleInfo(m_paigeRef, NULL, FALSE, &info, &mask);
        if ( (abs(info.style_sheet_id) == id) && ( mask.style_sheet_id != 0 ) )
            pCmdUI->SetCheck(1);
        else
            //
            // Style is indeterminate
            // 
            if ( mask.style_sheet_id == 0 )
            {
                pCmdUI->SetCheck(2);
            }
            else
                                //
                                // Style is not present
                                //
                pCmdUI->SetCheck(0);

    }
}


long CPaigeEdtView::GetAttachmentID(long offset)
{
    long linkID = 0;

    pg_hyperlink pgLink;

    if ( pgGetHyperlinkSourceInfo(m_paigeRef, offset, 0, TRUE, &pgLink) )
    {
        if (pgLink.type & HYPERLINK_EUDORA_ATTACHMENT)
        {
            if ( (offset >= pgLink.applied_range.begin) && (offset <= pgLink.applied_range.end) )
                linkID = pgLink.unique_id;
        }
    }

    return linkID;
}

void CPaigeEdtView::OnUpdateOpenAttachment(CCmdUI* pCmdUI)
{
    BOOL enable = FALSE;

    if (m_fRO)
    {
        select_pair sel;
        pgGetSelection(m_paigeRef, &sel.begin, &sel.end);

        if ( GetAttachmentID(sel.begin) )
            enable = TRUE;
    }

    pCmdUI->Enable(enable);
}

void CPaigeEdtView::OnOpenAttachment()
{
    select_pair sel;
    pgGetSelection(m_paigeRef, &sel.begin, &sel.end);

    long attachID;

    if (attachID = GetAttachmentID(sel.begin))
    {
        paige_rec_ptr pgPtr = (paige_rec_ptr) UseMemory(m_paigeRef);

        // simply stash the hyperlink's id for later processing
        PgStuffBucket* pSB = (PgStuffBucket*) pgPtr->user_refcon;
        pSB->hlinkEvent = attachID;
        UnuseMemory(m_paigeRef);
        PgProcessHyperlinks(m_paigeRef);
    }
}
void CPaigeEdtView::OnAddQuote()
{
	if (!m_fRO)
	{
		m_styleEx->ApplyExcerpt(TRUE);
	    GetDocument()->SetModifiedFlag();
	}
}

void CPaigeEdtView::OnUpdateAddQuote(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_fRO);
}

void CPaigeEdtView::OnRemoveQuote()
{
	if (!m_fRO)
	{
		m_styleEx->ApplyExcerpt(FALSE);
	    GetDocument()->SetModifiedFlag();
	}
}

void CPaigeEdtView::OnUpdateRemoveQuote(CCmdUI* pCmdUI)
{
	BOOL enable = FALSE;

	if (!m_fRO)
	{
		select_pair sel;
		pgGetSelection(m_paigeRef, &sel.begin, &sel.end);
		enable = m_styleEx->IsExcerpt(&sel);
	}

	pCmdUI->Enable(enable);
}

int CPaigeEdtView::TabToSpaces()
{
    select_pair         sel, line;
    int                         numSpaces;
    unsigned char       buf[8];

    pgGetSelection(m_paigeRef, &sel.begin, &sel.end);
        
    short tabstop = GetIniShort(IDS_INI_TAB_STOP);

    if ((sel.begin == 0) || (pgCharByte(m_paigeRef, sel.begin - 1, buf) == 1) )
    {
        if (sel.begin == 0 || buf[0] == PgGlobalsPtr()->line_wrap_char)
        {
            numSpaces = tabstop;
            return numSpaces;
        }
    }

    //If Selection is at end of doc, returns bogus values
    if (pgTextSize(m_paigeRef) == sel.begin)
        pgFindLine(m_paigeRef, sel.begin - 1, &line.begin, &line.end);
    else
        pgFindLine(m_paigeRef, sel.begin, &line.begin, &line.end);

    numSpaces = tabstop - ((sel.begin - line.begin) % tabstop);

    return numSpaces;
}



///////////////////////////////////////////////////////////////////////////////
// Spell-checking Dirty List

#define nil NULL

// QueueDirtyPair -- adds a selection pair to the list of text ranges that
// need spell-checked. allocates memory.
void CPaigeEdtView::QueueDirtyPair( long begin, long end )
{
    DirtyPairPtr pRng = new DirtyPair;

    if ( pRng ) {
        pRng->begin = begin;
        pRng->end = end;
        pRng->next = nil;

        if ( m_pDirtyPair ) {
            DirtyPairPtr pTrv = m_pDirtyPair;
            for ( ; pTrv->next; pTrv = pTrv->next )
                ;

            pTrv->next = pRng;
        }
        else
            m_pDirtyPair = pRng;
    }
}


// DequeueDirtyPair -- gets the next text range (FIFO) off the queue, and
// frees the memory associated with the DirtyPair.
bool CPaigeEdtView::DequeueDirtyPair( long* begin, long* end )
{
    bool bRet = false;

    if ( m_pDirtyPair ) {
        DirtyPairPtr pRng = m_pDirtyPair;
        m_pDirtyPair = pRng->next;
        *begin = pRng->begin;
        *end = pRng->end;
        delete pRng;
        bRet = true;
    }
    else {
        *begin = 0;
        *end = 0;
    }

    return bRet;
}

