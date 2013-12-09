//--------------------------------------------------------------------
//	
//	Header.cpp
//
//	Written by: Robert Polic
//	
//	Copyright 1996 Be, Inc. All Rights Reserved.
//	
//--------------------------------------------------------------------

#include <stdio.h>
#include <string.h>

#ifndef MAIL_H
#include "Mail.h"
#endif
#ifndef HEADER_H
#include "Header.h"
#endif


//====================================================================

THeaderView::THeaderView(BRect rect, TMailWindow *window, BRect wind_rect,
						 bool incoming, BRecord *record)
			:BView(rect, "", B_FOLLOW_NONE, B_WILL_DRAW)
{
	char		string[20];
	BRect		r;
	rgb_color	c;

	fWindow = window;
	fIncoming = incoming;
	fRecord = record;
	fWidth = 0;

	c.red = c.green = c.blue = VIEW_COLOR;
	SetViewColor(c);
	SetFontName("Erich");
	SetFontSize(9);
	SetDrawingMode(B_OP_OVER);

	if (fIncoming) {
		r.Set(FROM_FIELD_H, TO_FIELD_V,
			  wind_rect.Width() - (SEPERATOR_MARGIN + 1),
			  TO_FIELD_V + TO_FIELD_HEIGHT);
			sprintf(string, FROM_TEXT);
	}
	else {
		r.Set(TO_FIELD_H, TO_FIELD_V,
			  wind_rect.Width() - (SEPERATOR_MARGIN + 1),
			  TO_FIELD_V + TO_FIELD_HEIGHT);
		sprintf(string, TO_TEXT);
	}
	fTo = new TTextControl(r, string, new BMessage(TO_FIELD),
												fIncoming, fRecord);
	AddChild(fTo);

	r.left = SUBJECT_FIELD_H;
	r.top = SUBJECT_FIELD_V;
	r.bottom = SUBJECT_FIELD_V + SUBJECT_FIELD_HEIGHT;
	fSubject = new TTextControl(r, SUBJECT_TEXT, new BMessage(SUBJECT_FIELD),
												fIncoming, fRecord);
	AddChild(fSubject);

	if (!fIncoming) {
		r.Set(CC_FIELD_H, CC_FIELD_V,
			  CC_FIELD_H + CC_FIELD_WIDTH,
			  CC_FIELD_V + CC_FIELD_HEIGHT);
		fCc = new TTextControl(r, CC_TEXT, new BMessage(CC_FIELD),
												fIncoming, fRecord);
		AddChild(fCc);

		r.Set(BCC_FIELD_H, BCC_FIELD_V,
			  BCC_FIELD_H + BCC_FIELD_WIDTH,
			  BCC_FIELD_V + BCC_FIELD_HEIGHT);
		fBcc = new TTextControl(r, BCC_TEXT, new BMessage(BCC_FIELD),
												fIncoming, fRecord);
		AddChild(fBcc);
	}
}

//--------------------------------------------------------------------

void THeaderView::Draw(BRect where)
{
	BRect	r;

	r = Window()->Bounds();
	if (!fWidth)
		fWidth = r.Width();

	r.OffsetTo(0, 0);
	if (fIncoming)
		r.bottom = MIN_HEADER_HEIGHT;
	else
		r.bottom = HEADER_HEIGHT;
	SetHighColor(255, 255, 255);
	StrokeLine(BPoint(r.left, r.top), BPoint(r.right, r.top));
	StrokeLine(BPoint(r.left, r.top), BPoint(r.left, r.bottom - 1));
	SetHighColor(120, 120, 120);
	StrokeLine(BPoint(r.left, r.bottom - 1), BPoint(r.right, r.bottom - 1));
	StrokeLine(BPoint(r.right, r.bottom - 1), BPoint(r.right, r.top + 1));
	SetHighColor(64, 64, 64);
	StrokeLine(BPoint(r.left, r.bottom), BPoint(r.right, r.bottom));

	SetHighColor(255, 255, 255);
	r = ((BTextView *)(fSubject->ChildAt(0)))->Frame();
	fSubject->ConvertToParent(&r);
	r.left -= 3;
	r.right += 3;
	r.top -= 3;
	if (r.Intersects(where)) {
		StrokeLine(BPoint(r.left, r.bottom + 2),
				   BPoint(r.right, r.bottom + 2));
		StrokeLine(BPoint(r.right, r.bottom + 1),
				   BPoint(r.right, r.top));
	}

	r = ((BTextView *)(fTo->ChildAt(0)))->Frame();
	fTo->ConvertToParent(&r);
	r.left -= 3;
	r.right += 3;
	r.top -= 3;
	if (r.Intersects(where)) {
		StrokeLine(BPoint(r.left, r.bottom + 2),
				   BPoint(r.right, r.bottom + 2));
		StrokeLine(BPoint(r.right, r.bottom + 1),
				   BPoint(r.right, r.top));
	}

	if (!fIncoming) {
		r = ((BTextView *)(fCc->ChildAt(0)))->Frame();
		fCc->ConvertToParent(&r);
		r.left -= 3;
		r.right += 3;
		r.top -= 3;
		if (r.Intersects(where)) {
			StrokeLine(BPoint(r.left, r.bottom + 2),
					   BPoint(r.right, r.bottom + 2));
			StrokeLine(BPoint(r.right, r.bottom + 1),
					   BPoint(r.right, r.top));
		}

		r = ((BTextView *)(fBcc->ChildAt(0)))->Frame();
		fBcc->ConvertToParent(&r);
		r.left -= 3;
		r.right += 3;
		r.top -= 3;
		if (r.Intersects(where)) {
			StrokeLine(BPoint(r.left, r.bottom + 2),
					   BPoint(r.right, r.bottom + 2));
			StrokeLine(BPoint(r.right, r.bottom + 1),
					   BPoint(r.right, r.top));
		}
	}
}

