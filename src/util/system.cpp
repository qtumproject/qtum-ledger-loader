// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <util/system.h>
#include <util/strencodings.h>
#include <tinyformat.h>

static const int screenWidth = 79;
static const int optIndent = 2;
static const int msgIndent = 7;

std::string HelpMessageGroup(const std::string &message) {
    return std::string(message) + std::string("\n\n");
}

std::string HelpMessageOpt(const std::string &option, const std::string &message) {
    return std::string(optIndent,' ') + std::string(option) +
           std::string("\n") + std::string(msgIndent,' ') +
           FormatParagraph(message, screenWidth - msgIndent, msgIndent) +
           std::string("\n\n");
}

std::string GetHelpMessage()
{
    std::string usage = "";
    usage += HelpMessageGroup("Chain ledger application selection options:");
    usage += HelpMessageOpt("-main", "Use the main chain ledger application (default).");
    usage += HelpMessageOpt("-testnet", "Use the testnet chain ledger application.");
    usage += HelpMessageOpt("-signet", "Use the signet chain ledger application.");
    usage += HelpMessageOpt("-regtest", "Use the regtest chain ledger application.");
    return usage;
}

std::string CopyrightHolders(const std::string& strPrefix)
{
    const auto copyright_devs = strprintf(COPYRIGHT_HOLDERS, COPYRIGHT_HOLDERS_SUBSTITUTION);
    std::string strCopyrightHolders = strPrefix + copyright_devs;

    // Make sure Bitcoin Core copyright is not removed by accident
    if (copyright_devs.find("Qtum Core") == std::string::npos) {
        strCopyrightHolders += "\n" + strPrefix + "The Qtum Core Developers";
    }
    return strCopyrightHolders;
}
