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
#include "plot2d.h"
#include "axesdialog.h"

#include <float.h>
#include <math.h>
#include <QSettings>
#include <QPainter>
#include <QCloseEvent>
#include <QDebug>
#include <QIcon>


Plot2D::Plot2D(QWidget *parent, QString Title)
    : QWidget(parent)
    , sTitle(Title)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);
    setWindowFlags(windowFlags() |  Qt::WindowMinMaxButtonsHint);
    setMouseTracking(true);
//  setAttribute(Qt::WA_AlwaysShowToolTips);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setWindowIcon(QIcon(":/plot.png"));
    QSettings settings;
    restoreGeometry(settings.value(sTitle+QString("Plot2D")).toByteArray());
    xMarker      = 0.0;
    yMarker      = 0.0;
    bShowMarker  = false;
    bZooming     = false;

    pPropertiesDlg = new plotPropertiesDlg(sTitle);
    connect(pPropertiesDlg, SIGNAL(configChanged()),
            this, SLOT(UpdatePlot()));

    labelPen = pPropertiesDlg->labelColor;//QPen(Qt::white);
    gridPen  = pPropertiesDlg->gridColor; //QPen(Qt::blue);
    framePen = pPropertiesDlg->frameColor;//QPen(Qt::blue);
    gridPen.setWidth(pPropertiesDlg->gridPenWidth);

    sMouseCoord = QString("X=%1 Y=%2")
                  .arg(0.0, 10, 'g', 7, ' ')
                  .arg(0.0, 10, 'g', 7, ' ');

    setCursor(Qt::CrossCursor);
    setWindowTitle(Title);
}


Plot2D::~Plot2D() {
    QSettings settings;
    settings.setValue(sTitle+QString("Plot2D"), saveGeometry());
    while(!dataSetList.isEmpty()) {
        delete dataSetList.takeFirst();
    }
}


void
Plot2D::setTitle(QString sNewTitle) {
    sTitle = sNewTitle;
}


void
Plot2D::keyPressEvent(QKeyEvent *e) {
    // To avoid closing the Plot upon Esc keypress
    if(e->key() != Qt::Key_Escape)
        QWidget::keyPressEvent(e);
}


void
Plot2D::closeEvent(QCloseEvent *event) {
    QSettings settings;
    settings.setValue(sTitle+QString("Plot2D"), saveGeometry());
    event->ignore();
}


void
Plot2D::paintEvent(QPaintEvent *event) {
    QPainter painter;
    painter.begin(this);
    painter.setFont(pPropertiesDlg->painterFont);
    QFontMetrics fontMetrics = painter.fontMetrics();
    painter.fillRect(event->rect(), QBrush(pPropertiesDlg->painterBkColor));
    DrawPlot(&painter, fontMetrics);
    QRect textSize = fontMetrics.boundingRect(sMouseCoord);
    int nPosX = (width()/2) - (textSize.width()/2);
    int nPosY = height() - 4;
    painter.setPen(labelPen);
    painter.drawText(nPosX, nPosY, sMouseCoord);
    painter.end();
}


QSize
Plot2D::minimumSizeHint() const {
   return QSize(50, 50);
}


QSize
Plot2D::sizeHint() const {
   return QSize(330, 330);
}


void
Plot2D::setMaxPoints(int nPoints) {
    if(nPoints > 0) pPropertiesDlg->maxDataPoints = nPoints;
    for(int pos=0; pos<dataSetList.count(); pos++) {
        dataSetList.at(pos)->setMaxPoints(pPropertiesDlg->maxDataPoints);
    }
}


