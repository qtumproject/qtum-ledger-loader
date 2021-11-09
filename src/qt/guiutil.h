// Copyright (c) 2011-2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_GUIUTIL_H
#define BITCOIN_QT_GUIUTIL_H

class QWidget;

/** Utility functions used by the Bitcoin Qt UI.
 */
namespace GUIUtil
{
    // Set shortcut to close window
    void handleCloseWindowShortcut(QWidget* w);
} // namespace GUIUtil

#endif // BITCOIN_QT_GUIUTIL_H
