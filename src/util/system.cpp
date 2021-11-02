// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <util/system.h>
#include <tinyformat.h>

std::string GetHelpMessage()
{
    std::string usage = "";
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
