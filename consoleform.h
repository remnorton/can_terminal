#ifndef CONSOLEFORM_H
#define CONSOLEFORM_H

#include <QStandardItemModel>
#include "ui_consoleform.h"
#include "proto.h"

class ConsoleForm : public QWidget, private Ui::ConsoleForm
{
    Q_OBJECT

public:
    explicit ConsoleForm(CAN_USB_Filter_t f, QWidget *parent = nullptr);
    quint32 id();
    void logMessage(CAN_USB_Mess_t* mess, bool rx);
    CAN_USB_Filter_t getFilter();

protected:
    void closeEvent(QCloseEvent *event) override;

signals:
    void closed();

private slots:
    void on_btInfo_clicked();

    void on_toolButton_clicked();

private:
    CAN_USB_Filter_t filter;
    QStandardItemModel model;

};

#endif // CONSOLEFORM_H
