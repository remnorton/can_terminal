#include <QItemSelectionModel>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QFileDialog>
#include <QDateTime>
#include <QTimer>
#include <QEventLoop>
#include <QSerialPortInfo>
#include <QDebug>
#include <QHeaderView>
#include <QMessageBox>
#include "com_func.h"
#include "proto.h"
#include "mainwindow.h"
#include "filterdialog.h"
#include "senddialog.h"
#include "fileinfo.h"

#define _MAX_FILTERS_   14
#define _TXL_CCOUNT_    7

static const QString headerstr[_TXL_CCOUNT_] = {"Auto", "Send", "ID", "Type", "RTR", "DLC", "Data"};
static const uint16_t sizes[_TXL_CCOUNT_] = {24, 24, 80, 50, 30, 20, 0};

static QString title = "USB-CAN terminal v. ";
static const QString baudstr[CAN_BAUD_END] = {"" , "10", "20", "25", "50", "100", "125", "200", "250", "400", "500", "800", "1000"};
static ConsoleForm* console_forms[_MAX_FILTERS_];

//
//
//

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi(this);
    setWindowTitle(title+stringVersion(fileVersion(qApp->applicationFilePath())));
    memset(&filter, 0, sizeof(filter));

    connect(&port, &QSerialPort::readyRead, this, &MainWindow::onRead);

    for (int i = CAN_BAUD_NONE+1; i < CAN_BAUD_END; i++)
        cbBaud->addItem(baudstr[i], i);
    cbBaud->setCurrentIndex(-1);

    memset(&txmess, 0, sizeof(txmess));

    btRefresh->click();
    memset(console_forms, 0, sizeof(console_forms));

    txModel.setColumnCount(_TXL_CCOUNT_);
    tvSendList->setModel(&txModel);
    tvSendList->horizontalHeader()->setFont(this->font());
    tvSendList->setFont(font());

    for (int i = 0; i < _TXL_CCOUNT_; i++)
    {
        txModel.setHorizontalHeaderItem(i, new QStandardItem(headerstr[i]));
        if (sizes[i]) tvSendList->setColumnWidth(i, sizes[i]);
        else tvSendList->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
    }

    connect(&transmitter, &QTimer::timeout, this, &MainWindow::on_btSendAll_clicked);
    connect(tvSendList->selectionModel(), &QItemSelectionModel::currentChanged, this, &MainWindow::on_TxMessageSelectionChanged);

}

MainWindow::~MainWindow()
{
    if (port.isOpen()) port.close();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    for (int i = 0; i < _MAX_FILTERS_; i++)
    {
        ConsoleForm* cf = console_forms[i];
        if (cf)
        {
            filter = cf->getFilter();
            filter.FilterActivation = 0;
            setFilter();
            cf->blockSignals(true);
            cf->close();
        }
    }
    delay_ms(10);
    QMainWindow::closeEvent(event);
}

void MainWindow::sendRequest(uint8_t type)
{
    if (!port.isOpen()) return;
    QByteArray b;
    b.resize(sizeof(CAN_USB_Header_t));
    make_usb_can_pck(type, 0, 0, (uint8_t*)b.data());
    port.write(b);
}

