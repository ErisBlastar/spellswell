#include "Mail.h"
#include "WindowPropHandler.h"

ScriptHandler *
TMailApp::GetSubHandler(
	const char *			propertyName,
	EForm					form,
	const SData &			data)
{
	int propCount = -1;

	if (!strcmp(propertyName, "property") && (form == formIndex)) {
		propCount = data.index;
	}
	if (!strcmp(propertyName, "window") || (0 == --propCount)) {
		long fIndex = -1;
		BWindow *subWindow = NULL;
		switch (form) {
		case formReverseIndex:
			fIndex = CountWindows()+data.index;
			break;
		case formIndex:
			fIndex = data.index-1;
			break;
		case formFirst:
			fIndex = 0;
			break;
		case formLast:
			fIndex = CountWindows()-1;
			break;
		case formName:
			for (int ix=0; ix<CountWindows(); ix++) {
				if (!strcmp(data.name, WindowAt(ix)->Title())) {
					fIndex = ix;
					break;
				}
			}
			break;
		case formDirect:
			// return new DirectWindowProp("window", this);
			break;
		case formID:
		default:
			/*	These forms are not supported
			 */
			break;
		}
		if (fIndex >= 0)
			subWindow = WindowAt(fIndex);
		if (!subWindow)
			return NULL;
		ScriptHandler *ret = dynamic_cast<ScriptHandler *>(subWindow);
		if (ret)
			return ret->Reference();
		/*	For views that are not ScriptHandlers in their own right, 
		 *	we use the stock ViewPropHandler to give a certain sense of scriptability.
		 */
		return new WindowPropHandler(subWindow->Title(), subWindow);
	}
	if (propCount > 0) {
		SData newData;
		newData.index = propCount;
		return ScriptHandler::GetSubHandler(propertyName, formIndex, newData);
	}
	return ScriptHandler::GetSubHandler(propertyName, form, data);
}

