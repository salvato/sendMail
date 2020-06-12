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

#include "mainwindow.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QMessageBox>
#include <QGridLayout>
#include <syslog.h>

#include "pigpiod_if2.h"
#include <time.h>


size_t
payloadSource(void *ptr, size_t size, size_t nmemb, void *userp) {
    struct upload_status* upload_ctx = (struct upload_status*)userp;
    if((size == 0) || (nmemb == 0) || ((size*nmemb) < 1))
        return 0;
    if(upload_ctx->lines_read >= upload_ctx->pPayload->count())
        return 0;
    QString sLine = QString("%1\r\n").arg(upload_ctx->pPayload->at(upload_ctx->lines_read));
    size_t len = qMin(size_t(sLine.length()), size*nmemb);
    memcpy(ptr, sLine.toLatin1().constData(), len);
    upload_ctx->lines_read++;
    return len;
}


MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
    , recipients(nullptr)
    , pLogFile(nullptr)
{
    gpioHostHandle = -1;
    gpioSensorPin  = 23; // BCM 23: pin 16 in the 40 pins GPIO connector
    res            = CURLE_OK;

    if(!openLogFile()) { // If unable to open Log File then Log to syslog
        QString sAppName = QFileInfo(QCoreApplication::applicationFilePath()).fileName();
        // See the syslog(3) man page
        openlog(sAppName.toLatin1().constData(), LOG_PID, LOG_USER);
    }

    initLayout();

    // Initialize the GPIO handler
    gpioHostHandle = pigpio_start((char*)"localhost", (char*)"8888");
    if(gpioHostHandle >= 0) {
        if(set_mode(gpioHostHandle, gpioSensorPin, PI_INPUT) < 0) {
            logMessage(QString("Unable to initialize GPIO%1 as Output")
                       .arg(gpioSensorPin));
            gpioHostHandle = -1;
        }
        else if(set_pull_up_down(gpioHostHandle, gpioSensorPin, PI_PUD_UP) < 0) {
            logMessage(QString("Unable to set GPIO%1 Pull-Up")
                       .arg(gpioSensorPin));
            gpioHostHandle = -1;
        }
    }
    else {
        logMessage(QString("Unable to initialize the Pi GPIO."));
    }
    connect(&updateTimer, SIGNAL(timeout()),
            this, SLOT(onTimeToCheckTemperature()));
    connect(&resendTimer, SIGNAL(timeout()),
            this, SLOT(onTimeToResendAlarm()));

    // Check the Thermostat Status every minute
    updateTimer.start(60000);
//#ifndef QT_DEBUG
    logMessage("System Started");
    if(sendMail("UPS Temperature Alarm System [INFO]",
                "The Alarm System Has Been Restarted"))
        logMessage("Message Sent");
    else
        logMessage("Unable to Send the Message");
//#endif
}


void
MainWindow::initLayout() {
    setupButton.setText("Setup");
    connect(&setupButton, SIGNAL(clicked(bool)),
            this, SLOT(onSetupClicked()));
    sendButton.setText("Test Send");
    connect(&sendButton, SIGNAL(clicked(bool)),
            this, SLOT(onSendClicked()));
    pMainLayout  = new QGridLayout();
    console.setReadOnly(true);
    console.document()->setMaximumBlockCount(100);
    QPalette p = palette();
    p.setColor(QPalette::Base, Qt::white);
    p.setColor(QPalette::Text, Qt::black);
    console.setPalette(p);
    pMainLayout->addWidget(&console,     0, 0, 4, 3);
    pMainLayout->addWidget(&setupButton, 4, 0, 1, 1);
    pMainLayout->addWidget(&sendButton,  4, 2, 1, 1);
    setLayout(pMainLayout);
    restoreSettings();
}


MainWindow::~MainWindow() {
}


