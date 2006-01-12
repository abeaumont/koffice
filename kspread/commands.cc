/* This file is part of the KDE project
   Copyright 2004 Ariya Hidayat <ariya@kde.org>
   Copyright 2004 Laurent Montel <montel@kde.org>

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

#include "commands.h"
#include "damages.h"
#include "kspread_canvas.h"
#include "kspread_doc.h"
#include "kspread_locale.h"
#include "kspread_map.h"
#include "kspread_sheet.h"
#include "kspread_undo.h"
#include "kspread_util.h"

#include "kspread_sheetprint.h"

using namespace KSpread;

// ----- UndoWrapperCommand -----

UndoWrapperCommand::UndoWrapperCommand( UndoAction* ua )
{
  undoAction = ua;
}

void UndoWrapperCommand::execute()
{
  // This is not really safe and functional, but UndoWrapperCommand
  // is a workaround anyway. So leave it as it it (Ariya)
  undoAction->redo();
}

void UndoWrapperCommand::unexecute()
{
  undoAction->undo();
}

QString UndoWrapperCommand::name() const
{
  return undoAction->getName();
}

// ----- MergeCellsCommand -----

MergeCellCommand::MergeCellCommand( Cell* c, int cs, int rs )
{
  cell = c;
  colSpan = cs;
  rowSpan = rs;
  oldColSpan = cell->extraXCells();
  oldRowSpan = cell->extraYCells();
  if( cell )
  {
    QRect area( cell->column(), cell->row(), cs+1, rs+1 );
    rangeName = util_rangeName( area );
  }
}

QString MergeCellCommand::name() const
{
  if( rangeName.isEmpty() )
    return i18n("Merge Cells");
  else
    return i18n("Merge Cells %1").arg( rangeName );
}

void MergeCellCommand::execute()
{
  Sheet* sheet = cell->sheet();
  if( !sheet ) return;
  sheet->changeMergedCell( cell->column(), cell->row(), colSpan, rowSpan);
}

void MergeCellCommand::unexecute()
{
  Sheet* sheet = cell->sheet();
  if( !sheet ) return;
  sheet->changeMergedCell( cell->column(), cell->row(), oldColSpan, oldRowSpan);
}

// ----- DissociateCellCommand -----

DissociateCellCommand::DissociateCellCommand( Cell* c )
{
  cell = c;
  oldColSpan = cell->extraXCells();
  oldRowSpan = cell->extraYCells();
}

QString DissociateCellCommand::name() const
{
  return i18n("Dissociate Cell");
}

void DissociateCellCommand::execute()
{
  Sheet* sheet = cell->sheet();
  if( !sheet ) return;
  sheet->changeMergedCell( cell->column(), cell->row(), 0, 0 );
}

void DissociateCellCommand::unexecute()
{
  Sheet* sheet = cell->sheet();
  if( !sheet ) return;
  sheet->changeMergedCell( cell->column(), cell->row(), oldColSpan, oldRowSpan);
}

// ----- RenameSheetCommand -----

RenameSheetCommand::RenameSheetCommand( Sheet* s, const QString &name )
{
  sheet = s;
  if( s ) oldName = s->sheetName();
  newName = name;
}

QString RenameSheetCommand::name() const
{
  return i18n("Rename Sheet");
}

void RenameSheetCommand::execute()
{
  if( sheet )
    sheet->setSheetName( newName );
}

void RenameSheetCommand::unexecute()
{
  if( sheet )
    sheet->setSheetName( oldName );
}

// ----- HideSheetCommand -----

HideSheetCommand::HideSheetCommand( Sheet* sheet )
{
  doc = sheet->doc();
  sheetName = sheet->sheetName();
}

void HideSheetCommand::execute()
{
  Sheet* sheet = doc->map()->findSheet( sheetName );
  if( !sheet ) return;

  sheet->hideSheet( true );
}

void HideSheetCommand::unexecute()
{
  Sheet* sheet = doc->map()->findSheet( sheetName );
  if( !sheet ) return;

  sheet->hideSheet( false );
}

QString HideSheetCommand::name() const
{
    QString n = QString( i18n("Hide Sheet %1") ).arg( sheetName );
    if( n.length() > 64 ) n = i18n("Hide Sheet");
    return n;
}

// ----- ShowSheetCommand -----

ShowSheetCommand::ShowSheetCommand( Sheet* sheet )
{
  doc = sheet->doc();
  sheetName = sheet->sheetName();
}

void ShowSheetCommand::execute()
{
  Sheet* sheet = doc->map()->findSheet( sheetName );
  if( !sheet ) return;

  sheet->hideSheet( false );
}

void ShowSheetCommand::unexecute()
{
  Sheet* sheet = doc->map()->findSheet( sheetName );
  if( !sheet ) return;

  sheet->hideSheet( true );
}

QString ShowSheetCommand::name() const
{
    QString n = QString( i18n("Show Sheet %1") ).arg( sheetName );
    if( n.length() > 64 ) n = i18n("Show Sheet");
    return n;
}


// ----- AddSheetCommand -----

AddSheetCommand::AddSheetCommand( Sheet* s )
{
    sheet = s;
    doc = sheet->doc();
    doc->map()->addSheet( s );
}

void AddSheetCommand::execute()
{
    sheet->workbook()->insertSheet( sheet );
    doc->insertSheet( sheet );
}

void AddSheetCommand::unexecute()
{
    sheet->workbook()->takeSheet( sheet );
    doc->takeSheet( sheet );
}

QString AddSheetCommand::name() const
{
    return i18n("Add Sheet");
}


// ----- RemoveSheetCommand -----

RemoveSheetCommand::RemoveSheetCommand( Sheet* s )
{
    sheet = s;
    doc = sheet->doc();
}

void RemoveSheetCommand::execute()
{
    sheet->workbook()->takeSheet( sheet );
    doc->takeSheet( sheet );
}

void RemoveSheetCommand::unexecute()
{
    sheet->workbook()->insertSheet( sheet );
    doc->insertSheet( sheet );
}

QString RemoveSheetCommand::name() const
{
    return i18n("Remove Sheet");
}

// ----- SheetPropertiesCommand -----

SheetPropertiesCommand::SheetPropertiesCommand( Doc* d, Sheet* s )
{
    sheet = s;
    doc = d;
    oldDirection = newDirection = sheet->layoutDirection();
    oldAutoCalc = newAutoCalc = sheet->getAutoCalc();
    oldShowGrid = newShowGrid = sheet->getShowGrid();
    oldShowPageBorders = newShowPageBorders = sheet->isShowPageBorders();
    oldShowFormula = newShowFormula = sheet->getShowFormula();
    oldHideZero = newHideZero = sheet->getHideZero();
    oldShowFormulaIndicator = newShowFormulaIndicator = sheet->getShowFormulaIndicator();
    oldShowCommentIndicator = newShowCommentIndicator = sheet->getShowCommentIndicator();
    oldColumnAsNumber = newColumnAsNumber = sheet->getShowColumnNumber();
    oldLcMode = newLcMode = sheet->getLcMode();
    oldCapitalizeFirstLetter = newCapitalizeFirstLetter = sheet->getFirstLetterUpper();
}

QString SheetPropertiesCommand::name() const
{
    return i18n("Change Sheet Properties");
}

void SheetPropertiesCommand::setLayoutDirection( Sheet::LayoutDirection dir )
{
    newDirection = dir;
}

void SheetPropertiesCommand::setAutoCalc( bool b )
{
    newAutoCalc = b;
}

void SheetPropertiesCommand::setShowGrid( bool b )
{
    newShowGrid = b;
}

void SheetPropertiesCommand::setShowPageBorders( bool b )
{
    newShowPageBorders = b;
}

void SheetPropertiesCommand::setShowFormula( bool b )
{
    newShowFormula = b;
}

void SheetPropertiesCommand::setHideZero( bool b  )
{
    newHideZero = b;
}

void SheetPropertiesCommand::setShowFormulaIndicator( bool b )
{
    newShowFormulaIndicator = b;
}

void SheetPropertiesCommand::setShowCommentIndicator( bool b )
{
  newShowCommentIndicator = b;
}

void SheetPropertiesCommand::setColumnAsNumber( bool b  )
{
    newColumnAsNumber = b;
}

void SheetPropertiesCommand::setLcMode( bool b  )
{
    newLcMode = b;
}

void SheetPropertiesCommand::setCapitalizeFirstLetter( bool b )
{
    newCapitalizeFirstLetter = b;
}

void SheetPropertiesCommand::execute()
{
    sheet->setLayoutDirection( newDirection );
    sheet->setAutoCalc( newAutoCalc );
    sheet->setShowGrid( newShowGrid );
    sheet->setShowPageBorders( newShowPageBorders );
    sheet->setShowFormula( newShowFormula );
    sheet->setHideZero( newHideZero );
    sheet->setShowFormulaIndicator( newShowFormulaIndicator );
    sheet->setShowCommentIndicator( newShowCommentIndicator );
    sheet->setShowColumnNumber( newColumnAsNumber );
    sheet->setLcMode( newLcMode );
    sheet->setFirstLetterUpper( newCapitalizeFirstLetter );
    doc->addDamage( new SheetDamage( sheet, SheetDamage::PropertiesChanged ) );
}

void SheetPropertiesCommand::unexecute()
{
    sheet->setLayoutDirection( oldDirection );
    sheet->setAutoCalc( oldAutoCalc );
    sheet->setShowGrid( oldShowGrid );
    sheet->setShowPageBorders( oldShowPageBorders );
    sheet->setShowFormula( oldShowFormula );
    sheet->setHideZero( oldHideZero );
    sheet->setShowFormulaIndicator( oldShowFormulaIndicator );
    sheet->setShowCommentIndicator( oldShowCommentIndicator );
    sheet->setShowColumnNumber( oldColumnAsNumber );
    sheet->setLcMode( oldLcMode );
    sheet->setFirstLetterUpper( oldCapitalizeFirstLetter );
    doc->addDamage( new SheetDamage( sheet, SheetDamage::PropertiesChanged ) );
}


// ----- InsertColumnCommand -----

InsertColumnCommand::InsertColumnCommand( Sheet* s , unsigned int _column, unsigned int _nbCol )
{
  doc = s->doc();
  sheetName = s->sheetName();
  insertPosColumn = _column;
  nbColumnInserted = _nbCol;
}

void InsertColumnCommand::execute()
{
    Sheet* sheet = doc->map()->findSheet( sheetName );
    if( !sheet ) return;
    sheet->insertColumn( insertPosColumn,nbColumnInserted);
}

void InsertColumnCommand::unexecute()
{
    Sheet* sheet = doc->map()->findSheet( sheetName );
    if( !sheet ) return;
    sheet->removeColumn( insertPosColumn,nbColumnInserted );
}

QString InsertColumnCommand::name() const
{
    return i18n("Insert Columns");
}


// ----- DefinePrintRangeCommand -----


DefinePrintRangeCommand::DefinePrintRangeCommand( Sheet *s )
{
  doc = s->doc();
  sheetName = s->sheetName();
  printRange = s->print()->printRange();
}

void DefinePrintRangeCommand::execute()
{
    Sheet* sheet = doc->map()->findSheet( sheetName );
    if( !sheet ) return;
    sheet->print()->setPrintRange( printRangeRedo );

}

void DefinePrintRangeCommand::unexecute()
{
    Sheet* sheet = doc->map()->findSheet( sheetName );
    if( !sheet ) return;
    printRangeRedo = sheet->print()->printRange();
    sheet->print()->setPrintRange( printRange );
}

QString DefinePrintRangeCommand::name() const
{
    return i18n("Set Page Layout");
}

// ----- PaperLayoutCommand -----


PaperLayoutCommand::PaperLayoutCommand( Sheet *s )
{
  doc = s->doc();
  sheetName = s->sheetName();
  pl = s->print()->paperLayout();
  hf = s->print()->headFootLine();
  unit = doc->unit();
  printGrid = s->print()->printGrid();
  printCommentIndicator = s->print()->printCommentIndicator();
  printFormulaIndicator = s->print()->printFormulaIndicator();
  printRange = s->print()->printRange();
  printRepeatColumns = s->print()->printRepeatColumns();
  printRepeatRows = s->print()->printRepeatRows();
  zoom = s->print()->zoom();
  pageLimitX = s->print()->pageLimitX();
  pageLimitY = s->print()->pageLimitY();
}

void PaperLayoutCommand::execute()
{
    Sheet* sheet = doc->map()->findSheet( sheetName );
    if( !sheet ) return;
    SheetPrint* print = sheet->print();

    print->setPaperLayout( plRedo.ptLeft,  plRedo.ptTop,
                           plRedo.ptRight, plRedo.ptBottom,
                           plRedo.format, plRedo.orientation );

    print->setHeadFootLine( hfRedo.headLeft, hfRedo.headMid, hfRedo.headRight,
                            hfRedo.footLeft, hfRedo.footMid, hfRedo.footRight );

    doc->setUnit( unitRedo );

    print->setPrintGrid( printGridRedo );
    print->setPrintCommentIndicator( printCommentIndicatorRedo );
    print->setPrintFormulaIndicator( printFormulaIndicatorRedo );

    print->setPrintRange( printRangeRedo );
    print->setPrintRepeatColumns( printRepeatColumnsRedo );
    print->setPrintRepeatRows( printRepeatRowsRedo );

    print->setZoom( zoomRedo );

    print->setPageLimitX( pageLimitX );
    print->setPageLimitY( pageLimitY );
}

void PaperLayoutCommand::unexecute()
{
    Sheet* sheet = doc->map()->findSheet( sheetName );
    if( !sheet ) return;
    SheetPrint* print = sheet->print();
    plRedo = print->paperLayout();
    print->setPaperLayout( pl.ptLeft,  pl.ptTop,
                           pl.ptRight, pl.ptBottom,
                           pl.format,  pl.orientation );

    hfRedo = print->headFootLine();
    print->setHeadFootLine( hf.headLeft, hf.headMid, hf.headRight,
                            hf.footLeft, hf.footMid, hf.footRight );

    unitRedo = doc->unit();
    doc->setUnit( unit );

    printGridRedo = print->printGrid();
    print->setPrintGrid( printGrid );

    printCommentIndicatorRedo = print->printCommentIndicator();
    print->setPrintCommentIndicator( printCommentIndicator );

    printFormulaIndicatorRedo = print->printFormulaIndicator();
    print->setPrintFormulaIndicator( printFormulaIndicator );

    printRangeRedo = print->printRange();
    print->setPrintRange( printRange );

    printRepeatColumnsRedo = print->printRepeatColumns();
    print->setPrintRepeatColumns( printRepeatColumns );

    printRepeatRowsRedo = print->printRepeatRows();
    print->setPrintRepeatRows( printRepeatRows );

    zoomRedo = print->zoom();
    print->setZoom( zoom );

    pageLimitXRedo = print->pageLimitX();
    print->setPageLimitX( pageLimitX );

    pageLimitYRedo = print->pageLimitY();
    print->setPageLimitY( pageLimitY );

}

QString PaperLayoutCommand::name() const
{
    return i18n("Set Page Layout");
}

LinkCommand::LinkCommand( Cell* c, const QString& text, const QString& link )
{
  cell = c;
  oldText = cell->text();
  oldLink = cell->link();
  newText = text;
  newLink = link;

  Sheet* s = cell->sheet();
  if( s ) doc = s->doc();
}

void LinkCommand::execute()
{
  if( !cell ) return;

  if( !newText.isEmpty() )
    cell->setCellText( newText );
  cell->setLink( newLink  );

  doc->addDamage( new CellDamage( cell ) );
}

void LinkCommand::unexecute()
{
  if( !cell ) return;

  cell->setCellText( oldText );
  cell->setLink( oldLink );

  doc->addDamage( new CellDamage( cell ) );
}

QString LinkCommand::name() const
{
  return newLink.isEmpty() ? i18n("Remove Link") : i18n("Set Link");
}

ChangeObjectGeometryCommand::ChangeObjectGeometryCommand( KSpreadObject *_obj, KoRect &_newGeometry )
{
  obj = _obj;
  newGeometry = _newGeometry;
}

void ChangeObjectGeometryCommand::execute()
{
  KoRect tmp = obj->geometry();
  obj->setGeometry( newGeometry );
  newGeometry = tmp;
  obj->sheet()->doc()->repaint( obj );
  obj->sheet()->doc()->repaint( obj->sheet()->doc()->zoomRect( newGeometry ) );
}

void ChangeObjectGeometryCommand::unexecute()
{
  KoRect tmp = obj->geometry();
  obj->setGeometry( newGeometry );
  newGeometry = tmp;
  obj->sheet()->doc()->repaint( obj );
  obj->sheet()->doc()->repaint( obj->sheet()->doc()->zoomRect( newGeometry ) );
}

QString ChangeObjectGeometryCommand::name() const
{
  if ( fabs( obj->geometry().width() - newGeometry.width() )<1e-3  && fabs( obj->geometry().height() - newGeometry.height() ) < 1e-3 )
    return i18n("Move object");
  else
    return i18n("Resize object");
}

RemoveObjectCommand::RemoveObjectCommand( KSpreadObject *_obj, bool _cut )
{
  obj = _obj;
  cut = _cut;
  doc = obj->sheet()->doc();
}

RemoveObjectCommand::~RemoveObjectCommand()
{
  if ( !executed )
    return;
  //kdDebug() << "*********Deleting object..." << endl;
  if ( obj->getType() == OBJECT_CHART )
  {
    KSpreadChild *chart = dynamic_cast<KSpreadChild *>(obj);
    chart->embeddedObject()->setDeleted(true);
  }

  delete obj;
}

void RemoveObjectCommand::execute()
{

//  I don't think we need this:
//       Make shure that this child has no active embedded view -> activate ourselfs
//       doc()->emitBeginOperation( false );
//       partManager()->setActivePart( koDocument(), this );
//       partManager()->setSelectedPart( 0 );
//       doc()->emitEndOperation( d->activeSheet->visibleRect( d->canvas ) );

  doc->embeddedObjects().removeRef( obj );

  obj->setSelected( false );
  doc->repaint( obj );
  executed = true;
}

void RemoveObjectCommand::unexecute()
{
  doc->embeddedObjects().append( obj );
  doc->repaint( obj );
  executed = false;
}

QString RemoveObjectCommand::name() const
{
  if ( cut )
    return i18n("Cut object");
  else
    return i18n("Remove object");
}

InsertObjectCommand::InsertObjectCommand( const KoRect& _geometry, KoDocumentEntry& _entry, Canvas *_canvas ) //child
{
  geometry = _geometry;
  entry = _entry;
  canvas = _canvas;
  type = OBJECT_KOFFICE_PART;
  obj = 0;
}

InsertObjectCommand::InsertObjectCommand(const KoRect& _geometry, KoDocumentEntry& _entry, const QRect& _data, Canvas *_canvas ) //chart
{
  geometry = _geometry;
  entry = _entry;
  data = _data;
  canvas = _canvas;
  type = OBJECT_CHART;
  obj = 0;
}

InsertObjectCommand::InsertObjectCommand( const KoRect& _geometry, KURL& _file, Canvas *_canvas ) //picture
{
  geometry = _geometry;
  file = _file;
  canvas = _canvas;
  type = OBJECT_PICTURE;
  obj = 0;
}

InsertObjectCommand::~InsertObjectCommand()
{
  if ( executed )
    return;
  //kdDebug() << "*********Deleting object..." << endl;
  if ( obj->getType() == OBJECT_CHART )
  {
    KSpreadChild *chart = dynamic_cast<KSpreadChild *>(obj);
    chart->embeddedObject()->setDeleted(true);
  }

  delete obj;
}

void InsertObjectCommand::execute()
{
  if ( obj ) //restore the object which was removed from the object list in InsertObjectCommand::unexecute()
  {
    canvas->doc()->embeddedObjects().append( obj );
    canvas->doc()->repaint( obj );
  }
  else
  {
    bool success = false;
    switch ( type )
    {
      case OBJECT_CHART:
      {
        success = canvas->activeSheet()->insertChart( geometry, entry, data );
        break;
      }
      case OBJECT_KOFFICE_PART:
      {
        success = canvas->activeSheet()->insertChild( geometry, entry );
        break;
      }
      case OBJECT_PICTURE:
      {
        success = canvas->activeSheet()->insertPicture( geometry, file );
        break;
      }
      default:
        break;
    }
    if ( success )
      obj = canvas->doc()->embeddedObjects().last();
    else
      obj = 0;
  }
  executed = true;
}

void InsertObjectCommand::unexecute()
{
  if ( !obj )
    return;

  canvas->doc()->embeddedObjects().removeRef( obj );
  obj->setSelected( false );
  canvas->doc()->repaint( obj );

  executed = false;
}

QString InsertObjectCommand::name() const
{
  return i18n("Insert object");
}
