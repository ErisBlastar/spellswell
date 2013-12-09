//--------------------------------------------------------------------
//	
//	Signature.cpp
//
//	Written by: Robert Polic
//	
//	Copyright 1996 Be, Inc. All Rights Reserved.
//	
//--------------------------------------------------------------------

#include <Debug.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#ifndef MAIL_H
#include "Mail.h"
#endif
#ifndef SIGNATURE_H
#include "Signature.h"
#endif

extern long			level;
extern BTable		*sig_table;


//====================================================================

TSignatureWindow::TSignatureWindow(BRect rect, char *font, long size)
				 :BWindow(rect, "Signatures", B_TITLED_WINDOW, 0)
{
	float		height;
	BMenu		*menu;
	BMenuBar	*menu_bar;
	BMenuItem	*item;
	BRect		r;

	fRecord = NULL;
	r.Set(0, 0, 32767, 15);
	menu_bar = new BMenuBar(r, "");
	menu = new BMenu("Signature");
	menu->AddItem(new BMenuItem("New", new BMessage(M_NEW)));
	fSignature = new TMenu("Open");
	menu->AddItem(new BMenuItem(fSignature));
	menu->AddSeparatorItem();
	menu->AddItem(fSave = new BMenuItem("Save", new BMessage(M_SAVE), 'S'));
	menu->AddItem(fDelete = new BMenuItem("Delete", new BMessage(M_DELETE), 'T'));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Close", new BMessage(B_CLOSE_REQUESTED), 'W'));
	menu_bar->AddItem(menu);

	menu = new BMenu("Edit");
	menu->AddItem(fUndo = new BMenuItem("Undo", new BMessage(M_UNDO), 'Z'));
	menu->AddSeparatorItem();
	menu->AddItem(fCut = new BMenuItem("Cut", new BMessage(B_CUT), 'X'));
	fCut->SetTarget(this, TRUE);
	menu->AddItem(fCopy = new BMenuItem("Copy", new BMessage(B_COPY), 'C'));
	fCopy->SetTarget(this, TRUE);
	menu->AddItem(fPaste = new BMenuItem("Paste", new BMessage(B_PASTE), 'V'));
	fPaste->SetTarget(this, TRUE);
	menu->AddItem(item = new BMenuItem("Select All",
								new BMessage(M_SELECT), 'A'));
	item->SetTarget(this, TRUE);
	menu_bar->AddItem(menu);

	Lock();
	AddChild(menu_bar);
	height = menu_bar->Bounds().bottom + 1;
	Unlock();

	r.top = height;
	r.right = rect.Width();
	r.bottom = rect.Height();
	fSigView = new TSignatureView(r, this, font, size);

	Lock();
	AddChild(fSigView);
	Unlock();
}

//--------------------------------------------------------------------

void TSignatureWindow::FrameResized(float width, float height)
{
	fSigView->ReSize();
}

//--------------------------------------------------------------------

void TSignatureWindow::MenusWillShow(void)
{
	bool		enable = FALSE;
	long		finish = 0;
	long		len;
	long		start = 0;
	BTextView	*text_view;

	if (fRecord)
		fDelete->SetEnabled(TRUE);
	else
		fDelete->SetEnabled(FALSE);

	if (IsDirty())
		fSave->SetEnabled(TRUE);
	else
		fSave->SetEnabled(FALSE);

	fUndo->SetEnabled(FALSE);		// ***TODO***
	text_view = (BTextView *)fSigView->fName->ChildAt(0);
	if (text_view->IsFocus())
		text_view->GetSelection(&start, &finish);
	else
		fSigView->fTextView->GetSelection(&start, &finish);

	if (start != finish)
		enable = TRUE;
	fCut->SetEnabled(enable);
	fCopy->SetEnabled(enable);

	enable = FALSE;
	be_clipboard->Lock();
	be_clipboard->FindText(&len);
	be_clipboard->Unlock();
	if (len)
		enable = TRUE;
	fPaste->SetEnabled(enable);
}

//--------------------------------------------------------------------

