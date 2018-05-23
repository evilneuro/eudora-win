// QCCommandObject.cpp: implementation of the QCCommandObject class.
//
//////////////////////////////////////////////////////////////////////

#include "STDAFX.H"

#include "QCCommandDirector.h"
#include "QCCommandObject.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNAMIC(QCCommandObject, CObject)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QCCommandObject::QCCommandObject( 
QCCommandDirector *pDirector ) : m_pDirector ( pDirector )
{
	if( m_pDirector == NULL )
	{
		AfxThrowUserException();
	}
}

QCCommandObject::~QCCommandObject()
{

}


void QCCommandObject::NotifyDirector(
COMMAND_ACTION_TYPE theAction,
void*				pData )
{
	m_pDirector->NotifyClients( this, theAction, pData );
}


UINT QCCommandObject::GetFlyByID(
COMMAND_ACTION_TYPE)
{
	return 0;
}

CString	QCCommandObject::GetToolTip(
COMMAND_ACTION_TYPE)
{
	return "";
}
