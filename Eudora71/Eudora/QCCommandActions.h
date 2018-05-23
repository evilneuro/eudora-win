// QCCommandActions.h
//
// Contains action IDs for Command/Director architecture
//
// Copyright (c) 1997-2000 by QUALCOMM, Incorporated
/* Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. */

//


#ifndef QCCOMMANDACTIONS_H
#define QCCOMMANDACTIONS_H

// Need to preserve the values of these because they get persisted out to
// the Eudora.ini file if the user puts a custom item on the toolbar.
//
// If you ever have to get rid of something, just comment
// it out and preserve the value of the next item.
enum COMMAND_ACTION_TYPE
{
	CA_NONE,
	CA_NEW,
	CA_RENAME,
	CA_DELETE,
	CA_REPLY_WITH,
	CA_NEW_MESSAGE_WITH,
	CA_NEW_MESSAGE_TO,
	CA_FORWARD_TO,
	CA_REDIRECT_TO,
	CA_INSERT_RECIPIENT,
	CA_OPEN,
 	CA_AUTO_REPLY_WITH,
	CA_TRANSFER_TO,
	CA_COPY,
	CA_GRAFT,
//	CA_INSERT_FCC = 15,
	CA_UPDATE_STATUS = 16,
	CA_COMPACT,
	CA_ATTACH_PLUGIN,
	CA_SPECIAL_PLUGIN,
	CA_TRANSLATE_PLUGIN,
	CA_DRAW,
	CA_MEASURE,
	CA_TRANSMISSION_PLUGIN,
	CA_TOGGLE,
	CA_NEW_MAILBOX,
	CA_TRANSFER_NEW,
//	CA_FCC_NEW = 27,
//	CA_FILTER_TRANSFER = 28,
//	CA_FILTER_TRANSFER_NEW = 29,
	CA_REPLY_TO_ALL_WITH = 30,
	CA_MAKE_FILTER_RECIP,
	CA_IMAP_REFRESH,
	CA_IMAP_RESYNC,
	CA_IMAP_PROPERTIES,
	CA_DELETE_MESSAGE,
	CA_UNDELETE_MESSAGE,
	CA_EXPUNGE,
	CA_IMAP_GRAFT,
	CA_CHANGE_PERSONA,
	CA_SORT_AFTER_RENAME,
	CA_JUNK,
	CA_NOT_JUNK,
	CA_IMAP_RESYNC_TREE,
	CA_EMOTICON,
	CA_SAVED_SEARCH,

// =======================================================================
// Any items below this point must not be user visible commands. That is
// they must not be menu items / items that go in the toolbar. Because
// they're not toolbar items, the values for these do not need to be
// preserved.
//
// *** Do NOT put any items below this point whose values need
// to be preserved. ***
// =======================================================================

// For broadcasting spelling changes - used to fix bug reported by Irwin
// where changes to the dictionary did not propagate.
	CA_SPELLING_ADD_WORD,
// For registration/shareware manager
	CA_SWM_CHANGE_FEATURE,
	CA_SWM_CHANGE_REGCODE,
	CA_SWM_CHANGE_USERSTATE,
	CA_SELECTED_TEXT_URL
};

#endif
