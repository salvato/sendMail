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
#pragma once

#include <QDialog>

#include "AxisLimits.h"


QT_FORWARD_DECLARE_CLASS(QDialogButtonBox)
QT_FORWARD_DECLARE_CLASS(QLineEdit)
QT_FORWARD_DECLARE_CLASS(QCheckBox)


class AxesDialog : public QDialog
{
  Q_OBJECT

public:
    explicit AxesDialog(QWidget *parent = Q_NULLPTR);
    ~AxesDialog();
    void initDialog(AxisLimits AxisLimits);

private:
    QLineEdit        *pEditXMin;
    QLineEdit        *pEditXMax;
    QLineEdit        *pEditYMin;
    QLineEdit        *pEditYMax;
    QCheckBox        *pAutoX;
    QCheckBox        *pAutoY;
    QCheckBox        *pLogX;
    QCheckBox        *pLogY;
    QDialogButtonBox *pButtonBox;

public:
    AxisLimits newLimits;

private slots:
    void onButtonBoxAccepted();
    void onButtonBoxRejected();
};

