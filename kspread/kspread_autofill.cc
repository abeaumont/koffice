#include "kspread_autofill.h"
#include "kspread_cell.h"
#include "kspread_table.h"

#include <math.h>

QStrList *AutoFillSequenceItem::month = 0L;
QStrList *AutoFillSequenceItem::day = 0L;

/**********************************************************************************
 *
 * AutoFillDeltaSequence
 *
 **********************************************************************************/

AutoFillDeltaSequence::AutoFillDeltaSequence( AutoFillSequence *_first, AutoFillSequence *_next )
{
  ok = TRUE;
  sequence = 0L;
  
  if ( _first->count() != _next->count() )
  {
    ok = FALSE;
    return;
  }
  
  sequence = new QArray<double> ( _first->count() );
  
  AutoFillSequenceItem *item = _first->getFirst();
  AutoFillSequenceItem *item2 = _next->getFirst();
  int i = 0;
  // for( item = _first->getFirst(); item != 0L && item2 != 0L; item = _first->getNext() );
  for ( i = 0; i < _first->count(); i++ )
  {
    double d;
    if ( !item->getDelta( item2, d ) )
      {
	ok = FALSE;
	return;
      }
    sequence->at( i++ ) = d;
    item2 = _next->getNext();
    item = _first->getNext();
  }
}

AutoFillDeltaSequence::~AutoFillDeltaSequence()
{
  if ( sequence )
    delete sequence;
}

bool AutoFillDeltaSequence::equals( AutoFillDeltaSequence *_delta )
{
  if ( sequence == 0L )
    return FALSE;
  if ( _delta->getSequence() == 0L )
    return FALSE;
  if ( sequence->size() != _delta->getSequence()->size() )
    return FALSE;
  
  for ( unsigned int i = 0; i < sequence->size(); i++ )
  {
    if ( sequence->at( i ) != _delta->getSequence()->at( i ) )
      return FALSE;
  }
    
  return TRUE;
}

double AutoFillDeltaSequence::getItemDelta( int _pos ) 
{
  if ( sequence == 0L )
    return 0.0;
  return sequence->at( _pos );
}

/**********************************************************************************
 *
 * AutoFillSequenceItem
 *
 **********************************************************************************/

AutoFillSequenceItem::AutoFillSequenceItem( int _i )
{
    ivalue = _i;
    type = INTEGER;
}

AutoFillSequenceItem::AutoFillSequenceItem( double _d )
{
    dvalue = _d;
    type = FLOAT;
}

AutoFillSequenceItem::AutoFillSequenceItem( const char *_str )
{
    string = _str;
    type = STRING;
    
    if ( month == 0L )
    {
	month = new QStrList();
	month->append( "January" );
	month->append( "February" );
	month->append( "March" );
	month->append( "April" );
	month->append( "Mai" );
	month->append( "June" );
	month->append( "Juli" );
	month->append( "August" );
	month->append( "September" );
	month->append( "October" );
	month->append( "November" );
	month->append( "December" );
    }
    
    if ( day == 0L )
    {
	day = new QStrList();
	day->append( "Monday" );
	day->append( "Tuesday" );
	day->append( "Wednesday" );
	day->append( "Thursday" );
	day->append( "Friday" );
	day->append( "Saturday" );
	day->append( "Sunday" );
    }
    
    if ( month->find( _str ) != -1 )
    {
	type = MONTH;
	return;
    }
    
    if ( day->find( _str ) != -1 )
    {
	type = DAY;
	return;
    }

    if ( string.data()[0] == '=' )
	type = FORMULAR;
}

