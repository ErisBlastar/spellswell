//--------------------------------------------------------------------
//	
//	Mail.cpp
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

// MDC Word Services Begin
#include "Scripting.h"
#include "WordServices.h"		// Try using same file as MacOS!
// MDC Word Services End

#ifndef MAIL_H
#include "Mail.h"
#endif
#ifndef HEADER_H
#include "Header.h"
#endif
#ifndef CONTENT_H
#include "Content.h"
#endif
#ifndef ENCLOSURES_H
#include "Enclosures.h"
#endif
#ifndef SIGNATURE_H
#include "Signature.h"
#endif
#ifndef STATUS_H
#include "Status.h"
#endif
#ifndef UTIL_H
#include "Util.h"
#endif

// MDC Word Services Begin
#include "WSBeMail.h"
#include "WSClient.h"
// MDC Word Services End

long		level = L_BEGINNER;
BTable		*sig_table;
record_ref	save_dir;


//====================================================================

int main(void)
{	
	TMailApp	*myApp;

	myApp = new TMailApp();
	myApp->Run();

	delete myApp;
	return B_NO_ERROR;
}

//--------------------------------------------------------------------

TMailApp::TMailApp(void)
		 :BApplication('MAIL'), ScriptHandler( 0, "Foo" )
{
	char		size[4];
	int			ref;
	long		count;
	long		loop;
	BDatabase	*db;
	BMenu		*font;
	BMenu		*sub_menu;
	BMenuItem	*item;
	BPopUpMenu	*menu;
	font_name	name;

	fWindowCount = 0;
	fFontSize = 9;
	fSigWindow = NULL;
	strcpy(fFontName, "Erich");
	save_dir.record = 0;
	fMailWinPos.Set(0, 0, 0, 0);
	fSigWinPos.Set(BROWSER_WIND, TITLE_BAR_HEIGHT, BROWSER_WIND + SIG_WIDTH,
		  			TITLE_BAR_HEIGHT + SIG_HEIGHT);
	if ((ref = open("/system/settings/Mail_data", 0)) >= 0) {
		read(ref, &fMailWinPos, sizeof(BRect));
		read(ref, &level, sizeof(level));
		read(ref, fFontName, sizeof(font_name));
		read(ref, &fFontSize, sizeof(long));
		read(ref, &save_dir, sizeof(record_ref));
		read(ref, &fSigWinPos, sizeof(BRect));
		close(ref);
	}

	menu = new BPopUpMenu("BeMail", FALSE, FALSE);
	menu->AddItem(new BMenuItem("About Be\245Mail",
						new BMessage(B_ABOUT_REQUESTED)));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("New Mail Message", new BMessage(M_NEW), 'N'));
	menu->AddSeparatorItem();
	
	sub_menu = new BMenu("Preferences");
	font = new BMenu("Font");
	count = count_fonts();
	for (loop = 0; loop < count; loop++) {
		get_font_name(loop, &name);
		font->AddItem(item = new BMenuItem(name, new BMessage(M_FONT)));
		if (strcmp(fFontName, name) == 0)
			item->SetMarked(TRUE);
	}
	font->SetRadioMode(TRUE);
	sub_menu->AddItem(font);

	font = new BMenu("Size");
	for (loop = 9; loop <= 14; loop++) {
		sprintf(size, "%d", loop);
		font->AddItem(item = new BMenuItem(size, new BMessage(M_SIZE)));
		if (fFontSize == loop)
			item->SetMarked(TRUE);
	}
	font->SetRadioMode(TRUE);
	sub_menu->AddItem(font);

	fLevelMenu = new BMenu("User Level");
	fLevelMenu->SetRadioMode(TRUE);
	fLevelMenu->AddItem(item = new BMenuItem("Beginner",
						new BMessage(M_BEGINNER)));
	if (level == L_BEGINNER)
		item->SetMarked(TRUE);
	fLevelMenu->AddItem(item = new BMenuItem("Expert",
						new BMessage(M_EXPERT)));
	if (level == L_EXPERT)
		item->SetMarked(TRUE);
	sub_menu->AddItem(fLevelMenu);

	sub_menu->AddItem(item = new BMenuItem("Signatures...",
						new BMessage(M_EDIT_SIGNATURE)));
	menu->AddItem(sub_menu);

	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Quit Be\245Mail",
						new BMessage(B_QUIT_REQUESTED), 'Q'));
	SetMainMenu(menu);

	// check for existence of signature table...
	db = boot_volume().Database();
	if ((sig_table = db->FindTable("Signatures")) == NULL) {
		sig_table = db->CreateTable("Signatures");
		if (sig_table) {
			sig_table->AddStringField("name", B_INDEXED_FIELD);
			sig_table->AddRawField("signature");
		}
	}
}

//--------------------------------------------------------------------

void TMailApp::AboutRequested(void)
{
	(new BAlert("", "...by Robert Polic\n\nWord Services by Mike Crawford crawford@scruznet.com\nhttp://www.wordservices.org/", "Big Deal"))->Go();
}

//--------------------------------------------------------------------

void TMailApp::ArgvReceived(int argc, char **argv)
{
	char	*addr;
	char	*names = NULL;

	for (int i = 1; i < argc; i++) {
		if (strncmp(argv[i], "mailto:", 7) == 0) {
			addr = argv[i] + 7;
			if (!names)
				names = strdup(addr);
			else {
				names = (char *)realloc(names, strlen(names) + 1 +
										strlen(addr) + 1 + 2);
				strcat(names, ", ");
				strcat(names, addr);
			}
		}
	}

	if (names) {
		TMailWindow	*window = NewWindow(NULL, names);
		free(names);
		window->Show();
	}
}

