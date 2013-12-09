//--------------------------------------------------------------------
//	
//	Content.cpp
//
//	Written by: Robert Polic
//	
//	Copyright 1996 Be, Inc. All Rights Reserved.
//	
//--------------------------------------------------------------------

#include <stdlib.h>
#include <string.h>

#ifndef MAIL_H
#include "Mail.h"
#endif
#ifndef CONTENT_H
#include "Content.h"
#endif

#include "Util.h"

// MDC Word Services Begin
#include "LazyPropHandler.h"
#include "CharPropHandler.h"
// MDC Word Services End

//====================================================================
// Class: TContentView
//
// The content view is the primary displayer of the content of 
// the mail message.  It maintains a scroll view and a text
// view.  It is also responsible for drawing the fancy border
// and label that shows up.
//====================================================================

// MDC Word Services Begin
// We need to name the content field so we can ask for it in a scripting message
//
// TContentView::TContentView(BRect rect, TMailWindow *window, bool incoming,
//							BRecord *record, char* font, long size)
//			 :BView(rect, "", B_FOLLOW_ALL, B_WILL_DRAW)
//
TContentView::TContentView(BRect rect, TMailWindow *window, bool incoming,
							BRecord *record, char* font, long size)
			 :BView(rect, "Content", B_FOLLOW_ALL, B_WILL_DRAW),
			 	ScriptHandler( 0, "foo" )
// MDC Word Services End
{
	BRect		r;
	BRect		text;
	rgb_color	c;

	fWindow = window;
	fIncoming = incoming;
	fRecord = record;
	fFocus = FALSE;
	fWidth = 0;

	c.red = c.green = c.blue = VIEW_COLOR;
	SetViewColor(c);
	SetFontName("Erich");
	SetFontSize(9);
	SetDrawingMode(B_OP_OVER);

	r = rect;
	r.OffsetTo(0, 0);
	r.left = MESSAGE_FIELD_H;
	r.right -= (B_V_SCROLL_BAR_WIDTH + 11);
	r.top = MESSAGE_FIELD_V;
	r.bottom -= 12;
	text = r;
	text.OffsetTo(0, 0);

	fTextView = new TTextView(r, text, fIncoming, fRecord, this, font, size);
	fScroll = new TScrollView(fTextView);
	AddChild(fScroll);
}

// MDC Word Services Begin
long
TContentView::PerformScriptAction(
	BMessage *				message,
	BMessage * &			reply)
{
	switch (message->what) {
	case kSetVerb:
		//return DoSet(message, reply);
		break;
	case kGetVerb:
		if (!reply) {
			reply = new BMessage(kReplyVerb);
		}
		reply->AddData(kDefaultDataName, 'TEXT', fTextView->Text(), fTextView->TextLength() );
		return B_NO_ERROR;
		break;
	default:
		return SCRIPT_BAD_VERB;
	}

	return 0;	//	not reached
}

