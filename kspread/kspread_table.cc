#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

#include <qmsgbox.h>
#include <qpainter.h>
#include <qdrawutl.h>
#include <qkeycode.h>
#include <qregexp.h>
#include <qpoint.h>
#include <qprinter.h>
#include <qcursor.h>
#include <qstack.h>
#include <qbuffer.h>

#include "kspread_table.h"
#include "kspread_view.h"
#include "kspread_python.h"
#include "kspread_dlg_print.h"
#include "kspread_undo.h"
#include "kspread_map.h"

/*****************************************************************************
 *
 * CellBinding
 *
 *****************************************************************************/

CellBinding::CellBinding( KSpreadTable *_table, int _left, int _top, int _right, int _bottom )
{
  m_rctRect.setCoords( _left, _top, _right, _bottom );
    
  m_pTable = _table;
  m_pTable->addCellBinding( this );

  m_bIgnoreChanges = FALSE;
}

CellBinding::~CellBinding()
{
  m_pTable->removeCellBinding( this );
}

void CellBinding::cellChanged( KSpreadCell *_cell )
{
  if ( m_bIgnoreChanges )
    return;
  
  emit changed( _cell );
}

bool CellBinding::contains( int _x, int _y )
{
    return m_rctRect.contains( QPoint( _x, _y ) );
}

/*****************************************************************************
 *
 * KSpreadTable
 *
 *****************************************************************************/

KSpreadTable::KSpreadTable( KSpreadDoc *_doc, const char *_name )
{
  m_pDoc = _doc;
  
  m_bShowPageBorders = FALSE;
   
  m_bLoading = FALSE;
    
  m_lstCellBindings.setAutoDelete( FALSE );
            
  m_strName = _name;
    
  m_dctCells.setAutoDelete( true );
  m_dctRows.setAutoDelete( true );
  m_dctColumns.setAutoDelete( true );

  m_pDefaultCell = new KSpreadCell( this, 0, 0 );
  m_pDefaultRowLayout = new RowLayout( this, 0 );    
  m_pDefaultRowLayout->setDefault();
  m_pDefaultColumnLayout = new ColumnLayout( this, 0 );    
  m_pDefaultColumnLayout->setDefault();

  // No selection is active
  m_rctSelection.setCoords( 0, 0, 0, 0 );

  m_pWidget = new QWidget();
  m_pPainter = new QPainter;
  m_pPainter->begin( m_pWidget );
}

ColumnLayout* KSpreadTable::columnLayout( int _column )
{
    ColumnLayout *p = m_dctColumns[ _column ];
    if ( p != 0L )
	return p;

    return m_pDefaultColumnLayout;
}

RowLayout* KSpreadTable::rowLayout( int _row )
{
    RowLayout *p = m_dctRows[ _row ];
    if ( p != 0L )
	return p;

    return m_pDefaultRowLayout;
}

int KSpreadTable::leftColumn( int _xpos, int &_left, KSpreadView *_view )
{
    _xpos += _view->xOffset();
    _left = -_view->xOffset();
    
    int col = 1;
    int x = columnLayout( col )->width( _view );
    while ( x < _xpos )
    {
	// Should never happen
	if ( col == 0xFFFF )
	    return 1;
	_left += columnLayout( col )->width( _view );
	col++;
	x += columnLayout( col )->width( _view );
    }
    
    return col;
}

int KSpreadTable::rightColumn( int _xpos, KSpreadView *_view )
{
    _xpos += _view->xOffset();

    int col = 1;
    int x = 0;
    while ( x < _xpos )
    {
	// Should never happen
	if ( col == 0xFFFF )
	    return 0xFFFF;
	x += columnLayout( col )->width( _view );
	col++;
    }
    
    return col;
}

int KSpreadTable::topRow( int _ypos, int & _top, KSpreadView *_view )
{
    _ypos += _view->yOffset();
    _top = -_view->yOffset();
    
    int row = 1;
    int y = rowLayout( row )->height( _view );
    while ( y < _ypos )
    {
	// Should never happen
	if ( row == 0xFFFF )
	    return 1;
	_top += rowLayout( row )->height( _view );
	row++;
	y += rowLayout( row )->height( _view);
    }
    
    return row;
}

int KSpreadTable::bottomRow( int _ypos, KSpreadView *_view )
{
    _ypos += _view->yOffset();

    int row = 1;
    int y = 0;
    while ( y < _ypos )
    {
	// Should never happen
	if ( row == 0xFFFF )
	    return 0xFFFF;
	y += rowLayout( row )->height( _view );
	row++;
    }
    
    return row;
}

int KSpreadTable::columnPos( int _col, KSpreadView *_view )
{
    int col = 1;
    int x = -_view->xOffset();
    while ( col < _col )
    {
	// Should never happen
	if ( col == 0xFFFF )
	    return x;
	
	x += columnLayout( col )->width( _view );
	col++;
    }

    return x;
}

int KSpreadTable::rowPos( int _row, KSpreadView *_view )
{
    int row = 1;
    int y = -_view->yOffset();
    while ( row < _row )
    {
	// Should never happen
	if ( row == 0xFFFF )
	    return y;
	
	y += rowLayout( row )->height( _view );
	row++;
    }

    return y;
}

KSpreadCell* KSpreadTable::cellAt( int _column, int _row )
{
    int i = _row + ( _column * 0xFFFF );
    
    KSpreadCell *p = m_dctCells[ i ];
    if ( p != 0L )
	return p;

    return m_pDefaultCell;
}

ColumnLayout* KSpreadTable::nonDefaultColumnLayout( int _column )
{
    ColumnLayout *p = m_dctColumns[ _column ];
    if ( p != 0L )
	return p;
	
    p = new ColumnLayout( this, _column );
    p->setWidth( m_pDefaultColumnLayout->width() );
    m_dctColumns.insert( _column, p );

    return p;
}

RowLayout* KSpreadTable::nonDefaultRowLayout( int _row )
{
    RowLayout *p = m_dctRows[ _row ];
    if ( p != 0L )
	return p;
	
    p = new RowLayout( this, _row );
    // TODO: copy the default RowLayout here!!
    p->setHeight( m_pDefaultRowLayout->height() );
    m_dctRows.insert( _row, p );

    return p;
}

KSpreadCell* KSpreadTable::nonDefaultCell( int _column, int _row )
{
    int key = _row + ( _column * 0xFFFF );
    
    KSpreadCell *p = m_dctCells[ key ];
    if ( p != 0L )
	return p;

    KSpreadCell *cell = new KSpreadCell( this, _column, _row );
    m_dctCells.insert( key, cell );

    return cell;
}

void KSpreadTable::setText( int _row, int _column, const char *_text )
{
    m_pDoc->setModified( true );

    KSpreadCell *cell = nonDefaultCell( _column, _row );

    KSpreadUndoSetText *undo;
    if ( !m_pDoc->undoBuffer()->isLocked() )
    {
	undo = new KSpreadUndoSetText( m_pDoc, this, cell->text(), _column, _row );
	m_pDoc->undoBuffer()->appendUndo( undo );
    }
        
    printf("1\n");
    cell->setText( _text );
    printf("2\n");

    // drawCellList();
    QRect r( _column, _row, _column, _row );
    emit sig_updateView( r );
}

void KSpreadTable::setLayoutDirtyFlag()
{
    QIntDictIterator<KSpreadCell> it( m_dctCells );
    for ( ; it.current(); ++it ) 
	it.current()->setLayoutDirtyFlag();
}

void KSpreadTable::setCalcDirtyFlag()
{
    QIntDictIterator<KSpreadCell> it( m_dctCells );
    for ( ; it.current(); ++it ) 
	it.current()->setCalcDirtyFlag();
}

void KSpreadTable::unselect()
{
    if ( m_rctSelection.left() == 0 )
	return;

    QRect r = m_rctSelection;
    // Discard the selection
    m_rctSelection.setCoords( 0, 0, 0, 0 );
    
    emit sig_unselect( r );
}

void KSpreadTable::setSelection( const QRect &_sel )
{
  QRect old( m_rctSelection );
  m_rctSelection = _sel;
  
  emit sig_changeSelection( old, m_rctSelection );
}

