#if defined(HAVE_CONFIG_H)
#include <config/bitcoin-config.h>
#endif

#include <qt/qtumledgerinstallerdialog.h>
#include <qt/forms/ui_qtumledgerinstallerdialog.h>
#include <qt/waitmessagebox.h>
#include <qt/qtumversionchecker.h>
#include <qt/utilitydialog.h>
#include <clientversion.h>

#include <QVariant>
#include <QMessageBox>
#include <QSettings>
#include <QTimer>
#include <QMenu>
#include <QAction>
#include <QMenuBar>
#include <QActionGroup>

static const bool DEFAULT_CHECK_FOR_UPDATES = true;

class QtumLedgerInstallerDialogPriv
{
public:
    QtumLedgerInstallerDialogPriv(QObject *parent)
    {
        tool = new QtumLedgerTool(parent);
    }

    QtumLedgerTool* tool = 0;
    bool ret = false;
    QString message;
    bool mainnet = true;
};

QtumLedgerInstallerDialog::QtumLedgerInstallerDialog(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::QtumLedgerInstallerDialog)
{
    ui->setupUi(this);
    d = new QtumLedgerInstallerDialogPriv(this);

    QString title = windowTitle() + " " + QString::fromStdString(FormatFullVersion());
    setWindowTitle(title);

    ui->cbLedgerApp->addItem(tr("Qtum Wallet Nano S"), InstallDevice::WalletNanoS);
    ui->cbLedgerApp->addItem(tr("Qtum Stake Nano S"), InstallDevice::StakeNanoS);

    ui->labelApp->setStyleSheet("QLabel { color: red; }");

    createActions();
    createMenuBar();

    QSettings settings;
    if (!settings.contains("fCheckForUpdates"))
        settings.setValue("fCheckForUpdates", DEFAULT_CHECK_FOR_UPDATES);
    bool fCheckForUpdates = settings.value("fCheckForUpdates").toBool();
    ui->updateCheckBox->setChecked(fCheckForUpdates);
    if(fCheckForUpdates)
    {
        QTimer::singleShot(1000, this, SLOT(checkForUpdates()));
    }
}

QtumLedgerInstallerDialog::~QtumLedgerInstallerDialog()
{
    delete ui;
    delete d;
}

void QtumLedgerInstallerDialog::on_addButton_clicked()
{
    QString message;
    if(checkFirmware(message))
    {
        if(message.isEmpty())
        {
            installApp();
        }
        else
        {
            QString title = getDeviceAppTitle(true);
            QMessageBox::warning(this, title, message);
        }
    }

    bool dependency = false;
    if(!d->ret && parseErrorMessage(message, dependency))
    {
        QString title = getDeviceAppTitle(true);
        QMessageBox::warning(this, title, message);
    }

    if(dependency)
        installDependency();
}

void QtumLedgerInstallerDialog::on_removeButton_clicked()
{
    // Remove Qtum app from ledger
    d->tool->getKeyPair();
    WaitMessageBox dlg(tr("Ledger Status"), uninstallInfo(getDeviceType()), [this]() {
        d->ret = d->tool->removeApp(getDeviceType());
    }, this);

    dlg.exec();

    QString message;
    bool dependency = false;
    if(!d->ret && parseErrorMessage(message, dependency))
    {
        QString title = getDeviceAppTitle(false);
        QMessageBox::warning(this, title, message);
    }

    if(dependency)
        installDependency();
}

void QtumLedgerInstallerDialog::on_updateCheckBox_clicked(bool checked)
{
    QSettings settings;
    settings.setValue("fCheckForUpdates", checked);
}

InstallDevice::DeviceType QtumLedgerInstallerDialog::getDeviceType()
{
    int deviceType = ui->cbLedgerApp->currentData().toInt();
    switch (deviceType) {
    case InstallDevice::WalletNanoS:
        return InstallDevice::WalletNanoS;
    case InstallDevice::StakeNanoS:
        return InstallDevice::StakeNanoS;
    default:
        break;
    }

    return InstallDevice::WalletNanoS;
}