//--------------------------------------------------------------------

void TMailApp::MessageReceived(BMessage* msg)
{
	bool		all = FALSE;
	BMenuItem	*item;
	BMessage	*message;
	BRecord		*record;
	TMailWindow	*window;
	TMailWindow	*src_window = NULL;

	switch (msg->what) {
		case M_NEW:
			if (msg->HasObject("window")) {
				src_window = (TMailWindow *)msg->FindObject("window");
				if (!src_window->Lock())
					break;
			}
			window = NewWindow();
			if (msg->HasObject("record")) {
				window->Lock();
				record = (BRecord *)msg->FindObject("record");
				if ((msg->FindLong("type") == M_REPLY) ||
					(all = (msg->FindLong("type") == M_REPLY_ALL)))
					window->Reply(record, src_window, all);
				else
					window->Forward(record, src_window);
				window->Unlock();
			}
			window->Show();
			if (src_window)
				src_window->Unlock();
			break;

		case M_EDIT_SIGNATURE:
			if (fSigWindow)
				fSigWindow->Activate(TRUE);
			else {
				fSigWindow = new TSignatureWindow(fSigWinPos, fFontName,
													fFontSize);
				fSigWindow->Show();
			}
			break;

		case M_FONT:
			item = (BMenuItem *)msg->FindObject("source");
			if ((item) && (strcmp(fFontName, item->Label()))) {
				strcpy(fFontName, item->Label());
				FontChange();
			}
			break;

		case M_SIZE:
			if (fFontSize != msg->FindLong("index") + 9) {
				fFontSize = msg->FindLong("index") + 9;
				FontChange();
			}
			break;

		case M_BEGINNER:
		case M_EXPERT:
			level = msg->what - M_BEGINNER;
			break;

		case REFS_RECEIVED:
			if (msg->HasObject("window")) {
				window = (TMailWindow *)msg->FindObject("window");
				message = new BMessage(msg);
				window->PostMessage(message, window);
			}
			be_app->CloseFilePanel();
			break;

		case WINDOW_CLOSED:
			switch (msg->FindLong("kind")) {
				case MAIL_WINDOW:
					fWindowCount--;
					if ((!fWindowCount) && (msg->HasRect("window")))
						fMailWinPos = msg->FindRect("window");
					break;

				case SIG_WINDOW:
					fSigWindow = NULL;
					if (msg->HasRect("window"))
						fSigWinPos = msg->FindRect("window");
					break;
			}

			if ((!fWindowCount) && (!fSigWindow))
				be_app->PostMessage(B_QUIT_REQUESTED);
			break;

		case B_REFS_RECEIVED:
			RefsReceived(msg);
			break;

		default:
			// BApplication::MessageReceived(msg);
			// MDC Word Services Begin
			if (!TryScriptMessage(msg, this)){
				BApplication::MessageReceived(msg);
			}
			break;
			// MDC Word Services End

	}
}

//--------------------------------------------------------------------

bool TMailApp::QuitRequested(void)
{
	int			ref;

	if (BApplication::QuitRequested()) {
		mkdir("/system/settings", 0777);
		if ((ref = open("/system/settings/Mail_data", O_CREAT | O_TRUNC)) >= 0) {
			write(ref, &fMailWinPos, sizeof(BRect));
			write(ref, &level, sizeof(level));
			write(ref, fFontName, sizeof(font_name));
			write(ref, &fFontSize, sizeof(long));
			write(ref, &save_dir, sizeof(record_ref));
			write(ref, &fSigWinPos, sizeof(BRect));
			close(ref);
		}
	}
	return TRUE;
}

//--------------------------------------------------------------------

void TMailApp::ReadyToRun(void)
{
	TMailWindow	*window;

	if (!fWindowCount) {
		window = NewWindow();
		window->Show();
	}
}

//--------------------------------------------------------------------

void TMailApp::RefsReceived(BMessage *msg)
{
	char		*names;
	const char	*address;
	short		count = 0;
	long		item = 0;
	ulong		type;
	BMessage	*message;
	BRecord		*record;
	TMailWindow	*window;

	while (msg->HasRef("refs", item)) {
		record = new BRecord(msg->FindRef("refs", item));
		type = record->FindLong("fsType");
		if ((record->Error() == B_NO_ERROR) && (type == B_MAIL_TYPE)) {
			if (window = FindWindow(msg->FindRef("refs", item))) {
				window->Activate(TRUE);
				delete record;
			}
			else {
				window = NewWindow(record);
				window->Show();
			}
		}
		else {
			if (type == 'PERS') {
				if (strcmp(record->Table()->Name(), "Person") == 0) {
					address = record->FindString("E-mail");
					if (strlen(address)) {
						if (!count) {
							names = (char *)malloc(strlen(address) + 1);
							strcpy(names, address);
						}
						else {
							names = (char *)realloc(names, strlen(names) + 1 +
													strlen(address) + 1 + 2);
							strcat(names, ", ");
							strcat(names, address);
						}
						count++;
					}
				}
			}
			delete record;
		}
		item++;
	}

	if (count) {
		window = NewWindow(NULL, names);
		free(names);
		window->Show();
	}
}

//--------------------------------------------------------------------

