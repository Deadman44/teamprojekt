// client.cpp, mostly network related client game code

#include "cube.h"




ENetHost *clienthost = NULL;
int connecting = 0;
int connattempts = 0;
int disconnecting = 0;
int clientnum = -1;         // our client id in the game
bool c2sinit = false;       // whether we need to tell the other clients our stats

//TP
bool secondSATrequired = false;





int getclientnum() { return clientnum; };

bool multiplayer()
{
    // check not correct on listen server?
    if(clienthost) conoutf("operation not available in multiplayer");
    return clienthost!=NULL;
};

bool allowedittoggle()
{
    bool allow = !clienthost || gamemode==1;
    if(!allow) conoutf("editing in multiplayer requires coopedit mode (1)");
    return allow; 
};

VARF(rate, 0, 0, 25000, if(clienthost && (!rate || rate>1000)) enet_host_bandwidth_limit (clienthost, rate, rate));

void throttle();

VARF(throttle_interval, 0, 5, 30, throttle());
VARF(throttle_accel,    0, 2, 32, throttle());
VARF(throttle_decel,    0, 2, 32, throttle());

void throttle()
{
    if(!clienthost || connecting) return;
    assert(ENET_PEER_PACKET_THROTTLE_SCALE==32);
    enet_peer_throttle_configure(clienthost->peers, throttle_interval*1000, throttle_accel, throttle_decel);
};

void newname(char *name) { c2sinit = false; strn0cpy(player1->name, name, 16); };
void newteam(char *name) { c2sinit = false; strn0cpy(player1->team, name, 5); };

COMMANDN(team, newteam, ARG_1STR);
COMMANDN(name, newname, ARG_1STR);

void writeclientinfo(FILE *f)
{
    fprintf(f, "name \"%s\"\nteam \"%s\"\n", player1->name, player1->team);
};

void connects(char *servername, char *port) //chg +1 param
{   
	/*
	das was bei /t connect ip im spiel passiert...
	*/

    disconnect(1);  // reset state
    addserver(servername);

	acquire_SAT(); //fordere SAT vom Lizenzserver an

	int serverport = atoi(port);
    conoutf("attempting to connect to %s", servername, serverport);
    ENetAddress address = { ENET_HOST_ANY, serverport }; // erstmal leere adresse...  // port war SERVER_PoRT
    if(enet_address_set_host(&address, servername) < 0) //versuch der adresszuweisung.
    {
        conoutf("could not resolve server %s", servername);
        return;
    };

    clienthost = enet_host_create(NULL, 1, rate, rate); // server = clienthost...

    if(clienthost)
    {
        enet_host_connect(clienthost, &address, 1); 
        enet_host_flush(clienthost);
		

        connecting = lastmillis;
        connattempts = 0;
    }
    else
    {
        conoutf("could not connect to server");
        disconnect();
    };
	std::cout << "Empfangenes SAT -> " << sat << " \n";
};

void disconnect(int onlyclean, int async)
{
    if(clienthost) 
    {
        if(!connecting && !disconnecting) 
        {
            enet_peer_disconnect(clienthost->peers);
            enet_host_flush(clienthost);
            disconnecting = lastmillis;
        };
        if(clienthost->peers->state != ENET_PEER_STATE_DISCONNECTED)
        {
            if(async) return;
            enet_peer_reset(clienthost->peers);
        };
        enet_host_destroy(clienthost);
    };

    if(clienthost && !connecting) conoutf("disconnected");
    clienthost = NULL;
    connecting = 0;
    connattempts = 0;
    disconnecting = 0;
    clientnum = -1;
    c2sinit = false;
    player1->lifesequence = 0;
    loopv(players) zapdynent(players[i]);
    
    localdisconnect();

	satSent = 1; //warten bis connection zum server vollst�ndig etabliert ist, dann auf 0 setzen /siehe ENET_CONNECT

    if(!onlyclean) { stop(); localconnect(); };
};

void trydisconnect()
{
    if(!clienthost)
    {
        conoutf("not connected");
        return;
    };
    if(connecting) 
    {
        conoutf("aborting connection attempt");
        disconnect();
        return;
    };
    conoutf("attempting to disconnect...");
    disconnect(0, !disconnecting);
};

string ctext;
void toserver(char *text) { conoutf("%s:\f %s", player1->name, text); strn0cpy(ctext, text, 80); };
void echo(char *text) { conoutf("%s", text); };

COMMAND(echo, ARG_VARI);
COMMANDN(say, toserver, ARG_VARI);
COMMANDN(connect, connects, ARG_2STR); //von 1str auf 2str wegen portangabe
COMMANDN(disconnect, trydisconnect, ARG_NONE);

// collect c2s messages conveniently

vector<ivector> messages;

