//--------------------------------------------------------------------
//	
//	Mail.h
//
//	Written by: Robert Polic
//	
//	Copyright 1996 Be, Inc. All Rights Reserved.
//	
//--------------------------------------------------------------------

#ifndef MAIL_H
#define MAIL_H

#ifndef _ALERT_H
#include <Alert.h>
#endif
#ifndef _APPLICATION_H
#include <Application.h>
#endif
#ifndef _BEEP_H
#include <Beep.h>
#endif
#ifndef _DIRECTORY_H
#include <Directory.h>
#endif
#ifndef _E_MAIL_H
#include <E-mail.h>
#endif
#ifndef _LOOPER_H
#include <Looper.h>
#endif
#ifndef _MENU_H
#include <Menu.h>
#endif
#ifndef _MENU_BAR_H
#include <MenuBar.h>
#endif
#ifndef _MENU_ITEM_H
#include <MenuItem.h>
#endif
#ifndef _POP_UP_MENU_H
#include <PopUpMenu.h>
#endif
#ifndef _POINT_H
#include <Point.h>
#endif
#ifndef _PRINT_JOB_H
#include <PrintJob.h>
#endif
#ifndef _HANDLER_H
#include <Handler.h>
#endif
#ifndef _RECT_H
#include <Rect.h>
#endif
#ifndef _ROSTER_H
#include <Roster.h>
#endif
#ifndef _SCROLL_VIEW_H
#include <ScrollView.h>
#endif
#ifndef _WINDOW_H
#include <Window.h>
#endif

// MDC Word Services Begin
#include "ScriptHandler.h"
// MDC Word Services End

#define	BROWSER_WIND		 82
#define	TITLE_BAR_HEIGHT	 25
#define	WIND_WIDTH			457
#define WIND_HEIGHT			400

enum	MESSAGES			{REFS_RECEIVED = 64, LIST_INVOKED, WINDOW_CLOSED,
							 CHANGE_FONT};

enum	TEXT				{SUBJECT_FIELD = REFS_RECEIVED + 64, TO_FIELD,
							 ENCLOSE_FIELD, CC_FIELD, BCC_FIELD, NAME_FIELD};

enum	MENUS	/* app */	{M_NEW = SUBJECT_FIELD + 64, M_EDIT_SIGNATURE,
									M_FONT, M_SIZE, M_BEGINNER, M_EXPERT,
				/* file */	 M_REPLY, M_REPLY_ALL, M_FORWARD, M_DELETE,
									M_HEADER, M_SEND_NOW, M_SEND_LATER,
									M_SAVE, M_PRINT, M_CLOSE_READ,
									M_CLOSE_SAME, M_CLOSE_CUSTOM,
				/* edit */	 M_UNDO, M_SELECT, M_QUOTE, M_REMOVE_QUOTE,
									M_SIGNATURE,
				/* encls */	 M_ADD, M_REMOVE, M_INFO};

enum	USER_LEVEL			 {L_BEGINNER = 0, L_EXPERT};

enum	WINDOW_TYPES		 {MAIL_WINDOW = 0, SIG_WINDOW};

#define SEPERATOR_MARGIN	  7

#define	VIEW_COLOR			216

#define QUOTE				">"


class	TMailWindow;
class	THeaderView;
class	TEnclosuresView;
class	TContentView;
class	TMenu;
class	TSignatureWindow;


//====================================================================

class ScriptHandler;

class TMailApp : public BApplication, public ScriptHandler{

private:

long			fFontSize;
long			fWindowCount;
BMenu			*fLevelMenu;
BRect			fMailWinPos;
BRect			fSigWinPos;
TSignatureWindow	*fSigWindow;
font_name		fFontName;

public:

				TMailApp(void);
virtual void	AboutRequested(void);
virtual void	ArgvReceived(int, char**);
virtual void	MessageReceived(BMessage*);
virtual bool	QuitRequested(void);
virtual void	ReadyToRun(void);
virtual void	RefsReceived(BMessage*);
TMailWindow*	FindWindow(record_ref);
void			FontChange(void);
TMailWindow*	NewWindow(BRecord *rec = NULL, char *to = NULL);

// MDC Word Services Begin
ScriptHandler *GetSubHandler(
	const char *			propertyName,
	EForm					form,
	const SData &			data);
// MDC Word Services End
};

//--------------------------------------------------------------------

class TMailWindow : public BWindow{

private:

bool			fDelete;
bool			fIncoming;
bool			fReplying;
char			fStatus[256];
BMenuItem		*fAdd;
BMenuItem		*fCut;
BMenuItem		*fCopy;
BMenuItem		*fHeader;
BMenuItem		*fInfo;
BMenuItem		*fPaste;
BMenuItem		*fPrint;
BMenuItem		*fQuote;
BMenuItem		*fRemove;
BMenuItem		*fRemoveQuote;
BMenuItem		*fSave;
BMenuItem		*fSendNow;
BMenuItem		*fSendLater;
BMenuItem		*fUndo;
TContentView	*fContentView;
THeaderView		*fHeaderView;
TEnclosuresView	*fEnclosuresView;
TMenu			*fSignature;

public:

BRecord			*fRecord;

				TMailWindow(BRect, char*, BRecord*, char*, char*, long);
				~TMailWindow(void);
virtual void	FrameResized(float, float);
virtual void	MenusWillShow(void);
virtual void	MessageReceived(BMessage*);
virtual bool	QuitRequested(void);
virtual void	SaveRequested(record_ref, const char*);
virtual void	Show(void);
void			Forward(BRecord*, TMailWindow*);
void			Print(void);
void			Reply(BRecord*, TMailWindow*, bool);
void			Send(bool);
};

//====================================================================

class TMenu: public BMenu {

private:

public:
				TMenu(const char*);
virtual void	AttachedToWindow(void);
void			BuildMenu(void);
};

//====================================================================

class TScrollView : public BScrollView {

private:

public:
				TScrollView(BView*);
virtual	void	Draw(BRect);
};
#endif