ScriptHandler *
TContentView::GetSubHandler(
	const char *			propertyName,
	EForm					form,
	const SData &			data)
{
	int propCount = -1;
	LazyPropHandler *lprop;

	if (!strcmp(propertyName, "property") && (form == formIndex)) {
		propCount = data.index;
	}
	
	// MDC Word Services Begin
	if ( !strcmp( propertyName, "character" ) || ( 0 == -- propCount ) ){
		return new CharPropHandler( (BTextView*)fTextView, form, data );
	}
	// MDC Word Services End

#if 0
	if (!strcmp(propertyName, "messenger") || (0 == --propCount)) {
		return new MessengerHandler("messenger", fView);
	}
	if (!strcmp(propertyName, "frame") || (0 == --propCount)) {
		fFrame = fView->Frame();
		lprop = new LazyPropHandler("frame", &fFrame, sizeof(BRect), B_RECT_TYPE, false);
		lprop->SetHandler(this);
		return lprop;
	}
	if (!strcmp(propertyName, "color") || (0 == --propCount)) {
		fColor = fView->ViewColor();
		lprop = new NotifyLazyPropHandler<ViewPropHandler, long>("color", &fColor, sizeof(fColor), B_RGB_COLOR_TYPE,
				*this, 0);
		lprop->SetHandler(this);
		return lprop;
	}
	if (!strcmp(propertyName, "view") || (0 == --propCount)) {
		long fIndex = -1;
		BView *subView = NULL;
		switch (form) {
		case formReverseIndex:
			fIndex = this->CountChildren()+data.index;
			break;
		case formIndex:
			fIndex = data.index-1;
			break;
		case formFirst:
			fIndex = 0;
			break;
		case formLast:
			fIndex = this->CountChildren()-1;
			break;
		case formName:
			subView = this->FindView(data.name);
			break;
		case formID:
		case formDirect:
		default:
			/*	These forms are not supported
			 */
			break;
		}
		if (fIndex >= 0)
			subView = this->ChildAt(fIndex);
		if (!subView)
			return NULL;
		ScriptHandler *ret = dynamic_cast<ScriptHandler *>(subView);
		if (ret)
			return ret->Reference();
		/*	For views that are not ScriptHandlers in their own right, 
		 *	we use the stock ViewPropHandler to give a certain sense of scriptability.
		 */
		return new ViewPropHandler(subView);
	}
	if (propCount > 0) {
		SData newData;
		newData.index = propCount;
		return inherited::GetSubHandler(propertyName, formIndex, newData);
	}
#endif

//	return ViewPropHandler::GetSubHandler(propertyName, form, data);
	return NULL;
}
// MDC Word Services End

//-----------------------------------------------------------------
// Method: Draw
//
// Draw all the border trim and label associated with the content
// view.
//-----------------------------------------------------------------

void TContentView::Draw(BRect where)
{
	BRect	r;

	r = Bounds();
	if (!fWidth)
		fWidth = r.Width();

	// Draw some nice raised border type stuff.
	SetHighColor(255, 255, 255);
	StrokeLine(BPoint(r.left, r.top), BPoint(r.right, r.top));
	StrokeLine(BPoint(r.left, r.top + 1), BPoint(r.left, r.bottom - 1));
	SetHighColor(120, 120, 120);
	StrokeLine(BPoint(r.right, r.top + 1), BPoint(r.right, r.bottom));
	StrokeLine(BPoint(r.right - 1, r.bottom), BPoint(r.left, r.bottom));
	FrameText();

	// Draw the 'Message:' label next to the view.  If the view
	// is currently the focus view, then draw a black line
	// under this label.  Otherwise, draw a line the same color
	// as the background.
	SetHighColor(0, 0, 0);
	SetFontName("Erich");
	SetFontSize(9);
	MovePenTo(MESSAGE_TEXT_H, MESSAGE_TEXT_V);
	DrawString(MESSAGE_TEXT);
	if (!fFocus)
		SetHighColor(VIEW_COLOR, VIEW_COLOR, VIEW_COLOR);
	StrokeLine(BPoint(MESSAGE_TEXT_H, MESSAGE_TEXT_V + 2),
			   BPoint(MESSAGE_TEXT_H + StringWidth(MESSAGE_TEXT),
									  MESSAGE_TEXT_V + 2));
}

//-----------------------------------------------------------------
// Method: MessageReceived
//
// This is where all the action comes to.  Commands from the menus
// are sent here for processing.  Quote, Remove Quote, and add
// signature.
//-----------------------------------------------------------------

