/*
 *
Copyright (C) 2016  Gabriele Salvato

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/
#include "axesdialog.h"


#include <QDialogButtonBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>


AxesDialog::AxesDialog(QWidget *parent)
    : QDialog(parent)
{
    pEditXMin  = new QLineEdit();
    pEditXMax  = new QLineEdit();
    pEditYMin  = new QLineEdit();
    pEditYMax  = new QLineEdit();

    pAutoX     = new QCheckBox("Auto X");
    pAutoY     = new QCheckBox("Auto Y");
    pLogX      = new QCheckBox("Log X");
    pLogY      = new QCheckBox("Log Y");

    pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                      QDialogButtonBox::Cancel);

    QGridLayout *pLayout = new QGridLayout();
    pLayout->addWidget(new QLabel("X Min"), 0, 0, 1, 1);
    pLayout->addWidget(new QLabel("X Max"), 1, 0, 1, 1);
    pLayout->addWidget(new QLabel("Y Min"), 2, 0, 1, 1);
    pLayout->addWidget(new QLabel("Y Max"), 3, 0, 1, 1);

    pLayout->addWidget(pEditXMin, 0, 1, 1, 2);
    pLayout->addWidget(pEditXMax, 1, 1, 1, 2);
    pLayout->addWidget(pEditYMin, 2, 1, 1, 2);
    pLayout->addWidget(pEditYMax, 3, 1, 1, 2);

    pLayout->addWidget(pAutoX, 0, 3, 1, 1);
    pLayout->addWidget(pLogX,  1, 3, 1, 1);
    pLayout->addWidget(pAutoY, 2, 3, 1, 1);
    pLayout->addWidget(pLogY,  3, 3, 1, 1);

    pLayout->addWidget(pButtonBox, 4, 0, 1, 4);

    connect(pButtonBox,
            SIGNAL(accepted()),
            this,
            SLOT(onButtonBoxAccepted()));

    connect(pButtonBox,
            SIGNAL(rejected()),
            this,
            SLOT(onButtonBoxRejected()));

    setLayout(pLayout);
}


AxesDialog::~AxesDialog() {
}


void
AxesDialog::initDialog(AxisLimits AxisLimits) {
    newLimits = AxisLimits;
    pEditXMin->setText(QString::number(AxisLimits.XMin, 'g', 4));
    pEditXMax->setText(QString::number(AxisLimits.XMax, 'g', 4));
    pEditYMin->setText(QString::number(AxisLimits.YMin, 'g', 4));
    pEditYMax->setText(QString::number(AxisLimits.YMax, 'g', 4));
    pAutoX->setChecked(AxisLimits.AutoX);
    pAutoY->setChecked(AxisLimits.AutoY);
    pLogX->setChecked(AxisLimits.LogX);
    pLogY->setChecked(AxisLimits.LogY);
}


void
AxesDialog::onButtonBoxAccepted() {
    newLimits.XMin =  pEditXMin->text().toDouble();
    newLimits.XMax =  pEditXMax->text().toDouble();
    newLimits.YMin =  pEditYMin->text().toDouble();
    newLimits.YMax =  pEditYMax->text().toDouble();
    newLimits.AutoX=  pAutoX->isChecked();
    newLimits.AutoY=  pAutoY->isChecked();
    newLimits.LogX =  pLogX->isChecked();
    newLimits.LogY =  pLogY->isChecked();
    accept();
}


void
AxesDialog::onButtonBoxRejected() {
    accept();
}