TMailWindow* TMailApp::FindWindow(record_ref ref)
{
	long		index = 0;
	TMailWindow	*window;

	while (window = (TMailWindow *)WindowAt(index++)) {
		if ((window->fRecord) && (window->fRecord->Ref() == ref))
			return window;
	}
	return NULL;
}

//--------------------------------------------------------------------

void TMailApp::FontChange(void)
{
	long		index = 0;
	BMessage	*message;
	BWindow		*window;

	for (;;) {
		message = new BMessage(CHANGE_FONT);
		message->AddString("name", fFontName);
		message->AddLong("size", fFontSize);

		window = WindowAt(index++);
		if (!window)
			break;
		window->PostMessage(message);
	}
}

//--------------------------------------------------------------------

TMailWindow* TMailApp::NewWindow(BRecord *record, char *to)
{
	char			*title = NULL;
	const char		*str1;
	const char		*str2;
	long			len;
	BPoint			win_pos;
	BRect			r;
	TMailWindow		*window;
	screen_info 	screen_info;

	get_screen_info(0, &screen_info);
	if ((fMailWinPos.Width()) && (fMailWinPos.Height()))
		r = fMailWinPos;
	else
		r.Set(BROWSER_WIND, TITLE_BAR_HEIGHT,
			  BROWSER_WIND + WIND_WIDTH, TITLE_BAR_HEIGHT + WIND_HEIGHT);
	r.OffsetBy(fWindowCount * 20, fWindowCount * 20);
	if ((r.left - 6) < screen_info.frame.left)
		r.OffsetTo(screen_info.frame.left + 8, r.top);
	if ((r.left + 20) > screen_info.frame.right)
		r.OffsetTo(BROWSER_WIND, r.top);
	if ((r.top - 26) < screen_info.frame.top)
		r.OffsetTo(r.left, screen_info.frame.top + 26);
	if ((r.top + 20) > screen_info.frame.bottom)
		r.OffsetTo(r.left, TITLE_BAR_HEIGHT);
	if (r.Width() < WIND_WIDTH)
		r.right = r.left + WIND_WIDTH;
	fWindowCount++;

	if (record) {
		record->FindRaw("header", &len);
		if (len) {
			str1 = record->FindString("Name");
			str2 = record->FindString("Subject");
			title = (char *)malloc(strlen(str1) + strlen(str2) + 3);
			sprintf(title, "%s->%s", str1, str2);
		}
	}
	if (!title) {
		title = (char *)malloc(strlen("Be\245Mail"));
		sprintf(title, "Be\245Mail");
	}
	window = new TMailWindow(r, title, record, to, fFontName, fFontSize);
	free(title);
	return window;
}


//====================================================================

// MDC Word Services Begin
TMailWindow::TMailWindow(BRect rect, char *title, BRecord *record, char *to,
						 char *font, long size)
			:BWindow(rect, title, B_TITLED_WINDOW, 0)
