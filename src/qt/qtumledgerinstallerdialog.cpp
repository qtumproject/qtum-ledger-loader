#include <qt/qtumledgerinstallerdialog.h>
#include <qt/forms/ui_qtumledgerinstallerdialog.h>
#include <qt/waitmessagebox.h>
#include <qt/qtumversionchecker.h>

#include <QVariant>
#include <QMessageBox>
#include <QSettings>
#include <QTimer>

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
};

QtumLedgerInstallerDialog::QtumLedgerInstallerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QtumLedgerInstallerDialog)
{
    ui->setupUi(this);
    d = new QtumLedgerInstallerDialogPriv(this);

    ui->cbLedgerApp->addItem(tr("Qtum Wallet Nano S"), InstallDevice::WalletNanoS);
    ui->cbLedgerApp->addItem(tr("Qtum Stake Nano S"), InstallDevice::StakeNanoS);

    ui->labelApp->setStyleSheet("QLabel { color: red; }");
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
    // Install Qtum app from ledger
    WaitMessageBox dlg(tr("Ledger Status"), tr("Confirm Qtum install on your Ledger device..."), [this]() {
        d->ret = d->tool->installApp(getDeviceType());
    }, this);

    dlg.exec();

    QString message;
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
    WaitMessageBox dlg(tr("Ledger Status"), tr("Confirm Qtum removal on your Ledger device..."), [this]() {
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
    if(errorMessage.contains("denied by the user", Qt::CaseInsensitive))
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

    message = d->tool->errorMessage();
    return true;
}

void QtumLedgerInstallerDialog::on_cbLedgerApp_currentIndexChanged(int index)
{
    int deviceType = index;
    switch (deviceType) {
    case InstallDevice::WalletNanoS:
        return ui->labelApp->setText("");
    case InstallDevice::StakeNanoS:
        return ui->labelApp->setText(tr("When Qtum Stake is installed, please turn off the auto lock:\n"
                                        "Nano S > Settings > Security > Auto-lock > OFF\n"
                                        "\n"
                                        "When Qtum Stake is removed, please turn on the auto lock:\n"
                                        "Nano S > Settings > Security > Auto-lock > 10 minutes\n"));
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
