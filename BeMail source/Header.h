//--------------------------------------------------------------------
//	
//	Header.h
//
//	Written by: Robert Polic
//	
//	Copyright 1996 Be, Inc. All Rights Reserved.
//	
//--------------------------------------------------------------------

#ifndef HEADER_H
#define HEADER_H

#ifndef _POINT_H
#include <Point.h>
#endif
#ifndef _RECORD_H
#include <Record.h>
#endif
#ifndef _RECT_H
#include <Rect.h>
#endif
#ifndef _TABLE_H
#include <Table.h>
#endif
#ifndef _TEXT_CONTROL_H
#include <TextControl.h>
#endif
#ifndef _TEXT_VIEW_H
#include <TextView.h>
#endif
#ifndef _VIEW_H
#include <View.h>
#endif
#ifndef _WINDOW_H
#include <Window.h>
#endif

#define HEADER_HEIGHT		 82
#define MIN_HEADER_HEIGHT	(HEADER_HEIGHT - 26)

#define TO_TEXT				"To:"
#define FROM_TEXT			"From:"
#define TO_FIELD_H			 39
#define FROM_FIELD_H		 31
#define TO_FIELD_V			  8
#define TO_FIELD_WIDTH		270
#define FROM_FIELD_WIDTH	280
#define TO_FIELD_HEIGHT		 16

#define SUBJECT_TEXT		"Subject:"
#define SUBJECT_FIELD_H		 18
#define SUBJECT_FIELD_V		 33
#define SUBJECT_FIELD_WIDTH	270
#define SUBJECT_FIELD_HEIGHT 16

#define CC_TEXT				"Cc:"
#define CC_FIELD_H			 40
#define CC_FIELD_V			 58
#define CC_FIELD_WIDTH		192
#define CC_FIELD_HEIGHT		 16

#define BCC_TEXT			"Bcc:"
#define BCC_FIELD_H			252
#define BCC_FIELD_V			 58
#define BCC_FIELD_WIDTH		197
#define BCC_FIELD_HEIGHT	 16

class	TMailWindow;
class	TTextControl;


//====================================================================
// Class: THeaderView
//
// This is the class that displays all the header information
// for the message.  This includes the To, From, Subject, Cc, 
// and Bcc fields.  It uses text controls to display the data.
//====================================================================

class THeaderView : public BView {

private:

bool			fIncoming;
float			fWidth;
BRecord			*fRecord;
TMailWindow		*fWindow;

public:

TTextControl	*fBcc;
TTextControl	*fCc;
TTextControl	*fSubject;
TTextControl	*fTo;

				THeaderView(BRect, TMailWindow*, BRect, bool, BRecord*); 
virtual	void	Draw(BRect);
void			ReSize(void);
};


//====================================================================
// Class: TTextControl
//
// This class manages the display and editing of a single piece of 
// text.
//====================================================================

class TTextControl : public BTextControl {

private:

bool			fIncoming;
char			fLabel[100];
long			fCommand;
BRecord			*fRecord;

public:

				TTextControl(BRect, char*, BMessage*, bool, BRecord*);
virtual void	AttachedToWindow(void);
virtual void	Draw(BRect);
virtual void	MessageReceived(BMessage*);
};
#endif