void TContentView::MessageReceived(BMessage *msg)
{
	char		*str;
	char		*quote;
	const char	*text;
	char		new_line = '\n';
	long		finish;
	long		len;
	long		loop;
	long		new_start;
	long		offset;
	long		removed = 0;
	long		size;
	long		start;
	long		value;
	BRecord		*record;
	BRect		r;

	switch (msg->what) {
		// 'quote' the currently selected text
		case M_QUOTE:
			r = fTextView->Bounds();
			fTextView->GetSelection(&start, &finish);
			quote = (char *)malloc(strlen(QUOTE));
			strcpy(quote, QUOTE);
			len = strlen(QUOTE);
			fTextView->GoToLine(fTextView->CurrentLine());
			fTextView->GetSelection(&new_start, &new_start);
			fTextView->Select(new_start, finish);
			finish -= new_start;
			str = (char *)malloc(finish + 1);
			fTextView->GetText(str, new_start, finish);
			offset = 0;
			for (loop = 0; loop < finish; loop++) {
				if (str[loop] == '\n') {
					quote = (char *)realloc(quote, len + loop - offset + 1);
					memcpy(&quote[len], &str[offset], loop - offset + 1);
					len += loop - offset + 1;
					offset = loop + 1;
					if (offset < finish) {
						quote = (char *)realloc(quote, len + strlen(QUOTE));
						memcpy(&quote[len], QUOTE, strlen(QUOTE));
						len += strlen(QUOTE);
					}
				}
			}
			if (offset != finish) {
				quote = (char *)realloc(quote, len + (finish - offset));
				memcpy(&quote[len], &str[offset], finish - offset);
				len += finish - offset;
			}
			free(str);

			fTextView->Delete();
			fTextView->Insert(quote, len);
			if (start != new_start) {
				start += strlen(QUOTE);
				len -= (start - new_start);
			}
			fTextView->Select(start, start + len);
			fTextView->ScrollTo(r.LeftTop());
			free(quote);
			break;

		// Remove the 'quotes' and the beginning of the message text
		case M_REMOVE_QUOTE:
			r = fTextView->Bounds();
			fTextView->GetSelection(&start, &finish);
			len = start;
			fTextView->GoToLine(fTextView->CurrentLine());
			fTextView->GetSelection(&start, &start);
			fTextView->Select(start, finish);
			new_start = finish;
			finish -= start;
			str = (char *)malloc(finish + 1);
			fTextView->GetText(str, start, finish);
			for (loop = 0; loop < finish; loop++) {
				if (strncmp(&str[loop], QUOTE, strlen(QUOTE)) == 0) {
					finish -= strlen(QUOTE);
					memcpy(&str[loop], &str[loop + strlen(QUOTE)],
									finish - loop);
					removed += strlen(QUOTE);
				}
				while ((loop < finish) && (str[loop] != '\n')) {
					loop++;
				}
				if (loop == finish)
					break;
			}
			if (removed) {
				fTextView->Delete();
				fTextView->Insert(str, finish);
				new_start -= removed;
				fTextView->Select(new_start - finish + (len - start) - 1,
								  new_start);
			}
			else
				fTextView->Select(len, new_start);
			fTextView->ScrollTo(r.LeftTop());
			free(str);
			break;

		// Add the signature into the body of the message
		case M_SIGNATURE:
			record = new BRecord(msg->FindRef("sig"));
			str = (char *)record->FindRaw("signature", &size);
			if (size) {
				fTextView->GetSelection(&start, &finish);
				text = fTextView->Text();
				len = fTextView->TextLength();
				if ((len) && (text[len - 1] != '\n')) {
					fTextView->Select(len, len);
					fTextView->Insert(&new_line, 1);
					len++;
				}
				fTextView->Select(len, len);
				fTextView->Insert(str, size);
				fTextView->Select(len, len + size);
				fTextView->ScrollToSelection();
				fTextView->Select(start, finish);
				fTextView->ScrollToSelection();
			}
			delete record;
			break;

		default:
			// MDC Word Services Begin
			// was inherited::
			BView::MessageReceived(msg);
			// MDC Word Services End
	}
}

//--------------------------------------------------------------------
// Method: Focus
//
// This is usually called by one of the child views when they
// change their focus.  This way we can redraw ourself and change
// the highlighting of the label.
//--------------------------------------------------------------------

void TContentView::Focus(bool focus)
{
	BRect	r;

	if (fFocus != focus) {
		r = Frame();
		fFocus = focus;
		Draw(r);
	}
}

//--------------------------------------------------------------------
// Method: FrameText
//
// Draws a sunken bezel leading down to the scrollview that contains
// the text of the message.
//--------------------------------------------------------------------

