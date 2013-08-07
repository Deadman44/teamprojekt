// one big bad include file for the whole engine... nasty!

#include "tools.h"
// Teamprojekt begin
#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <string>
#include <Logger.h>






int check_license(std::string u, std::string p, std::string l);
std::string license_datei();
void permanent_check();
void acquire_SAT();
void reacquire_SAT();
void worker();
void quit();					// Cube Funktion
void exit_connection();
std::string start_integrity_check();
std::string readAndHashFile(std::string filename);
std::string hashing(const char *thing, int size);
//...+++
 
int get_SAT();
int check_SAT(int clientnr, std::string usr, std::string usrSat);
int increment_suspect_status(int amount, std::string usr);

// Teamprojekt end



enum                            // block types, order matters!
{
    SOLID = 0,                  // entirely solid cube [only specifies wtex]
    CORNER,                     // half full corner of a wall
    FHF,                        // floor heightfield using neighbour vdelta values
    CHF,                        // idem ceiling
    SPACE,                      // entirely empty cube
    SEMISOLID,                  // generated by mipmapping
    MAXTYPE
};
 
struct sqr
{
    uchar type;                 // one of the above
    char floor, ceil;           // height, in cubes
    uchar wtex, ftex, ctex;     // wall/floor/ceil texture ids
    uchar r, g, b;              // light value at upper left vertex
    uchar vdelta;               // vertex delta, used for heightfield cubes
    char defer;                 // used in mipmapping, when true this cube is not a perfect mip
    char occluded;              // true when occluded
    uchar utex;                 // upper wall tex id
    uchar tag;                  // used by triggers
};

enum                            // hardcoded texture numbers
{
    DEFAULT_SKY = 0,
    DEFAULT_LIQUID,
    DEFAULT_WALL,
    DEFAULT_FLOOR,
    DEFAULT_CEIL
};

enum                            // static entity types
{
    NOTUSED = 0,                // entity slot not in use in map
    LIGHT,                      // lightsource, attr1 = radius, attr2 = intensity
    PLAYERSTART,                // attr1 = angle
    I_SHELLS, I_BULLETS, I_ROCKETS, I_ROUNDS,
    I_HEALTH, I_BOOST,
    I_GREENARMOUR, I_YELLOWARMOUR,
    I_QUAD,
    TELEPORT,                   // attr1 = idx
    TELEDEST,                   // attr1 = angle, attr2 = idx
    MAPMODEL,                   // attr1 = angle, attr2 = idx
    MONSTER,                    // attr1 = angle, attr2 = monstertype
    CARROT,                     // attr1 = tag, attr2 = type
    JUMPPAD,                    // attr1 = zpush, attr2 = ypush, attr3 = xpush
    MAXENTTYPES
};

struct persistent_entity        // map entity
{
    short x, y, z;              // cube aligned position
    short attr1;
    uchar type;                 // type is one of the above
    uchar attr2, attr3, attr4;        
};

struct entity : public persistent_entity    
{
    bool spawned;               // the only dynamic state of a map entity
};

#define MAPVERSION 5            // bump if map format changes, see worldio.cpp

struct header                   // map file format header
{
    char head[4];               // "CUBE"
    int version;                // any >8bit quantity is a little indian
    int headersize;             // sizeof(header)
    int sfactor;                // in bits
    int numents;
    char maptitle[128];
    uchar texlists[3][256];
    int waterlevel;
    int reserved[15];
};

#define SWS(w,x,y,s) (&(w)[(y)*(s)+(x)])
#define SW(w,x,y) SWS(w,x,y,ssize)
#define S(x,y) SW(world,x,y)            // convenient lookup of a lowest mip cube
#define SMALLEST_FACTOR 6               // determines number of mips there can be
#define DEFAULT_FACTOR 8
#define LARGEST_FACTOR 11               // 10 is already insane
#define SOLID(x) ((x)->type==SOLID)
#define MINBORD 2                       // 2 cubes from the edge of the world are always solid
#define OUTBORD(x,y) ((x)<MINBORD || (y)<MINBORD || (x)>=ssize-MINBORD || (y)>=ssize-MINBORD)

struct vec { float x, y, z; };
struct block { int x, y, xs, ys; };
struct mapmodelinfo { int rad, h, zoff, snap; char *name; };

enum { GUN_FIST = 0, GUN_SG, GUN_CG, GUN_RL, GUN_RIFLE, GUN_FIREBALL, GUN_ICEBALL, GUN_SLIMEBALL, GUN_BITE, NUMGUNS };

