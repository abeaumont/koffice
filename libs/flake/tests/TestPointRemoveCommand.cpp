/* This file is part of the KDE project
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#include "TestPointRemoveCommand.h"

#include <QPainterPath>
#include "KPathShape.h"
#include "KPathPointRemoveCommand.h"
#include "KShapeController.h"
#include <MockShapes.h>

void TestPointRemoveCommand::redoUndoPointRemove()
{
    KPathShape path1;
    path1.moveTo(QPointF(0, 0));
    path1.lineTo(QPointF(0, 100));
    KPathPoint *point1 = path1.curveTo(QPointF(0, 50), QPointF(100, 50), QPointF(100, 100));
    KPathPoint *point2 = path1.lineTo(QPointF(200, 100));
    path1.curveTo(QPointF(200, 50), QPointF(300, 50), QPointF(300, 100));

    QPainterPath orig1(QPointF(0, 0));
    orig1.lineTo(0, 100);
    orig1.cubicTo(0, 50, 100, 50, 100, 100);
    orig1.lineTo(200, 100);
    orig1.cubicTo(200, 50, 300, 50, 300, 100);

    QVERIFY(orig1 == path1.outline());

    KPathShape path2;
    path2.moveTo(QPointF(0, 0));
    KPathPoint *point3 = path2.curveTo(QPointF(50, 0), QPointF(100, 50), QPointF(100, 100));
    path2.curveTo(QPointF(50, 100), QPointF(0, 50), QPointF(0, 0));
    path2.closeMerge();

    QList<KPathPointData> pd;
    pd.append(KPathPointData(&path1, path1.pathPointIndex(point1)));
    pd.append(KPathPointData(&path1, path1.pathPointIndex(point2)));
    pd.append(KPathPointData(&path2, path2.pathPointIndex(point3)));

    QPainterPath ppath1Org = path1.outline();
    QPainterPath ppath2Org = path2.outline();

    MockShapeController mockController;
    KShapeController shapeController(0, &mockController);

    QUndoCommand *cmd = KPathPointRemoveCommand::createCommand(pd, &shapeController);
    cmd->redo();

    QPainterPath ppath1(QPointF(0, 0));
    ppath1.lineTo(0, 100);
    ppath1.cubicTo(0, 50, 300, 50, 300, 100);

    QPainterPath ppath2(QPointF(0, 0));
    ppath2.cubicTo(50, 0, 0, 50, 0, 0);
    ppath2.closeSubpath();

    QVERIFY(ppath1 == path1.outline());
    QVERIFY(ppath2 == path2.outline());

    cmd->undo();

    QVERIFY(ppath1Org == path1.outline());
    QVERIFY(ppath2Org == path2.outline());

    delete cmd;
}

void TestPointRemoveCommand::redoUndoSubpathRemove()
{
    KPathShape path1;
    KPathPoint *point11 = path1.moveTo(QPointF(0, 0));
    KPathPoint *point12 = path1.lineTo(QPointF(0, 100));
    KPathPoint *point13 = path1.curveTo(QPointF(0, 50), QPointF(100, 50), QPointF(100, 100));
    KPathPoint *point14 = path1.lineTo(QPointF(200, 100));
    KPathPoint *point15 = path1.curveTo(QPointF(200, 50), QPointF(300, 50), QPointF(300, 100));
    KPathPoint *point21 = path1.moveTo(QPointF(0, 0));
    KPathPoint *point22 = path1.curveTo(QPointF(50, 0), QPointF(100, 50), QPointF(100, 100));
    path1.curveTo(QPointF(50, 100), QPointF(0, 50), QPointF(0, 0));
    path1.closeMerge();
    path1.moveTo(QPointF(100, 0));
    path1.lineTo(QPointF(100, 100));

    QList<KPathPointData> pd;
    pd.append(KPathPointData(&path1, path1.pathPointIndex(point11)));
    pd.append(KPathPointData(&path1, path1.pathPointIndex(point12)));
    pd.append(KPathPointData(&path1, path1.pathPointIndex(point13)));
    pd.append(KPathPointData(&path1, path1.pathPointIndex(point14)));
    pd.append(KPathPointData(&path1, path1.pathPointIndex(point15)));

    QPainterPath ppath1Org = path1.outline();

    MockShapeController mockController;
    KShapeController shapeController(0, &mockController);

    QUndoCommand *cmd1 = KPathPointRemoveCommand::createCommand(pd, &shapeController);
    cmd1->redo();

    QPainterPath ppath1(QPointF(0, 0));
    ppath1.cubicTo(50, 0, 100, 50, 100, 100);
    ppath1.cubicTo(50, 100, 0, 50, 0, 0);
    ppath1.closeSubpath();
    ppath1.moveTo(100, 0);
    ppath1.lineTo(100, 100);

    QPainterPath ppath1mod = path1.outline();
    QVERIFY(ppath1 == ppath1mod);

    QList<KPathPointData> pd2;
    pd2.append(KPathPointData(&path1, path1.pathPointIndex(point21)));
    pd2.append(KPathPointData(&path1, path1.pathPointIndex(point22)));

    QUndoCommand *cmd2 = KPathPointRemoveCommand::createCommand(pd2, &shapeController);
    cmd2->redo();

    QPainterPath ppath2(QPointF(0, 0));
    ppath2.lineTo(0, 100);

    QVERIFY(ppath2 == path1.outline());

    cmd2->undo();

    QVERIFY(ppath1mod == path1.outline());

    cmd1->undo();

    QVERIFY(ppath1Org == path1.outline());

    delete cmd2;
    delete cmd1;
}

void TestPointRemoveCommand::redoUndoShapeRemove()
{
    KPathShape *path1 = new KPathShape();
    KPathPoint *point11 = path1->moveTo(QPointF(0, 0));
    KPathPoint *point12 = path1->lineTo(QPointF(0, 100));
    KPathPoint *point13 = path1->curveTo(QPointF(0, 50), QPointF(100, 50), QPointF(100, 100));
    KPathPoint *point14 = path1->lineTo(QPointF(200, 100));
    KPathPoint *point15 = path1->curveTo(QPointF(200, 50), QPointF(300, 50), QPointF(300, 100));
    KPathShape *path2 = new KPathShape();
    KPathPoint *point21 = path2->moveTo(QPointF(0, 0));
    KPathPoint *point22 = path2->curveTo(QPointF(50, 0), QPointF(100, 50), QPointF(100, 100));
    path2->curveTo(QPointF(50, 100), QPointF(0, 50), QPointF(0, 0));
    path2->closeMerge();

    QList<KPathPointData> pd;
    pd.append(KPathPointData(path1, path1->pathPointIndex(point12)));
    pd.append(KPathPointData(path1, path1->pathPointIndex(point11)));
    pd.append(KPathPointData(path1, path1->pathPointIndex(point13)));
    pd.append(KPathPointData(path1, path1->pathPointIndex(point15)));
    pd.append(KPathPointData(path1, path1->pathPointIndex(point14)));
    pd.append(KPathPointData(path2, path2->pathPointIndex(point22)));
    pd.append(KPathPointData(path2, path2->pathPointIndex(point21)));

    QPainterPath ppath1Org = path1->outline();
    QPainterPath ppath2Org = path2->outline();

    MockShapeController mockController;
    mockController.addShape(path1);
    mockController.addShape(path2);
    KShapeController shapeController(0, &mockController);

    QUndoCommand *cmd = KPathPointRemoveCommand::createCommand(pd, &shapeController);
    cmd->redo();
    QVERIFY(!mockController.contains(path1));
    QVERIFY(!mockController.contains(path2));
    cmd->undo();

    QVERIFY(mockController.contains(path1));
    QVERIFY(mockController.contains(path2));

    QVERIFY(ppath1Org == path1->outline());
    QVERIFY(ppath2Org == path2->outline());

    delete cmd;
    delete path1;
    delete path2;
}

void TestPointRemoveCommand::redoUndo()
{
    KPathShape *path1 = new KPathShape();
    KPathPoint *point11 = path1->moveTo(QPointF(0, 0));
    KPathPoint *point12 = path1->lineTo(QPointF(0, 100));
    KPathPoint *point13 = path1->curveTo(QPointF(0, 50), QPointF(100, 50), QPointF(100, 100));
    KPathPoint *point14 = path1->lineTo(QPointF(200, 100));
    KPathPoint *point15 = path1->curveTo(QPointF(200, 50), QPointF(300, 50), QPointF(300, 100));
    KPathPoint *point16 = path1->moveTo(QPointF(100, 0));
    KPathPoint *point17 = path1->curveTo(QPointF(150, 0), QPointF(200, 50), QPointF(200, 100));
    path1->curveTo(QPointF(150, 100), QPointF(100, 50), QPointF(100, 0));
    path1->closeMerge();
    KPathPoint *point18 = path1->moveTo(QPointF(200, 0));
    KPathPoint *point19 = path1->lineTo(QPointF(200, 100));

    KPathShape *path2 = new KPathShape();
    KPathPoint *point21 = path2->moveTo(QPointF(0, 0));
    KPathPoint *point22 = path2->curveTo(QPointF(50, 0), QPointF(100, 50), QPointF(100, 100));
    path2->curveTo(QPointF(50, 100), QPointF(0, 50), QPointF(0, 0));
    path2->closeMerge();

    KPathShape *path3 = new KPathShape();
    KPathPoint *point31 = path3->moveTo(QPointF(0, 0));
    KPathPoint *point32 = path3->lineTo(QPointF(100, 100));
    KPathPoint *point33 = path3->lineTo(QPointF(200, 150));

    MockShapeController mockController;
    mockController.addShape(path1);
    mockController.addShape(path2);
    mockController.addShape(path3);

    QList<KPathPointData> pd;
    pd.append(KPathPointData(path2, path2->pathPointIndex(point21)));
    pd.append(KPathPointData(path1, path1->pathPointIndex(point13)));
    pd.append(KPathPointData(path1, path1->pathPointIndex(point11)));
    pd.append(KPathPointData(path3, path3->pathPointIndex(point31)));
    pd.append(KPathPointData(path1, path1->pathPointIndex(point12)));
    pd.append(KPathPointData(path1, path1->pathPointIndex(point15)));
    pd.append(KPathPointData(path2, path2->pathPointIndex(point22)));
    pd.append(KPathPointData(path1, path1->pathPointIndex(point14)));

    KShapeController shapeController(0, &mockController);

    QPainterPath ppath1Org = path1->outline();
    QPainterPath ppath2Org = path2->outline();
    QPainterPath ppath3Org = path3->outline();

    QUndoCommand *cmd1 = KPathPointRemoveCommand::createCommand(pd, &shapeController);
    cmd1->redo();

    QVERIFY(mockController.contains(path1));
    QVERIFY(!mockController.contains(path2));
    QVERIFY(mockController.contains(path3));

    QPainterPath ppath1(QPointF(0, 0));
    ppath1.cubicTo(50, 0, 100, 50, 100, 100);
    ppath1.cubicTo(50, 100, 0, 50, 0, 0);
    ppath1.closeSubpath();
    ppath1.moveTo(100, 0);
    ppath1.lineTo(100, 100);

    QPainterPath ppath1mod = path1->outline();
    QVERIFY(ppath1 == ppath1mod);

    QPainterPath ppath3(QPointF(0, 0));
    ppath3.lineTo(100, 50);

    QPainterPath ppath3mod = path3->outline();
    QVERIFY(ppath3 == ppath3mod);

    QList<KPathPointData> pd2;
    pd2.append(KPathPointData(path1, path1->pathPointIndex(point16)));
    pd2.append(KPathPointData(path1, path1->pathPointIndex(point17)));
    pd2.append(KPathPointData(path1, path1->pathPointIndex(point18)));
    pd2.append(KPathPointData(path1, path1->pathPointIndex(point19)));
    pd2.append(KPathPointData(path3, path3->pathPointIndex(point32)));
    pd2.append(KPathPointData(path3, path3->pathPointIndex(point33)));

    QUndoCommand *cmd2 = KPathPointRemoveCommand::createCommand(pd2, &shapeController);
    cmd2->redo();

    QVERIFY(!mockController.contains(path1));
    QVERIFY(!mockController.contains(path2));
    QVERIFY(!mockController.contains(path3));

    cmd2->undo();

    QVERIFY(mockController.contains(path1));
    QVERIFY(!mockController.contains(path2));
    QVERIFY(mockController.contains(path3));
    QVERIFY(ppath1 == ppath1mod);
    QVERIFY(ppath3 == ppath3mod);

    cmd1->undo();

    QVERIFY(ppath1Org == path1->outline());
    QVERIFY(ppath2Org == path2->outline());
    QVERIFY(ppath3Org == path3->outline());

    cmd1->redo();
    cmd2->redo();

    delete cmd2;
    delete cmd1;
}

QTEST_MAIN(TestPointRemoveCommand)
#include "TestPointRemoveCommand.moc"