// MDC Word Services End
{
	void		*header;
	bool		enclosures = TRUE;
	char		status[272];
	long		len;
	long		message;
	float		height;
	BMenu		*menu;
	BMenu		*sub_menu;
	BMenuBar	*menu_bar;
	BMenuItem	*item;
	BRect		r;

	fDelete = FALSE;
	fReplying = FALSE;
	if (fRecord = record) {
		fIncoming = (fRecord->FindLong("mail_flags") == 0);
		header = fRecord->FindRaw("header", &len);
		if (!len)
			fIncoming = FALSE;	// browser started us by creating a new record
		if (!fRecord->FindLong("Enclosures"))
			enclosures = FALSE;
	}
	else
		fIncoming = FALSE;

	r.Set(0, 0, 32767, 15);
	menu_bar = new BMenuBar(r, "");
	menu = new BMenu("Message");
	if (fIncoming) {
		menu->AddItem(new BMenuItem("Reply To Sender",
						new BMessage(M_REPLY), 'R'));
		menu->AddItem(new BMenuItem("Reply To All",
						new BMessage(M_REPLY_ALL), 'R', B_SHIFT_KEY));
		menu->AddItem(new BMenuItem("Forward", new BMessage(M_FORWARD), 'F'));
		menu->AddSeparatorItem();
		menu->AddItem(new BMenuItem("Delete", new BMessage(M_DELETE), 'T'));
		menu->AddItem(fHeader = new BMenuItem("Show Header",
								new BMessage(M_HEADER), 'H'));
	}
	else {
		menu->AddItem(fSendNow = new BMenuItem("Send Now",
								new BMessage(M_SEND_NOW), 'M', B_SHIFT_KEY));
		menu->AddItem(fSendLater = new BMenuItem("Send Later",
								new BMessage(M_SEND_LATER), 'M'));
	}
	menu->AddSeparatorItem();
	menu->AddItem(fSave = new BMenuItem("Save as Text...",
								new BMessage(M_SAVE), 'S'));
	menu->AddItem(fPrint = new BMenuItem("Print...",
								new BMessage(M_PRINT), 'P'));
	menu->AddSeparatorItem();
	if (fIncoming) {
		sub_menu = new BMenu("Close");
		if (strcmp(fRecord->FindString("Status"), "New") == 0) {
			sub_menu->AddItem(item = new BMenuItem("Set To 'Read'",
							new BMessage(M_CLOSE_READ), 'W'));
			sub_menu->AddItem(item = new BMenuItem("Leave As 'New'",
							new BMessage(M_CLOSE_SAME), 'W', B_SHIFT_KEY));
			message = M_CLOSE_READ;
		}
		else {
			sprintf(status, "Leave As '%s'", fRecord->FindString("Status"));
			sub_menu->AddItem(item = new BMenuItem(status,
							new BMessage(M_CLOSE_SAME), 'W'));
			message = M_CLOSE_SAME;
		}
		sub_menu->AddItem(item = new BMenuItem("Set To...",
							new BMessage(M_CLOSE_CUSTOM), 'W',
							B_SHIFT_KEY | B_CONTROL_KEY));
		menu->AddItem(new BMenuItem(sub_menu, new BMessage(message)));
	}
	else
		menu->AddItem(new BMenuItem("Close",
								new BMessage(B_CLOSE_REQUESTED), 'W'));
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
	if (!fIncoming) {
		menu->AddSeparatorItem();
		menu->AddItem(fQuote = new BMenuItem("Quote", new BMessage(M_QUOTE),
								B_RIGHT_ARROW));
		menu->AddItem(fRemoveQuote = new BMenuItem("Remove Quote",
								new BMessage(M_REMOVE_QUOTE), B_LEFT_ARROW));
		fSignature = new TMenu("Add Signature");
		menu->AddItem(new BMenuItem(fSignature));
	}
	menu_bar->AddItem(menu);

	menu = new BMenu("Enclosures");
	menu->AddItem(fAdd = new BMenuItem("Add...", new BMessage(M_ADD), 'O'));
	if (fIncoming)
		menu->AddItem(fRemove = new BMenuItem("Move to Trash",
									new BMessage(M_REMOVE), 'D'));
	else
		menu->AddItem(fRemove = new BMenuItem("Remove",
									new BMessage(M_REMOVE), 'D'));
	menu->AddItem(fInfo = new BMenuItem("Info...", new BMessage(M_INFO), 'I'));
	if ((fIncoming) && (!enclosures))
		menu->SetEnabled(FALSE);
	menu_bar->AddItem(menu);
	
	// MDC Word Services Begin
	menu = new WSClientMenu( "Services", M_WORD_SERVICES, this );
	if ( menu )
		menu_bar->AddItem( menu );
	// MDC Word Services End

	Lock();
	AddChild(menu_bar);
	height = menu_bar->Bounds().bottom + 1;
	Unlock();

	r.top = height;
	if (!fIncoming)
		r.bottom = height + HEADER_HEIGHT;
	else
		r.bottom = height + MIN_HEADER_HEIGHT;
	fHeaderView = new THeaderView(r, this, rect, fIncoming, fRecord);

	r = Frame();
	r.OffsetTo(0, 0);
	r.right = 32767;
	if ((fIncoming) && (!enclosures))
		r.bottom += ENCLOSURES_HEIGHT;
	r.top = r.bottom - ENCLOSURES_HEIGHT;
	fEnclosuresView = new TEnclosuresView(r, this, rect, fIncoming, fRecord);

	r.right = Frame().right - Frame().left;
	if (fIncoming)
		r.top = height + MIN_HEADER_HEIGHT;
	else
		r.top = height + HEADER_HEIGHT;
	r.bottom -= ENCLOSURES_HEIGHT;
	fContentView = new TContentView(r, this, fIncoming, fRecord, font, size);

	Lock();
	AddChild(fHeaderView);
	AddChild(fContentView);
	AddChild(fEnclosuresView);
	Unlock();

	if (to) {
		Lock();
		fHeaderView->fTo->SetText(to);
		Unlock();
	}

	SetSizeLimits(WIND_WIDTH, 32767,
				  HEADER_HEIGHT + ENCLOSURES_HEIGHT + height + 50, 32767);
}

//--------------------------------------------------------------------

TMailWindow::~TMailWindow(void)
{
	void		*header;
	char		path[512];
	long		flags;
	long		len;
	BFile		file;
	BDirectory	dir;
	record_ref	ref;

	if (fRecord) {
		if (fIncoming) {
			if (fDelete) {
				ref = fRecord->Ref();
				ref.record = 0;
				ref.record = fRecord->FindRecordID("content_file");
				fRecord->Remove();
				if (ref.record) {
					file.SetRef(ref);
					if (file.Error() == B_NO_ERROR) {
						file.GetParent(&dir);
						dir.Remove(&file);
					}
				}
			}
			else {
				flags = fRecord->FindLong("mail_flags");
				if (!flags) {
					if (strcmp(fRecord->FindString("Status"), fStatus)) {
						fRecord->SetString("Status", fStatus);
						fRecord->Commit();
					}
				}
			}
		}
		else {
			header = fRecord->FindRaw("header", &len);
			if (!len)
				fRecord->Remove();	// remove Browser created record
		}
		delete fRecord;
	}
}

//--------------------------------------------------------------------

void TMailWindow::FrameResized(float width, float height)
{
	fHeaderView->ReSize();
	fContentView->ReSize();
	fEnclosuresView->ReSize();
}

//--------------------------------------------------------------------

