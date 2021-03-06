/* This file is part of the KDE project
   Copyright 2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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

#include "KCNamedAreaCommand.h"

#include "klocale.h"

#include "Damages.h"
#include "KCFormulaStorage.h"
#include "kcells_limits.h"
#include "KCMap.h"
#include "KCNamedAreaManager.h"
#include "KCSheet.h"

KCNamedAreaCommand::KCNamedAreaCommand(QUndoCommand* parent)
        : KCAbstractRegionCommand(parent)
{
    setText(i18n("Add Named Area"));
}

KCNamedAreaCommand::~KCNamedAreaCommand()
{
}

void KCNamedAreaCommand::setAreaName(const QString& name)
{
    m_areaName = name;
}

void KCNamedAreaCommand::setReverse(bool reverse)
{
    KCAbstractRegionCommand::setReverse(reverse);
    if (!m_reverse)
        setText(i18n("Add Named Area"));
    else
        setText(i18n("Remove Named Area"));
}

bool KCNamedAreaCommand::preProcessing()
{
    if (!m_firstrun)
        return true;
    if (m_reverse)
        return true;

    const KCRegion namedArea = m_sheet->map()->namedAreaManager()->namedArea(m_areaName);
    if (!namedArea.isEmpty()) {
        if (namedArea == *this)
            return false;
        m_oldArea = namedArea;
    }
    // no protection or matrix lock check needed
    return isContiguous();
}

bool KCNamedAreaCommand::mainProcessing()
{
    kDebug() ;
    if (!m_reverse) {
        if (!m_oldArea.isEmpty())
            m_sheet->map()->namedAreaManager()->remove(m_areaName);
        m_sheet->map()->namedAreaManager()->insert(*this, m_areaName);
    } else {
        m_sheet->map()->namedAreaManager()->remove(m_areaName);
        if (!m_oldArea.isEmpty())
            m_sheet->map()->namedAreaManager()->insert(m_oldArea, m_areaName);
    }
    return true;
}

bool KCNamedAreaCommand::postProcessing()
{
    // update formulas containing either the new or the old name
    KCMap* const map = m_sheet->map();
    foreach(KCSheet* sheet, map->sheetList()) {
        const QString tmp = '\'' + m_areaName + '\'';
        const KCFormulaStorage* const storage = sheet->formulaStorage();
        for (int c = 0; c < storage->count(); ++c) {
            if (storage->data(c).expression().contains(tmp)) {
                KCCell cell(sheet, storage->col(c), storage->row(c));
                if (cell.makeFormula()) {
                    // recalculate cells
                    map->addDamage(new KCCellDamage(cell, KCCellDamage::Appearance | KCCellDamage::KCBinding |
                                                  KCCellDamage::KCValue));
                }
            }
        }
    }
    return KCAbstractRegionCommand::postProcessing();
}
