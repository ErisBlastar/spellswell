// WSClient.h
// Client-side classes for doing Word Services
// 26 Apr 97 Mike Crawford

class WSClientMenu: public BMenu{
	public:
		WSClientMenu( char *title, long messageID, BHandler *target );
		virtual ~WSClientMenu();
};

long SendWordServicesMessage( BMessage *menuMsg, BMessage *wsMsg );