void TMailWindow::MenusWillShow(void)
{
	bool		enable;
	long		finish = 0;
	long		len;
	long		start = 0;
	BTextView	*text_view;

	if (!fIncoming) {
		enable = FALSE;
		if (strlen(fHeaderView->fTo->Text()))
			enable = TRUE;
		else {
			if (strlen(fHeaderView->fBcc->Text()))
				enable = TRUE;
		}
		fSendNow->SetEnabled(enable);
		fSendLater->SetEnabled(enable);

		fUndo->SetEnabled(FALSE);		// ***TODO***

		enable = FALSE;
		be_clipboard->Lock();
		be_clipboard->FindText(&len);
		be_clipboard->Unlock();
		if ((len) && (!fEnclosuresView->fList->IsFocus()))
			enable = TRUE;
		fPaste->SetEnabled(enable);

		fQuote->SetEnabled(FALSE);
		fRemoveQuote->SetEnabled(FALSE);

		fAdd->SetEnabled(TRUE);
	}
	else {
		fUndo->SetEnabled(FALSE);
		fPaste->SetEnabled(FALSE);
		fAdd->SetEnabled(FALSE);
	}

	enable = FALSE;
	if (fContentView->fTextView->TextLength())
		enable = TRUE;
	fSave->SetEnabled(enable);
	fPrint->SetEnabled(enable);

	enable = FALSE;
	text_view = (BTextView *)fHeaderView->fTo->ChildAt(0);
	if (text_view->IsFocus())
		text_view->GetSelection(&start, &finish);
	else {
		text_view = (BTextView *)fHeaderView->fSubject->ChildAt(0);
		if (text_view->IsFocus())
			text_view->GetSelection(&start, &finish);
		else {
			if (fContentView->fTextView->IsFocus()) {
				fContentView->fTextView->GetSelection(&start, &finish);
				if (!fIncoming) {
					fQuote->SetEnabled(TRUE);
					fRemoveQuote->SetEnabled(TRUE);
				}
			}
			else if (!fIncoming) {
				text_view = (BTextView *)fHeaderView->fCc->ChildAt(0);
				if (text_view->IsFocus())
					text_view->GetSelection(&start, &finish);
				else {
					text_view = (BTextView *)fHeaderView->fBcc->ChildAt(0);
					if (text_view->IsFocus())
						text_view->GetSelection(&start, &finish);
				}
			}
		}
	}
	if (start != finish)
		enable = TRUE;
	fCopy->SetEnabled(enable);
	if (fIncoming)
		enable = FALSE;
	fCut->SetEnabled(enable);

	enable = FALSE;
	if (fEnclosuresView->fList->CurrentSelection() >= 0)
		enable = TRUE;
	fRemove->SetEnabled(enable);
	fInfo->SetEnabled(FALSE);			// ***TODO***
}

//--------------------------------------------------------------------