void
Plot2D::SetLimits (double XMin, double XMax, double YMin, double YMax,
                   bool AutoX, bool AutoY, bool LogX, bool LogY)
{
    Ax.XMin  = XMin;
    Ax.XMax  = XMax;
    Ax.YMin  = YMin;
    Ax.YMax  = YMax;
    Ax.AutoX = AutoX;
    Ax.AutoY = AutoY;
    Ax.LogX  = LogX;
    Ax.LogY  = LogY;

    if(!dataSetList.isEmpty()) {
        if(AutoX | AutoY) {
            bool EmptyData = true;
            if(Ax.AutoX) {
                if(Ax.LogX) {
                    XMin = double(FLT_MAX);
                    XMax = double(FLT_MIN);
                } else {
                    XMin = double(FLT_MAX);
                    XMax =-double(FLT_MAX);
                }
            }
            if(Ax.AutoY) {
                if(Ax.LogY) {
                    YMin = double(FLT_MAX);
                    YMax = double(FLT_MIN);
                } else {
                    YMin = double(FLT_MAX);
                    YMax =-double(FLT_MAX);
                }
            }
            DataStream2D* pData;
            for(int pos=0; pos<dataSetList.count(); pos++) {
                pData = dataSetList.at(pos);
                if(pData->isShown) {
                    if(pData->m_pointArrayX.count() != 0) {
                        EmptyData = false;
                        if(Ax.AutoX) {
                            if(XMin > pData->minx) {
                                XMin = pData->minx;
                            }
                            if(XMax < pData->maxx) {
                                XMax = pData->maxx;
                            }
                        }// if(Ax.AutoX)
                        if(Ax.AutoY) {
                            if(YMin > pData->miny) {
                                YMin = pData->miny;
                            }
                            if(YMax < pData->maxy) {
                                YMax = pData->maxy;
                            }
                        }// if(Ax.AutoY)
                    }// if(pData->m_pointArray.GetSize() != 0)
                }// if(pData->isShowed)
            }// while (pos != NULL)
            if(EmptyData) {
                XMin = Ax.XMin;
                XMax = Ax.XMax;
                YMin = Ax.YMin;
                YMax = Ax.YMax;
            }
        }
    }
    if(abs(XMin-XMax) < double(FLT_MIN)) {
        XMin  -= 0.05*(XMax+XMin)+double(FLT_MIN);
        XMax  += 0.05*(XMax+XMin)+double(FLT_MIN);
    }
    if(abs(YMin-YMax)  < double(FLT_MIN)) {
        YMin  -= 0.05*(YMax+YMin)+double(FLT_MIN);
        YMax  += 0.05*(YMax+YMin)+double(FLT_MIN);
    }
    if(XMin > XMax) {
        double tmp = XMin;
        XMin = XMax;
        XMax = tmp;
    }
    if(YMin > YMax) {
        double tmp = YMin;
        YMin = YMax;
        YMax = tmp;
    }
    if(LogX) {
        if(XMin <= 0.0) XMin = double(FLT_MIN);
        if(XMax <= 0.0) XMax = 2.0*double(FLT_MIN);
    }
    if(LogY) {
        if(YMin <= 0.0) YMin = double(FLT_MIN);
        if(YMax <= 0.0) YMax = 2.0*double(FLT_MIN);
    }
    Ax.XMin  = XMin;
    Ax.XMax  = XMax;
    Ax.YMin  = YMin;
    Ax.YMax  = YMax;
}


DataStream2D*
Plot2D::NewDataSet(int Id, int PenWidth, QColor Color, int Symbol, QString Title) {
    DataStream2D* pDataItem = new DataStream2D(Id, PenWidth, Color, Symbol, Title);
    pDataItem->setMaxPoints(pPropertiesDlg->maxDataPoints);
    dataSetList.append(pDataItem);
    return pDataItem;
}


bool
Plot2D::ClearDataSet(int Id) {
    bool bResult = false;
    for(int i=0; i<dataSetList.count(); i++) {
        DataStream2D* pDataItem = dataSetList.at(i);
        if(pDataItem->GetId() == Id) {
            pDataItem->RemoveAllPoints();
            bResult = true;
        }
    }
    return bResult;
}


void
Plot2D::SetShowDataSet(int Id, bool Show) {
    if(!dataSetList.isEmpty()) {
        for(int pos=0; pos<dataSetList.count(); pos++) {
            DataStream2D* pData = dataSetList.at(pos);
            if(pData->GetId() == Id) {
                pData->SetShow(Show);
                break;
            }
        }
    }
}


void
Plot2D::NewPoint(int Id, double x, double y) {
    if(std::isnan(y)) return;
    if(dataSetList.isEmpty())  return;
    DataStream2D* pData = Q_NULLPTR;
    for(int pos=0; pos<dataSetList.count(); pos++) {
        if(dataSetList.at(pos)->GetId() == Id) {
            pData = dataSetList.at(pos);
            break;
        }
    }
    if(pData) {
        pData->AddPoint(x, y);
    }
}


void
Plot2D::DrawData(QPainter* painter, QFontMetrics fontMetrics) {
    if(dataSetList.isEmpty()) return;
    DataStream2D* pData;
    for(int pos=0; pos<dataSetList.count(); pos++) {
        pData = dataSetList.at(pos);
        if(pData->isShown) {
            if(pData->GetProperties().Symbol == iline) {
                LinePlot(painter, pData);
            } else if(pData->GetProperties().Symbol == ipoint) {
                PointPlot(painter, pData);
            } else {
                ScatterPlot(painter, pData);
            }
            if(pData->bShowCurveTitle) ShowTitle(painter, fontMetrics, pData);
        }
    }
}


void
Plot2D::SetShowTitle(int Id, bool show) {
    if(dataSetList.isEmpty()) return;
    DataStream2D* pData;
    for(int pos=0; pos<dataSetList.count(); pos++) {
        pData = dataSetList.at(pos);
        if(pData->GetId() == Id) {
            pData->SetShowTitle(show);
            return;
        }
    }
}


void
Plot2D::ShowTitle(QPainter* painter, QFontMetrics fontMetrics, DataStream2D *pData) {
    QPen titlePen = QPen(pData->GetProperties().Color);
    painter->setPen(titlePen);
    painter->drawText(int(Pf.right+4), int(Pf.top+fontMetrics.height()*(pData->GetId())), pData->GetTitle());
}


