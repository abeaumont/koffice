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

#ifndef KPATHPOINTTYPECOMMAND_H
#define KPATHPOINTTYPECOMMAND_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Flake API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//


#include <QUndoCommand>
#include <QList>
#include "KPathBaseCommand_p.h"
//#include "KPathShape.h"
#include "KPathPoint.h"
#include "KPathPointData.h"
#include "flake_export.h"

/// The undo / redo command for changing the path point type.
class FLAKE_TEST_EXPORT KPathPointTypeCommand : public KPathBaseCommand
{
public:
    /// The type of the point
    enum PointType {
        Corner,
        Smooth,
        Symmetric,
        Line,
        Curve
    };
    /**
     * Command to change the type of the given points
     * @param pointDataList List of point for changing the points
     * @param pointType the new point type to set
     * @param parent the parent command used for macro commands
     */
    KPathPointTypeCommand(const QList<KPathPointData> &pointDataList, PointType pointType, QUndoCommand *parent = 0);
    ~KPathPointTypeCommand();

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();

private:
    // used for storing the data for undo
    struct PointData {
        PointData(const KPathPointData pointData)
                : m_pointData(pointData) {}
        KPathPointData m_pointData;
        // old control points in document coordinates
        QPointF m_oldControlPoint1;
        QPointF m_oldControlPoint2;
        KPathPoint::PointProperties m_oldProperties;
        bool m_hadControlPoint1;
        bool m_hadControlPoint2;
    };

    bool appendPointData(KPathPointData data);
    void undoChanges(const QList<PointData> &data);

    PointType m_pointType;
    QList<PointData> m_oldPointData;
    QList<PointData> m_additionalPointData;
};

#endif // KPATHPOINTTYPECOMMAND_H