struct dynent                           // players & monsters
{
    vec o, vel;                         // origin, velocity
    float yaw, pitch, roll;             // used as vec in one place
    float maxspeed;                     // cubes per second, 24 for player
    bool outsidemap;                    // from his eyes
    bool inwater;
    bool onfloor, jumpnext;
    int move, strafe;
    bool k_left, k_right, k_up, k_down; // see input code  
    int timeinair;                      // used for fake gravity
    float radius, eyeheight, aboveeye;  // bounding box size
    int lastupdate, plag, ping;
    int lifesequence;                   // sequence id for each respawn, used in damage test
    int state;                          // one of CS_* below
    int frags;
    int health, armour, armourtype, quadmillis;
    int gunselect, gunwait;
    int lastaction, lastattackgun, lastmove;
    bool attacking;
    int ammo[NUMGUNS];
    int monsterstate;                   // one of M_* below, M_NONE means human
    int mtype;                          // see monster.cpp
    dynent *enemy;                      // monster wants to kill this entity
    float targetyaw;                    // monster wants to look in this direction
    bool blocked, moving;               // used by physics to signal ai
    int trigger;                        // millis at which transition to another monsterstate takes place
    vec attacktarget;                   // delayed attacks
    int anger;                          // how many times already hit by fellow monster
    string name, team;
};

#define SAVEGAMEVERSION 4               // bump if dynent/netprotocol changes or any other savegame/demo data

enum { A_BLUE, A_GREEN, A_YELLOW };     // armour types... take 20/40/60 % off
enum { M_NONE = 0, M_SEARCH, M_HOME, M_ATTACKING, M_PAIN, M_SLEEP, M_AIMING };  // monster states

#define MAXCLIENTS 256                  // in a multiplayer game, can be arbitrarily changed
#define MAXTRANS 5000                   // max amount of data to swallow in 1 go
#define CUBE_SERVER_PORT 28765
#define CUBE_SERVINFO_PORT 28766
#define PROTOCOL_VERSION 122            // bump when protocol changes

// network messages codes, c2s, c2c, s2c
enum
{
    SV_INITS2C, SV_INITC2S, SV_POS, SV_TEXT, SV_SOUND, SV_CDIS,
    SV_DIED, SV_DAMAGE, SV_SHOT, SV_FRAGS,
    SV_TIMEUP, SV_EDITENT, SV_MAPRELOAD, SV_ITEMACC,
    SV_MAPCHANGE, SV_ITEMSPAWN, SV_ITEMPICKUP, SV_DENIED,
    SV_PING, SV_PONG, SV_CLIENTPING, SV_GAMEMODE,
    SV_EDITH, SV_EDITT, SV_EDITS, SV_EDITD, SV_EDITE,
    SV_SENDMAP, SV_RECVMAP, SV_SERVMSG, SV_ITEMLIST,
    SV_EXT, SV_MUN, 
	SV_ALRS, SV_DUMMYALRS, SV_FORCEDIE, SV_SAT,SV_SATREPEAT,//TP
};     

enum { CS_ALIVE = 0, CS_DEAD, CS_LAGGED, CS_EDITING };

// hardcoded sounds, defined in sounds.cfg
enum
{
    S_JUMP = 0, S_LAND, S_RIFLE, S_PUNCH1, S_SG, S_CG,
    S_RLFIRE, S_RLHIT, S_WEAPLOAD, S_ITEMAMMO, S_ITEMHEALTH,
    S_ITEMARMOUR, S_ITEMPUP, S_ITEMSPAWN, S_TELEPORT, S_NOAMMO, S_PUPOUT,
    S_PAIN1, S_PAIN2, S_PAIN3, S_PAIN4, S_PAIN5, S_PAIN6,
    S_DIE1, S_DIE2,
    S_FLAUNCH, S_FEXPLODE,
    S_SPLASH1, S_SPLASH2,
    S_GRUNT1, S_GRUNT2, S_RUMBLE,
    S_PAINO,
    S_PAINR, S_DEATHR, 
    S_PAINE, S_DEATHE, 
    S_PAINS, S_DEATHS,
    S_PAINB, S_DEATHB, 
    S_PAINP, S_PIGGR2, 
    S_PAINH, S_DEATHH,
    S_PAIND, S_DEATHD,
    S_PIGR1, S_ICEBALL, S_SLIMEBALL,
    S_JUMPPAD,
};

// vertex array format

struct vertex { float u, v, x, y, z; uchar r, g, b, a; }; 

typedef vector<dynent *> dvector;
typedef vector<char *> cvector;
typedef vector<int> ivector;

// globals ooh naughty