void MainWindow::onParse()
{
    uint16_t offs = 0;
    while((((uint8_t)rxBuf.at(offs)) != _PREFIX_) && (offs < rxBuf.count())) offs++;

    if (offs)
        rxBuf = rxBuf.remove(0, offs);

    if (rxBuf.count() < (sizeof(CAN_USB_Header_t))) return;

    CAN_USB_Header_t* hdr = (CAN_USB_Header_t*)rxBuf.data();

    bool has_pck = (rxBuf.count() >= (sizeof(CAN_USB_Header_t) + hdr->datalen));

    while (has_pck)
    {
        offs = 0;

        uint8_t* payload = (uint8_t*)(rxBuf.data()+sizeof(CAN_USB_Header_t));
        switch (hdr->type)
        {
            case CAN_PT_MESS:
            {
                CAN_USB_Mess_t* p = (CAN_USB_Mess_t*)payload;
                logMessage(p);
                break;
            }
            case  CAN_PT_BAUD:
            {
                uint8_t baud = *payload;
                btStart->setChecked(baud > 0);
                cbBaud->setCurrentIndex(cbBaud->findData(baud));
                //qDebug() << "CAN_PT_BAUD" << baud;
                break;
            }
            case CAN_PT_FILTER:
            {
                qDebug() << "CAN_PT_FILTER";
                CAN_USB_Filter_t* pl = (CAN_USB_Filter_t*)payload;
                filter.FilterActivation = pl->FilterActivation;
                filter.FilterBank = pl->FilterBank;
                filter.FilterFIFOAssignment = pl->FilterFIFOAssignment;
                filter.FilterIdHigh = pl->FilterIdHigh;
                filter.FilterIdLow = pl->FilterIdLow;
                filter.FilterMaskIdHigh = pl->FilterMaskIdHigh;
                filter.FilterMaskIdLow = pl->FilterMaskIdLow;
                filter.FilterMode = pl->FilterMode;
                filter.FilterScale = pl->FilterScale;
                filter.SlaveStartFilterBank = pl->SlaveStartFilterBank;
                break;
            }
        }
        offs = sizeof(CAN_USB_Header_t) + hdr->datalen;

        while((((uint8_t)rxBuf.at(offs)) != _PREFIX_) && (offs < rxBuf.count())) offs++;

        if (offs)
            rxBuf = rxBuf.remove(0, offs);

        has_pck = (rxBuf.count() >= (sizeof(CAN_USB_Header_t) + hdr->datalen));
    }
}

void MainWindow::setFilter()
{
    if (!port.isOpen()) return;
    QByteArray b;
    b.resize(sizeof(CAN_USB_Header_t) + sizeof(filter));
    make_usb_can_pck(CAN_PT_FILTER, &filter, sizeof(filter), (uint8_t*)b.data());
    port.write(b);
}

void MainWindow::canStart(uint8_t brate)
{
    if (!port.isOpen()) return;
    QByteArray b;
    b.resize(sizeof(CAN_USB_Header_t) + sizeof(brate));
    make_usb_can_pck(CAN_PT_BAUD, &brate, sizeof(brate), (uint8_t*)b.data());
    port.write(b);
}

void MainWindow::logMessage(CAN_USB_Mess_t* mess, bool rx)
{
    if (rx)
    {
        ConsoleForm* con = getConsole(mess->filter);
        if (con) con->logMessage(mess, rx);
    }
    else
    {
        for (int i = 0; i < _MAX_FILTERS_; i++)
        {
            ConsoleForm* con = console_forms[i];
            if (con) con->logMessage(mess, rx);
        }
    }
}

void MainWindow::addTxMessage(CAN_USB_Mess_t& mess)
{
    int row = txModel.rowCount();
    txModel.setRowCount(row+1);
    for (int i = 0; i < _TXL_CCOUNT_; i++) txModel.setItem(row, i, new QStandardItem);
    setTxRowData(row, &mess);
    updateControlState();
}

void MainWindow::sendData(QByteArray dt, bool send)
{
    if (port.error() == QSerialPort::NoError)
    {
        logMessage((CAN_USB_Mess_t*)dt.data(), false);

        QByteArray b;
        b.resize(sizeof(CAN_USB_Header_t)+sizeof(CAN_USB_Mess_t));
        make_usb_can_pck(CAN_PT_MESS, dt.data(), sizeof(CAN_USB_Mess_t), (uint8_t*)b.data());
        txBuf.append(b);

        if (send)
        {
            port.write(txBuf);
            port.flush();
            txBuf.clear();
        }
    }
    else
    {
        btOpen->setChecked(false);
        QMessageBox::critical(this, "Error", "Communication broken. Reason: "+port.errorString());
    }
}

void MainWindow::setTxRowData(int row, CAN_USB_Mess_t* mess)
{
    QByteArray b;
    b.resize(sizeof(CAN_USB_Mess_t));
    memcpy(b.data(), mess, sizeof(CAN_USB_Mess_t));

    txModel.item(row, 0)->setText("🗸");
    txModel.item(row, 0)->setData(true);

    txModel.item(row, 1)->setData(b);
    txModel.item(row, 1)->setIcon(QIcon(":/Icons/SEND"));

    txModel.item(row, 2)->setText("0x"+QString("%1").arg(mess->id, mess->flags.ide?6:3, 16, QChar('0'))); //id
    txModel.item(row, 3)->setText(mess->flags.ide?"EXT":"STD"); //type
    txModel.item(row, 4)->setText(mess->flags.rtr?"🗸":""); //rtr
    txModel.item(row, 5)->setText(QVariant(mess->flags.dlc).toString()); //dlc
    txModel.item(row, 6)->setText(toHexString(QByteArray((char*)mess->data, mess->flags.dlc))); //data
}

