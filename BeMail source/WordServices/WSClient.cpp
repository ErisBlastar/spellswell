// WSClient.cpp
// Client side class implementation for Word Services
// 26 Apr 97 Mike Crawford crawford@scruznet.com

#include "WSClient.h"

WSClientMenu::WSClientMenu( char *title, long messageID, BHandler *target )
	: BMenu( title )
{
	long	numVols;
	long	volNum;
	BVolume	vol;
	BDatabase	*db;
	BTable		*table;
	BQuery		*query;
	long		numRecs;
	long		recNum;
	
	// Loop through each volume.  For each volume's database,
	// find the WordService table, and convert its records into
	// menu items.  Append the items to this (a BMenu).

	numVols = count_volumes();
	
	for ( volNum = 0; volNum < numVols; volNum++ ){
		vol = volume_at( volNum );
		db = vol.Database();
		
		db->Sync();

		table = db->FindTable( "WordService" );
		if ( table == (BTable*)NULL )
			continue;
		
		query = new BQuery;
		if ( !query )
			continue;
		
		query->AddTable( table );
		query->PushOp( B_ALL );
		
		long	err;
		
		err = query->Fetch();
		if ( err != B_NO_ERROR )  
			continue;
		
		numRecs = query->CountRecordIDs();
		
		for ( recNum = 0; recNum < numRecs; recNum++ ){
			record_id	id;
			BRecord		*rec;
			const char	*menuString;
			long		index;
			record_ref	serverRef;
		
			id = query->RecordIDAt( recNum );
			
			rec = new BRecord( db, id );
			if ( !rec )
				continue;
			
			menuString = rec->FindString( "MenuString" );
			index = rec->FindLong( "Index" );
			serverRef.record = rec->FindRecordID( "RecordID" );
			serverRef.database = db->ID();

			// STUB
			// If the server application is missing (has been deleted)
			// skip this item.

			BMessage *msg;
			
			msg = new BMessage( messageID );
			if ( !msg )
				continue;
			
			msg->AddLong( "Index", index );			// the service number
			msg->AddRef( "RecordRef", serverRef );
			
			BMenuItem	*menuItem;
			
			menuItem = new BMenuItem( menuString, msg );
			if ( !menuItem )
				continue;
			
			menuItem->SetTarget( target );
			
			AddItem( menuItem );
		}
		
	}
			
	return;
}

WSClientMenu::~WSClientMenu()
{
	return;
}

long SendWordServicesMessage( BMessage *menuMsg, BMessage *wsMsg )
{
	// menuMsg is the message the app recieved from its menu
	// wsMsg is the message we will send to the speller

	record_ref	serverRef;
	long		index;
	
	serverRef = menuMsg->FindRef( "RecordRef" );
	index = menuMsg->FindLong( "Index" );
	
	wsMsg->AddLong( "Index", index );		// Tell server which service
											// to perform.
											
	// The roster is kind enough to look for both running and
	// non-running (dead?) apps.  The thread_id will be -1 if the
	// app is not running.  In that case we launch it
		
	team_id	serverTeam;

	serverTeam = be_roster->TeamFor( serverRef );
	
	if ( serverTeam == -1 ){
		// Launch the server
		be_roster->Launch( serverRef, (BMessage*)NULL, &serverTeam );
		
		if ( serverTeam == (thread_id)-1 ){
			// STUB show an error alert
			return -1;
		}
	}
	
	BMessenger	*mess;
	
	mess = new BMessenger( NULL, serverTeam );
	if ( !mess )
		return -1;
		
	if ( mess->Error() != B_NO_ERROR )
		return mess->Error();
	
	mess->SendMessage( wsMsg );
	if ( mess->Error() != B_NO_ERROR )
		return mess->Error();
	
	delete mess;
	
	return B_NO_ERROR;
}
