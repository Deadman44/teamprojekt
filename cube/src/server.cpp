// server.cpp: little more than enhanced multicaster
// runs dedicated or as client coroutine

#include "cube.h" 

enum { ST_EMPTY, ST_LOCAL, ST_TCPIP };

struct client                   // server side version of "dynent" type
{
    int type;
    ENetPeer *peer;
    string hostname;
    string mapvote;
    string name;
    int modevote;

	//TEAMPROJEKT
	dynent *representer; //spielfigur des clients auf dem server
	int clientnr;
	int awaitingSpawnSignal; //zeigt an, ob client bereits eine spawnanfrage geschickt hat
	int allowRespawn; //sollte auf 0 stehen wenn respawn erlaubt is bzw der client neue pakete senden darf, enh�lt im zwischenzustand einen zufallswert (rnd -rnd+1 == -1))
};

vector<client> clients;

int maxclients = 8;
string smapname;

struct server_entity            // server side version of "entity" type
{
    bool spawned; //item liegt auf boden == gespawned
    int spawnsecs; //wie lange braucht das item zu spawnen?, wird vom client beim pickup �bertragen (additem methode..)

	//TP -- man ben�tigt noch den typ, um festzustellen was aufgesammelt wurde (wichtig: armour, ammo, hp sollte auf server+clients identisch sein)
	uchar type; 
};

vector<server_entity> sents;

bool notgotitems = true;        // true when map has changed and waiting for clients to send item
int mode = 0;

void restoreserverstate(vector<entity> &ents)   // hack: called from savegame code, only works in SP
{
    loopv(sents)
    {
        sents[i].spawned = ents[i].spawned;
        sents[i].spawnsecs = 0;
    }; 
};

int interm = 0, minremain = 0, mapend = 0;
bool mapreload = false;

char *serverpassword = "";

bool isdedicated;
ENetHost * serverhost = NULL;
int bsend = 0, brec = 0, laststatus = 0, lastsec = 0;

#define MAXOBUF 100000

void process(ENetPacket *packet, int sender);
void multicast(ENetPacket *packet, int sender);
void disconnect_client(int n, char *reason);

void send(int n, ENetPacket *packet) // normale �bertragung
{
	if(!packet) return;
    switch(clients[n].type)
    {
        case ST_TCPIP: //falls client im netzwerk liegt, dann verschicke �bers netz
        {
            enet_peer_send(clients[n].peer, 0, packet);
            bsend += packet->dataLength;
            break;
        };

        case ST_LOCAL: //falls das ziel local liegt bzw localhost, dann oeffne direkt die funktion unten
            localservertoclient(packet->data, packet->dataLength);
            break;

    };
};

void send2(bool rel, int cn, int a, int b) //wird bei einigen �bertragungen eingesetzt, z.b. ping, item placement usw
	//entspricht dem ENET RELIABLE PACKET
{
    ENetPacket *packet = enet_packet_create(NULL, 32, rel ? ENET_PACKET_FLAG_RELIABLE : 0);
    uchar *start = packet->data;
    uchar *p = start+2;
    putint(p, a);
    putint(p, b);
    *(ushort *)start = ENET_HOST_TO_NET_16(p-start);
    enet_packet_resize(packet, p-start);
    if(cn<0) process(packet, -1);
    else send(cn, packet);
    if(packet->referenceCount==0) enet_packet_destroy(packet);
};

void sendservmsg(char *msg)
{
    ENetPacket *packet = enet_packet_create(NULL, _MAXDEFSTR+10, ENET_PACKET_FLAG_RELIABLE);
    uchar *start = packet->data;
    uchar *p = start+2;
    putint(p, SV_SERVMSG);
    sendstring(msg, p);
    *(ushort *)start = ENET_HOST_TO_NET_16(p-start);
    enet_packet_resize(packet, p-start);
    multicast(packet, -1);
    if(packet->referenceCount==0) enet_packet_destroy(packet);
};

void disconnect_client(int n, char *reason)
{
    printf("disconnecting client (%s) [%s]\n", clients[n].hostname, reason);
    enet_peer_disconnect(clients[n].peer);
    clients[n].type = ST_EMPTY;
    send2(true, -1, SV_CDIS, n);
};