//--------------------------------------------------------------------

void THeaderView::ReSize(void)
{
	long		height;
	float		width;

	if (fIncoming)
		height = MIN_HEADER_HEIGHT;
	else
		height = HEADER_HEIGHT;

	if (fWidth != (width = Window()->Frame().Width())) {
		if (fWidth < width) {
			SetHighColor(VIEW_COLOR, VIEW_COLOR, VIEW_COLOR);
			StrokeLine(BPoint(fWidth, 1),
					   BPoint(fWidth, height - 2));
		}
		else {
			SetHighColor(120, 120, 120);
			StrokeLine(BPoint(width, 1),
					   BPoint(width, height - 2));
		}
		fWidth = width;
	}
}


//====================================================================

TTextControl::TTextControl(BRect rect, char *label, BMessage *msg,
									bool incoming, BRecord *record)
			 :BTextControl(rect, "", label, "", msg, B_FOLLOW_NONE)
{
	strcpy(fLabel, label);
	fCommand = msg->what;
	fRecord = record;
	fIncoming = incoming;
}

//--------------------------------------------------------------------

void TTextControl::AttachedToWindow(void)
{
	const char	*string;
	BTextView	*text;

	SetHighColor(0, 0, 0);
	BTextControl::AttachedToWindow();
	SetFontName("Erich");

	SetDivider(StringWidth(fLabel) + 6);
	text = (BTextView *)ChildAt(0);
	text->SetFontName("Erich");

	if (fRecord) {
		if (fIncoming)
			SetEnabled(FALSE);
		switch (fCommand) {
			case SUBJECT_FIELD:
				string = fRecord->FindString("Subject");
				if (fRecord->Error() == B_NO_ERROR)
					SetText(string);
				break;

			case TO_FIELD:
				string = fRecord->FindString("Name");
				if (fRecord->Error() == B_NO_ERROR)
					SetText(string);
				break;
		}
	}
}

//--------------------------------------------------------------------

void TTextControl::Draw(BRect where)
{
	BRect		r;
	BTextView	*text;

	SetHighColor(0, 0, 0);
	BTextControl::Draw(where);

	text = (BTextView *)ChildAt(0);
	r = text->Frame();
	r.InsetBy(-3, -3);
	
	SetHighColor(120, 120, 120);
	StrokeLine(BPoint(r.left, r.top), BPoint(r.left, r.bottom - 1));
	StrokeLine(BPoint(r.left + 1, r.top), BPoint(r.right - 1, r.top));
}

//--------------------------------------------------------------------

void TTextControl::MessageReceived(BMessage *msg)
{
	bool		handled = FALSE;
	char		separator[4] = ", ";
	const char*	address;
	long		item = 0;
	long		len;
	long		result;
	BRecord		*record;
	BTextView	*text_view;
	record_ref	ref;

	switch (msg->what) {
		case B_SIMPLE_DATA:
			if (fCommand != SUBJECT_FIELD) {
				if ((!fIncoming) && (msg->HasRef("refs"))) {
					do {
						ref = msg->FindRef("refs", item++);
						if ((result = msg->Error()) == B_NO_ERROR) {
							record = new BRecord(ref);
							if (strcmp(record->Table()->Name(), "Person") == 0) {
								address = record->FindString("E-mail");
								if (strlen(address)) {
									text_view = (BTextView *)ChildAt(0);
									if (len = text_view->TextLength()) {
										text_view->Select(len, len);
										text_view->Insert(separator);
									}
									text_view->Insert(address);
								}
							}
							delete record;
						}
					} while (result == B_NO_ERROR);
				}
			}
			break;

		case M_SELECT:
			text_view = (BTextView *)ChildAt(0);
			text_view->Select(0, text_view->TextLength());
			break;

		default:
			BTextControl::MessageReceived(msg);
	}
}
