#include <string.h>
#include "senddialog.h"

SendDialog::SendDialog(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);
    databox.append(sbData0);
    databox.append(sbData1);
    databox.append(sbData2);
    databox.append(sbData3);
    databox.append(sbData4);
    databox.append(sbData5);
    databox.append(sbData6);
    databox.append(sbData7);
}

void SendDialog::setData(CAN_USB_Mess_t* mess)
{
    sbID->setValue(mess->id);
    cbExt->setChecked(mess->flags.ide);
    cbRtr->setChecked(mess->flags.rtr);
    sbDlc->setValue(mess->flags.dlc);
    for(int i = 0; i < 8; i++)
        databox.at(i)->setValue((i < mess->flags.dlc)?mess->data[i]:0);
}

void SendDialog::getData(CAN_USB_Mess_t* mess)
{
    memset(mess, 0, sizeof(CAN_USB_Mess_t));
    mess->id = sbID->value();
    mess->flags.ide = cbExt->isChecked()?1:0;
    mess->flags.rtr = cbRtr->isChecked()?1:0;
    mess->flags.dlc = sbDlc->value();
    for (int i = 0; i < mess->flags.dlc; i++)
        mess->data[i] = databox.at(i)->value();
}

void SendDialog::on_sbDlc_valueChanged(int arg1)
{
    for(int i = 0; i < 8; i++)
        databox.at(i)->setEnabled(i < sbDlc->value());
}
