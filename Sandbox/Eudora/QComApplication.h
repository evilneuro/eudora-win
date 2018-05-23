#ifndef _QComApplication_h_
#define _QComApplication_h_

#include "resource.h"       // main symbols
#include "Eudora.h"
#include "QComApplicationEvents.h"
#include "QComObject.h"

class QCAutomationDirector;
class QComFolders;
class QComApplication;

/////////////////////////////////////////////////////////////////////////////
// QComApplication

class ATL_NO_VTABLE QComApplication :
	public QComObject<QComApplication, IEuApplication, &IID_IEuApplication, &CLSID_EuApplication, &LIBID_EudoraLib>,
	public IConnectionPointContainerImpl<QComApplication>,
	public QComApplicationEvents<QComApplication>
{
	friend QCAutomationDirector;
public:
	QComApplication();
	~QComApplication();
	void OnClose();
	// Class factory functions
	static QComApplication* Create(void);

DECLARE_REGISTRY_RESOURCEID(IDR_COM_APPLICATION)
DECLARE_CLASSFACTORY_SINGLETON(QComApplication)

BEGIN_COM_MAP(QComApplication)
	COM_INTERFACE_ENTRY(IEuApplication)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()

BEGIN_CONNECTION_POINT_MAP(QComApplication)
	CONNECTION_POINT_ENTRY(DIID_IEuApplicationEvents)
END_CONNECTION_POINT_MAP()


// ISupportsErrorInfo interface
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

// IEuApplication interface
public:
	STDMETHOD(get_ProcessID)(/*[out, retval]*/ long *pVal);
	STDMETHOD(get_RefCount)(/*[out, retval]*/ long *pVal);
	STDMETHOD(CloseEudora)();
	STDMETHOD(get_Height)(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_Height)(/*[in]*/ long newVal);
	STDMETHOD(get_Top)(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_Top)(/*[in]*/ long newVal);
	STDMETHOD(get_Width)(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_Width)(/*[in]*/ long newVal);
	STDMETHOD(get_Left)(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_Left)(/*[in]*/ long newVal);
	STDMETHOD(get_StatusBar)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_StatusBar)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_Visible)(/*[out, retval]*/ BOOL *pVal);
	STDMETHOD(put_Visible)(/*[in]*/ BOOL newVal);
	STDMETHOD(get_Interactive)(/*[out, retval]*/ BOOL *pVal);
	STDMETHOD(get_Caption)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Caption)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_Parent)(/*[out, retval]*/ IEuApplication** pVal);
	STDMETHOD(get_Application)(/*[out, retval]*/ IEuApplication** pVal);
	STDMETHOD(get_Folders)(IEuFolders** pVal);
	STDMETHOD(get_RootFolder)(IEuFolder** pVal);
	STDMETHOD(get_Name)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(get_Path)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(get_FullName)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(get_InBox)(IEuFolder** pVal);
	STDMETHOD(Folder)(BSTR Name, BOOL FindRecursive, /*[out, retval]*/ IEuFolder** pVal);
	STDMETHOD(FolderByID)(long ID, BOOL FindRecursive, /*[out, retval]*/ IEuFolder** pVal);
	STDMETHOD(CheckMail)(/*[in, optional]*/ VARIANT Password);
	STDMETHOD(EmptyTrash)();
	STDMETHOD(CompactFolders)();
	STDMETHOD(get_VersionMajor)(/*[out, retval]*/ short *pVal);
	STDMETHOD(get_VersionMinor)(/*[out, retval]*/ short *pVal);
	STDMETHOD(get_VersionMinor2)(/*[out, retval]*/ short *pVal);
	STDMETHOD(get_VersionBuild)(/*[out, retval]*/ short *pVal);
	STDMETHOD(SendQueuedMessages)();
	STDMETHOD(QueueMessage)(BSTR To, BSTR Subject, BSTR Cc, BSTR Bcc, BSTR Attach, BSTR Body);
// global functions
	void RegisterActiveObject(void);
	void UnregisterActiveObject(void);
// utility functions
protected:
	QComFolder* GetRootFolder(void) { return m_pRootFolder; }
	bool GetMainRect(LPRECT pRect);
	bool SetMainRect(LPRECT pRect);
// data
protected:
	BOOL m_bInteractive;
	CString m_strTemp;
	CString m_strPath;
	CString m_strName;
	CString m_strFullName;
	QComFolder* m_pRootFolder;
	unsigned long m_AppActiveObjectID;
};

extern QComApplication* g_pApplication;

#endif // _QComApplication_h_