void KSpreadTable::setSelectionFont( const QPoint &_marker, const char *_font, int _size,
				     signed char _bold, signed char _italic )
{
    m_pDoc->setModified( true );

    bool selected = ( m_rctSelection.left() != 0 );
    
    // Complete rows selected ?
    if ( selected && m_rctSelection.right() == 0x7FFF )
    {
      // TODO
    }
    // Complete columns selected ?
    else if ( selected && m_rctSelection.bottom() == 0x7FFF )
    {
      // TODO
    }
    else
    {
	QRect r( m_rctSelection );
	if ( !selected )
	    r.setCoords( _marker.x(), _marker.y(), _marker.x(), _marker.y() );
	
	KSpreadUndoCellLayout *undo;
	if ( !m_pDoc->undoBuffer()->isLocked() )
	{
	    undo = new KSpreadUndoCellLayout( m_pDoc, this, r );
	    m_pDoc->undoBuffer()->appendUndo( undo );
	}

	for ( int x = r.left(); x <= r.right(); x++ )
	    for ( int y = r.top(); y <= r.bottom(); y++ )
	    {		
	      KSpreadCell *cell = cellAt( x, y );
	      
	      if ( cell == m_pDefaultCell )
	      {
		cell = new KSpreadCell( this, x, y );
		int key = y + ( x * 0xFFFF );
		m_dctCells.insert( key, cell );
	      }

	      if ( _font )
		cell->setTextFontFamily( _font );
	      if ( _size > 0 )
		cell->setTextFontSize( _size );
	      if ( _italic >= 0 )
		cell->setTextFontItalic( !cell->textFontItalic() );
	      if ( _bold >= 0 )
		cell->setTextFontBold( !cell->textFontBold() );
	    }
	
	emit sig_updateView( r );
    }
}

void KSpreadTable::setSelectionPercent( const QPoint &_marker )
{
    m_pDoc->setModified( true );

    bool selected = ( m_rctSelection.left() != 0 );
    
    // Complete rows selected ?
    if ( selected && m_rctSelection.right() == 0x7FFF )
    {
      // TODO
    }
    // Complete columns selected ?
    else if ( selected && m_rctSelection.bottom() == 0x7FFF )
    {
      // TODO
    }
    else
    {
	QRect r( m_rctSelection );
	if ( !selected )
	    r.setCoords( _marker.x(), _marker.y(), _marker.x(), _marker.y() );

	KSpreadUndoCellLayout *undo;
	if ( !m_pDoc->undoBuffer()->isLocked() )
	{
	    undo = new KSpreadUndoCellLayout( m_pDoc, this, r );
	    m_pDoc->undoBuffer()->appendUndo( undo );
	}
	
	for ( int x = r.left(); x <= r.right(); x++ )
	    for ( int y = r.top(); y <= r.bottom(); y++ )
	    {		
		KSpreadCell *cell = cellAt( x, y );
		
		if ( cell == m_pDefaultCell )
		{
		    cell = new KSpreadCell( this, x, y );
		    int key = y + ( x * 0xFFFF );
		    m_dctCells.insert( key, cell );
		}

		cell->setFaktor( 100.0 );
		cell->setPrecision( 0 );
		cell->setPostfix( "%" );
		cell->setPrefix( "" );
	    }
	
	emit sig_updateView( r );
    }
}

void KSpreadTable::setSelectionMultiRow( const QPoint &_marker)
{
    m_pDoc->setModified( true );
    
    bool selected = ( m_rctSelection.left() != 0 );
    
    // Complete rows selected ?
    if ( selected && m_rctSelection.right() == 0x7FFF )
    {
      // TODO
    }
    // Complete columns selected ?
    else if ( selected && m_rctSelection.bottom() == 0x7FFF )
    {
      // TODO
    }
    else
    {
	QRect r( m_rctSelection );
	if ( !selected )
	    r.setCoords( _marker.x(), _marker.y(), _marker.x(), _marker.y() );

	KSpreadUndoCellLayout *undo;
	if ( !m_pDoc->undoBuffer()->isLocked() )
	{
	    undo = new KSpreadUndoCellLayout( m_pDoc, this, r );
	    m_pDoc->undoBuffer()->appendUndo( undo );
	}
	
	for ( int x = r.left(); x <= r.right(); x++ )
	    for ( int y = r.top(); y <= r.bottom(); y++ )
	    {		
		KSpreadCell *cell = cellAt( x, y );
		
		if ( cell == m_pDefaultCell )
		{
		    cell = new KSpreadCell( this, x, y );
		    int key = y + ( x * 0xFFFF );
		    m_dctCells.insert( key, cell );
		}

		cell->setMultiRow( !cell->multiRow() );
	    }

	emit sig_updateView( r );
    }
}

void KSpreadTable::setSelectionAlign( const QPoint &_marker, KSpreadLayout::Align _align )
{
    m_pDoc->setModified( true );
    
    bool selected = ( m_rctSelection.left() != 0 );
    
    // Complete rows selected ?
    if ( selected && m_rctSelection.right() == 0x7FFF )
    {
    }
    // Complete columns selected ?
    else if ( selected && m_rctSelection.bottom() == 0x7FFF )
    {
    }
    else
    {
	QRect r( m_rctSelection );
	if ( !selected )
	    r.setCoords( _marker.x(), _marker.y(), _marker.x(), _marker.y() );

	KSpreadUndoCellLayout *undo;
	if ( !m_pDoc->undoBuffer()->isLocked() )
	{
	    undo = new KSpreadUndoCellLayout( m_pDoc, this, r );
	    m_pDoc->undoBuffer()->appendUndo( undo );
	}
	
	for ( int x = r.left(); x <= r.right(); x++ )
	    for ( int y = r.top(); y <= r.bottom(); y++ )
	    {		
		KSpreadCell *cell = cellAt( x, y );
    
		if ( cell == m_pDefaultCell )
		{
		    cell = new KSpreadCell( this, x, y );
		    int key = y + ( x * 0xFFFF );
		    m_dctCells.insert( key, cell );
		}

		cell->setAlign( _align );
	    }

	emit sig_updateView( r );
    }
}

void KSpreadTable::setSelectionPrecision( const QPoint &_marker, int _delta )
{
    m_pDoc->setModified( true );
    
    bool selected = ( m_rctSelection.left() != 0 );
    
    // Complete rows selected ?
    if ( selected && m_rctSelection.right() == 0x7FFF )
    {
      // TODO
    }
    // Complete columns selected ?
    else if ( selected && m_rctSelection.bottom() == 0x7FFF )
    {
      // TODO
    }
    else
    {
	QRect r( m_rctSelection );
	if ( !selected )
	    r.setCoords( _marker.x(), _marker.y(), _marker.x(), _marker.y() );

	KSpreadUndoCellLayout *undo;
	if ( !m_pDoc->undoBuffer()->isLocked() )
	{
	    undo = new KSpreadUndoCellLayout( m_pDoc, this, r );
	    m_pDoc->undoBuffer()->appendUndo( undo );
	}
	
	for ( int x = r.left(); x <= r.right(); x++ )
	    for ( int y = r.top(); y <= r.bottom(); y++ )
	    {		
		KSpreadCell *cell = cellAt( x, y );
    
		if ( cell == m_pDefaultCell )
		{
		    cell = new KSpreadCell( this, x, y );
		    int key = y + ( x * 0xFFFF );
		    m_dctCells.insert( key, cell );
		}

		if ( _delta == 1 )
		{
		    if ( cell->precision() >= 0 )
			cell->setPrecision( cell->precision() + 1 );
		    else
			cell->setPrecision( 0 );
		}
		else if ( cell->precision() >= 0 )
		    cell->setPrecision( cell->precision() - 1 );
	    }

	emit sig_updateView( r );
    }
}

void KSpreadTable::setSelectionMoneyFormat( const QPoint &_marker )
{
    m_pDoc->setModified( true );
    
    bool selected = ( m_rctSelection.left() != 0 );
    
    // Complete rows selected ?
    if ( selected && m_rctSelection.right() == 0x7FFF )
    {
      // TODO
    }
    // Complete columns selected ?
    else if ( selected && m_rctSelection.bottom() == 0x7FFF )
    {
      // TODO
    }
    else
    {
	QRect r( m_rctSelection );
	if ( !selected )
	    r.setCoords( _marker.x(), _marker.y(), _marker.x(), _marker.y() );

	KSpreadUndoCellLayout *undo;
	if ( !m_pDoc->undoBuffer()->isLocked() )
	{
	    undo = new KSpreadUndoCellLayout( m_pDoc, this, r );
	    m_pDoc->undoBuffer()->appendUndo( undo );
	}
	
	for ( int x = r.left(); x <= r.right(); x++ )
	    for ( int y = r.top(); y <= r.bottom(); y++ )
	    {		
		KSpreadCell *cell = cellAt( x, y );
    
		if ( cell == m_pDefaultCell )
		{
		    cell = new KSpreadCell( this, x, y );
		    int key = y + ( x * 0xFFFF );
		    m_dctCells.insert( key, cell );
		}

		cell->setPostfix( " DM" );
		cell->setPrefix( "" );
		cell->setPrecision( 2 );
	    }

	emit sig_updateView( r );
    }
}