bool QtumLedgerInstallerDialog::parseErrorMessage(QString &message, bool& dependency)
{
    dependency = false;
    QString errorMessage = d->tool->errorMessage();
    if(errorMessage.contains("denied by the user", Qt::CaseInsensitive) ||
            errorMessage.contains("Invalid status 5501", Qt::CaseInsensitive))
        return false;

    if(errorMessage.contains("ModuleNotFoundError", Qt::CaseInsensitive) && errorMessage.contains("ledgerblue", Qt::CaseInsensitive))
    {
        message = tr("Module ledgerblue not found, you can install it manually with the command:") + "\n" + DEPENDENCY_INSTALL_CMD;
        dependency = true;
        return true;
    }

    if(errorMessage.contains("No dongle found", Qt::CaseInsensitive))
    {
        message = tr("Please insert your Ledger. Verify the cable is connected and that no other application is using it.");
        return true;
    }

    if(errorMessage.contains("verify that the right application is opened", Qt::CaseInsensitive))
    {
        message = tr("Please close the Qtum application on your ledger.");
        return true;
    }

    if(errorMessage.contains("Invalid status 5103", Qt::CaseInsensitive))
    {
        message = tr("Not enough memory on your ledger.");
        return true;
    }

    message = d->tool->errorMessage();
    return true;
}

void QtumLedgerInstallerDialog::on_cbLedgerApp_currentIndexChanged(int index)
{
    int deviceType = index;
    switch (deviceType) {
    case InstallDevice::WalletNanoS:
        ui->labelInfo->setText(appInfo(InstallDevice::WalletNanoS));
        return ui->labelApp->setText("");
    case InstallDevice::StakeNanoS:
        ui->labelInfo->setText(appInfo(InstallDevice::StakeNanoS));
        return ui->labelApp->setText(tr("When Qtum Stake is installed, please turn off the PIN lock:\n"
                                        "Nano S > Settings > Security > PIN lock > Off\n"
                                        "\n"
                                        "When Qtum Stake is removed, please turn on the PIN lock:\n"
                                        "Nano S > Settings > Security > PIN lock > 10 minutes\n"));
    default:
        break;
    }
}

void QtumLedgerInstallerDialog::checkForUpdates()
{
    // Check for updates
    QtumVersionChecker qtumVersionChecker(this);
    if(qtumVersionChecker.newVersionAvailable())
    {
        QString link = QString("<a href=%1>%2</a>").arg(QTUM_RELEASES, QTUM_RELEASES);
        QString message(tr("New version of Qtum ledger application loader is available on the Qtum source code repository: <br /> %1. <br />It is recommended to download it and update this application").arg(link));
        QMessageBox::information(this, tr("Check for updates"), message);
    }
}

void QtumLedgerInstallerDialog::installDependency()
{
    QString message = tr("Would you like to install module ledgerblue now?");
    QMessageBox::StandardButton retval = QMessageBox::question(this, tr("Install missing module"), message,
        QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);

    if(retval == QMessageBox::Yes)
    {
        // Remove Qtum app from ledger
        WaitMessageBox dlg(tr("Missing module"), tr("Installing module ledgerblue..."), [this]() {
            d->ret = d->tool->installDependency();
        }, this);

        dlg.exec();

        if(!d->ret)
        {
            QMessageBox::warning(this, tr("Missing module"), tr("Fail to install module ledgerblue"));
        }
    }
}

QString QtumLedgerInstallerDialog::getDeviceAppTitle(bool install)
{
    if(install)
        return getDeviceType() == InstallDevice::WalletNanoS ? tr("Qtum Wallet install problem") : tr("Qtum Stake install problem");
    return getDeviceType() == InstallDevice::WalletNanoS ? tr("Qtum Wallet remove problem") : tr("Qtum Stake remove problem");
}

void QtumLedgerInstallerDialog::createActions()
{
    mainnetAction = new QAction(tr("&Mainnet Apps"), this);
    mainnetAction->setStatusTip(tr("Select mainnet ledger applications"));
    mainnetAction->setCheckable(true);
    connect(mainnetAction, &QAction::triggered, this, &QtumLedgerInstallerDialog::mainnetClicked);

    testnetAction = new QAction(tr("&Testnet Apps"), this);
    testnetAction->setStatusTip(tr("Select testnet ledger applications"));
    testnetAction->setCheckable(true);
    connect(testnetAction, &QAction::triggered, this, &QtumLedgerInstallerDialog::testnetClicked);

    aboutAction = new QAction(tr("&About %1").arg(PACKAGE_NAME), this);
    aboutAction->setStatusTip(tr("Show information about %1").arg(PACKAGE_NAME));
    aboutAction->setMenuRole(QAction::AboutRole);
    connect(aboutAction, &QAction::triggered, this, &QtumLedgerInstallerDialog::aboutClicked);

    aboutQtAction = new QAction(tr("About &Qt"), this);
    aboutQtAction->setStatusTip(tr("Show information about Qt"));
    aboutQtAction->setMenuRole(QAction::AboutQtRole);
    connect(aboutQtAction, &QAction::triggered, qApp, QApplication::aboutQt);
}

