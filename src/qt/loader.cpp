// Copyright (c) 2011-2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/loader.h>
#include <qt/qtumledgerinstallerdialog.h>
#include <QApplication>
#include <qt/styleSheet.h>
#include <qt/networkstyle.h>
#include <qt/guiconstants.h>
#include <QString>
#include <QMessageBox>

#if defined(QT_STATICPLUGIN)
#include <QtPlugin>
#if defined(QT_QPA_PLATFORM_XCB)
Q_IMPORT_PLUGIN(QXcbIntegrationPlugin);
#elif defined(QT_QPA_PLATFORM_WINDOWS)
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
#elif defined(QT_QPA_PLATFORM_COCOA)
Q_IMPORT_PLUGIN(QCocoaIntegrationPlugin);
#endif
#endif

bool getNetworkIDString(QStringList arguments, QString& networkID)
{
    int numNetworks = 0;
    if(arguments.contains("-testnet"))
    {
        networkID = "test";
        numNetworks++;
    }
    if(arguments.contains("-signet"))
    {
        networkID = "signet";
        numNetworks++;
    }
    if(arguments.contains("-regtest"))
    {
        networkID = "regtest";
        numNetworks++;
    }
    if(numNetworks == 0)
    {
        networkID = "main";
        numNetworks++;
    }
    return numNetworks == 1;
}

int GuiMain(int argc, char* argv[])
{
    Q_INIT_RESOURCE(qtum);
    // Generate high-dpi pixmaps
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#if QT_VERSION >= 0x050600
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QApplication app(argc, argv);

    // Application identification
    QApplication::setOrganizationName(QAPP_ORG_NAME);
    QApplication::setOrganizationDomain(QAPP_ORG_DOMAIN);
    QApplication::setApplicationName(QAPP_APP_NAME_DEFAULT);

    // Parse command-line options
    QString networkID;
    if(!getNetworkIDString(app.arguments(), networkID))
    {
        QMessageBox::critical(nullptr, PACKAGE_NAME,"Error parsing command line arguments.");
        return EXIT_FAILURE;
    }

    // Set network style
    QScopedPointer<const NetworkStyle> networkStyle(NetworkStyle::instantiate(networkID.toStdString()));
    Q_ASSERT(!networkStyle.isNull());
    // Allow for separate UI settings for testnets
    QApplication::setApplicationName(networkStyle->getAppName());

    // Set application style
    SetObjectStyleSheet(&app, StyleSheetNames::App);

    // Show installer dialog
    QtumLedgerInstallerDialog w;
    QApplication::setWindowIcon(networkStyle->getTrayAndWindowIcon());
    w.setWindowIcon(networkStyle->getTrayAndWindowIcon());
    w.setWindowTitle(w.windowTitle() + " " + networkStyle->getTitleAddText());
    w.show();

    app.exec();
    return EXIT_SUCCESS;
}
