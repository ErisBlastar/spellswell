//--------------------------------------------------------------------
//	
//	Content.h
//
//	Written by: Robert Polic
//	
//	Copyright 1996 Be, Inc. All Rights Reserved.
//	
//--------------------------------------------------------------------

#ifndef CONTENT_H
#define CONTENT_H

#ifndef _FILE_H
#include <File.h>
#endif
#ifndef _LOOPER_H
#include <Looper.h>
#endif
#ifndef _POINT_H
#include <Point.h>
#endif
#ifndef _RECT_H
#include <Rect.h>
#endif
#ifndef _SCROLL_VIEW_H
#include <ScrollView.h>
#endif
#ifndef _TEXT_VIEW_H
#include <TextView.h>
#endif
#ifndef _WINDOW_H
#include <Window.h>
#endif

#define MESSAGE_TEXT		"Message:"
#define MESSAGE_TEXT_H		 16
#define MESSAGE_TEXT_V		 20
#define MESSAGE_FIELD_H		 59
#define MESSAGE_FIELD_V		 11

class	TMailWindow;
class	TScrollView;
class	TTextView;

typedef struct {
	bool		header;
	bool		quote;
	bool		incoming;
	TTextView	*view;
	BRecord		*record;
	sem_id		*stop_sem;
} reader;

bool	get_semaphore(BWindow*, sem_id*);


//====================================================================
// Class: TContentView
//
// The content view displays the contents of the message.  It 
// achieves this through the use of a scrollview and a text view.
//====================================================================

// MDC Word Services Begin
class TContentView : public BView, public ScriptHandler {
// MDC Word Services End

private:

	bool			fFocus;
	bool			fIncoming;
	float			fWidth;
	BRecord			*fRecord;
	TMailWindow		*fWindow;
	TScrollView		*fScroll;

public:

	TTextView		*fTextView;

					TContentView(BRect, TMailWindow*, bool, BRecord*, char*, long); 

	virtual	void	Draw(BRect);
	virtual void	MessageReceived(BMessage*);
			void	Focus(bool);
			void	FrameText(void);
			void	ReSize(void);
// MDC Word Services Begin
long PerformScriptAction(
	BMessage *				message,
	BMessage * &			reply);
ScriptHandler *GetSubHandler(
	const char *			propertyName,
	EForm					form,
	const SData &			data);
// MDC Word Services End
};

//====================================================================
// Class: TTextView
//
// This class is what goes into the scroll view.  Here is where
// the text of the message is actually displayed.
//====================================================================

class TTextView : public BTextView {

private:

bool			fIncoming;
char			*fYankBuffer;
long			fFontSize;
long			fLastPosition;
BRecord			*fRecord;
TContentView	*fParent;
font_name		fFontName;
sem_id			fStopSem;
thread_id		fThread;

public:

bool			fReady;

					TTextView(BRect, BRect, bool, BRecord*, TContentView*,
						  char*, long); 
					~TTextView(void);

	virtual	void	AttachedToWindow(void);
	virtual void	KeyDown(ulong);
	virtual void	MakeFocus(bool);
	virtual void	MessageReceived(BMessage*);
			void	LoadMessage(BRecord*, bool, bool, const char*);
	static  long	Reader(reader*);
			void	StopLoad(void);
};
#endif