void resetitems() { sents.setsize(0); notgotitems = true; };

void pickup(uint i, int sec, int sender)         // server side item pickup, acknowledge first client that gets it
{
    if(i>=(uint)sents.length()) return;
    if(sents[i].spawned)
    {
        sents[i].spawned = false;
        sents[i].spawnsecs = sec;
        send2(true, sender, SV_ITEMACC, i);
		
		serverrealpickup(i,clients[sender].representer); //die nummer des items, und das dynent object dass das item bekommen soll

    };
};

void resetvotes()
{
    loopv(clients) clients[i].mapvote[0] = 0;
};

bool vote(char *map, int reqmode, int sender)
{
    strcpy_s(clients[sender].mapvote, map);
    clients[sender].modevote = reqmode;
    int yes = 0, no = 0; 
    loopv(clients) if(clients[i].type!=ST_EMPTY)
    {
        if(clients[i].mapvote[0]) { if(strcmp(clients[i].mapvote, map)==0 && clients[i].modevote==reqmode) yes++; else no++; }
        else no++;
    };
    if(yes==1 && no==0) return true;  // single player
    sprintf_sd(msg)("%s suggests %s on map %s (set map to vote)", clients[sender].name, modestr(reqmode), map);
    sendservmsg(msg);
    if(yes/(float)(yes+no) <= 0.5f) return false;
    sendservmsg("vote passed");
    resetvotes();
    return true;    
};

// server side processing of updates: does very little and most state is tracked client only
// could be extended to move more gameplay to server (at expense of lag)