void
Plot2D::XTicLin(QPainter* painter, QFontMetrics fontMetrics) {
    double xmax, xmin;
    double dx, dxx, b, fmant;
    int isx, ic, iesp, jy, isig, ix, ix0, iy0;
    QString Label;

    if (Ax.XMax <= 0.0) {
        xmax =-Ax.XMin;	xmin=-Ax.XMax; isx= -1;
    } else {
        xmax = Ax.XMax; xmin= Ax.XMin; isx= 1;
    }
    dx = xmax - xmin;
    b = log10(dx);
    ic = qRound(b) - 2;
    dx = double(qRound(pow(10.0, (b-ic-1.0))));

    if(dx < 11.0) dx = 10.0;
    else if(dx < 28.0) dx = 20.0;
    else if(dx < 70.0) dx = 50.0;
    else dx = 100.0;

    dx = dx * pow(10.0, double(ic));
    xfact = (Pf.right-Pf.left) / (xmax-xmin);
    dxx = (xmax+dx) / dx;
    dxx = floor(dxx) * dx;
    iy0 = int(Pf.bottom + fontMetrics.height()+5);
    iesp = int(floor(log10(dxx)));
    if (dxx > xmax) dxx = dxx - dx;
    do {
        if(isx == -1)
            ix = int(Pf.right-(dxx-xmin) * xfact);
        else
            ix = int((dxx-xmin) * xfact + Pf.left);
        jy = int(Pf.bottom + 5);// Perche' 5 ?
        painter->setPen(gridPen);
        painter->drawLine(QLine(ix, int(Pf.top), ix, jy));
        isig = 0;
        if(dxx == 0.0)
            fmant= 0.0;
        else {
            isig = int(dxx/fabs(dxx));
            dxx = fabs(dxx);
            fmant = log10(dxx) - double(iesp);
            fmant = pow(10.0, fmant)*10000.0 + 0.5;
            fmant = floor(fmant)/10000.0;
            fmant = isig * fmant;
        }
        if(double(isx*fmant) <= -10.0)
            Label = QString("%1").arg(double(isx*fmant), 6, 'f', 2, ' ');
        else
            Label = QString("%1").arg(double(isx*fmant), 6, 'f', 3, ' ');
        ix0 = ix - fontMetrics.horizontalAdvance(Label)/2;
        painter->setPen(labelPen);
        painter->drawText(QPoint(ix0, iy0), Label);
        dxx = isig*dxx - dx;
    } while(dxx >= xmin);
    painter->setPen(labelPen);
    painter->drawText(QPoint(int(Pf.right + 2),	int(Pf.bottom - 0.5*fontMetrics.height())), "x10");
    int icx = fontMetrics.horizontalAdvance("x10 ");
    Label = QString("%1").arg(iesp, 0, 10, QLatin1Char(' '));
    painter->setPen(labelPen);
    painter->drawText(QPoint(int(Pf.right+icx),	int(Pf.bottom - fontMetrics.height())), Label);
}


void
Plot2D::YTicLin(QPainter* painter, QFontMetrics fontMetrics) {
    double ymax, ymin;
    double dy, dyy, b, fmant;
    int isy, icc, iesp, jx, isig, iy, ix0, iy0;
    QString Label;

    if (Ax.YMax <= 0.0) {
        ymax = -Ax.YMin; ymin= -Ax.YMax; isy= -1;
    } else {
        ymax = Ax.YMax; ymin= Ax.YMin; isy= 1;
    }
    dy = ymax - ymin;
    b = log10(dy);
    icc = qRound(b) - 2;
    dy = double(qRound(pow(10.0, (b-icc-1.0))));

    if(dy < 11.0) dy = 10.0;
    else if(dy < 28.0) dy = 20.0;
    else if(dy < 70.0) dy = 50.0;
    else dy = 100.0;

    dy = dy * pow(10.0, double(icc));
    yfact = (Pf.top-Pf.bottom) / (ymax-ymin);
    dyy = (ymax+dy) / dy;
    dyy = floor(dyy) * dy;
    iesp = int(floor(log10(dyy)));
    if(dyy > ymax) dyy = dyy - dy;
    do {
        if(isy == -1)
            iy = int(Pf.top - (dyy-ymin) * yfact);
        else
            iy = int((dyy-ymin) * yfact + Pf.bottom);
        jx = int(Pf.right);
        painter->setPen(gridPen);
        painter->drawLine(QLine(int(Pf.left-5), iy, jx, iy));
        isig = 0;
        if(dyy == 0.0)
            fmant = 0.0;
        else{
            isig = int(dyy/fabs(dyy));
            dyy = fabs(dyy);
            fmant = log10(dyy) - double(iesp);
            fmant = pow(10.0, fmant)*10000.0 + 0.5;
            fmant = floor(fmant)/10000.0;
            fmant = isig * fmant;
        }
        if(double(isy*fmant) <= -10.0)
            Label = QString("%1").arg(double(isy*fmant), 7, 'f', 3, ' ');
        else
            Label = QString("%1").arg(double(isy*fmant), 7, 'f', 4, ' ');
        ix0 = int(Pf.left - fontMetrics.horizontalAdvance(Label) - 5);
        iy0 = iy + fontMetrics.height()/2;
        painter->setPen(labelPen);
        painter->drawText(QPoint(ix0, iy0), Label);
        dyy = isig*dyy - dy;
    }	while (dyy >= ymin);
    QPoint point(int(Pf.left), int(Pf.top-0.5*fontMetrics.height()));
    painter->setPen(labelPen);
    painter->drawText(point, "x10");
    int icx = fontMetrics.horizontalAdvance("x10 ");
    Label = QString("%1").arg(iesp, 0, 10, QLatin1Char(' '));
    painter->setPen(labelPen);
    painter->drawText(QPoint(int(int(Pf.left)+icx),int(Pf.top-fontMetrics.height())),Label);
}