void
MainWindow::closeEvent(QCloseEvent *event) {
    QString sMessage = QString("%1\n%2\n%3")
                       .arg("Switching Off the Program")
                       .arg("Close the Temperature Monitoring")
                       .arg("Are you sure ?");
    if(QMessageBox::question(this, "Quit Program", sMessage) == QMessageBox::Yes)
    {
        logMessage("Switching Off the Program");
        updateTimer.stop();
        resendTimer.stop();
        saveSettings();
        if(sendMail("UPS Temperature Alarm System [INFO]",
                    "The Alarm System Has Been Switched Off"))
            logMessage("Message Sent");
        else
            logMessage("Unable to Send the Message");
        if(gpioHostHandle >= 0)
            pigpio_stop(gpioHostHandle);
        if(pLogFile) {
            if(pLogFile->isOpen()) {
                pLogFile->flush();
            }
            pLogFile->deleteLater();
            pLogFile = nullptr;
        }
        closelog();
        event->accept();
    }
    else {
        event->ignore();
    }
}


// Open the log file
bool
MainWindow::openLogFile() {
    QString sLogFileName = QString("UPS-AlarmLog.txt");
    QString sLogDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    if(!sLogDir.endsWith(QString("/"))) sLogDir+= QString("/");
    sLogFileName = sLogDir+sLogFileName;
    // Rotate 5 previous logs, removing the oldest, to avoid data loss
    QFileInfo checkFile(sLogFileName);
    if(checkFile.exists() && checkFile.isFile()) {
        QDir renamed;
        renamed.remove(sLogFileName+QString("_4.txt"));
        for(int i=4; i>0; i--) {
            renamed.rename(sLogFileName+QString("_%1.txt").arg(i-1),
                           sLogFileName+QString("_%1.txt").arg(i));
        }
        renamed.rename(sLogFileName,
                       sLogFileName+QString("_0.txt"));
    }
    // Open the new log file
    pLogFile = new QFile(sLogFileName);
    if (!pLogFile->open(QIODevice::WriteOnly)) {
        logMessage(QString("Unable to open file %1: %2.")
                   .arg(sLogFileName).arg(pLogFile->errorString()));
        delete pLogFile;
        pLogFile = Q_NULLPTR;
        return false;
    }
    return true;
}


void
MainWindow::logMessage(QString sMessage) {
    QString sDebugMessage = currentTime.currentDateTime().toString() +
            QString(": ") +
            sMessage;
    console.appendPlainText(sDebugMessage);
    if(pLogFile) {
        if(pLogFile->isOpen()) {
            pLogFile->write(sDebugMessage.toLatin1().constData());
            pLogFile->write("\n");
            pLogFile->flush();
        }
        else
            syslog(LOG_ALERT|LOG_USER, "%s", sMessage.toLatin1().constData());
    }
    else
        syslog(LOG_ALERT|LOG_USER, "%s", sMessage.toLatin1().constData());
}


void
MainWindow::restoreSettings() {
    // Restore Geometry of the window
    restoreGeometry(settings.value("mainWindowGeometry").toByteArray());
}


void
MainWindow::saveSettings() {
    settings.setValue("mainWindowGeometry", saveGeometry());
}


void
MainWindow::buildPayload(QString sSubject, QString sMessage) {
    payloadText.clear();

    payloadText.append(QString("Date: %1")
                       .arg(currentTime.currentDateTime().toString(Qt::RFC2822Date)));
    payloadText.append(QString("To: %1")
                       .arg(configureDialog.getToDestination()));
    QString sUsername = configureDialog.getUsername();
    QString sMailFrom = QString("<%1@%2>")
                        .arg(sUsername)
                        .arg(configureDialog.getMailServer());
    payloadText.append(QString("From: %1")
                       .arg(sMailFrom));
    QString sCc = configureDialog.getCcDestination();
    if(sCc != QString()) {
        payloadText.append(QString("Cc: <%1>")
                           .arg(sCc));
    }
    QString sCc1 = configureDialog.getCc1Destination();
    if(sCc1 != QString()) {
        payloadText.append(QString("Cc: <%1>")
                           .arg(sCc1));
    }
    QString sMessageId = QString("Message-ID: <%1@ipcf.cnr.it>")
            .arg(currentTime.currentDateTime().toString().replace(QChar(' '), QChar('#')));
    payloadText.append(sMessageId);
    payloadText.append(QString("Subject: %1")
                       .arg(sSubject));
    // empty line to divide headers from body, see RFC5322
    payloadText.append(QString());
    // Body
    payloadText.append(currentTime.currentDateTime().toString());
    QStringList sMessageBody = sMessage.split("\n");
    payloadText.append(sMessageBody);
}


