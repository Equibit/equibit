// Copyright (c) 2020 Equibit Group AG
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef EQUIBIT_BITMSGFILTER_H
#define EQUIBIT_BITMSGFILTER_H

#include <protocol.h>
#include <random.h>
#include <sync.h>
#include <timedata.h>
#include <util.h>

#include <vector>

/** 
 * BitMessage filter - keeps a rolling log of messages sent in the past to
 * avoid processing the same message multiple times.
 */
class CBitMessageFilter
{
private:
    int timeToExpire;
    std::vector<CBitMessage> vMessages;

    void Prune();

public:
    CBitMessageFilter(int pTimeToExpire);

    bool TryAdd(const CBitMessage& msg);
};

#endif // EQUIBIT_BITMSGFILTER_H
