#include "TestTextFrameSorting.h"
#include "KWPageManager.h"
#include "KWPage.h"
#include "KWTextFrameSet.h"
#include "KWTextFrame.h"

#include <KoTextShapeData.h>
#include <MockShapes.h>
#include <kcomponentdata.h>

TestTextFrameSorting::TestTextFrameSorting() {
    new KComponentData("TestTextFrameSorting");
}

void TestTextFrameSorting::testSimpleSorting() {
    KWTextFrameSet tfs(0);
    KWFrame * frame1 = createFrame(QPointF(10, 10), tfs);
    KWFrame * frame2 = createFrame(QPointF(120, 10), tfs);
    KWFrame * frame3 = createFrame(QPointF(10, 110), tfs);

    qSort(tfs.m_frames.begin(), tfs.m_frames.end(), KWTextFrameSet::sortTextFrames);

    QCOMPARE(tfs.m_frames[0], frame1);
    QCOMPARE(tfs.m_frames[1], frame3);
    QCOMPARE(tfs.m_frames[2], frame2);
}

void TestTextFrameSorting::testSortingOnPagespread() {
    KWTextFrameSet tfs(0);
    KWFrame * frame1 = createFrame(QPointF(10, 200), tfs);
    KWFrame * frame2 = createFrame(QPointF(120, 10), tfs);
    KWFrame * frame3 = createFrame(QPointF(10, 10), tfs);
    KWFrame * frame4 = createFrame(QPointF(340, 10), tfs);
    KWFrame * frame5 = createFrame(QPointF(230, 10), tfs);

    KWPageManager pm;
    pm.setStartPage(2);
    KWPage *page = pm.appendPage();
    page->setWidth(450);
    page->setHeight(150);
    page->setDirectionHint(KoText::LeftRightTopBottom);
    page->setPageSide(KWPage::PageSpread);
    page = pm.appendPage();
    page->setDirectionHint(KoText::LeftRightTopBottom);
    page->setWidth(200);
    tfs.setPageManager(&pm);

    // test KWPageManager::pageNumber first
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

void TestTextFrameSorting::testRtlSorting() {
    QEXPECT_FAIL("", "rest of functionality not implemented yet", Abort);
    KWTextFrameSet tfs(0);
    KWFrame * frame1 = createFrame(QPointF(10, 10), tfs);
    KWFrame * frame2 = createFrame(QPointF(120, 10), tfs);

    KWPageManager pm;
    KWPage *page = pm.appendPage();
    page->setDirectionHint(KoText::RightLeftTopBottom);
    page->setWidth(200);

    qSort(tfs.m_frames.begin(), tfs.m_frames.end(), KWTextFrameSet::sortTextFrames);

    // in RTL columns also sort right-to-left
    QCOMPARE(tfs.m_frames[0], frame2);
    QCOMPARE(tfs.m_frames[1], frame1);
}

KWFrame * TestTextFrameSorting::createFrame(const QPointF &position, KWTextFrameSet &fs) {
    MockShape *shape = new MockShape();
    shape->setUserData(new KoTextShapeData());
    KWTextFrame *frame = new KWTextFrame(shape, &fs);
    shape->setPosition(position);
    return frame;
}


QTEST_MAIN(TestTextFrameSorting)
#include "TestTextFrameSorting.moc"
