#Copyright (C) 2020  Gabriele Salvato

#This program is free software: you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation, either version 3 of the License, or
#(at your option) any later version.

#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.

#You should have received a copy of the GNU General Public License
#along with this program.  If not, see <http://www.gnu.org/licenses/>.


QT += core
QT += gui
QT += widgets

CONFIG += c++11

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000


SOURCES += main.cpp
SOURCES += configuredialog.cpp
SOURCES += axesdialog.cpp
SOURCES += AxisFrame.cpp
SOURCES += AxisLimits.cpp
SOURCES += DataSetProperties.cpp
SOURCES += datastream2d.cpp
SOURCES += plot2d.cpp
SOURCES += plotpropertiesdlg.cpp
SOURCES += mainwindow.cpp


HEADERS += mainwindow.h
HEADERS += axesdialog.h
HEADERS += AxisFrame.h
HEADERS += AxisLimits.h
HEADERS += DataSetProperties.h
HEADERS += datastream2d.h
HEADERS += plot2d.h
HEADERS += plotpropertiesdlg.h


LIBS += -lcurl
LIBS += -lpigpiod_if2


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
