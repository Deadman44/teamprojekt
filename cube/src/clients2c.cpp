// client processing of the incoming network stream

#include "cube.h"

extern int clientnum;
extern bool c2sinit, senditemstoserver;
extern string toservermap;
extern string clientpassword;

int clientAllowRespawn = -1; //TP, ersten Respawn erlauben

void neterr(char *s)
{
    conoutf("illegal network message (%s)", s);
    disconnect();
};

void changemapserv(char *name, int mode)        // forced map change from the server
{
    gamemode = mode;
    load_world(name);
};

void changemap(char *name)                      // request map change, server may ignore
{
    strcpy_s(toservermap, name);
};

// update the position of other clients in the game in our world
// don't care if he's in the scenery or other players,
// just don't overlap with our client

void updatepos(dynent *d)
{
    const float r = player1->radius+d->radius;
    const float dx = player1->o.x-d->o.x;
    const float dy = player1->o.y-d->o.y;
    const float dz = player1->o.z-d->o.z;
    const float rz = player1->aboveeye+d->eyeheight;
    const float fx = (float)fabs(dx), fy = (float)fabs(dy), fz = (float)fabs(dz);
    if(fx<r && fy<r && fz<rz && d->state!=CS_DEAD)
    {
        if(fx<fy) d->o.y += dy<0 ? r-fy : -(r-fy);  // push aside
        else      d->o.x += dx<0 ? r-fx : -(r-fx);
    };
    int lagtime = lastmillis-d->lastupdate;
    if(lagtime)
    {
        d->plag = (d->plag*5+lagtime)/6;
        d->lastupdate = lastmillis;
    };
};