void TContentView::FrameText(void)
{
	BRect	r;

	r = Bounds();
	if (!fWidth)
		fWidth = r.Width();

	// Draw white line
	SetHighColor(255, 255, 255);
	StrokeLine(BPoint(r.left + MESSAGE_FIELD_H - 2, r.bottom - 8),
			   BPoint(r.right - 7, r.bottom - 8));
	StrokeLine(BPoint(r.right - 7, r.bottom - 8),
			   BPoint(r.right - 7, r.top + 8));
	StrokeLine(BPoint(r.left + MESSAGE_FIELD_H - 2, r.bottom - 9),
			   BPoint(r.right - 7, r.bottom - 9));

	// Draw medium gray
	SetHighColor(120, 120, 120);
	StrokeLine(BPoint(r.left + MESSAGE_FIELD_H - 2,
					  r.top + MESSAGE_FIELD_V - 3),
			   BPoint(r.right - 8,
					  r.top + MESSAGE_FIELD_V - 3));
	StrokeLine(BPoint(r.left + MESSAGE_FIELD_H - 2,
					  r.top + MESSAGE_FIELD_V - 2),
			   BPoint(r.left + MESSAGE_FIELD_H - 2,
					  r.bottom - 9));
	
	// Draw very light gray
	SetHighColor(176, 176, 176);
	StrokeLine(BPoint(r.left + MESSAGE_FIELD_H - 1,
					  r.top + MESSAGE_FIELD_V - 2),
			   BPoint(r.right - 9,
					  r.top + MESSAGE_FIELD_V - 2));
	StrokeLine(BPoint(r.left + MESSAGE_FIELD_H - 1,
					  r.top + MESSAGE_FIELD_V - 1),
			   BPoint(r.left + MESSAGE_FIELD_H - 1,
					  r.bottom - MESSAGE_FIELD_V - 1));
}

//--------------------------------------------------------------------
// Method: Resize
//
// When resizing, try to refresh the areas of the view that
// are newly exposed.
//--------------------------------------------------------------------

void TContentView::ReSize(void)
{
	float	width;
	BRect	r;

	r = Bounds();
	if (fWidth != (width = Window()->Frame().Width())) {
		if (fWidth < width) {
			SetHighColor(VIEW_COLOR, VIEW_COLOR, VIEW_COLOR);
			StrokeLine(BPoint(fWidth, 1),
					   BPoint(fWidth, r.bottom - 2));
		}
		else {
			SetHighColor(120, 120, 120);
			StrokeLine(BPoint(width, 1),
					   BPoint(width, r.bottom - 2));
			SetHighColor(VIEW_COLOR, VIEW_COLOR, VIEW_COLOR);
			r.left = r.right - 9;
			r.top++;
			r.right--;
			r.bottom--;
			FillRect(r);
			r.right = r.left;
			r.top = r.bottom - 8;
			r.left = MESSAGE_FIELD_H - 5;
			FillRect(r);
		}
		fWidth = width;
		FrameText();
	}
}


//====================================================================
// Class: TTextView
//
// A TTextView is stuck into a scrollable view in the content area.
// This view is responsible for the loading of a text message and 
// displaying its contents.  It is also used to type in new messages
// and supports a small amount of keyboard navigation.
//====================================================================

TTextView::TTextView(BRect frame, BRect text, bool incoming, BRecord *record,
					  TContentView *view, char *font, long size)
		  :BTextView(frame, "", text, B_FOLLOW_ALL, B_WILL_DRAW |
													B_FRAME_EVENTS |
													B_NAVIGABLE)
{
	fIncoming = incoming;
	fRecord = record;
	fParent = view;
	strcpy(fFontName, font);
	fFontSize = size;
	fReady = FALSE;
	fLastPosition = -1;
	fYankBuffer = NULL;
	fStopSem = create_sem(1, "reader_sem");
	fThread = NULL;
}

//--------------------------------------------------------------------
// Method: Destructor
//
// Simply clean up data and get rid of the semaphore.
//--------------------------------------------------------------------

TTextView::~TTextView(void)
{
	if (fYankBuffer)
		free(fYankBuffer);
	delete_sem(fStopSem);
}

//------------------------------------------------------------------
// Method: AttachedToWindow
//
// When we're attached to the window, we'll setup the font and if
// we already have a record, we'll try to display it.
//------------------------------------------------------------------

