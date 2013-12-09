//--------------------------------------------------------------------
//	
//	Enclosures.cpp
//
//	Written by: Robert Polic
//	
//	Copyright 1996 Be, Inc. All Rights Reserved.
//	
//--------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef MAIL_H
#include "Mail.h"
#endif
#ifndef ENCLOSURES_H
#include "Enclosures.h"
#endif

extern long	level;


//====================================================================

TEnclosuresView::TEnclosuresView(BRect rect, TMailWindow *window,
							 BRect wind_rect, bool incoming, BRecord *record)
				:BView(rect, "", B_FOLLOW_BOTTOM, B_WILL_DRAW)
{
	BRect		r;
	rgb_color	c;

	fWindow = window;
	fIncoming = incoming;
	fRecord = record;
	fWidth = 0;
	fFocus = FALSE;

	c.red = c.green = c.blue = VIEW_COLOR;
	SetViewColor(c);
	SetFontName("Erich");
	SetFontSize(9);
	SetDrawingMode(B_OP_OVER);

	r.Set(ENCLOSE_FIELD_H, ENCLOSE_FIELD_V,
		  wind_rect.right - wind_rect.left - B_V_SCROLL_BAR_WIDTH - 11,
		  ENCLOSE_FIELD_V + ENCLOSE_FIELD_HEIGHT);
	fList = new TListView(r, fRecord, this);
	fList->SetInvocationMessage(new BMessage(LIST_INVOKED));

	fScroll = new TScrollView(fList);
	AddChild(fScroll);
}

//--------------------------------------------------------------------

void TEnclosuresView::Draw(BRect where)
{
	BRect	r;

	r = Window()->Bounds();
	if (!fWidth)
		fWidth = r.Width();

	r.top = r.bottom - ENCLOSURES_HEIGHT;
	r.OffsetTo(0, 0);
	SetHighColor(64, 64, 64);
	StrokeLine(BPoint(r.left, r.top), BPoint(r.right, r.top));
	SetHighColor(255, 255, 255);
	StrokeLine(BPoint(r.left, r.top + 1), BPoint(r.right, r.top + 1));
	StrokeLine(BPoint(r.left, r.top + 1), BPoint(r.left, r.bottom));
	SetHighColor(120, 120, 120);
	StrokeLine(BPoint(r.left, r.bottom), BPoint(r.right, r.bottom));
	StrokeLine(BPoint(r.right, r.bottom), BPoint(r.right, r.top + 2));

	SetHighColor(0, 0, 0);
	MovePenTo(ENCLOSE_TEXT_H, ENCLOSE_TEXT_V);
	DrawString(ENCLOSE_TEXT);
	if (!fFocus)
		SetHighColor(VIEW_COLOR, VIEW_COLOR, VIEW_COLOR);
	StrokeLine(BPoint(ENCLOSE_TEXT_H, ENCLOSE_TEXT_V + 2),
			   BPoint(ENCLOSE_TEXT_H + StringWidth(ENCLOSE_TEXT),
									  ENCLOSE_TEXT_V + 2));

	r = fScroll->Frame();
	r.InsetBy(-2, -2);
	if (r.Intersects(where)) {
		SetHighColor(255, 255, 255);
		StrokeLine(BPoint(r.left, r.bottom),
				   BPoint(r.right, r.bottom));
		StrokeLine(BPoint(r.right, r.bottom - 1),
				   BPoint(r.right, r.top));
		StrokeLine(BPoint(r.left + 1, r.bottom - 1),
				   BPoint(r.right - 1, r.bottom - 1));
		SetHighColor(120, 120, 120);
		StrokeLine(BPoint(r.left, r.top),
				   BPoint(r.left, r.bottom - 1));
		StrokeLine(BPoint(r.left + 1, r.top),
				   BPoint(r.right - 1, r.top));
		SetHighColor(176, 176, 176);
		StrokeLine(BPoint(r.left + 1, r.top + 1),
				   BPoint(r.right - 2, r.top + 1));
		StrokeLine(BPoint(r.left + 1, r.top + 2),
				   BPoint(r.left + 1, r.bottom - 2));
	}
}

//--------------------------------------------------------------------

