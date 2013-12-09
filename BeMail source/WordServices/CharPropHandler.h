/*	CharPropHandler.h
 * Handle character ranges in a BTextView
 */

#pragma once


#include "LazyScriptHandler.h"
#include "NotifyLazyPropHandler.h"

class BTextView;

class CharPropHandler :
	public LazyScriptHandler
{
	public:
		CharPropHandler( BTextView *view, EForm form, const SData &data);
		~CharPropHandler() { }

		long PerformScriptAction(
			BMessage *				message,
			BMessage * &			reply);

	protected:

		BTextView *				fView;
		long					fStartIndex;
		long					fEndIndex;

		ScriptHandler *			GetSubHandler(
									const char *			propertyName,
									EForm					form,
									const SData &			data);
};