void TTextView::AttachedToWindow(void)
{
	BTextView::AttachedToWindow();
	SetFontName(fFontName);
	SetFontSize(fFontSize);
	if (fRecord) {
		LoadMessage(fRecord, FALSE, FALSE, NULL);
		if (fIncoming)
			MakeEditable(FALSE);
	}
}

//------------------------------------------------------------------
// Method: KeyDown
//
// Do some simple edit maneuvering.  This intercepting the keydowns
// allows us to do some moving around in the displayed text.
//------------------------------------------------------------------

void TTextView::KeyDown(ulong key)
{
	bool	up = FALSE;
	char	new_line = '\n';
	long	end;
	long	height;
	long 	start;
	ulong	mods;
	BRect	r;

	mods = Window()->CurrentMessage()->FindLong("modifiers");

	switch (key) {
		case B_HOME:
			if (mods & B_CONTROL_KEY)	// ^a - start of line
				GoToLine(CurrentLine());
			else {
				Select(0, 0);
				ScrollToSelection();
			}
			break;

		case 0x02:						// ^b - back 1 char
			GetSelection(&start, &end);
			start--;
			if (start >= 0) {
				Select(start, start);
				ScrollToSelection();
			}
			break;

		case B_END:
		case B_DELETE:
			if ((key == B_DELETE) || (mods & B_CONTROL_KEY)) {	// ^d
				if (IsEditable()) {
					GetSelection(&start, &end);
					if (start != end)
						Delete();
					else {
						Select(start, start + 1);
						Delete();
					}
				}
			}
			else
				Select(TextLength(), TextLength());
			ScrollToSelection();
			break;

		case 0x05:						// ^e - end of line
			if (mods & B_CONTROL_KEY) {
				GoToLine(CurrentLine() + 1);
				GetSelection(&start, &end);
				Select(start - 1, start - 1);
			}
			break;

		case 0x06:						// ^f - forward 1 char
			GetSelection(&start, &end);
			if (end > start)
				start = end;
			Select(start + 1, start + 1);
			ScrollToSelection();
			break;

		case 0x0e:						// ^n - next line
			GoToLine(CurrentLine() + 1);
			ScrollToSelection();
			break;

		case 0x0f:						// ^o - open line
			if (IsEditable()) {
				GetSelection(&start, &end);
				Delete();
				Insert(&new_line, 1);
				Select(start, start);
				ScrollToSelection();
			}
			break;

		case B_PAGE_UP:
			if (mods & B_CONTROL_KEY) {	// ^k kill text from cursor to e-o-line
				if (IsEditable()) {
					GetSelection(&start, &end);
					if ((start != fLastPosition) && (fYankBuffer)) {
						free(fYankBuffer);
						fYankBuffer = NULL;
					}
					fLastPosition = start;
					GoToLine(CurrentLine() + 1);
					GetSelection(&end, &end);
					end--;
					if (end < 0)
						break;
					if (start == end)
						end++;
					Select(start, end);
					if (fYankBuffer) {
						fYankBuffer = (char *)realloc(fYankBuffer,
									 strlen(fYankBuffer) + (end - start) + 1);
						GetText(&fYankBuffer[strlen(fYankBuffer)], start,
								end - start);
					}
					else {
						fYankBuffer = (char *)malloc(end - start + 1);
						GetText(fYankBuffer, start, end - start);
					}
					Delete();
					ScrollToSelection();
				}
				break;
			}
			else
				up = TRUE;
				// yes, fall through!

		case B_PAGE_DOWN:
			r = Bounds();
			height = (up ? r.top - r.bottom : r.bottom - r.top) - 25;
			if ((up) && (!r.top))
				break;
			ScrollBy(0, height);
			break;

		case 0x10:						// ^p goto previous line
			GoToLine(CurrentLine() - 1);
			ScrollToSelection();
			break;

		case 0x19:						// ^y yank text
			if ((IsEditable()) && (fYankBuffer)) {
				Delete();
				Insert(fYankBuffer);
				ScrollToSelection();
			}
			break;

		default:
			BTextView::KeyDown(key);
	}
}

//------------------------------------------------------------------
// Method: MessageReceived
//
// Deals with some of the messages which are headed our way.
//------------------------------------------------------------------

