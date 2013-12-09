//--------------------------------------------------------------------
//	
//	Signature.h
//
//	Written by: Robert Polic
//	
//	Copyright 1996 Be, Inc. All Rights Reserved.
//	
//--------------------------------------------------------------------

#ifndef SIGNATURE_H
#define SIGNATURE_H

#ifndef _ALERT_H
#include <Alert.h>
#endif
#ifndef _BEEP_H
#include <Beep.h>
#endif
#ifndef _MENU_H
#include <Menu.h>
#endif
#ifndef _MENU_BAR_H
#include <MenuBar.h>
#endif
#ifndef _MENU_ITEM_H
#include <MenuItem.h>
#endif
#ifndef _TEXT_CONTROL_H
#include <TextControl.h>
#endif
#ifndef _WINDOW_H
#include <Window.h>
#endif

#define	SIG_WIDTH			457
#define SIG_HEIGHT			200

#define NAME_TEXT			"Name:"
#define NAME_FIELD_H		 29
#define NAME_FIELD_V		  8
#define NAME_FIELD_WIDTH	282
#define NAME_FIELD_HEIGHT	 16

#define SIG_TEXT			"Signature:"
#define SIG_TEXT_H			 12
#define SIG_TEXT_V			 45
#define SIG_FIELD_H			 59
#define SIG_FIELD_V			 36

class	TMenu;
class	TNameControl;
class	TScrollView;
class	TSignatureView;
class	TSigTextView;

//====================================================================

class TSignatureWindow : public BWindow {

private:

BMenuItem		*fCut;
BMenuItem		*fCopy;
BMenuItem		*fDelete;
BMenuItem		*fPaste;
BMenuItem		*fSave;
BMenuItem		*fUndo;
BRecord			*fRecord;
TMenu			*fSignature;
TSignatureView	*fSigView;

public:

				TSignatureWindow(BRect, char*, long);
virtual void	FrameResized(float, float);
virtual void	MenusWillShow(void);
virtual void	MessageReceived(BMessage*);
virtual bool	QuitRequested(void);
virtual void	Show(void);
bool			Clear(void);
bool			IsDirty(void);
void			Save(void);
};

//=================================================================
// Class: TSignatureView
//
// Displays a name and signature text for the user to edit.
//=================================================================

class TSignatureView : public BView {

private:

bool			fFocus;
float			fWidth;
TSignatureWindow	*fWindow;
TScrollView		*fScroll;

public:

TNameControl	*fName;
TSigTextView	*fTextView;

				TSignatureView(BRect, TSignatureWindow*, char*, long); 
virtual	void	Draw(BRect);
void			Focus(bool);
void			FrameText(void);
void			ReSize(void);
};

//=================================================================
// Class: TNameControl
//
// Contains controls the input and display of the user's
// name in the signature view.
//=================================================================


class TNameControl : public BTextControl {

private:

char			fLabel[100];

public:

				TNameControl(BRect, char*, BMessage*);
virtual void	AttachedToWindow(void);
virtual void	Draw(BRect);
virtual void	MessageReceived(BMessage*);
};

//=================================================================
// Class: TSigTextView
//
// Controls the display and editing of the signature text
// in a signature view.
//=================================================================

class TSigTextView : public BTextView {

private:

long			fFontSize;
TSignatureView	*fParent;
font_name		fFontName;

public:

				TSigTextView(BRect, BRect, TSignatureView*, char*, long); 
virtual	void	AttachedToWindow(void);
virtual void	KeyDown(ulong);
virtual void	MessageReceived(BMessage*);
virtual void	MakeFocus(bool);
};
#endif
