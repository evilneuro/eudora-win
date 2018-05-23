/*
 *  Filename: SetngDlg.h
 *
 *  Author: Jeff Beckley
 *
 *  Copyright 1995-2005 QUALCOMM Incorporated
 Copyright (c) 2016, Computer History Museum 
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
DAMAGE. 


 *
 *  Send comments and questions to <emsapi-info@qualcomm.com>
 */

#include "CAPICOM.tlh"

/////////////////////////////////////////////////////////////////////////////
// CSettingsDlg dialog

//! Dialog for allowing user to modify plug-in settings
class CSettingsDlg : public CDialog
{
// Construction
public:
	CSettingsDlg();

// Dialog Data
	//{{AFX_DATA(CSettingsDlg)
	enum { IDD = IDD_SETTINGS_DLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSettingsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSettingsDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	//! Constants for specifying where errors are displayed in the UI
	enum {
		ED_DIALOG,    //!< Only in a dialog
		ED_MESSAGE,   //!< Only in the the message
		ED_BOTH       //!< In both a dialog and the message
	};
};



//! Name of the section in the INI file where settings are stored
#define kINISection					"S/MIME Plugin"
//! Name of the setting to control where errors are displayed in the UI
#define kINIErrorDisplayName		"ErrorDisplay"
//! Name of the setting to control whether signed messages are automatically verified when the message is downloaded
#define kINIVerifyOnDownloadName	"VerifyOnDownload"
//! Name of the setting to control whether certificates of signed messages not found in the OS certificate store are automatically added
#define kINIAddNewCertsName			"AddNewCerts"
//! Name of the setting to choose which encryption algorithm is used
#define kINIEncryptAlgorithmName	"EncryptAlgorithm"
//! Name of the setting to choose which key length to use for encryption
#define kINIEncryptKeyLengthName	"EncryptKeyLength"
//! Name of the setting to control whether the pre-standard "x-" MIME types are used for outgoing messages
#define kINIUseOldSigTypeName		"UseOldSigType"


//! Class for maintaining the settings for the plug-in
class Settings
{
public:
	//! Constructor that sets default values of settings
	Settings()
	{	
		m_ErrorDisplay = CSettingsDlg::ED_DIALOG;
		m_VerifyOnDownload = false;
		m_AddNewCerts = false;
		m_EncryptAlgorithm = CAPICOM::CAPICOM_ENCRYPTION_ALGORITHM_3DES;
		m_EncryptKeyLength = CAPICOM::CAPICOM_ENCRYPTION_KEY_LENGTH_MAXIMUM;
		m_UseOldSigType = false;
	}

	//! Called when the plug-in first loads to get the settings from the INI file in to memory
	void Load()
	{
		m_pWinApp = AfxGetApp();
		if (!m_pWinApp)
		{
			// If we don't get the App instance from Eudora (which most likely occurs because this plugin and
			// Eudora aren't using the same version of MFC), then let's get the App instance of the plugin.
			// This will cause the settings to be read from/written to from an SMIME.INI file in the root
			// of the Windows directory.
			AFX_MANAGE_STATE(AfxGetStaticModuleState());
			m_pWinApp = AfxGetApp();
		}
		if (m_pWinApp)
		{
			m_ErrorDisplay =     m_pWinApp->GetProfileInt(kINISection, kINIErrorDisplayName,     m_ErrorDisplay);
			m_VerifyOnDownload = m_pWinApp->GetProfileInt(kINISection, kINIVerifyOnDownloadName, m_VerifyOnDownload);
			m_AddNewCerts =      m_pWinApp->GetProfileInt(kINISection, kINIAddNewCertsName,      m_AddNewCerts);
			m_EncryptAlgorithm = m_pWinApp->GetProfileInt(kINISection, kINIEncryptAlgorithmName, m_EncryptAlgorithm);
			m_EncryptKeyLength = m_pWinApp->GetProfileInt(kINISection, kINIEncryptKeyLengthName, m_EncryptKeyLength);
			m_UseOldSigType =    m_pWinApp->GetProfileInt(kINISection, kINIUseOldSigTypeName,    m_UseOldSigType);
		}
	}

	//! Called when the user hits the OK button in the settings dialog to write the settings out to the INI file
	void Save()
	{
		if (m_pWinApp)
		{
			m_pWinApp->WriteProfileInt(kINISection, kINIErrorDisplayName,     m_ErrorDisplay);
			m_pWinApp->WriteProfileInt(kINISection, kINIVerifyOnDownloadName, m_VerifyOnDownload);
			m_pWinApp->WriteProfileInt(kINISection, kINIAddNewCertsName,      m_AddNewCerts);
			m_pWinApp->WriteProfileInt(kINISection, kINIEncryptAlgorithmName, m_EncryptAlgorithm);
			m_pWinApp->WriteProfileInt(kINISection, kINIEncryptKeyLengthName, m_EncryptKeyLength);
			m_pWinApp->WriteProfileInt(kINISection, kINIUseOldSigTypeName,    m_UseOldSigType);
		}
	}

	int	m_ErrorDisplay;       //!< Setting to control where errors are displayed in the UI
	int	m_VerifyOnDownload;   //!< Setting to control whether signed messages are automatically verified when the message is downloaded
	int	m_AddNewCerts;        //!< Setting to control whether certificates of signed messages not found in the OS certificate store are automatically added
	int m_EncryptAlgorithm;   //!< Setting to choose which encryption algorithm is used
	int m_EncryptKeyLength;   //!< Setting to choose which key length to use for encryption
	int m_UseOldSigType;      //!< Setting to control whether the pre-standard "x-" MIME types are used for outgoing messages

private:
	CWinApp* m_pWinApp;       //!< Pointer to Eudora's CWinApp, used to load/save settings in Eudora's INI file
};

//! The one and only instance of a settings structure
extern Settings g_Settings;
