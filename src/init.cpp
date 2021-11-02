// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#if defined(HAVE_CONFIG_H)
#include <config/bitcoin-config.h>
#endif

#include <init.h>
#include <util/system.h>
#include <tinyformat.h>

std::string LicenseInfo()
{
    const std::string URL_SOURCE_CODE = "<https://github.com/qtumproject/qtum>";

    return CopyrightHolders(strprintf("Copyright (C) %i", COPYRIGHT_YEAR) + " ") + "\n" +
           "\n" +
           strprintf("Please contribute if you find %s useful. "
                       "Visit %s for further information about the software.",
               PACKAGE_NAME, "<" PACKAGE_URL ">") +
           "\n" +
           strprintf("The source code is available from %s.",
               URL_SOURCE_CODE) +
           "\n" +
           "\n" +
           "This is experimental software." + "\n" +
           strprintf("Distributed under the MIT software license, see the accompanying file %s or %s", "COPYING", "<https://opensource.org/licenses/MIT>") +
           "\n";
}
