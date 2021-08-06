#include <qt/qtumledgerinstallerdialog.h>
#include <qt/waitmessagebox.h>
#include <qt/qtumversionchecker.h>

#include <QVariant>
#include <QMessageBox>
#include <QAction>
#include <QButtonGroup>
#include <QComboBox>
#include <QDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QWidget>
#include <QCheckBox>
#include <QSettings>
#include <QTimer>

static const bool DEFAULT_CHECK_FOR_UPDATES = true;

class QtumLedgerInstallerDialogPriv
{
public:
    QtumLedgerInstallerDialogPriv(QDialog *parentWidget)
    {
        tool = new QtumLedgerTool(parentWidget);
        setupUi(parentWidget);
    }

    QtumLedgerTool* tool = 0;
    bool ret = false;

    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout;
    QLabel *labelTitle;
    QHBoxLayout *horizontalLayout;
    QLabel *labelLedgerType;
    QComboBox *cbLedgerApp;
    QSpacerItem *verticalSpacer;
    QLabel *labelApp;
    QWidget *buttonsContainerWhite;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *addButton;
    QPushButton *removeButton;
    QSpacerItem *horizontalSpacer;
    QCheckBox *updateCheckBox;

private:
    void setupUi(QDialog *parentWidget)
    {
        if (parentWidget->objectName().isEmpty())
            parentWidget->setObjectName(QStringLiteral("QtumLedgerInstallerDialog"));
        parentWidget->resize(662, 338);
        verticalLayout_2 = new QVBoxLayout(parentWidget);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, -1, 0, 0);
        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(12);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(30, -1, 30, -1);
        labelTitle = new QLabel(parentWidget);
        labelTitle->setObjectName(QStringLiteral("labelTitle"));
        QFont font;
        font.setBold(true);
        font.setWeight(75);
        labelTitle->setFont(font);

        verticalLayout->addWidget(labelTitle);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setContentsMargins(10, -1, -1, -1);
        labelLedgerType = new QLabel(parentWidget);
        labelLedgerType->setObjectName(QStringLiteral("labelLedgerType"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(labelLedgerType->sizePolicy().hasHeightForWidth());
        labelLedgerType->setSizePolicy(sizePolicy);

        horizontalLayout->addWidget(labelLedgerType);

        cbLedgerApp = new QComboBox(parentWidget);
        cbLedgerApp->setObjectName(QStringLiteral("cbLedgerApp"));

        horizontalLayout->addWidget(cbLedgerApp);

        verticalLayout->addLayout(horizontalLayout);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        labelApp = new QLabel(parentWidget);
        labelApp->setObjectName(QStringLiteral("labelApp"));
        labelApp->setWordWrap(true);

        verticalLayout->addWidget(labelApp);

        verticalLayout_2->addLayout(verticalLayout);

        buttonsContainerWhite = new QWidget(parentWidget);
        buttonsContainerWhite->setObjectName(QStringLiteral("buttonsContainerWhite"));
        horizontalLayout_2 = new QHBoxLayout(buttonsContainerWhite);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(30, 15, 30, -1);
        addButton = new QPushButton(buttonsContainerWhite);
        addButton->setObjectName(QStringLiteral("addButton"));

        horizontalLayout_2->addWidget(addButton);

        removeButton = new QPushButton(buttonsContainerWhite);
        removeButton->setObjectName(QStringLiteral("removeButton"));

        horizontalLayout_2->addWidget(removeButton);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);

        updateCheckBox = new QCheckBox(buttonsContainerWhite);
        updateCheckBox->setChecked(DEFAULT_CHECK_FOR_UPDATES);
        updateCheckBox->setObjectName(QStringLiteral("updateCheckBox"));

        horizontalLayout_2->addWidget(updateCheckBox);
        verticalLayout_2->addWidget(buttonsContainerWhite);

        QMetaObject::connectSlotsByName(parentWidget);
    }
};

QtumLedgerInstallerDialog::QtumLedgerInstallerDialog(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle(tr("Qtum ledger application loader"));

    d = new QtumLedgerInstallerDialogPriv(this);
    d->labelTitle->setText(tr("Install or remove QTUM app from ledger."));
    d->labelLedgerType->setText(tr("Select ledger application:"));
    d->labelApp->setText(QString());
    d->addButton->setText(tr("Install"));
    d->removeButton->setText(tr("Remove"));
    d->updateCheckBox->setText(tr("Check for updates"));
    d->cbLedgerApp->addItem(tr("Qtum Wallet Nano S"), InstallDevice::WalletNanoS);
    d->cbLedgerApp->addItem(tr("Qtum Stake Nano S"), InstallDevice::StakeNanoS);
    d->labelApp->setStyleSheet("QLabel { color: red; }");

    QSettings settings;
    if (!settings.contains("fCheckForUpdates"))
        settings.setValue("fCheckForUpdates", DEFAULT_CHECK_FOR_UPDATES);
    bool fCheckForUpdates = settings.value("fCheckForUpdates").toBool();
    d->updateCheckBox->setChecked(fCheckForUpdates);
    if(fCheckForUpdates)
    {
        QTimer::singleShot(1000, this, SLOT(checkForUpdates()));
    }
}

QtumLedgerInstallerDialog::~QtumLedgerInstallerDialog()
{
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
    int deviceType = d->cbLedgerApp->currentData().toInt();
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
        return d->labelApp->setText("");
    case InstallDevice::StakeNanoS:
        return d->labelApp->setText(tr("When Qtum Stake is installed, please turn off the auto lock:\n"
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
