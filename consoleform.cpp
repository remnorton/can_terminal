#include <QDebug>
#include <QDateTime>
#include "com_func.h"
#include "consoleform.h"
#include "filterdialog.h"

#define _LOG_CCOUNT_    7
static const QString headerstr[_LOG_CCOUNT_] = {"Dir.", "Time","ID", "Type", "RTR", "DLC", "Data"};
static const uint16_t sizes[_LOG_CCOUNT_] = {20, 160, 80, 50, 30, 20, 0};


ConsoleForm::ConsoleForm(CAN_USB_Filter_t f, QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);
    filter = f;

    model.setColumnCount(_LOG_CCOUNT_);
    tvLog->setModel(&model);

    for (int i = 0; i < _LOG_CCOUNT_; i++)
    {
        model.setHorizontalHeaderItem(i, new QStandardItem(headerstr[i]));
        if (sizes[i]) tvLog->setColumnWidth(i, sizes[i]);
        else tvLog->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
    }
    setWindowTitle(QString("CAN console #%1").arg(filter.FilterBank+1));
    setAttribute(Qt::WA_DeleteOnClose);
    tvLog->horizontalHeader()->setFont(font());
    tvLog->setFont(font());
}

quint32  ConsoleForm::id()
{
    return filter.FilterBank;
}

void ConsoleForm::logMessage(CAN_USB_Mess_t* mess, bool rx)
{
    int row = model.rowCount();
    if (rx || cbTxMess->isChecked())
    {
        model.setRowCount(row+1);
        model.setItem(row, 0, new QStandardItem(rx?"Rx":"Tx")); //direction
        model.setItem(row, 1, new QStandardItem(QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss"))); //Time

        model.setItem(row, 2, new QStandardItem("0x"+QString("%1").arg(mess->id, mess->flags.ide?6:3, 16, QChar('0')))); //id
        model.setItem(row, 3, new QStandardItem(mess->flags.ide?"EXT":"STD")); //type
        model.setItem(row, 4, new QStandardItem(mess->flags.rtr?"🗸":"")); //rtr
        model.setItem(row, 5, new QStandardItem(QVariant(mess->flags.dlc).toString())); //dlc
        model.setItem(row, 6, new QStandardItem(toHexString(QByteArray((char*)mess->data, mess->flags.dlc)))); //data

        QColor textcolor = QColor(rx?"#00aa00":"#ffaa00");
        for (int i = 0; i < model.columnCount(); i++)
            model.item(row, i)->setData(textcolor, Qt::TextColorRole);

        tvLog->scrollToBottom();
    }
}

CAN_USB_Filter_t ConsoleForm::getFilter()
{
    return filter;
}

void ConsoleForm::closeEvent(QCloseEvent *event)
{
    QWidget::closeEvent(event);
    emit closed();
}

void ConsoleForm::on_btInfo_clicked()
{
    FilterDialog dlg(this);
    dlg.setData(&filter);
    dlg.setReadOnly();
    dlg.exec();
}

void ConsoleForm::on_toolButton_clicked()
{
    model.removeRows(0, model.rowCount());
}
