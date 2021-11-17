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
#include <QTextStream>

#include <atomic>

static const QString LOAD_FORMAT = ":/ledger/%1_load";
static const QString DELETE_FORMAT = ":/ledger/%1_delete";
static const QString ID_FORMAT = ":/ledger/%1_app_id";
static const QString RC_PATH_FORMAT = ":/ledger";
static const char* UPDATE_FIRMWARE_FORMAT = "Firmware version: %1.\nTarget version: %2.\n\nPlease update the ledger firmware to the most recent version.";

static const QString DEPENDENCY_INSTALL_CMD = "python -m pip install --user ledgerblue";
static const QString DEPENDENCY_SHOW_CMD = "python -m pip show ledgerblue";

bool fMainnet = true;

bool isMainnet()
{
    return fMainnet;
}

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
    QMap<InstallDevice::DeviceType, LedgerAppInfo>& ledgerAppInfo()
    {
        return isMainnet() ? mainnetLedgerAppInfo : testnetLedgerAppInfo;
    }
    QString publicKeyP1;
    QString publicKeyP2;
    QString rootPrivateKey;

private:
    QMap<InstallDevice::DeviceType, LedgerAppInfo> mainnetLedgerAppInfo;
    QMap<InstallDevice::DeviceType, LedgerAppInfo> testnetLedgerAppInfo;
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


bool InstallDevice::firmwareCommand(QString &program, QStringList &arguments)
{
    QString command = "python -m ledgerblue.checkGenuine --targetId 0x31100004";
    return getCommand(command, program, arguments);
}

bool InstallDevice::genKeyPairCommand(QString &program, QStringList &arguments)
{
    QString command = "python -m ledgerblue.genCAPair";
    return getCommand(command, program, arguments);
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

    return getCommand(command, program, arguments);
}

bool InstallDevice::getCommand(const QString &command, QString &program, QStringList &arguments)
{
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
    getProgram(program);
    if(!d->rootPrivateKey.isEmpty())
    {
        arguments.push_back("--rootPrivateKey");
        arguments.push_back(d->rootPrivateKey);
    }
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
    getProgram(program);
    if(!d->rootPrivateKey.isEmpty())
    {
        arguments.push_back("--rootPrivateKey");
        arguments.push_back(d->rootPrivateKey);
    }
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

bool QtumLedgerTool::checkFirmware(InstallDevice::DeviceType type, QString& message)
{
    // Check data dir
    if(!checkDataDir())
        return false;

    // Get firmware for ledger
    InstallDevice device(type);
    QString program;
    QStringList arguments;
    bool ret = device.firmwareCommand(program, arguments);
    getProgram(program);
    if(ret)
    {
        d->process.start(program, arguments);
        d->fStarted = true;

        wait();

        ret &= QProcess::NormalExit == d->process.exitStatus();
        ret &= d->strError.isEmpty();

        message = firmwareMessage(type);
    }

    return ret;
}

bool QtumLedgerTool::getKeyPair()
{
    // Check data dir
    if(!checkDataDir())
        return false;

    // Get key pair
    InstallDevice device;
    QString program;
    QStringList arguments;
    bool ret = device.genKeyPairCommand(program, arguments);
    getProgram(program);
    if(ret)
    {
        d->process.start(program, arguments);
        d->fStarted = true;

        wait();

        ret &= QProcess::NormalExit == d->process.exitStatus();
        ret &= d->strStdout.contains("Public key");
        ret &= d->strStdout.contains("Private key");

        if(ret)
        {
            QString lines = d->strStdout;
            QStringList elements = lines.replace("\n", " ").split(" ");
            for(QString element : elements)
            {
                element = element.trimmed().toUpper();
                if(element.size() == 64)
                {
                    d->rootPrivateKey = element;
                }
                if(element.size() == 130)
                {
                    d->publicKeyP1 = element.left(65);
                    d->publicKeyP2 = element.right(65);
                }
            }

            ret &= !d->rootPrivateKey.isEmpty();
            ret &= !d->publicKeyP1.isEmpty();
            ret &= !d->publicKeyP1.isEmpty();
        }
    }

    return ret;
}

QString QtumLedgerTool::firmwareMessage(InstallDevice::DeviceType type)
{
    // Get app target
    QString ledgerFirmware;
    LedgerAppInfo info = appInfo(type);
    QString appTarget = info.targetVersion;

    // Parse firmware version
    QString line;
    QTextStream stream(&d->strStdout);
    while (stream.readLineInto(&line))
    {
        if(line.startsWith("SE Version"))
        {
            QStringList elements = line.split(" ");
            if(elements.size() > 2)
            {
                ledgerFirmware = elements[2].trimmed();
                break;
            }
        }
    }

    if(ledgerFirmware.isEmpty())
    {
        return tr("Fail to get ledger firmware");
    }

    if(appTarget.isEmpty())
    {
        return tr("Fail to get app target");
    }

    QList<int> firmwareVersion;
    QStringList elementsFirmware = ledgerFirmware.split(".");
    for(QString element : elementsFirmware)
    {
        bool ok = true;
        int version = element.toInt(&ok);
        if(ok)
        {
            firmwareVersion.push_back(version);
        }
        else
        {
            return tr("Fail to parse firmware version: ") + ledgerFirmware;
        }
    }

    QList<int> targetVersion;
    QStringList elementsTarget = appTarget.split(".");
    for(QString element : elementsTarget)
    {
        bool ok = true;
        int version = element.toInt(&ok);
        if(ok)
        {
            targetVersion.push_back(version);
        }
        else
        {
            return tr("Fail to parse target version: ") + appTarget;
        }
    }

    if(firmwareVersion.size() < 2)
    {
        return tr("Firmware version not complete: ") + ledgerFirmware;
    }

    if(targetVersion.size() < 2)
    {
        return tr("Target version not complete: ") + appTarget;
    }

    // Check firmware version
    bool needUpdate = false;
    if(firmwareVersion[0] < targetVersion[0])
    {
        needUpdate = true;
    }

    if(firmwareVersion[0] == targetVersion[0] &&
            firmwareVersion[1] < targetVersion[1])
    {
        needUpdate = true;
    }

    if(needUpdate)
    {
        return tr(UPDATE_FIRMWARE_FORMAT).arg(ledgerFirmware, appTarget);
    }

    return "";
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
    getProgram(program);
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
        getProgram(program);
        d->process.start(program, arguments);
        d->fStarted = true;
        wait();
        ret &= QProcess::NormalExit == d->process.exitStatus();
        ret &= d->strStdout.contains("ledgerblue");
    }

    return ret;
}

