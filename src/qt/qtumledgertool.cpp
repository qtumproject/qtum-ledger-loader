#include <qt/qtumledgertool.h>

#include <QProcess>
#include <QFile>
#include <QStringList>
#include <QByteArray>
#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QCryptographicHash>
#include <QMap>

#include <atomic>

static const QString LOAD_FORMAT = ":/ledger/%1_load";
static const QString DELETE_FORMAT = ":/ledger/%1_delete";
static const QString RC_PATH_FORMAT = ":/ledger";
static const char* INFO_APP_FORMAT = "App name:\t%1\nApp version:\t%2\nTarget version:\t%3\nSha256 hash:\t%4\nRoot private key:\t%5";

QString GetDataDir()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir d{path};
    if(d.mkpath(d.absolutePath()))
    {
        return d.absolutePath();
    }
    return "";
}

class QtumLedgerToolPriv
{
public:
    QtumLedgerToolPriv(QObject *parent)
    {}

    std::atomic<bool> fStarted{false};
    QProcess process;
    QString strStdout;
    QString strError;
    QMap<InstallDevice::DeviceType, QString> info;
};

QtumLedgerTool::QtumLedgerTool(QObject *parent) : QObject(parent)
{
    d = new QtumLedgerToolPriv(this);
}

QtumLedgerTool::~QtumLedgerTool()
{
    delete d;
}

QString QtumLedgerTool::errorMessage()
{
    // Get the last error message
    if(d->fStarted)
        return tr("Started");

    return d->strError;
}

bool QtumLedgerTool::isStarted()
{
    return d->fStarted;
}

void QtumLedgerTool::wait()
{
    if(d->fStarted)
    {
        bool wasStarted = false;
        if(d->process.waitForStarted())
        {
            wasStarted = true;
            d->process.waitForFinished(-1);
        }
        d->strStdout = d->process.readAllStandardOutput();
        d->strError = d->process.readAllStandardError();
        d->fStarted = false;
        if(!wasStarted && d->strError.isEmpty())
        {
            d->strError = tr("Application %1 fail to start.").arg(d->process.program());
        }
    }
}

class InstallDevicePriv
{
public:
    ~InstallDevicePriv()
    {
        for(QString path : filePaths)
        {
            QFile::remove(path);
        }
    }

    InstallDevice::DeviceType type = InstallDevice::WalletNanoS;
    QStringList filePaths;
};

InstallDevice::InstallDevice(InstallDevice::DeviceType type)
{
    d = new InstallDevicePriv();
    d->type = type;
}

InstallDevice::~InstallDevice()
{
    delete d;
}

bool isMainnet()
{
    QStringList arguments = qApp->arguments();
    if(arguments.contains("-regtest") || arguments.contains("-testnet") || arguments.contains("-signet"))
        return false;
    return true;
}

QString InstallDevice::deviceToString(InstallDevice::DeviceType type)
{
    bool mainnet = isMainnet();
    switch (type) {
    case InstallDevice::WalletNanoS:
        return mainnet ? "nanos" : "nanos_test";
    case InstallDevice::StakeNanoS:
        return mainnet ? "nanos_stake" : "nanos_stake_test";
    default:
        break;
    }

    return "";
}

bool InstallDevice::loadCommand(QString &program, QStringList &arguments)
{
    QString rcPath = LOAD_FORMAT.arg(deviceToString(d->type));
    return getRCCommand(rcPath, program, arguments);
}

bool InstallDevice::deleteCommand(QString &program, QStringList &arguments)
{
    QString rcPath = DELETE_FORMAT.arg(deviceToString(d->type));
    return getRCCommand(rcPath, program, arguments);
}

bool InstallDevice::getRCCommand(const QString &rcPath, QString &program, QStringList &arguments)
{
    // Get the command
    QString command;
    QFile file(rcPath);
    if(file.open(QIODevice::ReadOnly))
    {
        command = file.readAll().trimmed();
    }
    else
    {
        return false;
    }

    // Split to params
    arguments.clear();
    QStringList args = command.split(" ");
    for(QString arg: args)
    {
        arguments.push_back(parse(arg));
    }
    bool ret = arguments.count() > 1;
    if(ret)
    {
        program = arguments[0];
        arguments.removeAt(0);
    }

    return ret;
}