void TTextView::MessageReceived(BMessage *msg)
{
	char		*text;
	long		end;
	long		result;
	long		size;
	long		start;
	ulong		creator;
	ulong		type;
	BFile		*file;
	BMessage	*message;

	switch (msg->what) {
		// If the user drops a file on the view, then we'll
		// include that in the message.
		case B_SIMPLE_DATA:
			if ((!fIncoming) && (msg->HasRef("refs"))) {
				file = new BFile(msg->FindRef("refs"));
				if ((result = msg->Error()) == B_NO_ERROR) {
					file->GetTypeAndApp(&type, &creator);
					size = file->Size();
					if ((type == 'TEXT') && (size)) {
						file->Open(B_READ_ONLY);
						text = (char *)malloc(size);
						file->Read(text, size);
						file->Close();
						delete file;
						Delete();
						GetSelection(&start, &end);
						Insert(text, size);
						Select(start, start + size);
						free(text);
					}
					else if (size) {
						message = new BMessage(msg);
						Window()->PostMessage(message);
					}
				}
			}
			else
				BTextView::MessageReceived(msg);
			break;

		// This was probably generated by the menu item.  
		// Change the font accordingly
		case CHANGE_FONT:
			SetFontName(msg->FindString("name"));
			SetFontSize(msg->FindLong("size"));
			Invalidate(Bounds());
			break;

		// Show the header of the message
		case M_HEADER:
			Window()->Unlock();
			StopLoad();
			Window()->Lock();
			SetText(NULL);
			LoadMessage(fRecord, msg->FindBool("header"), FALSE, NULL);
			break;

		// Select All - from Edit menu
		case M_SELECT:
			if (IsSelectable())
				Select(0, TextLength());
			break;

		default:
			BTextView::MessageReceived(msg);
	}
}

//-----------------------------------------------------------------
// Method: MakeFocus
//
// Report changes in focus to our parent so that it has a chance
// to change its look based on that.
//-----------------------------------------------------------------

void TTextView::MakeFocus(bool focus)
{
	BTextView::MakeFocus(focus);
	fParent->Focus(focus);
}

//-----------------------------------------------------------------
// Method: LoadMessage
//
// This is the function that is spaws the thread which then
// in turn loads the content of the message.
//-----------------------------------------------------------------

void TTextView::LoadMessage(BRecord *record, bool show_header, bool quote_it,
							const char *text)
{
	reader		*info;

	MakeSelectable(FALSE);
	if (text)
		Insert(text, strlen(text));

	info = (reader *)malloc(sizeof(reader));
	info->header = show_header;
	info->quote = quote_it;
	info->incoming = fIncoming;
	info->view = this;
	info->record = record;
	info->stop_sem = &fStopSem;
	resume_thread(fThread = spawn_thread((long (*)(void *)) Reader,
							   "reader", B_DISPLAY_PRIORITY, info));
}

//--------------------------------------------------------------------
// Static Method: Reader
//
// This reads the content of the message from wherever it is located
// and displays it in the view.
//--------------------------------------------------------------------

