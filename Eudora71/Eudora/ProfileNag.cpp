// ProfileNag.cpp -- simple, unscheduled nag dialog
//
// impliments a modeless, on-top "toolwindow" type dialog box, and a simple
// global-scope routine for invoking it.

// the usual suspects
#include "stdafx.h"
#include "resource.h"
#include "guiutils.h"

// shareware/adware support
#include "QCSharewareManager.h"

// our own sorry butt
#include "ProfileNag.h"

#include "DebugNewHelpers.h"


// dialog with heading in bold font [just one per, please]
class PooterDialog : public CDialog
{
public:
	PooterDialog() {}
	PooterDialog( UINT nIDTemplate, CWnd* pParentWnd )
		: CDialog( nIDTemplate, pParentWnd ) {}

	~PooterDialog() {}

protected:
	virtual BOOL OnInitDialog() {
		// base first (always)
		CDialog::OnInitDialog();

		// make the heading font bold
		CFont font, *pCurFont;
		LOGFONT lf;
		CWnd* pWnd = GetDlgItem( IDC_HEADING );

		if ( pWnd ) {
			pCurFont = pWnd->GetFont();

			if ( pCurFont ) {
				pCurFont->GetLogFont( &lf );
				lf.lfWeight = FW_BOLD;
				font.CreateFontIndirect( &lf );
				pWnd->SetFont( &font );
			}
		}

		return TRUE;
	}
};


// dialog for generating irate emails from customers
class ProfileNagDialog : public PooterDialog
{
	int m_level;      // what kind of nag are we?
	CString m_text;   // say it loud!

public:
	ProfileNagDialog( int level, LPCTSTR pText )
		{ m_level = level; m_text = pText; }

protected:
	BOOL OnInitDialog() {
		// base first (always)
		PooterDialog::OnInitDialog();

		// set the optional message text
		CWnd* pWnd = GetDlgItem( IDC_NAGTEXT );

		if ( pWnd /*&& m_text.GetLength()*/ )
			pWnd->SetWindowText( m_text );

		// if we're a level 1 nag, load the "features" text
		if ( m_level == 1 ) {
			CString features;
			features.LoadString( IDS_NAG_FULLFEATURED_TEXT );
			pWnd = GetDlgItem( IDC_MISSINGFEATURESTEXT );

			if ( pWnd )
				pWnd->SetWindowText( features );
		}

		// default to the "Profile" button
		GotoDlgCtrl( GetDlgItem(IDOK) );
		return FALSE;
	}

	// user wants to profile (sucker!)
	void OnOK() {
		PooterDialog::OnOK();
		LaunchURLWithQuery( NULL, ACTION_PROFILE );

		// Output the result of what the user chose in response to the nag dialog
		char		szDismissed[64];
		wsprintf(szDismissed, CRString(IDS_DEBUG_DISMISSED_ALERT), IDOK);
		PutDebugLog(DEBUG_MASK_DIALOG, szDismissed);
	}

	// because i'm lazy, and don't want mess around with message maps
	LRESULT WindowProc( UINT message, WPARAM wParam, LPARAM lParam ) {
		if ( message == WM_COMMAND && HIWORD(wParam) == BN_CLICKED ) {

			// user wants to know more about fabulous world of profiling
			if ( LOWORD(wParam) == IDC_ABOUT ) {
				LaunchURLWithQuery( NULL, ACTION_PROFILE_FAQ );

				// Output the result of what the user chose in response to the nag dialog
				char		szDismissed[64];
				wsprintf(szDismissed, CRString(IDS_DEBUG_DISMISSED_ALERT), IDC_ABOUT);
				PutDebugLog(DEBUG_MASK_DIALOG, szDismissed);

				return 0;
			}
		}

		return PooterDialog::WindowProc( message, wParam, lParam );
	}
};

// spank the user, make 'em profile. we've tweaked this around a little to
// handle the switch-from-deadbeat-to-adware case, which is kind of ugly, but
// saves us having to mess around with the rest of the code.
class DeadbeatNagDialog : public PooterDialog
{
	bool m_bCancelOK;
	CString m_szNagDialogText;

public:
	DeadbeatNagDialog( UINT nIDTemplate, CWnd* pParentWnd )
		: PooterDialog( nIDTemplate, pParentWnd )
			{ m_bCancelOK = (nIDTemplate == IDD_NAG_DEADBEAT2ADWARE); }

	const CString & GetDialogText() const { return m_szNagDialogText; }

protected:

	virtual BOOL OnInitDialog() {
		BOOL	bResult = PooterDialog::OnInitDialog();

		// Get the nag dialog wording
		CString		szNagDialogText;
		CWnd *		pNagText = GetDlgItem(IDC_NAGTEXT);

		if (pNagText)
			pNagText->GetWindowText(m_szNagDialogText);

		return bResult;
	}