void TMailWindow::MessageReceived(BMessage* msg)
{
	bool		enable;
	bool		header;
	bool		queue = TRUE;
	const char	*subject;
	char		title[B_FILE_NAME_LENGTH];
	long		item = 0;
	long		len;
	ulong		type;
	BMessage	*mail;
	BMessage	*message;
	BQuery		query;
	BRecord		*record;
	BRect		r;

	switch(msg->what) {
		
			// MDC Word Services Begin
		case M_WORD_SERVICES:
				
			// We can, in principle, ask to check several disjoint text blocks
			// all at once, for example several selected text fields in a drawing.
			// Each text block gets its own object specifier.  We need to pass
			// an array of object specifiers, where each object specifier is an
			// array of PropertyItems.  We can have only 1-d arrays in BMessages;
			// what we need is some kind of list data structure.  One possibility
			// is to use one flattened BMessage for each object specifier - we can
			// unflatten it to get the object specifier back.
			//
			// We create an object specifier that points to the content text field
			// of our own mail window.  We send it to the server, which will send
			// it back to us.
			
			BMessage	*paramMessage = new BMessage();
			if ( !paramMessage )
				return;

			PropertyItem	*prop;
			
			prop = new PropertyItem;
			
			if ( !prop )
				return;
				
			strcpy( prop->property, "view" );
			prop->form = formName;
			strcpy( prop->data.name, "Content" );
			paramMessage->AddData( "target", PROPERTY_TYPE, prop, sizeof( PropertyItem ) );
			
			strcpy( prop->property, "window" );
			prop->form = formName;
			strcpy( prop->data.name, Title() );		// Copy window title
			paramMessage->AddData( "target", PROPERTY_TYPE, prop, sizeof( PropertyItem ) );
			
			char	*flatParam;
			long	numBytes;
			
			paramMessage->Flatten( &flatParam, &numBytes );
			
			BMessage	*wsMsg;
			
			wsMsg = new BMessage( kWSBatchCheckMe );
			if ( !wsMsg )
				return;
			
			// We're adding just one objspec-containing message here, but in 
			// principle we could add any number.
				
			wsMsg->AddData( "objspec", 0L, flatParam, numBytes );
			
			delete paramMessage;
			free( flatParam );
			
			SendWordServicesMessage( msg, wsMsg );
			
			break;
			// MDC Word Services End
		
		case LIST_INVOKED:
			PostMessage(msg, fEnclosuresView);
			break;
		case CHANGE_FONT:
			message = new BMessage(msg);
			PostMessage(msg, fContentView->fTextView);
			break;

		case M_REPLY:
		case M_REPLY_ALL:
		case M_FORWARD:
			message = new BMessage(M_NEW);
			message->AddObject("record", fRecord);
			message->AddObject("window", this);
			message->AddLong("type", msg->what);
			be_app->PostMessage(message);
			break;

		case M_DELETE:
			if (level == L_BEGINNER) {
				beep();
				if (!(new BAlert("",
						"Are you sure you want to delete this message?",
						"Cancel", "Delete", NULL, B_WIDTH_AS_USUAL,
						B_WARNING_ALERT))->Go())
					break;
			}
			fDelete = TRUE;
			PostMessage(new BMessage(B_CLOSE_REQUESTED));
			break;

		case M_CLOSE_READ:
			message = new BMessage(B_CLOSE_REQUESTED);
			message->AddString("status", "Read");
			PostMessage(message);
			break;

		case M_CLOSE_SAME:
			message = new BMessage(B_CLOSE_REQUESTED);
			message->AddString("status", fRecord->FindString("Status"));
			PostMessage(message);
			break;

		case M_CLOSE_CUSTOM:
			if (msg->HasString("status")) {
				message = new BMessage(B_CLOSE_REQUESTED);
				message->AddString("status", msg->FindString("status"));
				PostMessage(message);
			}
			else {
				r = Frame();
				r.left += ((r.Width() - STATUS_WIDTH) / 2);
				r.right = r.left + STATUS_WIDTH;
				r.top += 40;
				r.bottom = r.top + STATUS_HEIGHT;
				new TStatusWindow(r, this,
								(char *)fRecord->FindString("Status"));
			}
			break;

		case M_HEADER:
			header = !(fHeader->IsMarked());
			fHeader->SetMarked(header);
			message = new BMessage(M_HEADER);
			message->AddBool("header", header);
			PostMessage(message, fContentView->fTextView);
			break;

		case M_SEND_NOW:
			queue = FALSE;
			// yes, we are suppose to fall through
		case M_SEND_LATER:
			Send(queue);
			break;

		case M_SAVE:
			subject = fHeaderView->fSubject->Text();
			len = min(B_FILE_NAME_LENGTH - 1, strlen(subject) + 1);
			strncpy(title, subject, len);
			if (save_dir.record) {
				message = new BMessage(B_SAVE_REQUESTED);
				message->AddRef("directory", save_dir);
			}
			else
				message = NULL;
			RunSavePanel(title, NULL, NULL, NULL, message);
			break;

		case M_PRINT:
			Print();
			break;

		case M_UNDO:
			// ***TODO***
			break;

		case M_SELECT:
			break;

		case M_QUOTE:
		case M_REMOVE_QUOTE:
			PostMessage(msg->what, fContentView);
			break;

		case M_SIGNATURE:
			query.AddTable(sig_table);
			query.PushOp(B_ALL);
			query.Fetch();
			record = new BRecord(sig_table->Database(),
						query.RecordIDAt(msg->FindLong("index")));
			if (record->Error() == B_NO_ERROR) {
				message = new BMessage(M_SIGNATURE);
				message->AddRef("sig", record->Ref());
				PostMessage(message, fContentView);
			}
			delete record;
			break;

		case M_ADD:
			message = new BMessage(REFS_RECEIVED);
			message->AddObject("window", this);
			be_app->RunFilePanel("Add Enclosures", "Add", NULL, FALSE, message);
			break;

		case M_REMOVE:
			PostMessage(msg->what, fEnclosuresView);
			break;

		case M_INFO:
			// ***TODO***
			break;

		case B_SIMPLE_DATA:
		case B_REFS_RECEIVED:
		case REFS_RECEIVED:
			message = NULL;
			while (msg->HasRef("refs", item)) {
				record = new BRecord(msg->FindRef("refs", item));
				type = record->FindLong("fsType");
				if (type == B_MAIL_TYPE) {
					mail = new BMessage(B_REFS_RECEIVED);
					mail->AddRef("refs", msg->FindRef("refs", item));
					be_app->PostMessage(mail);
				}
				else {
					if (!message)
						message = new BMessage(REFS_RECEIVED);
					message->AddRef("refs", msg->FindRef("refs", item));
				}
				item++;
			}
			if (message)
				PostMessage(message, fEnclosuresView);
			break;

		default:
			BWindow::MessageReceived(msg);
			break;
	}
}

//--------------------------------------------------------------------

bool TMailWindow::QuitRequested(void)
{
	BMessage	*message;

	message = new BMessage(WINDOW_CLOSED);
	message->AddLong("kind", MAIL_WINDOW);
	message->AddRect("window", Frame());
	be_app->PostMessage(message);

	if ((CurrentMessage()) && (CurrentMessage()->HasString("status")))
		strcpy(fStatus, CurrentMessage()->FindString("status"));
	else if (fRecord) {
		if (strcmp(fRecord->FindString("Status"), "New") == 0)
			strcpy(fStatus, "Read");
		else
			strcpy(fStatus, fRecord->FindString("Status"));
	}
	return TRUE;
}

//--------------------------------------------------------------------

void TMailWindow::SaveRequested(record_ref ref, const char *name)
{
	char		str[256];
	char		*header;
	long		len;
	long		loop;
	long		result;
	long		start = 0;
	BDirectory	dir;
	BFile		file;

	save_dir.record = ref.record;
	save_dir.database = ref.database;

	dir.SetRef(ref);
	if ((result = dir.Create(name, &file)) == B_NO_ERROR) {
		file.Open(B_READ_WRITE);
		if ((result = file.Error()) == B_NO_ERROR) {
			if ((fIncoming) && !(fHeader->IsMarked())) {
				header = (char *)fRecord->FindRaw("header", &len);
				for (loop = 0; loop < len; loop++) {
					if ((loop) && (header[loop] == 10) &&
								  (header[loop - 1] == 13)) {
						header[loop - 1] = 10;
						file.Write(&header[start], loop - start);
						header[loop - 1] = 13;
						start = loop + 1;
					}
				}
				if (start < len)
					file.Write(&header[start], len - start);
			}
			file.Write(fContentView->fTextView->Text(),
					   fContentView->fTextView->TextLength());
			result = file.Error();
			file.Close();
			file.SetTypeAndApp('TEXT', 'EDIT');
		}
	}
	if (result != B_NO_ERROR) {
		sprintf(str, "An error occurred trying to save %s (0x%.8x).", name,
																	  result);
		beep();
		(new BAlert("", str, "OK"))->Go();
	}
}

