

#include "cube.h"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <boost\functional\hash.hpp>
#include <openssl\evp.h>
#include <iomanip>
using namespace std;




std::string hashing(const char *thing, int size) //�bergibt ptr auf bytearray und die gr��e davon
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
		const char *tmp = thing; //digestupdat ben�tigt pointer auf irgendwas... deswegen neue ptr
		EVP_DigestUpdate(mdctx,tmp,1); //ein byte hashen
		thing++; //um eins verschieben

	}
	EVP_DigestUpdate(mdctx,mySaltFromTicket,88); // zus�tzlich Ticket bzw. Salt hashen --> SHA1(DATEI.TICKET) ist das resultat!
	EVP_DigestFinal_ex(mdctx, md_value, &md_len); //schreibt in mdvalue, anzahl zeichen in md_len
	EVP_MD_CTX_destroy(mdctx); // zerst�rt mdctx obj

	std::string hash = "";

	// AUSGABE IN KONSOLE...
	int i;
	printf("Digest is: \n");
	cout << "\n";
	for(i = 0; i < md_len; i++)
	{
		printf("%02x", md_value[i]); //format f�gt nullen hinzu wenn gebruacht z.b. b --> 0b
		hash+=md_value[i];
		
	}

	cout << "\n";

	printf("\n");
	return hash; //�berpr�fen, ob hier irgendwann delete aufgerufen werden muss f�r speicherbereinigung... landet string auf heap oder stack??
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
			ss << hex << setfill('0') << setw(2) << (int) i; // minimale ausgabebreit 2, mit 0en f�llen
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

	
	int fileDecider = toHashData;

	toHashGameData[100] = "bin/cube.exe";
	toHashGameData[101] = "bin/SDL.dll";	
	toHashGameData[102] = "bin/libpng1.dll";
	toHashGameData[103] = "bin/msvcr71.dll";	
	toHashGameData[104] = "bin/jpeg.dll";
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
	toHashGameData[500] = "packages/aard";
	toHashGameData[501] = "packages/af";
	toHashGameData[502] = "packages/base";
	toHashGameData[503] = "packages/dcp_the_core";
	toHashGameData[504] = "packages/dg";
	toHashGameData[505] = "packages/drian";
	toHashGameData[506] = "packages/egyptsoc";
	toHashGameData[507] = "packages/fanatic";
	toHashGameData[508] = "packages/golgotha";
	toHashGameData[509] = "packages/ik2k";
	toHashGameData[510] = "packages/ikbase";
	toHashGameData[511] = "packages/jf1";
	toHashGameData[512] = "packages/kurt";
	toHashGameData[513] = "packages/makke";
	toHashGameData[514] = "packages/mitaman";
	toHashGameData[515] = "packages/models";
	toHashGameData[516] = "packages/packages";
	toHashGameData[517] = "packages/payne";
	toHashGameData[518] = "packages/pigcam";
	toHashGameData[519] = "packages/schwenz";
	toHashGameData[520] = "packages/socksky";
	toHashGameData[521] = "packages/songs";
	toHashGameData[522] = "packages/sounds";
	toHashGameData[523] = "packages/spen";
	toHashGameData[524] = "packages/stecki";
	toHashGameData[525] = "packages/tech1soc";
	toHashGameData[526] = "packages/than_ind";
	
	cout << "TRY TO OPEN FOLLOWING: " << fileDecider << toHashGameData[fileDecider];

	return readAndHashFile(toHashGameData[fileDecider]);
}



