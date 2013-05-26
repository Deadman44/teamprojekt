#include "cryptopp/sha.h"
#include "cryptopp/cryptlib.h"
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <string>
#include <sstream>
#include <openssl/rand.h>
#define DEBUG 1
#define DIGESTSIZE CryptoPP::SHA256::DIGESTSIZE
#define SALTSIZE 16
#define LICENSESIZE 4
#define HASH_ITERATIONS 128

using namespace std;

void SHA256(unsigned char*, byte*);
void SHA256extended(byte*);

int main(int argc, const char *args[]) {
	unsigned char licenseNumber[LICENSESIZE];
	for(int i=0; i<sizeof(licenseNumber); i++) {
		licenseNumber[i] = 0;
	}
	unsigned char salt[SALTSIZE];
	for(int i =0; i<sizeof(salt); i++) {
		salt[i] = 0;
	}
	byte hash[DIGESTSIZE];
	srand(time(NULL) * time(NULL));
	if(argc < 2){	// Dem Programm wurde kein Parameter übergeben
		RAND_bytes(licenseNumber, sizeof(licenseNumber));
		RAND_bytes(salt, sizeof(salt));
	} else if(argc < 3) {// Dem Programm wurde nur die Lizenznummer übergeben
		for(int i=0; args[1][i] != '\0'; i++) {
			licenseNumber[i] = args[1][i] - '0';
		}
		RAND_bytes(salt, sizeof(salt));	// salt erzeugen
	} else if(argc == 3) {	// Dem programm wurde die Lizenznummer und der Salt übergeben		
		for(int i=0; i!=args[1][i]!='\0'; i++) {
			licenseNumber[i] = args[1][i] - '0';
			cout << i << " ";
		}
		cout << endl;
		for(int i=0; i!=args[2][i]!='\0'; i++) {
			salt[i] = args[2][i] - '0';
		}
		cout << endl;
		//licenseNumber = atol(args[1]);	// Linzenznummer zu Long umwandeln
	} else {
		exit(-1);	// Falschen Anzahl Parameter übergeben
	}
	// Salt zu licenseNumber hinzufügen
	for(int i=32,j=0; i<sizeof(licenseNumber);i++,j++) {
		licenseNumber[i] = salt[0];
	}
	// Hash erzeugen
	SHA256(licenseNumber, hash);
	SHA256extended(hash);
	if(DEBUG) {
		cout << "argc: " << argc << endl;
		cout << "Lizenznummer:";
		for(int i=0; i<sizeof(licenseNumber); i++) {
			cout << dec << (unsigned int)licenseNumber[i];
		}
		cout << endl;
		// Ausgabe salt in hexadezimal
		cout << "Salt/SKEY: ";
		for(int i=0; i<sizeof(salt); i++) {
			cout << dec << (unsigned int)salt[i];
		}
		cout << endl;
		
		byte tmp[DIGESTSIZE];
		SHA256(licenseNumber, tmp);
		cout << "Lizenzschlüssel Hash(Lizenznummer+Salt): ";
		for(int i=0; i<DIGESTSIZE; i++) {
			cout << hex << int(tmp[i]);
		}
		cout << endl;
		
		cout << "HSerial: ";
		for(int i=0; i<DIGESTSIZE; i++) {
			cout << hex << int(hash[i]);
		}
		cout << endl;
	}else {	// ohne DEBUG
		// Rückgabe Hash
		for(int i=0; i<DIGESTSIZE; i++) {
			cout << hex << int(hash[i]);
		}
	} 
	return 0;
}

void SHA256(unsigned char* str, byte *hash)
{
    byte const* bStr = (byte*) str;
    unsigned int strLen = 160;
    CryptoPP::SHA256().CalculateDigest(hash, bStr, strLen);
}

void SHA256extended(byte *hash){
	unsigned int byteLen = sizeof(hash)/sizeof(hash[0]);
	for (int i=0;i<HASH_ITERATIONS;i++){
		CryptoPP::SHA256().CalculateDigest(hash, hash, byteLen);
		/*if(DEBUG){
			cout << "Hashwert Extended "<<i+1<<" : ";
			for(int j=0; j<DIGESTSIZE; j++) {
				cout << hex << int(hash[j]);
			}
			cout << endl;
			cout <<"Größe.: "<< sizeof(hash)/sizeof(hash[0]);
			cout << endl;
		}*/
	}
}