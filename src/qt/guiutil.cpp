// Copyright (c) 2011-2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/guiutil.h>

#include <Qt>
#include <QWidget>
#include <QShortcut>
#include <QKeySequence>

namespace GUIUtil {

void handleCloseWindowShortcut(QWidget* w)
{
    QObject::connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_W), w), &QShortcut::activated, w, &QWidget::close);
}

} // namespace GUIUtil
