// Copyright (c) 2020 Equibit Group AG
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <bitmsgmgr.h>
#include <boost/algorithm/string.hpp>
#include <chainparams.h>
#include <pow.h>
#include <pubkey.h>
#include <streams.h>
#include <univalue.h>
#include <utilstrencodings.h>

// Messages cannot be more than 2 days old.
const int timeToExpire = 2 * 24 * 3600;

// Messages can be received with a timestamp up to 10 minutes in the future.
const int timeFuzz = 10 * 60;

std::string emptyMessage;

CBitMessageMgr bitMsgMgr;

CBitMessageMgr::CBitMessageMgr()
    : msgFilter(timeToExpire)
{
}

bool CBitMessageMgr::AddReceived(const CBitMessage& msg)
{
    LOCK(cs_received);
    
    if (msgFilter.TryAdd(msg)) {
        vUnreadMessages.push_back(msg);
        return true;
    }
    else {
        return false;
    }
}

std::vector<CBitMessage> CBitMessageMgr::PullReceived()
{
    LOCK(cs_received);
    std::vector<CBitMessage> ret;

    for (const CBitMessage& msg : vUnreadMessages) {
        ret.push_back(msg);
    }

    vUnreadMessages.clear();

    return ret;
}

std::string CBitMessageMgr::Validate(const CBitMessage& msg) const
{
    int64_t currentTime = GetTime();

    LogPrint(BCLog::BITMSG, "validating bitmessage (type: %s, timestamp: %llu, payload: %s, hash: %s)\n", msg.data.type, msg.data.timestamp, msg.data.payload, msg.hash.ToString());

    if (msg.data.timestamp <= currentTime - timeToExpire) {
        return strprintf("Message rejected as too old (timestamp: %llu, hash: %s)", msg.data.timestamp, msg.hash.ToString());
    }

    if (msg.data.timestamp >= (currentTime + timeFuzz)) {
        return strprintf("Message rejected as too far in the future (timestamp: %llu, hash: %s)", msg.data.timestamp, msg.hash.ToString());
    }

    // Verify the signature.
    CPubKey publicKey(msg.data.sender_public_key);

    if (!publicKey.IsValid()) {
        return strprintf("Message rejected with invalid public key (hash: %s)", msg.hash.ToString());
    }

    if (!publicKey.Verify(msg.hash, msg.signature)) {
        return strprintf("Message rejected with invalid signature (hash: %s)", msg.hash.ToString());
    }

    // Check the PoW
    if (!VerifyProofOfWork(msg)) {
        return strprintf("Message rejected with invalid proof-of-work (hash: %s)", msg.hash.ToString());
    }

    LogPrint(BCLog::BITMSG, "BitMessage passed signature and proof-of-work checks (hash: %s)\n", msg.hash.ToString());

    // Validate the payload
    std::string error = ValidatePayload(msg);
    
    if (!error.empty()) {
        return strprintf("Message rejected with invalid payload (hash: %s): %s", error);
    }

    return emptyMessage;
}

std::string CBitMessageMgr::ValidatePayload(const CBitMessage& msg) const
{
    if (msg.data.type.empty()) {
        return "Message type is required";
    }

    if (boost::iequals(msg.data.type, "Bid") || boost::iequals(msg.data.type, "Ask")) {
        UniValue payload;

        int quantity;
        double price;
        std::string fill_or_kill;
        int duration;

        if (!payload.read(msg.data.payload) || !payload.isObject()) {
            return "Message payload is not a JSON document";
        }

        if (payload.exists("quantity")) {
            UniValue quantityVal = payload["quantity"];
            if (quantityVal.isNum()) {
                quantity = quantityVal.get_int();
                if (quantity <= 0) {
                    return "quantity must be greater than zero";
                }
            } else {
                return "quantity must be a number";
            }
        } else {
            return "quantity is required";
        }

        if (payload.exists("price")) {
            UniValue priceVal = payload["price"];
            if (priceVal.isNum() || priceVal.isStr()) {
                price = ::atof(priceVal.getValStr().c_str());
                if (price <= 0) {
                    return "price must be greater than zero";
                }
            } else {
                return "price must be a string";
            }
        } else {
            return "price is required";
        }

        if (payload.exists("fill_or_kill")) {
            UniValue fill_or_killVal = payload["fill_or_kill"];
            if (fill_or_killVal.isStr()) {
                fill_or_kill = fill_or_killVal.get_str();

                if (!boost::iequals(fill_or_kill, "Yes") && !boost::iequals(fill_or_kill, "No")) {
                    return "fill_or_kill must be either Yes or No";
                }
            } else {
                return "fill_or_kill must be a string";
            }
        } else {
            return "fill_or_kill is required";
        }

        if (payload.exists("duration")) {
            UniValue durationVal = payload["duration"];
            if (durationVal.isNum()) {
                duration = durationVal.get_int();
                if (duration <= 0) {
                    return "duration must be greater than zero";
                }
            } else {
                return "duration must be a number";
            }
        } else {
            return "duration is required";
        }

        LogPrint(BCLog::BITMSG, "BitMessage of type %s received (quantity: %d, price: %f, fill_or_kill: %s, duration: %d, hash: %s)\n",
            msg.data.type, quantity, price, fill_or_kill, duration, msg.hash.ToString());
    } else {
        LogPrint(BCLog::BITMSG, "BitMessage of type %s received (hash: %s)\n", msg.data.type, msg.hash.ToString());
    }

    return emptyMessage;
}

bool CBitMessageMgr::VerifyProofOfWork(const CBitMessage& msg) const
{
    const int difficulty = 2;

    uint256 messageHash = msg.ComputeMessageHash();

    std::string powHashHex = HexStr(messageHash.begin(), messageHash.end());

    for (int i = 1; i <= difficulty; ++i) {
        int p = powHashHex.length() - i;

        if (p < 0 || powHashHex[p] != '0') {
            return false;
        }
    }

    return true;
}