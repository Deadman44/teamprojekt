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
	std::string authname; //"echter" name des Spieler, also der auf dem Lizenzserver in Form der Email
	dynent *representer; //spielfigur des clients auf dem server
	int clientnr; //interne clientnummer
	int awaitingSpawnSignal; //zeigt an, ob client bereits eine spawnanfrage geschickt hat
	int allowRespawn; //sollte auf 0 stehen wenn respawn erlaubt is bzw der client neue pakete senden darf, enth�lt im zwischenzustand einen zufallswert)
	std::string clientSAT; //aktuelles server-access-ticket des clients
	std::string clientName; //clientname, also email
	int allowconnect; // deprecated
	int firstPacketsArrived; //zeigt mit 1 an, ob bereits eine SV_POS Message an den Server gesendet wurde. wird ben�tigt, damit server clients vom server werfen kann, die 
	//die nicht �ber den SAT-Mechanismus verf�gen, also �berhaupt keine SAT-Messages verschicken
	int temporaryPacketCounter; //z�hlt die datenpakete des clients in einem zeitraum von 5 sekunden
	int currentPacketCheckTime; //die zeit, an der zuletzt die zahl der pakete von 0 hochgez�hlt wurden
	int currentWeaponFiredTime; // Z�hler wie oben, jedoch f�r den Einsatz bei der schussfrequenz bestimmt
	int secondSATwaitTime;
	int SATacks; //wie h�ufig f�r den spieler ein SAT akzeptiert wurde (hier: max 2x)

	//speedhack erkennung 2: modifikation der geschwindigkeitsvariabler erkennen
	unsigned int lastx;
	unsigned int lasty;
	unsigned int currx;
	unsigned int curry;
	int posViolations;

	//waffen-kadenz-cheat
	int weaponsfired[5];



};

//TP



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

	//std::stringstream sstm;
	//sstm << "Spieler vom Server geworfen .... " << clients[n].authname;
	//messageLogger->writeToLog(sstm.str());


    printf("disconnecting client (%s) [%s]\n", clients[n].hostname, reason);
    enet_peer_disconnect(clients[n].peer);
    clients[n].type = ST_EMPTY;
	clients[n].temporaryPacketCounter = 0;
    send2(true, -1, SV_CDIS, n);
};

void resetitems() { sents.setsize(0); notgotitems = true; };


int countPlayers()
{
	int i = 0;
	for(int u = 0; u < clients.length();u++)
	{
		if(clients[u].type = ST_TCPIP)i++;
	}
	std::cout << " PLAYERS : " << i  << " \n";
	return i;
}

