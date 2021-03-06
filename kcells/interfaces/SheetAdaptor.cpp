/* This file is part of the KDE project

   Copyright 2006 Fredrik Edemar <f_edemar@linux.se>
   Copyright 2003 Lukas Tinkl <lukas@kde.org>
   Copyright 2000, 2002-2003 Laurent Montel <montel@kde.org>
   Copyright 2001-2003 Philipp Mueller <philipp.mueller@gmx.de>
   Copyright 2003 Joseph Wenninger <jowenn@kde.org>
   Copyright 2002 Ariya Hidayat <ariya@kde.org>
   Copyright 2002 Harri Porten <porten@kde.org>
   Copyright 2002 John Dailey <dailey@vt.edu>
   Copyright 2001 Simon Hausmann <hausmann@kde.org>
   Copyright 2000 Werner Trobin <trobin@kde.org>
   Copyright 1999 Torben Weis <weis@kde.org>

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
#include "SheetAdaptor.h"

#include <kdebug.h>

#include "Damages.h"
#include "KCMap.h"
#include "KCPrintSettings.h"
#include "KCSheet.h"
#include "KCSheetPrint.h"
#include "KCRegion.h"
#include "KCValueConverter.h"

// commands
#include "commands/DataManipulators.h"
#include "commands/RowColumnManipulators.h"

SheetAdaptor::SheetAdaptor(KCSheet* t)
        : QDBusAbstractAdaptor(t)
{
    setAutoRelaySignals(false);
    m_sheet = t;
    connect(m_sheet->map(), SIGNAL(damagesFlushed(const QList<KCDamage*> &)),
            this, SLOT(handleDamages(const QList<KCDamage*> &)));
}

SheetAdaptor::~SheetAdaptor()
{
}

QString SheetAdaptor::cellName(int x, int y)
{
    /* sebsauer 20061206: commented out cause if someone starts to believe that
    counting starts by 0,0 then he may very surprised why the first col/row
    got returned twice. That happened to me and I was believing that's a KCells
    bug too :-(
    // if someone calls us with either x or y 0 he _most_ most likely doesn't
    // know that the cell counting starts with 1 (Simon)
    // P.S.: I did that mistake for weeks and already started looking for the
    // "bug" in kcells ;-)
    if ( x == 0 ) x = 1;
    if ( y == 0 ) y = 1;
    */
    return KCCell::name(x, y);
}

int SheetAdaptor::cellRow(const QString& cellname)
{
    return cellLocation(cellname).x();
}

int SheetAdaptor::cellColumn(const QString& cellname)
{
    return cellLocation(cellname).y();
}

QPoint SheetAdaptor::cellLocation(const QString& cellname)
{
    const KCRegion region(cellname, m_sheet->map(), m_sheet);
    if (region.firstRange().isNull())
        return QPoint();
    return region.firstRange().topLeft();
}

QString SheetAdaptor::text(int x, int y)
{
    KCCell cell = KCCell(m_sheet, x, y);
    return cell.userInput();
}

QString SheetAdaptor::text(const QString& cellname)
{
    const QPoint location = cellLocation(cellname);
    return text(location.x(), location.y());
}

bool SheetAdaptor::setText(int x, int y, const QString& text, bool parse)
{
    //FIXME: there is some problem with asString parameter, when it's set
    //to true KCells says: ASSERT: "f" in Dependencies.cpp (621)
    //kcells: KCCell at row 6, col 1 marked as formula, but formula is NULL

    DataManipulator *dm = new DataManipulator();
    dm->setSheet(m_sheet);
    dm->setValue(KCValue(text));
    dm->setParsing(parse);
    dm->add(QPoint(x, y));
    return dm->execute();
}

bool SheetAdaptor::setText(const QString& cellname, const QString& text, bool parse)
{
    const QPoint location = cellLocation(cellname);
    return setText(location.x(), location.y(), text, parse);
}

