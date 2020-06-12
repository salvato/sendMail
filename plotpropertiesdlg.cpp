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
#include "plotpropertiesdlg.h"

#include <QGridLayout>
#include <QLabel>
#include <QColorDialog>
#include <QFontDialog>
#include <QDebug>


plotPropertiesDlg::plotPropertiesDlg(QString sTitle, QWidget *parent)
    : QDialog(parent)
{
    sTitleGroup = sTitle;
    restoreSettings();
    initUI();

    // Create the Dialog Layout
    QGridLayout* pLayout = new QGridLayout();

    pLayout->addWidget(&BkColorButton,    0, 0, 1, 1);
    pLayout->addWidget(&frameColorButton, 0, 1, 1, 1);
    pLayout->addWidget(&gridColorButton,  1, 0, 1, 1);
    pLayout->addWidget(&labelColorButton, 1, 1, 1, 1);
    pLayout->addWidget(&labelFontButton,  2, 0, 1, 1);

    pLayout->addWidget(new QLabel("Grid Lines Width"), 3, 0, 1, 1);
    pLayout->addWidget(&gridPenWidthEdit,              3, 1, 1, 1);
    pLayout->addWidget(new QLabel("Max Data Points"),  4, 0, 1, 1);
    pLayout->addWidget(&maxDataPointsEdit,             4, 1, 1, 1);

    pLayout->addWidget(pButtonBox, 5, 0, 1, 2);

    // Set the Layout
    setLayout(pLayout);
    sNormalStyle = gridPenWidthEdit.styleSheet();

    sErrorStyle  = "QLineEdit { ";
    sErrorStyle += "color: rgb(255, 255, 255);";
    sErrorStyle += "background: rgb(255, 0, 0);";
    sErrorStyle += "selection-background-color: rgb(128, 128, 255);";
    sErrorStyle += "}";

    connectSignals();
}


void
plotPropertiesDlg::restoreSettings() {
    QSettings settings;
    settings.beginGroup(sTitleGroup);
    painterBkColor.setRgba(settings.value("PainterBKColor", QColor(Qt::black).rgba()).toUInt());
    frameColor.setRgba(settings.value("FrameColor",         QColor(Qt::blue).rgba()).toUInt());
    gridColor.setRgba(settings.value("GridColor",           QColor(Qt::blue).rgba()).toUInt());
    labelColor.setRgba(settings.value("LabelColor",         QColor(Qt::white).rgba()).toUInt());
    gridPenWidth      = settings.value("GridPenWidth",      1).toInt();
    maxDataPoints     = settings.value("MaxDataPoints",     3000).toInt();
    painterFontName   = settings.value("PainterFontName",   QString("Ubuntu")).toString();
    painterFontSize   = settings.value("PainterFontSize",   10).toInt();
    painterFontWeight = QFont::Weight(settings.value("PainterFontWeight", QFont::Bold).toInt());
    painterFontItalic = settings.value("PainterFontItalic", false).toBool();
    painterFont       = QFont(painterFontName,
                              painterFontSize,
                              painterFontWeight,
                              painterFontItalic);
    emit configChanged();
}


void
plotPropertiesDlg::saveSettings() {
    QSettings settings;
    settings.beginGroup(sTitleGroup);
    settings.setValue("LabelColor", labelColor.rgba());
    settings.setValue("GridColor", gridColor.rgba());
    settings.setValue("FrameColor", frameColor.rgba());
    settings.setValue("PainterBKColor", painterBkColor.rgba());
    settings.setValue("GridPenWidth", gridPenWidth);
    settings.setValue("MaxDataPoints", maxDataPoints);
    settings.setValue("PainterFontName", painterFontName);
    settings.setValue("PainterFontSize", painterFontSize);
    settings.setValue("PainterFontWeight", painterFontWeight);
    settings.setValue("PainterFontItalic", painterFontItalic);
}


void
plotPropertiesDlg::setToolTips() {
    QString sHeader = QString("Enter values in range [%1 : %2]");
    gridPenWidthEdit.setToolTip(sHeader.arg(1).arg(10));
    maxDataPointsEdit.setToolTip(sHeader.arg(1).arg(10000));
}


void
plotPropertiesDlg::initUI() {
    BkColorButton.setText("Bkg Color");
    frameColorButton.setText("Frame Color");
    gridColorButton.setText("Grid Color");
    labelColorButton.setText("Labels Color");
    labelFontButton.setText("Label Font");

    gridPenWidthEdit.setText(QString("%1").arg(gridPenWidth));
    maxDataPointsEdit.setText(QString("%1").arg(maxDataPoints));

    pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                      QDialogButtonBox::Cancel);
    setToolTips();
}


