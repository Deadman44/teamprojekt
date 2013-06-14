// hashing_proj.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <boost\functional\hash.hpp>
#include <openssl\evp.h>
#include <iomanip>




using namespace std;


string hashing(const char *thing, int size, const char *salt, int saltsize) //übergibt ptr auf bytearray und die größe davon
{
	EVP_MD_CTX *mdctx;
	const EVP_MD *md;

	unsigned char md_value[EVP_MAX_MD_SIZE]; //160 Bit bei SHA1
	unsigned int md_len; //

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
	//EVP_DigestUpdate(mdctx,salt,saltsize);
	EVP_DigestFinal_ex(mdctx, md_value, &md_len); //schreibt in mdvalue, anzahl zeichen in md_len
	EVP_MD_CTX_destroy(mdctx); // zerstört mdctx obj

	string hash = "";

	int i;
	printf("Digest is: \n");
	cout << "\n";
	for(i = 0; i < md_len; i++)
	{
		printf("%02x", md_value[i]); //format fügt nullen hinzu wenn gebruacht z.b. b --> 0b
		hash+=md_value[i];
		
	}

	cout << "\n";

	/*
	unsigned char *ptr = md_value;
	for(int u = 0; u < md_len;u++)
	{
		cout << *ptr;
		ptr++;
	}
	*/
	printf("\n");
	return hash; //überprüfen, ob hier irgendwann delete aufgerufen werden muss für speicherbereinigung... landet string auf heap oder stack??
}

void readFile(string filename)
{
	ifstream::pos_type size;
	char* memblock;
	char* salt;
	string myhash;

	//hier unten eher filename...
	ifstream myfile ("../Debug/hashing_proj.exe",ios::in |ios::binary | ios::ate);
	
	if (myfile.is_open())
	{
    size = myfile.tellg(); //weil zeiger am ende der datei steht kann hier einfach die dateigroesse abgelesen werden
    memblock = new char [size];
    myfile.seekg (0, ios::beg);
    myfile.read (memblock, size);
    myfile.close();


	salt = new char[64]; //64 * 8 = 512!
	for(int i = 0; i < 64; i++)
	{
		salt[i] = 'L';
	}

	myhash = hashing(memblock,size,salt,64);
    delete[] memblock;
	delete[] salt;
	}
	else cout << "Unable to open file";
	//cout << "\n hier der hash aus der ftk " << myhash;

	stringstream ss;

	ofstream outfile ("hash.txt",ofstream::binary);
	for(int m = 0; m < 20; m++)
	{
			unsigned char i = myhash[m];
			ss << hex << setfill('0') << setw(2) << (int) i; // minimale ausgabebreit 2, mit 0en füllen
	}

	char *formattedhash = new char[60];
	ss.getline(formattedhash,60);
	string s (formattedhash);
	cout << formattedhash;
	delete[] formattedhash;

	outfile.close();


}
int _tmain(int argc, _TCHAR* argv[])
{
	readFile("BEGIN");
	

	int x;
	cin >> x;
}