void pickup(uint i, int sec, int sender)         // server side item pickup, acknowledge first client that gets it
{
    if(i>=(uint)sents.length()) return;
    if(sents[i].spawned)
    {
        sents[i].spawned = false;
		{ //Teamprojekt: Serverseitige Einstellung der Spawntimer f�r jeden Gegenstand
			int players = countPlayers();
			players++;

			players = players<3 ? 4 : (players>4 ? 2 : 3);         // spawn times are dependent on number of players
			int ammo = players*2;

			if(sents[i].type >=3 && sents[i].type <= 6)
			{
				 sents[i].spawnsecs = ammo;
			}
			else if(sents[i].type == I_HEALTH)
			{
				sents[i].spawnsecs = players*5;
			}
			else if(sents[i].type == I_BOOST)
			{
				sents[i].spawnsecs = 60;
			}
			else if(sents[i].type == I_GREENARMOUR || sents[i].type == I_YELLOWARMOUR)
			{
				sents[i].spawnsecs = 20;
			}
			else if(sents[i].type == I_QUAD)
			{
				sents[i].spawnsecs = 60;
			}
			//TP OUT


		}


        send2(true, sender, SV_ITEMACC, i);
		
		//TEAMPROJEKT
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


//TP

void setSATacks(int clientnr)
{
	clients[clientnr].SATacks++;
}

int getSATacks(int clientnr)
{
	return clients[clientnr].SATacks;
}

void setClientSATWaitTime(int clientnr)
{

	clients[clientnr].secondSATwaitTime =0;
}

// setzt den "echten" Namen des Spielers auf die Email des registrierten Benutzers
void setAuthName(int clientnr, std::string authname)
{
	clients[clientnr].authname = authname;
}


void checkSecondSATTime(int clientnr, int millis)
{
	int waittime = clients[clientnr].secondSATwaitTime;
	if(waittime != 0)
	{
		int diff = millis - waittime;
		if(millis - waittime > 30)
		{
			clients[clientnr].secondSATwaitTime = 0; //reset, ggfls wichtig wenn spieler erneut joinen will
			std::stringstream sstm;
			sstm << "Spieler abgelehnt: Fehler bei authentifizierung, ung�ltiges zweites SAT" << clients[clientnr].authname;
			messageLogger->writeToLog(sstm.str());
			disconnect_client(clientnr,"NO VALID SECOND SAT in 30 SECONDS");
		}

	}

}




/*

diese Methode pr�ft, wie h�ufig eine Waffe innerhalb eines 1-Sekunden Intervalls abgefeuert wurde.
Die Schussfrequenz ist fest eingebaut im Spiel und l�sst sich nur durch Hacks k�nstlich erh�hen.
Dies wird hier erkannt.

*/


void checkWeaponFireRate(int clientnr, int millis)
{
	bool violation = false;
	if(millis-clients[clientnr].currentWeaponFiredTime > 1)
	{		
			for(int u = 0; u  < 5; u++)
			{
				std::cout << " Waffe " << u << " --- Schuss: " << clients[clientnr].weaponsfired[u] << "\n";
			}
			for(int u = 0; u < 5; u++)
			{
				clients[clientnr].weaponsfired[u] = 0;
			}
			clients[clientnr].currentWeaponFiredTime = time(NULL); //zur�cksetzen der uhr
	}

	if(clients[clientnr].weaponsfired[0] > 9)
		{
			std::cout << " VIO FIST " << clients[clientnr].weaponsfired[0] << "\n";
			violation = true;
		}
		else if(clients[clientnr].weaponsfired[1] > 3)
		{
			std::cout << " VIO sg " << clients[clientnr].weaponsfired[1] << "\n";
			violation = true;
		}
		else if(clients[clientnr].weaponsfired[2] >19)
		{
			std::cout << " VIO mini " << clients[clientnr].weaponsfired[2] << "\n";
			violation = true;
		}
		else if(clients[clientnr].weaponsfired[3] > 4)
		{
			std::cout << " VIO rl " << clients[clientnr].weaponsfired[3] << "\n";
			violation = true;
		}
		else if(clients[clientnr].weaponsfired[4] > 3)
		{
			std::cout << " VIO rifle " << clients[clientnr].weaponsfired[4] << "\n";
			violation = true;
		}

		if(violation)
		{
			std::cout << " Feuerraten-Cheat erkannt " << "\n";
			boost::thread checkworker(increment_suspect_status,5,clients[clientnr].clientName);	//ANTICHEAT
			std::stringstream sstm;
			sstm << "Feuerraten-Cheat erkannt " << clients[clientnr].authname;
			messageLogger->writeToLog(sstm.str());
			
			disconnect_client(clientnr,"FireRate to high, CHEAT DETECTED");
		}

}

/*
Diese Methode wird bei jedem Empfang einer SV_POS Message eines Spielers aufgerufen. SV_POS ist zwignend f�r jedes Datenpaket. 
Sollte der Spieler einen Speedhack benutzen (beispielsweise der in der CheatEngine eingebaute) so werden vom Client unnat�rlich viele 
Datenpakete verschickt. Ein normaler Client versendet etwa 300-350 alle 10 Sekunden. Hier wurde ein Toleranzfaktor von 1.2 benutzt, damit nicht bei
pl�tzlich auftretendem Packetloss in einer Zeitspanne von 5 Sekunden zu viele Pakete ankommen und f�lschlicherweise gekickt wird.


zweite Speedhackerkennung:
Ein Spieler hat eine maximale Geschwindigkeit von 22 "Cubes" pro Sekunde. Daher l�sst sich grob vorhersagen wie "schnell" sich
ein Spieler bewegen kann. Jede ankommende SV_POS Message enth�lt die x,y und z Koordinaten eines Spieler auf der Karte.
Durch experimentelles Vorgehen konnten die maximalen Unterschiede ermittelt werden, die die x und y Koordinaten zwischen zwei SV_POS Messages
annehmen k�nnen. Es werden daher die Koordinaten aus der letzten Nachrichten mit den Koordinaten aus der aktuellen Nachricht miteinander verglichen.
ALle Verst��e dieser Obergrenze werden vermerkt. �bersteigen diese Verst��e innerhalb von 10 Sekunden einen bestimmten Schwellwert, wird der Spieler
vom Server geworfen und verwarnt.
Der Schwellwert wurde experimentell herausgefunden. Zu beachten ist, dass es Teleporter und �nhliche Einrichtungen auf vielen Karten gibt, die die Geschwindigkeit des Spielers
k�nstlich erh�hen.
*/
void incrementPacketCounter(int clientnr, int millis)
{
	



	if(millis-clients[clientnr].currentPacketCheckTime >10)
	{
		//std::cout << "Renew PacketCheckTime @ " << clients[clientnr].temporaryPacketCounter << " PACKETS \n";
		clients[clientnr].currentPacketCheckTime = time(NULL);
		clients[clientnr].temporaryPacketCounter = 0;
		clients[clientnr].posViolations = 0; //setzt die Anzahl der "Positionsverletzungen" 

	}

	int xdiff = clients[clientnr].lastx - clients[clientnr].currx;
	int ydiff = clients[clientnr].lasty - clients[clientnr].curry;

	//Positionsverletzungen melden, getrennt f�r x und y-Achse. Z-Achse ignoriert, da nicht prim�r wichtig
	if( abs(xdiff) > 22)
	{
		clients[clientnr].posViolations++;
		std::cout << " X VIO \n";
	}
	if( abs(ydiff) > 22)
	{
		clients[clientnr].posViolations++;
		std::cout << " Y VIO \n";
	}

	clients[clientnr].temporaryPacketCounter++;
	if(clients[clientnr].temporaryPacketCounter > 300) //standardwert sollte zwischen 25 und 35 innerhalb von 1 sekunde liegen, leichte toleranz wegen packetloss usw
	{
		std::cout << " POSSIBLE SPEEDHACK//Packetloss--> PLAYER " << clients[clientnr].clientName << "  KICK! " << clients[clientnr].temporaryPacketCounter << "\n";
		boost::thread checkworker(increment_suspect_status,5,clients[clientnr].clientName);	//ANTICHEAT
		std::stringstream sstm;
		sstm << "SPEEDHACK ERKANNT (TIMER) " << clients[clientnr].authname;
		messageLogger->writeToLog(sstm.str());
		disconnect_client(clientnr,"SPEEDHACK -- TIMER");

	}

	//Bei zu vielen Verletzungen der Positionen Spieler vom Server werfen und Versto� an Lizenzserver melden
	if(clients[clientnr].posViolations > 50)
	{
		std::cout << " POSSIBLE SPEEDHACK--> PLAYER " << clients[clientnr].clientName << "  KICK! " << clients[clientnr].posViolations << "\n";
		boost::thread checkworker(increment_suspect_status,5,clients[clientnr].clientName);	//ANTICHEAT
		std::stringstream sstm;
		sstm << "SPEEDHACK ERKANNT " << clients[clientnr].authname;
		messageLogger->writeToLog(sstm.str());
		disconnect_client(clientnr,"SPEEDHACK");
	}
}

//TP OUT


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
			//TP, sorgt daf�r, dass auch nach kartenwechsel//modiwechsel die korrekten states der spieler existieren
			if(isdedicated)
			{

				for(int uu = 0; uu < clients.length(); uu++)
				{
					spawnstateForServer(clients[uu].representer);
				}

			}
			
            break;
        };
		
		//Teamprojekt, leicht modifizierte kopie von clients2c
		case SV_DAMAGE: //schaden an interne repr�sentanten vergeben           
        {

			int target = getint(p); //ziel==clientnummer
			int damage = getint(p); //damage
			int ls = getint(p); //lifesequenz.. also welches "leben" aktuell is, durchnummeriert
			if(isdedicated)
			{
				std::cout << "ORIGIN: " << cn << " TARGET: " << target << " DAMAGE " << damage << " LIFESEQ " << ls;
			
				loopv(clients)
				{
					if(clients[i].clientnr == target ) //&& clients[i].representer->lifesequence == ls //tempor�r raus, wegen bugs bei suizid
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

			if(isdedicated && clients[cn].representer->state == CS_DEAD && target != clients[cn].clientnr) //unbedingt zuerst auf isdedicated fragen, zugriff auf clients[cn] im sp nicht m�glich!
			{
				boost::thread checkworker(increment_suspect_status,5,clients[cn].clientName);	//ANTICHEAT
				std::stringstream sstm;
				sstm << "CHEAT erkannt: falscher Zustand auf Clientseite //DAMAGE" << clients[cn].authname;
				messageLogger->writeToLog(sstm.str());
				disconnect_client(cn,"CHEAT erkannt: falscher Zustand auf Clientseite //DAMAGE");
			}
            break;
        };

		case SV_MUN:
		{
			int gun = getint(p);
			if(isdedicated)
			{
				clients[cn].representer->ammo[gun]--;		
				if(clients[cn].representer->ammo[gun] < 0 && gun != 0 )
				{
					boost::thread checkworker(increment_suspect_status,5,clients[cn].clientName);	//ANTICHEAT

					std::stringstream sstm;
					sstm << "Munition Cheat-Versuch " << clients[cn].authname;
					messageLogger->writeToLog(sstm.str());
					disconnect_client(cn,"CHEAT DETECTED //MUNITION");
				}

				if(gun == GUN_FIST)
				{
					clients[cn].weaponsfired[0]++;
				}
				else if(gun == GUN_SG)
				{
					clients[cn].weaponsfired[1]++;
				}
				else if(gun == GUN_CG)
				{
					clients[cn].weaponsfired[2]++;
				}
				else if(gun == GUN_RL)
				{
					clients[cn].weaponsfired[3]++;
				}
				else if(gun == GUN_RIFLE)
				{
					clients[cn].weaponsfired[4]++;
				}


			}
			else //dieser Teil wird auf dem Client ausgegeben
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
				if(clients[cn].allowRespawn == 0) // Client muss gleiche RND zur�cksenden
				{
					std::cout << "KORREKTER RESPAWN \n";
					send2(1,cn,SV_ALRS,rnd+1);
					std::string clientname = clients[cn].authname;
					messageLogger->writeToLog("Spieler respawn... " + clientname);
					spawnstateForServer(clients[cn].representer);
					clients[cn].representer->state = CS_ALIVE;
					clients[cn].representer->lifesequence++;
					clients[cn].awaitingSpawnSignal = 0;
				
				}
				else
				{
					std::cout << " CLIENT HAT FALSCHE NR GESENDET \n";
				
					std::stringstream sstm;
					sstm << "HP Cheat-Versuch " << clients[cn].authname;
					messageLogger->writeToLog(sstm.str());
					disconnect_client(cn,"HP CHEAT VERSUCH");
				}
			}
			

			/*
			wenn das ARLS paket den server durchl�uft, wird dieses paket auch wieder an alle anderen clients verteilt
			das muss verhindert werden! deswegen wird ein dummy-SV_ALRS message code eingef�hrt, der keinen inhalt besitzt, aber gleiche l�nge hat
			*/
			break;
		};

		case SV_SAT: //SAT +username==email herausfiltern
		{


			int len = getint(p);
			
			uchar *tmp = p;

			std::cout << len << " <--- SAT_LEN \n";
			char *username = new char[len-5];
			char *cSAT = new char[7];
			//SAT aus paket nehmen
			
			for(int z = 0; z < 6; z++)
			{
				cSAT[z] = getint(p);
				*tmp = 0;
				tmp++;


			}
			cSAT[6] = '\0';




			std::cout << " THIS IS THE SAT STRING: " << cSAT;
			//username aus paket nehmen
			for(int u = 0; u < len-6; u++) // -SAT LEN
			{
				username[u] = getint(p);
				*tmp = 0;
				tmp++;
				
			}
			username[len-6] = '\0';

			std::cout << " THIS IS THE USR STRING: " << username;
			if(isdedicated) //standardproblem: auch im lokalen spiel greift client auf server-fkts zu
			{
				
				if(clients[cn].secondSATwaitTime ==0) //Username String nur dann neu setzen, wenn erstes SAT angekommen ist
					//verhindert einen exploit, wennn ein angreifert 2 sats von 2 verschiedenen usern nimmt
				{
					clients[cn].clientName = std::string(username); //konstruktor, wichtig da 0-bytes fehlen..
				}

				clients[cn].clientSAT = std::string(cSAT);

				std::cout << " DER EMPFANGENE STRING LAUTET " << clients[cn].clientName << " und der SAT " << clients[cn].clientSAT << "\n";
				std::cout << " �berpr�fe SAT... ";

				
				boost::thread checkworker(check_SAT,cn,clients[cn].clientName,
				clients[cn].clientSAT);
				
				
			}

			delete[] username;
			delete[] cSAT;

			if(clients[cn].secondSATwaitTime == 0)
			{
				clients[cn].secondSATwaitTime = time(NULL); //Zeitpunkt festlegen, ab dem auf das zweite SAT gewartet wird.
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
				sents[n].type = getint(p); //TP 
				//std::cout << " \n\n INR: " << n << " << TYPE: " << (int)sents[n].type << "\n\n";
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
				
				std::stringstream sstm;
				sstm << "CHEAT erkannt: falscher Zustand auf Clientseite //PICKUP " << clients[cn].authname;
				messageLogger->writeToLog(sstm.str());
				disconnect_client(cn,"CHEAT erkannt: falscher Zustand auf Clientseite //PICKUP");
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
            //loopi(size-2) getint(p);

            unsigned int xco   = getint(p);
            unsigned int yco   = getint(p);
            int zco   = getint(p);
            int yawco   = getint(p)/DAF;
            int pitchco = getint(p)/DAF;
            int rollco  = getint(p)/DAF;
            int velx = getint(p)/DAF;
            int vely = getint(p)/DVF;
            int velz = getint(p)/DVF;
			int f = getint(p);

			//TP
			if(isdedicated)
			{
				clients[cn].lastx = clients[cn].currx;
				clients[cn].lasty = clients[cn].curry;
				clients[cn].currx = xco;
				clients[cn].curry = yco;


				if(clients[cn].firstPacketsArrived == 0) //wird sowohl bei speedhack als auch SAT ben�tigt
				{
					clients[cn].firstPacketsArrived = 1; //TP, erstes "richtige" paket vom client empfangen
					clients[cn].currentPacketCheckTime = time(NULL); //erstes setzen der "uhr"
					clients[cn].currentWeaponFiredTime = time(NULL); //setzen der Uhr f�r die �berpr�fung der Feuerrate
				}
				else
				{
					int currTime = time(NULL);
					incrementPacketCounter(cn,currTime);
					checkWeaponFireRate(cn,currTime);
				}
			}
			//TPOUT
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
		uchar rnd = rand() % 100 + 1; //wuerfel zufallszahl zw 1 und 100, schicke diese zum client
		// client muss diese best�tigen mit rnd, damit soll dann der allowRespawn zur�ckgesetzt werden, ist das passiert
		//schickt der server sein OK (rnd+1) an den client, der dann weiterspielen darf
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
	if(isdedicated) messageLogger->writeToLog("Sende Willkommensnachricht an neuen Spieler..."); //TP
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
	//if(isdedicated)messageLogger->writeToLog("Starte Server mit neuer Karte");
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

//TPBEGIN kopie von originaler spawnsate 
//modifiziert: gamemode erkennung (andere variable)
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
    if(mode >=4)
    {
        d->gunselect = GUN_RIFLE;
        d->armour = 0;
        if(mode <=5)
        {
            d->health = 1;
            d->ammo[GUN_RIFLE] = 100;
        }
        else
        {
            if(mode==12) { d->gunselect = GUN_FIST; return; };  // eihrul's secret "instafist" mode
            d->health = 256;
            if(mode >=10)
            {
                int gun1 = rnd(4)+1;
                baseammo(d->gunselect = gun1);
                for(;;)
                {
                    int gun2 = rnd(4)+1;
                    if(gun1!=gun2) { baseammo(gun2); break; };
                };
            }
            else if(mode >=8)    // insta arena
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
			if(clients[i].type != ST_EMPTY)
			{
				dynent *tmp = clients[i].representer;
				std::cout << " \n NAME: " << clients[i].clientName << "  HP--> " << (*tmp).health<< "ARMOUR --> " << tmp->armour << " ARMORTYPE--> "  << tmp->armourtype;

			}
			

		}
		std::cout << " \n";
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
				c.allowRespawn = 0;
				c.clientName ="EMPTY";
				c.firstPacketsArrived = 0;
				c.representer->lifesequence = 0;
				c.secondSATwaitTime = 0;
				
				messageLogger->writeToLog("Neuer Spieler verbindet sich..");
				/// TP OUT
                char hn[1024];
                strcpy_s(c.hostname, (enet_address_get_host(&c.peer->address, hn, sizeof(hn))==0) ? hn : "localhost");
                printf("client connected (%s)\n", c.hostname);
                send_welcome(lastconnect = &c-&clients[0]);
				c.clientnr = lastconnect; //TEAMPROJEKT
                break;
            }
            case ENET_EVENT_TYPE_RECEIVE: //Empfange normales Paket...
                brec += event.packet->dataLength;
                process(event.packet, (int)event.peer->data); //processing methode, versenden der daten an weitere clients)
				/* man muss hier unterscheiden zwischen event.packet (enth�lt die daten) und
				event.peer (informationen �ber den client)
				*/

                if(event.packet->referenceCount==0) enet_packet_destroy(event.packet);

				//TP der spieler sollte sp�testens beim zweiten Paket ein (=das erste) g�ltiges SAT geschickt haben
				for(int p = 0; p < clients.length();p++)
				{
					if(clients[p].firstPacketsArrived ==1 && (clients[p].clientName.compare("EMPTY") == 0))
					{
						messageLogger->writeToLog("Spieler abgewiesen, falsche Client-Version");
						disconnect_client(p, "NO PERMISSION TO JOIN: UNKNOWN CLIENT");
					}

					checkSecondSATTime(p, time(NULL)); //�berpr�ft, ob im bestimmten Zeitintervall das zweite SAT angekommen ist 


				}
				//TP OUT
                break;

            case ENET_EVENT_TYPE_DISCONNECT: 
                if((int)event.peer->data<0) break;
				//TP
				std::string cname = clients[(int)event.peer->data].authname;
				messageLogger->writeToLog("Spieler verl�sst den Server... " + cname);
				//TP OUT
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
	//servermsinit(master ? master : "wouter.fov120.com/cube/masterserver/", sdesc, dedicated); TP, auskommentiert
    

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
		//TP
		messageLogger = new Logger();
		messageLogger->startLog();
		messageLogger->writeToConsole("Log gestartet...");
		messageLogger->writeToLog("LOG STARTED");
		//TP OUT
        atexit(cleanupserver);
        atexit(enet_deinitialize);
        for(;;) serverslice(/*enet_time_get_sec()*/time(NULL), 5);
		messageLogger->endLog();
    };
};