void
Plot2D::XTicLog(QPainter* painter, QFontMetrics fontMetrics) {
    int i, ix, ix0, iy0, jy, j;
    double dx;
    QString Label;

    jy = int(Pf.bottom + 5);// Perche' 5 ?
    iy0 = int(Pf.bottom + fontMetrics.height()+5);

    if(Ax.XMin < double(FLT_MIN)) Ax.XMin = double(FLT_MIN);
    if(Ax.XMax < double(FLT_MIN)) Ax.XMax = 10.0*double(FLT_MIN);

    double xlmin = log10(Ax.XMin);
    int minx = int(xlmin);
    if((xlmin < 0.0) && fabs(xlmin-minx) <= double(FLT_MIN)) minx= minx - 1;

    double xlmax = log10(Ax.XMax);
    int maxx = int(xlmax);
    if((xlmax > 0.0) && fabs(xlmax-maxx) <= double(FLT_MIN)) maxx= maxx + 1;

    xfact = (Pf.right-Pf.left) / ((xlmax-xlmin)+double(FLT_MIN));

    bool init = true;
    int decades = maxx - minx;
    double x = pow(10.0, minx);
    if(decades < 6) {
        for(i=0; i<decades; i++) {
            dx = pow(10.0, (minx + i));
            if(x >= Ax.XMin) {
                ix = int(Pf.left + (log10(x)-xlmin)*xfact);
                Label = QString("%1").arg(x, 7, 'e', 0, ' ');
                ix0 = ix - fontMetrics.horizontalAdvance(Label)/2;
                painter->setPen(labelPen);
                painter->drawText(QPoint(ix0, iy0), Label);
                init = false;
            }
            for(j=1; j<10; j++){
                x = x + dx;
                if((x >= Ax.XMin) && (x <= Ax.XMax)) {
                    ix = int(Pf.left + (log10(x)-xlmin)*xfact);
                    painter->setPen(gridPen);
                    painter->drawLine(QLine(ix, int(Pf.top), ix, jy));
                    Label = QString("%1").arg(x, 7, 'e', 0, ' ');
                    if(init || (j == 9 && decades == 1)) {
                        ix0 = ix - fontMetrics.horizontalAdvance(Label)/2;
                        painter->setPen(labelPen);
                        painter->drawText(QPoint(ix0, iy0), Label);
                        init = false;
                    } else if (decades == 1) {
                        Label = Label.left(2);
                        ix0 = ix - fontMetrics.horizontalAdvance(Label)/2;
                        painter->setPen(labelPen);
                        painter->drawText(QPoint(ix0, iy0), Label);
                    }
                }
            }
        }// for(i=0; i<decades; i++)
        if((decades != 1) && (x <= Ax.XMax)) {
            Label = QString("%1").arg(x, 7, 'e', 0, ' ');
            ix = int(Pf.left + (log10(x)-xlmin)*xfact);
            ix0 = ix - fontMetrics.horizontalAdvance(Label)/2;
            painter->setPen(labelPen);
            painter->drawText(QPoint(ix0, iy0), Label);
        }
    } else {// decades > 5
        for(i=1; i<=decades; i++) {
            x = pow(10.0, minx + i);
            if((x >= Ax.XMin) && (x <= Ax.XMax)) {
                ix = int(Pf.left + (log10(x)-xlmin)*xfact);
                painter->setPen(gridPen);
                painter->drawLine(QLine(ix, int(Pf.top),ix, jy));
                Label = QString("%1").arg(x, 7, 'e', 0, ' ');
                ix0 = ix - fontMetrics.horizontalAdvance(Label)/2;
                painter->setPen(labelPen);
                painter->drawText(QPoint(ix0, iy0), Label);
            }
        }
    }//if(decades < 6)
}