void TEnclosuresView::MessageReceived(BMessage *msg)
{
	bool		bad_type = FALSE;
	void		*data;
	short		loop;
	long		item;
	long		result;
	BListView	*list;
	BMessage	*message;
	BMessenger	*browser;
	BRecord		*record;
	BRect		r;
	BTable		*table;
	record_ref	ref;

	switch (msg->what) {
		case LIST_INVOKED:
			list = (BListView *)msg->FindObject("source");
			if (msg->Error() == B_NO_ERROR) {
				data = list->ItemAt(msg->FindLong("index"));
				browser = new BMessenger('SHRK');
				if (browser->Error() == B_NO_ERROR) {
					message = new BMessage('Bopn'); //B_REFS_RECEIVED);
					memcpy(&ref, data, sizeof(record_ref));
					message->AddRef("refs", ref);
					browser->SendMessage(message);
				}
			}
			break;

		case M_REMOVE:
			item = fList->CurrentSelection();
			if (item >= 0) {
				data = fList->ItemAt(item);
				if (fIncoming) {
					if (RemoveEnclosure((record_ref *)data) != B_NO_ERROR)
						break;
				}
				fList->RemoveItem(item);
				free(data);
				if (item > (fList->CountItems() - 1))
					item--;
				if (fList->CountItems())
					fList->Select(item);
			}
			break;

		case B_SIMPLE_DATA:
		case B_REFS_RECEIVED:
		case REFS_RECEIVED:
			if ((!fIncoming) && (msg->HasRef("refs"))) {
				item = 0;
				do {
					ref = msg->FindRef("refs", item++);
					if ((result = msg->Error()) == B_NO_ERROR) {
						record = new BRecord(ref);
						table = record->Table();
						delete record;
						if (strcmp(table->Name(), "File") == 0) {
							for (loop = 0; loop < fList->CountItems(); loop++) {
								data = fList->ItemAt(loop);
								if (!memcmp((void *)&ref, data,
											sizeof(record_ref)))
									goto next;
							}
							data = malloc(sizeof(record_ref));
							memcpy(data, (void *)&ref, sizeof(record_ref));
							fList->AddItem(data);
							fList->Select(fList->CountItems() - 1);
						}
						else
							bad_type = TRUE;
					}
next:;
				} while (result == B_NO_ERROR);
				if (bad_type) {
					beep();
					(new BAlert("", "Only files can be added as enclosures.",
								"OK"))->Go();
				}
			}
			break;

		default:
			BView::MessageReceived(msg);
	}
}

//--------------------------------------------------------------------

void TEnclosuresView::CopyList(TEnclosuresView *trailer)
{
	void		*src_data;
	void		*dst_data;
	long		count;
	long		loop;

	count = trailer->fList->CountItems();
	for (loop = 0; loop < count; loop++) {
		src_data = trailer->fList->ItemAt(loop);
		dst_data = malloc(sizeof(record_ref));
		memcpy(dst_data, src_data, sizeof(record_ref));
		fList->AddItem(dst_data);
	}
}

//--------------------------------------------------------------------

void TEnclosuresView::Focus(bool focus)
{
	BRect	r;

	if (fFocus != focus) {
		r = Frame();
		fFocus = focus;
		Draw(r);
	}
}

//--------------------------------------------------------------------

long TEnclosuresView::RemoveEnclosure(record_ref *ref)
{
	char		*data;
	char		name[B_FILE_NAME_LENGTH];
	char		new_name[B_FILE_NAME_LENGTH];
	char		string[512];
	long		len;
	long		size;
	long		offset = 0;
	BDirectory	dir;
	BFile		file;

	file.SetRef(*ref);
	file.GetName(name);
	if (level == L_BEGINNER) {
		sprintf(string, "Are you sure you want to move \"%s\" to the trash?",
				name);
		beep();
		if (!(new BAlert("", string, "Cancel", "Trash", NULL, B_WIDTH_AS_USUAL,
				B_WARNING_ALERT))->Go())
			return B_ERROR;
	}

	data = (char *)fRecord->FindRaw("enclosures", &len);
	if (fRecord->Error() == B_NO_ERROR) {
		for (;;) {
			if (memcmp(ref, &data[offset], sizeof(record_ref)) == 0) {
				size = sizeof(record_ref);
				size += strlen(&data[offset + size]) + 1;
				size += strlen(&data[offset + size]) + 1;
				len -= size;
				memmove(&data[offset], &data[offset + size], len - offset);
				fRecord->SetRaw("enclosures", data, len);
				fRecord->SetLong("Enclosures",
								  fRecord->FindLong("Enclosures") - 1);
				fRecord->Commit();
				if (!fRecord->FindLong("mail_flags")) {
					volume_for_database(fRecord->Database()).GetRootDirectory(&dir);
					dir.GetDirectory("Trash", &dir);
					offset = 0;
					while (dir.Contains(name)) {
						if (strlen(name) >= B_FILE_NAME_LENGTH - 4)
							name[B_FILE_NAME_LENGTH - 4] = 0;
						sprintf(new_name, "%s%d", name, offset++);
						strcpy(name, new_name);
					}
					file.MoveTo(&dir, name);
					if (file.Error() != B_NO_ERROR)
						return B_ERROR;
				}
				break;
			}
			else {
				offset += sizeof(record_ref);
				offset += strlen(&data[offset]) + 1;
				offset += strlen(&data[offset]) + 1;
				if (((((record_ref *)&data[offset])->record == 0) &&
					(((record_ref *)&data[offset])->database == 0)) ||
					(offset > len))
					return B_ERROR;
			}
		}
	}
	return B_NO_ERROR;
}

//--------------------------------------------------------------------

void TEnclosuresView::ReSize(void)
{
	float		width;

	if (fWidth != (width = Window()->Frame().Width())) {
		if (fWidth < width) {
			SetHighColor(VIEW_COLOR, VIEW_COLOR, VIEW_COLOR);
			StrokeLine(BPoint(fWidth, 1),
					   BPoint(fWidth, ENCLOSURES_HEIGHT - 2));
		}
		else {
			SetHighColor(120, 120, 120);
			StrokeLine(BPoint(width, 1),
					   BPoint(width, ENCLOSURES_HEIGHT - 2));
		}
		fWidth = width;
	}
}


