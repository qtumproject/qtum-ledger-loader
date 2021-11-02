#ifndef QTUMLEDGERINSTALLERDIALOG_H
#define QTUMLEDGERINSTALLERDIALOG_H

#include <QDialog>
#include <qt/qtumledgertool.h>

class QtumLedgerInstallerDialogPriv;

namespace Ui {
class QtumLedgerInstallerDialog;
}

class QtumLedgerInstallerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QtumLedgerInstallerDialog(QWidget *parent = nullptr);
    ~QtumLedgerInstallerDialog();

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

private:
    Ui::QtumLedgerInstallerDialog *ui;
    QtumLedgerInstallerDialogPriv *d;
};

#endif // QTUMLEDGERINSTALLERDIALOG_H