void
Plot2D::YTicLog(QPainter* painter, QFontMetrics fontMetrics) {
    int i, iy, ix0, iy0, j;
    double dy;
    QString Label;

    if(Ax.YMin < double(FLT_MIN)) Ax.YMin = double(FLT_MIN);
    if(Ax.YMax < double(FLT_MIN)) Ax.YMax = 10.0*double(FLT_MIN);

    double ylmin = log10(Ax.YMin);
    int miny = int(ylmin);
    if((ylmin < 0.0) && fabs(ylmin-miny) <= double(FLT_MIN)) miny= miny - 1;

    double ylmax = log10(Ax.YMax);
    int maxy = int(ylmax);
    if((ylmax > 0.0) && fabs(ylmax-maxy) <= double(FLT_MIN)) maxy= maxy + 1;

    yfact = (Pf.top-Pf.bottom) / ((ylmax-ylmin)+double(FLT_MIN));

    bool init = true;
    int decades = maxy - miny;
    double y = pow(10.0, miny);
    if(decades < 6) {
        for(i=0; i<decades; i++) {
            dy = pow(10.0, (miny + i));
            if(y >= Ax.YMin) {
                iy = int(Pf.bottom + (log10(y)-ylmin)*yfact);
                Label = QString("%1").arg(y, 7, 'e', 0, ' ');
                ix0 = int(Pf.left - fontMetrics.horizontalAdvance(Label) - 5);
                iy0 = iy + fontMetrics.height()/2;
                painter->setPen(labelPen);
                painter->drawText(QPoint(ix0, iy0), Label);
                init = false;
            }
            for(j=1; j<10; j++){
                y = y + dy;
                if((y >= Ax.YMin) && (y <= Ax.YMax)) {
                    iy = int(Pf.bottom + (log10(y)-ylmin)*yfact);
                    painter->setPen(gridPen);
                    painter->drawLine(QLine(int(Pf.left-5), iy, int(Pf.right), iy));
                    Label = QString("%1").arg(y, 7, 'e', 0, ' ');
                    if(init || (j == 9 && decades == 1)) {
                        ix0 = int(Pf.left - fontMetrics.horizontalAdvance(Label) - 5);
                        iy0 = iy + fontMetrics.height()/2;
                        painter->setPen(labelPen);
                        painter->drawText(QPoint(ix0, iy0), Label);
                        init = false;
                    } else if (decades == 1) {
                        Label = Label.left(2);
                        ix0 = int(Pf.left - fontMetrics.horizontalAdvance(Label) - 5);
                        iy0 = iy + fontMetrics.height()/2;
                        painter->setPen(labelPen);
                        painter->drawText(QPoint(ix0, iy0), Label);
                    }
                }
            }
        }// for(i=0; i<decades; i++)
        if((decades != 1) && (y <= Ax.YMax)) {
            Label = QString("%1").arg(y, 7, 'e', 0, ' ');
            iy = int(Pf.bottom - (log10(y)-ylmin)*yfact);
            ix0 = int(Pf.left - fontMetrics.horizontalAdvance(Label) - 5);
            iy0 = iy + fontMetrics.height()/2;
            painter->setPen(labelPen);
            painter->drawText(QPoint(ix0, iy0), Label);
        }
    } else {// decades > 5
        for(i=1; i<=decades; i++) {
            y = pow(10.0, miny + i);
            if((y >= Ax.YMin) && (y <= Ax.YMax)) {
                iy = int(Pf.bottom + (log10(y)-ylmin)*yfact);
                painter->setPen(gridPen);
                painter->drawLine(QLine(int(Pf.left-5), iy, int(Pf.right), iy));
                Label = QString("%1").arg(y, 7, 'e', 0, ' ');
                ix0 = int(Pf.left - fontMetrics.horizontalAdvance(Label) - 5);
                iy0 = iy + fontMetrics.height()/2;
                painter->setPen(labelPen);
                painter->drawText(QPoint(ix0, iy0), Label);
            }
        }
    }//if(decades < 6)
}


void
Plot2D::DrawFrame(QPainter* painter, QFontMetrics fontMetrics) {
    if(Ax.LogX) XTicLog(painter, fontMetrics); else XTicLin(painter, fontMetrics);
    if(Ax.LogY) YTicLog(painter, fontMetrics); else YTicLin(painter, fontMetrics);

    painter->setPen(framePen);
    painter->drawLine(QLine(int(Pf.left), int(Pf.bottom), int(Pf.right), int(Pf.bottom)));
    painter->drawLine(QLine(int(Pf.right), int(Pf.bottom), int(Pf.right), int(Pf.top)));
    painter->drawLine(QLine(int(Pf.right), int(Pf.top), int(Pf.left), int(Pf.top)));
    painter->drawLine(QLine(int(Pf.left), int(Pf.top), int(Pf.left), int(Pf.bottom)));

    painter->setPen(labelPen);
    int icx = fontMetrics.horizontalAdvance((sTitle));
    painter->drawText(QPoint(int((width()-icx)/2), int(fontMetrics.height())), sTitle);
}


void
Plot2D::DrawPlot(QPainter* painter, QFontMetrics fontMetrics) {
    if(Ax.AutoX || Ax.AutoY) {
        SetLimits (Ax.XMin, Ax.XMax, Ax.YMin, Ax.YMax, Ax.AutoX, Ax.AutoY, Ax.LogX, Ax.LogY);
    }

    Pf.left = fontMetrics.horizontalAdvance("-0.00000") + 2.0;
    Pf.right = width() - fontMetrics.horizontalAdvance("x10-999") - 5.0;
    Pf.top = 2.0 * fontMetrics.height();
    Pf.bottom = height() - 3.0*fontMetrics.height();

    DrawFrame(painter, fontMetrics);
    DrawData(painter, fontMetrics);
    if(bZooming) {
        QPen zoomPen(Qt::yellow);
        painter->setPen(zoomPen);
        int ix0 = zoomStart.rx() < zoomEnd.rx() ? zoomStart.rx() : zoomEnd.rx();
        int iy0 = zoomStart.ry() < zoomEnd.ry() ? zoomStart.ry() : zoomEnd.ry();
        painter->drawRect(ix0, iy0, abs(zoomStart.rx()-zoomEnd.rx()), abs(zoomStart.ry()-zoomEnd.ry()));
    }
}