void
plotPropertiesDlg::connectSignals() {
    connect(&BkColorButton, SIGNAL(clicked()),
            this, SLOT(onChangeBkColor()));
    connect(&frameColorButton, SIGNAL(clicked()),
            this, SLOT(onChangeFrameColor()));
    connect(&gridColorButton, SIGNAL(clicked()),
            this, SLOT(onChangeGridColor()));
    connect(&labelColorButton, SIGNAL(clicked()),
            this, SLOT(onChangeLabelsColor()));
    connect(&labelFontButton, SIGNAL(clicked()),
            this, SLOT(onChangeLabelsFont()));
    // Line Edit
    connect(&gridPenWidthEdit, SIGNAL(textChanged(const QString)),
            this, SLOT(onChangeGridPenWidth(const QString)));
    connect(&maxDataPointsEdit, SIGNAL(textChanged(const QString)),
            this, SLOT(onChangeMaxDataPoints(const QString)));
    // Button Box
    connect(pButtonBox, SIGNAL(accepted()),
            this, SLOT(onOk()));
    connect(pButtonBox, SIGNAL(rejected()),
            this, SLOT(onCancel()));
}


void
plotPropertiesDlg::onCancel() {
    restoreSettings();
    emit configChanged();
    reject();
}


void
plotPropertiesDlg::onOk() {
    saveSettings();
    accept();
}


void
plotPropertiesDlg::onChangeBkColor() {
    QColorDialog colorDialog(painterBkColor);
    colorDialog.setOption(QColorDialog::DontUseNativeDialog, true);
    if(colorDialog.exec()==QDialog::Accepted) {
        painterBkColor = colorDialog.currentColor();
        emit configChanged();
    }
}


void
plotPropertiesDlg::onChangeFrameColor() {
    QColorDialog colorDialog(frameColor);
    colorDialog.setOption(QColorDialog::DontUseNativeDialog, true);
    if(colorDialog.exec()==QDialog::Accepted) {
        frameColor = colorDialog.currentColor();
        emit configChanged();
    }
}


void
plotPropertiesDlg::onChangeGridColor() {
    QColorDialog colorDialog(gridColor);
    colorDialog.setOption(QColorDialog::DontUseNativeDialog, true);
    if(colorDialog.exec()==QDialog::Accepted) {
        gridColor = colorDialog.currentColor();
        emit configChanged();
    }
}


void
plotPropertiesDlg::onChangeLabelsColor() {
    QColorDialog colorDialog(labelColor);
    colorDialog.setOption(QColorDialog::DontUseNativeDialog, true);
    if(colorDialog.exec()==QDialog::Accepted) {
        labelColor = colorDialog.currentColor();
        emit configChanged();
    }
}


void
plotPropertiesDlg::onChangeLabelsFont() {
    QFontDialog* pFontDialog = new QFontDialog(this);
    pFontDialog->setCurrentFont(painterFont);
    pFontDialog->setOptions(QFontDialog::MonospacedFonts);
    if(pFontDialog->exec() == QDialog::Accepted) {
        painterFont       = pFontDialog->currentFont();
        painterFontName   = painterFont.family();
        painterFontSize   = painterFont.pointSize();
        painterFontWeight = QFont::Weight(painterFont.weight());
        painterFontItalic = painterFont.italic();
        //qDebug() << painterFontName << painterFontSize <<painterFontWeight;
        emit configChanged();
    }
}


void
plotPropertiesDlg::onChangeGridPenWidth(const QString sNewVal) {
    if((sNewVal.toInt() > 0) &&
       (sNewVal.toInt() < 11))
    {
        gridPenWidth = sNewVal.toInt();
        gridPenWidthEdit.setStyleSheet(sNormalStyle);
        emit configChanged();
    }
    else {
        gridPenWidthEdit.setStyleSheet(sErrorStyle);
    }
}


void
plotPropertiesDlg::onChangeMaxDataPoints(const QString sNewVal) {
    if((sNewVal.toInt() > 0) &&
       (sNewVal.toInt() < 10001))
    {
        maxDataPoints = sNewVal.toInt();
        maxDataPointsEdit.setStyleSheet(sNormalStyle);
        emit configChanged();
    }
    else {
        maxDataPointsEdit.setStyleSheet(sErrorStyle);
    }
}


