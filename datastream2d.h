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

#include <QVector>
#include <QColor>

#include "DataSetProperties.h"

class DataStream2D
{
public:
    DataStream2D(int Id, int PenWidth, QColor Color, int Symbol, QString Title);
    DataStream2D(DataSetProperties Properties);
    virtual ~DataStream2D();
    // Operations
    void setMaxPoints(int nPoints);
    int  getMaxPoints();
    void AddPoint(double pointX, double pointY);
    void RemoveAllPoints();
    int  GetId();
    QString GetTitle();
    DataSetProperties GetProperties();
    void SetProperties(DataSetProperties newProperties);
    void SetColor(QColor Color);
    void SetShowTitle(bool show);
    void SetTitle(QString myTitle);
    void SetShow(bool);

 // Attributes
 public:
    QVector<double> m_pointArrayX;
    QVector<double> m_pointArrayY;
    double minx;
    double maxx;
    double miny;
    double maxy;
    bool bShowCurveTitle;
    bool isShown;

 protected:
    DataSetProperties Properties;
    int maxPoints;
};