ConsoleForm* MainWindow::getConsole(uint32_t filter)
{
    if (filter > _MAX_FILTERS_) return 0;
    return console_forms[filter];
}

void MainWindow::updateControlState()
{
    btSendAll->setDisabled(txModel.rowCount() < 1);
    btSave->setDisabled(txModel.rowCount() < 1);
    sbInterval->setDisabled(txModel.rowCount() < 1);
    cbAutoSend->setDisabled(txModel.rowCount() < 1);
}

void MainWindow::onConsoleClosed()
{
    ConsoleForm* cf = qobject_cast<ConsoleForm*>(sender());
    if (!cf) return;
    quint32 id = cf->id();
    if (id > _MAX_FILTERS_) return;
    filter = cf->getFilter();
    filter.FilterActivation = 0;
    setFilter();
    console_forms[id] = 0;
}

void MainWindow::on_btRefresh_clicked()
{
    cbPort->clear();
    foreach (QSerialPortInfo i, QSerialPortInfo::availablePorts())
    {
        cbPort->addItem(QString("%1 (%2)").arg(i.portName()).arg(i.description()), i.portName());
    }
}

void MainWindow::on_TxMessageSelectionChanged()
{
    int row = tvSendList->currentIndex().row();
    int col = tvSendList->currentIndex().column();

    btDelTxMessage->setDisabled((col < 0) || (row < 0));
}

void MainWindow::on_cbPort_currentIndexChanged(int index)
{
    btOpen->setDisabled(index < 0);
}

void MainWindow::onRead()
{
    QByteArray bt = port.readAll();
    rxBuf.append(bt);
    onParse();
}

void MainWindow::on_btOpen_toggled(bool checked)
{
    if (checked) //try to connect
    {
        port.setPort(QSerialPortInfo(cbPort->currentData().toString()));
        port.setBaudRate(921600);
        port.setParity(QSerialPort::NoParity);
        port.setDataBits(QSerialPort::Data8);
        port.setStopBits(QSerialPort::OneStop);
        port.setFlowControl(QSerialPort::NoFlowControl);

        port.open(QSerialPort::ReadWrite);
        if (!port.isOpen())
        {
            btOpen->setChecked(false);
            QMessageBox::critical(this, "Error", "Can not open port: "+port.errorString());
        }
        else
        {
            sendRequest(CAN_PT_BAUD);
            btOpen->setText("Close");
        }
    }
    else //close port
    {
        if (port.isOpen())
        {
            canStart(0);
            btOpen->setText("Open");
            btStart->setChecked(false);
            delay_ms(50);
            port.close();
        }
    }
}

void MainWindow::on_btStart_toggled(bool checked)
{
    btStart->setText(checked?"Stop":"Start");
}

void MainWindow::on_cbBaud_currentIndexChanged(int index)
{
    btStart->setDisabled(index < 0);
}

void MainWindow::on_btNewConsole_clicked()
{
    //find free slot
    int idx = -1;
    for (int i = 0; (i < _MAX_FILTERS_) && (idx < 0); i++)
        if (!console_forms[i]) idx = i;
    if (idx < 0) return;

    FilterDialog dlg;
    dlg.setData(&filter);
    if (dlg.exec() == QDialog::Accepted)
    {
        dlg.getData(&filter);
        filter.FilterBank = idx;
        setFilter();
        console_forms[idx] = new ConsoleForm(filter);
        console_forms[idx]->show();
        connect(console_forms[idx], &ConsoleForm::closed, this, &MainWindow::onConsoleClosed);
    }
}

void MainWindow::on_tvSendList_clicked(const QModelIndex &index)
{
    int row = index.row();
    if (row < 0) return;

    QStandardItem* item = txModel.item(row, index.column());

    switch(index.column())
    {
        case 0: //enable/disable
        {

            bool en = !item->data().toBool();
            item->setText(en?"🗸":"");
            item->setData(en);
            break;
        }
        case 1:
        {
            //if (txModel.item(row)->data().toBool())
            sendData(item->data().toByteArray());
            break;
        }
    }

}

void MainWindow::on_btAddTxMess_clicked()
{
    SendDialog dlg;
    dlg.setData(&txmess);
    if (dlg.exec() == QDialog::Accepted)
    {
        dlg.getData(&txmess);
        addTxMessage(txmess);
    }
}

