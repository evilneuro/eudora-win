// MsgOpts.cpp : implementation file
//

#include "stdafx.h"
#include "eudora.h"
#include "MsgOpts.h"

#include "persona.h"
#include "rs.h"
#include "QCStationeryDirector.h"

extern QCStationeryDirector	g_theStationeryDirector;

#include "DebugNewHelpers.h"

/////////////////////////////////////////////////////////////////////////////
// CMessageOptions dialog


CMessageOptions::CMessageOptions(CWnd* pParent /*=NULL*/)
	: CHelpxDlg(CMessageOptions::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMessageOptions)
	m_Persona = _T("");
	m_Stationery = _T("");
	//}}AFX_DATA_INIT
}


void CMessageOptions::DoDataExchange(CDataExchange* pDX)
{
	CHelpxDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMessageOptions)
	DDX_Control(pDX, IDC_STATIONERY, m_StationeryCombo);
	DDX_Control(pDX, IDC_PERSONA_NAME, m_PersonaCombo);
	DDX_CBString(pDX, IDC_PERSONA_NAME, m_Persona);
	DDX_CBString(pDX, IDC_STATIONERY, m_Stationery);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMessageOptions, CHelpxDlg)
	//{{AFX_MSG_MAP(CMessageOptions)
	ON_CBN_SELCHANGE(IDC_PERSONA_NAME, OnSelchangePersonaName)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMessageOptions message handlers

BOOL CMessageOptions::OnInitDialog() 
{
	CHelpxDlg::OnInitDialog();

	// init m_Stationery based on the Dominant personality
	const char * defaultStat = GetIniString(IDS_INI_STATIONERY);
	if (defaultStat && *defaultStat )
		m_Stationery = defaultStat;

	// fill in combo boxes
	FillInPersonalities();
	FillInStationery();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CMessageOptions::OnSelchangePersonaName() 
{
	// change the stationery if the persona has a default
	CString Persona;
	CRString DomDude( IDS_DOMINANT );
	char szSel[ 256 ];

	int tmp = m_PersonaCombo.GetCurSel();
	m_PersonaCombo.SendMessage(CB_GETLBTEXT, tmp, (LPARAM)szSel ); 
	Persona = szSel;

	m_Stationery.Empty();

	if ( Persona.IsEmpty() || ( Persona == DomDude ) )
	{
		const char * defaultStat = GetIniString(IDS_INI_STATIONERY);
		if ( defaultStat && *defaultStat )
			m_Stationery = defaultStat;
	}
	else
	{
		char defaultStat[ 80 ];
		CRString Key( IDS_INI_PERSONA_STATIONERY );
		g_Personalities.GetProfileString( Persona, Key, "", defaultStat, sizeof( defaultStat ) );
		if ( *defaultStat )
			m_Stationery = defaultStat;
	}

	// set the current selection in the Stationery combo
	int Index = m_StationeryCombo.FindStringExact( -1, m_Stationery );
	if ( Index >= 0 )
		m_StationeryCombo.SetCurSel( Index );
	else
	{
		m_StationeryCombo.SetCurSel( 0 );
		m_Stationery.Empty();
	}
}

// helper methods
void CMessageOptions::FillInPersonalities() 
{
	m_PersonaCombo.ResetContent();

	// add personalities to the list box control
	LPSTR lpPersonalities = g_Personalities.List();
	while ( lpPersonalities[ 0 ] )
	{
		m_PersonaCombo.AddString( lpPersonalities );

		// advance to next personality
		lpPersonalities += strlen( lpPersonalities ) + 1;
	}

	// hilight the default persona
	if ( m_Persona.IsEmpty() )
	{
		m_PersonaCombo.SetCurSel( 0 );
	}
	else
	{
		int Index = m_PersonaCombo.FindStringExact( -1, m_Persona );
		if ( Index >= 0 )
			m_PersonaCombo.SetCurSel( Index );	// select current personality
		else
			ASSERT( FALSE );
	}
}

void CMessageOptions::FillInStationery()
{
	CRString NoDefaultStr( IDS_NO_DEFAULT );

	m_StationeryCombo.ResetContent();
	m_StationeryCombo.AddString( NoDefaultStr );	// always have a blank option

#ifdef OLDSTUFF
	CString listItem;

  	POSITION pos = g_StationeryList.GetHeadPosition();
	while( pos )
	{
		listItem = g_StationeryList.GetNext(pos);
		m_StationeryCombo.AddString( listItem );
	}
#else

	g_theStationeryDirector.FillComboBox( &m_StationeryCombo );

#endif

	// set the current selection
	int Index = m_StationeryCombo.FindStringExact( -1, m_Stationery );
	if ( Index >= 0 )
		m_StationeryCombo.SetCurSel( Index );
	else
	{
		m_StationeryCombo.SetCurSel( 0 );
		m_Stationery.Empty();
	}
}
