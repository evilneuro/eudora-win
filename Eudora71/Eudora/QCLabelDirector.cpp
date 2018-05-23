// QCLabelDirector.cpp: implementation of the QCLabelDirector class.
//
// --------------------------------------------------------------------------

#include "stdafx.h"
#include "resource.h"
#include "rs.h"
#include <vector>
#include <limits.h>
#include <stdlib.h>
#include "MsgLabel.h"

#include "QCLabelDirector.h"

#include "DebugNewHelpers.h"

// IDS_LABEL_INI_PREFIX = "Label"
// IDS_LABEL_INI_COUNT = "LabelCount"
// IDS_LABEL_INI_SECTION = "Labels"

// --------------------------------------------------------------------------

const unsigned long g_MAX_LABEL_COUNT = SHRT_MAX;

/* static */ QCLabelDirector *QCLabelDirector::m_pTheLabelDirector = NULL;

// --------------------------------------------------------------------------

// [PUBLIC] GetLabelDirector
/* static */ QCLabelDirector *QCLabelDirector::GetLabelDirector()
{
	ASSERT(m_pTheLabelDirector);

	if (!m_pTheLabelDirector)
		VERIFY(Initialize());

	return (m_pTheLabelDirector);
}

// --------------------------------------------------------------------------

// [PUBLIC] Initialize
/* static */ bool QCLabelDirector::Initialize()
{
	TRACE("QCLabelDirector::Initialize()\n");
	ASSERT(!m_pTheLabelDirector);

	if (m_pTheLabelDirector)
		delete m_pTheLabelDirector;

	m_pTheLabelDirector = DEBUG_NEW_NOTHROW QCLabelDirector;

	return (m_pTheLabelDirector != NULL);
}

// --------------------------------------------------------------------------

// [PUBLIC] Destroy
/* static */ bool QCLabelDirector::Destroy()
{
	TRACE("QCLabelDirector::Destroy()\n");

	if (m_pTheLabelDirector)
	{
		delete m_pTheLabelDirector;
		m_pTheLabelDirector = NULL;
	}

	return (true);
}

// --------------------------------------------------------------------------

// [PROTECTED] QCLabelDirector (Default Constructor)
QCLabelDirector::QCLabelDirector()
{
	m_INICountKey = CRString(IDS_LABEL_INI_COUNT); // "LabelCount"
	m_INISection = CRString(IDS_LABEL_INI_SECTION); // "Labels"
	m_INIKeyFormat = CRString(IDS_LABEL_INI_LBLFORMAT); // "Label%u"

	ASSERT(!m_INICountKey.IsEmpty());
	ASSERT(!m_INISection.IsEmpty());
	ASSERT(!m_INIKeyFormat.IsEmpty());

	Read();
}

// --------------------------------------------------------------------------

// [PROTECTED] ~QCLabelDirector (Destructor)
QCLabelDirector::~QCLabelDirector()
{
	VERIFY(Write());
	m_vLabels.erase(m_vLabels.begin(), m_vLabels.end());
}

// --------------------------------------------------------------------------

LPCSTR QCLabelDirector::GetLabelText(unsigned int nIdx) const
{
	ASSERT(nIdx > 0); // Labels are 1-based
	ASSERT(nIdx <= m_vLabels.size());

	if ((nIdx > 0) && (nIdx <= m_vLabels.size()))
		return (m_vLabels[nIdx - 1].GetText());

	return "";
}

// --------------------------------------------------------------------------

COLORREF QCLabelDirector::GetLabelColor(unsigned int nIdx) const
{
	ASSERT(nIdx > 0); // Labels are 1-based
	ASSERT(nIdx <= m_vLabels.size());

	if ((nIdx > 0) && (nIdx <= m_vLabels.size()))
		return (m_vLabels[nIdx - 1].GetColor());

	return (RGB(0,0,0));
}

// --------------------------------------------------------------------------