bool AutoFillSequenceItem::getDelta( AutoFillSequenceItem *seq, double &_delta )
{
    if ( seq->getType() != type )
	return FALSE;

    switch( type )
    {
    case INTEGER:
	_delta = (double)( seq->getIValue() - ivalue );
	return TRUE;
    case FLOAT:
	_delta = seq->getDValue() - dvalue;
	return TRUE;
    case FORMULAR:
    case STRING:
	if ( string == seq->getString() )
	{
	    _delta = 0.0;
	    return TRUE;
	}
	return FALSE;
    case MONTH:
	{
	    int i = month->find( string );
	    int j = month->find( seq->getString() );
	    int k = j;
	    if ( j < i )
		k += month->count();
	    if ( j + 1 == i )
		_delta = -1.0;
	    else
		_delta = ( double )( k - i );
	    return TRUE;
	}
    
    case DAY:
	{
	    int i = day->find( string );
	    int j = day->find( seq->getString() );
	    int k = j;
	    if ( j < i )
		k += day->count();
	    if ( j + 1 == i )
		_delta = -1.0;
	    else
		_delta = ( double )( k - i );
	    return TRUE;
	}
    default:
	return FALSE;
    }
}

QString AutoFillSequenceItem::getSuccessor( int _no, double _delta )
{
    QString erg;
   
    switch( type )
    {
    case INTEGER:
	erg.sprintf("%i", ivalue + _no * (int)_delta );
	break;
    case FLOAT:
	erg.sprintf("%i", ivalue + (float)_no * _delta );
	break;
    case FORMULAR:
    case STRING:
	erg = string.data();
	break;
    case MONTH:
	{
	    int i = month->find( string );
	    int j = i + _no * (int) _delta;
	    int k = j % month->count();
	    erg = month->at( k );
	}
        break;
    case DAY:
	{
	    int i = day->find( string );
	    int j = i + _no * (int) _delta;
	    int k = j % day->count();
	    erg = day->at( k );
	}
    }

    return QString( erg.data() );
}

/**********************************************************************************
 *
 * AutoFillSequence
 *
 **********************************************************************************/

AutoFillSequence::AutoFillSequence( KSpreadCell *_cell )
{
    sequence.setAutoDelete( TRUE );
    
    if ( _cell->isFormular() )
    {
	QString d = _cell->encodeFormular().data();
	sequence.append( new AutoFillSequenceItem( d.data() ) );
    }
    else if ( _cell->isValue() )
    {
	if ( floor( _cell->valueDouble() ) == _cell->valueDouble() )
	{
	    sequence.append( new AutoFillSequenceItem( (int)_cell->valueDouble() ) );
	}
	else
	    sequence.append( new AutoFillSequenceItem( _cell->valueDouble() ) );
    }
    else if ( _cell->text() )
	sequence.append( new AutoFillSequenceItem( _cell->text() ) );
}

bool AutoFillSequence::matches( AutoFillSequence* _seq, AutoFillDeltaSequence *_delta )
{
    AutoFillDeltaSequence delta( this, _seq );
    if ( !delta.isOk() )
	return FALSE;
    
    if ( delta.equals( _delta ) )
	 return TRUE;

    return FALSE;
}

void AutoFillSequence::fillCell( KSpreadCell *src, KSpreadCell *dest, AutoFillDeltaSequence *delta, int _block )
{
    QString erg = "";

    // Special handling for formulars
    if ( sequence.first() != 0L && sequence.first()->getType() == AutoFillSequenceItem::FORMULAR )
    {
	QString f = dest->decodeFormular( sequence.first()->getString() ).data();
	dest->setText( f.data() );
	dest->copyLayout( src );
	return;
    }
    
    AutoFillSequenceItem *item;
    int i = 0;
    for ( item = sequence.first(); item != 0L; item = sequence.next() )
	erg += item->getSuccessor( _block, delta->getItemDelta( i++ ) );

    dest->setText( erg.data() );
    dest->copyLayout( src );
}

/**********************************************************************************
 *
 * KSpreadTable
 *
 **********************************************************************************/

