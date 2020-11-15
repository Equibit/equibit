// playground.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <vector>

#include <utilstrencodings.h>
#include <pubkey.h>
#include <key.h>
#include <pow.h>
#include <chainparams.h>
#include <streams.h>
#include <univalue.h>
#include <boost/algorithm/string.hpp>

static std::unique_ptr<ECCVerifyHandle> globalVerifyHandle;

uint256 SHA256DoubleHash(const unsigned char* data, size_t len)
{
    CSHA256 hasher;
    uint256 hash;
    unsigned char hashIter[32] = {};

    hasher.Write(data, len).Finalize(hashIter);
    hasher.Reset();
    hasher.Write(hashIter, sizeof(hashIter)).Finalize((unsigned char*)&hash);

    return hash;
}

int main()
{
    ECC_Start();
    globalVerifyHandle.reset(new ECCVerifyHandle());
    /*
    std::string messageHex = "034269644425b05f0000000068300300210200c925fb73bfe5e5a9633aae1f25733b3a2ccd13c506bcc7ca7ada1f849689274a7b227175616e74697479223a20353030302c227072696365223a2022302e333333222c2266696c6c5f6f725f6b696c6c223a20226e6f222c226475726174696f6e223a2038363430307d";
    std::vector<unsigned char> messageBytes = ParseHex(messageHex);

    std::string publicKeyHex = "03a6afa2211fc96a4130e767da4a9e802f5e922a151c5cd6d4bffa80358dd1f9a3";
    std::vector<unsigned char> publicKeyBytes = ParseHex(publicKeyHex);

    std::string signatureHex = "3045022100d953d40eed362277ca211aca44714da6628bb436df467856c2763c330e177d760220665470555e0f70bb2318f68dfd16befb83b5dc162efe0d0a23cf6f65ffd418dd01";
    std::vector<unsigned char> signatureBytes = ParseHex(signatureHex);
    */
    std::string signedMessageHex = "034269640379b05f00000000701101002103a6afa2211fc96a4130e767da4a9e802f5e922a151c5cd6d4bffa80358dd1f9a34a7b227175616e74697479223a20353030302c227072696365223a2022302e333333222c2266696c6c5f6f725f6b696c6c223a20226e6f222c226475726174696f6e223a2038363430307d47304402205f9212f80d7926550ee34d3aa61dad4c4efe4968b9d761eb4ca0357f13839b480220241980abc5d4711310044766cfa9795fea2c2f8afc04ed605b63e49ed8c30626013c01000000000000";
    std::vector<unsigned char> signedMessageBytes = ParseHex(signedMessageHex);

    CDataStream messageStream(signedMessageBytes, 0, 0);

    CBitMessage msg;

    messageStream >> msg;

    std::string messageHashHex = HexStr(msg.hash.begin(), msg.hash.end());

    std::cout << "Computed hash: " << messageHashHex << "\n";

    std::cout << "Public key: " << HexStr(msg.data.sender_public_key.begin(), msg.data.sender_public_key.end()) << "\n";

    CPubKey publicKey(msg.data.sender_public_key);

    std::cout << "Key valid = " << publicKey.IsValid() << "\n";

    std::cout << "Signature valid = " << publicKey.Verify(msg.hash, msg.signature) << "\n";

    /*
    uint256 hash = SHA256DoubleHash(messageBytes.data(), messageBytes.size());

    std::string messageHashHex = HexStr(hash.begin(), hash.end());

    std::cout << "Computed hash: " << messageHashHex << "\n";

    CPubKey publicKey(publicKeyBytes);

    std::cout << "Key valid = " << publicKey.IsValid() << "\n";

    std::cout << "Signature valid = " << publicKey.Verify(hash, signatureBytes) << "\n";
    */
    /*
    

    CBitMessageData data;
    std::vector<unsigned char> signature;
    uint64_t nonce;

    messageStream >> data;
    messageStream >> signature;
    messageStream >> nonce;
    */
    //std::cout << "nonce: " << nonce << "\n";

    /*
    stream.write((const char*)signedMessageBytes.data(), (const char*)(signedMessageBytes.size() - sizeof(uint64_t)));

    std::cout << "message into pow: " << HexStr(temp.begin(), temp.end()) << "\n";

    temp.insert(temp.end(), (const unsigned char*)&nonce, (const unsigned char*)&nonce + sizeof(uint64_t));
    */
    /*
    uint256 powHash = SHA256DoubleHash(signedMessageBytes.data(), signedMessageBytes.size());   

    std::string powHashHex = HexStr(powHash.begin(), powHash.end());

    std::cout << "PoW hash: " << powHashHex << "\n";

    int difficulty = 2;
    bool powValid = true;

    for (int i = 1; i <= difficulty; ++i) {
        int p = powHashHex.length() - i;
        if (p < 0 || powHashHex[p] != '0') {
            powValid = false;
            break;
        }
    }   
    
    std::cout << "PoW valid = " << powValid << "\n";
    */

    CBitMessageData data = msg.data;

    UniValue payload;

    if (payload.read(data.payload)) {
        // '{"quantity": 5000,"price": "0.333","fill_or_kill": "no","duration": 86400}'
        if (payload.isObject()) {
            if (payload.exists("quantity")) {
                UniValue quantity = payload["quantity"];
                if (quantity.isNum()) {
                    std::cout << "Quantity: " << quantity.get_int() << "\n";
                }
                else {
                    std::cout << "Payload quantity is not a number: " << data.payload << "\n";
                }
            }
            else {
                std::cout << "Payload missing quantity: " << data.payload << "\n";
            }

            if (payload.exists("price")) {
                UniValue price = payload["price"];
                if (price.isNum() || price.isStr()) {
                    double p = ::atof(price.getValStr().c_str());
                    std::cout << "Price: " << p << "\n";
                }
                else {
                    std::cout << "Payload price is not a string: " << data.payload << "\n";
                }
            }
            else {
                std::cout << "Payload missing price: " << data.payload << "\n";
            }

            if (payload.exists("fill_or_kill")) {
                UniValue fill_or_kill = payload["fill_or_kill"];
                if (fill_or_kill.isStr()) {
                    std::string s = boost::algorithm::to_lower_copy(fill_or_kill.get_str());

                    if (s == "yes" || s == "no") {
                        std::cout << "fill_or_kill: " << s << "\n";
                    }
                    else {
                        std::cout << "Payload fill_or_kill is not a valid option: " << data.payload << "\n";
                    }
                }
                else {
                    std::cout << "Payload fill_or_kill is not a number: " << data.payload << "\n";
                }
            }
            else {
                std::cout << "Payload missing fill_or_kill: " << data.payload << "\n";
            }

            if (payload.exists("duration")) {
                UniValue duration = payload["duration"];
                if (duration.isNum()) {
                    std::cout << "Duration: " << duration.get_int() << "\n";
                }
                else {
                    std::cout << "Payload duration is not a number: " << data.payload << "\n";
                }
            }
            else {
                std::cout << "Payload missing duration: " << data.payload << "\n";
            }
        }
        else {
            std::cout << "Payload is not an object: " << data.payload << "\n";
        }
    }
    else {
        std::cout << "Failed to read payload: " << data.payload << "\n";
    }

    ECC_Stop();
}

