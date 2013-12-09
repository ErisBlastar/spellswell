/*	LazyScriptHandler.h
 *	$Id: LazyScriptHandler.h,v 1.1 1996/12/04 11:58:38 hplus Exp elvis $
 *	You would use a LazyScriptHandler when you're talking to something that's not really 
 *	a ScriptHandler in your application. Typically, you'll always use it for referencing 
 *	properties of your items. You may want to use it to reference sub-items, too, to be able 
 *	to store context information about them.
 */
#pragma once

#include "ScriptHandler.h"


class LazyScriptHandler :
	public ScriptHandler
{
public:
								LazyScriptHandler(	//	set ref count to 1
									const unsigned long		id,
									const char *			name);

		ScriptHandler *			Reference();	//	increase ref count
		void					Done();			//	decrease ref count, delete if 0

private:

								~LazyScriptHandler();

		int						fRefCount;
};
