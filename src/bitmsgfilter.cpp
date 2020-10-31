// Copyright (c) 2020 Equibit Group AG
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <bitmsgfilter.h>

CBitMessageFilter::CBitMessageFilter(int pTimeToExpire)
    : timeToExpire(pTimeToExpire)
{
}

bool CBitMessageFilter::TryAdd(const CBitMessage& msg)
{
    int64_t timeLimit = GetTime() - timeToExpire;

    if (msg.messageTime <= timeLimit) {
        return false;
    }

    // TODO: Optimize this using a lookup-based rolling time filter
    for (const CBitMessage& existingMsg : vMessages) {
        if (msg.hash == existingMsg.hash) {
            return false;
        }
    }

    vMessages.push_back(msg);

    Prune();

    return true;
}

void CBitMessageFilter::Prune()
{
    std::vector<CBitMessage>::const_iterator pruneUpTo = vMessages.begin();
    int64_t timeLimit = GetTime() - timeToExpire;

    while (pruneUpTo != vMessages.end()) {
        if (pruneUpTo->messageTime > timeLimit) {
            break;
        }
        pruneUpTo++;
    }

    if (pruneUpTo != vMessages.begin()) {
        vMessages.erase(vMessages.begin(), pruneUpTo);
    }
}