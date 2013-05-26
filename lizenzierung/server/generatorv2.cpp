#include <iostream>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <cstring>

#define SALTSIZE 128
#define ITERATIONS 1000
#define KEYSIZE 16
using namespace std;

void hexToString(unsigned char*, string&);

int main(int argc, char *args[]) {
	const char *licenseNumber = args[1];
	unsigned char salt[SALTSIZE];
	unsigned char out[KEYSIZE];
	// Salt erzeugen
	RAND_bytes(salt, SALTSIZE);
	PKCS5_PBKDF2_HMAC_SHA1(licenseNumber, strlen(licenseNumber), salt, SALTSIZE, ITERATIONS, KEYSIZE, out);
	/*string str = "Hallo";
	const unsigned char s[] = "";
	unsigned char hash[64];
	SHA512(s, sizeof(str), hash);
	for(int i=0; i<sizeof(hash)/sizeof(hash[0]); i++) {
		cout << hex << (unsigned int)hash[i];
	}
	cout << endl;*/
} 

void hexToString(unsigned char *hex, string& str) {
	
}