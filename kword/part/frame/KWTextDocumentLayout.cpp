/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#include "KWTextDocumentLayout.h"
#include "KWTextFrameSet.h"
#include "KWTextFrame.h"

#include <KoTextShapeData.h>

#include <kdebug.h>
#include <QTextBlock>
#include <QTextFragment>
#include <QTextLayout>
#include <QTextLine>
#include <QPainter>
#include <QPaintDevice>

#include <limits.h>

class PostscriptPaintDevice : public QPaintDevice {
public:
    PostscriptPaintDevice() {
    }
//   int devType () const {
//   }
    QPaintEngine *paintEngine () const {
        return 0;
    }
    int metric (PaintDeviceMetric metric) const {
        switch(metric) {
            case QPaintDevice::PdmWidth:
            case QPaintDevice::PdmHeight:
            case QPaintDevice::PdmWidthMM:
            case QPaintDevice::PdmHeightMM:
            case QPaintDevice::PdmNumColors:
                return INT_MAX;
            case QPaintDevice::PdmDepth:
                return 32;
            case QPaintDevice::PdmDpiX:
            case QPaintDevice::PdmDpiY:
            case QPaintDevice::PdmPhysicalDpiX:
            case QPaintDevice::PdmPhysicalDpiY:
                return 72;
        }
    }
};


KWTextDocumentLayout::KWTextDocumentLayout(KWTextFrameSet *frameSet)
: QAbstractTextDocumentLayout(frameSet->document())
, m_frameSet(frameSet)
{
    setPaintDevice( new PostscriptPaintDevice() );
}

KWTextDocumentLayout::~KWTextDocumentLayout() {
}

QRectF KWTextDocumentLayout::blockBoundingRect(const QTextBlock &block) const {
// TODO
kDebug() << "KWTextDocumentLayout::blockBoundingRect"<< endl;
    return QRectF(0, 0, 10, 10);
}

QSizeF KWTextDocumentLayout::documentSize() const {
// TODO
kDebug() << "KWTextDocumentLayout::documentSize"<< endl;
    return QSizeF(10, 10);
}

void KWTextDocumentLayout::draw(QPainter *painter, const PaintContext &context) {
    if(document()->begin().layout()->lineCount() == 0) // only first time
        layout();
    const QRegion clipRegion = painter->clipRegion();
    painter->setBrush(QBrush(Qt::black));
    painter->setPen(QPen(Qt::black));
    // da real work
    QTextBlock block = document()->begin();
    while(block.isValid()) {
        QTextLayout *layout = block.layout();
        if(!painter->hasClipping() || ! clipRegion.intersect(QRegion(layout->boundingRect().toRect())).isEmpty())
            layout->draw(painter, QPointF(0,0));
        block = block.next();
    }
}

QRectF KWTextDocumentLayout::frameBoundingRect(QTextFrame *frame) const {
// TODO
kDebug() << "KWTextDocumentLayout::frameBoundingRect"<< endl;
    return QRectF(0, 0, 10, 10);
}

int KWTextDocumentLayout::hitTest(const QPointF &point, Qt::HitTestAccuracy accuracy) const {
    // kDebug() << "hitTest[" << point.x() << "," << point.y() << "]" << endl;
    QTextBlock block = document()->begin();
    int position = -1;
    while(block.isValid()) {
        QTextLayout *layout = block.layout();
        if(point.y() > layout->boundingRect().bottom()) {
            position = block.position() + block.length();
            block = block.next();
            continue;
        }
        for(int i=0; i < layout->lineCount(); i++) {
            QTextLine line = layout->lineAt(i);
            // kDebug() << " + line[" << line.textStart() << "]: " << line.y() << "-" << line.height() << endl;
            if(point.y() > line.y() + line.height()) {
                position = line.textStart() + line.textLength();
                continue;
            }
            if(accuracy == Qt::ExactHit && point.y() < line.y()) // between lines
                return -1;
            if(accuracy == Qt::ExactHit && // left or right of line
                    (point.x() < line.x() || point.x() > line.x() + line.width()))
                return -1;
            return block.position() + line.xToCursor(point.x());
        }
        block = block.next();
    }
    if(accuracy == Qt::ExactHit)
        return -1;
    return position;
}

int KWTextDocumentLayout::pageCount () const {
kDebug() << "KWTextDocumentLayout::pageCount"<< endl;
    return 1;
}