QVariant valueToVariant(const KCValue& value, KCSheet* sheet)
{
    //Should we use following value-format enums here?
    //fmt_None, fmt_Boolean, fmt_Number, fmt_Percent, fmt_Money, fmt_DateTime, fmt_Date, fmt_Time, fmt_String
    switch (value.type()) {
    case KCValue::Empty:
        return QVariant();
    case KCValue::Boolean:
        return QVariant(value.asBoolean());
    case KCValue::Integer:
        return static_cast<qint64>(value.asInteger());
    case KCValue::Float:
        return (double) numToDouble(value.asFloat());
    case KCValue::Complex:
        return sheet->map()->converter()->asString(value).asString();
    case KCValue::String:
        return value.asString();
    case KCValue::Array: {
        QVariantList colarray;
        for (uint j = 0; j < value.rows(); j++) {
            QVariantList rowarray;
            for (uint i = 0; i < value.columns(); i++) {
                KCValue v = value.element(i, j);
                rowarray.append(valueToVariant(v, sheet));
            }
            colarray.append(rowarray);
        }
        return colarray;
    }
    break;
    case KCValue::CellRange:
        //FIXME: not yet used
        return QVariant();
    case KCValue::Error:
        return QVariant();
    }
    return QVariant();
}

QVariant SheetAdaptor::value(int x, int y)
{
    KCCell cell = KCCell(m_sheet, x, y);
    return valueToVariant(cell.value(), m_sheet);
}

QVariant SheetAdaptor::value(const QString& cellname)
{
    const QPoint location = cellLocation(cellname);
    return value(location.x(), location.y());
}

bool SheetAdaptor::setValue(int x, int y, const QVariant& value)
{
    KCCell cell = KCCell(m_sheet, x, y);
    if (! cell) return false;
    KCValue v = cell.value();
    switch (value.type()) {
    case QVariant::Bool: v = KCValue(value.toBool()); break;
    case QVariant::ULongLong: v = KCValue(value.toLongLong()); break;
    case QVariant::Int: v = KCValue(value.toInt()); break;
    case QVariant::Double: v = KCValue(value.toDouble()); break;
    case QVariant::String: v = KCValue(value.toString()); break;
        //case QVariant::Date: v = KCValue( value.toDate() ); break;
        //case QVariant::Time: v = KCValue( value.toTime() ); break;
        //case QVariant::DateTime: v = KCValue( value.toDateTime() ); break;
    default: return false;
    }
    return true;
}

bool SheetAdaptor::setValue(const QString& cellname, const QVariant& value)
{
    const QPoint location = cellLocation(cellname);
    return setValue(location.x(), location.y(), value);
}

#if 0
QString SheetAdaptor::column(int _col)
{
    //First col number = 1
    if (_col < 1)
        return QString();
    return "";//m_sheet->nonDefaultColumnFormat( _col )->getName/*objectName*/();/*dcopObject()->objId()*/

}

QString SheetAdaptor::row(int _row)
{
    //First row number = 1
    if (_row < 1)
        return QString();
    return "";//m_sheet->nonDefaultRowFormat( _row )->/*dcopObject()->*/getName/*objectName*/();
}
#endif

QString SheetAdaptor::sheetName() const
{
    return m_sheet->sheetName();
}

bool SheetAdaptor::setSheetName(const QString & name)
{
    return m_sheet->setSheetName(name);
}

int SheetAdaptor::lastColumn() const
{
    return m_sheet->usedArea().width();
}

int SheetAdaptor::lastRow() const
{
    return m_sheet->usedArea().height();
}

// bool SheetAdaptor::processDynamic( const DCOPCString& fun, const QByteArray&/*data*/,
//                                         DCOPCString& replyType, QByteArray &replyData )
// {
//     kDebug(36001) <<"Calling '" << fun.data() << '\'';
//     // Does the name follow the pattern "foobar()" ?
//     uint len = fun.length();
//     if ( len < 3 )
//         return false;
//
//     if ( fun[ len - 1 ] != ')' || fun[ len - 2 ] != '(' )
//         return false;
//
//     // Is the function name a valid cell like "B5" ?
//     Point p( fun.left( len - 2 ).data() );
//     if ( !p.isValid() )
//         return false;
//
//     DCOPCString str = objId() + '/' + fun.left( len - 2 );
//
//     replyType = "DCOPRef";
//     QDataStream out( &replyData,QIODevice::WriteOnly );
//     out.setVersion(QDataStream::Qt_3_1);
//     out << DCOPRef( kapp->dcopClient()->appId(), str );
//     return true;
// }

