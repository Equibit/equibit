// Copyright (c) 2020 Equibit Group AG
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <bitmsgmgr.h>
#include <utilstrencodings.h>

const int timeToExpire = 2 * 24 * 3600;

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

bool CBitMessageMgr::Validate(const CBitMessage& msg)
{
    int64_t timeLimit = GetTime() - timeToExpire;

    if (msg.messageTime <= timeLimit) {
        LogPrint(BCLog::BITMSG, "bitmessage rejected as too old (messageTime: %d, hash: %s)\n", msg.messageTime, msg.hash.ToString());
        return false;
    }

    if (!IsHex(msg.message)) {
        LogPrint(BCLog::BITMSG, "bitmessage rejected as not hex string (hash: %s)\n", msg.hash.ToString());
        return false;
    }

    return true;
}