void KWTextDocumentLayout::documentChanged(int position, int charsRemoved, int charsAdded) {
    if(m_frameSet->frameCount() == 0) // nothing to do.
        return;
    m_frameSet->requestLayout();

    int end = position + qMax(charsAdded, charsRemoved);
    foreach(KWFrame *frame, m_frameSet->frames()) {
        if(frame->isCopy())
            continue;
        KoTextShapeData *data = dynamic_cast<KoTextShapeData*> (frame->shape()->userData());
        if(data->position() <= position && data->endPosition() >= position) {
            // found our (first) frame
            data->faul();
            return;
        }
    }
    // if still here; then the change was not in any frame, lets relayout the last for now.
    KWFrame *last = m_frameSet->frames().last();
    KoTextShapeData *data = dynamic_cast<KoTextShapeData*> (last->shape()->userData());
    data->faul();
}

void KWTextDocumentLayout::layout() {
//kDebug() << "KWTextDocumentLayout::layout" << endl;
    class State {
      public:
        State(KWTextFrameSet *fs) : m_frameSet(fs) {
            m_y = 0;
            layout = 0;
            frameNumber = -1;
            nextFrame();
            m_block = m_frameSet->document()->begin();
            nextParag();
        }

        double width() {
            double ptWidth = shape->size().width() - m_format.leftMargin() - m_format.rightMargin();
            return ptWidth;
        }

        double x() {
            return m_format.leftMargin();
        }

        double y() {
            return m_y;
        }

        /// when a line is added, update internal vars.  Return true if line does not fit in shape
        bool addLine(const QTextLine &line) {
            m_newShape = false;
            if(m_fragmentIterator.atEnd())
                return false; // no text in block.
            double height=0.0;
//kDebug() << "a: " << m_fragmentIterator.fragment().position() << "+" << m_fragmentIterator.fragment().length() << ", b:" << line.textStart() << "+" << line.textLength() << endl;
            height = qMax(height, m_fragmentIterator.fragment().charFormat().fontPointSize());
            while(! (m_fragmentIterator.atEnd() || m_fragmentIterator.fragment().contains(
                           m_block.position() + line.textStart() + line.textLength() -1))) {
                m_fragmentIterator++;
                height = qMax(height, m_fragmentIterator.fragment().charFormat().fontPointSize());
//kDebug() << "   next fragment\n";
            }
            if(height < 0.01) height = 12; // default size for uninitialized styles.
//kDebug() << "   h: " << height << endl;
            if(m_data->documentOffset() + shape->size().height() < m_y + height) {
//kDebug() << "   NEXT frame" << endl;
                // line does not fit.
                m_data->setEndPosition(line.textStart()-1);
                m_y = m_data->documentOffset() + shape->size().height() + 10.0;
                m_data->wipe();
                nextFrame();
                if(m_data) {
                    m_data->setDocumentOffset(m_y);
                    m_data->setPosition(line.textStart());
                }
                return true;
            }
            m_y += height * 1.2;
            return false;
        }

        bool nextParag() {
            if(layout != 0) { // guard against first time
                layout->endLayout();
                m_block = m_block.next();
                if(!m_newShape) { // ignore bottomMargin when we went to a new shape anyway.
                    m_y += m_format.bottomMargin();
                }
            }
            if(! m_block.isValid())
                return false;
            m_format = m_block.blockFormat();
            if(! m_newShape) { // ignore margins at top of shape
                m_y += m_format.topMargin();
            }
            layout = m_block.layout();
            layout->beginLayout();
            m_fragmentIterator = m_block.begin();
            return true;
        }

        int frameNumber;
        KoShape *shape;
        QTextLayout *layout;

      private:
        void nextFrame() {
            frameNumber++;
            if(frameNumber >= m_frameSet->frameCount()) {
                shape = 0;
                m_data = 0;
                return;
            }
            shape = m_frameSet->frames()[frameNumber]->shape();
            m_data = dynamic_cast<KoTextShapeData*> (shape->userData());
            m_newShape = true;
        }

      private:
        KWTextFrameSet *m_frameSet;

        double m_y;
        QTextBlock m_block;
        QTextBlockFormat m_format;
        QTextBlock::Iterator m_fragmentIterator;
        KoTextShapeData *m_data;
        bool m_newShape;
    };

    State state(m_frameSet);
    if(m_frameSet->frameCount() <= state.frameNumber)
        return;
    while(state.shape) {
        QTextLine line = state.layout->createLine();
        if (!line.isValid()) { // end of parag
            if(! state.nextParag())
                return;
            continue;
        }

        line.setLineWidth(state.width());
//kDebug() << "New line at point " << state.x() << "," << state.y() << endl;
        line.setPosition(QPointF(state.x(), state.y()));
//kDebug() << "         reading: " << line.position().x() << "," << line.position().y() << endl;
        while(state.addLine(line)) {
            if(state.shape == 0) { // no more shapes to put the text in!
                line.setPosition(QPointF(0, state.y()+20));
                return;
            }
            line.setLineWidth(state.width());
            line.setPosition(QPointF(state.x(), state.y()));
        }
//kDebug() << "Line has " << line.textLength() << " chars" << endl;
//kDebug() << "     width: " << line.width() << ", height: " << line.height() << endl;
    }
}