bool QCLabelDirector::GetLabel(unsigned int nIdx,
							   COLORREF *clr,
							   CString *txt) const
{
	ASSERT(nIdx > 0); // Labels are 1-based

	if (nIdx > m_vLabels.size())
		return (false);

	(*clr) = m_vLabels[nIdx - 1].GetColor();
	(*txt) = m_vLabels[nIdx - 1].GetText();

	return (true);
}

// --------------------------------------------------------------------------

bool QCLabelDirector::GetLabel(unsigned int nIdx, COLORREF *clr) const
{
	ASSERT(nIdx > 0); // Labels are 1-based

	if (nIdx > m_vLabels.size())
		return (false);

	(*clr) = m_vLabels[nIdx - 1].GetColor();

	return (true);
}

// --------------------------------------------------------------------------

bool QCLabelDirector::GetLabel(unsigned int nIdx, CString *txt) const
{
	ASSERT(nIdx > 0); // Labels are 1-based

	if (nIdx > m_vLabels.size())
		return (false);

	(*txt) = m_vLabels[nIdx - 1].GetText();

	return (true);
}

// --------------------------------------------------------------------------

unsigned int QCLabelDirector::GetCount() const
{
	return (m_vLabels.size());
}

// --------------------------------------------------------------------------

bool QCLabelDirector::SetLabel(unsigned int nIdx,
							   const COLORREF& clr,
							   LPCSTR txt)
{
	ASSERT(nIdx > 0); // Labels are 1-based

	if (nIdx > m_vLabels.size())
		return (false);

	m_vLabels[nIdx - 1].SetColor(clr);
	m_vLabels[nIdx - 1].SetText(txt);

	return (true);
}

// --------------------------------------------------------------------------

bool QCLabelDirector::SetLabel(unsigned int nIdx, const COLORREF& clr)
{
	ASSERT(nIdx > 0); // Labels are 1-based

	if (nIdx > m_vLabels.size())
		return (false);

	m_vLabels[nIdx - 1].SetColor(clr);

	return (true);
}

// --------------------------------------------------------------------------

bool QCLabelDirector::SetLabel(unsigned int nIdx, LPCSTR txt)
{
	ASSERT(nIdx > 0); // Labels are 1-based

	if (nIdx > m_vLabels.size())
		return (false);

	m_vLabels[nIdx - 1].SetText(txt);

	return (true);
}

// --------------------------------------------------------------------------

bool QCLabelDirector::SetCount(unsigned int nCount)
{
	if (nCount < m_vLabels.size())
	{
		std::vector<CMsgLabel>::iterator first, last;
		first = m_vLabels.begin() + nCount;
		last = m_vLabels.end();

		m_vLabels.erase(first, last);

	}
	else if (nCount > m_vLabels.size())
	{
		m_vLabels.reserve(nCount);

		while (nCount > m_vLabels.size())
		{
			CMsgLabel lbl;
			if (GetDefaultLabel(m_vLabels.size() + 1, &lbl))
				m_vLabels.push_back(lbl);
			else
				break; // Could not get the label
		}
	}

	ASSERT(nCount == m_vLabels.size());

	return (nCount == m_vLabels.size());
}

// --------------------------------------------------------------------------

CString QCLabelDirector::GetDefaultText(unsigned int nIdx) const
{
	ASSERT(nIdx > 0); // Labels are 1-based

	CString str;
	str.Format("Label %u", nIdx);
	
	return (str);
}

// --------------------------------------------------------------------------

COLORREF QCLabelDirector::GetDefaultColor(unsigned int nIdx) const
{
	ASSERT(nIdx > 0); // Labels are 1-based

	return (RGB(0,0,0));
}

// --------------------------------------------------------------------------

