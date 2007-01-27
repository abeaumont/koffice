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

#include "Cell.h"
#include "Formula.h"
#include "RectStorage.h"
#include "StyleStorage.h"
#include "Value.h"

#include "CellStorage.h"

using namespace KSpread;

class CellStorage::Private
{
public:
    Private( Sheet* sheet )
        : sheet( sheet )
        , commentStorage( sheet )
        , conditionsStorage( sheet )
        , fusionStorage( sheet )
        , styleStorage( sheet )
        , validityStorage( sheet ) {}

    Sheet*              sheet;
    CommentStorage      commentStorage;
    ConditionsStorage   conditionsStorage;
    FormulaStorage      formulaStorage;
    FusionStorage       fusionStorage;
    LinkStorage         linkStorage;
    StyleStorage        styleStorage;
    ValidityStorage     validityStorage;
    ValueStorage        valueStorage;
    PointStorage<Cell>  storage; // TODO Stefan: Remove!
};

CellStorage::CellStorage( Sheet* sheet )
    : d( new Private( sheet ) )
{
}

CellStorage::~CellStorage()
{
    delete d;
}

void CellStorage::take( int col, int row )
{
    // TODO Stefan: Undo?!
    d->formulaStorage.take( col, row );
    d->linkStorage.take( col, row );
    d->valueStorage.take( col, row );
}

QString CellStorage::comment( int column, int row ) const
{
    return commentStorage()->contains( QPoint( column, row ) );
}

void CellStorage::setComment( const Region& region, const QString& comment ) const
{
    commentStorage()->insert( region, comment );
}

Conditions CellStorage::conditions( int column, int row ) const
{
    return conditionsStorage()->contains( QPoint( column, row ) );
}

void CellStorage::setConditions( const Region& region, Conditions conditions ) const
{
    conditionsStorage()->insert( region, conditions );
}

Formula CellStorage::formula( int column, int row ) const
{
    if ( column == 0 || row == 0 )
        return Formula();
    return formulaStorage()->lookup( column, row );
}

void CellStorage::setFormula( int column, int row, const Formula& formula )
{
    if ( column == 0 || row == 0 )
        return;
    if ( formula.expression().isEmpty() )
        formulaStorage()->take( column, row );
    else
        formulaStorage()->insert( column, row, formula );
}

QString CellStorage::link( int column, int row ) const
{
    if ( column == 0 || row == 0 )
        return QString();
    return linkStorage()->lookup( column, row );
}

void CellStorage::setLink( int column, int row, const QString& link )
{
    if ( column == 0 || row == 0 )
        return;
    if ( link.isEmpty() )
        linkStorage()->take( column, row );
    else
        linkStorage()->insert( column, row, link );
}

Validity CellStorage::validity( int column, int row ) const
{
    return validityStorage()->contains( QPoint( column, row ) );
}

void CellStorage::setValidity( const Region& region, Validity validity ) const
{
    validityStorage()->insert( region, validity );
}

Value CellStorage::value( int column, int row ) const
{
    if ( column == 0 || row == 0 )
        return Value();
    return valueStorage()->lookup( column, row );
}

void CellStorage::setValue( int column, int row, const Value& value )
{
    if ( column == 0 || row == 0 )
        return;
    if ( value.isEmpty() )
        valueStorage()->take( column, row );
    else
        valueStorage()->insert( column, row, value );
}

QVector< QPair<QPoint,Cell> > CellStorage::insertColumns( int position, int number )
{
    return d->storage.insertColumns( position, number );
}

QVector< QPair<QPoint,Cell> > CellStorage::removeColumns( int position, int number )
{
    return d->storage.removeColumns( position, number );
}

QVector< QPair<QPoint,Cell> > CellStorage::insertRows( int position, int number )
{
    return d->storage.insertRows( position, number );
}

QVector< QPair<QPoint,Cell> > CellStorage::removeRows( int position, int number )
{
    return d->storage.removeRows( position, number );
}

QVector< QPair<QPoint,Cell> > CellStorage::removeShiftLeft( const QRect& rect )
{
    return d->storage.removeShiftLeft( rect );
}

QVector< QPair<QPoint,Cell> > CellStorage::insertShiftRight( const QRect& rect )
{
    return d->storage.insertShiftRight( rect );
}

QVector< QPair<QPoint,Cell> > CellStorage::removeShiftUp( const QRect& rect )
{
    return d->storage.removeShiftUp( rect );
}

QVector< QPair<QPoint,Cell> > CellStorage::insertShiftDown( const QRect& rect )
{
    return d->storage.insertShiftDown( rect );
}

Cell CellStorage::firstInColumn( int col ) const
{
    int newRow = 0;
    int tmpRow = 0;
    d->formulaStorage.firstInColumn( col, &tmpRow );
    newRow = tmpRow;
    d->valueStorage.firstInColumn( col, &tmpRow );
    if ( tmpRow )
        newRow = newRow ? qMin( newRow, tmpRow ) : tmpRow;
    if ( !tmpRow )
        return Cell();
    return Cell( d->sheet, col, newRow );
}

