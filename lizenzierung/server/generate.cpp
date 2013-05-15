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
	byte hash[DIGESTSIZE];
	srand(time(NULL) * time(NULL));
	if(argc < 2){	// Dem Programm wurde kein Parameter übergeben
		licenseNumber = long(rand());
	} else {
		cout << "Anzahl der uebergebenen Parameter: " << argc << endl;
		licenseNumber = atol(args[1]);	// Parameter zu Long umwandeln
		
	}
	long salt = long(rand());
	stringstream ss;
	ss << licenseNumber << salt;
	SHA256(ss.str(), hash);
	if(DEBUG) {
		cout << "Lizenznummer: " << licenseNumber << endl;
		cout << "Salt: " << salt << endl;
		cout << "Hashwert für " << ss.str() << ": ";
		for(int i=0; i<DIGESTSIZE; i++) {
			cout << hex << int(hash[i]);
		}
		cout << endl;
	}else {
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
	SHA256extended(hash);
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