void TSignatureWindow::MessageReceived(BMessage* msg)
{
	const char	*sig;
	long		len;
	BQuery		query;
	BRecord		*record;
	BTextView	*text_view;

	switch(msg->what) {
		case CHANGE_FONT:
			fSigView->fTextView->SetFontName(msg->FindString("name"));
			fSigView->fTextView->SetFontSize(msg->FindLong("size"));
			fSigView->fTextView->Invalidate(fSigView->fTextView->Bounds());
			break;

		case M_NEW:
			if (Clear()) {
				fSigView->fName->SetText("");
				fSigView->fTextView->SetText(NULL, 0);
				fSigView->fName->MakeFocus(TRUE);
			}
			break;

		case M_SAVE:
			Save();
			break;

		case M_DELETE:
			if (level == L_BEGINNER) {
				beep();
				if (!(new BAlert("",
						"Are you sure you want to delete this signature?",
						"Cancel", "Delete", NULL, B_WIDTH_AS_USUAL,
						B_WARNING_ALERT))->Go())
					break;
			}
			if (fRecord) {
				fRecord->Remove();
				fRecord = NULL;
				fSigView->fName->SetText("");
				fSigView->fTextView->SetText(NULL, 0);
				fSigView->fName->MakeFocus(TRUE);
			}
			break;

		case M_SIGNATURE:
			if (Clear()) {
				query.AddTable(sig_table);
				query.PushOp(B_ALL);
				query.Fetch();
				record = new BRecord(sig_table->Database(),
							query.RecordIDAt(msg->FindLong("index")));
				if (record->Error() == B_NO_ERROR) {
					fSigView->fName->SetText(record->FindString("name"));
					sig = (const char*)record->FindRaw("signature", &len);
					fSigView->fTextView->SetText(sig, len);
				}
				fRecord = record;
				fSigView->fName->MakeFocus(TRUE);
				text_view = (BTextView *)fSigView->fName->ChildAt(0);
				text_view->Select(0, text_view->TextLength());
			}
			break;

		default:
			BWindow::MessageReceived(msg);
	}
}

//--------------------------------------------------------------------

bool TSignatureWindow::QuitRequested(void)
{
	BMessage	*message;

	if (Clear()) {
		message = new BMessage(WINDOW_CLOSED);
		message->AddLong("kind", SIG_WINDOW);
		message->AddRect("window", Frame());
		be_app->PostMessage(message);
		return TRUE;
	}
	else
		return FALSE;
}

//--------------------------------------------------------------------

void TSignatureWindow::Show(void)
{
	BTextView	*text_view;

	Lock();
	text_view = (BTextView *)fSigView->fName->ChildAt(0);
	fSigView->fName->MakeFocus(TRUE);
	text_view->Select(0, text_view->TextLength());
	Unlock();
	BWindow::Show();
}

//--------------------------------------------------------------------

bool TSignatureWindow::Clear(void)
{
	long		result;

	if (IsDirty()) {
		beep();
		result = (new BAlert("", "Save changes to signature?",
				"Don't save", "Cancel", "Save", B_WIDTH_AS_USUAL,
				B_WARNING_ALERT))->Go();
		if (result == 1)
			return FALSE;
		if (result == 2)
			Save();
	}

	delete fRecord;
	fRecord = NULL;
	return TRUE;
}

//--------------------------------------------------------------------

