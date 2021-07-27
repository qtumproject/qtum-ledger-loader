#include <qt/qtumledgertool.h>

#include <QProcess>
#include <QFile>
#include <QStringList>
#include <QByteArray>
#include <QApplication>

#include <atomic>

static const QString LOAD_FORMAT = ":/ledger/%1_load";
static const QString DELETE_FORMAT = ":/ledger/%1_delete";
static const QString RC_PATH_FORMAT = ":/ledger";

class QtumLedgerToolPriv
{
public:
    QtumLedgerToolPriv(QObject *parent)
    {}

    std::atomic<bool> fStarted{false};
    QProcess process;
    QString strStdout;
    QString strError;
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
        QString dataDir = QString::fromStdString(GetDataDir().string());
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
