/*
 * This file is part of KOffice tests
 *
 * Copyright (C) 2005-2010 Thomas Zander <zander@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "TestTextFrameSorting.h"
#include "../KWPageManager.h"
#include "../KWPage.h"
#include "../frames/KWTextFrameSet.h"
#include "../frames/KWTextFrame.h"
#include "../KWDocument.h"
#include "../frames/KWCopyShape.h"

#include <KTextShapeData.h>
#include <MockShapes.h>
#include <kcomponentdata.h>

TestTextFrameSorting::TestTextFrameSorting()
{
    new KComponentData("TestTextFrameSorting");
}

void TestTextFrameSorting::testSimpleSorting()
{
    KWTextFrameSet tfs(0);
    KWFrame *frame1 = createFrame(QPointF(10, 10), tfs);
    KWFrame *frame2 = createFrame(QPointF(120, 10), tfs);
    frame2->shape()->setSize(QSizeF(100, 100));
    KWFrame *frame3 = createFrame(QPointF(10, 110), tfs);

    qSort(tfs.m_frames.begin(), tfs.m_frames.end(), KWTextFrameSet::sortTextFrames);

    QCOMPARE(tfs.m_frames[0], frame1);
    QCOMPARE(tfs.m_frames[1], frame3);
    QCOMPARE(tfs.m_frames[2], frame2);

    KWPageManager pm;
    tfs.setPageManager(&pm);
    pm.appendPage();
    pm.appendPage();
    KWFrame *frame4 = createFrame(QPointF(10, 1000), tfs); // page 2
    KWFrame *frame5 = createFrame(QPointF(120, 1000), tfs);

    qSort(tfs.m_frames.begin(), tfs.m_frames.end(), KWTextFrameSet::sortTextFrames);

    QCOMPARE(tfs.m_frames[0], frame1);
    QCOMPARE(tfs.m_frames[1], frame3);
    QCOMPARE(tfs.m_frames[2], frame2);
    QCOMPARE(tfs.m_frames[3], frame4);
    QCOMPARE(tfs.m_frames[4], frame5);
}

void TestTextFrameSorting::testSimpleSorting2() // cascading usecase
{
    KWTextFrameSet tfs(0);
    KWFrame *frame4 = createFrame(QPointF(120, 300), tfs); // note that each frame is 50 x 50 per default.
    KWFrame *frame2 = createFrame(QPointF(120, 120), tfs);
    KWFrame *frame3 = createFrame(QPointF(10, 240), tfs);
    KWFrame *frame1 = createFrame(QPointF(10, 10), tfs);

    qSort(tfs.m_frames.begin(), tfs.m_frames.end(), KWTextFrameSet::sortTextFrames);

    QCOMPARE(tfs.m_frames[0], frame1);
    QCOMPARE(tfs.m_frames[1], frame2);
    QCOMPARE(tfs.m_frames[2], frame3);
    QCOMPARE(tfs.m_frames[3], frame4);

    KWPageManager pm;
    tfs.setPageManager(&pm);
    pm.appendPage();
    pm.appendPage();
    KWFrame *frame21 = createFrame(QPointF(120, 1000), tfs);
    KWFrame *frame20 = createFrame(QPointF(10, 1000), tfs);
    KWFrame *frame23 = createFrame(QPointF(120, 1200), tfs);
    KWFrame *frame22 = createFrame(QPointF(10, 1110), tfs);

    qSort(tfs.m_frames.begin(), tfs.m_frames.end(), KWTextFrameSet::sortTextFrames);

    QCOMPARE(tfs.m_frames[0], frame1);
    QCOMPARE(tfs.m_frames[1], frame2);
    QCOMPARE(tfs.m_frames[2], frame3);
    QCOMPARE(tfs.m_frames[3], frame4);
    QCOMPARE(tfs.m_frames[4], frame20);
    QCOMPARE(tfs.m_frames[5], frame21);
    QCOMPARE(tfs.m_frames[6], frame22);
    QCOMPARE(tfs.m_frames[7], frame23);
}

void TestTextFrameSorting::testSortingOnPagespread()
{
    KWTextFrameSet tfs(0);
    KWFrame * frame1 = createFrame(QPointF(10, 200), tfs);
    KWFrame * frame2 = createFrame(QPointF(120, 10), tfs);
    KWFrame * frame3 = createFrame(QPointF(10, 10), tfs);
    KWFrame * frame4 = createFrame(QPointF(340, 10), tfs);
    KWFrame * frame5 = createFrame(QPointF(230, 10), tfs);

    KWPageManager pm;
    KWPage page = pm.appendPage();
    page.setPageNumber(2);
    KOdfPageLayoutData layout = page.pageStyle().pageLayout();
    layout.width = 450;
    layout.height = 150;
    page.setDirectionHint(KOdfText::LeftRightTopBottom);
    page.setPageSide(KWPage::PageSpread);
    page.pageStyle().setPageLayout(layout);
    page = pm.appendPage();
    page.setDirectionHint(KOdfText::LeftRightTopBottom);
    layout.width = 200;
    page.pageStyle().setPageLayout(layout);
    tfs.setPageManager(&pm);

    QCOMPARE(page.offsetInDocument(), 150.);

    // test KWPageManager::pageNumber first
    QCOMPARE(pm.begin().pageNumber(), 2);
    QCOMPARE(pm.pageNumber(frame2->shape()), 2);
    QCOMPARE(pm.pageNumber(frame1->shape()), 4);

    QCOMPARE(KWTextFrameSet::sortTextFrames(frame4, frame5), false);
    QCOMPARE(KWTextFrameSet::sortTextFrames(frame5, frame4), true);
    QCOMPARE(KWTextFrameSet::sortTextFrames(frame2, frame4), true);
    QCOMPARE(KWTextFrameSet::sortTextFrames(frame1, frame4), false);

    qSort(tfs.m_frames.begin(), tfs.m_frames.end(), KWTextFrameSet::sortTextFrames);

    QCOMPARE(tfs.m_frames[0], frame3);
    QCOMPARE(tfs.m_frames[1], frame2);
    QCOMPARE(tfs.m_frames[2], frame5);
    QCOMPARE(tfs.m_frames[3], frame4);
    QCOMPARE(tfs.m_frames[4], frame1);
}

void TestTextFrameSorting::testRtlSorting()
{
    KWTextFrameSet tfs(0);
    KWFrame * frame1 = createFrame(QPointF(10, 10), tfs);
    KWFrame * frame2 = createFrame(QPointF(120, 10), tfs);

    KWPageManager pm;
    KWPage page = pm.appendPage();
    page.setDirectionHint(KOdfText::RightLeftTopBottom);
    QCOMPARE(page.directionHint(), KOdfText::RightLeftTopBottom);
    KOdfPageLayoutData layout = page.pageStyle().pageLayout();
    layout.width = 200;
    page.pageStyle().setPageLayout(layout);
    tfs.setPageManager(&pm);

    qSort(tfs.m_frames.begin(), tfs.m_frames.end(), KWTextFrameSet::sortTextFrames);

    // in RTL columns also sort right-to-left
    QCOMPARE(tfs.m_frames[0], frame2);
    QCOMPARE(tfs.m_frames[1], frame1);
}

void TestTextFrameSorting::testSortingById()
{
    KWTextFrameSet tfs(0);
    KWTextFrame * frame1 = createFrame(QPointF(10, 200), tfs);
    KWTextFrame * frame2 = createFrame(QPointF(120, 10), tfs);
    KWTextFrame * frame3 = createFrame(QPointF(10, 10), tfs);

    frame1->setSortingId(1);
    frame2->setSortingId(2);
    frame3->setSortingId(3);

    qSort(tfs.m_frames.begin(), tfs.m_frames.end(), KWTextFrameSet::sortTextFrames);

    QCOMPARE(tfs.m_frames[0], frame1);
    QCOMPARE(tfs.m_frames[1], frame2);
    QCOMPARE(tfs.m_frames[2], frame3);
}

void TestTextFrameSorting::testCopyAfterTextSorting()
{
    KWTextFrameSet tfs(0);
    KWTextFrame * frame1 = createFrame(QPointF(70, 1300), tfs);
    KWFrame * frame2 = createCopyFrame(QPointF(70, 100), frame1->shape(), tfs);
    KWTextFrame * frame3 = createFrame(QPointF(70, 2000), tfs);

    tfs.removeFrame(frame1);

    QCOMPARE(tfs.m_frames[0], frame2);
    QCOMPARE(tfs.m_frames[1], frame3);

    qSort(tfs.m_frames.begin(), tfs.m_frames.end(), KWTextFrameSet::sortTextFrames);

    QCOMPARE(tfs.m_frames[0], frame3);
    QCOMPARE(tfs.m_frames[1], frame2);
}


KWTextFrame * TestTextFrameSorting::createFrame(const QPointF &position, KWTextFrameSet &fs)
{
    MockShape *shape = new MockShape();
    shape->setUserData(new KTextShapeData());
    KWTextFrame *frame = new KWTextFrame(shape, &fs);
    shape->setPosition(position);
    return frame;
}

KWFrame * TestTextFrameSorting::createCopyFrame(const QPointF &position, KShape *orig, KWTextFrameSet &fs)
{
    KShape *shape = new KWCopyShape(orig);
    KWFrame *frame = new KWFrame(shape, &fs);
    shape->setPosition(position);
    return frame;
}


QTEST_KDEMAIN(TestTextFrameSorting, GUI)

#include <TestTextFrameSorting.moc>
