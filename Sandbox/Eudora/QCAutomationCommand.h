// QCAutomationCommand.h
//

#ifndef _QCAutomationCommand_h_
#define _QCAutomationCommand_h_

#include "QCCommandObject.h"
#include "QCCommandDirector.h"

class QCAutomationDirector;

class QCAutomationCommand : public QCCommandObject  
{
public:
	QCAutomationCommand(QCAutomationDirector* pDirector);
	virtual ~QCAutomationCommand();
	virtual void Execute(COMMAND_ACTION_TYPE theAction, void* pData = NULL);
protected:
	friend class QCAutomationDirector;
public:
	DECLARE_DYNAMIC(QCAutomationCommand)
};

#endif // _QCAutomationCommand_h_