//====================================================================

TListView::TListView(BRect rect, BRecord *record, TEnclosuresView *view)
		  :BListView(rect, "", B_FOLLOW_ALL)
{
	fRecord = record;
	fParent = view;
}

//--------------------------------------------------------------------

void TListView::AttachedToWindow(void)
{
	char		*enclosures;
	long		length;
	long		offset = 0;
	record_ref	*ref;

	BListView::AttachedToWindow();
	SetFontName("Erich");
	SetFontSize(9);
	SetDrawingMode(B_OP_OVER);

	if (fRecord) {
		enclosures = (char *)fRecord->FindRaw("enclosures", &length);
		if (length) {
			for (;;) {
				ref = (record_ref *)malloc(sizeof(record_ref));
				memcpy(ref, &enclosures[offset], sizeof(record_ref));
				if ((ref->record == 0) && (ref->database == 0))
					break;
				AddItem(ref);
				offset += sizeof(record_ref);
				offset += strlen(&enclosures[offset]) + 1;
				offset += strlen(&enclosures[offset]) + 1;
			}
			free(ref);
		}
	}
}

//--------------------------------------------------------------------

void TListView::DrawItem(BRect where, long item)
{
	char		name[B_FILE_NAME_LENGTH];
	char		*bits = NULL;
	char		*path;
	long		result;
	long		len;
	BBitmap		*bitmap;
	BFile		*file;
	BDirectory	*dir;
	BQuery		*query;
	BRecord		*icon;
	BRecord		*record;
	BRect		r;
	BRect		sr;
	BRect		dr;
	record_id	id;
	record_ref	*ref;

	r = ItemFrame(item);

	path = (char *)malloc(32768);
	ref = (record_ref *)ItemAt(item);
	record = new BRecord(*ref);
	result = record->Error();
	if ((result != B_NO_ERROR) || ((result == B_NO_ERROR) &&
								   (!strlen(record->FindString("Name"))))) {
		sprintf(path, "<missing enclosure>");
		delete record;
	}
	else {
		delete record;
		file = new BFile(*ref);
		if (file->Error() == B_NO_ERROR)
			file->GetPath(path, 32767);
		else {
			dir = new BDirectory(*ref);
			dir->GetPath(path, 32767);
			if (dir->Error() != B_NO_ERROR) {
				record = new BRecord(*ref);
				dir->GetName(name);
				path[0] = 0;
				strcat(path, "db/");
				strcat(path, record->Table()->Name());
				strcat(path, "/");
				strcat(path, record->FindString("Name"));
				delete record;
			}
			else if (path[strlen(path) - 1] != '/')
				strcat(path, "/");
			delete dir;
		}
		delete file;

		record = new BRecord(*ref);
		id = record->FindRecordID("iconRef");
		if (record->Error() == B_NO_ERROR) {
			icon = new BRecord(record->Database(), id);
			if (icon) {
				sr.Set(0, 0, 15, 15);
				bits = (char *)icon->FindRaw("smallBits", &len);
				if (!bits) {
					query = new BQuery();
					query->AddTable(record->Database()->FindTable("Icon"));
					query->PushField("creator");
					query->PushLong(0);
					query->PushOp(B_EQ);
					query->PushField("type");
					query->PushLong(0);
					query->PushOp(B_EQ);
					query->PushOp(B_AND);
					query->FetchOne();

					if (query->CountRecordIDs()) {
						delete icon;
						id = query->RecordIDAt(0);
						icon = new BRecord(record->Database(), id);
						bits = (char *)icon->FindRaw("smallBits", &len);
					}
					delete query;
				}
			}

			if (bits) {
				bitmap = new BBitmap(sr, B_COLOR_8_BIT);
				bitmap->SetBits(bits, bitmap->BitsLength(), 0, B_COLOR_8_BIT);
				dr.Set(r.left + 4, r.top + 1, r.left + 4 + 15, r.top + 1 + 15);
				DrawBitmap(bitmap, sr, dr);
				delete bitmap;
			}
			if (icon)
				delete icon;
		}
		delete record;
	}

	r.left += 24;
	while (StringWidth(path) >= r.Width()) {
		len = strlen(path) >> 1;
		path[len - 1] = 0xc9;
		memcpy(&path[len], &path[len + 2], len);
	}
	MovePenTo(r.left, r.bottom - 4);
	DrawString(path);
	free(path);
}

//--------------------------------------------------------------------

void TListView::HighlightItem(bool flag, long index)
{
	BRect	r;

	r = ItemFrame(index);
	r.left += 22;
	InvertRect(r);
}

//--------------------------------------------------------------------

float TListView::ItemHeight(void)
{
	float	height;

	height = BListView::ItemHeight();
	if (height < 18)
		return 18;
	else
		return height;
}

//--------------------------------------------------------------------

void TListView::MakeFocus(bool focus)
{
	BListView::MakeFocus(focus);
	fParent->Focus(focus);
}