//--------------------------------------------------------------------

void TMailWindow::Show(void)
{
	BTextView	*text_view;

	Lock();
	if ((fIncoming) || (fReplying))
		fContentView->fTextView->MakeFocus(TRUE);
	else {
		text_view = (BTextView *)fHeaderView->fTo->ChildAt(0);
		fHeaderView->fTo->MakeFocus(TRUE);
		text_view->Select(0, text_view->TextLength());
	}
	Unlock();
	BWindow::Show();
}

//--------------------------------------------------------------------

void TMailWindow::Forward(BRecord *record, TMailWindow *wind)
{
	const char	*string;
	char		*str;
	long		len;

	string = record->FindString("Subject");
	if (record->Error() == B_NO_ERROR) {
		len = strlen(string);
		if ((len >= 5) && (strcmp(&string[len - 5], "(fwd)") == 0))
			fHeaderView->fSubject->SetText(string);
		else {
			str = (char *)malloc(len + 1 + 6);
			strcpy(str, string);
			strcat(str, " (fwd)");
			fHeaderView->fSubject->SetText(str);
			free(str);
		}
	}
	fContentView->fTextView->LoadMessage(record, TRUE, FALSE,
										"Forwarded message:\n");
	fEnclosuresView->CopyList(wind->fEnclosuresView);
}

//--------------------------------------------------------------------

void TMailWindow::Print(void)
{
	long			lines;
	long			lines_page;
	long			loop;
	long			pages;
	long			result;
	float			line_height;
	BPrintJob		*print;
	BRect			r;

	lines = fContentView->fTextView->CountLines();
	line_height = fContentView->fTextView->LineHeight();
	if ((lines) && ((int)line_height)) {
		print = new BPrintJob("mail_print");
		if ((result = print->InitJob()) == B_NO_ERROR) {
			r = print->PrintableRect();
			lines_page = r.Height() / line_height;
			pages = lines / lines_page;
			r.top = 0;
			r.bottom = line_height * lines_page;
			r.right -= r.left;
			r.left = 0;

			print->BeginPrinting();
			if (!print->CanContinue())
				goto out;
			for (loop = 0; loop <= pages; loop++) {
				print->DrawView(fContentView->fTextView, r, BPoint(0, 0));
				print->SpoolPage();
				r.top += (line_height * lines_page);
				r.bottom += (line_height * lines_page);
				if (!print->CanContinue())
					goto out;
			}
			print->Commit();
		}
/*
		else if (result == B_ERROR) {
			beep();
			(new BAlert("", "The print_server is not responding.", "OK"))->Go();
		}
*/
out:;
		delete print;
	}
}

//--------------------------------------------------------------------

void TMailWindow::Reply(BRecord *record, TMailWindow *wind, bool all)
{
	bool		have_to = FALSE;
	bool		have_cc = FALSE;
	char		*cc;
	char		*header;
	char		*str;
	const char	*string;
	const char	*to;
	long		finish;
	long		len;
	long		loop;
	long		start;

	to = record->FindString("Reply");
	if ((record->Error() == B_NO_ERROR) && (strlen(to)))
		fHeaderView->fTo->SetText(to);
	else {
		to = record->FindString("From");
		if (record->Error() == B_NO_ERROR)
			fHeaderView->fTo->SetText(to);
	}

	string = record->FindString("Subject");
	if (record->Error() == B_NO_ERROR) {
		if (cistrncmp((char *)string, "re:", 3) != 0) {
			str = (char *)malloc(strlen(string) + 1 + 4);
			strcpy(str, "Re: ");
			strcat(str, string);
			fHeaderView->fSubject->SetText(str);
			free(str);
		}
		else
			fHeaderView->fSubject->SetText(string);
	}

	wind->fContentView->fTextView->GetSelection(&start, &finish);
	if (start != finish) {
		str = (char *)malloc(finish - start + 1);
		wind->fContentView->fTextView->GetText(str, start, finish - start);
		if ((str[strlen(str) - 1] != '\r') &&
			(str[strlen(str) - 1] != '\n')) {
			str[strlen(str)] = B_RETURN;
			finish++;
		}
		fContentView->fTextView->SetText(str, finish - start);
		free(str);

		finish = fContentView->fTextView->CountLines();
		for (loop = 0; loop < finish; loop++) {
			fContentView->fTextView->GoToLine(loop);
			fContentView->fTextView->Insert((const char *)QUOTE);
		}
		fContentView->fTextView->GoToLine(0);
	}
	else
		fContentView->fTextView->LoadMessage(record, wind->fHeader->IsMarked(),
											TRUE, NULL);

	if (all) {
		header = (char *)record->FindRaw("header", &len);
		start = 0;
		cc = (char *)malloc(1);
		cc[0] = 0;
		for (;;) {
			if ((!have_to) &&
				!(cistrncmp(B_MAIL_TO, &header[start], strlen(B_MAIL_TO) - 1))) {
				have_to = TRUE;
				extract(&cc, &header[start + strlen(B_MAIL_TO)]);
				if (have_cc)
					break;
			}
			else if ((!have_cc) &&
				!(cistrncmp(B_MAIL_CC, &header[start], strlen(B_MAIL_CC) - 1))) {
				have_cc = TRUE;
				extract(&cc, &header[start + strlen(B_MAIL_CC)]);
				if (have_to)
					break;
			}
			start += linelen(&header[start], len - start);
			if (start >= len)
				break;
		}
		if (have_to || have_cc) {
			str = cistrstr(cc, (char *)to);
			if (str) {
				len = 0;
				while ((str > cc) && ((str[-1] == ' ') || (str[-1] == ','))) {
					str--;
					len++;
				}
				memmove(str, &str[strlen(to) + len], &cc[strlen(cc)] - 
												 &str[strlen(to) + len] + 1);
			}
			fHeaderView->fCc->SetText(cc);
		}
		free(cc);
	}
	fReplying = TRUE;
}