void addmsg(int rel, int num, int type, ...)
{
    if(demoplayback) return;
    if(num!=msgsizelookup(type)) { sprintf_sd(s)("inconsistant msg size for %d (%d != %d)", type, num, msgsizelookup(type)); fatal(s); }; //hier abfrage ob es den typ der message gibt
	//fals nicht, dann kommt die fehlermeldung, siehe SV_XXXXX nachrichten
    if(messages.length()==100) { conoutf("command flood protection (type %d)", type); return; };
    ivector &msg = messages.add();
    msg.add(num);
    msg.add(rel);
    msg.add(type);
    va_list marker;
    va_start(marker, type); //c-style varargs verarbeitung...
    loopi(num-1) msg.add(va_arg(marker, int));
    va_end(marker);  
};

void server_err()
{
    conoutf("server network error, disconnecting...");
    disconnect();
};

int lastupdate = 0, lastping = 0;
string toservermap;
bool senditemstoserver = false;     // after a map change, since server doesn't have map data

string clientpassword;
void password(char *p) { strcpy_s(clientpassword, p); };
COMMAND(password, ARG_1STR);

bool netmapstart() { senditemstoserver = true; return clienthost!=NULL; };

void initclientnet()
{
    ctext[0] = 0;
    toservermap[0] = 0;
    clientpassword[0] = 0;
    newname("unnamed");
    newteam("red");
};

void sendpackettoserv(void *packet)
{
/* achtung! schickt nur die maps zum server, nicht die eigentlich positionsdaten usw....
*/

    if(clienthost) { enet_host_broadcast(clienthost, 0, (ENetPacket *)packet); enet_host_flush(clienthost); }
    else localclienttoserver((ENetPacket *)packet);
}