void KSpreadTable::insertRow( int _row )
{
    KSpreadUndoInsertRow *undo;
    if ( !m_pDoc->undoBuffer()->isLocked() )
    {
	undo = new KSpreadUndoInsertRow( m_pDoc, this, _row );
	m_pDoc->undoBuffer()->appendUndo( undo );
    }

    m_dctCells.setAutoDelete( FALSE );
    m_dctRows.setAutoDelete( FALSE );
    
    QIntDictIterator<KSpreadCell> it( m_dctCells );
    int max_row = 1;
    for ( ; it.current(); ++it ) 
	if ( it.current()->row() > max_row )
	    max_row = it.current()->row();
    
    for ( int i = max_row; i >= _row; i-- )
    {
	it.toFirst();
	for ( ; it.current(); ++it ) 
	{   
	    if ( it.current()->row() == i && !it.current()->isDefault() )
	    {
		int key = it.current()->row() + ( it.current()->column() * 0xFFFF );
		m_dctCells.remove( key );
	
		it.current()->setRow( it.current()->row() + 1 );
		
		key = it.current()->row() + ( it.current()->column() * 0xFFFF );
		m_dctCells.insert( key, it.current() );
	    }
	}
    }

    QIntDictIterator<RowLayout> it2( m_dctRows );
    max_row = 1;
    for ( ; it2.current(); ++it2 ) 
	if ( it2.current()->row() > max_row )
	    max_row = it2.current()->row();
    for ( int i = max_row; i >= _row; i-- )
    {
	it2.toFirst();
	for ( ; it2.current(); ++it2 ) 
	{
	    if ( it2.current()->row() == i )
	    {
		int key = it2.current()->row();
		m_dctRows.remove( key );
		
		it2.current()->setRow( it2.current()->row() + 1 );
		
		key = it2.current()->row();
		m_dctRows.insert( key, it2.current() );
	    }
	}
    }

    m_dctCells.setAutoDelete( TRUE );
    m_dctRows.setAutoDelete( TRUE );

    emit sig_updateView();
    emit sig_updateHBorder();
    emit sig_updateVBorder();
    
    /*
    if ( pGui )
    {
	drawVisibleCells( TRUE );
	pGui->hBorderWidget()->repaint();
	pGui->vBorderWidget()->repaint();    
    } */
}

void KSpreadTable::deleteRow( int _row )
{    
    KSpreadUndoDeleteRow *undo = 0L;
    if ( !m_pDoc->undoBuffer()->isLocked() )
    {
	undo = new KSpreadUndoDeleteRow( m_pDoc, this, _row );
	m_pDoc->undoBuffer()->appendUndo( undo  );
    }
    
    m_dctCells.setAutoDelete( FALSE );
    m_dctRows.setAutoDelete( FALSE );
    
    // QStack<KSpreadCell> st;
    // st.setAutoDelete( FALSE );
    
    QIntDictIterator<KSpreadCell> it( m_dctCells );
    for ( ; it.current(); ++it ) 
    {
	int key = it.current()->row() + ( it.current()->column() * 0xFFFF );

	if ( it.current()->row() == _row && !it.current()->isDefault() )
	{
	    KSpreadCell *cell = it.current();
	    m_dctCells.remove( key );
	    // st.push( cell );
	    if ( undo )
	      undo->appendCell( cell );
	    else
	      delete cell;
	}
    }

    it.toFirst();    
    int max_row = 1;
    for ( ; it.current(); ++it ) 
	if ( it.current()->row() > max_row )
	    max_row = it.current()->row();
    for ( int i = _row + 1; i <= max_row; i++ )
    {
	it.toFirst();
	for ( ; it.current(); ++it ) 
	{
	    if ( it.current()->row() == i && !it.current()->isDefault() )
	    {
		int key = it.current()->row() + ( it.current()->column() * 0xFFFF );
		m_dctCells.remove( key );
		
		it.current()->setRow( it.current()->row() - 1 );
		
		key = it.current()->row() + ( it.current()->column() * 0xFFFF );
		m_dctCells.insert( key, it.current() );
	    }
	}
    }
    
    QStack<RowLayout> st2;
    st2.setAutoDelete( FALSE );
    
    QIntDictIterator<RowLayout> it2( m_dctRows );
    for ( ; it2.current(); ++it2 ) 
    {
	int key = it2.current()->row();
	if ( it2.current()->row() == _row && !it2.current()->isDefault() )
	{
	    RowLayout *l = it2.current();
	    m_dctRows.remove( key );
	    // st2.push( l );
	    if ( undo )
	      undo->setRowLayout( l );
	    else
	      delete l;
	}
    }

    it2.toFirst();
    max_row = 1;
    for ( ; it2.current(); ++it2 ) 
	if ( it2.current()->row() > max_row )
	    max_row = it2.current()->row();
    for ( int i = _row + 1; i <= max_row; i++ )
    {
	it2.toFirst();
	for ( ; it2.current(); ++it2 ) 
	{
	    if ( it2.current()->row() == i && !it2.current()->isDefault() )
	    {
		int key = it2.current()->row();
		m_dctRows.remove( key );
		
		it2.current()->setRow( it2.current()->row() - 1 );
		
		key = it2.current()->row();
		m_dctRows.insert( key, it2.current() );
	    }
	}
    }

    m_dctCells.setAutoDelete( true );
    m_dctRows.setAutoDelete( true );
    
    /* while ( !st.isEmpty() )
    {
	if ( undo )
	    objectList.removeRef( st.pop() );
	else
	{
	    objectList.removeRef( st.top() );
	    delete st.pop();
	}
    }
    while ( !st2.isEmpty() )
    {
	if ( undo )
	    rowList.removeRef( st2.pop() );
	else
	{
	    rowList.removeRef( st2.top() );
	    delete st2.pop();
	}
    } */

    emit sig_updateView();
    emit sig_updateHBorder();
    emit sig_updateVBorder();

    /*
    if ( pGui )
    {
	drawVisibleCells( TRUE );
	pGui->hBorderWidget()->repaint();
	pGui->vBorderWidget()->repaint();    
    } */
}

void KSpreadTable::insertColumn( int _column )
{
    KSpreadUndoInsertColumn *undo;
    if ( !m_pDoc->undoBuffer()->isLocked() )
    {
	undo = new KSpreadUndoInsertColumn( m_pDoc, this, _column );
	m_pDoc->undoBuffer()->appendUndo( undo  );
    }
    
    m_dctCells.setAutoDelete( FALSE );
    m_dctColumns.setAutoDelete( FALSE );
    
    QIntDictIterator<KSpreadCell> it( m_dctCells );
    int max_column = 1;
    for ( ; it.current(); ++it ) 
	if ( it.current()->column() > max_column )
	    max_column = it.current()->column();
    
    for ( int i = max_column; i >= _column; i-- )
    {
	it.toFirst();
	for ( ; it.current(); ++it ) 
	{   
	    if ( it.current()->column() == i && !it.current()->isDefault() )
	    {
		int key = it.current()->row() + ( it.current()->column() * 0xFFFF );
		m_dctCells.remove( key );
	
		it.current()->setColumn( it.current()->column() + 1 );
		
		key = it.current()->row() + ( it.current()->column() * 0xFFFF );
		m_dctCells.insert( key, it.current() );
	    }
	}
    }

    QIntDictIterator<ColumnLayout> it2( m_dctColumns );
    max_column = 1;
    for ( ; it2.current(); ++it2 ) 
	if ( it2.current()->column() > max_column )
	    max_column = it2.current()->column();
    for ( int i = max_column; i >= _column; i-- )
    {
	it2.toFirst();
	for ( ; it2.current(); ++it2 ) 
	{
	    if ( it2.current()->column() == i )
	    {
		int key = it2.current()->column();
		m_dctColumns.remove( key );
		
		it2.current()->setColumn( it2.current()->column() + 1 );
		
		key = it2.current()->column();
		m_dctColumns.insert( key, it2.current() );
	    }
	}
    }

    m_dctCells.setAutoDelete( TRUE );
    m_dctColumns.setAutoDelete( TRUE );

    emit sig_updateView();
    emit sig_updateHBorder();
    emit sig_updateVBorder();
    /*
    if ( pGui )
    {
	drawVisibleCells( TRUE );
	pGui->hBorderWidget()->repaint();
	pGui->vBorderWidget()->repaint();    
    } */
}