//--------------------------------------------------------------------

void TMailWindow::Send(bool now)
{
	bool			close = FALSE;
	char			mime[256];
	long			index = 0;
	long			len;
	long			result;
	BMailMessage	*mail;
	BRecord			*record;
	record_ref		*ref;

	mail = new BMailMessage();
	if (len = strlen(fHeaderView->fTo->Text()))
		mail->SetField(B_MAIL_TO, (char *)fHeaderView->fTo->Text(), len);

	if (len = strlen(fHeaderView->fSubject->Text()))
		mail->SetField(B_MAIL_SUBJECT,
							(char *)fHeaderView->fSubject->Text(), len);

	if (len = strlen(fHeaderView->fCc->Text()))
		mail->SetField(B_MAIL_CC, (char *)fHeaderView->fCc->Text(), len);

	if (len = strlen(fHeaderView->fBcc->Text()))
		mail->SetField(B_MAIL_BCC, (char *)fHeaderView->fBcc->Text(), len);

	if (len = fContentView->fTextView->TextLength())
		mail->SetField(B_MAIL_CONTENT,
							(char *)fContentView->fTextView->Text(), len);

	for (;;) {
		ref = (record_ref *)fEnclosuresView->fList->ItemAt(index++);
		if (ref) {
			record = new BRecord(*ref);
			if (record->Error() == B_NO_ERROR)
				mail->SetEnclosure(ref, "application", "befile");
			delete record;
		}
		else
			break;
	}
	result = mail->Send(now);
	delete mail;

	switch (result) {
		case B_NO_ERROR:
			close = TRUE;
			break;

		case B_MAIL_NO_DAEMON:
			close = TRUE;
			sprintf(mime, "The mail_daemon is not running.  The message is \
queued and will be sent when the mail_daemon is started.");
			break;

		case B_MAIL_UNKNOWN_HOST:
		case B_MAIL_ACCESS_ERROR:
			sprintf(mime, "An error occurred trying to connect with the SMTP \
host.  Check your SMTP host name.");
			break;

		case B_MAIL_NO_RECIPIENT:
			sprintf(mime, "You must have either a \"To\" or \"Bcc\" recipient.");
			break;

		default:
			sprintf(mime, "An error occurred trying to send mail (0x%.8x).",
							result);
	}
	if (result != B_NO_ERROR) {
		beep();
		(new BAlert("", mime, "OK"))->Go();
	}
	if (close)
		PostMessage(new BMessage(B_CLOSE_REQUESTED));
}


//====================================================================

TMenu::TMenu(const char *name)
	  :BMenu(name)
{
	BuildMenu();
}

//--------------------------------------------------------------------

void TMenu::AttachedToWindow(void)
{
	BuildMenu();
	BMenu::AttachedToWindow();
}

//--------------------------------------------------------------------

void TMenu::BuildMenu(void)
{
	const char	*name;
	long		len;
	long		loop;
	BMenuItem	*item;
	BQuery		query;
	BRecord		*record;
	record_id	id;

	while (item = RemoveItem((long)0)) {
		free(item);
	}
	query.AddTable(sig_table);
	query.PushOp(B_ALL);
	query.Fetch();
	if (len = query.CountRecordIDs()) {
		for (loop = 0; loop < len; loop++) {
			id = query.RecordIDAt(loop);
			record = new BRecord(sig_table->Database(), id);
			name = record->FindString("name");
			if (loop < 9)
				AddItem(new BMenuItem(name, new BMessage(M_SIGNATURE),
												'1' + loop));
			else
				AddItem(new BMenuItem(name, new BMessage(M_SIGNATURE)));
			delete record;
		}
	}
}

//====================================================================

TScrollView::TScrollView(BView *target)
			:BScrollView("", target, B_FOLLOW_ALL, B_WILL_DRAW, FALSE, TRUE)
{
}

//--------------------------------------------------------------------

void TScrollView::Draw(BRect where)
{
	BRect	r;

	r = Bounds();
	SetHighColor(216, 216, 216);
	StrokeLine(BPoint(r.left, r.top), BPoint(r.right, r.top));
	StrokeLine(BPoint(r.left, r.top + 1), BPoint(r.left, r.bottom - 1));
	StrokeLine(BPoint(r.right, r.top + 1), BPoint(r.right, r.bottom - 1));
}

