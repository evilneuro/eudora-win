// QComFolder.h : Declaration of the QComFolder

#ifndef _QComFolder_h_
#define _QComFolder_h_

#include "EudoraExe.h"	// interface declarations
#include "QComObject.h"
#include "resource.h"   // main symbols

class QCAutomationDirector;
class QCMailboxCommand;
class QComFolders;
class QComMessage;
class QComMessages;

/////////////////////////////////////////////////////////////////////////////
// QComFolder
class ATL_NO_VTABLE QComFolder :
	public CObject, 
	public QComObject<QComFolder, IEuFolder, &IID_IEuFolder, &CLSID_EuFolder, &LIBID_EudoraLib>
{
	friend class QComFolders;
	friend class QComMessage;
	friend class QComMessages;
	friend class QCAutomationDirector;
public:
	QComFolder();
	~QComFolder();
	// Class factory functions
	static QComFolder* Create(void);
	// Internal app only functions
	LPCSTR GetFullName(void) { return m_FullName; }
	QComFolders* GetFolders(void);
	QComMessages* GetMessages(void);
	short GetLevel(void) { return m_Level; }
	QCMailboxCommand* GetMailboxCommand(void);
	LPCSTR GetName(void) { return m_Name; }
	QComFolder* GetParent(void) { return m_pParent; }
	bool Init(QComFolder* pParent, QCMailboxCommand* pCommand, short Level);
	bool Init(QComFolder* pParent, LPCSTR Name, LPCSTR Path, LPCSTR FullName, bool bIsMailbox, bool bIsImap, short Level);
	void SetLevel(short Level) { m_Level = Level; }
	void SetName(LPCSTR Name) { m_Name = Name; }
	void SetPath(LPCSTR Path) { m_Path = Path; }
	void SetFullName(LPCSTR FullName) { m_FullName = FullName; }
	void SetParent(QComFolder* pParent) { m_pParent = pParent; }
	QComFolder* InsertNewChild(QCMailboxCommand* pc);
	void SetIndex(long Index) {  m_Index = Index; }
	void Unlink(void);

	DECLARE_REGISTRY_RESOURCEID(IDR_COM_FOLDER)

BEGIN_COM_MAP(QComFolder)
	COM_INTERFACE_ENTRY(IEuFolder)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

	// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

	// IEuFolder
public:
	STDMETHOD(get_ID)(/*[out, retval]*/ long *pVal);
	STDMETHOD(get_bContainsUnreadMessages)(/*[out, retval]*/ BOOL *pVal);
	STDMETHOD(get_bIsImapFolder)(/*[out, retval]*/ BOOL *pVal);
	STDMETHOD(get_bCanContainSubFolders)(/*[out, retval]*/ BOOL *pVal);
	STDMETHOD(get_bCanContainMessages)(/*[out, retval]*/ BOOL *pVal);
	STDMETHOD(Move)(IEuFolder* NewParentFolder);
	STDMETHOD(Open)();
	STDMETHOD(get_Index)(/*[out, retval]*/ long *pVal);
	STDMETHOD(get_RefCount)(/*[out, retval]*/ long *pVal);
	STDMETHOD(get_Level)(/*[out, retval]*/ short *pVal);
	STDMETHOD(get_FullName)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(get_Path)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(get_Name)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(get_Folders)(/*[out, retval]*/ IEuFolders** pVal);
	STDMETHOD(get_Messages)(/*[out, retval]*/ IEuMessages** pVal);
protected:
	long m_ID;
	long m_Index;
	short m_Level;
	bool m_bIsImap;
	bool m_bIsMailbox;
	CString m_Name;
	CString m_Path;
	CString m_FullName;
	QComFolder* m_pParent;	
	QComFolders* m_pFolders;
	QComMessages* m_pMessages;
	QCMailboxCommand *m_pCommand;
};

#endif // _QComFolder_h_
