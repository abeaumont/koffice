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

#include "KCCSVDataCommand.h"

#include <klocale.h>

#include "KCCalculationSettings.h"
#include "KCMap.h"
#include "KCSheet.h"
#include "KCValue.h"
#include "KCValueConverter.h"

KCCSVDataCommand::KCCSVDataCommand()
        : KCAbstractDataManipulator()
{
}

KCCSVDataCommand::~KCCSVDataCommand()
{
}

void KCCSVDataCommand::setValue(const KCValue& value)
{
    m_value = value;
}

void KCCSVDataCommand::setColumnDataTypes(const QList<KoCsvImportDialog::DataType>& dataTypes)
{
    m_dataTypes = dataTypes;
}

void KCCSVDataCommand::setDecimalSymbol(const QString& symbol)
{
    m_decimalSymbol = symbol;
}

void KCCSVDataCommand::setThousandsSeparator(const QString& separator)
{
    m_thousandsSeparator = separator;
}

KCValue KCCSVDataCommand::newValue(Element* element, int col, int row, bool* parse, KCFormat::Type* fmtType)
{
    Q_UNUSED(fmtType)
    const int colidx = col - element->rect().left();
    const int rowidx = row - element->rect().top();

    KCValue value;
    switch (m_dataTypes.value(colidx)) {
    case KoCsvImportDialog::Generic:
        value = m_value.element(colidx, rowidx);
        *parse = true;
        break;
    case KoCsvImportDialog::Text:
        value = m_value.element(colidx, rowidx);
        break;
    case KoCsvImportDialog::Date:
        value = m_sheet->map()->converter()->asDate(m_value.element(colidx, rowidx));
        break;
    case KoCsvImportDialog::Currency:
        value = m_sheet->map()->converter()->asFloat(m_value.element(colidx, rowidx));
        value.setFormat(KCValue::fmt_Money);
        break;
    case KoCsvImportDialog::None:
        break;
    }
    return value;
}

bool KCCSVDataCommand::wantChange(Element* element, int col, int row)
{
    Q_UNUSED(row)
    return (m_dataTypes.value(col - element->rect().left()) != KoCsvImportDialog::None);
}

bool KCCSVDataCommand::preProcessing()
{
    if (!KCAbstractDataManipulator::preProcessing())
        return false;
    // Initialize the decimal symbol and thousands separator to use for parsing.
    m_documentDecimalSymbol = m_sheet->map()->calculationSettings()->locale()->decimalSymbol();
    m_documentThousandsSeparator = m_sheet->map()->calculationSettings()->locale()->thousandsSeparator();
    m_sheet->map()->calculationSettings()->locale()->setDecimalSymbol(m_decimalSymbol);
    m_sheet->map()->calculationSettings()->locale()->setThousandsSeparator(m_thousandsSeparator);
    return true;
}

bool KCCSVDataCommand::postProcessing()
{
    if (!KCAbstractDataManipulator::postProcessing())
        return false;
    // Restore the document's decimal symbol and thousands separator.
    m_sheet->map()->calculationSettings()->locale()->setDecimalSymbol(m_documentDecimalSymbol);
    m_sheet->map()->calculationSettings()->locale()->setThousandsSeparator(m_documentThousandsSeparator);
    m_documentDecimalSymbol.clear();
    m_documentThousandsSeparator.clear();
    return true;
}
