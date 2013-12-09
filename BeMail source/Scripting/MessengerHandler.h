/*	MessengerHandler.h
 *	$Id: MessengerHandler.h,v 1.1 1996/12/04 11:58:41 hplus Exp elvis $
 *	Handles a "messenger" property of a handler
 */

#pragma once

#include "LazyScriptHandler.h"


class BHandler;


class MessengerHandler :
	public LazyScriptHandler
{
public:
								MessengerHandler(
									const char *		name,
									BHandler *			handler);
								~MessengerHandler() { }

protected:

		BHandler *				fHandler;

virtual	long					PerformScriptAction(
									BMessage *				message,
									BMessage * &			reply);

virtual	long					DoGet(
									BMessage *				message,
									BMessage * &			reply);

};