void
Plot2D::LinePlot(QPainter* painter, DataStream2D* pData) {
    if(!pData->isShown) return;
    int iMax = int(pData->m_pointArrayX.count());
    if(iMax == 0) return;
    QPen dataPen = QPen(pData->GetProperties().Color);
    dataPen.setWidth(pData->GetProperties().PenWidth);
    painter->setPen(dataPen);
    int ix0, iy0, ix1, iy1;
    double xlmin, ylmin;
    if(Ax.XMin > 0.0)
        xlmin = log10(Ax.XMin);
    else
        xlmin = double(FLT_MIN);
    if(Ax.YMin > 0.0)
        ylmin = log10(Ax.YMin);
    else ylmin = double(FLT_MIN);

    if(Ax.LogX) {
        if(pData->m_pointArrayX[0] > 0.0)
            ix0 = int((Pf.left + (log10(pData->m_pointArrayX[0]) - xlmin)*xfact));
        else
            ix0 =-INT_MAX; // Solo per escludere il punto
    } else
        ix0 = int((Pf.left + (pData->m_pointArrayX[0] - Ax.XMin)*xfact));

    if(Ax.LogY) {
        if(pData->m_pointArrayY[0] > 0.0)
            iy0 = int((Pf.bottom + (log10(pData->m_pointArrayY[0]) - ylmin)*yfact));
        else
            iy0 =-INT_MAX; // Solo per escludere il punto
    } else
        iy0 = int((Pf.bottom + (pData->m_pointArrayY[0] - Ax.YMin)*yfact));

    for(int i=1; i<iMax; i++) {
        if(Ax.LogX)
            ix1 = int(((log10(pData->m_pointArrayX[i]) - xlmin)*xfact) + Pf.left);
        else
            ix1 = int(((pData->m_pointArrayX[i] - Ax.XMin)*xfact) + Pf.left);
        if(Ax.LogY)
            if(pData->m_pointArrayY[i] > 0.0)
                iy1 = int((Pf.bottom + (log10(pData->m_pointArrayY[i]) - ylmin)*yfact));
            else
                iy1 =-INT_MAX; // Solo per escludere il punto
        else
            iy1 = int((Pf.bottom + (pData->m_pointArrayY[i] - Ax.YMin)*yfact));

        if(!(ix1<Pf.left || iy1<Pf.top || iy1>Pf.bottom)) {
            painter->drawLine(ix0, iy0, ix1, iy1);
        }
        ix0 = ix1;
        iy0 = iy1;
        if(ix1 > Pf.right) {
            break;
        }
    }
    DrawLastPoint(painter, pData);
}


void
Plot2D::DrawLastPoint(QPainter* painter, DataStream2D* pData) {
    if(!pData->isShown) return;
    int ix, iy, i;
    i = int(pData->m_pointArrayX.count()-1);

    double xlmin, ylmin;
    if(Ax.XMin > 0.0)
        xlmin = log10(Ax.XMin);
    else
        xlmin = double(FLT_MIN);
    if(Ax.YMin > 0.0)
        ylmin = log10(Ax.YMin);
    else ylmin = double(FLT_MIN);

    if(Ax.LogX) {
        if(pData->m_pointArrayX[i] > 0.0)
            ix = int(((log10(pData->m_pointArrayX[i]) - xlmin)*xfact) + Pf.left);
        else
            return;
    } else {
        ix = int(((pData->m_pointArrayX[i] - Ax.XMin)*xfact) + Pf.left);
    }
    if(Ax.LogY) {
        if(pData->m_pointArrayY[i] > 0.0)
            iy = int((Pf.bottom + (log10(pData->m_pointArrayY[i]) - ylmin)*yfact));
        else
            return;
    }
    else {
        iy = int((Pf.bottom + (pData->m_pointArrayY[i] - Ax.YMin)*yfact));
    }
    if(ix<=Pf.right && ix>=Pf.left && iy>=Pf.top && iy<=Pf.bottom)
        painter->drawPoint(ix, iy);
    return;
}


void
Plot2D::PointPlot(QPainter* painter, DataStream2D* pData) {
    int iMax = int(pData->m_pointArrayX.count());
    if(iMax == 0) return;
    QPen dataPen = QPen(pData->GetProperties().Color);
    dataPen.setWidth(pData->GetProperties().PenWidth);
    painter->setPen(dataPen);
    int ix, iy;
    double xlmin, ylmin;
    if(Ax.XMin > 0.0)
        xlmin = log10(Ax.XMin);
    else
        xlmin = double(FLT_MIN);
    if(Ax.YMin > 0.0)
        ylmin = log10(Ax.YMin);
    else ylmin = double(FLT_MIN);

    for (int i=0; i < iMax; i++) {
        if(!(pData->m_pointArrayX[i] < Ax.XMin ||
             pData->m_pointArrayX[i] > Ax.XMax ||
             pData->m_pointArrayY[i] < Ax.YMin ||
             pData->m_pointArrayY[i] > Ax.YMax ))
        {
            if(Ax.LogX) {
                if(pData->m_pointArrayX[i] > 0.0)
                    ix = int(((log10(pData->m_pointArrayX[i]) - xlmin)*xfact) + Pf.left);
                else
                    ix = -INT_MAX;
            } else
                ix = int(((pData->m_pointArrayX[i] - Ax.XMin)*xfact) + Pf.left);
            if(Ax.LogY) {
                if(pData->m_pointArrayY[i] > 0.0)
                    iy = int((Pf.bottom + (log10(pData->m_pointArrayY[i]) - ylmin)*yfact));
                else
                    iy =-INT_MAX; // Solo per escludere il punto
            } else
                iy = int((Pf.bottom + (pData->m_pointArrayY[i] - Ax.YMin)*yfact));
            painter->drawPoint(ix, iy);
        }
    }//for (int i=0; i <= iMax; i++)
}


