//--------------------------------------------------------------------
//	
//	Status.cpp
//
//	Written by: Robert Polic
//	
//	Copyright 1996 Be, Inc. All Rights Reserved.
//	
//--------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#ifndef MAIL_H
#include "Mail.h"
#endif
#ifndef STATUS_H
#include "Status.h"
#endif


//====================================================================
// Class: TStatusWindow
// 
// A simple alert panel that displays some text status.
// 
//====================================================================

TStatusWindow::TStatusWindow(BRect rect, BWindow *window, char *status)
			  :BWindow(rect, "", B_MODAL_WINDOW, 0)
{
	BRect		r;

	// Create the view that will diplay the information
	r.Set(0, 0, STATUS_WIDTH, STATUS_HEIGHT);
	fView = new TStatusView(r, window, status);
	Lock();
	AddChild(fView);
	Unlock();
	
	// Show ourselves by default
	Show();
}

//--------------------------------------------------------------------
// Method: MessageReceived
//
// If any messages come to the window, they should just be sent
// on to the view for handling.  This window is here only to support
// displaying the view.  It doesn't actually do any work itself.
//--------------------------------------------------------------------

void TStatusWindow::MessageReceived(BMessage *msg)
{
	PostMessage(msg, fView);
}


//====================================================================
// Class: TStatusView
//
// This is the class that does all the work in a status window.  In 
// the constructor we only setup those things that don't require being
// attached to the window.
//====================================================================

TStatusView::TStatusView(BRect rect, BWindow *window, char *status)
			:BView(rect, "", B_FOLLOW_ALL, B_WILL_DRAW)
{
	rgb_color	c;

	fWindow = window;
	fString = status;

	c.red = c.green = c.blue = VIEW_COLOR;
	SetViewColor(c);
	SetFontName("Erich");
	SetFontSize(9);
	SetDrawingMode(B_OP_OVER);
}

//--------------------------------------------------------------------
// Method: AttachedToWindow
//
// The windowing framework will call this method when the view
// is actually attached to a window.  It is a good idea to setup
// any other views, buttons and the like here because they generally
// require that the view is attached to the window in order to work.
//--------------------------------------------------------------------

void TStatusView::AttachedToWindow(void)
{
	BRect		r;

	r.Set(STATUS_FIELD_H, STATUS_FIELD_V,
		  STATUS_FIELD_WIDTH,
		  STATUS_FIELD_V + STATUS_FIELD_HEIGHT);
	fStatus = new BTextControl(r, "", STATUS_TEXT, fString,
							new BMessage(STATUS));
	AddChild(fStatus);
	fStatus->SetFontName("Erich");
	fStatus->SetFontSize(9);
	fStatus->SetDivider(StringWidth(STATUS_TEXT) + 6);
	fStatus->BTextControl::MakeFocus(TRUE);

	r.Set(OK_BUTTON_H, OK_BUTTON_V,
		  OK_BUTTON_H + BUTTON_WIDTH,
		  OK_BUTTON_V + BUTTON_HEIGHT);
	fOK = new BButton(r, "", OK_BUTTON_TEXT, new BMessage(OK));
	AddChild(fOK);

	r.Set(CANCEL_BUTTON_H, CANCEL_BUTTON_V,
		  CANCEL_BUTTON_H + BUTTON_WIDTH,
		  CANCEL_BUTTON_V + BUTTON_HEIGHT);
	fCancel = new BButton(r, "", CANCEL_BUTTON_TEXT, new BMessage(CANCEL));
	AddChild(fCancel);

	fOK->MakeDefault(TRUE);
}

//--------------------------------------------------------------------
// Method: Draw
//
// Purpose:
//	This is called automatically by the framework when needed.  All
//	we do here is draw a nice shadowed border around the view to give 
//	it some depth.
//--------------------------------------------------------------------

void TStatusView::Draw(BRect where)
{
	BRect	r;

	r = Bounds();

	SetHighColor(255, 255, 255);
	StrokeLine(BPoint(r.left, r.top), BPoint(r.right, r.top));
	StrokeLine(BPoint(r.left, r.top + 1), BPoint(r.left, r.bottom - 1));
	SetHighColor(120, 120, 120);
	StrokeLine(BPoint(r.right, r.top + 1), BPoint(r.right, r.bottom));
	StrokeLine(BPoint(r.right - 1, r.bottom - 1), BPoint(r.left, r.bottom - 1));
}


//--------------------------------------------------------------------
// Method: MessageReceived
//
// All messages that are generated by the buttons should show up here.
// We basically want to deal with the OK button by telling the fWindow
// instance that we are ready to close, and that there is information
// to be processed.
//
// In the case of Quit be pressed, just quit the current window and
// leave it at that.
//--------------------------------------------------------------------

void TStatusView::MessageReceived(BMessage *msg)
{
	BMessage	*message;

	switch (msg->what) {
		case STATUS:
			break;

		case OK:
			message = new BMessage(M_CLOSE_CUSTOM);
			message->AddString("status", fStatus->Text());
			fWindow->PostMessage(message);
			// will fall through
		case CANCEL:
			Window()->Quit();
			break;
	}
}
