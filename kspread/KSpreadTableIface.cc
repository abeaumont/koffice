#include "KSpreadTableIface.h"
#include "KSpreadCellIface.h"
#include "KSpreadColumnIface.h"
#include "KSpreadRowIface.h"


#include "kspread_table.h"
#include "kspread_util.h"
#include "kspread_doc.h"

#include <kapplication.h>
#include <dcopclient.h>
#include <kdebug.h>

/*********************************************
 *
 * KSpreadCellProxy
 *
 *********************************************/

class KSpreadCellProxy : public DCOPObjectProxy
{
public:
    KSpreadCellProxy( KSpreadTable* table, const QCString& prefix );
    ~KSpreadCellProxy();

    virtual bool process( const QCString& obj, const QCString& fun, const QByteArray& data,
                          QCString& replyType, QByteArray &replyData );

private:
    QCString m_prefix;
    KSpreadCellIface* m_cell;
    KSpreadTable* m_table;
};

KSpreadCellProxy::KSpreadCellProxy( KSpreadTable* table, const QCString& prefix )
    : DCOPObjectProxy( kapp->dcopClient() ), m_prefix( prefix )
{
    m_cell = new KSpreadCellIface;
    m_table = table;
}

KSpreadCellProxy::~KSpreadCellProxy()
{
    delete m_cell;
}

bool KSpreadCellProxy::process( const QCString& obj, const QCString& fun, const QByteArray& data,
                                        QCString& replyType, QByteArray &replyData )
{
    if ( strncmp( m_prefix.data(), obj.data(), m_prefix.length() ) != 0 )
        return FALSE;

    KSpreadPoint p( obj.data() + m_prefix.length() );
    if ( !p.isValid() )
        return FALSE;

    m_cell->setCell( m_table, p.pos );
    return m_cell->process( fun, data, replyType, replyData );
}

/************************************************
 *
 * KSpreadTableIface
 *
 ************************************************/

KSpreadTableIface::KSpreadTableIface( KSpreadTable* t )
    : DCOPObject( t )
{
    m_table = t;

    QCString str = objId();
    str += "/";
    m_proxy = new KSpreadCellProxy( t, str );
}

KSpreadTableIface::~KSpreadTableIface()
{
    delete m_proxy;
}

DCOPRef KSpreadTableIface::cell( int x, int y )
{
    // if someone calls us with either x or y 0 he _most_ most likely doesn't
    // know that the cell counting starts with 1 (Simon)
    // P.S.: I did that mistake for weeks and already started looking for the
    // "bug" in kspread ;-)
    if ( x == 0 )
        x = 1;
    if ( y == 0 )
        y = 1;

    QCString str = objId();
    str += '/';
    str += util_cellName( x, y ).latin1();

    return DCOPRef( kapp->dcopClient()->appId(), str );
}

DCOPRef KSpreadTableIface::cell( const QString& name )
{
    QCString str = objId();
    str += "/";
    str += name.latin1();

    return DCOPRef( kapp->dcopClient()->appId(), str );
}

DCOPRef KSpreadTableIface::column( int _col )
{
    //First col number = 1
    if(_col <1)
        return DCOPRef();
    return DCOPRef( kapp->dcopClient()->appId(),
		    m_table->nonDefaultColumnLayout( _col )->dcopObject()->objId() );

}

DCOPRef KSpreadTableIface::row( int _row )
{
    //First row number = 1
    if(_row <1)
        return DCOPRef();
    return DCOPRef( kapp->dcopClient()->appId(),
		    m_table->nonDefaultRowLayout( _row )->dcopObject()->objId() );
}


QRect KSpreadTableIface::selection() const
{
    return m_table->selectionRect();
}

void KSpreadTableIface::setSelection( const QRect& selection )
{
    m_table->setSelection( selection );
}

QString KSpreadTableIface::name() const
{
    return m_table->tableName();
}


int KSpreadTableIface::maxColumn() const
{
    return m_table->maxColumn();

}

int KSpreadTableIface::maxRow() const
{
    return m_table->maxRow();
}

bool KSpreadTableIface::processDynamic( const QCString& fun, const QByteArray&/*data*/,
                                        QCString& replyType, QByteArray &replyData )
{
    kdDebug(36001) << "Calling '" << fun.data() << "'" << endl;
    // Does the name follow the pattern "foobar()" ?
    uint len = fun.length();
    if ( len < 3 )
        return FALSE;

    if ( fun[ len - 1 ] != ')' || fun[ len - 2 ] != '(' )
        return FALSE;

    // Is the function name a valid cell like "B5" ?
    KSpreadPoint p( fun.left( len - 2 ).data() );
    if ( !p.isValid() )
        return FALSE;

    QCString str = objId() + "/" + fun.left( len - 2 );

    replyType = "DCOPRef";
    QDataStream out( replyData, IO_WriteOnly );
    out << DCOPRef( kapp->dcopClient()->appId(), str );
    return TRUE;
}

bool KSpreadTableIface::setTableName( const QString & name)
{
    return m_table->setTableName( name);
}

void KSpreadTableIface::setSelectionMoneyFormat( bool b )
{
    m_table->setSelectionMoneyFormat(m_table->markerRect().topLeft (),b);
}

void KSpreadTableIface::setSelectionPrecision( int _delta )
{
    m_table->setSelectionPrecision(m_table->markerRect().topLeft (),_delta);
}

void KSpreadTableIface::setSelectionPercent( bool b )
{
    m_table->setSelectionPercent(m_table->markerRect().topLeft (),b);
}

