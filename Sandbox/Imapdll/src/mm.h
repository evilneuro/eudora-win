/////////////////////////////////////////////////////////////////////////////
// 	File: MM.H 
//
//  Purpose:  This file provides the declarations for the MAILGETS
//				and mm*() routines for c-client.
//
//	Author:	   J. King
//	Created:   10/23/96
//  Modified:
//
//
/////////////////////////////////////////////////////////////////////////////

#ifndef _MM_H_
#define _MM_H_


// Forward def.
class	CWriter;
class   CLister;
class	CIMLogin;
class	CProtocol;

//	=====================================================
class CMailGets
{

public:
	CMailGets();
	CMailGets(CProtocol* m_pStream, CWriter *Writer);
	~CMailGets();
    
// Implementation
public:
	void		SetMailGets		(void);
	void		ResetMailGets	(void);	
	BOOL		DoMailgets (readfn_t readfn, void *read_data, CProtocol *pStream, unsigned long size);


private:
	mailgets_t		m_oldMailGets;		// Save original mm_gets in this.
	mailgets_t		m_newMailGets;		// Our mm_gets.
	char*			m_buffer;
	CWriter*		m_pWriter;
	CProtocol*		m_pStream;
};



//===================== declarations of the CMboxGets class =============/

class CMboxGets
{
public:
	CMboxGets(CProtocol *pStream, CLister *Lister);	
	~CMboxGets();

	BOOL DoMboxgets (char *mailbox, int delimiter, long attributes);

private:
	CProtocol*	m_pStream;
	CLister		*m_pLister;


};


//===================== declarations of the CLoginGets class =============/

class CLoginGets
{
public:
	CLoginGets(CProtocol *pStream, CIMLogin *Login);	
	~CLoginGets();

	BOOL DoLogingets (NETMBX *mb, char *user, char *pwd, long trial);

private:
	CProtocol*	m_pStream;
	CIMLogin	*m_pLogin;
};



// ============================== EXPORTED =======================//

void mm_set_debug_log(void (*fn) (char *, long));
void mm_log (char *string,long errflg);

#endif // _MM_H_
