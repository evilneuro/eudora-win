// Debugger.cpp
//
// Small app that attaches as a debugger to Eudora
//

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <tchar.h>


const TCHAR Title[] = "Eudora Debugger";


int WINAPI WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPSTR lpCmdLine, int /*nCmdShow*/)
{
	DWORD dwProcessID = strtoul(lpCmdLine, NULL, 10);
	TCHAR Buffer[1024];

	if (!dwProcessID)
	{
		HWND EudoraHwnd = NULL;

		for (int RetryCount = 120; RetryCount > 0; RetryCount--)
		{
			EudoraHwnd = ::FindWindow(_T("EudoraMainWindow"), NULL);
			if (EudoraHwnd)
				break;
			else
				Sleep(500);
		}

		if (!EudoraHwnd)
		{
			MessageBox(NULL, _T("Couldn't find a running instance of Eudora."), Title, MB_ICONSTOP);
			return 0;
		}

		GetWindowThreadProcessId(EudoraHwnd, &dwProcessID);
	}

	if (!DebugActiveProcess(dwProcessID))
	{
		DWORD dwError = GetLastError();
		// When trying to attach a debugger to a process that's already being debugged,
		// you get an ERROR_ACCESS_DENIED.  So let's just silently exit in that case.
		if (dwError != ERROR_ACCESS_DENIED)
		{
			wsprintf(Buffer, _T("Couldn't attach to process %lu to debug."), dwProcessID);
			MessageBox(NULL, Buffer, Title, MB_ICONSTOP);
		}
		return 0;
	}


	DEBUG_EVENT DebugEvent;
	DWORD dwContinueStatus;
	BOOL bContinueDebugging = TRUE;

	while (bContinueDebugging)
	{
		WaitForDebugEvent(&DebugEvent, INFINITE);

		dwContinueStatus = DBG_CONTINUE;

		switch (DebugEvent.dwDebugEventCode)
		{
		case EXCEPTION_DEBUG_EVENT: 
			// Process the exception code. When handling
			// exceptions, remember to set the continuation
			// status parameter (dwContinueStatus). This value
			// is used by the ContinueDebugEvent function.
			switch (DebugEvent.u.Exception.ExceptionRecord.ExceptionCode)
            {
			case EXCEPTION_ACCESS_VIOLATION:
				// First chance: Pass this on to the kernel.
				// Last chance: Display an appropriate error.
				dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
				if (DebugEvent.u.Exception.dwFirstChance == 0)
				{
//char DebugAppFormat[MAX_PATH];
//if (GetProfileString("AeDebug", "Debugger", "", DebugAppFormat, sizeof(DebugAppFormat) - 1) && *DebugAppFormat)
//{
//	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
//	wsprintf(Buffer, DebugAppFormat, DebugEvent.dwProcessId, hEvent);
//	if (WinExec(Buffer, SW_SHOW) > 31 && hEvent)
//	{
//		// Wait for 5 minutes.  Certainly the debugger has started by then.
//		WaitForSingleObject(hEvent, 300000);
//	}
//}

					EXCEPTION_RECORD* pER = &DebugEvent.u.Exception.ExceptionRecord;
					wsprintf(Buffer, _T("Access violation at location %#08lx, attempting to %s virtual memory address %#08lx"),
						pER->ExceptionAddress,
						(pER->NumberParameters > 0 && pER->ExceptionInformation[0])? _T("write to") : _T("read from"),
						(pER->NumberParameters > 1)? pER->ExceptionInformation[1] : 0);
					MessageBox(NULL, Buffer, Title, MB_ICONSTOP | MB_SYSTEMMODAL);
				}
				break;

			case EXCEPTION_BREAKPOINT:
				// First chance: Display the current
				// instruction and register values.
				break;

			case EXCEPTION_DATATYPE_MISALIGNMENT:
				// First chance: Pass this on to the kernel.
				// Last chance: Display an appropriate error.
				break;

			case EXCEPTION_SINGLE_STEP:
				// First chance: Update the display of the
				// current instruction and register values.
				break;

			case DBG_CONTROL_C:
				// First chance: Pass this on to the kernel.
				// Last chance: Display an appropriate error.
				// Handle other exceptions.
				break;
			}
			break;

		case CREATE_THREAD_DEBUG_EVENT:
			// As needed, examine or change the thread's registers
			// with the GetThreadContext and SetThreadContext functions;
			// and suspend and resume thread execution with the
			// SuspendThread and ResumeThread functions.
			break;

		case CREATE_PROCESS_DEBUG_EVENT:
			// As needed, examine or change the registers of the
			// process's initial thread with the GetThreadContext and
			// SetThreadContext functions; read from and write to the
			// process's virtual memory with the ReadProcessMemory and
			// WriteProcessMemory functions; and suspend and resume
			// thread execution with the SuspendThread and ResumeThread
			// functions.
			break;

		case EXIT_THREAD_DEBUG_EVENT:
			// Display the thread's exit code.
			break;

		case EXIT_PROCESS_DEBUG_EVENT:
			// Display the process's exit code.
			bContinueDebugging = FALSE;
			break;

		case LOAD_DLL_DEBUG_EVENT:
			// Read the debugging information included in the newly
			// loaded DLL.
			break;

		case UNLOAD_DLL_DEBUG_EVENT:
			// Display a message that the DLL has been unloaded.
			break;

		case OUTPUT_DEBUG_STRING_EVENT:
			// Display the output debugging string.
			break;
		}	

		// Resume executing the thread that reported the debugging event.  
		if (!ContinueDebugEvent(DebugEvent.dwProcessId, DebugEvent.dwThreadId, dwContinueStatus))
			break;
	}

	return 0;
}