void KSpreadTable::deleteColumn( int _column )
{    
    KSpreadUndoDeleteColumn *undo = 0L;
    if ( !m_pDoc->undoBuffer()->isLocked() )
    {
	undo = new KSpreadUndoDeleteColumn( m_pDoc, this, _column );
	m_pDoc->undoBuffer()->appendUndo( undo  );
    }
    
    m_dctCells.setAutoDelete( FALSE );
    m_dctColumns.setAutoDelete( FALSE );
    
    // QStack<KSpreadCell> st;
    // st.setAutoDelete( FALSE );
    
    QIntDictIterator<KSpreadCell> it( m_dctCells );
    for ( ; it.current(); ++it ) 
    {
	int key = it.current()->row() + ( it.current()->column() * 0xFFFF );

	if ( it.current()->column() == _column && !it.current()->isDefault() )
	{
	    KSpreadCell *cell = it.current();
	    m_dctCells.remove( key );
	    // st.push( cell );
	    if ( undo )
	      undo->appendCell( cell );
	    else
	      delete cell;
	}
    }

    it.toFirst();    
    int max_column = 1;
    for ( ; it.current(); ++it ) 
	if ( it.current()->column() > max_column )
	    max_column = it.current()->column();
    for ( int i = _column + 1; i <= max_column; i++ )
    {
	it.toFirst();
	for ( ; it.current(); ++it ) 
	{
	    if ( it.current()->column() == i && !it.current()->isDefault() )
	    {
		int key = it.current()->row() + ( it.current()->column() * 0xFFFF );
		m_dctCells.remove( key );
		
		it.current()->setColumn( it.current()->column() - 1 );
		
		key = it.current()->row() + ( it.current()->column() * 0xFFFF );
		m_dctCells.insert( key, it.current() );
	    }
	}
    }
    
    // QStack<ColumnLayout> st2;
    // st2.setAutoDelete( FALSE );
    
    QIntDictIterator<ColumnLayout> it2( m_dctColumns );
    for ( ; it2.current(); ++it2 ) 
    {
	int key = it2.current()->column();
	if ( it2.current()->column() == _column && !it2.current()->isDefault() )
	{
	    ColumnLayout *l = it2.current();
	    m_dctColumns.remove( key );
	    // st2.push( l );
	    if ( undo )
	      undo->setColumnLayout( l );
	    else
	      delete l;
	}
    }

    it2.toFirst();
    max_column = 1;
    for ( ; it2.current(); ++it2 ) 
	if ( it2.current()->column() > max_column )
	    max_column = it2.current()->column();
    for ( int i = _column + 1; i <= max_column; i++ )
    {
	it2.toFirst();
	for ( ; it2.current(); ++it2 ) 
	{
	    if ( it2.current()->column() == i && !it2.current()->isDefault() )
	    {
		int key = it2.current()->column();
		m_dctColumns.remove( key );
		
		it2.current()->setColumn( it2.current()->column() - 1 );
		
		key = it2.current()->column();
		m_dctColumns.insert( key, it2.current() );
	    }
	}
    }

    m_dctCells.setAutoDelete( TRUE );
    m_dctColumns.setAutoDelete( TRUE );
    
    /* while ( !st.isEmpty() )
    {
	if ( undo )
	    objectList.removeRef( st.pop() );
	else
	{
	    objectList.removeRef( st.top() );
	    delete st.pop();
	}
    }
    while ( !st2.isEmpty() )
    {
	if ( undo )
	    columnList.removeRef( st2.pop() );
	else
	{
	    columnList.removeRef( st2.top() );
	    delete st2.pop();
	}
    } */

    emit sig_updateView();
    emit sig_updateHBorder();
    emit sig_updateVBorder();
    /*
    if ( pGui )
    {
	drawVisibleCells( TRUE );
	pGui->hBorderWidget()->repaint();
	pGui->vBorderWidget()->repaint();    
    } */
}

void KSpreadTable::copySelection( const QPoint &_marker )
{   
  /*    QBuffer device( m_pDoc->shell()->clipboard() );
    device.open( IO_WriteOnly );

    KorbSession *korb = new KorbSession( &device );
    OBJECT o_root = 0;
    
    // No selection ? => copy active cell
    if ( m_rctSelection.left() == 0 )
    {
	o_root = saveCells( korb, markerColumn, markerRow, markerColumn, markerRow );
    }
    else if ( m_rctSelection.right() == 0x7fff )
    {
    }
    else if ( m_rctSelection.bottom() == 0x7fff )
    {
    }
    else
    {
	o_root = saveCells( korb, m_rctSelection.left(), m_rctSelection.top(), m_rctSelection.right(), m_rctSelection.bottom() );
    }

    if ( o_root != 0 )
	korb->setRootCell( o_root );

    korb->release();
    delete korb;
    device.close(); */
}

void KSpreadTable::cutSelection( const QPoint &_marker )
{
    m_pDoc->setModified( true );

    copySelection( _marker );
    deleteSelection( _marker );
}

void KSpreadTable::paste( const QPoint &_marker )
{
  /* if ( m_pDoc->shell()->clipboard().isEmpty() )
	return;

    loadCells( m_pDoc->shell()->clipboard(), markerColumn, markerRow );
    
    m_pDoc->setModified( true );

    if ( pGui )
	pGui->canvasWidget()->repaint(); */
}

void KSpreadTable::loadCells( QByteArray &_array, int _column, int _row )
{
  /* QBuffer device( _array );
    device.open( IO_ReadOnly );

    KorbSession *korb = new KorbSession( &device );
    
    OBJECT o_root = korb->rootCell();
    if ( o_root == 0 )
    {
	printf("ERROR: No root object\n");
	return;
    }

    TYPE t_cellRectangle = korb->registerType( "KDE:kxcl:CellRectangle" );
    if ( t_cellRectangle && korb->cellType( o_root ) == t_cellRectangle )
    {
	loadCells( korb, o_root, _column, _row );
    }

    korb->release();
    delete korb;
    device.close();    

    setKSpreadLayoutDirtyFlag();
    setDisplayDirtyFlag(); */
}

void KSpreadTable::deleteCells( int _left, int _top, int _right, int _bottom )
{
    // A list of all cells we want to delete.
    QStack<KSpreadCell> cellStack;
    cellStack.setAutoDelete( TRUE );

    QIntDictIterator<KSpreadCell> it( m_dctCells );
    for ( ; it.current(); ++it ) 
    {
	if ( !it.current()->isDefault() && it.current()->row() >= _top &&
	     it.current()->row() <= _bottom && it.current()->column() >= _left &&
	     it.current()->column() <= _right )
	  cellStack.push( it.current() );
    }

    while ( !cellStack.isEmpty() )
    {
	KSpreadCell *cell = cellStack.pop();
	
	int key = cell->row() + ( cell->column() * 0xFFFF );
	m_dctCells.remove( key );
    }

    setLayoutDirtyFlag();

    QIntDictIterator<KSpreadCell> it2( m_dctCells );
    for ( ; it2.current(); ++it2 )
      if ( it2.current()->isForceExtraCells() && !it2.current()->isDefault() )
	it2.current()->forceExtraCells( it2.current()->column(), it2.current()->row(),
					it2.current()->extraXCells(), it2.current()->extraYCells() );
}

