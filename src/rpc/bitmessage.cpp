// Copyright (c) 2020 Equibit Group AG
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <rpc/server.h>

#include <bitMsgMgr.h>
#include <chainparams.h>
#include <clientversion.h>
#include <core_io.h>
#include <validation.h>
#include <net.h>
#include <net_processing.h>
#include <netbase.h>
#include <policy/policy.h>
#include <rpc/protocol.h>
#include <sync.h>
#include <timedata.h>
#include <ui_interface.h>
#include <univalue.h>
#include <util.h>
#include <utilstrencodings.h>
#include <version.h>
#include <warnings.h>

UniValue pullrawmessages(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() != 0)
        throw std::runtime_error(
            "pullrawmessages\n"
            "\nRetrieves the current list of received BitMessages.\n"
            "Once a message is pulled it will be removed from the node cache and will not be pulled a second time.\n"
            "\nExamples:\n"
            + HelpExampleCli("pullrawmessages", "")
            + HelpExampleRpc("pullrawmessages", "")
        );

    if (!g_connman)
        throw JSONRPCError(RPC_CLIENT_P2P_DISABLED, "Error: Peer-to-peer functionality missing or disabled");

    std::vector<CBitMessage> messages = bitMsgMgr.PullReceived();

    UniValue ret(UniValue::VARR);

    for (const CBitMessage& msg : messages) {
        ret.push_back(msg.ToHex());
    }

    return ret;
}

UniValue sendrawmessage(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() != 1 || request.params[0].isNull())
        throw std::runtime_error(
            "sendrawmessage \"msg\"\n"
            "\nSends a raw BitMessage out to all nodes in the network\n"
            "\nArguments:\n"
            "1. \"msg\"   (string, required) Specifies the content of the BitMessage that's being sent out.\n"
            "\nExamples:\n"
            + HelpExampleCli("sendrawmessage", "\"<long hex string>\"")
            + HelpExampleRpc("sendrawmessage", "\"<long hex string>\"")
        );

    if (!g_connman)
        throw JSONRPCError(RPC_CLIENT_P2P_DISABLED, "Error: Peer-to-peer functionality missing or disabled");

    CBitMessage msg;

    if (!msg.FromHex(request.params[0].get_str())) {
        throw JSONRPCError(RPC_BITMSG_INVALID, "Error: BitMessage did not read correctly");
    }

    std::string error = bitMsgMgr.Validate(msg);
    
    if (error.empty()) {
        LogPrint(BCLog::BITMSG, "sending BitMessage into network (hash: %s)\n", msg.hash.ToString());

        // Request that each node send out the message on next processing pass
        g_connman->ForEachNode([&msg](CNode* pnode) {
            pnode->PushBitMessage(msg);
        });
    } 
    else {
        LogPrint(BCLog::BITMSG, "%s\n", error);
        throw JSONRPCError(RPC_BITMSG_INVALID, error);
    }

    return NullUniValue;
}

static const CRPCCommand commands[] =
{ //  category              name                      actor (function)         argNames
  //  --------------------- ------------------------  -----------------------  ----------
    { "bitmessage",         "pullrawmessages",        &pullrawmessages,        {} },
    { "bitmessage",         "sendrawmessage",         &sendrawmessage,         {"msg"} },
};

void RegisterBitMessageRPCCommands(CRPCTable &t)
{
    for (unsigned int vcidx = 0; vcidx < ARRAYLEN(commands); vcidx++)
        t.appendCommand(commands[vcidx].name, &commands[vcidx]);
}
