#ifndef QTUMLEDGERINSTALLERDIALOG_H
#define QTUMLEDGERINSTALLERDIALOG_H

#include <QDialog>
#include <qt/qtumledgertool.h>

class QtumLedgerInstallerDialogPriv;

class QtumLedgerInstallerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QtumLedgerInstallerDialog(QWidget *parent = nullptr);
    ~QtumLedgerInstallerDialog();

private Q_SLOTS:
    void on_addButton_clicked();
    void on_removeButton_clicked();
    void on_cancelButton_clicked();
    void on_cbLedgerApp_currentIndexChanged(int index);

protected:
    InstallDevice::DeviceType getDeviceType();
    bool parseErrorMessage(QString& message);

private:
    QtumLedgerInstallerDialogPriv *d;
};

#endif // QTUMLEDGERINSTALLERDIALOG_H
