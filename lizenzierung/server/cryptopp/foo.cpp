#include "sha.h"
#include "base64.h"
#include <iostream>
#include "cryptlib.h"
#include <stdio.h>
#include "sha3.h"
#define LEN CryptoPP::SHA256::DIGESTSIZE
using namespace std;
void SHA256(string data)
{
	//byte hash[LEN];
	/*for(int i=0;i<512;i++) {
		hash[i]=0;
	}*/
    byte const* pbData = (byte*) data.data();
    unsigned int nDataLen = data.size();
    byte abDigest[LEN];

    CryptoPP::SHA256().CalculateDigest(abDigest, pbData, nDataLen);
	//CryptoPP::SHA3_224 kecc;
	//kecc.TruncatedFinal(hash,LEN);
	//cout << hex << abDigest[0];
	for(int i=0; i<LEN; i++) {
		cout << hex << int(abDigest[i]);
		//printf("%X",abDigest[i]);
	}
	cout << endl;
}
int main(void) {
	SHA256("Hallo");
	return 0;
}
