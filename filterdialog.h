#ifndef FILTERDIALOG_H
#define FILTERDIALOG_H

#include "ui_filterdialog.h"
#include "proto.h"

class FilterDialog : public QDialog, private Ui::FilterDialog
{
    Q_OBJECT

public:
    explicit FilterDialog(QWidget *parent = nullptr);
    void setData(CAN_USB_Filter_t* fdata);
    void getData(CAN_USB_Filter_t* fdata);
    void setReadOnly(bool ro = true);

private slots:
    void onDataChanged();
};

#endif // FILTERDIALOG_H
