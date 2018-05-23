// EudoraMsgs.h: Eudora-specifc messages

#ifndef _EUDORAMSGS_H_
#define _EUDORAMSGS_H_

// --------------------------------------------------------------------------
// WM_USER: 0x0400 ... 0x7FFF
//
// Message numbers in this range (WM_USER through 0x7FFF) can
// be defined and used by an application to send messages within
// a private window class. These values cannot be used to define
// messages that are meaningful throughout an application, because
// some predefined window classes already define values in this
// range. For example, predefined control classes such as BUTTON,
// EDIT, LISTBOX, and COMBOBOX may use these values. Messages in
// this range should not be sent to other applications unless the
// applications have been designed to exchange messages and to
// attach the same meaning to the message numbers. 

// We should probably avoid using WM_USER msgs.


// --------------------------------------------------------------------------
// WM_APP: 0x8000 ... 0xBFFF
//
// Message numbers in this range (WM_APP through 0xBFFF) are
// available for application to use as private messages. Message
// in this range do not conflict with system messages. 

#define msgTaskViewInfoAlive       (WM_APP + 0)
#define msgTaskViewInfoChanged     (WM_APP + 1)
#define msgTaskViewInfoDead        (WM_APP + 2)
#define msgTaskViewUpdateCols      (WM_APP + 3)

#define msgErrorViewNewError       (WM_APP + 4)
#define msgErrorViewShowErrDlg     (WM_APP + 5)
#define msgErrorViewDelError       (WM_APP + 6)
#define msgErrorViewRemoveAll      (WM_APP + 7)

#define msgStatusBarError          (WM_APP + 8) // CErrorStatus *pErrorStatus = wParam
#define msgStatusBarTask           (WM_APP + 9) // CTaskStatus *pTaskStatus = wParam

#define msgPersViewAdvNew          (WM_APP + 10)
#define msgPersViewEditDom         (WM_APP + 11)

#define msgFindMsgMaiboxSel        (WM_APP + 12) // Tell Find Messages window to select a given mailbox

#define msgConnectionWasMade	   (WM_APP + 13) // Tell mainframe that a connection was made

#define msgFindMsgReloadCriteria   (WM_APP + 14) // Tell Find Messages window to reload its criteria
#define msgFindMsgParentFolderSel  (WM_APP + 15) // Tell Find Messages window to select the parent folder of a given mailbox
#define msgFindMsgAllMailboxesSel  (WM_APP + 16) // Tell Find Messages window to select all mailboxes

#endif // _EUDORAMSGS_H_