void KSpreadTable::autofill( QRect &src, QRect &dest )
{
    if ( src.left() == dest.left() && src.right() < dest.right() )
    {
	for ( int y = src.top(); y <= src.bottom(); y++ )
	{
	    int x;
	    QList<KSpreadCell> destList;
	    for ( x = src.right() + 1; x <= dest.right(); x++ )
		destList.append( nonDefaultCell( x, y ) );
	    QList<KSpreadCell> srcList;
	    for ( x = src.left(); x <= src.right(); x++ )
		srcList.append( cellAt( x, y ) );
	    QList<AutoFillSequence> seqList;
	    seqList.setAutoDelete( TRUE );
	    for ( x = src.left(); x <= src.right(); x++ )
		seqList.append( new AutoFillSequence( cellAt( x, y ) ) );
	    fillSequence( srcList, destList, seqList );
	}
    }

    if ( src.top() == dest.top() && src.bottom() < dest.bottom() )
    {
	for ( int x = src.left(); x <= dest.right(); x++ )
	{
	    int y;
	    QList<KSpreadCell> destList;
	    for ( y = src.bottom() + 1; y <= dest.bottom(); y++ )
		destList.append( nonDefaultCell( x, y ) );
	    QList<KSpreadCell> srcList;
	    for ( y = src.top(); y <= src.bottom(); y++ )
		srcList.append( cellAt( x, y ) );
	    QList<AutoFillSequence> seqList;
	    seqList.setAutoDelete( TRUE );
	    for ( y = src.top(); y <= src.bottom(); y++ )
		seqList.append( new AutoFillSequence( cellAt( x, y ) ) );
	    fillSequence( srcList, destList, seqList );
	}
    }
    
}

void KSpreadTable::fillSequence( QList<KSpreadCell>& _srcList, QList<KSpreadCell>& _destList, 
				 QList<AutoFillSequence>& _seqList )
{
    QList<AutoFillDeltaSequence> deltaList;
    deltaList.setAutoDelete( TRUE );

    for ( unsigned int step = 1; step <= _seqList.count() / 2; step++ )
    {
	if ( _seqList.count() % step == 0 )
	{
	    bool ok = TRUE;
	    deltaList.clear();
	    for ( unsigned int t = 0; t < step; t++ )
	    {
		deltaList.append( new AutoFillDeltaSequence( _seqList.at(t), _seqList.at(t+step) ) );
		if ( !deltaList.getLast()->isOk() )
		    ok = FALSE;
	    }
	    
	    // Testphase
	    for ( unsigned int tst = 1; ok && ( tst * step < _seqList.count() ); tst++ )
	    {
		for ( unsigned int s = 0; ok && ( s < step ); s++ )
		{
		    if ( !_seqList.at( (tst-1) * step + s )->matches( _seqList.at( tst * step + s ),
								       deltaList.at( s ) ) )
			ok = FALSE;
		}
	    }

	    if ( ok )
	    {
		KSpreadCell *cell = _destList.first();
		unsigned int s = 0;
		int block = _seqList.count() / step;
		while ( cell )
		{
		    if ( s == step )
		    {
			block++;
			s = 0;
		    }
		    _seqList.at( s )->fillCell( _srcList.at( s ), cell, deltaList.at( s ), block );
		    cell = _destList.next();
		    s++;
		}

		return;
	    }
	}
    }

    KSpreadCell *cell = _destList.first();
    unsigned int s = 0;
    while ( cell )
    {
	if ( s == _srcList.count() )
	    s = 0;

	if ( _srcList.at( s )->text() != 0L )
	{
	    if ( _srcList.at( s )->isFormular() )
	    {
		QString d = _srcList.at( s )->encodeFormular();
		cell->setText( cell->decodeFormular( d.data() ).data() );
	    }
	    else
		cell->setText( _srcList.at( s )->text() );
	}
	else
	    cell->setText( "" );
	cell->copyLayout( _srcList.at( s ) );
	cell = _destList.next();
	s++;
    }
}