void QtumLedgerInstallerDialog::aboutClicked()
{
    HelpMessageDialog dlg(this, true);
    dlg.exec();
}

void QtumLedgerInstallerDialog::createMenuBar()
{
#ifdef Q_OS_MAC
    // Create a decoupled menu bar on Mac which stays even if the window is closed
    appMenuBar = new QMenuBar();
#else
    // Get the main window's menu bar on other platforms
    appMenuBar = menuBar();
#endif

    QMenu *network = appMenuBar->addMenu(tr("&Network"));
    QActionGroup* group = new QActionGroup(this);
    group->addAction(mainnetAction);
    group->addAction(testnetAction);
    mainnetAction->setChecked(true);
    network->addAction(mainnetAction);
    network->addAction(testnetAction);

    QMenu *help = appMenuBar->addMenu(tr("&Help"));
    help->addAction(aboutAction);
    help->addAction(aboutQtAction);
}

QString QtumLedgerInstallerDialog::installInfo(InstallDevice::DeviceType type)
{
    QString publicKeyP1, publicKeyP2;
    d->tool->getPubKey(publicKeyP1, publicKeyP2);
    LedgerAppInfo info = d->tool->appInfo(type);
    QString message = tr("Confirm Qtum install on your Ledger device...\n\n");
    message += "Public key:\n%1\n%2\n";
    message += "Allow manager\n";
    message += "Install app %3\n";
    message += "Version %4\n";
    message += "Identifier:\n%5\n";
    message += "Perform Installation\n";
    return message.arg(publicKeyP1, publicKeyP2, info.appName, info.appVersion, info.appIdentifier);
}

QString QtumLedgerInstallerDialog::uninstallInfo(InstallDevice::DeviceType type)
{
    QString publicKeyP1, publicKeyP2;
    d->tool->getPubKey(publicKeyP1, publicKeyP2);
    LedgerAppInfo info = d->tool->appInfo(type);
    QString message = tr("Confirm Qtum removal on your Ledger device...\n\n");
    message += "Public key:\n%1\n%2\n";
    message += "Allow manager\n";
    message += "Uninstall %3\n";
    message += "Confirm action\n";
    return message.arg(publicKeyP1, publicKeyP2, info.appName);
}

QString QtumLedgerInstallerDialog::appInfo(InstallDevice::DeviceType type)
{
    LedgerAppInfo info = d->tool->appInfo(type);
    QString message = tr("App name:\t%1\nApp version:\t%2\nTarget version:\t%3\nHash app.hex:\t%4\n");
    return message.arg(info.appName, info.appVersion, info.targetVersion, info.fileHash);
}

QString QtumLedgerInstallerDialog::firmwareInfo()
{
    QString message = tr("Allow reading firmware information from your Ledger device...\n\n");
    message += "Public key (random)\n";
    message += "Allow manager\n";
    return message;
}

bool QtumLedgerInstallerDialog::installApp()
{
    // Install Qtum app from ledger
    d->tool->getKeyPair();
    WaitMessageBox dlg(tr("Ledger Status"), installInfo(getDeviceType()), [this]() {
        d->ret = d->tool->installApp(getDeviceType());
    }, this);

    dlg.exec();
    return d->ret;
}

bool QtumLedgerInstallerDialog::checkFirmware(QString &message)
{
    // Check ledger firmware
    d->message.clear();
    WaitMessageBox dlg(tr("Ledger Status"), firmwareInfo(), [this]() {
        d->ret = d->tool->checkFirmware(getDeviceType(), d->message);
    }, this);

    dlg.exec();
    message = d->message;
    return d->ret;
}

void QtumLedgerInstallerDialog::mainnetClicked()
{
    fMainnet = true;
    refreshNetAppInfo();
}

void QtumLedgerInstallerDialog::testnetClicked()
{
    fMainnet = false;
    refreshNetAppInfo();
}

void QtumLedgerInstallerDialog::refreshNetAppInfo()
{
    int index = ui->cbLedgerApp->currentIndex();
    on_cbLedgerApp_currentIndexChanged(index);
}