void SheetAdaptor::insertColumn(int col, int nbCol)
{
    InsertDeleteColumnManipulator* manipulator = new InsertDeleteColumnManipulator();
    manipulator->setSheet(m_sheet);
    manipulator->add(KCRegion(QRect(col, 1, nbCol, 1)));
    manipulator->execute();
}

void SheetAdaptor::insertRow(int row, int nbRow)
{
    InsertDeleteRowManipulator* manipulator = new InsertDeleteRowManipulator();
    manipulator->setSheet(m_sheet);
    manipulator->add(KCRegion(QRect(1, row, 1, nbRow)));
    manipulator->execute();
}

void SheetAdaptor::removeColumn(int col, int nbCol)
{
    InsertDeleteColumnManipulator* manipulator = new InsertDeleteColumnManipulator();
    manipulator->setSheet(m_sheet);
    manipulator->setReverse(true);
    manipulator->add(KCRegion(QRect(col, 1, nbCol, 1)));
    manipulator->execute();
}

void SheetAdaptor::removeRow(int row, int nbRow)
{
    InsertDeleteRowManipulator* manipulator = new InsertDeleteRowManipulator();
    manipulator->setSheet(m_sheet);
    manipulator->setReverse(true);
    manipulator->add(KCRegion(QRect(1, row, 1, nbRow)));
    manipulator->execute();
}

bool SheetAdaptor::isHidden() const
{
    return m_sheet->isHidden();
}

void SheetAdaptor::setHidden(bool hidden)
{
    m_sheet->setHidden(hidden);
}

#if 0
bool SheetAdaptor::showGrid() const
{
    return m_sheet->getShowGrid();
}

bool SheetAdaptor::showFormula() const
{
    return m_sheet->getShowFormula();
}

bool SheetAdaptor::lcMode() const
{
    return m_sheet->getLcMode();
}

bool SheetAdaptor::autoCalc() const
{
    return m_sheet->isAutoCalculationEnabled();
}

bool SheetAdaptor::showColumnNumber() const
{
    return m_sheet->getShowColumnNumber();
}

bool SheetAdaptor::hideZero() const
{
    return m_sheet->getHideZero();
}

bool SheetAdaptor::firstLetterUpper() const
{
    return m_sheet->getFirstLetterUpper();
}

void SheetAdaptor::setShowPageBorders(bool b)
{
    m_sheet->setShowPageBorders(b);
    m_sheet->doc()->updateBorderButton();
}
#endif

float SheetAdaptor::paperHeight()const
{
    return m_sheet->print()->settings()->pageLayout().height;
}

void SheetAdaptor::setPrinterHeight(float height)
{
    KOdfPageLayoutData pageLayout = m_sheet->print()->settings()->pageLayout();
    pageLayout.format = KOdfPageFormat::CustomSize;
    pageLayout.height = MM_TO_POINT(height);
    m_sheet->print()->settings()->setPageLayout(pageLayout);
}

float SheetAdaptor::paperWidth()const
{
    return m_sheet->print()->settings()->pageLayout().width;
}

void SheetAdaptor::setPaperWidth(float width)
{
    KOdfPageLayoutData pageLayout = m_sheet->print()->settings()->pageLayout();
    pageLayout.format = KOdfPageFormat::CustomSize;
    pageLayout.width = MM_TO_POINT(width);
    m_sheet->print()->settings()->setPageLayout(pageLayout);
}

float SheetAdaptor::paperLeftBorder()const
{
    return m_sheet->print()->settings()->pageLayout().leftMargin;
}

float SheetAdaptor::paperRightBorder()const
{
    return m_sheet->print()->settings()->pageLayout().rightMargin;
}

float SheetAdaptor::paperTopBorder()const
{
    return m_sheet->print()->settings()->pageLayout().topMargin;
}

float SheetAdaptor::paperBottomBorder()const
{
    return m_sheet->print()->settings()->pageLayout().bottomMargin;
}

QString SheetAdaptor::paperFormat() const
{
    return m_sheet->printSettings()->paperFormatString();
}