/*
void KWTextDocumentLayout::layout() {
    double offset = 0.0;
    double previousOffset = offset;
    bool reLayout = false;
    foreach(KWFrame *frame, m_frameSet->frames()) {
kDebug() << "frame [" << offset << "]"  << frame->shape()->position().x() << "," << frame->shape()->position().y() << endl;
        KoTextShapeData *data = dynamic_cast<KoTextShapeData*> (frame->shape()->userData());
        Q_ASSERT(data); // only TextShapes are allowed on a KWTextFrameSet
        if(reLayout || data->isDirty())
            // relayout all frames from here.
            reLayout = true;
kDebug() << "  reLayout: " << reLayout << endl;
        //if(reLayout) {
            frame->shape()->repaint();
            if(frame->isCopy()) {
                data->setDocumentOffset(previousOffset);
                data->wipe();
                continue;
            }
            data->setDocumentOffset(offset);
            reLayout = layout(static_cast<KWTextFrame*> (frame), offset);
            data->wipe();
        //}
        previousOffset = offset;
        offset += frame->shape()->size().height() + 10;
    }
}
*/
/*
bool KWTextDocumentLayout::layout(KWTextFrame *frame, double offset) {
kDebug() << "KWTextDocumentLayout::layout TxtFrame " << offset << endl;
    bool firstParag = true;
    const double bottom = offset + frame->shape()->size().height();

    // find first block to layout
    QTextBlock block = document()->begin();
    while(block.isValid()) {
        if(block.length() > 0 && block.layout()->lineCount() == 0)
            // not marked up yet, the boundingRect will return 0x0
            break;
        if(offset <= block.layout()->boundingRect().bottom())
            break;
        block = block.next();
    }

    KoTextShapeData *data = dynamic_cast<KoTextShapeData*> (frame->shape()->userData());
//    if(!data->isDirty() && data->documentOffset() == offset)
//        return false;
    data->setPosition(block.position());

    // do layout
    bool started=false;
    while(1) {
        kDebug() << "   layout block" << endl;
        if(!block.isValid())
            break;
        int x=0, width = frame->shape()->size().width();
        QTextBlockFormat blockFormat = block.blockFormat();
        if(blockFormat.isValid()) {
            if(!firstParag)
                offset += blockFormat.topMargin();
            x = blockFormat.leftMargin();
            width -= blockFormat.leftMargin() + blockFormat.rightMargin();
            width = qMax(width, 20);
        }
        QTextLayout *layout = block.layout();
        layout->beginLayout();
        QFontMetricsF fontMetrics(block.charFormat().font());
        int position = block.position();
        while(1) {
            kDebug() << "     new line" << endl;
            if(offset + fontMetrics.lineSpacing() > bottom) {
                layout->endLayout();
                data->setEndPosition(position);
                return true;
            }
            QTextLine line = layout->createLine();
            if (!line.isValid())
                break;
kDebug() << "width: " << width << endl;
            line.setLineWidth(width);
            offset += fontMetrics.leading();
            line.setPosition(QPoint(x, offset));
            offset += fontMetrics.height();
            position += line.textLength();
        }
        layout->endLayout();
        block = block.next();
        firstParag = false;
    }
}

void KWTextDocumentLayout::layout(QTextLayout &layout, bool recalc) {
    if(!recalc && layout.lineCount() > 0)
        return;
    layout.setCacheEnabled(true);
    layout.beginLayout();
    QTextLine line = layout.createLine();
double y=10.0;
    while(line.isValid()) {
        line.setLineWidth(200.0);
        line.setPosition(QPointF(0, y));
        y+= 20;
        line = layout.createLine();
    }
    layout.endLayout();
} */
