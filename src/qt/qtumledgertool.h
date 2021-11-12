#ifndef QTUMLEDGERTOOL_H
#define QTUMLEDGERTOOL_H

#include <QObject>
#include <QString>
#include <QStringList>
class QtumLedgerToolPriv;
class InstallDevicePriv;

static const QString DEPENDENCY_INSTALL_CMD = "pip3 install --user ledgerblue";
static const QString DEPENDENCY_SHOW_CMD = "pip3 show ledgerblue";

extern bool fMainnet;

/**
 * @brief The InstallDevice class Install Qtum app to device
 */
class InstallDevice
{
public:
    /**
     * @brief The DeviceType enum Supported device type to install
     */
    enum DeviceType
    {
        WalletNanoS,
        StakeNanoS
    };

    /**
     * @brief InstallDevice Constructor
     */
    InstallDevice(InstallDevice::DeviceType type = InstallDevice::WalletNanoS);

    /**
     * @brief ~InstallDevice Destructor
     */
    ~InstallDevice();

    /**
     * @brief deviceToString Device type to string
     * @param type Device type
     * @return String result
     */
    static QString deviceToString(InstallDevice::DeviceType type);

    /**
     * @brief loadCommand Get the load command
     * @param program Program to start
     * @param arguments Program arguments
     * @return Success of the operation
     */
    bool loadCommand(QString &program, QStringList &arguments);

    /**
     * @brief deleteCommand Get the delete command
     * @param program Program to start
     * @param arguments Program arguments
     * @return Success of the operation
     */
    bool deleteCommand(QString &program, QStringList &arguments);

    /**
     * @brief firmwareCommand Get the firmware command
     * @param program Program to start
     * @param arguments Program arguments
     * @return Success of the operation
     */
    bool firmwareCommand(QString &program, QStringList &arguments);

    /**
     * @brief genPairCommand Gen key pair command
     * @param program Program to start
     * @param arguments Program arguments
     * @return Success of the operation
     */
    bool genKeyPairCommand(QString &program, QStringList &arguments);

private:
    bool getRCCommand(const QString &rcPath, QString &program, QStringList &arguments);
    bool getCommand(const QString &command, QString &program, QStringList &arguments);
    QString parse(QString arg);

private:
    InstallDevicePriv *d;
};

struct LedgerAppInfo
{
    QString appName;
    QString appVersion;
    QString appIdentifier;
    QString targetVersion;
    QString fileHash;
};

/**
 * @brief The QtumLedgerTool class Communicate with the Qtum Hardware Wallet Interface Tool
 */
class QtumLedgerTool : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief QtumLedgerTool Constructor
     * @param parent Parent object
     */
    explicit QtumLedgerTool(QObject *parent = nullptr);

    /**
     * @brief ~QtumHwiTool Destructor
     */
    ~QtumLedgerTool();

    /**
     * @brief installApp Install Qtum App to ledger
     * @param type Ledger device type
     * @return success of the operation
     */
    bool installApp(InstallDevice::DeviceType type);

    /**
     * @brief removeApp Remove Qtum App to ledger
     * @param type Ledger device type
     * @return success of the operation
     */
    bool removeApp(InstallDevice::DeviceType type);

    /**
     * @brief checkFirmware Check the firmware
     * @param type Application for install
     * @param message Firmware check message
     * @return whether or not the application can be installed on this firmware
     */
    bool checkFirmware(InstallDevice::DeviceType type, QString& message);

    /**
     * @brief getKeyPair Get key pair for root
     * @return success of the operation
     */
    bool getKeyPair();

    /**
     * @brief errorMessage Get the last error message
     * @return Last error message
     */
    QString errorMessage();

    /**
     * @brief installDependency Install ledgerblue loader
     * @return success of the operation
     */
    bool installDependency();

    /**
     * @brief appInfo Get app info
     * @param type Application for install
     * @return Ledger application information
     */
    LedgerAppInfo appInfo(InstallDevice::DeviceType type);

    /**
     * @brief getPubKey Get public key
     * @param publicKeyP1 Part 1
     * @param publicKeyP2 Part 2
     */
    void getPubKey(QString& publicKeyP1, QString& publicKeyP2);

Q_SIGNALS:

public Q_SLOTS:

private:
    bool isStarted();
    void wait();
    bool checkDataDir();
    QString firmwareMessage(InstallDevice::DeviceType type);

    QtumLedgerToolPriv* d;
};

#endif // QTUMLEDGERTOOL_H