void process(ENetPacket * packet, int sender)   // sender may be -1
{
    if(ENET_NET_TO_HOST_16(*(ushort *)packet->data)!=packet->dataLength)
    {
        disconnect_client(sender, "packet length");
        return;
    };
        
    uchar *end = packet->data + packet->dataLength;
    uchar *p = packet->data+2; // die �bertragenen daten
    char text[MAXTRANS];
    int cn = -1, type;



    while(p<end) switch(type = getint(p))
    {
        case SV_TEXT:
            sgetstr();
            break;

        case SV_INITC2S:
            sgetstr();
            strcpy_s(clients[cn].name, text);
            sgetstr();
            getint(p);
            break;

        case SV_MAPCHANGE:
        {
            sgetstr();
            int reqmode = getint(p);
            if(reqmode<0) reqmode = 0;
            if(smapname[0] && !mapreload && !vote(text, reqmode, sender)) return;
            mapreload = false;
            mode = reqmode;
            minremain = mode&1 ? 15 : 10;
            mapend = lastsec+minremain*60;
            interm = 0;
            strcpy_s(smapname, text);
            resetitems();
            sender = -1;
            break;
        };
		
		//Teamprojekt, leicht modifizierte kopie von clients2c
		case SV_DAMAGE: //schaden an interne repr�sentanten vergeben           
        {

			int target = getint(p); //ziel==clientnummer
			int damage = getint(p); //damage
			int ls = getint(p); //lifesequenze.. also welches "leben" aktuell is, durchnummeriert
			if(isdedicated)
			{
				std::cout << "ORIGIN: " << cn << " TARGET: " << target << " DAMAGE " << damage << " LIFESEQ " << ls;
			
				loopv(clients)
				{
					if(clients[i].clientnr == target)
					{
						serverselfdamage(damage,cn,clients[i].representer,i); //schaden, von wem, an wen(dynent),an wen(nr) 
						std::cout <<" \n make server damage \n ";
					}
				}
			}

						/* TP
			Cheatschutz: verhindert, dass ein Spieler der serverseitig tot ist
			dennoch Items aufsammelt (=z.B. indem er das "kill-signal" des Server ignoriert
			und einfach weiterspielt

			*/

			if(isdedicated && clients[cn].representer->state == CS_DEAD) //unbedingt zuerst auf isdedicated fragen, zugriff auf clients[cn] im sp nicht m�glich!
			{
				disconnect_client(cn,"CHEAT erkannt: falscher Zustand auf Clientseite");
			}
            break;
        };

		case SV_MUN:
		{
			int gun = getint(p);
			if(isdedicated)
			{
				clients[cn].representer->ammo[gun]--;		
				if(clients[cn].representer->ammo[gun] < 0)
				{
					disconnect_client(cn,"CHEAT DETECTED");
				}
			}
			else
			{
				std::cout << " Sending simple Shot Request (SV_MUN)";
			}
			break;

		}
		

		case SV_ALRS:
		{
			uchar *tmp = p; //tempor�re zeigerkopie auf das paket
			tmp--;
			*tmp = SV_DUMMYALRS; //dummykodierung um ALRS bei anderen clients zu verschleiern
			tmp++;
			std::cout << "LESE ARLS-PAKET von CLIENT AUF SERVER \n";
			int rnd = getint(p);
			*tmp = 7; //Dummywert

			if(isdedicated)
			{

				clients[cn].allowRespawn = clients[cn].allowRespawn - rnd;
				std::cout << "RND: " << rnd << " ERGEBNIS : " << clients[cn].allowRespawn;
				if(clients[cn].allowRespawn == 0)
				{
					std::cout << "KORREKTER RESPAWN \n";
					send2(1,cn,SV_ALRS,rnd+1);
					spawnstateForServer(clients[cn].representer);
					clients[cn].representer->state = CS_ALIVE;
					clients[cn].representer->lifesequence++;
					clients[cn].awaitingSpawnSignal = 0;
				
				}
				else
				{
					std::cout << " CLIENT HAT FALSCHE NR GESENDET \n";
				}
			}
			

			/*
			wenn das ARLS paket den server durchl�uft, wird dieses paket auch wieder an alle anderen clients verteilt
			das muss verhindert werden! deswegen wird ein dummy-SV_ALRS message code eingef�hrt, der keinen inhalt besitzt, aber gleiche l�nge hat
			*/
			break;
		};

		case SV_TICKET:
		{



			if(isdedicated)
			{

			}
			break;
		};


		//teamprojekt out
		
        
        case SV_ITEMLIST:
        {
            int n;
            while((n = getint(p))!=-1) if(notgotitems) //der erste verbundene client sendet die itemlist, ab dem zweiten ist hier schluss
            {
                server_entity se = { false, 0 };
                while(sents.length()<=n) sents.add(se);
                sents[n].spawned = true; //am anfang sollen alle items "gespawned" sein
				sents[n].type = getint(p); //TP TEST
				std::cout << " \n\n INR: " << n << " << TYPE: " << (int)sents[n].type << "\n\n";
            };
            notgotitems = false;
            break;
        };

        case SV_ITEMPICKUP:
        {
            int n = getint(p);
            pickup(n, getint(p), sender); //ist sender == cn??

			/* TP
			Cheatschutz: verhindert, dass ein Spieler der serverseitig tot ist
			dennoch Items aufsammelt (=z.B. indem er das "kill-signal" des Server ignoriert
			und einfach weiterspielt

			*/

			if(isdedicated && clients[cn].representer->state == CS_DEAD) //unbedingt zuerst auf isdedicated fragen, zugriff auf clients[cn] im sp nicht m�glich!
			{
				disconnect_client(cn,"CHEAT erkannt: falscher Zustand auf Clientseite");
			}

            break;
        };

        case SV_PING:
            send2(false, cn, SV_PONG, getint(p));
            break;

        case SV_POS:
        {
			//SV_POS steht am anfang jedes pakets!
            cn = getint(p); //hier wird die clientnummer des clients herausgenommen, der dieses paket geschickt hat, kann �berall im konstrukt verwendet werden, default = -1
            if(cn<0 || cn>=clients.length() || clients[cn].type==ST_EMPTY)
            {
                disconnect_client(sender, "client num");
                return;
            };
            int size = msgsizelookup(type);
            assert(size!=-1);
            loopi(size-2) getint(p);
            break;
        };

        case SV_SENDMAP:
        {
            sgetstr();
            int mapsize = getint(p);
            sendmaps(sender, text, mapsize, p);
            return;
        }

        case SV_RECVMAP:
			send(sender, recvmap(sender));
            return;
            
        case SV_EXT:   // allows for new features that require no server updates 
        {
            for(int n = getint(p); n; n--) getint(p);
            break;
        };

        default:
        {
            int size = msgsizelookup(type);
            if(size==-1) { disconnect_client(sender, "tag type"); return; };
            loopi(size-1) getint(p);
        };
    };

    if(p>end) { disconnect_client(sender, "end of packet"); return; }; 

    multicast(packet, sender);
};

