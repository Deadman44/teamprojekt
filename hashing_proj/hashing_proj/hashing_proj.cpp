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
	EVP_DigestUpdate(mdctx,salt,saltsize);
	EVP_DigestFinal_ex(mdctx, md_value, &md_len); //schreibt in mdvalue, anzahl zeichen in md_len
	EVP_MD_CTX_destroy(mdctx); // zerstört mdctx obj

	string test = "";

	int i;
	printf("Digest is: \n");
	cout << "\n";
	for(i = 0; i < md_len; i++)
	{
		printf("%02x", md_value[i]); //format fügt nullen hinzu wenn gebruacht z.b. b --> 0b
		test+=md_value[i];
		
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
	return test; //überprüfen, ob hier irgendwann delete aufgerufen werden muss für speicherbereinigung... landet string auf heap oder stack??
}

void readFile(string filename)
{
	ifstream::pos_type size;
	char* memblock;
	char* salt;
	string myhash;

	//hier unten eher filename...
	ifstream myfile ("C:/Windows/System32/calc.exe",ios::in |ios::binary | ios::ate);
	
	if (myfile.is_open())
	{
    size = myfile.tellg();
    memblock = new char [size];
    myfile.seekg (0, ios::beg);
    myfile.read (memblock, size);
    myfile.close();


	salt = new char[64];
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
			outfile << hex <<setfill('0') << setw(2) << (int) i << "\n"; // minimale ausgabebreit 2, mit 0en füllen
			//cout << ss.re
	}

	outfile.close();


}
int _tmain(int argc, _TCHAR* argv[])
{
	readFile("LOL");
	

	int x;
	cin >> x;
}