void KSpreadTable::deleteSelection( const QPoint &_marker )
{
    m_pDoc->setModified( true );

    if ( m_rctSelection.left() == 0 )
    {
	KSpreadUndoDelete *undo;
	if ( !m_pDoc->undoBuffer()->isLocked() )
	{
	    QRect r;
	    r.setCoords( _marker.x(), _marker.y(), _marker.x(), _marker.y() );
	    undo = new KSpreadUndoDelete( m_pDoc, this, r );
	    m_pDoc->undoBuffer()->appendUndo( undo );
	}

	deleteCells( _marker.x(), _marker.y(), _marker.x(), _marker.y() );
    }
    else if ( m_rctSelection.right() == 0x7fff )
    {
      // TODO
    }
    else if ( m_rctSelection.bottom() == 0x7fff )
    {
      // TODO
    }
    else
    {
	KSpreadUndoDelete *undo;
	if ( !m_pDoc->undoBuffer()->isLocked() )
	{
	    QRect r;
	    r.setCoords( m_rctSelection.left(), m_rctSelection.top(), m_rctSelection.right(), m_rctSelection.bottom() );
	    undo = new KSpreadUndoDelete( m_pDoc, this, r );
	    m_pDoc->undoBuffer()->appendUndo( undo );
	}
	
	deleteCells( m_rctSelection.left(), m_rctSelection.top(), m_rctSelection.right(), m_rctSelection.bottom() );	
    }
    
    emit sig_updateView();
    /* if ( pGui )
	pGui->canvasWidget()->repaint(); */
}
    
void KSpreadTable::print( QPainter &painter, bool _asChild, QPrinter *_printer )
{
    unsigned int pages = 1;

    QRect cell_range;
    cell_range.setCoords( 1, 1, 1, 1 );

    // Find maximum right/bottom cell with content
    QIntDictIterator<KSpreadCell> it( m_dctCells );
    for ( ; it.current(); ++it ) 
    {
	if ( it.current()->column() > cell_range.right() )
	    cell_range.setRight( it.current()->column() );
	if ( it.current()->row() > cell_range.bottom() )
	    cell_range.setBottom( it.current()->row() );
    }

    QList<QRect> page_list;
    page_list.setAutoDelete( TRUE );
    
    // Calculate the printable area
    // QRect rect = painter.window();
    QRect rect( (int)(MM_TO_POINT * m_pDoc->leftBorder() ), (int)(MM_TO_POINT * m_pDoc->topBorder() ),
		(int)(MM_TO_POINT * m_pDoc->printableWidth() ),
		(int)(MM_TO_POINT * m_pDoc->printableHeight() ) );

    if ( _asChild )
	rect.setCoords( 0, 0, (int)m_pDoc->printableWidth(), (int)m_pDoc->printableHeight() );
    
    // Up to this row everything is already printed
    int bottom = 0;
    // Start of the next page
    int top = 1;
    // Calculate all pages, but if we are embedded, print only the first one
    while ( bottom < cell_range.bottom() && ( page_list.count() == 0 || !_asChild ) )
    {
	// Up to this column everything is already printed
	int right = 0;
	// Start of the next page
	int left = 1;
	while ( right < cell_range.right() )
	{
	    QRect *page_range = new QRect;
	    page_list.append( page_range );
	    page_range->setLeft( left );
	    page_range->setTop( top );

	    int col = left;
	    int x = columnLayout( col )->width();
	    while ( x < rect.width() )
	    {
		col++;
		x += columnLayout( col )->width();
	    }
	    // We want to print at least one column
	    if ( col == left )
		col = left + 1;
	    page_range->setRight( col - 1 );
	    
	    int row = top;
	    int y = rowLayout( row )->height();
	    while ( y < rect.height() )
	    {
		row++;
		y += rowLayout( row )->height();
	    }
	    // We want to print at least one row
	    if ( row == top )
		row = top + 1;
	    page_range->setBottom( row - 1 );

	    right = page_range->right();
	    left = page_range->right() + 1;
	    bottom = page_range->bottom();
	}
	
	top = bottom + 1;
    }

    int pagenr = 1;
    
    // Print all pages in the list
    QRect *p;
    for ( p = page_list.first(); p != 0L; p = page_list.next() )
    {
	// print head line only if we are not embedded
	if ( !_asChild )
	{
	    QFont font( "Times", 10 );
	    painter.setFont( font );
	    QFontMetrics fm = painter.fontMetrics();
	    int w = fm.width( m_pDoc->headLeft( pagenr, m_strName ) );
	    if ( w > 0 )
		painter.drawText( (int)( MM_TO_POINT * m_pDoc->leftBorder() ),
				  (int)( MM_TO_POINT * 10.0 ), m_pDoc->headLeft( pagenr, m_strName ) );
	    w = fm.width( m_pDoc->headMid( pagenr, m_strName ) );
	    if ( w > 0 )
		painter.drawText( (int)( MM_TO_POINT * m_pDoc->leftBorder() +
					 ( MM_TO_POINT * m_pDoc->printableWidth() - (float)w ) / 2.0 ),
				  (int)( MM_TO_POINT * 10.0 ), m_pDoc->headMid( pagenr, m_strName ) );
	    w = fm.width( m_pDoc->headRight( pagenr, m_strName ) );
	    if ( w > 0 )
		painter.drawText( (int)( MM_TO_POINT * m_pDoc->leftBorder() +
					 MM_TO_POINT * m_pDoc->printableWidth() - (float)w ),
				  (int)( MM_TO_POINT * 10.0 ), m_pDoc->headRight( pagenr, m_strName ) );
	    
	    // print foot line
	    w = fm.width( m_pDoc->footLeft( pagenr, m_strName ) );
	    if ( w > 0 )
		painter.drawText( (int)( MM_TO_POINT * m_pDoc->leftBorder() ),
				  (int)( MM_TO_POINT * ( m_pDoc->paperHeight() - 10.0 ) ),
				  m_pDoc->footLeft( pagenr, m_strName ) );
	    w = fm.width( m_pDoc->footMid( pagenr, m_strName ) );
	    if ( w > 0 )
		painter.drawText( (int)( MM_TO_POINT * m_pDoc->leftBorder() +
					 ( MM_TO_POINT * m_pDoc->printableWidth() - (float)w ) / 2.0 ),
				  (int)( MM_TO_POINT * ( m_pDoc->paperHeight() - 10.0 ) ),
				  m_pDoc->footMid( pagenr, m_strName ) );
	    w = fm.width( m_pDoc->footRight( pagenr, m_strName ) );
	    if ( w > 0 )
		painter.drawText( (int)( MM_TO_POINT * m_pDoc->leftBorder() +
					 MM_TO_POINT * m_pDoc->printableWidth() - (float)w ),
				  (int)( MM_TO_POINT * ( m_pDoc->paperHeight() - 10.0 ) ),
				  m_pDoc->footRight( pagenr, m_strName ) );
	}
	
	// Translate the coordinate system to match the left and upper border.
	// But if we are embedded, we just skip this.
	if ( !_asChild )
	    painter.translate( MM_TO_POINT * m_pDoc->leftBorder(),
			       MM_TO_POINT * m_pDoc->rightBorder() );

	// Print the page
	printPage( painter, p );
	
	// Draw all parts on this page
	/*
	int xpos = columnPos( p->left() );
	int ypos = rowPos( p->top() );
	int xpos2 = columnPos( p->right() + 1 );
	int ypos2 = rowPos( p->bottom() + 1 );
	QRect r;
	r.setCoords( xpos, ypos, xpos2, ypos2 );
	
	KPart *part;
	for ( part = partList.first(); part != 0L; part = partList.next() )
	{
	    QRect r2( part->visual()->x(), part->visual()->y(), part->visual()->width(), part->visual()->height() );
	    if ( r.intersects( r2 ) )
	    {
		painter.translate( part->visual()->x() + xOffset() - xpos,
				   part->visual()->y() + yOffset() - ypos );
		part->print( painter );
		painter.translate( -( part->visual()->x() + xOffset() - xpos ),
				   -( part->visual()->y() + yOffset() - ypos ) );
	    }
	}
	*/

	// If we are embedded, we just skip this.
	if ( !_asChild )
	    painter.translate( - MM_TO_POINT * m_pDoc->leftBorder(),
			       - MM_TO_POINT * m_pDoc->rightBorder() );

	if ( pages <= page_list.count() && !_asChild )
	    _printer->newPage();
	pagenr++;
    }
}


void KSpreadTable::printPage( QPainter &_painter, QRect *page_range )
{
    int xpos;
    int ypos = 0;
    int x,y;
        
    for ( y = page_range->top(); y <= page_range->bottom() + 1; y++ )
    {
	RowLayout *row_lay = rowLayout( y );
	xpos = 0;
 
 	for ( x = page_range->left(); x <= page_range->right() + 1; x++ )
	{
	    ColumnLayout *col_lay = columnLayout( x );

	    // painter.window();	    
	    KSpreadCell *cell = cellAt( x, y );
	    if ( y > page_range->bottom() && x > page_range->right() )
	    { /* Do nothing */ }
	    else if ( y > page_range->bottom() )
		cell->print( _painter, xpos, ypos, x, y, col_lay, row_lay, FALSE, TRUE );
	    else if ( x > page_range->right() )
		cell->print( _painter, xpos, ypos, x, y, col_lay, row_lay, TRUE, FALSE );
	    else
		cell->print( _painter, xpos, ypos, x, y, col_lay, row_lay, FALSE, FALSE ); 

	    xpos += col_lay->width();
	}

	ypos += row_lay->height();
    }
}