//TP
void serverselfdamage(int damage, int actor, dynent *act,int clientnr)
{
	if(act->state == CS_DEAD) return;

    int ad = damage*(act->armourtype+1)*20/100;     // let armour absorb when possible
    if(ad>act->armour) ad = act->armour;
    act->armour -= ad;
    damage -= ad;
	if((act->health -= damage)<=0)
    {
		std::cout << " \n PLAYER should die now with ... \n" << act->health;
		act->state = CS_DEAD;
		

		if ( clients[clientnr].awaitingSpawnSignal == 1) //hat das �berhaupt eine auswirkung? sicherheitsabfrage falls methode 2x aufgerufen wird (client 2x serverseitig gestorben)
		{
			return;
		}
		uchar rnd = 50; //wuerfel zufallszahl zw 20 und 120, schicke diese zum client
		// client muss diese best�tigen mit rnd+1, damit soll dann der allowRespawn zur�ckgesetzt werden, ist das passiert
		//schickt der server sein OK (rnd+2) an den client, der dann weiterspielen darf
		clients[clientnr].allowRespawn = rnd;
		clients[clientnr].awaitingSpawnSignal = 1;
		send2(true,clientnr,SV_FORCEDIE,actor); //zwinge client zum sterben
		send2(true,clientnr,SV_ALRS,rnd); // //da client tot is, fordere ihn zur respawn-challenge heraus
		
		std::cout << " \n ZUFALLSWERT " << rnd << " \n";
		
	}
	

};
//TP OUT



void send_welcome(int n)
{
    ENetPacket * packet = enet_packet_create (NULL, MAXTRANS, ENET_PACKET_FLAG_RELIABLE);
    uchar *start = packet->data;
    uchar *p = start+2;
    putint(p, SV_INITS2C);
    putint(p, n);
    putint(p, PROTOCOL_VERSION);
    putint(p, smapname[0]);
    sendstring(serverpassword, p);
    putint(p, clients.length()>maxclients);
    if(smapname[0])
    {
        putint(p, SV_MAPCHANGE);
        sendstring(smapname, p);
        putint(p, mode);
        putint(p, SV_ITEMLIST);
        loopv(sents) if(sents[i].spawned) putint(p, i);
        putint(p, -1);
    };
    *(ushort *)start = ENET_HOST_TO_NET_16(p-start);
    enet_packet_resize(packet, p-start);
    send(n, packet);
};

void multicast(ENetPacket *packet, int sender)
{
    loopv(clients)
    {
        if(i==sender) continue;
        send(i, packet);
    };
};

void localclienttoserver(ENetPacket *packet)
{
    process(packet, 0);
    if(!packet->referenceCount) enet_packet_destroy (packet);
};

client &addclient()
{
    loopv(clients) if(clients[i].type==ST_EMPTY) return clients[i];
    return clients.add();
};

void checkintermission()
{
    if(!minremain)
    {
        interm = lastsec+10;
        mapend = lastsec+1000;
    };
    send2(true, -1, SV_TIMEUP, minremain--);
};

void startintermission() { minremain = 0; checkintermission(); };

void resetserverifempty()
{
    loopv(clients) if(clients[i].type!=ST_EMPTY) return;
    clients.setsize(0);
    smapname[0] = 0;
    resetvotes();
    resetitems();
    mode = 0;
    mapreload = false;
    minremain = 10;
    mapend = lastsec+minremain*60;
    interm = 0;
};

int nonlocalclients = 0;
int lastconnect = 0;