LedgerAppInfo QtumLedgerTool::appInfo(InstallDevice::DeviceType type)
{
    // Check the list
    if(d->ledgerAppInfo().contains(type))
    {
        return d->ledgerAppInfo()[type];
    }

    // Parameters list
    LedgerAppInfo info;

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
                    info.appName = arguments[i];
                }
            }
            else if(argument == "--appVersion")
            {
                i++;
                if(i < arguments.size())
                {
                    info.appVersion = arguments[i];
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
            else if(argument.startsWith("--targetVersion"))
            {
                QStringList target = argument.split("=");
                if(target.size() > 1)
                {
                    info.targetVersion = target[1];
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
            info.fileHash = QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex().toUpper();
        }
    }

    // Get the app identifer
    QString identifierRC = ID_FORMAT.arg(InstallDevice::deviceToString(type));
    if(QFile::exists(identifierRC))
    {
        QFile fileIn(identifierRC);
        if(fileIn.open(QIODevice::ReadOnly))
        {
            info.appIdentifier = QString(fileIn.readAll()).trimmed().toUpper();
        }
    }

    // Add the info into the list
    d->ledgerAppInfo()[type] = info;
    return info;
}

void QtumLedgerTool::getPubKey(QString &publicKeyP1, QString &publicKeyP2)
{
    publicKeyP1 = d->publicKeyP1;
    publicKeyP2 = d->publicKeyP2;
}

bool QtumLedgerTool::getPythonExec(QString &execName)
{
    execName = "python3";
    int execVersion = -1;
    if(getPythonVersion(execName, execVersion) && execVersion > 2)
        return true;

    execName = "python";
    if(getPythonVersion(execName, execVersion) && execVersion > 2)
        return true;

    if(d->strError.isEmpty())
    {
        d->strError = tr("Python 3 or higher is not installed.");
    }

    return false;
}

bool QtumLedgerTool::getPythonVersion(const QString &execName, int &execVersion)
{
    execVersion = -1;
    QString program = execName;
    QStringList arguments;
    arguments << "--version";

    d->process.start(program, arguments);
    d->fStarted = true;
    wait();

    bool ret = QProcess::NormalExit == d->process.exitStatus();
    QString line = d->strStdout.trimmed();
    line.replace("\n", "");
    ret &= line.startsWith("Python", Qt::CaseInsensitive);
    if(!ret) return false;

    QStringList elements = line.split(" ");
    ret = elements.size() == 2;
    if(!ret) return false;

    QStringList pythonElements = elements[1].split(".");
    QList<int> pythonVersion;
    for(QString element : pythonElements)
    {
        bool ok = true;
        int version = element.toInt(&ok);
        if(ok)
        {
            pythonVersion.push_back(version);
        }
        else
        {
            return false;
        }
    }

    ret = pythonVersion.size() > 0;
    if(ret)
    {
        execVersion = pythonVersion[0];
    }

    return ret;
}

bool QtumLedgerTool::getProgram(QString &program)
{
    if(program.startsWith("python"))
    {
        QString execName;
        if(getPythonExec(execName))
        {
            program = execName;
        }
        else
        {
            program = "python3";
            return false;
        }
    }

    return true;
}

QString QtumLedgerTool::dependencyCommand()
{
    QString command = DEPENDENCY_INSTALL_CMD;
    QString execName;
    if(getPythonExec(execName))
        command = command.replace("python", execName);
    return command;
}