void MainWindow::on_btSendAll_clicked()
{
    if (!btSendAll->isEnabled()) return;
    for(int i = 0; i < txModel.rowCount(); i++)
    {
        if (txModel.item(i)->data().toBool())
            sendData(txModel.item(i, 1)->data().toByteArray(), (i+1) == txModel.rowCount());
    }
}

void MainWindow::on_tvSendList_doubleClicked(const QModelIndex &index)
{
    int row = index.row();
    if (row < 0) return;
    if (index.column() < 2) return;

    SendDialog dlg;
    QByteArray b = txModel.item(row, 1)->data().toByteArray();
    dlg.setData((CAN_USB_Mess_t*)b.data());
    if (dlg.exec() == QDialog::Accepted)
    {
        dlg.getData((CAN_USB_Mess_t*)b.data());
        setTxRowData(row, (CAN_USB_Mess_t*)b.data());
    }

}

void MainWindow::on_btStart_clicked(bool checked)
{
    canStart(checked?cbBaud->currentData().toUInt():0);
}

void MainWindow::on_sbInterval_valueChanged(int arg1)
{
    if (arg1)
        transmitter.setInterval(arg1);
}

void MainWindow::on_btSave_clicked()
{
    QString fName = QFileDialog::getSaveFileName(this, "Save", "", "CAN message files (*.cmf)");
    if (fName.isEmpty()) return;
    if (QFileInfo(fName).suffix().toLower() != "cmf")
        fName += ".cmf";

    QVariantList root;
    for(int i = 0; i < txModel.rowCount(); i++)
    {
        QByteArray b = txModel.item(i,1)->data().toByteArray();
        CAN_USB_Mess_t* mess = (CAN_USB_Mess_t*)b.data();

        QVariantList mdata;
        for (int c = 0; c < mess->flags.dlc; c++)
            mdata.append(mess->data[c]);

        QVariantMap mmap;
        mmap.insert("id", mess->id);
        mmap.insert("dlc", mess->flags.dlc);
        mmap.insert("ide",mess->flags.ide);
        mmap.insert("rtr",mess->flags.rtr);
        mmap.insert("data",mdata);

        root.append(mmap);
    }

    QJsonDocument doc(QJsonArray::fromVariantList(root));
    QFile f(fName);
    if (f.open(QFile::WriteOnly))
        f.write(doc.toJson(QJsonDocument::Compact));
    f.close();
}

void MainWindow::on_btLoad_clicked()
{
    //clear model
    while(txModel.rowCount()) txModel.removeRow(0);
    //load new data
    QString fName = QFileDialog::getOpenFileName(this, "Open", "", "CAN message files (*.cmf)");
    if (fName.isEmpty()) return;

    QByteArray d;
    QFile f(fName);
    if (f.open(QFile::ReadOnly))
        d = f.readAll();
    f.close();

    QJsonParseError jpe;
    QJsonDocument doc = QJsonDocument::fromJson(d, &jpe);
    if (jpe.error != QJsonParseError::NoError) return;

    QVariant jvar = doc.toVariant();
    if (jvar.type() != QVariant::List) return;

    QVariantList vl = jvar.toList();
    foreach(QVariant vv, vl)
    {
        if (vv.type() == QVariant::Map)
        {
            CAN_USB_Mess_t mess;
            QVariantMap vm = vv.toMap();
            memset(&mess, 0, sizeof(mess));
            mess.id = vm.value("id").toUInt();
            mess.flags.dlc = vm.value("dlc").toUInt();
            mess.flags.ide = vm.value("ide").toUInt();
            mess.flags.rtr = vm.value("rtr").toUInt();

            if (vm.value("data").type() == QVariant::List)
            {
                QVariantList vd = vm.value("data").toList();
                for (int i = 0; i < vd.count(); i++)
                    mess.data[i] = vd.at(i).toUInt();
            }
            addTxMessage(mess);
        }
    }
}

void MainWindow::on_cbAutoSend_toggled(bool checked)
{
    if (checked)
    {
        transmitter.start(sbInterval->value());
        on_btSendAll_clicked();
    }
    else transmitter.stop();
}

void MainWindow::on_btDelTxMessage_clicked()
{
    int row = tvSendList->currentIndex().row();
    if (row < 0) return;
    txModel.removeRow(row);
}