void
Plot2D::ScatterPlot(QPainter* painter, DataStream2D* pData) {
    int iMax = int(pData->m_pointArrayX.count());
    if(iMax == 0) return;
    QPen dataPen = QPen(pData->GetProperties().Color);
    dataPen.setWidth(pData->GetProperties().PenWidth);
    painter->setPen(dataPen);
    int ix, iy;

    double xlmin, ylmin;
    if(Ax.XMin > 0.0)
        xlmin = log10(Ax.XMin);
    else
        xlmin = double(FLT_MIN);
    if(Ax.YMin > 0.0)
        ylmin = log10(Ax.YMin);
    else ylmin = double(FLT_MIN);

    int SYMBOLS_DIM = 8;
    QSize Size(SYMBOLS_DIM, SYMBOLS_DIM);

    for (int i=0; i < iMax; i++) {
        if(pData->m_pointArrayX[i] >= Ax.XMin &&
           pData->m_pointArrayX[i] <= Ax.XMax &&
           pData->m_pointArrayY[i] >= Ax.YMin &&
           pData->m_pointArrayY[i] <= Ax.YMax)
        {
            if(Ax.LogX)
                if(pData->m_pointArrayX[i] > 0.0)
                    ix = int(((log10(pData->m_pointArrayX[i]) - xlmin)*xfact) + Pf.left);
                else
                    ix = -INT_MAX;
            else//Asse X Lineare
                ix= int(((pData->m_pointArrayX[i] - Ax.XMin)*xfact) + Pf.left);
            if(Ax.LogY) {
                if(pData->m_pointArrayY[i] > 0.0)
                    iy = int(((log10(pData->m_pointArrayY[i]) - ylmin)*yfact) + Pf.bottom);
                else
                    iy =-INT_MAX; // Solo per escludere il punto
            } else
                iy = int(((pData->m_pointArrayY[i] - Ax.YMin)*yfact) + Pf.bottom);

            if(pData->GetProperties().Symbol == iplus) {
                painter->drawLine(ix, iy-Size.height()/2, ix, iy+Size.height()/2+1);
                painter->drawLine(ix-Size.width()/2, iy, ix+Size.width()/2+1, iy);
            } else if(pData->GetProperties().Symbol == iper) {
                painter->drawLine(ix-Size.width()/2+1, iy+Size.height()/2-1, ix+Size.width()/2-1, iy-Size.height()/2);
                painter->drawLine(ix+Size.width()/2-1, iy+Size.height()/2-1, ix-Size.width()/2+1, iy-Size.height()/2);
            } else if(pData->GetProperties().Symbol == istar) {
                painter->drawLine(ix, iy-Size.height()/2, ix, iy+Size.height()/2+1);
                painter->drawLine(ix-Size.width()/2, iy, ix+Size.width()/2+1, iy);
                painter->drawLine(ix-Size.width()/2+1, iy+Size.height()/2-1, ix+Size.width()/2-1, iy-Size.height()/2);
                painter->drawLine(ix+Size.width()/2-1, iy+Size.height()/2-1, ix-Size.width()/2+1, iy-Size.height()/2);
            } else if(pData->GetProperties().Symbol == iuptriangle) {
                painter->drawLine(ix, iy-Size.height()/2, ix+Size.width()/2, iy+Size.height()/2);
                painter->drawLine(ix+Size.width()/2, iy+Size.height()/2, ix-Size.width()/2, iy+Size.height()/2);
                painter->drawLine(ix-Size.width()/2, iy+Size.height()/2, ix, iy-Size.height()/2);
            } else if(pData->GetProperties().Symbol == idntriangle) {
                painter->drawLine(ix, iy+Size.height()/2, ix+Size.width()/2, iy-Size.height()/2);
                painter->drawLine(ix+Size.width()/2, iy-Size.height()/2, ix-Size.width()/2, iy-Size.height()/2);
                painter->drawLine(ix-Size.width()/2, iy-Size.height()/2, ix, iy+Size.height()/2);
            } else if(pData->GetProperties().Symbol == icircle) {
                painter->drawEllipse(QRect(ix-Size.width()/2, iy-Size.height()/2, Size.width(), Size.height()));
            } else {
                painter->drawLine(ix-Size.width()/2, iy, ix-Size.width()/2, iy-Size.height());
                painter->drawLine(ix, iy-Size.height()/2, ix-Size.width(), iy-Size.height()/2);
            }
        }
    }
}


