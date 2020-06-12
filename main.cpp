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
#include <QApplication>
#include <curl/curl.h>

int
main(int argc, char *argv[]) {

    QCoreApplication::setOrganizationDomain("ipcf.cnr.it");
    QCoreApplication::setOrganizationName("IPCF-CNR");
    QCoreApplication::setApplicationName("UPS-Alarm");
    QCoreApplication::setApplicationVersion("0.1");
    CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
    if(res != 0)
        exit(EXIT_FAILURE);
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