	void OnCancel() {
		if ( !m_bCancelOK )     // user is becoming a deadbeat; cannot cancel
			MessageBeep( MB_ICONEXCLAMATION );
		else
			EndDialog( IDOK );  // user chooses to remain a deadbeat
	}

	// because i'm lazy, and don't want mess around with message maps
	LRESULT WindowProc( UINT message, WPARAM wParam, LPARAM lParam ) {
		if ( message == WM_COMMAND && HIWORD(wParam) == BN_CLICKED ) {

			// user wants to profile
			if ( LOWORD(wParam) == IDC_PROFILE ) {
				EndDialog( IDCANCEL );
				return 0;
			}
		}

		return PooterDialog::WindowProc( message, wParam, lParam );
	}
};

// wait for the user to profile
class DeadbeatWaitDialog : public PooterDialog
{
public:
	DeadbeatWaitDialog( UINT nIDTemplate, CWnd* pParentWnd )
		: PooterDialog( nIDTemplate, pParentWnd ) {}

	// because i'm lazy, and don't want mess around with message maps
	LRESULT WindowProc( UINT message, WPARAM wParam, LPARAM lParam ) {
		if ( message == WM_SHOWWINDOW && wParam == TRUE )
			LaunchURLWithQuery( NULL, ACTION_PROFILE );

		return PooterDialog::WindowProc( message, wParam, lParam );
	}
};

// api for invoking profile nag
void DoProfileNag( CWnd* pParent, int level, LPCTSTR pText /*= NULL*/ )
{
// Nagging, be gone!
//
//	ProfileNagDialog* pDlg = DEBUG_NEW_MFCOBJ_NOTHROW ProfileNagDialog( level, pText );
//
//	if ( pDlg ) {
//		int dlgTemplate = 0;
//
//		if ( level == 0 )
//			dlgTemplate = IDD_NAG_LEVEL_0;
//		else if ( level == 1 )
//			dlgTemplate = IDD_NAG_LEVEL_1;
//		else if ( level == 2 )
//			dlgTemplate = IDD_NAG_LEVEL_2;
//		else
//			ASSERT(0);   // caller needs a clue
//
//		// Format the nag dialog log entry
//		CString		szNagDialogLogEntry;
//		szNagDialogLogEntry.Format( IDS_NAG_DIALOG_LOG_ENTRY, dlgTemplate, pText ? pText : "" );
//
//		// Output the nag dialog log entry
//		PutDebugLog( DEBUG_MASK_DIALOG, szNagDialogLogEntry, szNagDialogLogEntry.GetLength() );
//
//		pDlg->Create( dlgTemplate, pParent );
//		pDlg->ShowWindow( SW_SHOWNORMAL );
//	}
}

// put "the arm" on the user, make 'em profile---a return value of "true" means
// that they at least went through the motions. doing an immediate playlist
// fetch will catch the cheaters.
bool DoDeadbeatNag( CWnd* pParent /*=NULL*/, bool bUpgradeNag /*=false*/ )
{
	return true;
// Nagging, be gone!
//
//	bool bUserProfiled = false;
//
//	// our first dialog is different if the user is curently a deadbeat, and
//	// is using Payment&Registration to switch into adware mode.
//	int dlgTemplate = bUpgradeNag ? IDD_NAG_DEADBEAT2ADWARE : IDD_NAG_DEADBEAT2;
//
//	// tell the user they are a Deadbeat Loser
//	DeadbeatNagDialog nagDlg( dlgTemplate, pParent );
//
//	int ret = nagDlg.DoModal();
//
//	// Format the nag dialog log entry
//	CString		szNagDialogLogEntry;
//	szNagDialogLogEntry.Format( IDS_NAG_DIALOG_LOG_ENTRY, dlgTemplate, nagDlg.GetDialogText() );
//
//	// Output the nag dialog log entry
//	PutDebugLog( DEBUG_MASK_DIALOG, szNagDialogLogEntry, szNagDialogLogEntry.GetLength() );
//
//	// Output the result of what the user chose in response to the nag dialog
//	char		szDismissed[64];
//	wsprintf(szDismissed, CRString(IDS_DEBUG_DISMISSED_ALERT), ret);
//	PutDebugLog(DEBUG_MASK_DIALOG, szDismissed);
//
//	// if the user clicks OK we're done, otherwise profile. note that if the
//	// user is switching to adware, the cancel button maps to IDOK.
//	if ( ret != IDOK ) {
//		// put up "i'm waiting" dialog, which does the jump
//		DeadbeatWaitDialog waitDlg( IDD_NAG_WAITING, pParent );
//
//		ret = waitDlg.DoModal();
//
//		// if the user clicks "OK," they're claiming to have profiled
//		if ( ret == IDOK )
//			bUserProfiled = true;
//	}
//
//	return bUserProfiled;
}

