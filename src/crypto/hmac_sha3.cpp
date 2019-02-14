// Copyright (c) 2014-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <crypto/hmac_sha3.h>

#include <string.h>

CHMAC_SHA3::CHMAC_SHA3(const unsigned char* key, size_t keylen)
{
    // Block size of 136 from Table 3 FIPS 202 http://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.202.pdf
    unsigned char rkey[136];
    if (keylen <= 136) {
        memcpy(rkey, key, keylen);
        memset(rkey + keylen, 0, 136 - keylen);
    } else {
        CSHA3().Write(key, keylen).Finalize(rkey);
        memset(rkey + 32, 0, 32);
    }

    for (int n = 0; n < 136; n++)
        rkey[n] ^= 0x5c;
    outer.Write(rkey, 136);

    for (int n = 0; n < 136; n++)
        rkey[n] ^= 0x5c ^ 0x36;
    inner.Write(rkey, 136);
}

void CHMAC_SHA3::Finalize(unsigned char hash[OUTPUT_SIZE])
{
    unsigned char temp[32];
    inner.Finalize(temp);
    outer.Write(temp, 32).Finalize(hash);
}
