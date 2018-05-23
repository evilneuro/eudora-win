////////////////////////////////////////////////////////////////////////
// CMCFUNC.CPP
//
// CMC entry points for 16-bit/32-bit Eudora MAPI DLL.
////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
//#include <afxwin.h>		// FORNOW, might be better to use precompiled AFX headers

#include "xcmc.h"

#ifdef _DEBUG
#define OUTPUTDEBUGSTRING(_dbg_Msg_) OutputDebugString(_dbg_Msg_)
#else
#define OUTPUTDEBUGSTRING(_dbg_Msg_)
#endif // _DEBUG


////////////////////////////////////////////////////////////////////////
// cmc_send
//
////////////////////////////////////////////////////////////////////////
extern "C" CMC_return_code EXPORT FAR PASCAL
cmc_send(
	CMC_session_id          session,
	CMC_message FAR         *message,
	CMC_flags               send_flags,
	CMC_ui_id               ui_id,
	CMC_extension FAR       *send_extensions)
{
DBG_UNREFERENCED_PARAMETER(session);
DBG_UNREFERENCED_PARAMETER(message);
DBG_UNREFERENCED_PARAMETER(send_flags);
DBG_UNREFERENCED_PARAMETER(ui_id);
DBG_UNREFERENCED_PARAMETER(send_extensions);

	OUTPUTDEBUGSTRING("cmc_send\n");

	return CMC_E_FAILURE;	// declare general failure
}

////////////////////////////////////////////////////////////////////////
// cmc_send_documents
//
////////////////////////////////////////////////////////////////////////
extern "C" CMC_return_code EXPORT FAR PASCAL
cmc_send_documents(
	CMC_string              recipient_addresses,
	CMC_string              subject,
	CMC_string              text_note,
	CMC_flags               send_doc_flags,
	CMC_string              file_paths,
	CMC_string              file_names,
	CMC_string              delimiter,
	CMC_ui_id               ui_id)
{
DBG_UNREFERENCED_PARAMETER(recipient_addresses);
DBG_UNREFERENCED_PARAMETER(subject);
DBG_UNREFERENCED_PARAMETER(text_note);
DBG_UNREFERENCED_PARAMETER(send_doc_flags);
DBG_UNREFERENCED_PARAMETER(file_paths);
DBG_UNREFERENCED_PARAMETER(file_names);
DBG_UNREFERENCED_PARAMETER(delimiter);
DBG_UNREFERENCED_PARAMETER(ui_id);

	OUTPUTDEBUGSTRING("cmc_send_documents\n");

	return CMC_E_FAILURE;	// declare general failure
}

////////////////////////////////////////////////////////////////////////
// cmc_act_on
//
////////////////////////////////////////////////////////////////////////
extern "C" CMC_return_code EXPORT FAR PASCAL
cmc_act_on(
	CMC_session_id          session,
	CMC_message_reference FAR   *message_reference,
	CMC_enum                operation,
	CMC_flags               act_on_flags,
	CMC_ui_id               ui_id,
	CMC_extension FAR       *act_on_extensions)
{
DBG_UNREFERENCED_PARAMETER(session);
DBG_UNREFERENCED_PARAMETER(message_reference);
DBG_UNREFERENCED_PARAMETER(operation);
DBG_UNREFERENCED_PARAMETER(act_on_flags);
DBG_UNREFERENCED_PARAMETER(ui_id);
DBG_UNREFERENCED_PARAMETER(act_on_extensions);

	OUTPUTDEBUGSTRING("cmc_act_on\n");

	return CMC_E_UNSUPPORTED_ACTION;	// tell caller to go away
}

////////////////////////////////////////////////////////////////////////
// cmc_list
//
////////////////////////////////////////////////////////////////////////
extern "C" CMC_return_code EXPORT FAR PASCAL
cmc_list(
	CMC_session_id          session,
	CMC_string              message_type,
	CMC_flags               list_flags,
	CMC_message_reference FAR   *seed,
	CMC_uint32 FAR          *count,
	CMC_ui_id               ui_id,
	CMC_message_summary FAR * FAR *result,
	CMC_extension FAR       *list_extensions)
{
DBG_UNREFERENCED_PARAMETER(session);
DBG_UNREFERENCED_PARAMETER(message_type);
DBG_UNREFERENCED_PARAMETER(list_flags);
DBG_UNREFERENCED_PARAMETER(seed);
DBG_UNREFERENCED_PARAMETER(count);
DBG_UNREFERENCED_PARAMETER(ui_id);
DBG_UNREFERENCED_PARAMETER(result);
DBG_UNREFERENCED_PARAMETER(list_extensions);

	OUTPUTDEBUGSTRING("cmc_list\n");

	return CMC_E_UNSUPPORTED_FLAG;	// tell caller to go away
}

////////////////////////////////////////////////////////////////////////
// cmc_read
//
////////////////////////////////////////////////////////////////////////
extern "C" CMC_return_code EXPORT FAR PASCAL
cmc_read(
	CMC_session_id          session,
	CMC_message_reference FAR   *message_reference,
	CMC_flags               read_flags,
	CMC_message FAR * FAR   *message,
	CMC_ui_id               ui_id,
	CMC_extension FAR       *read_extensions)
{
DBG_UNREFERENCED_PARAMETER(session);
DBG_UNREFERENCED_PARAMETER(message_reference);
DBG_UNREFERENCED_PARAMETER(read_flags);
DBG_UNREFERENCED_PARAMETER(message);
DBG_UNREFERENCED_PARAMETER(ui_id);
DBG_UNREFERENCED_PARAMETER(read_extensions);

	OUTPUTDEBUGSTRING("cmc_read\n");

	return CMC_E_UNSUPPORTED_FLAG;	// tell caller to go away
}