QString InstallDevice::parse(QString arg)
{
    arg = arg.replace("&nbsp;", " ");
    arg = arg.remove("\"");
    if(arg.startsWith(RC_PATH_FORMAT))
    {
        QString dataDir = GetDataDir();
        QFile fileIn(arg);
        if(fileIn.open(QIODevice::ReadOnly))
        {
            QByteArray data = fileIn.readAll();
            arg = arg.replace(RC_PATH_FORMAT, dataDir);
            arg += ".hex";
            QFile fileOut(arg);
            if(fileOut.open(QIODevice::WriteOnly))
            {
                fileOut.write(data);
            }
            d->filePaths << arg;
        }
    }
    return arg;
}

bool QtumLedgerTool::installApp(InstallDevice::DeviceType type)
{
    // Check data dir
    if(!checkDataDir())
        return false;

    // Install Qtum App to ledger
    InstallDevice device(type);
    QString program;
    QStringList arguments;
    bool ret = device.loadCommand(program, arguments);
    if(ret)
    {
        d->process.start(program, arguments);
        d->fStarted = true;

        wait();

        ret &= QProcess::NormalExit == d->process.exitStatus();
        ret &= d->strError.isEmpty();
    }

    return ret;
}

bool QtumLedgerTool::removeApp(InstallDevice::DeviceType type)
{
    // Check data dir
    if(!checkDataDir())
        return false;

    // Remove Qtum App from ledger
    InstallDevice device(type);
    QString program;
    QStringList arguments;
    bool ret = device.deleteCommand(program, arguments);
    if(ret)
    {
        d->process.start(program, arguments);
        d->fStarted = true;

        wait();

        ret &= QProcess::NormalExit == d->process.exitStatus();
        ret &= d->strError.isEmpty();
    }

    return ret;
}

bool QtumLedgerTool::checkDataDir()
{
    if(GetDataDir().isEmpty())
    {
        d->strError = tr("Fail to create temporary data directory on disk");
        return false;
    }
    return true;
}

bool QtumLedgerTool::installDependency()
{
    // Install dependencies
    bool ret = true;
    QStringList arguments = DEPENDENCY_INSTALL_CMD.split(" ");
    QString program = arguments[0];
    arguments.removeAt(0);
    d->process.start(program, arguments);
    d->fStarted = true;
    wait();
    ret &= QProcess::NormalExit == d->process.exitStatus();

    // Check dependencies
    if(ret)
    {
        arguments = DEPENDENCY_SHOW_CMD.split(" ");
        program = arguments[0];
        arguments.removeAt(0);
        d->process.start(program, arguments);
        d->fStarted = true;
        wait();
        ret &= QProcess::NormalExit == d->process.exitStatus();
        ret &= d->strStdout.contains("ledgerblue");
    }

    return ret;
}

QString QtumLedgerTool::infoApp(InstallDevice::DeviceType type)
{
    // Check the list
    if(d->info.contains(type))
    {
        return d->info[type];
    }

    // Parameters list
    QString appName;
    QString appVersion;
    QString targetVersion;
    QString fileHash;
    QString rootPrivateKey;

    // Get Qtum App info from arguments
    InstallDevice device(type);
    QString program;
    QStringList arguments;
    QString fileName;
    bool ret = device.loadCommand(program, arguments);
    if(ret)
    {
        for(int i = 0; i < arguments.size(); i++)
        {
            QString argument = arguments[i];
            if(argument == "--appName")
            {
                i++;
                if(i < arguments.size())
                {
                    appName = arguments[i];
                }
            }
            else if(argument == "--appVersion")
            {
                i++;
                if(i < arguments.size())
                {
                    appVersion = arguments[i];
                }
            }
            else if(argument == "--fileName")
            {
                i++;
                if(i < arguments.size())
                {
                    fileName = arguments[i];
                }
            }
            else if(argument == "--rootPrivateKey")
            {
                i++;
                if(i < arguments.size())
                {
                    rootPrivateKey = arguments[i];
                }
            }
            else if(argument.startsWith("--targetVersion"))
            {
                QStringList target = argument.split("=");
                if(target.size() > 1)
                {
                    targetVersion = target[1];
                }
            }
        }
    }

    // Compute the hash
    if(QFile::exists(fileName))
    {
        QFile fileIn(fileName);
        if(fileIn.open(QIODevice::ReadOnly))
        {
            QByteArray data = fileIn.readAll();
            fileHash = QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex();
        }
    }

    // Add the info into the list
    QString info = tr(INFO_APP_FORMAT).arg(appName, appVersion, targetVersion, fileHash, rootPrivateKey);
    d->info[type] = info;
    return info;
}
