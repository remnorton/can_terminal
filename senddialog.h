#ifndef SENDDIALOG_H
#define SENDDIALOG_H

#include <QList>
#include "proto.h"
#include "ui_senddialog.h"

class SendDialog : public QDialog, private Ui::SendDialog
{
    Q_OBJECT

public:
    explicit SendDialog(QWidget *parent = nullptr);
    void setData(CAN_USB_Mess_t* mess);
    void getData(CAN_USB_Mess_t* mess);

private slots:
    void on_sbDlc_valueChanged(int arg1);

private:
    QList<QSpinBox*> databox;
};

#endif // SENDDIALOG_H
