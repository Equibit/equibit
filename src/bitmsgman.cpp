// Copyright (c) 2020 Equibit Group AG
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <bitmsgman.h>

CBitMessageMan bitMsgMan;

bool CBitMessageMan::AddReceived(const CBitMessage& msg)
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

std::vector<CBitMessage> CBitMessageMan::PullReceived()
{
    LOCK(cs_received);
    std::vector<CBitMessage> ret;

    for (const CBitMessage& msg : vUnreadMessages) {
        ret.push_back(msg);
    }

    vUnreadMessages.clear();

    return ret;
}

bool CBitMessageMan::Validate(const CBitMessage& msg)
{
    return true;
}