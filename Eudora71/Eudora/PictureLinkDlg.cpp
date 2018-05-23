// PictureLinkDlg.cpp : implementation file
//

#include "stdafx.h"

#include "resource.h"
#include "PictureLinkDlg.h"
#include "Text2Html.h"

#include "DebugNewHelpers.h"

#define MAX_URL_LEN 1030

/////////////////////////////////////////////////////////////////////////////
// CPictureLinkDlg dialog


CPictureLinkDlg::CPictureLinkDlg(LPCTSTR szURL /*= NULL*/, CWnd* pParent /*= NULL*/)
	: CDialog(CPictureLinkDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPictureLinkDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	if (!szURL || !*szURL)
		m_szURL = "http://";
	else
		m_szURL = szURL;
}


void CPictureLinkDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPictureLinkDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Text(pDX, IDC_INSERT_PICTURE_LINK, m_szURL);
	DDX_Text(pDX, IDC_INSERT_PICTURE_ALT_TEXT, m_szAltText);
	DDX_Text(pDX, IDC_INSERT_PICTURE_CLICK_LINK, m_szClickLink);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPictureLinkDlg, CDialog)
	//{{AFX_MSG_MAP(CPictureLinkDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPictureLinkDlg message handlers

static void AddURLScheme(CString& url)
{
	if (url.IsEmpty() == FALSE)
	{
		char buffer[MAX_URL_LEN];

		if (strnicmp(url, "www.", 4) == 0 || !strchr(url, ':'))
		{
			strcpy(buffer, "http://");
			strncat(buffer, url, sizeof(buffer) - 8);
			url = buffer;
		}
		else if (strnicmp(url, "ftp.", 4) == 0)
		{
			strcpy(buffer, "ftp://");
			strncat(buffer, url, sizeof(buffer) - 7);
			url = buffer;
		}
	}
}

void CPictureLinkDlg::OnOK()
{
	CDialog::OnOK();

	// Have to do these after the base class OnOK(), as OnOK() will
	// cause DoDataExchange() to be called and reset the member
	// variables to the contents of the edit controls
	AddURLScheme(m_szURL);
	AddURLScheme(m_szClickLink);
}

CString CPictureLinkDlg::GetHtmlCode()
{
	CString html;
	CString image;

	if (m_szAltText.IsEmpty())
		image.Format("<img src=\"%s\">", (LPCTSTR)m_szURL);
	else
	{
		CString HtmlizedAltText(Text2Html(m_szAltText, FALSE, FALSE));

		image.Format("<img src=\"%s\" alt=\"%s\">", (LPCTSTR)m_szURL, (LPCTSTR)HtmlizedAltText);
	}

	if (m_szClickLink.IsEmpty())
		html = image;
	else
		html.Format("<a href=\"%s\">%s</a>", (LPCTSTR)m_szClickLink, (LPCTSTR)image);

	return html;
}