/*
OBJECT KSpreadTable::save( KorbSession *korb )
{
    QDataStream stream;

    // For use as values in the KSpreadCellType property
    TYPE t_table =  korb->registerType( "KDE:kxcl:KSpreadTable" );

    // Real types
    TYPE t_cellList  =  korb->registerType( "KDE:kxcl:CellList" );
    TYPE t_rlList  =  korb->registerType( "KDE:kxcl:RowLayoutList" );
    TYPE t_clList  =  korb->registerType( "KDE:kxcl:ColumnLayoutList" );
    TYPE t_pList  =  korb->registerType( "KDE:kxcl:KPartList" );

    PROPERTY p_name   = korb->registerProperty( "KDE:kxcl:Name" );
    PROPERTY p_cells = korb->registerProperty( "KDE:kxcl:Cells" );
    PROPERTY p_rl = korb->registerProperty( "KDE:kxcl:RowLayouts" );
    PROPERTY p_cl = korb->registerProperty( "KDE:kxcl:ColumnLayouts" );
    PROPERTY p_parts = korb->registerProperty( "KDE:kxcl:KParts" );

    OBJECT o_table( korb->newCell( t_table ) );

    // A list of all cell object ids.
    QStack<OBJECT> cellStack;
    cellStack.setAutoDelete( TRUE );

    // Save all cells.
    QListIterator<KSpreadCell> it( objectList );
    for ( ; it.current(); ++it ) 
    {
	if ( !it.current()->isDefault() )
	{
	    OBJECT *o_cell = new OBJECT( it.current()->save( korb ) );
	    
	    if ( *o_cell == 0 )
		return 0;
	    
	    cellStack.push( o_cell );
	}
    }

    // A list of all RowLayout object ids.
    QStack<OBJECT> rlStack;
    rlStack.setAutoDelete( TRUE );

    // Save all RowLayout objects.
    QListIterator<RowLayout> rl( rowList );
    for ( ; rl.current(); ++rl ) 
    {
	if ( !rl.current()->isDefault() )
	{
	    OBJECT *o_rl = new OBJECT( rl.current()->save( korb ) );
	    
	    if ( *o_rl == 0 )
		return 0;
	    
	    rlStack.push( o_rl );
	}
    } 

    // A list of all ColumnLayout object ids.
    QStack<OBJECT> clStack;
    clStack.setAutoDelete( TRUE );

    // Save all ColumnLayout objects.
    QListIterator<ColumnLayout> cl( columnList );
    for ( ; cl.current(); ++cl ) 
    {
	if ( !cl.current()->isDefault() )
	{
	    OBJECT *o_cl = new OBJECT( cl.current()->save( korb ) );
	    
	    if ( *o_cl == 0 )
		return 0;
	    
	    clStack.push( o_cl );
	}
    } 

    // A list of all KParts ids
    QStack<OBJECT> plStack;
    QStack<KPart> kStack;
    plStack.setAutoDelete( TRUE );

    // Save all KParts.
    QListIterator<KPart> pl( partList );
    for ( ; pl.current(); ++pl ) 
    {
	OBJECT *o_pl = new OBJECT( pl.current()->save( korb ) );
	    
	if ( *o_pl == 0 )
	    return 0;
	    
	plStack.push( o_pl );
	kStack.push( pl.current() );
    } 
    
    // Write the tables name
    korb->writeStringValue( o_table, p_name, name() );

    // Write the list of cells
    VALUE value = korb->newValue( o_table, p_cells, t_cellList );
    KorbDevice *device = korb->deviceForValue( value );
    stream.setDevice( device );
    stream << (UINT32)cellStack.count();
    printf("Writing %i cells\n",cellStack.count());
    while ( !cellStack.isEmpty() )
	stream << *( cellStack.pop() );
    stream.unsetDevice();
    korb->release( device );

    // Write the list of RowLayouts
    value = korb->newValue( o_table, p_rl, t_rlList );
    device = korb->deviceForValue( value, device );
    stream.setDevice( device );
    stream << (UINT32)rlStack.count();
    printf("Writing %i rls\n",rlStack.count());
    while ( !rlStack.isEmpty() )
	stream << *( rlStack.pop() );
    stream.unsetDevice();
    korb->release( device );

    // Write the list of ColumnLayouts
    value = korb->newValue( o_table, p_cl, t_clList );
    device = korb->deviceForValue( value, device );
    stream.setDevice( device );
    stream << (UINT32)clStack.count();
    printf("Writing %i cls\n",clStack.count());
    while ( !clStack.isEmpty() )
	stream << *( clStack.pop() );
    stream.unsetDevice();
    korb->release( device );

    // Write the list of KParts
    value = korb->newValue( o_table, p_parts, t_pList );
    device = korb->deviceForValue( value, device );
    stream.setDevice( device );
    stream << (UINT32)plStack.count();
    printf("Writing %i parts\n",plStack.count());
    while ( !plStack.isEmpty() )
    {
	stream << *( plStack.pop() );
	stream << ( kStack.top()->visual()->x() + xOffset() );
	stream << ( kStack.pop()->visual()->y() + yOffset() );
    }
    stream.unsetDevice();
    korb->release( device );
    delete device;
    
    m_pDoc->setDocumentChanged( FALSE );

    return o_table;
}

bool KSpreadTable::load( KorbSession *korb, OBJECT o_table )
{
    QDataStream stream;

    // For use as values in the KSpreadCellType property
    TYPE t_table =  korb->findType( "KDE:kxcl:KSpreadTable" );

    // Real types
    TYPE t_cellList  =  korb->findType( "KDE:kxcl:CellList" );
    TYPE t_rlList  =  korb->findType( "KDE:kxcl:RowLayoutList" );
    TYPE t_clList  =  korb->findType( "KDE:kxcl:ColumnLayoutList" );
    TYPE t_pList  =  korb->findType( "KDE:kxcl:KPartList" );

    PROPERTY p_name   = korb->findProperty( "KDE:kxcl:Name" );
    PROPERTY p_cells = korb->findProperty( "KDE:kxcl:Cells" );
    PROPERTY p_rl = korb->findProperty( "KDE:kxcl:RowLayouts" );
    PROPERTY p_cl = korb->findProperty( "KDE:kxcl:ColumnLayouts" );
    PROPERTY p_parts = korb->findProperty( "KDE:kxcl:KParts" );
    PROPERTY p_partClass = korb->findProperty( "KDE:kpart:Class" );

    PROPERTY p_isPartCell = korb->findProperty( "KDE:kxcl:IsPartCell" );

    if ( p_name == 0L )
    {
	printf("ERROR: Could not find Property KDE:kxcl:Name \n");
	return FALSE;
    }
    if ( p_cells == 0L )
    {
	printf("ERROR: Could not find Property KDE:kxcl:Cells \n");
	return FALSE;
    }
    if ( p_cl == 0L )
    {
	printf("ERROR: Could not find Property KDE:kxcl:ColumnLayouts \n");
	return FALSE;
    }
    if ( p_rl == 0L )
    {
	printf("ERROR: Could not find Property KDE:kxcl:RowLayouts \n");
	return FALSE;
    }
    if ( t_cellList == 0L )
    {
	printf("ERROR: Could not find Property KDE:kxcl:CellList \n");
	return FALSE;
    }
    if ( t_clList == 0L )
    {
	printf("ERROR: Could not find Property KDE:kxcl:ColumnLayoutList \n");
	return FALSE;
    }
    if ( t_rlList == 0L )
    {
	printf("ERROR: Could not find Property KDE:kxcl:RowLayoutList \n");
	return FALSE;
    }
    if ( t_table == 0L )
    {
	printf("ERROR: Could not find Property KDE:kxcl:KSpreadTable \n");
	return FALSE;
    }

    // Read the tables name
    QString t = korb->readStringValue( o_table, p_name );
    if ( t.isNull() )
    {
	printf("ERROR: Could not load tables name\n");
	return FALSE;
    }
    name = t.data();

    // Load all RowLayouts
    VALUE value = korb->findValue( o_table, p_rl, t_rlList );
    if ( value == 0L )
    {
	printf("Error: Error in table\n");
	return FALSE;
    }

    KorbDevice *dev = korb->deviceForValue( value );
    stream.setDevice( dev );
    UINT32 crl;
    stream >> crl;
    printf("Loading %i RowLayouts\n",crl);

    for ( UINT32 j = 0; j < crl; j++ )
    {
	OBJECT o_rl;
	stream >> o_rl;
	printf("Loading RowLayout id %i .... \n",o_rl);
	
	RowLayout *rl = new RowLayout( this, 0 );
	if ( !rl->load( korb, o_rl ) )
	{
	    printf("ERROR: while loading RowLayout\n");
	    stream.unsetDevice();
	    korb->release( dev );
	    delete dev;
	    return FALSE;
	}
	
	printf(".... Loaded RowLayout\n");
	
	m_dctRows.insert( rl->row(), rl );
    }

    // Load all ColumnLayouts
    value = korb->findValue( o_table, p_cl, t_clList );
    if ( value == 0L )
    {
	printf("Error: Error in table\n");
	return FALSE;
    }

    dev = korb->deviceForValue( value, dev );
    stream.setDevice( dev );
    UINT32 ccl;
    stream >> ccl;
    printf("Loading %i ColumnLayouts\n",ccl);

    for ( UINT32 j = 0; j < ccl; j++ )
    {
	OBJECT o_cl;
	stream >> o_cl;
	printf("Loading ColumnLayout id %i .... \n",o_cl);
	
	ColumnLayout *cl = new ColumnLayout( this, 0 );
	if ( !cl->load( korb, o_cl ) )
	{
	    printf("ERROR: while loading ColumnLayout\n");
	    stream.unsetDevice();
	    korb->release( dev );
	    delete dev;
	    return FALSE;
	}
	
	printf(".... Loaded ColumnLayout\n");
	
	m_dctColumns.insert( cl->column(), cl );
    }

    // Load all cells
    value = korb->findValue( o_table, p_cells, t_cellList );
    if ( value == 0L )
    {
	printf("Error: Error in table\n");
	return FALSE;
    }

    dev = korb->deviceForValue( value, dev );
    stream.setDevice( dev );
    UINT32 ccells;
    stream >> ccells;
    printf("Loading %i cells\n",ccells);

    for ( UINT32 j = 0; j < ccells; j++ )
    {
	OBJECT o_cell;
	stream >> o_cell;
	// printf("Loading cell id %i .... \n",o_cell);

	KSpreadCell *cell;
	bool b = FALSE;
	if ( p_isPartCell != 0L && korb->readBoolValue( o_cell, p_isPartCell, b ) && b )
	{
	    printf("Loading PartCell\n");
	    cell = new PartCell( this );
	    if ( !cell->load( korb, o_cell ) )
	    {
		printf("ERROR: while loading cell\n");
		stream.unsetDevice();
		korb->release( dev );
		delete dev;
		return FALSE;
	    }
	}
	else
	{
	    cell = new KSpreadCell( this, 0, 0 );
	    if ( !cell->load( korb, o_cell ) )
	    {
		printf("ERROR: while loading cell\n");
		stream.unsetDevice();
		korb->release( dev );
		delete dev;
		return FALSE;
	    }
	}
	
	// printf(".... Loaded cell\n");
	
	int key = cell->row() + ( cell->column() * 0xFFFF );
	m_dctCells.insert( key, cell );
    }

    // Load all KParts
    value = korb->findValue( o_table, p_parts, t_pList );
    if ( value == 0L )
    {
	printf("Error: Error in table\n");
	return FALSE;
    }

    dev = korb->deviceForValue( value, dev );
    stream.setDevice( dev );
    UINT32 p;
    stream >> p;
    printf("Loading %i KParts\n",p);

    for ( UINT32 j = 0; j < p; j++ )
    {
	OBJECT o_p;
	UINT32 x,y;
	stream >> o_p;
	stream >> x;
	stream >> y;
	printf("Loading KPart id %i .... \n",o_p);
	QString clas = korb->readStringValue( o_p, p_partClass );
	if ( clas.isNull() )
	{
	    printf("ERROR: while loading KPart\nDont know class");
	    stream.unsetDevice();
	    korb->release( dev );
	    delete dev;
	    return FALSE;
	}
	printf("KPart of class '%s'\n",clas.data());
	if ( clas == "kchart" )
	{
	    KPart *bp = m_pDoc->shell()->newPart( "kchart" );

	    ChartPart *cp = (ChartPart*)bp;
	    cp->setKSpreadTable( this );

	    if ( !cp->load( korb, o_p ) )
	    {
		printf("ERROR: while loading KPart Chart\n");
		stream.unsetDevice();
		korb->release( dev );
		delete dev;
		return FALSE;
	    }
	    
	    if ( pGui )
	    {
		cp->createVisual( pGui->canvasWidget() );
		cp->visual()->move( x, y );
		cp->visual()->show();
	    }
	    
	    appendPart( cp );
	}
	else
	{
	    KPart *bp = m_pDoc->shell()->newPart( clas.data() );
	    if ( !bp->load( korb, o_p ) )
	    {
		printf("ERROR: while loading KPart '%s'\n",clas.data());
		stream.unsetDevice();
		korb->release( dev );
		delete dev;
		return FALSE;
	    }
	    
	    if ( pGui )
	    {
		bp->createVisual( pGui->canvasWidget() );
		bp->visual()->move( x, y );
		bp->visual()->show();
	    }
	    
	    appendPart( bp );
	}
	
	printf(".... Loaded KPart\n");
    }
    
    printf("Loading done\n");
    
    stream.unsetDevice();
    korb->release( dev );
    delete dev;

    m_pDoc->setDocumentChanged( FALSE );

    return TRUE;
}

void KSpreadTable::initAfterLoading()
{
    bLoading = FALSE;
    
    setKSpreadLayoutDirtyFlag();
    setDisplayDirtyFlag();

    // Now we can do the rest of the initialization, since all objects are
    // created now => No more pointer problems
    KSpreadCell *cell;
    for ( cell = objectList.first(); cell != 0L; cell = objectList.next() )
	cell->initAfterLoading();
    
    for ( cell = objectList.first(); cell != 0L; cell = objectList.next() )
	if ( cell->isForceExtraCells() && !cell->isDefault() )
	    cell->forceExtraCells( cell->column(), cell->row(),
				  cell->extraXCells(), cell->extraYCells() );

    // Complete the loading of all KParts
    KPart *p;
    for ( p = partList.first(); p != 0L; p = partList.next() )
	p->initAfterLoading();
    
    // Update all cell bindings
    CellBinding *bind;
    for ( bind = cellBindingsList.first(); bind != 0L; bind = cellBindingsList.next() )
    {
	bind->setIgnoreChanges( FALSE );
	bind->cellChanged( 0L );
    }
}

OBJECT KSpreadTable::saveCells( KorbSession *korb, int _left, int _top, int _right, int _bottom )
{
    QDataStream stream;

    // For use as values in the KSpreadCellType property
    TYPE t_cellRectangle =  korb->registerType( "KDE:kxcl:CellRectangle" );

    // Real types
    TYPE t_cellList  =  korb->registerType( "KDE:kxcl:CellList" );

    PROPERTY p_cells = korb->registerProperty( "KDE:kxcl:Cells" );
    PROPERTY p_rows = korb->registerProperty( "KDE:kxcl:Rows" );
    PROPERTY p_columns = korb->registerProperty( "KDE:kxcl:Columns" );

    OBJECT o_cells( korb->newCell( t_cellRectangle ) );

    // A list of all cell object ids.
    QStack<OBJECT> cellStack;
    cellStack.setAutoDelete( TRUE );

    // Save all cells.
    QListIterator<KSpreadCell> it( objectList );
    for ( ; it.current(); ++it ) 
    {
	if ( !it.current()->isDefault() && it.current()->row() >= _top &&
	     it.current()->row() <= _bottom && it.current()->column() >= _left &&
	     it.current()->column() <= _right )
	{
	    OBJECT *o_cell = new OBJECT( it.current()->save( korb, _left, _top ) );
	    
	    if ( *o_cell == 0 )
		return 0;
	    
	    cellStack.push( o_cell );
	}
    }

    // Write size of rectangle
    korb->writeUIntValue( o_cells, p_rows, _bottom - _top + 1 );	
    korb->writeUIntValue( o_cells, p_columns, _right - _left + 1 );
    
    // Write the list of cells
    VALUE value = korb->newValue( o_cells, p_cells, t_cellList );
    KorbDevice *device = korb->deviceForValue( value );
    stream.setDevice( device );
    stream << (UINT32)cellStack.count();
    printf("Writing %i cells\n",cellStack.count());
    while ( !cellStack.isEmpty() )
	stream << *( cellStack.pop() );
    stream.unsetDevice();
    korb->release( device );

    delete device;
    
    return o_cells;
}

bool KSpreadTable::loadCells( KorbSession *korb, OBJECT o_cells, int _insert_x, int _insert_y )
{
    QDataStream stream;

    TYPE t_cellList  =  korb->findType( "KDE:kxcl:CellList" );

    PROPERTY p_cells = korb->findProperty( "KDE:kxcl:Cells" );
    PROPERTY p_rows = korb->findProperty( "KDE:kxcl:Rows" );
    PROPERTY p_columns = korb->findProperty( "KDE:kxcl:Columns" );

    PROPERTY p_isPartCell = korb->findProperty( "KDE:kxcl:IsPartCell" );

    if ( p_cells == 0L )
    {
	printf("ERROR: Could not find Property KDE:kxcl:Cells \n");
	return FALSE;
    }
    if ( p_rows == 0L )
    {
	printf("ERROR: Could not find Property KDE:kxcl:Rows \n");
	return FALSE;
    }
    if ( p_columns == 0L )
    {
	printf("ERROR: Could not find Property KDE:kxcl:Columns \n");
	return FALSE;
    }
    if ( t_cellList == 0L )
    {
	printf("ERROR: Could not find Property KDE:kxcl:CellList \n");
	return FALSE;
    }

    unsigned int columns, rows;
    
    if ( !korb->readUIntValue( o_cells, p_rows, rows ) )
    {
	printf("ERROR: Could not find rows\n");
	return FALSE;
    }
    else if ( !korb->readUIntValue( o_cells, p_columns, columns ) )
    {
	printf("ERROR: Could not find columns\n");
	return FALSE;
    }

    // Delete the cells we want to overwrite
    deleteCells( _insert_x, _insert_y, _insert_x + columns - 1, _insert_y + rows - 1 );

    // Load all cells
    VALUE value = korb->findValue( o_cells, p_cells, t_cellList );
    if ( value == 0L )
    {
	printf("Error: Error in table\n");
	return FALSE;
    }

    bLoading = TRUE;
    
    KorbDevice* dev = korb->deviceForValue( value, 0L );
    stream.setDevice( dev );
    UINT32 ccells;
    stream >> ccells;
    printf("Loading %i cells\n",ccells);

    QList<KSpreadCell> list;
    list.setAutoDelete( FALSE );
    
    for ( UINT32 j = 0; j < ccells; j++ )
    {
	OBJECT o_cell;
	stream >> o_cell;
	printf("Loading cell id %i .... \n",o_cell);

	KSpreadCell *cell;
	bool b = FALSE;
	if ( p_isPartCell != 0L && korb->readBoolValue( o_cell, p_isPartCell, b ) && b )
	{
	    printf("Loading PartCell\n");
	    cell = new PartCell( this );
	    if ( !cell->load( korb, o_cell, _insert_x, _insert_y ) )
	    {
		printf("ERROR: while loading cell\n");
		stream.unsetDevice();
		korb->release( dev );
		bLoading = FALSE;
		delete dev;
		return FALSE;
	    }
	    list.append( cell );
	    
	}
	else
	{
	    cell = new KSpreadCell( this, _insert_x, _insert_y );
	    if ( !cell->load( korb, o_cell, _insert_x, _insert_y ) )
	    {
		printf("ERROR: while loading cell\n");
		stream.unsetDevice();
		korb->release( dev );
		bLoading = FALSE;
		delete dev;
		return FALSE;
	    }
	    list.append( cell );
	}
	
	printf(".... Loaded cell\n");
	
	// cell->setRow( cell->row() + _insert_y - 1 );
	// cell->setColumn( cell->column() + _insert_x - 1 );
	
	int key = cell->row() + ( cell->column() * 0xFFFF );
	m_dctCells.insert( key, cell );
    }

    bLoading = FALSE;
		
    setKSpreadLayoutDirtyFlag();
    setDisplayDirtyFlag();

    KSpreadCell *cell;
    for ( cell = list.first(); cell != 0L; cell = list.next() )
	cell->initAfterLoading();

    for ( cell = objectList.first(); cell != 0L; cell = objectList.next() )
	if ( cell->isForceExtraCells() && !cell->isDefault() )
	    cell->forceExtraCells( cell->column(), cell->row(),
				  cell->extraXCells(), cell->extraYCells() );

    // Update all cell bindings
    CellBinding *bind;
    for ( bind = cellBindingsList.first(); bind != 0L; bind = cellBindingsList.next() )
    {
	bind->setIgnoreChanges( FALSE );
	bind->cellChanged( 0L );
    }
    
    stream.unsetDevice();
    korb->release( dev );
    delete dev;

    m_pDoc->setModified( true );

    return TRUE;
}
*/

