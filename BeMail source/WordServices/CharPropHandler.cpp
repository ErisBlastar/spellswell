/*	CharPropHandler.cpp
 *	Scripting of a BTextView's character range for Word Services
 *  3 May 97 Mike Crawford crawford@scruznet.com
 */

#include "CharPropHandler.h"
#include "LazyPropHandler.h"
#include "MessengerHandler.h"
#include "NotifyLazyPropHandler.h"
#include "WordServicesBe.h"

CharPropHandler::CharPropHandler( BTextView * view, EForm form, const SData &data) :
	fView( view ),
	fStartIndex( -1 ),
	fEndIndex( -1 ),
	LazyScriptHandler((long)view, view->Name())
{

	switch (form) {
	case formReverseIndex:
		fStartIndex = fView->TextLength()+data.index;
		fEndIndex = fStartIndex;
		break;
	
	case formReverseIndexRange:
		fStartIndex = fView->TextLength()+data.range.start;
		fEndIndex = fStartIndex + data.range.length ;
		break;

	case formIndex:
		fStartIndex = data.index-1;
		fEndIndex = fStartIndex;
		break;
	case formFirst:
		fStartIndex = 0;
		fEndIndex = fStartIndex;
		break;
	case formLast:
		fStartIndex = fView->TextLength()-1;
		fEndIndex = fStartIndex;
		break;
	case formName:
	case formID:
	case formDirect:
	default:
		/*	These forms are not supported
		 */
		break;
	}

}


ScriptHandler *
CharPropHandler::GetSubHandler(
	const char *			propertyName,
	EForm					form,
	const SData &			data)
{
	int propCount = -1;
	LazyPropHandler *lprop;

	if (!strcmp(propertyName, "property") && (form == formIndex)) {
		propCount = data.index;
	}

	if (!strcmp(propertyName, "character") || (0 == --propCount)) {
		fStartIndex = -1;
		fEndIndex = -1;

		switch (form) {
		case formReverseIndex:
			fStartIndex = fView->TextLength()+data.index;
			fEndIndex = fStartIndex;
			break;
		
		case formReverseIndexRange:
			fStartIndex = fView->TextLength()+data.range.start;
			fEndIndex = fStartIndex - data.range.length ;
			break;

		case formIndex:
			fStartIndex = data.index-1;
			fEndIndex = fStartIndex;
			break;
		case formFirst:
			fStartIndex = 0;
			fEndIndex = fStartIndex;
			break;
		case formLast:
			fStartIndex = fView->TextLength()-1;
			fEndIndex = fStartIndex;
			break;
		case formName:
		case formID:
		case formDirect:
		default:
			/*	These forms are not supported
			 */
			break;
		}

		// We can't be a container for any other data
		
		return NULL;

	}
	if (propCount > 0) {
		SData newData;
		newData.index = propCount;
		return inherited::GetSubHandler(propertyName, formIndex, newData);
	}
	return inherited::GetSubHandler(propertyName, form, data);
}

long
CharPropHandler::PerformScriptAction(
	BMessage *				message,
	BMessage * &			reply)
{
	switch (message->what) {
	case kSetVerb:
		long textLen;
		char *newText = (char*)message->FindData( "data", B_ANY_TYPE, &textLen );
		
		fView->Window()->Lock();
		fView->Select( fStartIndex, fEndIndex );
		fView->Delete();
		fView->Insert( newText, textLen );
		fView->Window()->Unlock();
		if (!reply)
			reply = new BMessage(kReplyVerb);
		return B_NO_ERROR;
	}
	return SCRIPT_BAD_VERB;
}