bool
MainWindow::sendMail(QString sSubject, QString sMessage) {
    buildPayload(sSubject, sMessage);
    upload_ctx.lines_read = 0;
    upload_ctx.pPayload = &payloadText;

    curl = curl_easy_init();
    if(curl) {
        QString mailserverURL = QString("smtps://%1@%2")
                .arg(configureDialog.getUsername())
                .arg(configureDialog.getMailServer());
        curl_easy_setopt(curl, CURLOPT_URL, mailserverURL.toLatin1().constData());
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

        QString sUsername = configureDialog.getUsername();
        QString sMailFrom = QString("<%1@%2>")
                            .arg(sUsername)
                            .arg(configureDialog.getMailServer());
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, sMailFrom.toLatin1().constData());
        curl_easy_setopt(curl, CURLOPT_USERNAME,  sUsername.toLatin1().constData());
        QString sPassword = configureDialog.getPassword();
        curl_easy_setopt(curl, CURLOPT_PASSWORD, sPassword.toLatin1().constData());

        QString sTo = QString("<%1>").arg(configureDialog.getToDestination());
        recipients = curl_slist_append(recipients, sTo.toLatin1().constData());
        QString sCc = configureDialog.getCcDestination();
        if(sCc != QString()) {
            sCc = QString("<%1>").arg(sCc);
            recipients = curl_slist_append(recipients, sCc.toLatin1().constData());
        }
        QString sCc1 = configureDialog.getCc1Destination();
        if(sCc1 != QString()) {
            sCc1 = QString("<%1>").arg(sCc1);
            recipients = curl_slist_append(recipients, sCc1.toLatin1().constData());
        }
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

        curl_easy_setopt(curl, CURLOPT_READFUNCTION, ::payloadSource);
        curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
#ifdef QT_DEBUG
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif
        // Send the message
        res = curl_easy_perform(curl);
        if(res != CURLE_OK)
            logMessage(QString("curl_easy_perform() failed: %1")
                       .arg(curl_easy_strerror(res)));

        // Free the list of recipients
        curl_slist_free_all(recipients);
        recipients = 0;
        curl_easy_cleanup(curl);
    }
    return (res==CURLE_OK);
}


void
MainWindow::onSetupClicked() {
    logMessage("Entering Configuration Manager");
    if(configureDialog.exec() == QDialog::Accepted)
        logMessage("Configuration Changed");
    else
        logMessage("No Changes in Configuration");
}


void
MainWindow::onSendClicked() {
    logMessage("Sending a Test Message");
    if(sendMail("UPS Temperature Alarm System [INFO]",
                "This is JUST A TEST MESSAGE"))
        logMessage("Message Sent");
    else
        logMessage("Unable to Send the Message");
}


void
MainWindow::onTimeToCheckTemperature() {
    int sensorStatus = gpio_read(gpioHostHandle, gpioSensorPin);
    if(sensorStatus == 0) {
        updateTimer.stop();
        logMessage("TEMPERATURE ALARM !");
        if(sendMail("UPS Temperature Alarm System [ALARM!]",
                    configureDialog.getMessage()))
            logMessage("Message Sent");
        else
            logMessage("Unable to Send the Message");
        // Start a timer to retransmit the alarm message every 30 minutes
        resendTimer.start(1800000);
    }
}


void
MainWindow::onTimeToResendAlarm() {
    int sensorStatus = gpio_read(gpioHostHandle, gpioSensorPin);
    if(sensorStatus != 0) {
        logMessage("Temperature Alarm Ceased");
        resendTimer.stop();
        // Restart the timer to check the sensor status every 10 minutes
        updateTimer.start(60000);
    }
    else { // Still on Alarm !
        logMessage("TEMPERATURE ALARM STILL ON!");
        if(sendMail("UPS Temperature Alarm System [ALARM!]",
                    configureDialog.getMessage()))
            logMessage("Message Sent");
        else
            logMessage("Unable to Send the Message");
    }
}
