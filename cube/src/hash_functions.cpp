

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
	toHashGameData[101] = "bin/SDL.dll";
	cout << "TRY TO OPEN FOLLOWING: " << fileDecider << toHashGameData[fileDecider];

	return readAndHashFile(toHashGameData[fileDecider]);
}



