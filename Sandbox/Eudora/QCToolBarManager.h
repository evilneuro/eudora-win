// QCToolBarManager.h: interface for the QCToolBarManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QCTOOLBARMANAGER_H__A46998C2_ED9C_11D0_9800_00805FD2F268__INCLUDED_)
#define AFX_QCTOOLBARMANAGER_H__A46998C2_ED9C_11D0_9800_00805FD2F268__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class QCToolBarManagerWithBM : public SECToolBarManager  
{
public:
	DECLARE_DYNAMIC( QCToolBarManagerWithBM )

	QCToolBarManagerWithBM( CFrameWnd* pFrame = NULL );
	virtual ~QCToolBarManagerWithBM();

	virtual BOOL AddButtonImage( CBitmap& theBitmap, UINT uID );
	virtual void CopyButtonImage( UINT uCopyButtonIndex, UINT uID );
	virtual void CopyButtonImage( CImageList *pImageList, int imageOffset, UINT uID, BOOL drawEdge = FALSE);
	virtual void DeleteBitmapAt( UINT iIndex );
	virtual void DeleteAllUnusedBitmaps();

protected:
};

class QCToolBarManager : public QCToolBarManagerWithBM  
{
public:
	CFrameWnd* GetFrameWnd() { return m_pFrameWnd; }
	QCToolBarManager( CFrameWnd* pFrame = NULL );

	DECLARE_DYNAMIC( QCToolBarManager )

	virtual ~QCToolBarManager();

	virtual BOOL LoadToolBarResource();
	virtual void LoadState(LPCTSTR lpszProfileName);
	virtual void OnCustomize();
	virtual void SaveState(LPCTSTR lpszProfileName);
	
	virtual SECCustomToolBar* CreateUserToolBar(LPCTSTR lpszTitle = NULL );
	BOOL ConvertOldStuff();
	virtual CControlBar* DynCreateControlBar(SECControlBarInfo* pBarInfo);
};

#endif // !defined(AFX_QCTOOLBARMANAGER_H__A46998C2_ED9C_11D0_9800_00805FD2F268__INCLUDED_)
