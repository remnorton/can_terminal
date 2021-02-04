#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
#include <QMainWindow>
#include <QSerialPort>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QTimer>
#include "consoleform.h"
#include "proto.h"

class MainWindow : public QMainWindow, private  Ui::MainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void sendRequest(uint8_t type);
    void onParse();
    void setFilter();
    void canStart(uint8_t brate);
    void logMessage(CAN_USB_Mess_t* mess, bool rx = true);
    void addTxMessage(CAN_USB_Mess_t& mess);
    void sendData(QByteArray dt, bool send = true);
    void setTxRowData(int row, CAN_USB_Mess_t* mess);
    ConsoleForm* getConsole(uint32_t filter);
    void updateControlState();

private slots:
    void onConsoleClosed();
    void on_btRefresh_clicked();

    void on_TxMessageSelectionChanged();

    void on_cbPort_currentIndexChanged(int index);

    void onRead();

    void on_btOpen_toggled(bool checked);

    void on_btStart_toggled(bool checked);

    void on_cbBaud_currentIndexChanged(int index);

    void on_btNewConsole_clicked();

    void on_tvSendList_clicked(const QModelIndex &index);

    void on_btAddTxMess_clicked();

    void on_btSendAll_clicked();

    void on_tvSendList_doubleClicked(const QModelIndex &index);

    void on_btStart_clicked(bool checked);

    void on_sbInterval_valueChanged(int arg1);

    void on_btSave_clicked();

    void on_btLoad_clicked();

    void on_cbAutoSend_toggled(bool checked);

    void on_btDelTxMessage_clicked();


private:
    QSerialPort port;
    QByteArray rxBuf;
    QByteArray txBuf;
    CAN_USB_Filter_t filter;
    CAN_USB_Mess_t txmess;
    QStandardItemModel txModel;
    QTimer transmitter;
};
#endif // MAINWINDOW_H