//TPBEGIN kopie von originaler spawnsate ggfls modifikationen hier noetig
void spawnstateForServer(dynent *d)              // reset player state not persistent accross spawns
{
    resetmovement(d);
    d->vel.x = d->vel.y = d->vel.z = 0; 
    d->onfloor = false;
    d->timeinair = 0;
    d->health = 100;
    d->armour = 50;
    d->armourtype = A_BLUE;
    d->quadmillis = 0;
    d->lastattackgun = d->gunselect = GUN_SG;
    d->gunwait = 0;
	d->attacking = false;
    d->lastaction = 0;
    loopi(NUMGUNS) d->ammo[i] = 0;
    d->ammo[GUN_FIST] = 1;
    if(m_noitems)
    {
        d->gunselect = GUN_RIFLE;
        d->armour = 0;
        if(m_noitemsrail)
        {
            d->health = 1;
            d->ammo[GUN_RIFLE] = 100;
        }
        else
        {
            if(gamemode==12) { d->gunselect = GUN_FIST; return; };  // eihrul's secret "instafist" mode
            d->health = 256;
            if(m_tarena)
            {
                int gun1 = rnd(4)+1;
                baseammo(d->gunselect = gun1);
                for(;;)
                {
                    int gun2 = rnd(4)+1;
                    if(gun1!=gun2) { baseammo(gun2); break; };
                };
            }
            else if(m_arena)    // insta arena
            {
                d->ammo[GUN_RIFLE] = 100;
            }
            else // efficiency
            {
                loopi(4) baseammo(i+1);
                d->gunselect = GUN_CG;
            };
            d->ammo[GUN_CG] /= 2;
        };
    }
    else
    {
        d->ammo[GUN_SG] = 5;
    };
};

//kopie von entities.cpp
struct serveritemstat { int add, max, sound; } serveritemstats[] =
{
     10,    50, S_ITEMAMMO,
     20,   100, S_ITEMAMMO,
      5,    25, S_ITEMAMMO,
      5,    25, S_ITEMAMMO,
     25,   100, S_ITEMHEALTH,
     50,   200, S_ITEMHEALTH,
    100,   100, S_ITEMARMOUR,
    150,   150, S_ITEMARMOUR,
  20000, 30000, S_ITEMPUP,
};

//modifizierte kopie von etitites.cpp ((ACHTUNG! unterschied ents zu sents (serverentities sind kleiner! werden hier aber gebraucht)
void serverradditem(int i, int &v)
{
	std::cout << " --- ITEM ADDED TO ENTITY " ;
    serveritemstat &is = serveritemstats[sents[i].type-I_SHELLS];
    v += is.add;
    if(v>is.max) v = is.max;
};
//kopie von entities.cpp
void serverrealpickup(int n, dynent *d)
{
	if(!isdedicated) return; //weglassen verursacht einen absturz beim client bei lokalen spielen! h�chstwahrscheinlich weil sents nicht initialisiert wurde
	//code wird aber ohnehin im SP nicht ben�tigt

	std::cout <<" --- PICKING UP ITEM " << n;
    switch(sents[n].type)
    {
        case I_SHELLS:  serverradditem(n, d->ammo[1]); break;
        case I_BULLETS: serverradditem(n, d->ammo[2]); break;
        case I_ROCKETS: serverradditem(n, d->ammo[3]); break;
        case I_ROUNDS:  serverradditem(n, d->ammo[4]); break;
        case I_HEALTH:  serverradditem(n, d->health);  break;
        case I_BOOST:   serverradditem(n, d->health);  break;

        case I_GREENARMOUR:
            serverradditem(n, d->armour);
            d->armourtype = A_GREEN;
            break;

        case I_YELLOWARMOUR:
            serverradditem(n, d->armour);
            d->armourtype = A_YELLOW;
            break;

        case I_QUAD:
            //serverradditem(n, d->quadmillis); //vorerst deaktiviert
            break;
    };
};


//TP OUT