void c2sinfo(dynent *d)                     // send update to the server
{
    if(clientnum<0) return;                 // we haven't had a welcome message from the server yet
    if(lastmillis-lastupdate<40) return;    // don't update faster than 25fps // MOD MOD MOD 27_04
	/*
	wenn das updateinvervall gr��er ist, hei�t das nicht unbedingt, dass die pakete gr��er werden
	nur die information, die per addmsg noch dazukommen machen die pakete gr��er, dazu geh�ren NICHT positionsupdates
	*/
    ENetPacket *packet = enet_packet_create (NULL, MAXTRANS, 0); //erstellt hier ein packet..... mit gr��e von MAXTRANS
    uchar *start = packet->data; //start ist zeiger aufs erste datenbyte
    uchar *p = start+2;
    bool serveriteminitdone = false;
    if(toservermap[0])                      // suggest server to change map
    {                                       // do this exclusively as map change may invalidate rest of update
        packet->flags = ENET_PACKET_FLAG_RELIABLE;
        putint(p, SV_MAPCHANGE);
        sendstring(toservermap, p);
        toservermap[0] = 0;
        putint(p, nextmode);
    }
    else
    {
		/*
		koordinaten �bertragen, das alles geht ins paket, immer!
		*/

        putint(p, SV_POS);
        putint(p, clientnum);
        putint(p, (int)(d->o.x*DMF));       // quantize coordinates to 1/16th of a cube, between 1 and 3 bytes
        putint(p, (int)(d->o.y*DMF));
        putint(p, (int)(d->o.z*DMF));
        putint(p, (int)(d->yaw*DAF));
        putint(p, (int)(d->pitch*DAF));
        putint(p, (int)(d->roll*DAF));
        putint(p, (int)(d->vel.x*DVF));     // quantize to 1/100, almost always 1 byte
        putint(p, (int)(d->vel.y*DVF));
        putint(p, (int)(d->vel.z*DVF));
        // pack rest in 1 byte: strafe:2, move:2, onfloor:1, state:3
        putint(p, (d->strafe&3) | ((d->move&3)<<2) | (((int)d->onfloor)<<4) | ((editmode ? CS_EDITING : d->state)<<5) );
 
        if(senditemstoserver)
        {
            packet->flags = ENET_PACKET_FLAG_RELIABLE;
            putint(p, SV_ITEMLIST);
            if(!m_noitems) putitems(p); //hier items zuf�gen
            putint(p, -1); //endekennung, wird sp�ter gebraucht
            senditemstoserver = false;
            serveriteminitdone = true;
        };
        if(ctext[0])    // player chat, not flood protected for now
        {
            packet->flags = ENET_PACKET_FLAG_RELIABLE;
            putint(p, SV_TEXT);
            sendstring(ctext, p);
            ctext[0] = 0;
        };
        if(!c2sinit)    // tell other clients who I am //
        {
            packet->flags = ENET_PACKET_FLAG_RELIABLE;
            c2sinit = true;
            putint(p, SV_INITC2S);
            sendstring(player1->name, p);
            sendstring(player1->team, p);
            putint(p, player1->lifesequence);
        };
        loopv(messages)     // send messages collected during the previous frames
			/* setzt alle anderen messages ins datenpaket ab
			*/

        {
            ivector &msg = messages[i];
            if(msg[1]) packet->flags = ENET_PACKET_FLAG_RELIABLE;
            loopi(msg[0]) putint(p, msg[i+2]);
        };
		//TP
		/*
		Ein client muss zun�chst ein server-access-ticket vom lizenzserver abrufen (geschieht per email+angabe eines g�ltigen tickets)
		SAT ist eine 6-stellige Zahl die der client dem spielserver nach dem beitritt zusenden muss (inklusive der eigenen email)
		der spielserver pr�ft die Kombination von SAT und USERNAME(=Email) auf ihre g�ltigkeit, dazu ruft er eine funktion auf dem lizenzserver auf
		gibt der lizenzserver sein ok, darf der spieler auf dem server bleiben

		will sich ein client verbinden, der garkeine SAT-Message verschickt, so wird nach dem erhalt des ersten datenpakets auf dem Server �berpr�ft
		ob der client einen username eingetragen/�bermittelt hat. ist dieser string leer (==empty), dann wird der spieler vom server geworfen

		*/
		if(satSent == 0 ||satSent == 2) //!
		{
			std::cout << " \n Sende USER+SAT an Server \n ";
			putint(p,SV_SAT);
			putint(p,user.length()+6); //wegen laenge-6-SAT



			std::cout << sat << "\n";
			//SAT ins Paket packen
			const char *satc = sat.c_str();

			std::cout << " USRSAT " << satc << "\n";
			for(int k = 0; k < 6;k++)
			{

				putint(p,satc[k]);
	
			}		
			std::cout << " USR " << user << "\n";
			//Email (==USER) ins Paket packen
			for(int q = 0; q < user.length(); q++)
			{
				putint(p,user[q]); 
			}
			//HIER OHNE 0-CHAR! muss auf Serverende angef�gt werden bzw implizit �ber std::string konstruktor


			if(satSent == 0)
			{
				std::cout << "erstes SAT an Server gesendet \n";
				satSent = 1; //erster Durchlauf komplett
				sat = "000000"; //erstes sat l�schen;
				
			}
			else if(satSent == 2)
			{
				std::cout << "zweites SAT an Server gesendet \n";
				satSent=3; //zweiter Durchlauf komplett
			}
		}
		
		if(secondSATrequired)
		{
			if(sat.compare("000000")!=0)
			{
				satSent = 2; //denn wenn der aktuelle sat NICHT 000000 ist, ist er g�ltig und kann verschickt werden
				//findet hier anwendung zum verschicken des zweiten sat, der asynchron zum spielfluss angefordert wird
				//damit er das spiel nicht blockiert
				secondSATrequired = false;
			}
		}

		//TP OUT
        messages.setsize(0);
        if(lastmillis-lastping>250)
        {
            putint(p, SV_PING);
            putint(p, lastmillis);
            lastping = lastmillis;
        };

    };
    *(ushort *)start = ENET_HOST_TO_NET_16(p-start);
    enet_packet_resize(packet, p-start); //paket so klein wie m�glich machen
    incomingdemodata(start, p-start, true);
    if(clienthost) { enet_host_broadcast(clienthost, 0, packet); enet_host_flush(clienthost); } //hier das eigentliche verschicken ????
    else localclienttoserver(packet);
    lastupdate = lastmillis;
    if(serveriteminitdone) loadgamerest();  // hack
};

void gets2c()           // get updates from the server
{
    ENetEvent event;
    if(!clienthost) return;
    if(connecting && lastmillis/3000 > connecting/3000)
    {
        conoutf("attempting to connect...");
        connecting = lastmillis;
        ++connattempts; 
        if(connattempts > 3)
        {
            conoutf("could not connect to server");
            disconnect();
            return;
        };
    };
    while(clienthost!=NULL && enet_host_service(clienthost, &event, 0)>0)
    switch(event.type)
    {

        case ENET_EVENT_TYPE_CONNECT:
            conoutf("connected to server");
            connecting = 0;
            throttle();
			satSent = 0; //TP, wir sind an dieser Stelle mit Server verbunden..
            break;
         
        case ENET_EVENT_TYPE_RECEIVE: //hier "normales" paket, der paketinhalt wird dann die methode localservetoclient weitergereicht
            if(disconnecting) conoutf("attempting to disconnect...");
            else localservertoclient(event.packet->data, event.packet->dataLength);
            enet_packet_destroy(event.packet);
            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            if(disconnecting) disconnect();
            else server_err();
            return;
    }
};