QString SheetAdaptor::paperOrientation() const
{
    return m_sheet->printSettings()->orientationString();
}

void SheetAdaptor::setPaperLayout(float leftBorder, float topBorder,
                                  float rightBorder, float bottomBoder,
                                  const QString& format, const QString& orientation)
{
    KOdfPageLayoutData pageLayout;
    pageLayout.format = KOdfPageFormat::formatFromString(format);
    pageLayout.orientation = (orientation  == "Portrait")
                             ? KOdfPageFormat::Portrait : KOdfPageFormat::Landscape;
    pageLayout.leftMargin   = leftBorder;
    pageLayout.rightMargin  = rightBorder;
    pageLayout.topMargin    = topBorder;
    pageLayout.bottomMargin = bottomBoder;
    m_sheet->print()->settings()->setPageLayout(pageLayout);
}

#if 0
QString SheetAdaptor::printHeadLeft()const
{
    return m_sheet->print()->headLeft();
}

QString SheetAdaptor::printHeadMid()const
{
    return m_sheet->print()->headMid();
}

QString SheetAdaptor::printHeadRight()const
{
    return m_sheet->print()->headRight();
}

QString SheetAdaptor::printFootLeft()const
{
    return m_sheet->print()->footLeft();
}

QString SheetAdaptor::printFootMid()const
{
    return m_sheet->print()->footMid();
}

QString SheetAdaptor::printFootRight()const
{
    return m_sheet->print()->footRight();
}

void SheetAdaptor::setPrintHeaderLeft(const QString & text)
{
    m_sheet->print()->setHeadFootLine(text,       headMid(), headRight(),
                                      footLeft(), footMid(), footRight());
}

void SheetAdaptor::setPrintHeaderMiddle(const QString & text)
{
    m_sheet->print()->setHeadFootLine(headLeft(), text,      headRight(),
                                      footLeft(), footMid(), footRight());

}

void SheetAdaptor::setPrintHeaderRight(const QString & text)
{
    m_sheet->print()->setHeadFootLine(headLeft(), headMid(), text,
                                      footLeft(), footMid(), footRight());
}

void SheetAdaptor::setPrintFooterLeft(const QString & text)
{
    m_sheet->print()->setHeadFootLine(headLeft(), headMid(), headRight(),
                                      text,       footMid(), footRight());
}

void SheetAdaptor::setPrintFooterMiddle(const QString & text)
{
    m_sheet->print()->setHeadFootLine(headLeft(), headMid(), headRight(),
                                      footLeft(), text,      footRight());
}

void SheetAdaptor::setPrintFooterRight(const QString & text)
{
    m_sheet->print()->setHeadFootLine(headLeft(), headMid(), headRight(),
                                      footLeft(), footMid(), text);
}
#endif

bool SheetAdaptor::checkPassword(const QByteArray& passwd) const
{
    return m_sheet->checkPassword(passwd);
}

bool SheetAdaptor::isProtected() const
{
    return m_sheet->isProtected();
}

void SheetAdaptor::setProtected(const QByteArray& passwd)
{
    m_sheet->setProtected(passwd);
}

void SheetAdaptor::handleDamages(const QList<KCDamage*>& damages)
{
    const QList<KCDamage*>::ConstIterator end(damages.end());
    for (QList<KCDamage*>::ConstIterator it = damages.begin(); it != end; ++it) {
        const KCDamage *const damage = *it;
        if (!damage) {
            continue;
        }
        if (damage->type() == KCDamage::DamagedSheet) {
            const KCSheetDamage *const sheetDamage = static_cast<const KCSheetDamage *>(damage);
            // Only process the sheet this adaptor works for.
            if (sheetDamage->sheet() != m_sheet) {
                continue;
            }
            kDebug(36007) << *sheetDamage;
            const KCSheetDamage::Changes changes = sheetDamage->changes();
            if (changes & KCSheetDamage::Name) {
                emit nameChanged();
            }
            if (changes & KCSheetDamage::Shown) {
                emit showChanged();
            }
            if (changes & KCSheetDamage::Hidden) {
                emit hideChanged();
            }
            continue;
        }
    }
}

#include "SheetAdaptor.moc"
