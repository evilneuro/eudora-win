// QCAutomationCommand.cpp
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "eudora.h"
#include "QCAutomationCommand.h"

#include "DebugNewHelpers.h"

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