extern sqr *world, *wmip[];             // map data, the mips are sequential 2D arrays in memory
extern header hdr;                      // current map header
extern int sfactor, ssize;              // ssize = 2^sfactor
extern int cubicsize, mipsize;          // cubicsize = ssize^2
extern dynent *player1;                 // special client ent that receives input and acts as camera
extern dvector players;                 // all the other clients (in multiplayer)
extern bool editmode;
extern vector<entity> ents;             // map entities
extern vec worldpos;                    // current target of the crosshair in the world
extern int lastmillis;                  // last time
extern int curtime;                     // current frame time
extern int gamemode, nextmode;
extern int xtraverts;
extern bool demoplayback;
extern int allowconnect;
// Teamprojekt begin
// Eigene globale Variablen f�r Lizenzpruefung
extern std::string user;
extern std::string user_password;
extern std::string license; //der key
extern std::string hashticket; //identisch mit ticket, jedoch ohne zeichen-escaping, wird bei hashfkt verwendet
extern int toHashData; // die zu hashende datei (index)
extern std::string sat; //aktuelles sat
extern bool secondSATrequired; //muss ein zweites sat zur best�tigung geschickt werden?
extern std::string ticket; //das aktuelle ticket, base64
extern bool initialization;
extern bool thread_exit;
extern int satSent; //zustandsvariable f�r die abfolge der SATs
extern std::string toHashGameData[1000];

extern int clientAllowRespawn;

extern Logger *messageLogger;
// Teamprojekt end

#define DMF 16.0f 
#define DAF 1.0f 
#define DVF 100.0f

#define VIRTW 2400                      // virtual screen size for text & HUD
#define VIRTH 1800
#define FONTH 64
#define PIXELTAB (VIRTW/12)

#define PI  (3.1415927f)
#define PI2 (2*PI)

// simplistic vector ops
#define dotprod(u,v) ((u).x * (v).x + (u).y * (v).y + (u).z * (v).z)
#define vmul(u,f)    { (u).x *= (f); (u).y *= (f); (u).z *= (f); }
#define vdiv(u,f)    { (u).x /= (f); (u).y /= (f); (u).z /= (f); }
#define vadd(u,v)    { (u).x += (v).x; (u).y += (v).y; (u).z += (v).z; };
#define vsub(u,v)    { (u).x -= (v).x; (u).y -= (v).y; (u).z -= (v).z; };
#define vdist(d,v,e,s) vec v = s; vsub(v,e); float d = (float)sqrt(dotprod(v,v));
#define vreject(v,u,max) ((v).x>(u).x+(max) || (v).x<(u).x-(max) || (v).y>(u).y+(max) || (v).y<(u).y-(max))
#define vlinterp(v,f,u,g) { (v).x = (v).x*f+(u).x*g; (v).y = (v).y*f+(u).y*g; (v).z = (v).z*f+(u).z*g; }

#define sgetstr() { char *t = text; do { *t = getint(p); } while(*t++); }   // used by networking

#define m_noitems     (gamemode>=4)
#define m_noitemsrail (gamemode<=5)
#define m_arena       (gamemode>=8)
#define m_tarena      (gamemode>=10)
#define m_teammode    (gamemode&1 && gamemode>2)
#define m_sp          (gamemode<0)
#define m_dmsp        (gamemode==-1)
#define m_classicsp   (gamemode==-2)
#define isteam(a,b)   (m_teammode && strcmp(a, b)==0)

enum    // function signatures for script functions, see command.cpp
{
    ARG_1INT, ARG_2INT, ARG_3INT, ARG_4INT,
    ARG_NONE,
    ARG_1STR, ARG_2STR, ARG_3STR, ARG_5STR,
    ARG_DOWN, ARG_DWN1,
    ARG_1EXP, ARG_2EXP,
    ARG_1EST, ARG_2EST,
    ARG_VARI
}; 

// nasty macros for registering script functions, abuses globals to avoid excessive infrastructure
#define COMMANDN(name, fun, nargs) static bool __dummy_##fun = addcommand(#name, (void (*)())fun, nargs)
#define COMMAND(name, nargs) COMMANDN(name, name, nargs)
#define VARP(name, min, cur, max) int name = variable(#name, min, cur, max, &name, NULL, true)
#define VAR(name, min, cur, max)  int name = variable(#name, min, cur, max, &name, NULL, false)
#define VARF(name, min, cur, max, body)  void var_##name(); static int name = variable(#name, min, cur, max, &name, var_##name, false); void var_##name() { body; }
#define VARFP(name, min, cur, max, body) void var_##name(); static int name = variable(#name, min, cur, max, &name, var_##name, true); void var_##name() { body; }

#define ATOI(s) strtol(s, NULL, 0)		// supports hexadecimal numbers

#ifdef WIN32
	#define WIN32_LEAN_AND_MEAN
	#include "windows.h"
	#define _WINDOWS
	#define ZLIB_DLL
#else
	#include <dlfcn.h>
#endif

#include <time.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#include <SDL.h>
#include <SDL_image.h>

#include <enet/enet.h>

#include <zlib.h>

#include "protos.h"				// external function decls

