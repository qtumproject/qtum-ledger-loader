#ifndef QTUMLEDGERINSTALLERDIALOG_H
#define QTUMLEDGERINSTALLERDIALOG_H

#include <QMainWindow>
#include <qt/qtumledgertool.h>

class QtumLedgerInstallerDialogPriv;
class QAction;
class QMenuBar;

namespace Ui {
class QtumLedgerInstallerDialog;
}

class QtumLedgerInstallerDialog : public QMainWindow
{
    Q_OBJECT

public:
    explicit QtumLedgerInstallerDialog(QWidget *parent = nullptr);
    ~QtumLedgerInstallerDialog();

public Q_SLOTS:
    void showHelpMessageClicked();
    void aboutClicked();

private Q_SLOTS:
    void on_addButton_clicked();
    void on_removeButton_clicked();
    void on_updateCheckBox_clicked(bool checked);
    void on_cbLedgerApp_currentIndexChanged(int index);
    void checkForUpdates();

protected:
    InstallDevice::DeviceType getDeviceType();
    QString getDeviceAppTitle(bool install);
    bool parseErrorMessage(QString& message, bool& dependency);
    void installDependency();
    QString installInfo(InstallDevice::DeviceType type);
    QString uninstallInfo(InstallDevice::DeviceType type);
    QString appInfo(InstallDevice::DeviceType type);
    QString firmwareInfo();
    bool installApp();
    bool checkFirmware(QString& message);

private:
    void createActions();
    void createMenuBar();

private:
    QMenuBar* appMenuBar = nullptr;

    QAction* showHelpMessageAction = nullptr;
    QAction* aboutAction = nullptr;
    QAction* aboutQtAction = nullptr;

    Ui::QtumLedgerInstallerDialog *ui;
    QtumLedgerInstallerDialogPriv *d;
};

#endif // QTUMLEDGERINSTALLERDIALOG_H
