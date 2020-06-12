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
#include "datastream2d.h"
#include <float.h>

DataStream2D::DataStream2D(int Id, int PenWidth, QColor Color, int Symbol, QString Title)
{
    Properties.SetId(Id);
    Properties.Color    = Color;
    Properties.PenWidth = PenWidth;
    Properties.Symbol   = Symbol;
    if(Title != QString())
        Properties.Title = Title;
    else
        Properties.Title = QString("Data Set %1").arg(Properties.GetId());
    isShown         = false;
    bShowCurveTitle = false;
    maxPoints = 100;
}


DataStream2D::DataStream2D(DataSetProperties myProperties) {
    Properties = myProperties;
    if(myProperties.Title == QString())
        Properties.Title = QString("Data Set %1").arg(Properties.GetId());
    isShown         = false;
    bShowCurveTitle = false;
    maxPoints = 100;
}


DataStream2D::~DataStream2D() {
}


void
DataStream2D::SetShow(bool show) {
   isShown = show;
}


void
DataStream2D::AddPoint(double x, double y) {
    m_pointArrayX.append(x);
    m_pointArrayY.append(y);
    if(m_pointArrayX.count() == 1) {
        minx = x-DBL_MIN;
        maxx = x+DBL_MIN;
        miny = y-DBL_MIN;
        maxy = y+DBL_MIN;
    }
    if(m_pointArrayX.count() > maxPoints) {
        m_pointArrayX.removeFirst();
        m_pointArrayY.removeFirst();
        minx = x-DBL_MIN;
        maxx = x+DBL_MIN;
        miny = y-DBL_MIN;
        maxy = y+DBL_MIN;
        for(int i=0; i<m_pointArrayX.count(); i++) {
            if(m_pointArrayX.at(i) < minx) minx = m_pointArrayX.at(i);
            if(m_pointArrayX.at(i) > maxx) maxx = m_pointArrayX.at(i);
            if(m_pointArrayY.at(i) < miny) miny = m_pointArrayY.at(i);
            if(m_pointArrayY.at(i) > maxy) maxy = m_pointArrayY.at(i);
        }
    }
    else {
        minx = minx < x ? minx : x-DBL_MIN;
        maxx = maxx > x ? maxx : x+DBL_MIN;
        miny = miny < y ? miny : y-DBL_MIN;
        maxy = maxy > y ? maxy : y+DBL_MIN;
    }
}


void
DataStream2D::SetColor(QColor Color) {
   Properties.Color = Color;
}


int
DataStream2D::GetId() {
   return Properties.GetId();
}


void
DataStream2D::RemoveAllPoints() {
    m_pointArrayX.clear();
    m_pointArrayY.clear();
}


void
DataStream2D::SetTitle(QString myTitle) {
   Properties.Title = myTitle;
}


void
DataStream2D::SetShowTitle(bool show) {
    bShowCurveTitle = show;
}


DataSetProperties
DataStream2D::GetProperties() {
    return Properties;
}


void
DataStream2D::SetProperties(DataSetProperties newProperties) {
    Properties = newProperties;
}



QString
DataStream2D::GetTitle() {
    return Properties.Title;
}


void
DataStream2D::setMaxPoints(int nPoints) {
    maxPoints = nPoints;
}


int
DataStream2D::getMaxPoints() {
    return maxPoints;
}

