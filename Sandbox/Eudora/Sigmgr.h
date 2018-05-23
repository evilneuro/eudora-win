
/////////////////////////////////////////////////////////////////////////////
// CSignatureManager

struct SigStruct
{
	CString SigName;
	int ID;
};


class CSignatureManager
{
public:
	CSignatureManager();
	~CSignatureManager();

	int AddSignature(CString sigfilename);		// Add Signature File name to list.
	int DeleteSignature(CString sigfilename);	// Remove Signature File.
	const SigStruct **GetSignatureList();		// References local storage.
	CString GetSignatureDir() { return (m_SignatureDir); }


private:
	struct SigList
	{
		SigList	*next;
		CString SigName;
		int ID;
	};

	CString m_SignatureDir;				// Full path to Signature Directory.
	SigList *m_SignatureList;			// Sorted linked list of Signatures.
	int	m_SignatureCount;				// Number of Signatures.
	SigStruct **m_SignatureArray;		// Reference returned in GetSignatureList
	int m_changed;						// Has an addition or deletion occured since the last get

	void MigrateSignatures( void );

};

/////////////////////////////////////////////////////////////////////////////
// CNewSignature dialog

class CNewSignature : public CDialog
{
// Construction
public:
	CNewSignature(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNewSignature)
	enum { IDD = IDD_TOOL_NEW_SIGNATURE };
	CString	m_NewSigName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewSignature)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNewSignature)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CSignatureManager *SigMgr;