bool TSignatureWindow::IsDirty(void)
{
	const char	*record_text;
	const char	*view_text;
	long		record_len;
	long		view_len;

	if (fRecord) {
		if (strcmp(fRecord->FindString("name"), fSigView->fName->Text()))
			return TRUE;
		else {
			view_text = fSigView->fTextView->Text();
			view_len = fSigView->fTextView->TextLength();
			record_text = (const char*)fRecord->FindRaw("signature", &record_len);
			if ((view_len != record_len) ||
				(memcmp(view_text, record_text, view_len)))
				return TRUE;
		}
	}
	else {
		if ((strlen(fSigView->fName->Text())) ||
			(fSigView->fTextView->TextLength()))
			return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------------

void TSignatureWindow::Save(void)
{
	const char	*text;
	long		len;

	if (!fRecord)
		fRecord = new BRecord(sig_table);
	fRecord->SetString("name", fSigView->fName->Text());
	text = fSigView->fTextView->Text();
	len = fSigView->fTextView->TextLength();
	fRecord->SetRaw("signature", text, len);
	fRecord->Commit();
}


//====================================================================

TSignatureView::TSignatureView(BRect rect, TSignatureWindow *window,
								char* font, long size)
			   :BView(rect, "", B_FOLLOW_ALL, B_WILL_DRAW)
{
	BRect		r;
	BRect		text;
	rgb_color	c;

	fWindow = window;
	fFocus = FALSE;
	fWidth = 0;

	c.red = c.green = c.blue = VIEW_COLOR;
	SetViewColor(c);
	SetFontName("Erich");
	SetFontSize(9);
	SetDrawingMode(B_OP_OVER);

	r.Set(NAME_FIELD_H, NAME_FIELD_V,
		  rect.Width() - (SEPERATOR_MARGIN + 1),
		  NAME_FIELD_V + NAME_FIELD_HEIGHT);
	fName = new TNameControl(r, NAME_TEXT, new BMessage(NAME_FIELD));
	AddChild(fName);

	r = rect;
	r.OffsetTo(0, 0);
	r.left = SIG_FIELD_H;
	r.right -= (B_V_SCROLL_BAR_WIDTH + 11);
	r.top = SIG_FIELD_V;
	r.bottom -= 12;
	text = r;
	text.OffsetTo(0, 0);

	fTextView = new TSigTextView(r, text, this, font, size);
	fScroll = new TScrollView(fTextView);
	AddChild(fScroll);
}

//--------------------------------------------------------------------

void TSignatureView::Draw(BRect where)
{
	BRect	r;

	r = Bounds();
	if (!fWidth)
		fWidth = r.Width();

	SetHighColor(255, 255, 255);
	StrokeLine(BPoint(r.left, r.top), BPoint(r.right, r.top));
	StrokeLine(BPoint(r.left, r.top + 1), BPoint(r.left, r.bottom - 1));
	SetHighColor(120, 120, 120);
	StrokeLine(BPoint(r.right, r.top + 1), BPoint(r.right, r.bottom));
	StrokeLine(BPoint(r.right - 1, r.bottom), BPoint(r.left, r.bottom));
	FrameText();

	SetHighColor(255, 255, 255);
	r = ((BTextView *)(fName->ChildAt(0)))->Frame();
	fName->ConvertToParent(&r);
	r.left -= 3;
	r.right += 3;
	r.top -= 3;
	if (r.Intersects(where)) {
		StrokeLine(BPoint(r.left, r.bottom + 2), BPoint(r.right, r.bottom + 2));
		StrokeLine(BPoint(r.right, r.bottom + 1), BPoint(r.right, r.top));
	}

	SetHighColor(0, 0, 0);
	SetFontName("Erich");
	SetFontSize(9);
	MovePenTo(SIG_TEXT_H, SIG_TEXT_V);
	DrawString(SIG_TEXT);
	if (!fFocus)
		SetHighColor(VIEW_COLOR, VIEW_COLOR, VIEW_COLOR);
	StrokeLine(BPoint(SIG_TEXT_H, SIG_TEXT_V + 2),
			   BPoint(SIG_TEXT_H + StringWidth(SIG_TEXT), SIG_TEXT_V + 2));
}

//--------------------------------------------------------------------

void TSignatureView::Focus(bool focus)
{
	BRect	r;

	if (fFocus != focus) {
		r = Frame();
		fFocus = focus;
		Draw(r);
	}
}

//--------------------------------------------------------------------

void TSignatureView::FrameText(void)
{
	BRect	r;

	r = Bounds();
	if (!fWidth)
		fWidth = r.Width();

	SetHighColor(255, 255, 255);
	StrokeLine(BPoint(r.left + SIG_FIELD_H - 2, r.bottom - 8),
			   BPoint(r.right - 7, r.bottom - 8));
	StrokeLine(BPoint(r.right - 7, r.bottom - 8),
			   BPoint(r.right - 7, r.top + SIG_FIELD_V));
	StrokeLine(BPoint(r.left + SIG_FIELD_H - 2, r.bottom - 9),
			   BPoint(r.right - 7, r.bottom - 9));

	SetHighColor(120, 120, 120);
	StrokeLine(BPoint(r.left + SIG_FIELD_H - 2, r.top + SIG_FIELD_V - 3),
			   BPoint(r.right - 8, r.top + SIG_FIELD_V - 3));
	StrokeLine(BPoint(r.left + SIG_FIELD_H - 2, r.top + SIG_FIELD_V - 2),
			   BPoint(r.left + SIG_FIELD_H - 2, r.bottom - 9));
	SetHighColor(176, 176, 176);
	StrokeLine(BPoint(r.left + SIG_FIELD_H - 1, r.top + SIG_FIELD_V - 2),
			   BPoint(r.right - 9, r.top + SIG_FIELD_V - 2));
	StrokeLine(BPoint(r.left + SIG_FIELD_H - 1, r.top + SIG_FIELD_V - 1),
			   BPoint(r.left + SIG_FIELD_H - 1, r.bottom - 8 - 1));
}

//--------------------------------------------------------------------

void TSignatureView::ReSize(void)
{
	float	width;
	BRect	r;

	r = Bounds();
	if (fWidth != (width = Window()->Frame().Width())) {
		if (fWidth < width) {
			SetHighColor(VIEW_COLOR, VIEW_COLOR, VIEW_COLOR);
			StrokeLine(BPoint(fWidth, 1), BPoint(fWidth, r.bottom - 2));
		}
		else {
			SetHighColor(120, 120, 120);
			StrokeLine(BPoint(width, 1), BPoint(width, r.bottom - 2));
			StrokeLine(BPoint(0, r.bottom - 1),
					   BPoint(width, r.bottom - 1));
			SetHighColor(VIEW_COLOR, VIEW_COLOR, VIEW_COLOR);
			r.left = r.right - 9;
			r.top++;
			r.right--;
			r.bottom--;
			FillRect(r);
			r.right = r.left;
			r.top = r.bottom - 8;
			r.left = 0;
			FillRect(r);
		}
		fWidth = width;
		FrameText();
	}
}


//====================================================================

TNameControl::TNameControl(BRect rect, char *label, BMessage *msg)
			 :BTextControl(rect, "", label, "", msg, B_FOLLOW_NONE)
{
	strcpy(fLabel, label);
}

//--------------------------------------------------------------------

void TNameControl::AttachedToWindow(void)
{
	const char	*string;
	BTextView	*text;

	SetHighColor(0, 0, 0);
	BTextControl::AttachedToWindow();
	SetFontName("Erich");

	SetDivider(StringWidth(fLabel) + 6);
	text = (BTextView *)ChildAt(0);
	text->SetFontName("Erich");
}

//--------------------------------------------------------------------

void TNameControl::Draw(BRect where)
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

void TNameControl::MessageReceived(BMessage *msg)
{
	BTextView	*text_view;

	switch (msg->what) {
		case M_SELECT:
			text_view = (BTextView *)ChildAt(0);
			text_view->Select(0, text_view->TextLength());
			break;

		default:
			BTextControl::MessageReceived(msg);
	}
}


//====================================================================

TSigTextView::TSigTextView(BRect frame, BRect text, TSignatureView *view,
							char *font, long size)
			 :BTextView(frame, "", text, B_FOLLOW_ALL, B_WILL_DRAW |
													   B_FRAME_EVENTS |
													   B_NAVIGABLE)
{
	fParent = view;
	strcpy(fFontName, font);
	fFontSize = size;
}

//--------------------------------------------------------------------

void TSigTextView::AttachedToWindow(void)
{
	BTextView::AttachedToWindow();
	SetFontName(fFontName);
	SetFontSize(fFontSize);
}

//--------------------------------------------------------------------

void TSigTextView::KeyDown(ulong key)
{
	bool	up = FALSE;
	long	height;
	BRect	r;

	switch (key) {
		case B_HOME:
			Select(0, 0);
			ScrollToSelection();
			break;

		case B_END:
			Select(TextLength(), TextLength());
			ScrollToSelection();
			break;

		case B_PAGE_UP:
			up = TRUE;
		case B_PAGE_DOWN:
			r = Bounds();
			height = (up ? r.top - r.bottom : r.bottom - r.top) - 25;
			if ((up) && (!r.top))
				break;
			ScrollBy(0, height);
			break;

		default:
			BTextView::KeyDown(key);
	}
}

//--------------------------------------------------------------------

void TSigTextView::MessageReceived(BMessage *msg)
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
		case B_SIMPLE_DATA:
			if (msg->HasRef("refs")) {
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

		case M_SELECT:
			if (IsSelectable())
				Select(0, TextLength());
			break;

		default:
			BTextView::MessageReceived(msg);
	}
}

//--------------------------------------------------------------------

void TSigTextView::MakeFocus(bool focus)
{
	BTextView::MakeFocus(focus);
	fParent->Focus(focus);
}
