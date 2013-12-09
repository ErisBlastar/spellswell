//	ScriptHandler.cpp

#include "ScriptHandler.h"
#include "Scripting.h"
#include "Coercion.h"
#include <Message.h>
#include "LazyPropHandler.h"



Coercions * ScriptHandler::sCoercions;

ScriptHandler::ScriptHandler(
	const unsigned long		id,
	const char *			name)
{
	fId = id;
	AllocName(name);
	if (!sCoercions) {
		sCoercions = new Coercions;
		sCoercions->AddCoercionHandler(new StandardNumberCoercions);
		sCoercions->AddCoercionHandler(new StandardGraphicsCoercions);
		sCoercions->AddCoercionHandler(new StandardTextCoercions);
	}
}


ScriptHandler::~ScriptHandler()
{
	FreeName();
}


void
ScriptHandler::FreeName()
{
	delete[] fName;
}


void
ScriptHandler::AllocName(
	const char *		name)
{
	fName = new char[strlen(name)+1];
	strcpy(fName, name);
}


void
ScriptHandler::SetName(
	const char *		name)
{
	FreeName();
	AllocName(name);
}


ScriptHandler *
ScriptHandler::Reference()
{
	return this;
}


void
ScriptHandler::Done()
{
	//	do nothing
}


ScriptHandler *
ScriptHandler::FindScriptTarget(
	BMessage *				message,
	const char *			parameter)
{
	if (!parameter) {
		parameter = kDefaultTargetName;
	}
	unsigned long type;
	long count;
	if (!message->GetInfo(parameter, &type, &count)) {
		return NULL;
	}
	ScriptHandler *handler = this->Reference();
	for (int ix=count-1; ix>=0; ix--) {
		long size;
		PropertyItem *item = (PropertyItem *)message->FindData(
			parameter, PROPERTY_TYPE, ix, &size);
		ScriptHandler *oldFriend = handler;
		handler = handler->GetSubHandler(item->property, item->form, item->data);
		oldFriend->Done();	//	good-bye, old friend!
		if (!handler)
			break;
	}
	return handler;
}


long
ScriptHandler::PerformScriptAction(
	BMessage *				message,
	BMessage * &			reply)
{
	return SCRIPT_BAD_VERB;
}


ScriptHandler *
ScriptHandler::GetSubHandler(
	const char *			propertyName,
	EForm					form,
	const SData &			data)
{
/*	Since we may be a lazy property, and reference ourselves with another lazy property, 
 *	we tell the lazy properties to reference us so we don't die ahead of time
 */
	LazyPropHandler *lprop = NULL;

	int propCount = -1;
/*	This test is especially marked; see below about enumerating properties
 */
	if (!strcmp(propertyName, "property") && (form == formIndex)) {
		propCount = data.index;
	}
	if (!strcmp(propertyName, "name") || (0 == --propCount)) {
		const char *name = GetName();
		lprop = new LazyPropHandler("name", (void *)name, strlen(name), B_ASCII_TYPE, false);
		lprop->SetHandler(this);
		return lprop;
	}
	if (!strcmp(propertyName, "id") || (0 == --propCount)) {
		lprop = new LazyPropHandler("id", &fId, sizeof(fId), B_LONG_TYPE, false);
		lprop->SetHandler(this);
		return lprop;
	}
/*	This is a neat thing. A script can say:
 *	target.property("name") as well as target.name
 *	This allows languages with strong typing to work with properties as first-class objects
 *	To support enumeration of properties, we also support indexing. This requires the 
 *	subclasses to test for this case (like the marked test above) and deduct for their number 
 *	of properties.
 *	target.property(2).name would return "id" for the base ScriptHandler, whereas 
 *	target.property(2) would return the actual ID.
 *	Notice that with lazy handlers, we have to think about life-span too! That's why we 
 *	build a chain of handlers to dispose in FindScriptTarget instead of disposing directly.
 */
	if (!strcmp(propertyName, "property")) {
		if (form != formName)
			return NULL;
		return GetSubHandler(data.name, formDirect, data);
	}
	return NULL;
}