void
Plot2D::mousePressEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::RightButton) {
        pPropertiesDlg->exec();
    }
    else if (event->buttons() & Qt::LeftButton) {
        if(event->modifiers() & Qt::ShiftModifier) {
            setCursor(Qt::SizeAllCursor);
            zoomStart = event->pos();
            bZooming = true;
        } else {
            setCursor(Qt::OpenHandCursor);
            lastPos = event->pos();
        }
    }
    event->accept();
}


void
Plot2D::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() & Qt::RightButton) {
        event->accept();
    } else if (event->button() & Qt::LeftButton) {
        if(bZooming) {
            bZooming = false;
            QPoint distance = zoomStart-zoomEnd;
            if(abs(distance.rx()) < 10 || abs(distance.ry()) < 10) return;
            double x1, x2, y1, y2, tmp;
            if(Ax.LogX) {
                x1 = pow(10.0, log10(Ax.XMin)+(zoomEnd.rx()-Pf.left)/xfact);
                x2 = pow(10.0, log10(Ax.XMin)+(zoomStart.rx()-Pf.left)/xfact);
            } else {
                x1 = (zoomEnd.rx()-Pf.left)/xfact + Ax.XMin;
                x2 = (zoomStart.rx()-Pf.left)/xfact + Ax.XMin;
            }
            if(Ax.LogY) {
                y1 = pow(10.0, log10(Ax.YMin)+(zoomEnd.ry()-Pf.bottom)/yfact);
                y2 = pow(10.0, log10(Ax.YMin)+(zoomStart.ry()-Pf.bottom)/yfact);
            } else {
                y1 = (zoomEnd.ry()-Pf.bottom) / yfact + Ax.YMin;
                y2 = (zoomStart.ry()-Pf.bottom) / yfact + Ax.YMin;
            }
            if(x2<x1) {
                tmp = x2;
                x2 = x1;
                x1 = tmp;
            }
            if(y2<y1) {
                tmp = y2;
                y2 = y1;
                y1 = tmp;
            }
            SetLimits(x1, x2, y1, y2, Ax.AutoX, Ax.AutoY, Ax.LogX, Ax.LogY);
        }
        event->accept();
    }
    update();
    setCursor(Qt::CrossCursor);
}


void
Plot2D::mouseMoveEvent(QMouseEvent *event) {
    if(event->buttons() & Qt::LeftButton) {
        if(!bZooming) {
            double xmin, xmax, ymin, ymax;
            double dxPix = event->pos().rx() - lastPos.rx();
            double dyPix = event->pos().ry() - lastPos.ry();
            double dx    = dxPix / xfact;
            double dy    = dyPix / yfact;
            if(Ax.LogX) {
                xmin = pow(10.0, log10(Ax.XMin)-dx);
                xmax = pow(10.0, log10(Ax.XMax)-dx);
            } else {
                xmin = Ax.XMin - dx;
                xmax = Ax.XMax - dx;
            }
            if(Ax.LogY) {
                ymin = pow(10.0, log10(Ax.YMin)-dy);
                ymax = pow(10.0, log10(Ax.YMax)-dy);
            } else {
                ymin = Ax.YMin - dy;
                ymax = Ax.YMax - dy;
            }
            lastPos = event->pos();
            SetLimits (xmin, xmax, ymin, ymax, Ax.AutoX, Ax.AutoY, Ax.LogX, Ax.LogY);
            update();
        } else {// is Zooming
            zoomEnd = event->pos();
            update();
        }
        event->accept();
        return;
    }
    double xval, yval;
    if(Ax.LogX) {
        xval = pow(10.0, log10(Ax.XMin)+(event->pos().rx()-Pf.left)/xfact);
    }
    else {
        xval =Ax.XMin + (event->pos().rx()-Pf.left) / xfact;
    }
    if(Ax.LogY) {
        yval = pow(10.0, log10(Ax.YMin)+(event->pos().ry()-Pf.bottom)/yfact);
    }
    else {
        yval =Ax.YMin + (event->pos().ry()-Pf.bottom) / yfact;
    }
    sMouseCoord = QString("X=%1 Y=%2")
              .arg(xval, 10, 'g', 7, ' ')
              .arg(yval, 10, 'g', 7, ' ');
    update();
    event->accept();
}


void
Plot2D::mouseDoubleClickEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    AxesDialog axesDialog(this);
    axesDialog.initDialog(Ax);
    int iRes = axesDialog.exec();
    if(iRes==QDialog::Accepted) {
        Ax = axesDialog.newLimits;
        SetLimits (Ax.XMin, Ax.XMax, Ax.YMin, Ax.YMax, Ax.AutoX, Ax.AutoY, Ax.LogX, Ax.LogY);
        update();
    }
}


void
Plot2D::UpdatePlot() {
    labelPen = pPropertiesDlg->labelColor;
    gridPen  = pPropertiesDlg->gridColor;
    framePen = pPropertiesDlg->frameColor;
    gridPen.setWidth(pPropertiesDlg->gridPenWidth);
    update();
}


void
Plot2D::ClearPlot() {
    while(!dataSetList.isEmpty()) {
        delete dataSetList.takeFirst();
    }
    update();
}