void KSpreadTableIface::setSelectionMultiRow( bool enable )
{
    m_table->setSelectionMultiRow(m_table->markerRect().topLeft (),enable);
}

void KSpreadTableIface::setSelectionSize(int _size )
{
    m_table->setSelectionSize(m_table->markerRect().topLeft (),_size);
}

void KSpreadTableIface::setSelectionUpperLower( int _type )
{
    m_table->setSelectionUpperLower(m_table->markerRect().topLeft (),_type);
}

void KSpreadTableIface::setSelectionfirstLetterUpper( )
{
    m_table->setSelectionfirstLetterUpper(m_table->markerRect().topLeft ());
}

void KSpreadTableIface::setSelectionVerticalText( bool b)
{
    m_table->setSelectionVerticalText(m_table->markerRect().topLeft (),b);
}

void KSpreadTableIface::setSelectionComment( const QString &_comment)
{
    m_table->setSelectionComment(m_table->markerRect().topLeft (),_comment);
}

void KSpreadTableIface::setSelectionRemoveComment()
{
    m_table->setSelectionRemoveComment(m_table->markerRect().topLeft ());
}

void KSpreadTableIface::setSelectionAngle(int _value)
{
    m_table->setSelectionAngle(m_table->markerRect().topLeft (),_value);
}

void KSpreadTableIface::setSelectionTextColor( const QColor &tbColor )
{
    m_table->setSelectionTextColor(m_table->markerRect().topLeft (),tbColor);
}

void KSpreadTableIface::setSelectionbgColor( const QColor &bg_Color )
{
    m_table->setSelectionbgColor(m_table->markerRect().topLeft (),bg_Color);
}

void KSpreadTableIface::setSelectionBorderColor(const QColor &bd_Color )
{
    m_table->setSelectionBorderColor(m_table->markerRect().topLeft (),bd_Color);
}

void KSpreadTableIface::deleteSelection()
{
    m_table->deleteSelection(m_table->markerRect().topLeft());
}

void KSpreadTableIface::copySelection()
{
    m_table->copySelection(m_table->markerRect().topLeft());
}

void KSpreadTableIface::cutSelection()
{
    m_table->cutSelection(m_table->markerRect().topLeft ());
}

void KSpreadTableIface::clearTextSelection()
{
    m_table->clearTextSelection(m_table->markerRect().topLeft());
}

void KSpreadTableIface::clearValiditySelection()
{
    m_table->clearValiditySelection(m_table->markerRect().topLeft());
}

void KSpreadTableIface::clearConditionalSelection()
{
    m_table->clearConditionalSelection(m_table->markerRect().topLeft ());
}


bool KSpreadTableIface::insertColumn( int col,int nbCol )
{
    return m_table->insertColumn(col,nbCol);
}

bool KSpreadTableIface::insertRow( int row,int nbRow)
{
    return m_table->insertRow(row,nbRow);
}

void KSpreadTableIface::removeColumn( int col,int nbCol )
{
    m_table->removeColumn( col,nbCol );
}

void KSpreadTableIface::removeRow( int row,int nbRow )
{
    m_table->removeRow( row,nbRow );
}

void KSpreadTableIface::borderLeft(const QColor &_color )
{
    m_table->borderLeft(m_table->markerRect().topLeft (),_color);
}

void KSpreadTableIface::borderTop( const QColor &_color )
{
    m_table->borderTop(m_table->markerRect().topLeft (),_color);
}

void KSpreadTableIface::borderOutline( const QColor &_color )
{
    m_table->borderOutline(m_table->markerRect().topLeft (),_color);
}

void KSpreadTableIface::borderAll( const QColor &_color )
{
    m_table->borderAll(m_table->markerRect().topLeft (), _color );
}

void KSpreadTableIface::borderRemove( )
{
    m_table->borderRemove(m_table->markerRect().topLeft ());
}

void KSpreadTableIface::borderBottom(const QColor &_color )
{
    m_table->borderBottom(m_table->markerRect().topLeft (),_color);
}

void KSpreadTableIface::borderRight( const QColor &_color )
{
    m_table->borderRight( m_table->markerRect().topLeft (),_color );
}

bool KSpreadTableIface::isHidden()const
{
    return m_table->isHidden();
}

void KSpreadTableIface::increaseIndent()
{
    m_table->increaseIndent(m_table->markerRect().topLeft ());
}

void KSpreadTableIface::decreaseIndent()
{
    m_table->decreaseIndent(m_table->markerRect().topLeft ());
}

bool KSpreadTableIface::showGrid() const
{
    return m_table->getShowGrid();
}

bool KSpreadTableIface::showFormula() const
{
    return m_table->getShowFormula();
}

bool KSpreadTableIface::lcMode() const
{
    return m_table->getLcMode();
}

bool KSpreadTableIface::autoCalc() const
{
    return m_table->getAutoCalc();
}

bool KSpreadTableIface::showColumnNumber() const
{
    return m_table->getShowColumnNumber();
}

bool KSpreadTableIface::hideZero() const
{
    return m_table->getHideZero();
}

bool KSpreadTableIface::firstLetterUpper() const
{
    return m_table->getFirstLetterUpper();
}

void KSpreadTableIface::copyAsText()
{
    m_table->copyAsText(m_table->markerRect().topLeft ());
}

void KSpreadTableIface::setShowPageBorders( bool b )
{
    m_table->setShowPageBorders( b );
    m_table->doc()->updateBorderButton();
}
