// Copyright (C) 2020  Gabriele Salvato

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef CONFIGUREDIALOG_H
#define CONFIGUREDIALOG_H

#include <QObject>
#include <QDialog>
#include <QSettings>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QDialogButtonBox>



QT_FORWARD_DECLARE_CLASS(QGridLayout)


class ConfigureDialog : public QDialog
{
    Q_OBJECT

public:
    ConfigureDialog(QWidget* parent = nullptr);
    int exec();
    QString getUsername();
    QString getMailServer();
    QString getPassword();
    QString getToDestination();
    QString getCcDestination();
    QString getCc1Destination();
    QString getMessage();

public slots:
    void onCancel();
    void onOk();
    void onMaxTemperatureEdit_textChanged(const QString &arg1);

protected:
    void initLayout();
    void saveSettings();
    void getSettings();
    void connectSignals();
    void setToolTips();

private:
    QSettings        settings;
    QGridLayout*     pMainLayout;

    QPushButton      okButton;
    QPushButton      cancelButton;

    QLabel           usernameLabel;
    QLabel           passwordLabel;
    QLabel           toLabel;
    QLabel           ccLabel;
    QLabel           cc1Label;
    QLabel           mailServerLabel;
    QLabel           maxTemperatureLabel;

    QLineEdit        usernameEdit;
    QLineEdit        passwordEdit;
    QLineEdit        toEdit;
    QLineEdit        ccEdit;
    QLineEdit        cc1Edit;
    QLineEdit        mailServerEdit;
    QLineEdit        maxTemperatureEdit;

    QLabel           textLabel;
    QPlainTextEdit   textMessage;

    QDialogButtonBox* pButtonBox;

    QString sNormalStyle;
    QString sErrorStyle;
    bool bCanClose;
};

#endif // CONFIGUREDIALOG_H
