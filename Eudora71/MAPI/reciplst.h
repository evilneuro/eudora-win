////////////////////////////////////////////////////////////////////////
//
// CRecipientList
//
// A customized derivation of CStringList that provides support for
// tokenizing a comma-separated list of address names into separate
// CString objects.
//
////////////////////////////////////////////////////////////////////////

class CRecipientList : public CStringList
{
public:
	CRecipientList(void);
	~CRecipientList(void);

	BOOL Tokenize(const CString& recipString);
};

