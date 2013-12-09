//--------------------------------------------------------------------
//	
//	Enclosures.h
//
//	Written by: Robert Polic
//	
//	Copyright 1996 Be, Inc. All Rights Reserved.
//	
//--------------------------------------------------------------------

#ifndef ENCLOSURES_H
#define ENCLOSURES_H

#ifndef _BITMAP_H
#include <Bitmap.h>
#endif
#ifndef _FILE_H
#include <File.h>
#endif
#ifndef _LIST_VIEW_H
#include <ListView.h>
#endif
#ifndef _POINT_H
#include <Point.h>
#endif
#ifndef _RECORD_H
#include <Record.h>
#endif
#ifndef _RECT_H
#include <Rect.h>
#endif
#ifndef _SCROLL_VIEW_H
#include <ScrollView.h>
#endif
#ifndef _VIEW_H
#include <View.h>
#endif
#ifndef _VOLUME_H
#include <Volume.h>
#endif

#define ENCLOSURES_HEIGHT	 94

#define ENCLOSE_TEXT		"Enclosures:"
#define ENCLOSE_TEXT_H		  7
#define ENCLOSE_TEXT_V		 21
#define ENCLOSE_FIELD_H		 59
#define ENCLOSE_FIELD_V		 11
#define ENCLOSE_FIELD_HEIGHT 71

class	TListView;
class	TMailWindow;
class	TScrollView;


//=================================================================
// Class: TEnclosureView
//
// This is the view that displays the enclosures for a particular
// email message.  It uses a ListView in a ScrollView.
//=================================================================

class TEnclosuresView : public BView {

private:

bool			fFocus;
bool			fIncoming;
float			fWidth;
BRecord			*fRecord;
TMailWindow		*fWindow;
TScrollView		*fScroll;

public:

TListView		*fList;

				TEnclosuresView(BRect, TMailWindow*, BRect, bool, BRecord*); 
virtual	void	Draw(BRect);
virtual void	MessageReceived(BMessage*);
void			CopyList(TEnclosuresView*);
void			Focus(bool);
long			RemoveEnclosure(record_ref*);
void			ReSize(void);
};


//=================================================================
// Class: TListView
//
// Displays the list of values that will be shown in the Enclosures
// view.  This view is responsible for the look that each individual
// line item has.
//=================================================================

class TListView : public BListView {

private:

BRecord			*fRecord;
TEnclosuresView	*fParent;

public:

				TListView(BRect, BRecord*, TEnclosuresView*);
virtual	void	AttachedToWindow(void);
virtual void	DrawItem(BRect, long);
virtual void	HighlightItem(bool, long);
virtual float	ItemHeight(void);
virtual void	MakeFocus(bool);
};
#endif
