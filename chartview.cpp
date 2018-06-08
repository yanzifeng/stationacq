/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Charts module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "chartview.h"
#include <QtGui/QMouseEvent>
#include <QtCore/QDebug>
#include <QtCharts/QAbstractAxis>
#include <QtCharts/QValueAxis>
#include <QDebug>
#include <QDateTime>
#include <QLineSeries>

QT_CHARTS_USE_NAMESPACE

ChartView::ChartView(QWidget *parent)
    : QChartView(parent)
{

}

//![1]
void ChartView::keyPressEvent(QKeyEvent *event)
{
    return; // // need no response for key press, by lsq, 2016.06.28
    switch (event->key()) {
    case Qt::Key_Plus:
        chart()->zoomIn();
        break;
    case Qt::Key_Minus:
        chart()->zoomOut();
        break;
    case Qt::Key_Left:
        chart()->scroll(-1.0, 0);
        break;
    case Qt::Key_Right:
        chart()->scroll(1.0, 0);
        break;
    case Qt::Key_Up:
        chart()->scroll(0, 1.0);
        break;
    case Qt::Key_Down:
        chart()->scroll(0, -1.0);
        break;
    case Qt::Key_Space:

        break;
    default:
        QGraphicsView::keyPressEvent(event);
        break;
    }
}

void ChartView::mouseMoveEvent(QMouseEvent * event)
{

    if(event->pos().x() > chart()->plotArea().x() && event->pos().y() > chart()->plotArea().y() && event->pos().x() < chart()->plotArea().x() + chart()->plotArea().width() && event->pos().y() < chart()->plotArea().y() + chart()->plotArea().height())
    {
        QPointF valuePoint = this->chart()->mapToValue(event->pos(),this->chart()->series().at(0));

        QLineSeries *series = (QLineSeries *)chart()->series().at(0);
        QVector<QPointF> v = series->pointsVector();
        for(int i=0;i<series->count();i++)
        {
            if(v.at(i).x() >= valuePoint.x())
            {
                valuePoint = QPointF(v.at(i).x(),v.at(i).y());
                break;
            }
        }

        mTextItem->setHtml("<div style='background-color:#e6ecf7;'>" + QString("测试时间:%1, 相似度:%2").arg(QDateTime::fromMSecsSinceEpoch(valuePoint.x()).toString("yyyy-MM-dd HH:mm")).arg(valuePoint.y(), 0, 'f', 3) + "</div>");
        int mX,mY;
        if(event->pos().x() > (this->width() - 262))
        {
            mX =  this->width() - 262;
        }
        else
        {
            mX = event->pos().x();
        }
        if(event->pos().y() < 53)
        {
            mY = 53 - 25;
        }
        else
        {
            mY = event->pos().y() - 25;
        }

        mTextItem->setPos(mX,mY);

//        mLineItem->setLine(event->pos().x(),chart()->plotArea().y(),event->pos().x(),chart()->plotArea().y() + chart()->plotArea().height());
//        mTextItem->show();
//        mLineItem->show();
    }
    else
    {
//        mTextItem->hide();
//        mLineItem->hide();
    }


    QGraphicsView::mouseMoveEvent(event);

}
void ChartView::addLineTtem(QGraphicsLineItem *item)
{
//    mLineItem = item;
}

void ChartView::addTextItem(QGraphicsTextItem *item)
{
    mTextItem = item;
}