long TTextView::Reader(reader *info)
{
	bool		from_file = FALSE;
	char		*data;
	long		data_len;
	BFile		file;
	BRecord		*record;
	record_ref	ref;

	record = new BRecord(info->record);
	
	// If all we're doing is displaying the header, then
	// we'll do that and we're done.
	if (info->header) {
		data = (char *)record->FindRaw("header", &data_len);
		if ((record->Error() == B_NO_ERROR) && (data_len))
			if (!strip_it(data, data_len, info->quote, info->view,
							info->stop_sem))
				goto done;
	}

	// Get the raw message content data from the record
	data = (char *)record->FindRaw("content", &data_len);
	if (record->Error() == B_NO_ERROR) {
		// If the data_len is zero, then the record doesn't actually
		// contain the contents of the message.  The data actually
		// lives in a file, so open that up and get the data.
		if (!data_len) {
			ref = record->Ref();
			ref.record = 0;
			ref.record = record->FindRecordID("content_file");
			if ((record->Error() == B_NO_ERROR) && (ref.record)) {
				file.SetRef(ref);
				if (file.Open(B_READ_ONLY) == B_NO_ERROR) {
					data_len = file.Size();
					data = (char *)malloc(data_len);
					if (data)
						file.Read(data, data_len);
					else
						data_len = 0;
					file.Close();
					from_file = TRUE;
				}
			}
		}
		
		// Whether the data came from a file, or directly from the database,
		// display the contents now.
		if (data_len)
			if (!strip_it(data, data_len, info->quote, info->view,
								info->stop_sem))
				goto done;

		// If the data came from a file, we allocated space for it, so we
		// should free it.  Otherwise, it will go away when the record is
		// deleted, so we wouldn't have to free it here.
		if (from_file)
			free(data);
	}

	// We're here if we had a problem looking up the content, which means
	// we are just making a new mail message perhaps.  So make the text editable
	// and selectable.
	if (get_semaphore(info->view->Window(), info->stop_sem)) {
		info->view->Select(0, 0);
		info->view->MakeSelectable(TRUE);
		if (!info->incoming)
			info->view->MakeEditable(TRUE);
		info->view->Window()->Unlock();
		release_sem(*(info->stop_sem));
	}

done:;
	delete record;
	free(info);
	return B_NO_ERROR;
}

//--------------------------------------------------------------------
// Method: StopLoad
//
// Calling this method will cause the thread that is doing the loading
// to stop.  Ultimately it is the strip_it function that exhibits this
// behavior.
//
// This works by grabbing the semaphore which the loading thread tries
// to grab every once in a while. If we hold it, then the loading thread
// can't get it, and it will simply quit.
//--------------------------------------------------------------------

void TTextView::StopLoad(void)
{
	long		result;
	thread_id	thread;
	thread_info	info;

	if ((thread = fThread) && (get_thread_info(fThread, &info) == B_NO_ERROR)) {
		acquire_sem(fStopSem);
		wait_for_thread(thread, &result);
		fThread = NULL;
		release_sem(fStopSem);
	}
}

//--------------------------------------------------------------------
// Function: get_semaphore
//
// Try to get the specified semaphore, in the context of a locked window.
//--------------------------------------------------------------------

bool get_semaphore(BWindow *window, sem_id *sem)
{
	long	result;

	if (!window->Lock())
		return FALSE;
	if (acquire_sem_etc(*sem, 1, B_TIMEOUT, 0.0) != B_NO_ERROR) {
		window->Unlock();
		return FALSE;
	}
	return TRUE;
}

//--------------------------------------------------------------------
// Function: strip_it
//
// Turns \r\n ==> \n
// Put the text into the view and put in the quote characters
// whgile we're at it.
//--------------------------------------------------------------------

bool strip_it(char* data, long data_len, bool quote, BTextView *view,
														sem_id *stop)
{
	long	loop;
	long	start = 0;

	// Read the bulk of the message.  Each line should
	// be terminated with a \r\n.  Strip off the \r because
	// BTextView doesn't want it.  Put in quote characters
	// where necessary.
	for (loop = 0; loop < data_len; loop++) {
		if ((loop) && (data[loop] == 10) && (data[loop - 1] == 13)) {
			data[loop - 1] = 10;
			
			// If we cannot get the semaphore, then stop processing.
			// This could happen if the window closes before we're done
			// or some other action interrupts the reading process.
			if (!get_semaphore(view->Window(), stop))
				return FALSE;
			
			if (quote)
				view->Insert(QUOTE, strlen(QUOTE));
			view->Insert(&data[start], loop - start);
			view->Window()->Unlock();
			data[loop - 1] = 13;
			start = loop + 1;
			release_sem(*stop);
		}
	}

	// We're here if there was a line that did not
	// terminate with a \r\n.  This will read to the
	// end of the data and put it into the text area.
	if (start < data_len) {
		if (!get_semaphore(view->Window(), stop))
			return FALSE;
		if (quote)
			view->Insert(QUOTE, strlen(QUOTE));
		view->Insert(&data[start], data_len - start);
		view->Window()->Unlock();
		release_sem(*stop);
	}

	return TRUE;
}
