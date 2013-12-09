/*	ScriptHandler.h
 *	$Id: ScriptHandler.h,v 1.1 1996/12/04 11:58:36 hplus Exp elvis $
 *	Simple superclass for "things" that handle script events
 */

#pragma once


#include "Scripting.h"


class Coercions;

class ScriptHandler
{
public:
								ScriptHandler(
									const unsigned long		id,
									const char *			name);
virtual							~ScriptHandler();

		ScriptHandler *			FindScriptTarget(
									BMessage *				message,
									const char *			parameter);

virtual	long					PerformScriptAction(
									BMessage *				message,
									BMessage * &			reply);

		unsigned long			GetID() { return fId; }
virtual	const char *			GetName() { return fName; }
virtual	void					SetName(
									const char *			name);

virtual	ScriptHandler *			Reference();
virtual	void					Done();

virtual	Coercions &				GetCoercions() { return *sCoercions; }

protected:

		unsigned long			fId;
		char *					fName;
		void					FreeName();
		void					AllocName(
									const char * name);

static	Coercions *				sCoercions;

virtual	ScriptHandler *			GetSubHandler(
									const char *			propertyName,
									EForm					form,
									const SData &			data);

};