Cell CellStorage::firstInRow( int row ) const
{
    int newCol = 0;
    int tmpCol = 0;
    d->formulaStorage.firstInRow( row, &tmpCol );
    newCol = tmpCol;
    d->valueStorage.firstInRow( row, &tmpCol );
    if ( tmpCol )
        newCol = newCol ? qMin( newCol, tmpCol ) : tmpCol;
    if ( !tmpCol )
        return Cell();
    return Cell( d->sheet, newCol, row );
}

Cell CellStorage::lastInColumn( int col ) const
{
    int newRow = 0;
    int tmpRow = 0;
    d->formulaStorage.lastInColumn( col, &tmpRow );
    newRow = tmpRow;
    d->valueStorage.lastInColumn( col, &tmpRow );
    newRow = qMax( newRow, tmpRow );
    if ( !tmpRow )
        return Cell();
    return Cell( d->sheet, col, newRow );
}

Cell CellStorage::lastInRow( int row ) const
{
    int newCol = 0;
    int tmpCol = 0;
    d->formulaStorage.lastInRow( row, &tmpCol );
    newCol = tmpCol;
    d->valueStorage.lastInRow( row, &tmpCol );
    newCol = qMax( newCol, tmpCol );
    if ( !tmpCol )
        return Cell();
    return Cell( d->sheet, newCol, row );
}

Cell CellStorage::nextInColumn( int col, int row ) const
{
    int newRow = 0;
    int tmpRow = 0;
    d->formulaStorage.nextInColumn( col, row, &tmpRow );
    newRow = tmpRow;
    d->valueStorage.nextInColumn( col, row, &tmpRow );
    if ( tmpRow )
        newRow = newRow ? qMin( newRow, tmpRow ) : tmpRow;
    if ( !tmpRow )
        return Cell();
    return Cell( d->sheet, col, newRow );
}

Cell CellStorage::nextInRow( int col, int row ) const
{
    int newCol = 0;
    int tmpCol = 0;
    d->formulaStorage.nextInRow( col, row, &tmpCol );
    newCol = tmpCol;
    d->valueStorage.nextInRow( col, row, &tmpCol );
    if ( tmpCol )
        newCol = newCol ? qMin( newCol, tmpCol ) : tmpCol;
    if ( !tmpCol )
        return Cell();
    return Cell( d->sheet, newCol, row );
}

Cell CellStorage::prevInColumn( int col, int row ) const
{
    int newRow = 0;
    int tmpRow = 0;
    d->formulaStorage.prevInColumn( col, row, &tmpRow );
    newRow = tmpRow;
    d->valueStorage.prevInColumn( col, row, &tmpRow );
    newRow = qMax( newRow, tmpRow );
    if ( !tmpRow )
        return Cell();
    return Cell( d->sheet, col, newRow );
}

Cell CellStorage::prevInRow( int col, int row ) const
{
    int newCol = 0;
    int tmpCol = 0;
    d->formulaStorage.prevInRow( col, row, &tmpCol );
    newCol = tmpCol;
    d->valueStorage.prevInRow( col, row, &tmpCol );
    newCol = qMax( newCol, tmpCol );
    if ( !tmpCol )
        return Cell();
    return Cell( d->sheet, newCol, row );
}

int CellStorage::columns() const
{
    int max = 0;
    max = qMax( max, d->commentStorage.usedArea().right() );
    max = qMax( max, d->conditionsStorage.usedArea().right() );
    max = qMax( max, d->fusionStorage.usedArea().right() );
    max = qMax( max, d->styleStorage.usedArea().right() );
    max = qMax( max, d->validityStorage.usedArea().right() );
    max = qMax( max, d->formulaStorage.columns() );
    max = qMax( max, d->linkStorage.columns() );
    max = qMax( max, d->valueStorage.columns() );
    return max;
}

int CellStorage::rows() const
{
    int max = 0;
    max = qMax( max, d->commentStorage.usedArea().bottom() );
    max = qMax( max, d->conditionsStorage.usedArea().bottom() );
    max = qMax( max, d->fusionStorage.usedArea().bottom() );
    max = qMax( max, d->styleStorage.usedArea().bottom() );
    max = qMax( max, d->validityStorage.usedArea().bottom() );
    max = qMax( max, d->formulaStorage.rows() );
    max = qMax( max, d->linkStorage.rows() );
    max = qMax( max, d->valueStorage.rows() );
    return max;
}

CellStorage CellStorage::subStorage( const Region& region ) const
{
    CellStorage subStorage( d->sheet );
    subStorage.d->formulaStorage = d->formulaStorage.subStorage( region );
    subStorage.d->linkStorage = d->linkStorage.subStorage( region );
    subStorage.d->valueStorage = d->valueStorage.subStorage( region );
    return subStorage;
}

CommentStorage* CellStorage::commentStorage() const
{
    return &d->commentStorage;
}

ConditionsStorage* CellStorage::conditionsStorage() const
{
    return &d->conditionsStorage;
}

FormulaStorage* CellStorage::formulaStorage() const
{
    return &d->formulaStorage;
}

FusionStorage* CellStorage::fusionStorage() const
{
    return &d->fusionStorage;
}

LinkStorage* CellStorage::linkStorage() const
{
    return &d->linkStorage;
}

StyleStorage* CellStorage::styleStorage() const
{
    return &d->styleStorage;
}

ValidityStorage* CellStorage::validityStorage() const
{
    return &d->validityStorage;
}

ValueStorage* CellStorage::valueStorage() const
{
    return &d->valueStorage;
}
