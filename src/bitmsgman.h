// Copyright (c) 2020 Equibit Group AG
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef EQUIBIT_BITMSGMAN_H
#define EQUIBIT_BITMSGMAN_H

#include <bitmsgfilter.h>
#include <netaddress.h>
#include <protocol.h>
#include <random.h>
#include <sync.h>
#include <timedata.h>
#include <util.h>

#include <vector>

/** 
 * BitMessage manager
 */
class CBitMessageMan
{
private:
    //! critical section to protect the inner data structures
    mutable CCriticalSection cs_received;

    CBitMessageFilter msgFilter;
    std::vector<CBitMessage> vUnreadMessages;

public:
    //! Add a new received bitmessage.
    bool AddReceived(const CBitMessage& msg);

    std::vector<CBitMessage> PullReceived();

    bool Validate(const CBitMessage& msg);
};

extern CBitMessageMan bitMsgMan;

#endif // EQUIBIT_BITMSGMAN_H