void KSpreadTable::setShowPageBorders( bool _b ) 
{
  if ( _b != m_bShowPageBorders )
    emit sig_updateView();
}

bool KSpreadTable::isOnNewPageX( int _column )
{
    int col = 1;
    float x = columnLayout( col )->mmWidth();
    while ( col <= _column )
    {
	// Should never happen
	if ( col == 0xFFFF )
	    return FALSE;

	if ( x > m_pDoc->printableWidth() )
	{
	    if ( col == _column )
		return TRUE;
	    else
		x = columnLayout( col )->mmWidth();
	}
	
	col++;
	x += columnLayout( col )->mmWidth();
    }
    
    return FALSE;
}

bool KSpreadTable::isOnNewPageY( int _row )
{
    int row = 1;
    float y = rowLayout( row )->mmHeight();
    while ( row <= _row )
    {
	// Should never happen
	if ( row == 0xFFFF )
	    return FALSE;

	if ( y > m_pDoc->printableHeight() )
	{
	    if ( row == _row )
		return TRUE;
	    else
		y = rowLayout( row )->mmHeight();
	}	
	row++;
	y += rowLayout( row )->mmHeight();
    }
    
    return FALSE;
}

void KSpreadTable::addCellBinding( CellBinding *_bind )
{
  m_lstCellBindings.append( _bind );

  m_pDoc->setModified( true );
}

