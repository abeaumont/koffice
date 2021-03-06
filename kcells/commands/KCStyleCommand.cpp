/* This file is part of the KDE project
   Copyright 2005-2006 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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
   Boston, MA 02110-1301, USA.
*/

// Local
#include "KCStyleCommand.h"

#include <QBrush>

#include <kdebug.h>
#include <klocale.h>

#include "KCCell.h"
#include "KCCellStorage.h"
#include "Damages.h"
#include "KCMap.h"
#include "KCSheet.h"
#include "KCStyle.h"
#include "KCStyleStorage.h"

KCStyleCommand::KCStyleCommand(QUndoCommand* parent)
        : KCAbstractRegionCommand(parent)
        , m_horizontalPen(QPen(QColor(), 0, Qt::NoPen))
        , m_verticalPen(QPen(QColor(), 0, Qt::NoPen))
        , m_horizontalPenChanged(false)
        , m_verticalPenChanged(false)
        , m_style(new KCStyle())
{
}

KCStyleCommand::~KCStyleCommand()
{
    delete m_style;
}

bool KCStyleCommand::process(Element* element)
{
    const QRect range = element->rect();
    if (!m_reverse) { // (re)do
        if (m_firstrun) {
            const QRect rect = range.adjusted(m_style->hasAttribute(KCStyle::LeftPen) ? -1 : 0,
                                              m_style->hasAttribute(KCStyle::TopPen) ? -1 : 0,
                                              m_style->hasAttribute(KCStyle::RightPen) ? 1 : 0,
                                              m_style->hasAttribute(KCStyle::BottomPen) ? 1 : 0);
            const QList< QPair<QRectF, KCSharedSubStyle> > rawUndoData
            = m_sheet->styleStorage()->undoData(KCRegion(rect));
            for (int i = 0; i < rawUndoData.count(); ++i) {
//                 if ( m_style->hasAttribute( rawUndoData[i].second->type() ) ||
//                      rawUndoData[i].second->type() == KCStyle::DefaultStyleKey ||
//                      rawUndoData[i].second->type() == KCStyle::NamedStyleKey )
                m_undoData << rawUndoData[i];
            }
        }
        // special handling for the border
        bool hasLeftPen = m_style->hasAttribute(KCStyle::LeftPen);
        bool hasRightPen = m_style->hasAttribute(KCStyle::RightPen);
        bool hasTopPen = m_style->hasAttribute(KCStyle::TopPen);
        bool hasBottomPen = m_style->hasAttribute(KCStyle::BottomPen);
        const QPen leftPen = m_style->leftBorderPen();
        const QPen rightPen = m_style->rightBorderPen();
        const QPen topPen = m_style->topBorderPen();
        const QPen bottomPen = m_style->bottomBorderPen();
        m_style->clearAttribute(KCStyle::LeftPen);
        m_style->clearAttribute(KCStyle::RightPen);
        m_style->clearAttribute(KCStyle::TopPen);
        m_style->clearAttribute(KCStyle::BottomPen);

        // use the horizontal/vertical pens
        if (m_horizontalPenChanged) {
            m_style->setTopBorderPen(m_horizontalPen);
            m_style->setBottomBorderPen(m_horizontalPen);
            hasTopPen = hasBottomPen = true;
        }
        if (m_verticalPenChanged) {
            m_style->setLeftBorderPen(m_verticalPen);
            m_style->setRightBorderPen(m_verticalPen);
            hasLeftPen = hasRightPen = true;
        }

        // special handling for indentation: reset the indentation first
        if (m_style->hasAttribute(KCStyle::Indentation)) {
            KCStyle style;
            style.setIndentation(0);
            m_sheet->cellStorage()->setStyle(KCRegion(range), style);
        }

        // special handling for precision: reset the precision first
        if (m_style->hasAttribute(KCStyle::Precision)) {
            KCStyle style;
            style.setPrecision(0);   // insert storage default
            m_sheet->cellStorage()->setStyle(KCRegion(range), style);
        }

        // set the actual style
        m_sheet->cellStorage()->setStyle(KCRegion(range), *m_style);

        // prepare to restore pens
        m_style->clearAttribute(KCStyle::LeftPen);
        m_style->clearAttribute(KCStyle::RightPen);
        m_style->clearAttribute(KCStyle::TopPen);
        m_style->clearAttribute(KCStyle::BottomPen);

        // set the outer border styles
        KCStyle style;
        if (hasLeftPen) {
            style.setLeftBorderPen(leftPen);
            m_sheet->cellStorage()->setStyle(KCRegion(QRect(range.left(), range.top(), 1, range.height())), style);
            m_style->setLeftBorderPen(leftPen); // restore pen
            // reset the border of the adjacent cell
            if (range.left() > 1) {
                KCStyle tmpStyle;
                tmpStyle.setRightBorderPen(QPen(Qt::NoPen));
                const KCRegion region(QRect(range.left() - 1, range.top(), 1, range.height()));
                m_sheet->cellStorage()->setStyle(region, tmpStyle);
                m_sheet->map()->addDamage(new KCCellDamage(m_sheet, region, KCCellDamage::Appearance));
            }
        }
        if (hasRightPen) {
            style.clear();
            style.setRightBorderPen(rightPen);
            m_sheet->cellStorage()->setStyle(KCRegion(QRect(range.right(), range.top(), 1, range.height())), style);
            m_style->setRightBorderPen(rightPen); // restore pen
            // reset the border of the adjacent cell
            if (range.right() < KS_colMax) {
                KCStyle tmpStyle;
                tmpStyle.setLeftBorderPen(QPen(Qt::NoPen));
                const KCRegion region(QRect(range.right() + 1, range.top(), 1, range.height()));
                m_sheet->cellStorage()->setStyle(region, tmpStyle);
                m_sheet->map()->addDamage(new KCCellDamage(m_sheet, region, KCCellDamage::Appearance));
            }
        }
        if (hasTopPen) {
            style.clear();
            style.setTopBorderPen(topPen);
            m_sheet->cellStorage()->setStyle(KCRegion(QRect(range.left(), range.top(), range.width(), 1)), style);
            m_style->setTopBorderPen(topPen); // restore pen
            // reset the border of the adjacent cell
            if (range.top() > 1) {
                KCStyle tmpStyle;
                tmpStyle.setBottomBorderPen(QPen(Qt::NoPen));
                const KCRegion region(QRect(range.left(), range.top() - 1, range.width(), 1));
                m_sheet->cellStorage()->setStyle(region, tmpStyle);
                m_sheet->map()->addDamage(new KCCellDamage(m_sheet, region, KCCellDamage::Appearance));
            }
        }
        if (hasBottomPen) {
            style.clear();
            style.setBottomBorderPen(bottomPen);
            m_sheet->cellStorage()->setStyle(KCRegion(QRect(range.left(), range.bottom(), range.width(), 1)), style);
            m_style->setBottomBorderPen(bottomPen); // restore pen
            // reset the border of the adjacent cell
            if (range.bottom() < KS_rowMax) {
                KCStyle tmpStyle;
                tmpStyle.setTopBorderPen(QPen(Qt::NoPen));
                const KCRegion region(QRect(range.left(), range.bottom() + 1, range.width(), 1));
                m_sheet->cellStorage()->setStyle(region, tmpStyle);
                m_sheet->map()->addDamage(new KCCellDamage(m_sheet, region, KCCellDamage::Appearance));
            }
        }
    }
    return true;
}

bool KCStyleCommand::preProcessing()
{
    if (m_firstrun) {
        if (m_style->isDefault())
            setText(i18n("Reset Style"));
        else
            setText(i18n("Change Style"));

        // special handling for precision
        if (m_style->hasAttribute(KCStyle::Precision)) {
            if (m_style->precision() == -1)   // KCStyle default
                m_style->setPrecision(0);   // storage default
            else if (m_style->precision() == 0)
                m_style->setPrecision(-1);   // anything resulting in zero, but not storage default
        }
    }
    return KCAbstractRegionCommand::preProcessing();
}

bool KCStyleCommand::mainProcessing()
{
    if (!m_reverse) {
    } else { // m_reverse
        KCStyle style;
        style.setDefault();
        // special handling for indentation
        style.setIndentation(0);   // reset to zero
        // special handling for precision
        style.setPrecision(0);   // reset to storage default
        m_sheet->cellStorage()->setStyle(*this, style);
        for (int i = 0; i < m_undoData.count(); ++i) {
            m_sheet->cellStorage()->insertSubStyle(m_undoData[i].first.toRect(), m_undoData[i].second);
        }
    }
    return KCAbstractRegionCommand::mainProcessing();
}

bool KCStyleCommand::postProcessing()
{
    return true;
}
