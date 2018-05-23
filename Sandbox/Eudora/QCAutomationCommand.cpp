// QCAutomationCommand.cpp
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "eudora.h"
#include "QCAutomationCommand.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(QCAutomationCommand, QCCommandObject)

QCAutomationCommand::QCAutomationCommand(
	QCAutomationDirector* pDirector) : QCCommandObject((QCCommandDirector*)pDirector)
{

}

QCAutomationCommand::~QCAutomationCommand()
{

}

void QCAutomationCommand::Execute(COMMAND_ACTION_TYPE theAction, void* pData)
{

}



