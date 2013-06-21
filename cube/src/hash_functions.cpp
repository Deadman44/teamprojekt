

#include "cube.h"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <boost\functional\hash.hpp>
#include <openssl\evp.h>
#include <iomanip>
using namespace std;




std::string hashing(const char *thing, int size) //übergibt ptr auf bytearray und die größe davon
{
	EVP_MD_CTX *mdctx;
	const EVP_MD *md;
	unsigned char md_value[EVP_MAX_MD_SIZE]; //160 Bit bei SHA1
	unsigned int md_len; //

	//kopieren string in c-zeichenkette...
	char mySaltFromTicket[88];
	for(int u = 0; u < 88; u++)
	{
		mySaltFromTicket[u] = hashticket[u];
	}

	OpenSSL_add_all_digests(); // muss
	md = EVP_sha1();//EVP_get_digestbyname("sha1");

	mdctx = EVP_MD_CTX_create();
	EVP_DigestInit_ex(mdctx,md,NULL);
	for(int q = 0; q < size; q++)
	{
		const char *tmp = thing; //digestupdat benötigt pointer auf irgendwas... deswegen neue ptr
		EVP_DigestUpdate(mdctx,tmp,1); //ein byte hashen
		thing++; //um eins verschieben

	}
	EVP_DigestUpdate(mdctx,mySaltFromTicket,88); //
	EVP_DigestFinal_ex(mdctx, md_value, &md_len); //schreibt in mdvalue, anzahl zeichen in md_len
	EVP_MD_CTX_destroy(mdctx); // zerstört mdctx obj

	std::string hash = "";

	// AUSGABE IN KONSOLE...
	int i;
	printf("Digest is: \n");
	cout << "\n";
	for(i = 0; i < md_len; i++)
	{
		printf("%02x", md_value[i]); //format fügt nullen hinzu wenn gebruacht z.b. b --> 0b
		hash+=md_value[i];
		
	}

	cout << "\n";

	printf("\n");
	return hash; //überprüfen, ob hier irgendwann delete aufgerufen werden muss für speicherbereinigung... landet string auf heap oder stack??
}

std::string readAndHashFile(std::string filename)
{
	ifstream::pos_type size;
	char* memblock;
	std::string myhash;

	//lese datei
	cout << " OPENING " << filename;
	ifstream myfile (filename,ios::in |ios::binary | ios::ate);	
	if (myfile.is_open())
	{
		size = myfile.tellg(); //weil zeiger am ende der datei steht kann hier einfach die dateigroesse abgelesen werden
		memblock = new char [size];
		myfile.seekg (0, ios::beg);
		myfile.read (memblock, size);
		myfile.close();
		myhash = hashing(memblock,size);
		delete[] memblock;
	}
	else cout << "Unable to open file";

	//konvertierungsfunktionen
	stringstream ss;
	ofstream outfile ("hash.txt",ofstream::binary);
	for(int m = 0; m < 20; m++)
	{
			unsigned char i = myhash[m];
			ss << hex << setfill('0') << setw(2) << (int) i; // minimale ausgabebreit 2, mit 0en füllen
	}

	char *formattedhash = new char[60];
	ss.getline(formattedhash,60);
	std::string s (formattedhash);
	cout << s;
	delete[] formattedhash;

	outfile.close();
	return s;

}

std::string start_integrity_check()
{
	std::string toHashGameData[1000]; //besser externalisieren
	int fileDecider = toHashData;

	toHashGameData[100] = "bin/cube.exe";
	toHashGameData[101] = "bin/jpeg.dll";
	toHashGameData[102] = "bin/libpng1.dll";
	toHashGameData[103] = "bin/msvcr71.dll";
	toHashGameData[104] = "bin/SDL.dll";
	toHashGameData[105] = "bin/SDL_image.dll";
	toHashGameData[106] = "bin/SDL_mixer.dll";
	toHashGameData[107] = "bin/zlib.dll";
	toHashGameData[108] = "data/martin/ball1.png";
	toHashGameData[109] = "data/martin/ball2.png";
	toHashGameData[110] = "data/martin/ball3.png";
	toHashGameData[111] = "data/martin/base.png";
	toHashGameData[112] = "data/martin/smoke.png";
	toHashGameData[113] = "data/crosshair.png";
	toHashGameData[114] = "data/default_map_settings.cfg";
	toHashGameData[115] = "data/defaults.cfg";
	toHashGameData[116] = "data/explosion.jpg";
	toHashGameData[117] = "data/Health.png";
	toHashGameData[118] = "data/items.png";
	toHashGameData[119] = "data/items_.png";
	toHashGameData[120] = "data/keymap.cfg";
	toHashGameData[121] = "data/menus.cfg";
	toHashGameData[122] = "data/newchars.png";
	toHashGameData[123] = "data/prefabs.cfg";
	toHashGameData[124] = "data/sounds.cfg";
	toHashGameData[125] = "packages/aard";
	toHashGameData[126] = "packages/af";
	toHashGameData[127] = "packages/base";
	toHashGameData[128] = "packages/dcp_the_core";
	toHashGameData[129] = "packages/dg";
	toHashGameData[130] = "packages/drian";
	toHashGameData[131] = "packages/egyptsoc";
	toHashGameData[132] = "packages/fanatic";
	toHashGameData[133] = "packages/golgotha";
	toHashGameData[134] = "packages/ik2k";
	toHashGameData[135] = "packages/ikbase";
	toHashGameData[136] = "packages/jf1";
	toHashGameData[137] = "packages/kurt";
	toHashGameData[138] = "packages/makke";
	toHashGameData[139] = "packages/mitaman";
	toHashGameData[140] = "packages/models";
	toHashGameData[141] = "packages/packages";
	toHashGameData[142] = "packages/payne";
	toHashGameData[143] = "packages/pigcam";
	toHashGameData[144] = "packages/schwenz";
	toHashGameData[145] = "packages/socksky";
	toHashGameData[146] = "packages/songs";
	toHashGameData[147] = "packages/sounds";
	toHashGameData[148] = "packages/spen";
	toHashGameData[149] = "packages/stecki";
	toHashGameData[150] = "packages/tech1soc";
	toHashGameData[151] = "packages/than_ind";
	
	cout << "TRY TO OPEN FOLLOWING: " << fileDecider << toHashGameData[fileDecider];

	return readAndHashFile(toHashGameData[fileDecider]);
}



