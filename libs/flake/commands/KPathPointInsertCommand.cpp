/* This file is part of the KDE project
 * Copyright (C) 2006,2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006,2007 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KPathPointInsertCommand.h"
#include <klocale.h>

class KPathPointInsertCommandPrivate
{
public:
    KPathPointInsertCommandPrivate() : deletePoints(true) { }
    ~KPathPointInsertCommandPrivate() {
        if (deletePoints)
            qDeleteAll(points);
    }
    QList<KPathPointData> pointDataList;
    QList<KPathPoint*> points;
    QList<QPair<QPointF, QPointF> > controlPoints;
    bool deletePoints;
};

KPathPointInsertCommand::KPathPointInsertCommand(const QList<KPathPointData> &pointDataList, qreal insertPosition, QUndoCommand *parent)
        : QUndoCommand(parent),
        d(new KPathPointInsertCommandPrivate())
{
    if (insertPosition < 0)
        insertPosition = 0;
    if (insertPosition > 1)
        insertPosition = 1;

    //TODO the list needs to be sorted

    QList<KPathPointData>::const_iterator it(pointDataList.begin());
    for (; it != pointDataList.end(); ++it) {
        KPathShape * pathShape = it->pathShape;

        KPathSegment segment = pathShape->segmentByIndex(it->pointIndex);

        // should not happen but to be sure
        if (! segment.isValid())
            continue;

        d->pointDataList.append(*it);

        QPair<KPathSegment, KPathSegment> splitSegments = segment.splitAt(insertPosition);

        KPathPoint * split1 = splitSegments.first.second();
        KPathPoint * split2 = splitSegments.second.first();
        KPathPoint * splitPoint = new KPathPoint(pathShape, split1->point());
        if (split1->activeControlPoint1())
            splitPoint->setControlPoint1(split1->controlPoint1());
        if (split2->activeControlPoint2())
            splitPoint->setControlPoint2(split2->controlPoint2());

        d->points.append(splitPoint);
        QPointF cp1 = splitSegments.first.first()->controlPoint2();
        QPointF cp2 = splitSegments.second.second()->controlPoint1();
        d->controlPoints.append(QPair<QPointF, QPointF>(cp1, cp2));
    }
}

KPathPointInsertCommand::~KPathPointInsertCommand()
{
    delete d;
}

void KPathPointInsertCommand::redo()
{
    QUndoCommand::redo();
    for (int i = d->pointDataList.size() - 1; i >= 0; --i) {
        KPathPointData pointData = d->pointDataList.at(i);
        KPathShape * pathShape = pointData.pathShape;

        KPathSegment segment = pathShape->segmentByIndex(pointData.pointIndex);

        ++pointData.pointIndex.second;

        if (segment.first()->activeControlPoint2()) {
            QPointF controlPoint2 = segment.first()->controlPoint2();
            qSwap(controlPoint2, d->controlPoints[i].first);
            segment.first()->setControlPoint2(controlPoint2);
        }

        if (segment.second()->activeControlPoint1()) {
            QPointF controlPoint1 = segment.second()->controlPoint1();
            qSwap(controlPoint1, d->controlPoints[i].second);
            segment.second()->setControlPoint1(controlPoint1);
        }

        pathShape->insertPoint(d->points.at(i), pointData.pointIndex);
        pathShape->update();
    }
    d->deletePoints = false;
}

void KPathPointInsertCommand::undo()
{
    QUndoCommand::undo();
    for (int i = 0; i < d->pointDataList.size(); ++i) {
        const KPathPointData &pdBefore = d->pointDataList.at(i);
        KPathShape * pathShape = pdBefore.pathShape;
        KoPathPointIndex piAfter = pdBefore.pointIndex;
        ++piAfter.second;

        KPathPoint * before = pathShape->pointByIndex(pdBefore.pointIndex);

        d->points[i] = pathShape->removePoint(piAfter);

        if (d->points[i]->properties() & KPathPoint::CloseSubpath) {
            piAfter.second = 0;
        }

        KPathPoint * after = pathShape->pointByIndex(piAfter);

        if (before->activeControlPoint2()) {
            QPointF controlPoint2 = before->controlPoint2();
            qSwap(controlPoint2, d->controlPoints[i].first);
            before->setControlPoint2(controlPoint2);
        }

        if (after->activeControlPoint1()) {
            QPointF controlPoint1 = after->controlPoint1();
            qSwap(controlPoint1, d->controlPoints[i].second);
            after->setControlPoint1(controlPoint1);
        }
        pathShape->update();
    }
    d->deletePoints = true;
}

QList<KPathPoint*> KPathPointInsertCommand::insertedPoints() const
{
    return d->points;
}
