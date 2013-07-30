// misc useful functions used by the server

#include "cube.h"

// all network traffic is in 32bit ints, which are then compressed using the following simple scheme (assumes that most values are small).

void putint(uchar *&p, int n) 
	/*
	was bedeutet *&p?
	eine referenz to pointer �bergabe,
	quasi call-by-reference f�r einen pointer,
	ist dann hilfreich, wenn der pointer, welche innerhalb der funktion ge�ndert wird
	auch au�erhalb so ge�ndert bleiben soll
	ohne das & zeichen wird call-by-value gemacht, d.h. der pointer wird kopiert
	�nderungen innerhalb der fkt die am pointer passieren sind au�erhalb nicht sichtbar
	http://www.codeproject.com/Articles/4894/Pointer-to-Pointer-and-Reference-to-Pointer
	*/

{


    if(n<128 && n>-127) { *p++ = n; } //werte zwischen 127 und -126 werden direkt reingelegt
    else if(n<0x8000 && n>=-0x8000) { *p++ = 0x80; *p++ = n; *p++ = n>>8;  } // so weit komprimieren wie m�glich, aufteilen der bits auf mehrere
    else { *p++ = 0x81; *p++ = n; *p++ = n>>8; *p++ = n>>16; *p++ = n>>24; }; //uchars (8bit) werte
};

/*
wird bei serverprocessing gebraucht
*/

int getint(uchar *&p)
{
	/* erinnerung:
	  *char ist in c ein pointer auf das erste zeichen einer
	zeichenkette, also strings!
	*/

    int c = *((char *)p);
    p++; //pointer rutscht weiter...., gilt auch f�rs processing
    if(c==-128) { int n = *p++; n |= *((char *)p)<<8; p++; return n;}
    else if(c==-127) { int n = *p++; n |= *p++<<8; n |= *p++<<16; return n|(*p++<<24); } //jede menge bitweises OR
    else return c;
};

void sendstring(char *t, uchar *&p)
{
    while(*t) putint(p, *t++);
    putint(p, 0);
};

const char *modenames[] =
{
    "SP", "DMSP", "ffa/default", "coopedit", "ffa/duel", "teamplay",
    "instagib", "instagib team", "efficiency", "efficiency team",
    "insta arena", "insta clan arena", "tactics arena", "tactics clan arena",
};
      
const char *modestr(int n) { return (n>=-2 && n<12) ? modenames[n+2] : "unknown"; };

/*
beschreibt wie gro� die einzelnen message-pakete sein sollen
d.h., sei sv_damage = 4, dann darf inklusive dem header (sv_damage) beim paket 4x getint(p) aufgerufen werden!
*/

char msgsizesl[] =               // size inclusive message token, 0 for variable or not-checked sizes
{ 
    SV_INITS2C, 4, SV_INITC2S, 0, SV_POS, 12, SV_TEXT, 0, SV_SOUND, 2, SV_CDIS, 2,
    SV_EDITH, 7, SV_EDITT, 7, SV_EDITS, 6, SV_EDITD, 6, SV_EDITE, 6,
    SV_DIED, 2, SV_DAMAGE, 4, SV_SHOT, 8, SV_FRAGS, 2,
    SV_MAPCHANGE, 0, SV_ITEMSPAWN, 2, SV_ITEMPICKUP, 3, SV_DENIED, 2,
    SV_PING, 2, SV_PONG, 2, SV_CLIENTPING, 2, SV_GAMEMODE, 2,
    SV_TIMEUP, 2, SV_EDITENT, 10, SV_MAPRELOAD, 2, SV_ITEMACC, 2,
    SV_SENDMAP, 0, SV_RECVMAP, 1, SV_SERVMSG, 0, SV_ITEMLIST, 0,
    SV_EXT, 0, SV_ALRS, 2, SV_DUMMYALRS, 2, SV_MUN,2, SV_FORCEDIE,2,
	SV_SAT,0,SV_SATREPEAT,2,//TP
    -1
};

char msgsizelookup(int msg)
{
    for(char *p = msgsizesl; *p>=0; p += 2) if(*p==msg) return p[1];

    return -1;
};

// sending of maps between clients

string copyname;
int copysize;
uchar *copydata = NULL;

void sendmaps(int n, string mapname, int mapsize, uchar *mapdata)
{
    if(mapsize <= 0 || mapsize > 256*256) return;
    strcpy_s(copyname, mapname);
    copysize = mapsize;
    if(copydata) free(copydata);
    copydata = (uchar *)alloc(mapsize);
    memcpy(copydata, mapdata, mapsize);
}

ENetPacket *recvmap(int n)
{
    if(!copydata) return NULL;
    ENetPacket *packet = enet_packet_create(NULL, MAXTRANS + copysize, ENET_PACKET_FLAG_RELIABLE);
    uchar *start = packet->data;
    uchar *p = start+2;
    putint(p, SV_RECVMAP);
    sendstring(copyname, p);
    putint(p, copysize);
    memcpy(p, copydata, copysize);
    p += copysize;
    *(ushort *)start = ENET_HOST_TO_NET_16(p-start);
    enet_packet_resize(packet, p-start);
	return packet;
}


#ifdef STANDALONE

void localservertoclient(uchar *buf, int len) {};
void fatal(char *s, char *o) { cleanupserver(); printf("servererror: %s\n", s); exit(1); };
void *alloc(int s) { void *b = calloc(1,s); if(!b) fatal("no memory!"); return b; };

int main(int argc, char* argv[])
{
    int uprate = 0, maxcl = 4;
    char *sdesc = "", *ip = "", *master = NULL, *passwd = "";
    int port = 28765;
    for(int i = 1; i<argc; i++)
    {
        char *a = &argv[i][2];
        if(argv[i][0]=='-') switch(argv[i][1])
        {
            case 'u': uprate = atoi(a); break;
            case 'n': sdesc  = a; break;
            case 'i': ip     = a; break;
            case 'm': master = a; break;
            case 'p': passwd = a; break;
            case 'c': maxcl  = atoi(a); break;
			case 'q': port = atoi(a); break;
            default: printf("WARNING: unknown commandline option\n");
        };
    };
    
    if(enet_initialize()<0) fatal("Unable to initialise network module");
    initserver(true, uprate, sdesc, ip, master, passwd, maxcl,port);
    return 0;
};
#endif



