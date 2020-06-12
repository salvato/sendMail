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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QSettings>
#include <QTimer>
#include <QDateTime>
#include <QCloseEvent>
#include <curl/curl.h>
#include <pigpiod_if2.h> // The header for using GPIO pins on Raspberry

#include "configuredialog.h"


QT_FORWARD_DECLARE_CLASS(QFile)
QT_FORWARD_DECLARE_CLASS(QHBoxLayout)
QT_FORWARD_DECLARE_CLASS(QGridLayout)
QT_FORWARD_DECLARE_CLASS(Plot2D)


struct
upload_status {
    QStringList* pPayload;
    int lines_read;
};


class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public:
    QStringList payloadText;
    struct upload_status upload_ctx;

public slots:
    void onSetupClicked();
    void onSendClicked();
    void onTimeToCheckTemperature();
    void onTimeToResendAlarm();

protected:
    void initLayout();
    void initTemperaturePlot();
    size_t payloadSource(void *ptr, size_t size, size_t nmemb, void *userp);
    void buildPayload(QString sSubject, QString sMessage);
    bool openLogFile();
    void restoreSettings();
    void saveSettings();
    void logMessage(QString sMessage);
    void closeEvent(QCloseEvent *event);
    bool sendMail(QString sSubject, QString sMessage);

protected:
    CURL* curl;
    CURLcode res;
    struct curl_slist* recipients;
    Plot2D* pPlotTemperature;

private:
    QFile*           pLogFile;
    ConfigureDialog  configureDialog;
    QSettings        settings;
    QGridLayout*     pMainLayout;
    QPushButton      sendButton;
    QPushButton      setupButton;
    QPlainTextEdit   console;
    QTimer           updateTimer;
    QTimer           resendTimer;
    QDateTime        currentTime;
    int              gpioHostHandle;
    int              gpioSensorPin;
};
#endif // MAINWINDOW_H
