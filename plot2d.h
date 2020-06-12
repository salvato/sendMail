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

#include "plotpropertiesdlg.h"
#include "datastream2d.h"
#include "AxisLimits.h"
#include "AxisFrame.h"

#include <QWidget>
#include <QPen>


class Plot2D : public QWidget
{
    Q_OBJECT
public:
    explicit Plot2D(QWidget *parent=Q_NULLPTR, QString Title="Plot 2D");
    ~Plot2D();
    void setTitle(QString sNewTitle);
    QSize minimumSizeHint() const;
    QSize sizeHint() const;
    void SetLimits (double XMin, double XMax, double YMin, double YMax,
                    bool AutoX, bool AutoY, bool LogX, bool LogY);
    DataStream2D* NewDataSet(int Id, int PenWidth, QColor Color, int Symbol, QString Title);
    bool DelDataSet(int Id);
    bool ClearDataSet(int Id);
    void NewPoint(int Id, double x, double y);
    void SetShowDataSet(int Id, bool Show);
    void SetShowTitle(int Id, bool show);
    void ClearPlot();
    void setMaxPoints(int nPoints);
    int  getMaxPoints();

signals:

public slots:
    void UpdatePlot();

public:
    static const int iline       = 0;
    static const int ipoint      = 1;
    static const int iplus       = 2;
    static const int iper        = 3;
    static const int istar       = 4;
    static const int iuptriangle = 5;
    static const int idntriangle = 6;
    static const int icircle     = 7;

protected:
    void closeEvent(QCloseEvent *event);
    void keyPressEvent(QKeyEvent *e);
    void paintEvent(QPaintEvent *event);
    void DrawPlot(QPainter* painter, QFontMetrics fontMetrics);
    void DrawFrame(QPainter* painter, QFontMetrics fontMetrics);
    void XTicLin(QPainter* painter, QFontMetrics fontMetrics);
    void XTicLog(QPainter* painter, QFontMetrics fontMetrics);
    void YTicLin(QPainter* painter, QFontMetrics fontMetrics);
    void YTicLog(QPainter* painter, QFontMetrics fontMetrics);
    void DrawData(QPainter* painter, QFontMetrics fontMetrics);
    void LinePlot(QPainter* painter, DataStream2D *pData);
    void PointPlot(QPainter* painter, DataStream2D* pData);
    void ScatterPlot(QPainter* painter, DataStream2D* pData);
    void DrawLastPoint(QPainter* painter, DataStream2D* pData);
    void ShowTitle(QPainter* painter, QFontMetrics fontMetrics, DataStream2D* pData);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
//  void wheelEvent(QWheelEvent* event);

protected:
    QList<DataStream2D*> dataSetList;
    QPen labelPen;
    QPen gridPen;
    QPen framePen;

    bool bZooming;
    bool bShowMarker;
    double xMarker, yMarker;
    AxisLimits Ax;
    AxisFrame Pf;
    QString sTitle;
    QString sMouseCoord;
    double xfact, yfact;
    QPoint lastPos, zoomStart, zoomEnd;
    plotPropertiesDlg* pPropertiesDlg;
};