////////////////////////////////////////////////////////////////////////
// cmc_look_up
//
////////////////////////////////////////////////////////////////////////
extern "C" CMC_return_code EXPORT FAR PASCAL
cmc_look_up(
	CMC_session_id          session,
	CMC_recipient FAR       *recipient_in,
	CMC_flags               look_up_flags,
	CMC_ui_id               ui_id,
	CMC_uint32 FAR          *count,
	CMC_recipient FAR * FAR *recipient_out,
	CMC_extension FAR       *look_up_extensions)
{
DBG_UNREFERENCED_PARAMETER(session);
DBG_UNREFERENCED_PARAMETER(recipient_in);
DBG_UNREFERENCED_PARAMETER(look_up_flags);
DBG_UNREFERENCED_PARAMETER(ui_id);
DBG_UNREFERENCED_PARAMETER(count);
DBG_UNREFERENCED_PARAMETER(recipient_out);
DBG_UNREFERENCED_PARAMETER(look_up_extensions);

	OUTPUTDEBUGSTRING("cmc_look_up\n");

	return CMC_E_NOT_SUPPORTED;	// tell caller to go away
}

////////////////////////////////////////////////////////////////////////
// cmc_free
//
////////////////////////////////////////////////////////////////////////
extern "C" CMC_return_code EXPORT FAR PASCAL
cmc_free(CMC_buffer memory)
{
DBG_UNREFERENCED_PARAMETER(memory);

	OUTPUTDEBUGSTRING("cmc_free\n");

	return CMC_E_FAILURE;	// declare general failure
}

////////////////////////////////////////////////////////////////////////
// cmc_logoff
//
////////////////////////////////////////////////////////////////////////
extern "C" CMC_return_code EXPORT FAR PASCAL
cmc_logoff(
	CMC_session_id          session,
	CMC_ui_id               ui_id,
	CMC_flags               logoff_flags,
	CMC_extension FAR       *logoff_extensions)
{
DBG_UNREFERENCED_PARAMETER(session);
DBG_UNREFERENCED_PARAMETER(ui_id);
DBG_UNREFERENCED_PARAMETER(logoff_flags);
DBG_UNREFERENCED_PARAMETER(logoff_extensions);

	OUTPUTDEBUGSTRING("cmc_logoff\n");

	return CMC_E_FAILURE;	// declare general failure
}

////////////////////////////////////////////////////////////////////////
// cmc_logon
//
////////////////////////////////////////////////////////////////////////
extern "C" CMC_return_code EXPORT FAR PASCAL
cmc_logon(
	CMC_string              service,			//(i) optional service provider location
	CMC_string              user,				//(i) optional user name
	CMC_string              password,			//(i) optional user password
	CMC_object_identifier   character_set,		//(i) required character set (see cmc_query_configuration)
	CMC_ui_id               ui_id,				//(i) GUI handle
	CMC_uint16              caller_cmc_version,	//(i) client app CMC version (multiplied by 100)
	CMC_flags               logon_flags,		//(i) flag bits
	CMC_session_id FAR      *session,			//(o) returned session id
	CMC_extension FAR       *logon_extensions)	//(io) on input, MAPI extensions; on output, logon info (can be NULL)
{
DBG_UNREFERENCED_PARAMETER(service);
DBG_UNREFERENCED_PARAMETER(user);
DBG_UNREFERENCED_PARAMETER(password);
DBG_UNREFERENCED_PARAMETER(character_set);
DBG_UNREFERENCED_PARAMETER(ui_id);
DBG_UNREFERENCED_PARAMETER(caller_cmc_version);
DBG_UNREFERENCED_PARAMETER(logon_flags);
DBG_UNREFERENCED_PARAMETER(session);
DBG_UNREFERENCED_PARAMETER(logon_extensions);

#ifdef _DEBUG
	CString msg("cmc_logon:\n");
	char numbuf[40];

	msg += "  service = ";
	msg += (service ? service : "<NULL>");
	msg += "\n";

	msg += "  user = ";
	msg += (user ? user : "<NULL>");
	msg += "\n";

	msg += "  password = ";
	msg += (password ? password : "<NULL>");
	msg += "\n";

	ASSERT(character_set != NULL);
	msg += "  character_set = ";
	msg += character_set;
	msg += "\n";

	wsprintf(numbuf, "%lu", ui_id);
	msg += "  ui_id = ";
	msg += numbuf;
	msg += "\n";

	wsprintf(numbuf, "%hu", caller_cmc_version);
	msg += "  caller_cmc_version = ";
	msg += numbuf;
	msg += "\n";

	wsprintf(numbuf, "%#lX", logon_flags);
	msg += "  logon_flags = ";
	msg += numbuf;
	msg += "\n";

	OUTPUTDEBUGSTRING(msg);
#endif // _DEBUG

	
	return CMC_E_LOGON_FAILURE;	// declare functional failure
}

////////////////////////////////////////////////////////////////////////
// cmc_query_configuration
//
////////////////////////////////////////////////////////////////////////
extern "C" CMC_return_code EXPORT FAR PASCAL
cmc_query_configuration(
	CMC_session_id          session,
	CMC_enum                item,
	CMC_buffer              reference,
	CMC_extension FAR       *config_extensions)
{
DBG_UNREFERENCED_PARAMETER(session);
DBG_UNREFERENCED_PARAMETER(item);
DBG_UNREFERENCED_PARAMETER(reference);
DBG_UNREFERENCED_PARAMETER(config_extensions);

	OUTPUTDEBUGSTRING("cmc_query_configuration\n");

	return CMC_E_NOT_SUPPORTED;	// tell caller to go away
}
