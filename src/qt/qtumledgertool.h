#ifndef QTUMLEDGERTOOL_H
#define QTUMLEDGERTOOL_H

#include <QObject>
#include <QString>
#include <QStringList>
class QtumLedgerToolPriv;
class InstallDevicePriv;

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

private:
    bool getRCCommand(const QString &rcPath, QString &program, QStringList &arguments);
    QString parse(QString arg);

private:
    InstallDevicePriv *d;
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
     * @brief errorMessage Get the last error message
     * @return Last error message
     */
    QString errorMessage();

Q_SIGNALS:

public Q_SLOTS:

private:
    bool isStarted();
    void wait();

    QtumLedgerTool* d;
};

#endif // QTUMLEDGERTOOL_H