bool QCLabelDirector::Write()
{
	long red, green, blue;
	CString txt, sKey;
	COLORREF clr;
	unsigned int nIdx;

	WriteEudoraProfileULong(m_INISection, m_INICountKey, m_vLabels.size());

	for (nIdx = 1; nIdx <= m_vLabels.size(); nIdx++)
	{
		sKey.Format(m_INIKeyFormat, nIdx);

		clr = m_vLabels[nIdx - 1].GetColor();

		red = (clr & 0xFF);
		green = ((clr >> 8) & 0xFF);
		blue = ((clr >> 16) & 0xFF);
		
		txt.Format("%ld,%ld,%ld,%s", red, green, blue, m_vLabels[nIdx - 1].GetText());

		WriteEudoraProfileString(m_INISection, sKey, txt);

		if (nIdx <= 7)
		{
			RemoveIniKey(IDS_INI_LABEL_COLOR_1 + nIdx - 1);
			RemoveIniKey(IDS_INI_LABEL_TEXT_1 + nIdx - 1);
		}
	}
	

	return (true);
}

// --------------------------------------------------------------------------

bool QCLabelDirector::Read()
{
	m_vLabels.erase(m_vLabels.begin(), m_vLabels.end());

	const unsigned int nCount = ReadLabelCount();

	m_vLabels.reserve(nCount);

	CMsgLabel lbl;
	unsigned int nIdx = 1;
	while (nIdx <= nCount)
	{
		if (!ReadNewINILabel(nIdx, &lbl))
		{
			if (nCount > 7)
				VERIFY(GetDefaultLabel(nIdx, &lbl));
			else
				VERIFY(ReadOldINILabel(nIdx, &lbl));
		}

		m_vLabels.push_back(lbl);
		nIdx++;
	}

	ASSERT(nCount == m_vLabels.size());

	return (nCount == m_vLabels.size());
}

// --------------------------------------------------------------------------

bool QCLabelDirector::ReadNewINILabel(unsigned int nIdx, CMsgLabel *lbl)
{
	CString sKey;
	sKey.Format(m_INIKeyFormat, nIdx);

	CString sValue;
	if (GetEudoraProfileString(m_INISection, sKey, &sValue))
	{
		long red, green, blue;
		LPSTR lpStr = NULL;

		red = strtol((LPCSTR)sValue, &lpStr, 10);

		if ((*lpStr++) != ',')
			return (false);

		green = strtol(lpStr, &lpStr, 10);

		if ((*lpStr++) != ',')
			return (false);
		
		blue = strtol(lpStr, &lpStr, 10);

		if ((*lpStr++) != ',')
			return (false);

		lbl->SetColor(RGB(red,green,blue));
		lbl->SetText(lpStr);

		return (true);
	}

	return (false);
}

// --------------------------------------------------------------------------

bool QCLabelDirector::ReadOldINILabel(unsigned int nIdx, CMsgLabel *lbl)
{
	if ((nIdx < 1) || (nIdx > 7))
		return (false);

	lbl->SetColor(GetIniLong(IDS_INI_LABEL_COLOR_1 + nIdx - 1));
	lbl->SetText(GetIniString(IDS_INI_LABEL_TEXT_1 + nIdx - 1));

	return (true);
}

// --------------------------------------------------------------------------

bool QCLabelDirector::GetDefaultLabel(unsigned int nIdx, CMsgLabel *lbl) const
{
	ASSERT(nIdx > 0); // Labels are 1-based

	lbl->SetColor(GetDefaultColor(nIdx));
	lbl->SetText(GetDefaultText(nIdx));

	return (true);
}

// --------------------------------------------------------------------------

unsigned int QCLabelDirector::ReadLabelCount()
{
	unsigned long nCount = 0;

	// We're just going to ignore the INI entry for Label Count for v4.1
	nCount = 7; 

	/*

	Sometime later we will have variable labels, use code like this...

	if (!GetEudoraProfileULong(m_INISection, m_INICountKey, &nCount))
	{
		nCount = 7; // Default to seven if no count value
	}
	else
	{
		if (nCount < 1)
		{
			nCount = 1;
		}
		else if (nCount > g_MAX_LABEL_COUNT)
		{
			nCount = g_MAX_LABEL_COUNT;
		}
	}
	*/

	return (static_cast<unsigned int>(nCount));
}
