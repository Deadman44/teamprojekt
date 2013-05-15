#include "cryptopp/sha.h"
#include "cryptopp/cryptlib.h"
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <string>
#include <sstream>
#define DEBUG 1
#define DIGESTSIZE CryptoPP::SHA256::DIGESTSIZE

using namespace std;

void SHA256(string, byte*);
void SHA256extended(byte*);

int main(int argc, const char *args[]) {
	long licenseNumber;
	long salt;
	byte hash[DIGESTSIZE];
	srand(time(NULL) * time(NULL));
	if(argc < 2){	// Dem Programm wurde kein Parameter übergeben
		licenseNumber = long(rand());
		salt = long(rand());
	} else if(argc < 3) {// Dem Programm wurde nur die Lizenznummer übergeben
		licenseNumber = atol(args[1]);	// String zu Long umwandeln
		salt = long(rand());
	} else {	// Dem programm wurde die Lizenznummer und der Salt übergeben		
		licenseNumber = atol(args[1]);	// Linzenznummer zu Long umwandeln
		salt = atol(args[2]);			// Lizenznummer zu Long umwandeln
	}
	stringstream licenseNumberStr;
	licenseNumberStr << licenseNumber;
	// Mit 0 auffüllen, bis 10 Zeichen erreicht sind
	for(int i=licenseNumberStr.str().size();i<=10;i++) {
		licenseNumberStr << 0;
	}
	stringstream key;
	key << licenseNumberStr.str() << salt;
	// Hash erzeugen
	SHA256(key.str(), hash);
	SHA256extended(hash);
	if(DEBUG) {
		cout << "Lizenznummer: " << licenseNumberStr.str() << endl;
		cout << "Salt/SKEY: " << dec << salt << endl;
		cout << "Hashwert für " << key.str() << ": ";
		cout << licenseNumberStr.str();
		for(int i=0; i<DIGESTSIZE; i++) {
			cout << hex << int(hash[i]);
		}
		cout << endl;
		cout << "HSerial: ";
		SHA256(licenseNumberStr.str(),hash);
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

void SHA256(string str, byte *hash)
{
    byte const* bStr = (byte*) str.data();
    unsigned int strLen = str.size();
    CryptoPP::SHA256().CalculateDigest(hash, bStr, strLen);
}

void SHA256extended(byte *hash){
	unsigned int byteLen = sizeof(hash)/sizeof(hash[0]);
	for (int i=0;i<3;i++){
		CryptoPP::SHA256().CalculateDigest(hash, hash, byteLen);
		if(DEBUG){
			cout << "Hashwert Extended "<<i+1<<" : ";
			for(int j=0; j<DIGESTSIZE; j++) {
				cout << hex << int(hash[j]);
			}
			cout << endl;
			cout <<"Größe.: "<< sizeof(hash)/sizeof(hash[0]);
			cout << endl;
		}
	}
}