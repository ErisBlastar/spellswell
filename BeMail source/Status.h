//--------------------------------------------------------------------
//	
//	Status.h
//
//	Written by: Robert Polic
//	
//	Copyright 1996 Be, Inc. All Rights Reserved.
//	
//--------------------------------------------------------------------

#ifndef STATUS_H
#define STATUS_H

#ifndef _BEEP_H
#include <Beep.h>
#endif
#ifndef _BUTTON_H
#include <Button.h>
#endif
#ifndef _TEXT_CONTROL_H
#include <TextControl.h>
#endif
#ifndef _WINDOW_H
#include <Window.h>
#endif

#define	STATUS_WIDTH		200
#define STATUS_HEIGHT		 70

#define STATUS_TEXT			"Status:"
#define STATUS_FIELD_H		  8
#define STATUS_FIELD_V		  8
#define STATUS_FIELD_WIDTH	(STATUS_WIDTH - 8)
#define STATUS_FIELD_HEIGHT	 16

#define BUTTON_WIDTH		 70
#define BUTTON_HEIGHT		 20
#define OK_BUTTON_TEXT		"OK"
#define OK_BUTTON_H			 15
#define OK_BUTTON_V			(STATUS_HEIGHT - BUTTON_HEIGHT - 12)

#define CANCEL_BUTTON_TEXT	"Cancel"
#define CANCEL_BUTTON_H		(STATUS_WIDTH - BUTTON_WIDTH - OK_BUTTON_H)
#define CANCEL_BUTTON_V		OK_BUTTON_V

enum	status_messages		{STATUS = 128, OK, CANCEL};

class	TStatusView;


//====================================================================

class TStatusWindow : public BWindow {

private:

TStatusView		*fView;

public:

				TStatusWindow(BRect, BWindow*, char*);
virtual void	MessageReceived(BMessage*);
};

//--------------------------------------------------------------------

class TStatusView : public BView {

private:

char			*fString;
BButton			*fOK;
BButton			*fCancel;
BTextControl	*fStatus;
BWindow			*fWindow;

public:

				TStatusView(BRect, BWindow*, char*); 
virtual	void	AttachedToWindow(void);
virtual	void	Draw(BRect);
virtual void	MessageReceived(BMessage*);
};
#endif