void serverslice(int seconds, unsigned int timeout)   // main server update, called from cube main loop in sp, or dedicated server loop
{
    loopv(sents)        // spawn entities when timer reached
    {
        if(sents[i].spawnsecs && (sents[i].spawnsecs -= seconds-lastsec)<=0)
        {
            sents[i].spawnsecs = 0; //setzt spawnzeit zur�ck, da item wieder verf�gbar ist
            sents[i].spawned = true;
            send2(true, -1, SV_ITEMSPAWN, i);
        };
    }; 
    lastsec = seconds;

	
    if((mode>1 || (mode==0 && nonlocalclients)) && seconds>mapend-minremain*60) checkintermission();
    if(interm && seconds>interm)
    {
        interm = 0;
        loopv(clients) if(clients[i].type!=ST_EMPTY)
        {
            send2(true, i, SV_MAPRELOAD, 0);    // ask a client to trigger map reload
            mapreload = true;
            break;
        };
    };

    resetserverifempty();
    
	/* macht auf den ersten blick keinen sinn: aber: diese methode wird auch im reinen client-betrieb
	ausgef�hrt, wenn das spiel dort ist, bricht es hier ab
	*/

	/*
	hinweis: keine server tick rate??
	*/

		

	
    if(!isdedicated) return;     // below is network only

	int numplayers = 0;
	loopv(clients) if(clients[i].type!=ST_EMPTY) ++numplayers;
	serverms(mode, numplayers, minremain, smapname, seconds, clients.length()>=maxclients);



	//TEAMPROJEKT

	if(seconds-laststatus>60)
	{
		loopv(clients)
		{
			dynent *tmp = clients[i].representer;
			std::cout << "\n HP--> " << (*tmp).health<< "ARMOUR --> " << tmp->armour << " TYPE--> "  << tmp->armourtype << "\n"; //c-style zugriff auf membervar

		}
	}

	//TP ENDE

    if(seconds-laststatus>60)   // display bandwidth stats, useful for server ops
    {
        nonlocalclients = 0;
        loopv(clients) if(clients[i].type==ST_TCPIP) nonlocalclients++;
        laststatus = seconds;     
        if(nonlocalclients || bsend || brec) printf("status: %d remote clients, %.1f send, %.1f rec (K/sec)\n", nonlocalclients, bsend/60.0f/1024, brec/60.0f/1024);
        bsend = brec = 0;
    };

    ENetEvent event;
    if(enet_host_service(serverhost, &event, timeout) > 0)
    {
        switch(event.type)
        {
            case ENET_EVENT_TYPE_CONNECT:
            {
				/*
				An dieser Stelle verbindet sich der Client mit dem Server
				*/

                client &c = addclient();
                c.type = ST_TCPIP;
                c.peer = event.peer;
                c.peer->data = (void *)(&c-&clients[0]);
				//TEAMPROJEKT
				c.representer=new dynent();
				spawnstateForServer(c.representer);
				c.representer->state = CS_ALIVE;
				//c.allowRespawn = -1; //alte fassung
				c.allowRespawn = 0;


				/// TP OUT
                char hn[1024];
                strcpy_s(c.hostname, (enet_address_get_host(&c.peer->address, hn, sizeof(hn))==0) ? hn : "localhost");
                printf("client connected (%s)\n", c.hostname);
                send_welcome(lastconnect = &c-&clients[0]);
				c.clientnr = lastconnect; //TEAMPROJEKT
                break;
            }
            case ENET_EVENT_TYPE_RECEIVE: //statusabfrage von enet, siehe doku von enet
                brec += event.packet->dataLength;
                process(event.packet, (int)event.peer->data); //processing methode, versenden der daten an weitere clients)
				/* man muss hier unterscheiden zwischen event.packet (enth�lt die daten) und
				event.peer (informationen �ber den client)
				*/

                if(event.packet->referenceCount==0) enet_packet_destroy(event.packet);
                break;

            case ENET_EVENT_TYPE_DISCONNECT: 
                if((int)event.peer->data<0) break;
                printf("disconnected client (%s)\n", clients[(int)event.peer->data].hostname);
                clients[(int)event.peer->data].type = ST_EMPTY;
                send2(true, -1, SV_CDIS, (int)event.peer->data);
                event.peer->data = (void *)-1;
                break;
        };
        
        if(numplayers>maxclients)   
        {
            disconnect_client(lastconnect, "maxclients reached");
        };
    };
    #ifndef WIN32
        fflush(stdout);
    #endif
};

void cleanupserver()
{
    if(serverhost) enet_host_destroy(serverhost);
};

void localdisconnect()
{
    loopv(clients) if(clients[i].type==ST_LOCAL) clients[i].type = ST_EMPTY;
};