void localservertoclient(uchar *buf, int len)   // processes any updates from the server
{
    if(ENET_NET_TO_HOST_16(*(ushort *)buf)!=len) neterr("packet length");
    incomingdemodata(buf, len);
    
    uchar *end = buf+len;
    uchar *p = buf+2; //da bei buf +0 die anfangskennung steht // zeiger 16 bits weiter setzen (??)
    char text[MAXTRANS];
    int cn = -1, type;
    dynent *d = NULL;
    bool mapchanged = false;

    while(p<end) switch(type = getint(p)) // das while < end muss sein, weil p in der getint methode verändert wird, hilft dagegen, dass p über end rutscht... und das alle
		//messages gelesen werden
    {
        case SV_INITS2C:                    // welcome messsage from the server
        {
            cn = getint(p); //die client-nummer, die vom server vergeben wird bei welcome_message
            int prot = getint(p);
			/* hier ebenfalls wichtig für mod client 
			auskommentieren ermoegtlicth verbindung mit mod client
			*/

            if(prot!=PROTOCOL_VERSION)
            {
                conoutf("you are using a different game protocol (you: %d, server: %d)", PROTOCOL_VERSION, prot);
                disconnect();
                return;
            };

            toservermap[0] = 0;
            clientnum = cn;                 // we are now fully connected
            if(!getint(p)) strcpy_s(toservermap, getclientmap());   // we are the first client on this server, set map
            sgetstr();
            if(text[0] && strcmp(text, clientpassword))
            {
                conoutf("you need to set the correct password to join this server!");
                disconnect();
                return;
            };
            if(getint(p)==1)
            {
                conoutf("server is FULL, disconnecting..");
            };
            break;
        };

        case SV_POS:                        // position of another client
        {
            cn = getint(p);
            d = getclient(cn);
            if(!d) return;
            d->o.x   = getint(p)/DMF;
            d->o.y   = getint(p)/DMF;
            d->o.z   = getint(p)/DMF;
            d->yaw   = getint(p)/DAF;
            d->pitch = getint(p)/DAF;
            d->roll  = getint(p)/DAF;
            d->vel.x = getint(p)/DVF;
            d->vel.y = getint(p)/DVF;
            d->vel.z = getint(p)/DVF;
            int f = getint(p);
            d->strafe = (f&3)==3 ? -1 : f&3;
            f >>= 2; 
            d->move = (f&3)==3 ? -1 : f&3;
            d->onfloor = (f>>2)&1;
            int state = f>>3;
            if(state==CS_DEAD && d->state!=CS_DEAD) d->lastaction = lastmillis;
            d->state = state;
            if(!demoplayback) updatepos(d);
            break;
        };
		//TP
		case SV_ALRS:
		{
			
			int rnd = getint(p);
			std::cout << "\n"<<rnd << " RANDOM AND ERGEBNIS AUF CLIENT " << (clientAllowRespawn-rnd);
			if(clientAllowRespawn == -1 && (clientAllowRespawn-rnd) != -1)
			{
				clientAllowRespawn = rnd;
				addmsg(1,2,SV_ALRS,rnd);
				std::cout << "CLIENT SEI TOT \n";
			}
			else if ((clientAllowRespawn-rnd) == -1)
			{
				clientAllowRespawn = clientAllowRespawn - rnd;
				std::cout << "CLIENT LEBT WIEDER \n";
				
			}

			break;
		};
		//TP OUT
        case SV_SOUND:
            playsound(getint(p), &d->o);
            break;

        case SV_TEXT:
            sgetstr();
            conoutf("%s:\f %s", d->name, text); 
            break;

        case SV_MAPCHANGE:     
            sgetstr();
            changemapserv(text, getint(p));
            mapchanged = true;
            break;
        
        case SV_ITEMLIST: //FORMAT NEU: INR,ITYPE,INR,ITYPE.....ENDEKENNUNG(-1)
        {
            int n;
            if(mapchanged) { senditemstoserver = false; resetspawns(); };
            while((n = getint(p))!=-1)
			{
				if(mapchanged)
					{
						setspawn(n, true);
						int tmp = getint(p); // weil zweiter uchar im paket jetzt auch immer typ ist, der hier nicht gebraucht wird
						if(tmp == -1) //sicherheitsabfrage ob endekennung erreicht ist
						{
							return;
						}
					}
			}
            break;
        };

        case SV_MAPRELOAD:          // server requests next map
        {
            getint(p);
            sprintf_sd(nextmapalias)("nextmap_%s", getclientmap());
            char *map = getalias(nextmapalias);     // look up map in the cycle
            changemap(map ? map : getclientmap());
            break;
        };

        case SV_INITC2S:            // another client either connected or changed name/team
        {
            sgetstr();
            if(d->name[0])          // already connected
            {
                if(strcmp(d->name, text))
                    conoutf("%s is now known as %s", d->name, text);
            }
            else                    // new client
            {
                c2sinit = false;    // send new players my info again 
                conoutf("connected: %s", text);
            }; 
            strcpy_s(d->name, text);
            sgetstr();
            strcpy_s(d->team, text);
            d->lifesequence = getint(p);
            break;
        };
		// wichtige stelle für incompatible/mod clients...  ändern ermöglicht connect mit mod client
        case SV_CDIS:
            cn = getint(p);
            if(!(d = getclient(cn))) break;
			conoutf("player %s disconnected", d->name[0] ? d->name : "[incompatible client]"); 
            zapdynent(players[cn]);
            break;

        case SV_SHOT: // hier wird optisch geschossen, also quasi die view des schuss
        {
            int gun = getint(p);
            vec s, e;
            s.x = getint(p)/DMF;
            s.y = getint(p)/DMF;
            s.z = getint(p)/DMF;
            e.x = getint(p)/DMF;
            e.y = getint(p)/DMF;
            e.z = getint(p)/DMF;
            if(gun==GUN_SG) createrays(s, e);
            shootv(gun, s, e, d);
            break;
        };

        case SV_DAMAGE: //hier wird quasi der schaden ueber das netz empfangen/uebertragen            
        {
			// *d is der spieler dessen paket angekommen ist, siehe oben,wird bei positionsdaten übertragen und angewandt
            int target = getint(p);
            int damage = getint(p);
            int ls = getint(p);
            if(target==clientnum) { if(ls==player1->lifesequence) selfdamage(damage, cn, d); }
            else playsound(S_PAIN1+rnd(5), &getclient(target)->o);
            break;
        };

        case SV_DIED:
        {
            int actor = getint(p);
            if(actor==cn) //cn wird bei jedem SV_POS geändert, hier: wenn der actor von sv_died == cn von sv_pos dann suizid von diesem Spieler
            {
                conoutf("%s suicided", d->name);
            }
            else if(actor==clientnum) //wenn der aktor identisch mit uns is (clientnum== die nr die der server uns gibt)
            {
                int frags;
                if(isteam(player1->team, d->team)) //d wird bei sv_pos genommen == dynent von cn!
                {
                    frags = -1;
                    conoutf("you fragged a teammate (%s)", d->name);
                }
                else
                {
                    frags = 1;
                    conoutf("you fragged %s", d->name);
                };
                addmsg(1, 2, SV_FRAGS, player1->frags += frags);
            }
            else
            {
                dynent *a = getclient(actor);
                if(a)
                {
                    if(isteam(a->team, d->name))
                    {
                        conoutf("%s fragged his teammate (%s)", a->name, d->name);
                    }
                    else
                    {
                        conoutf("%s fragged %s", a->name, d->name);
                    };
                };
            };
            playsound(S_DIE1+rnd(2), &d->o);
            d->lifesequence++;
            break;
        };

        case SV_FRAGS:
            players[cn]->frags = getint(p);
            break;

        case SV_ITEMPICKUP: //gegenstände aufnehmen
            setspawn(getint(p), false);
            getint(p);
            break;

        case SV_ITEMSPAWN:
        {
            uint i = getint(p);
            setspawn(i, true);
            if(i>=(uint)ents.length()) break;
            vec v = { ents[i].x, ents[i].y, ents[i].z };
            playsound(S_ITEMSPAWN, &v); 
            break;
        };

        case SV_ITEMACC:            // server acknowledges that I picked up this item
            realpickup(getint(p), player1);
            break;

        case SV_EDITH:              // coop editing messages, should be extended to include all possible editing ops
        case SV_EDITT:
        case SV_EDITS:
        case SV_EDITD:
        case SV_EDITE:
        {
            int x  = getint(p);
            int y  = getint(p);
            int xs = getint(p);
            int ys = getint(p);
            int v  = getint(p);
            block b = { x, y, xs, ys };
            switch(type)
            {
                case SV_EDITH: editheightxy(v!=0, getint(p), b); break;
                case SV_EDITT: edittexxy(v, getint(p), b); break;
                case SV_EDITS: edittypexy(v, b); break;
                case SV_EDITD: setvdeltaxy(v, b); break;
                case SV_EDITE: editequalisexy(v!=0, b); break;
            };
            break;
        };

        case SV_EDITENT:            // coop edit of ent
        {
            uint i = getint(p);
            while((uint)ents.length()<=i) ents.add().type = NOTUSED;
            int to = ents[i].type;
            ents[i].type = getint(p);
            ents[i].x = getint(p);
            ents[i].y = getint(p);
            ents[i].z = getint(p);
            ents[i].attr1 = getint(p);
            ents[i].attr2 = getint(p);
            ents[i].attr3 = getint(p);
            ents[i].attr4 = getint(p);
            ents[i].spawned = false;
            if(ents[i].type==LIGHT || to==LIGHT) calclight();
            break;
        };

        case SV_PING:
            getint(p);
            break;

        case SV_PONG: 
            addmsg(0, 2, SV_CLIENTPING, player1->ping = (player1->ping*5+lastmillis-getint(p))/6);
            break;

        case SV_CLIENTPING:
            players[cn]->ping = getint(p);
            break;

        case SV_GAMEMODE:
            nextmode = getint(p);
            break;

        case SV_TIMEUP:
            timeupdate(getint(p));
            break;

        case SV_RECVMAP:
        {
            sgetstr();
            conoutf("received map \"%s\" from server, reloading..", text);
            int mapsize = getint(p);
            writemap(text, mapsize, p);
            p += mapsize;
            changemapserv(text, gamemode);
            break;
        };
        
        case SV_SERVMSG:
            sgetstr();
            conoutf("%s", text);
            break;

        case SV_EXT:        // so we can messages without breaking previous clients/servers, if necessary
        {
            for(int n = getint(p); n; n--) getint(p);
            break;
        };
		
		//TP
		case SV_DUMMYALRS:
			{
				int dummyContent = getint(p);
				std::cout << "\n dummy \n" << dummyContent;
				break;
			};

		case SV_MUN:
		{
			int dummyContent = getint(p);
			break;
		}

		case SV_FORCEDIE: //message, die einen client zum sterben zwingt
		{
			int causeOfDeath = getint(p);
			std::cout << " SPIELER " << causeOfDeath << " hat kill erzwungen \n";
			dynent *dtmp = getclient(causeOfDeath);
			player1->health = 0;
			selfdamage(1,causeOfDeath,dtmp); 
			addmsg(1, 2, SV_DIED, causeOfDeath); //schicke an server die nachricht, wer mich umgebracht hat
			break;
		};

		case SV_SAT: //SAT +username==email herausfiltern
		{
			/*

			int len = getint(p);			

			std::cout << len << " <--- SAT_LEN \n";
			char *username = new char[len-5];
			char *cSAT = new char[7];
			//SAT aus paket nehmen
			
			for(int z = 0; z < 6; z++)
			{
				cSAT[z] = getint(p);


			}
			cSAT[6] = '\0';
			//username aus paket nehmen
			for(int u = 0; u < len-6; u++) // -SAT LEN
			{
				username[u] = getint(p);				
			}
			username[len-6] = '\0';
				
			delete[] username;
			delete[] cSAT;

			std::cout << "NAME: " << username << " SAT " << cSAT << "\n";


			*/

			int len = getint(p);

			for(int i = 0; i < len; i++)
			{
				getint(p);
			}
            break;
		};

		case SV_SATREPEAT:
			{
				int nr = getint(p);
				std::cout << " REACQUIRE SAT " << "\n";
				reacquire_SAT();
			}
			break;


		//TP OUT
        default:
            neterr("type");
            return;
    };
};
