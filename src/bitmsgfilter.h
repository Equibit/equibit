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
 * BitMessage filter - ensures that messages are only preserved for 2 days and never double-sent
 */
class CBitMessageFilter
{
private:
    std::vector<CBitMessage> vMessages;

    void Prune();

public:
    bool TryAdd(const CBitMessage& msg);
};

#endif // EQUIBIT_BITMSGFILTER_H