void localconnect()
{
    client &c = addclient();
    c.type = ST_LOCAL;
    strcpy_s(c.hostname, "local");
    send_welcome(&c-&clients[0]); 
};

void initserver(bool dedicated, int uprate, char *sdesc, char *ip, char *master, char *passwd, int maxcl, int port)
{
	/*
	dedicated = true wird �bergeben fall server.bat gestartet, siehe main.c, dort wird dieses argument abgefangen
	*/

    serverpassword = passwd;
    maxclients = maxcl;
	servermsinit(master ? master : "wouter.fov120.com/cube/masterserver/", sdesc, dedicated);
    

    if(isdedicated = dedicated) //das hier ist eine zuweisung! achtung!
    {
        ENetAddress address = { ENET_HOST_ANY, port };
        if(*ip && enet_address_set_host(&address, ip)<0) printf("WARNING: server ip not resolved");
        serverhost = enet_host_create(&address, MAXCLIENTS, 0, uprate);
        if(!serverhost) fatal("could not create server host\n");
        loopi(MAXCLIENTS) serverhost->peers[i].data = (void *)-1;
		
    };
    resetserverifempty();

	/*
	falls das spiel dedicated ist, dann l�uft es hier unten in den main loop, also nicht wie bei main.c
	dort in den loop, das passiert nur bei clients!
	*/

    if(isdedicated)       // do not return, this becomes main loop
    {
        #ifdef WIN32
        SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
        #endif
        printf("dedicated server started, waiting for clients...\nCtrl-C to exit\n\n");
        atexit(cleanupserver);
        atexit(enet_deinitialize);
        for(;;) serverslice(/*enet_time_get_sec()*/time(NULL), 5);
    };
};



/*
Ab hier folgt eine modifizerte Fassung f�r den SSL Connect vom Spielserver zum Lizenserver
dort wird �berpr�ft, ob ein neu verbundener CLient ein g�ltiges Ticket besitzt..

*/

enum { max_length = 1024};

class serverToLicenseSystem
{
public:
    std::stringstream ss;	// Hier wird die vollstaendige Antwort des Servers gespeichert
    std::string response;	// Hier wird der HTTP Body der Antwort gespeichert
	std::string clientTicket;
		
