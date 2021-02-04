#include <QPushButton>
#include <QDebug>
#include "filterdialog.h"

FilterDialog::FilterDialog(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);
}

void FilterDialog::setData(CAN_USB_Filter_t* fdata)
{
    rbFull->setChecked(fdata->FilterScale != 0);
    rbList->setChecked(fdata->FilterMode != 0);

    sbMinID->setValue(fdata->FilterIdLow);
    sbMaxID->setValue(fdata->FilterIdHigh);

    sbMinMask->setValue(fdata->FilterMaskIdLow);
    sbMaxMask->setValue(fdata->FilterMaskIdHigh);

}

void FilterDialog::getData(CAN_USB_Filter_t* fdata)
{
    memset(fdata, 0, sizeof(CAN_USB_Filter_t));
    fdata->FilterBank = 0;
    fdata->FilterScale = rbFull->isChecked()?1:0; //CAN_FILTERSCALE_16BIT
    fdata->FilterActivation = 1;
    fdata->FilterFIFOAssignment = 0; //CAN_FILTER_FIFO0
    fdata->FilterMode = rbList->isChecked()?1:0;
    fdata->FilterIdLow = sbMinID->value();
    fdata->FilterIdHigh = sbMaxID->value();
    fdata->FilterMaskIdLow = sbMinMask->value();
    fdata->FilterMaskIdHigh = sbMaxMask->value();

}

void FilterDialog::setReadOnly(bool ro)
{
    gbID->setDisabled(ro);
    gbMask->setDisabled(ro);
    gbType->setDisabled(ro);
    gbScale->setDisabled(ro);
}

void FilterDialog::onDataChanged()
{
    //buttonBox->button(QDialogButtonBox::Ok)->setDisabled((sbMinID->value() > sbMaxID->value()) || (sbMinMask->value() > sbMaxMask->value()));
}