void KSpreadTable::removeCellBinding( CellBinding *_bind )
{
  m_lstCellBindings.removeRef( _bind );

  m_pDoc->setModified( true );
}

const char *KSpreadTable::columnLabel(int column)
{
  if ( column <= 26 )
    sprintf( m_arrColumnLabel, "%c", 'A' + column - 1 );
  else if ( column <= 26 * 26 )
    sprintf( m_arrColumnLabel, "%c%c",'A'+((column-1)/26)-1,'A'+((column-1)%26));
  else
    strcpy( m_arrColumnLabel,"@@@");

  return m_arrColumnLabel;
}                           

/*
bool KSpreadTable::cellRectangle( const QRect& _range, QList<KSpreadCell> &_list )
{
  _list.clear();
  
  for ( int y = _range.top(); y <= _range.bottom(); y++ )
    for ( int x = _range.left(); x <= _range.right(); x++ )
      _list.append( cellAt( x, y ) );
  
  return TRUE;
}
*/

KSpreadTable* KSpreadTable::findTable( const char *_name )
{
  if ( !m_pMap )
    return 0L;
    
  return m_pMap->findTable( _name );
}

void KSpreadTable::insertCell( KSpreadCell *_cell )
{
  int key = _cell->row() + ( _cell->column() * 0xFFFF );    
  m_dctCells.replace( key, _cell );
}

void KSpreadTable::insertColumnLayout( ColumnLayout *_l )
{
  m_dctColumns.replace( _l->column(), _l );
}

void KSpreadTable::insertRowLayout( RowLayout *_l )
{
  m_dctRows.replace( _l->row(), _l );
}

KSpreadTable::~KSpreadTable()
{
  m_pPainter->end();
  delete m_pPainter;
  delete m_pWidget;
}

#include "kspread_table.moc"