	serverToLicenseSystem(boost::asio::io_service& io_service,
      boost::asio::ssl::context& context,
      boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
    : socket_(io_service, context)
  {
    socket_.set_verify_mode(boost::asio::ssl::verify_peer);
    socket_.set_verify_callback(
        boost::bind(&serverToLicenseSystem::verify_certificate, this, _1, _2));

    boost::asio::async_connect(socket_.lowest_layer(), endpoint_iterator,
        boost::bind(&serverToLicenseSystem::handle_connect, this,
          boost::asio::placeholders::error));
  }
  // Zertifikat 
  bool verify_certificate(bool preverified,
      boost::asio::ssl::verify_context& ctx)
  {
    // The verify callback can be used to check whether the certificate that is
    // being presented is valid for the peer. For example, RFC 2818 describes
    // the steps involved in doing this for HTTPS. Consult the OpenSSL
    // documentation for more details. Note that the callback is called once
    // for each certificate in the certificate chain, starting from the root
    // certificate authority.

    // In this example we will simply print the certificate's subject name.
    char subject_name[256];
    X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
    X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
    std::cout << "Verifying " << subject_name << "\n";

    return preverified;
  }

  void handle_connect(const boost::system::error_code& error)
  {
    if (!error)
    {
      socket_.async_handshake(boost::asio::ssl::stream_base::client,
          boost::bind(&serverToLicenseSystem::handle_handshake, this,
            boost::asio::placeholders::error));
    }
    else
    {
      std::cout << "Connect failed: " << error.message() << "\n";
    }
  }

  void handle_handshake(const boost::system::error_code& error)
  {
    if (!error)
    {
	  size_t request_length = strlen(request_);
	  // Bei der Erstverbindung werden die Benutzerdaten mitgesendet
	  {
		  boost::asio::async_write(socket_,
			  boost::asio::buffer("GET /sCheck_Connecting_User.php?ticket=" + clientTicket + " HTTP/1.1\r\nHost: localhost\r\n\r\n"),
			  boost::bind(&serverToLicenseSystem::handle_write, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
		  std::cout << " Schicke ClientTicket zum Lizenzserver....";
	  } 
    }
    else
    {
      std::cout << "Handshake failed: " << error.message() << "\n";
    }
  }

  void handle_write(const boost::system::error_code& error,
      size_t bytes_transferred)
  {
    if (!error)
    {
		socket_.async_read_some(boost::asio::buffer(reply_, bytes_transferred),boost::bind(&serverToLicenseSystem::handle_read, this,
               boost::asio::placeholders::error,
               boost::asio::placeholders::bytes_transferred));
    }
    else
    {
      std::cout << "Write failed: " << error.message() << "\n";
    }
  }

  void handle_read(const boost::system::error_code& error,
      size_t bytes_transferred)
  {
    if (!error)
    {
		ss << std::string(reply_, bytes_transferred);
      socket_.async_read_some(boost::asio::buffer(reply_, max_length),
          boost::bind(&serverToLicenseSystem::handle_read, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
    }
	else if (0 != bytes_transferred)
	       {
	           std::cout <<"\n" << "Quit";
	       }
  }


std::string urlencode(const std::string &c)
{
    
    std::string escaped="";
    int max = c.length();
    for(int i=0; i<max; i++)
    {
        if ( (48 <= c[i] && c[i] <= 57) ||//0-9
             (65 <= c[i] && c[i] <= 90) ||//abc...xyz
             (97 <= c[i] && c[i] <= 122) || //ABC...XYZ
             (c[i]=='~' || c[i]=='!' || c[i]=='*' || c[i]=='(' || c[i]==')' || c[i]=='\'')
        )
        {
            escaped.append( &c[i], 1);
        }
        else
        {
            escaped.append("%");
            escaped.append( char2hex(c[i]) );//converts char 255 to string "ff"
        }
    }
    return escaped;
}

std::string char2hex( char dec )
{
    char dig1 = (dec&0xF0)>>4;
    char dig2 = (dec&0x0F);
    if ( 0<= dig1 && dig1<= 9) dig1+=48;    //0,48inascii
    if (10<= dig1 && dig1<=15) dig1+=97-10; //a,97inascii
    if ( 0<= dig2 && dig2<= 9) dig2+=48;
    if (10<= dig2 && dig2<=15) dig2+=97-10;

    std::string r;
    r.append( &dig1, 1);
    r.append( &dig2, 1);
    return r;
}

private:
  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;
  char request_[max_length];
  char reply_[max_length];
  std::string license;

  
};

int check_client_license_from_server(std::string cTicket, int clientNr)
{
  try
  {
	 
	// Objekte der Boost::Asio-Bibliothek f�r asynchrone Kommunikation
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::resolver resolver(io_service);
    // Definiere ein Query mit Host (DNS Namen erlaubt) und Port
	boost::asio::ip::tcp::resolver::query query("127.0.0.1","443");
	// L�se DNS Namen in IP-Adresse auf
    boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);
	// SSL Kontext wird erzeugt
    boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);
	ctx.load_verify_file("server.crt");
    serverToLicenseSystem c(io_service, ctx, iterator);
	
	c.clientTicket = cTicket;
	// Blockierender Aufruf, der bis zum Erhalt aller Daten vom Server wartet
    io_service.run();
	// HTTP Parser, der den HTTP Header vom HTTP Body trennt und den Body in c.repsonse speichert
	char del = '\n';
	std::string item;
	bool headerEnd = false;
	while(std::getline(c.ss, item, del)) {
		if(headerEnd && item != "\r") {
			c.response.append(item);
		}
		// HTTP Body beginnt mit naechster Zeile
		if(item.compare("Content-Type: text/html\r")==0)
			headerEnd = true;
	}
	// Falls die Authentifizierung erfolgreich war, sendet der Server "True" konkateniert mit dem Ticket zur�ck
	if(c.response.substr(0,5).compare("False")==0) 
	{
		std::cout << " CLient hat kein g�ltiges Ticket! ";
		disconnect_client(clientNr,"kein g�ltiges Ticket");
	}
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}

