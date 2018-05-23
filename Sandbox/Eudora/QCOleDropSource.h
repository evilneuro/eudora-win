#ifndef __QCOLEDROPSOURCE_H__
#define __QCOLEDROPSOURCE_H__

////////////////////////////////////////////////////////////////////////
// File: QCOleDropSource.h
//
// QC customization of the standard MFC COleDropSource.  The only thing
// we do is to customize the drop cursor when the drop target indicates
// via DROPEFFECT_LINK that it can handle an "auto-activate" of a tabbed 
// window.  In this case, we just want to display a normal arrow cursor.
//
////////////////////////////////////////////////////////////////////////

class QCOleDropSource : public COleDropSource
{
public:
	QCOleDropSource(BOOL bWantTrackerGhost = FALSE);
	virtual ~QCOleDropSource();

	virtual SCODE GiveFeedback(DROPEFFECT dropEffect);
	virtual SCODE QueryContinueDrag(BOOL bEscapePressed, DWORD dwKeyState);

protected:
	void DrawGhostRect();
	void EraseGhostRect();

	BOOL m_bWantTrackerGhost;
	CRect m_rectLast;
};

#endif __QCOLEDROPSOURCE_H__
