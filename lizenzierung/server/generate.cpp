#include "cryptopp/sha.h"
#include "cryptopp/cryptlib.h"
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <string>
#include <sstream>
#include <fstream>
#include <cmath>
#define DEBUG 0
#define DIGESTSIZE CryptoPP::SHA256::DIGESTSIZE

using namespace std;

void SHA256(string, byte*);
void SHA256extended(byte*);

int main(int argc, const char *args[]) {
	unsigned int licenseNumber=0;
	unsigned int salt=0;
	unsigned int seed = 0;
	byte hash[DIGESTSIZE];
	string licenseNumberStr;
	char urand[4];
	ifstream src;
	src.open("/dev/urandom", ios::binary|ios::in);
	src.read(urand, 4);
	srand(seed);
	if(argc < 2){	// Dem Programm wurde kein Parameter übergeben
		licenseNumber = rand();
		stringstream tmp;
		tmp << licenseNumber;
		licenseNumberStr = tmp.str();
		salt = rand();
	} else if(argc < 3) {// Dem Programm wurde nur die Lizenznummer übergeben
		licenseNumberStr = args[1];
		licenseNumber = atoi(args[1]);	// String zu Long umwandeln
		salt = rand();
	} else if(argc==3){	// Dem programm wurde die Lizenznummer und der Salt übergeben		
		licenseNumberStr = args[1];
		licenseNumber = atoi(args[1]);	// Lizenznummer zu int umwandeln
		salt = atoi(args[2]);			// Salt zu int umwandeln
	} else {	// Dem Programm wurden zu viele Parameter übergeben
		exit(-1);
	}
	// Mit 0 auffüllen, bis 10 Zeichen erreicht sind
	for(int i=licenseNumberStr.size(); i<10; i++) {
		licenseNumberStr.insert(0, "0");
	}
	// Lizenzschlüssel, konkateniert aus der Lizenznummer und dem Salt erstellen
	stringstream key;
	key << licenseNumber << salt;
	stringstream saltStr;
	saltStr << salt;	// Salt zu String umwandeln
	// Hash erzeugen
	SHA256(key.str(), hash);
	SHA256extended(hash);
	if(DEBUG) {
		cout << "Lizenznummer: " << licenseNumberStr << endl;
		cout << "Salt/SKEY: " << dec << salt << endl;
		cout << "Hashwert für " << key.str() << ": ";
		for(int i=0; i<DIGESTSIZE; i++) {
			cout << hex << int(hash[i]);
		}
		cout << endl;
		cout << "HSerial: ";
		SHA256(licenseNumberStr,hash);
		for(int i=0; i<DIGESTSIZE; i++) {
			cout << hex << int(hash[i]);
		}
		cout << endl;
	}else {	// ohne DEBUG
		// Rückgabe Hash
		for(int i=0; i<DIGESTSIZE; i++) {
			cout << hex << int(hash[i]);
		}
		cout << endl;
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