// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

/**
 * Utilities for converting data from/to strings.
 */
#ifndef BITCOIN_UTIL_STRENCODINGS_H
#define BITCOIN_UTIL_STRENCODINGS_H

#include <string>

/**
 * Format a paragraph of text to a fixed width, adding spaces for
 * indentation to any added line.
 */
std::string FormatParagraph(const std::string& in, size_t width = 79, size_t indent = 0);

#endif // BITCOIN_UTIL_STRENCODINGS_H
