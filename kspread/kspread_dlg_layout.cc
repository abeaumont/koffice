/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qprinter.h>
#include <stdio.h>
#include <stdlib.h>

#include "kspread_dlg_layout.h"
#include "kspread_undo.h"
#include "kspread_table.h"
#include "kspread_cell.h"
#include "kspread_view.h"
#include "kspread_canvas.h"

#include <qlabel.h>
#include <qpainter.h>
#include <kcolordlg.h>
#include <klocale.h>
#include <kdebug.h>
#include <kstddirs.h>

KSpreadPatternSelect::KSpreadPatternSelect( QWidget *parent, const char * ) : QFrame( parent )
{
    penStyle = NoPen;
    penWidth = 1;
    penColor = black;
    selected = FALSE;
    undefined = FALSE;
}

void KSpreadPatternSelect::setPattern( const QColor &_color, int _width, PenStyle _style )
{
    penStyle = _style;
    penColor = _color;
    penWidth = _width;
    repaint();
}

void KSpreadPatternSelect::setUndefined()
{
    undefined = TRUE;
}

void KSpreadPatternSelect::paintEvent( QPaintEvent *_ev )
{
    QFrame::paintEvent( _ev );

    QPainter painter;
    QPen pen;

    if ( !undefined )
    {
	pen.setColor( penColor );
	pen.setStyle( penStyle );
	pen.setWidth( penWidth );

	painter.begin( this );
	painter.setPen( pen );
	painter.drawLine( 3, 9, width() - 6, 9 );
	painter.end();
    }
    else
    {
	painter.begin( this );
	painter.fillRect( 2, 2, width() - 4, height() - 4, BDiagPattern );
	painter.end();
    }
}

void KSpreadPatternSelect::mousePressEvent( QMouseEvent * )
{
    slotSelect();

    emit clicked( this );
}

void KSpreadPatternSelect::slotUnselect()
{
    selected = FALSE;

    setLineWidth( 1 );
    setFrameStyle( QFrame::Panel | QFrame::Sunken );
    repaint();
}

void KSpreadPatternSelect::slotSelect()
{
    selected = TRUE;

    setLineWidth( 2 );
    setFrameStyle( QFrame::Panel | QFrame::Plain );
    repaint();
}

QPixmap* CellLayoutDlg::formatOnlyNegSignedPixmap = 0L;
QPixmap* CellLayoutDlg::formatRedOnlyNegSignedPixmap = 0L;
QPixmap* CellLayoutDlg::formatRedNeverSignedPixmap = 0L;
QPixmap* CellLayoutDlg::formatAlwaysSignedPixmap = 0L;
QPixmap* CellLayoutDlg::formatRedAlwaysSignedPixmap = 0L;
QPixmap* CellLayoutDlg::undefinedPixmap = 0L;

CellLayoutDlg::CellLayoutDlg( KSpreadView *_view, KSpreadTable *_table, int _left, int _top,
			      int _right, int _bottom ) : QObject()
{
    table = _table;
    left = _left;
    top = _top;
    right = _right;
    bottom = _bottom;
    m_pView = _view;

    KSpreadCell *obj = table->cellAt( _left, _top );

    // Initialize with the upper left object
    leftBorderStyle = obj->leftBorderStyle( _left, _top );
    leftBorderWidth = obj->leftBorderWidth( _left, _top );
    leftBorderColor = obj->leftBorderColor( _left, _top );
    topBorderStyle = obj->topBorderStyle( _left, _top );
    topBorderWidth = obj->topBorderWidth( _left, _top );
    topBorderColor = obj->topBorderColor( _left, _top );
    fallDiagonalStyle = obj->fallDiagonalStyle( _left, _top );
    fallDiagonalWidth = obj->fallDiagonalWidth( _left, _top );
    fallDiagonalColor = obj->fallDiagonalColor( _left, _top );
    goUpDiagonalStyle = obj->goUpDiagonalStyle( _left, _top );
    goUpDiagonalWidth = obj->goUpDiagonalWidth( _left, _top );
    goUpDiagonalColor = obj->goUpDiagonalColor( _left, _top );
    obj = table->cellAt( _right, _top );
    rightBorderStyle = obj->rightBorderStyle( _right, _top );
    rightBorderWidth = obj->rightBorderWidth( _right, _top );
    rightBorderColor = obj->rightBorderColor( _right, _top );
    obj = table->cellAt( _left, _bottom );
    bottomBorderStyle = obj->bottomBorderStyle( _left, _bottom );
    bottomBorderWidth = obj->bottomBorderWidth( _left, _bottom );
    bottomBorderColor = obj->bottomBorderColor( _left, _bottom );
    // Just an assumption
    obj = table->cellAt( _right, _top );

    verticalBorderStyle = obj->leftBorderStyle( _right, _top );
    verticalBorderWidth = obj->leftBorderWidth( _right, _top );
    verticalBorderColor = obj->leftBorderColor( _right, _top );
    obj = table->cellAt( _right, _bottom );
    horizontalBorderStyle = obj->topBorderStyle( _right, _bottom );
    horizontalBorderWidth = obj->topBorderWidth( _right, _bottom );
    horizontalBorderColor = obj->topBorderColor( _right, _bottom );

    obj = table->cellAt( _left, _top );
    prefix = obj->prefix();
    postfix = obj->postfix();
    precision = obj->precision();
    floatFormat = obj->floatFormat();
    floatColor = obj->floatColor();

    alignX = obj->align();
    alignY = obj->alignY();

    textColor = obj->textColor();
    bgColor = obj->bgColor( _left, _top );
    textFontSize = obj->textFontSize();
    textFontFamily = obj->textFontFamily();
    textFontBold = obj->textFontBold();
    textFontItalic = obj->textFontItalic();
    strike=obj->textFontStrike();
    underline=obj->textFontUnderline();
    // Needed to initialize the font correctly ( bug in Qt )
    textFont = obj->textFont();
    eStyle = obj->style();
    actionText = obj->action();

    brushColor = obj->backGroundBrushColor(_left,_top);
    brushStyle = obj->backGroundBrushStyle(_left,_top);

    bMultiRow = obj->multiRow();

    RowLayout *rl;
    ColumnLayout *cl;
    widthSize=0;
    heigthSize=0;
    for ( int x = _left; x <= _right; x++ )
        {
	cl = m_pView->activeTable()->columnLayout(x);
    	widthSize=QMAX(cl->width(m_pView->canvasWidget()),widthSize);
    	}

    for ( int y = _top; y <= _bottom; y++ )
	{
    	rl = m_pView->activeTable()->rowLayout(y);
    	heigthSize=QMAX(rl->height(m_pView->canvasWidget()),heigthSize);
    	}


    // We assume, that all other objects have the same values
    bLeftBorderStyle = TRUE;
    bLeftBorderColor = TRUE;
    bRightBorderStyle = TRUE;
    bRightBorderColor = TRUE;
    bTopBorderStyle = TRUE;
    bTopBorderColor = TRUE;
    bBottomBorderColor = TRUE;
    bBottomBorderStyle = TRUE;
    bVerticalBorderColor = TRUE;
    bVerticalBorderStyle = TRUE;
    bHorizontalBorderColor = TRUE;
    bHorizontalBorderStyle = TRUE;
    bFallDiagonalStyle = TRUE;
    bfallDiagonalColor = TRUE;
    bGoUpDiagonalStyle = TRUE;
    bGoUpDiagonalColor = TRUE;
    bFloatFormat = TRUE;
    bFloatColor = TRUE;
    bTextColor = TRUE;
    bBgColor = TRUE;
    bTextFontFamily = TRUE;
    bTextFontSize = TRUE;
    bTextFontBold = TRUE;
    bTextFontItalic = TRUE;
    bStrike=TRUE;
    bUnderline=TRUE;
    if( left==right)
        oneCol=TRUE;
    else
        oneCol=FALSE;

    if( top==bottom)
        oneRow=TRUE;
    else
        oneRow=FALSE;

    // Do the other objects have the same values ?
    for ( int x = _left; x <= _right; x++ )
        {
        for ( int y = _top; y <= _bottom; y++ )
	{
	    KSpreadCell *obj = table->cellAt( x, y );


	   if ( fallDiagonalStyle != obj->fallDiagonalStyle( x, y ) )
		bFallDiagonalStyle = FALSE;
	    if ( fallDiagonalWidth != obj->fallDiagonalWidth( x, y ) )
		bFallDiagonalStyle = FALSE;
	    if ( fallDiagonalColor != obj->fallDiagonalColor( x, y ) )
		bfallDiagonalColor = FALSE;
	    if ( goUpDiagonalStyle != obj->goUpDiagonalStyle( x, y ) )
		bGoUpDiagonalStyle = FALSE;
	    if ( goUpDiagonalWidth != obj->goUpDiagonalWidth( x, y ) )
		bGoUpDiagonalStyle = FALSE;
	    if ( goUpDiagonalColor != obj->goUpDiagonalColor( x, y ) )
		bGoUpDiagonalColor = FALSE;
            if ( strike != obj->textFontStrike( ) )
		bStrike = FALSE;
            if ( underline != obj->textFontUnderline() )
		bUnderline = FALSE;
	    if ( prefix != obj->prefix() )
		prefix = QString::null;
	    if ( postfix != obj->postfix() )
		postfix = QString::null;
	    if ( precision != obj->precision() )
		precision = -2;
	    if ( floatFormat != obj->floatFormat() )
		bFloatFormat = FALSE;
	    if ( floatColor != obj->floatColor() )
		bFloatColor = FALSE;
	    if ( textColor != obj->textColor() )
		bTextColor = FALSE;
	    if ( strcmp( textFontFamily.data(), obj->textFontFamily() ) != 0 )
		bTextFontFamily = FALSE;
	    if ( textFontSize != obj->textFontSize() )
		bTextFontSize = FALSE;
	    if ( textFontBold != obj->textFontBold() )
		bTextFontBold = FALSE;
	    if ( textFontItalic != obj->textFontItalic() )
		bTextFontItalic = FALSE;
	    if ( bgColor != obj->bgColor( x, y ) )
		bBgColor = FALSE;
	    if ( eStyle != obj->style() )
		eStyle = KSpreadCell::ST_Undef;
	 }
        }

    for ( int y = _top; y <= _bottom; y++ )
        {
        KSpreadCell *obj = table->cellAt( _left, y );
        if ( leftBorderStyle != obj->leftBorderStyle( _left, y ) )
		bLeftBorderStyle = FALSE;
	if ( leftBorderWidth != obj->leftBorderWidth( _left, y ) )
		bLeftBorderStyle = FALSE;
	if ( leftBorderColor != obj->leftBorderColor( _left, y ) )
		bLeftBorderColor = FALSE;
        }


    for ( int y = _top; y <= _bottom; y++ )
        {
        KSpreadCell *obj = table->cellAt( _right, y );
        if ( rightBorderStyle != obj->rightBorderStyle( _right, y ) )
		bRightBorderStyle = FALSE;
	if ( rightBorderWidth != obj->rightBorderWidth( _right, y ) )
		bRightBorderStyle = FALSE;
	if ( rightBorderColor != obj->rightBorderColor( _right, y ) )
		bRightBorderColor = FALSE;
        }

    for ( int x = _left; x <= _right; x++ )
        {
        KSpreadCell *obj = table->cellAt( x, _top );
        if (  topBorderStyle != obj->topBorderStyle( x, _top ) )
		bTopBorderStyle = FALSE;
	if ( topBorderWidth != obj->topBorderWidth( x, _top ) )
		bTopBorderStyle = FALSE;
	if ( topBorderColor != obj->topBorderColor( x, _top ) )
		bTopBorderColor = FALSE;
        }

    for ( int x = _left; x <= _right; x++ )
        {
        KSpreadCell *obj = table->cellAt( x, _bottom );
        if ( bottomBorderStyle != obj->bottomBorderStyle( x, _bottom ) )
		bBottomBorderStyle = FALSE;
	if ( bottomBorderWidth != obj->bottomBorderWidth( x, _bottom ) )
		bBottomBorderStyle = FALSE;
	if ( bottomBorderColor != obj->bottomBorderColor( x, _bottom ) )
		bBottomBorderColor = FALSE;
        }


    // Look for the Outline
    for ( int x = _left; x <= _right; x++ )
        {
        for ( int y = _top+1; y <= _bottom; y++ )
        {
	KSpreadCell *obj = table->cellAt( x, y );

	if ( horizontalBorderStyle != obj->topBorderStyle( x, y ) )
	    bHorizontalBorderStyle = FALSE;
	if ( horizontalBorderWidth != obj->topBorderWidth( x, y ) )
	    bHorizontalBorderStyle = FALSE;
	if ( horizontalBorderColor != obj->topBorderColor( x, y ) )
	    bHorizontalBorderColor = FALSE;
        }

    }

    for ( int x = _left+1; x <= _right; x++ )
    {
    for ( int y = _top; y <= _bottom; y++ )
        {

        KSpreadCell *obj = table->cellAt( x, y );

	if ( verticalBorderStyle != obj->leftBorderStyle( x, y ) )
	    bVerticalBorderStyle = FALSE;
	if ( verticalBorderWidth != obj->leftBorderWidth( x, y ) )
	    bVerticalBorderStyle = FALSE;
	if ( verticalBorderColor != obj->leftBorderColor( x, y ) )
	    bVerticalBorderColor = FALSE;
        }

    }

    init();
}

void CellLayoutDlg::init()
{
    // Did we initialize the bitmaps ?
    if ( formatOnlyNegSignedPixmap == 0L )
    {
	formatOnlyNegSignedPixmap = paintFormatPixmap( "123.456", black, "-123.456", black );
	formatRedOnlyNegSignedPixmap = paintFormatPixmap( "123.456", black, "-123.456", red );
	formatRedNeverSignedPixmap = paintFormatPixmap( "123.456", black, "123.456", black );
	formatAlwaysSignedPixmap = paintFormatPixmap( "+123.456", black, "-123.456", black );
	formatRedAlwaysSignedPixmap = paintFormatPixmap( "+123.456", black, "-123.456", red );

	// Make the undefined pixmap
        undefinedPixmap = new QPixmap( 100, 12 );
        QPainter painter;
        painter.begin( undefinedPixmap );
	painter.setBackgroundColor( white );
	painter.setPen( black );
        painter.fillRect( 0, 0, 100, 12, BDiagPattern );
        painter.end();
    }

    tab = new QTabDialog( (QWidget*)m_pView, 0L, TRUE );
    tab->setGeometry( tab->x(), tab->y(), 420, 400 );

    borderPage = new CellLayoutPageBorder( tab, this );
    tab->addTab( borderPage, i18n("Border") );

    floatPage = new CellLayoutPageFloat( tab, this );
    tab->addTab( floatPage, i18n("Number Format") );

    miscPage = new CellLayoutPageMisc( tab, this );
    tab->addTab( miscPage, i18n("Misc") );

    fontPage = new CellLayoutPageFont( tab, this );
    tab->addTab( fontPage, i18n("Font") );

    positionPage = new CellLayoutPagePosition( tab, this);
    tab->addTab( positionPage, i18n("Position") );

    patternPage=new CellLayoutPagePattern(tab,this);
    tab->addTab( patternPage,i18n("Pattern"));

    // tab->setApplyButton();
    tab->setCancelButton();

    tab->setCaption(i18n("Cell Layout"));

    connect( tab, SIGNAL( applyButtonPressed() ), this, SLOT( slotApply() ) );
    // connect( tab, SIGNAL(cancelButtonPressed()), SLOT(setup()) );

    tab->show();
}

QPixmap* CellLayoutDlg::paintFormatPixmap( const char *_string1, const QColor & _color1,
					     const char *_string2, const QColor & _color2 )
{
    QPixmap *pixmap = new QPixmap( 150, 14 );

    QPainter painter;
    painter.begin( pixmap );
    painter.fillRect( 0, 0, 150, 14, white );
    painter.setPen( _color1 );
    painter.drawText( 2, 11, _string1 );
    painter.setPen( _color2 );
    painter.drawText( 75, 11, _string2 );
    painter.end();

    return pixmap;
}

int CellLayoutDlg::exec()
{
    return ( tab->exec() );
}

void CellLayoutDlg::slotApply()
{
    // Prepare the undo buffer
    KSpreadUndoCellLayout *undo;
    if ( !table->doc()->undoBuffer()->isLocked() )
    {
	QRect rect;
	// Since the right/bottom border is stored in objects right + 1 ( or: bottom + 1 )
	// So we have to save these layouts, too
	rect.setCoords( left, top, right + 1, bottom + 1 );
	undo = new KSpreadUndoCellLayout( table->doc(), table, rect );
	table->doc()->undoBuffer()->appendUndo( undo );
    }

    for ( int x = left; x <= right; x++ )
	for ( int y = top; y <= bottom; y++ )
	{
	    KSpreadCell *obj = table->nonDefaultCell( x, y );
	    floatPage->apply( obj );
	    miscPage->apply( obj );
	    fontPage->apply( obj );
            positionPage->apply( obj );
            patternPage->apply(obj);
	}
	
    if(positionPage->getSizeHeight()!=heigthSize)
    	{
    	for ( int x = top; x <= bottom; x++ )
    		{
    		m_pView->vBorderWidget()->resizeRow(positionPage->getSizeHeight(),x );
    		}
    	}
    if(positionPage->getSizeWidth()!=widthSize)
    	{
    	for ( int x = left; x <= right; x++ )
    		{
    		m_pView->hBorderWidget()->resizeColumn(positionPage->getSizeWidth(),x );
    		}	
    	}
    	
    // Outline
      borderPage->applyOutline( left, top, right, bottom );

    // m_pView->drawVisibleCells();
    QRect r;
    r.setCoords( left, top, right, bottom );
    m_pView->slotUpdateView( table, r );
}


CellLayoutPageFloat::CellLayoutPageFloat( QWidget* parent, CellLayoutDlg *_dlg ) : QWidget ( parent )
{
    dlg = _dlg;

    postfix = new QLineEdit( this, "LineEdit_1" );
    precision = new QLineEdit ( this, "LineEdit_2" );
    prefix = new QLineEdit( this, "LineEdit_3" );
    // format = new QListBox( this, "ListBox_1" );
    format = new QComboBox( this, "ListBox_1" );

    QLabel* tmpQLabel;
    tmpQLabel = new QLabel( this, "Label_1" );
    tmpQLabel->setGeometry( 10, 30, 40, 30 );
    tmpQLabel->setText( i18n("Prefix") );

    postfix->raise();
    postfix->setGeometry( 70, 70, 100, 30 );
    if ( dlg->postfix.isNull() )
	postfix->setText( "########" );
    else
	postfix->setText( dlg->postfix.data() );

    tmpQLabel = new QLabel( this, "Label_2" );
    tmpQLabel->setGeometry( 10, 70, 50, 30 );
    tmpQLabel->setText( i18n("Postfix") );

    precision->raise();
    precision->setGeometry( 70, 110, 100, 30 );
    char buffer[ 100 ];
    if ( dlg->precision == -1 )
    {
      precision->setText( i18n("variable") );
    }
    else if ( dlg->precision != -2 )
    {
	sprintf( buffer, "%i", dlg->precision );
	precision->setText( buffer );
    }
    else
	precision->setText( "########" );

    tmpQLabel = new QLabel( this, "Label_3" );
    tmpQLabel->setGeometry( 10, 110, 60, 30 );
    tmpQLabel->setText( i18n("Precision") );

    prefix->raise();
    prefix->setGeometry( 70, 30, 100, 30 );
    if ( dlg->prefix.isNull() )
	prefix->setText( "########" );
    else
	prefix->setText( dlg->prefix.data() );

    format->raise();
    format->setGeometry( 190, 30, 170, 30 );

    format->insertItem( *CellLayoutDlg::formatOnlyNegSignedPixmap, 0 );
    format->insertItem( *CellLayoutDlg::formatRedOnlyNegSignedPixmap, 1 );
    format->insertItem( *CellLayoutDlg::formatRedNeverSignedPixmap, 2 );
    format->insertItem( *CellLayoutDlg::formatAlwaysSignedPixmap, 3 );
    format->insertItem( *CellLayoutDlg::formatRedAlwaysSignedPixmap, 4 );
    format->insertItem( *CellLayoutDlg::undefinedPixmap, 5 );

    tmpQLabel = new QLabel( this, "Label_4" );
    tmpQLabel->setGeometry( 190, 0, 100, 30 );
    tmpQLabel->setText( i18n("Format") );

    if ( !dlg->bFloatFormat || !dlg->bFloatColor )
	format->setCurrentItem( 5 );
    else if ( dlg->floatFormat == KSpreadCell::OnlyNegSigned && dlg->floatColor == KSpreadCell::AllBlack )
	format->setCurrentItem( 0 );
    else if ( dlg->floatFormat == KSpreadCell::OnlyNegSigned && dlg->floatColor == KSpreadCell::NegRed )
	format->setCurrentItem( 1 );
    else if ( dlg->floatFormat == KSpreadCell::AlwaysUnsigned && dlg->floatColor == KSpreadCell::NegRed )
	format->setCurrentItem( 2 );
    else if ( dlg->floatFormat == KSpreadCell::AlwaysSigned && dlg->floatColor == KSpreadCell::AllBlack )
	format->setCurrentItem( 3 );
    else if ( dlg->floatFormat == KSpreadCell::AlwaysSigned && dlg->floatColor == KSpreadCell::NegRed )
	format->setCurrentItem( 4 );

    this->resize( 400, 400 );
}

void CellLayoutPageFloat::apply( KSpreadCell *_obj )
{
    if ( strcmp( postfix->text(), dlg->postfix.data() ) != 0 )
	if ( strcmp( postfix->text(), "########" ) != 0 )
	    _obj->setPostfix( postfix->text() );
    if ( strcmp( prefix->text(), dlg->prefix.data() ) != 0 )
	if ( strcmp( prefix->text(), "########" ) != 0 )
	    _obj->setPrefix( prefix->text() );
    if ( precision->text() && precision->text()[0] != '#' )
    {
      int prec = -1;
      if ( precision->text()[0] >= '0' && precision->text()[0] <= '9' )
	prec = atoi( precision->text() );
      if ( dlg->precision != prec )
	_obj->setPrecision( prec );
    }

    switch( format->currentItem() )
    {
    case 0:
	_obj->setFloatFormat( KSpreadCell::OnlyNegSigned );
	_obj->setFloatColor( KSpreadCell::AllBlack );
	break;
    case 1:
	_obj->setFloatFormat( KSpreadCell::OnlyNegSigned );
	_obj->setFloatColor( KSpreadCell::NegRed );
	break;
    case 2:
	_obj->setFloatFormat( KSpreadCell::AlwaysUnsigned );
	_obj->setFloatColor( KSpreadCell::NegRed );
	break;
    case 3:
	_obj->setFloatFormat( KSpreadCell::AlwaysSigned );
	_obj->setFloatColor( KSpreadCell::AllBlack );
	break;
    case 4:
	_obj->setFloatFormat( KSpreadCell::AlwaysSigned );
	_obj->setFloatColor( KSpreadCell::NegRed );
	break;
    }
}


CellLayoutPageMisc::CellLayoutPageMisc( QWidget* parent, CellLayoutDlg *_dlg ) : QWidget( parent )
{
    dlg = _dlg;

    bTextColorUndefined = !dlg->bTextColor;
    bBgColorUndefined = !dlg->bBgColor;


    QLabel *tmpQLabel;

    tmpQLabel = new QLabel( this, "Label_1" );
    tmpQLabel->setGeometry( 20, 20, 100, 30 );
    tmpQLabel->setText( i18n("Text Color") );





    textColorButton = new KColorButton( this, "ComboBox_1" );
    textColorButton->setGeometry( 20, 50, 100, 30 );


    connect( textColorButton, SIGNAL( changed( const QColor & ) ),
             this, SLOT( slotSetTextColor( const QColor & ) ) );

    tmpQLabel = new QLabel( this, "Label_2" );
    tmpQLabel->setGeometry( 140, 20, 120, 30 );
    tmpQLabel->setText( i18n("Background Color") );


    bgColorButton = new KColorButton( this, "ComboBox_3" );
    bgColorButton->setGeometry( 140, 50, 100, 30 );


    connect( bgColorButton, SIGNAL( changed( const QColor & ) ),
             this, SLOT( slotSetBackgroundColor( const QColor & ) ) );


    tmpQLabel = new QLabel( this, "Label_3" );
    tmpQLabel->setGeometry( 20, 100, 120, 30 );
    tmpQLabel->setText( i18n("Functionality") );


    styleButton = new QComboBox( this, "ComboBox_2" );
    styleButton->setGeometry( 20, 130, 100, 30 );

    idStyleNormal = 0; styleButton->insertItem( i18n("Normal"), 0 );
    idStyleButton = 1; styleButton->insertItem( i18n("Button"), 1 );
    idStyleSelect = 2; styleButton->insertItem( i18n("Select"), 2 );
    if ( dlg->eStyle == KSpreadCell::ST_Undef )
    {
      idStyleUndef = 3; styleButton->insertItem( i18n("######"), 3 );
    }
    else
      idStyleUndef = -1;
    connect( styleButton, SIGNAL( activated( int ) ), this, SLOT( slotStyle( int ) ) );

    tmpQLabel = new QLabel( this, "Label_3" );
    tmpQLabel->setGeometry( 20, 180, 120, 30 );

    tmpQLabel->setText( i18n("Action") );

    actionText = new QLineEdit( this );
    actionText->setGeometry( 20, 210, 200, 30 );

    if ( dlg->isSingleCell() )
    {
      if ( !dlg->actionText.isEmpty() )
	actionText->setText( dlg->actionText );
      if ( dlg->eStyle == KSpreadCell::ST_Normal || dlg->eStyle == KSpreadCell::ST_Undef )
	actionText->setEnabled( false );
    }
    else
      actionText->setEnabled( false );

    if ( dlg->eStyle == KSpreadCell::ST_Normal )
      styleButton->setCurrentItem( idStyleNormal );
    else if ( dlg->eStyle == KSpreadCell::ST_Button )
      styleButton->setCurrentItem( idStyleButton );
    else if ( dlg->eStyle == KSpreadCell::ST_Select )
      styleButton->setCurrentItem( idStyleSelect );
    else if ( dlg->eStyle == KSpreadCell::ST_Undef )
      styleButton->setCurrentItem( idStyleUndef );

    if ( dlg->bTextColor )
    {
	textColor = dlg->textColor;
        textColorButton->setColor( dlg->textColor);
    }
    else
    {
	textColor = black;
        textColorButton->setColor( black );
    }

    if ( dlg->bBgColor )
    {
	bgColor = dlg->bgColor;
        bgColorButton->setColor(bgColor);
    }
    else
    {
	bgColor = white;
        bgColorButton->setColor(white);
    }

    this->resize( 400, 400 );
}

void CellLayoutPageMisc::apply( KSpreadCell *_obj )
{
    if ( !bTextColorUndefined )
	_obj->setTextColor( textColor );
    if ( !bBgColorUndefined )
	_obj->setBgColor( bgColor );
    if ( styleButton->currentItem() == idStyleNormal )
      _obj->setStyle( KSpreadCell::ST_Normal );
    else if ( styleButton->currentItem() == idStyleButton )
      _obj->setStyle( KSpreadCell::ST_Button );
    else if ( styleButton->currentItem() == idStyleSelect )
      _obj->setStyle( KSpreadCell::ST_Select );
    if ( actionText->isEnabled() )
      _obj->setAction( actionText->text() );
}

void CellLayoutPageMisc::slotStyle( int _i )
{
  if ( dlg->isSingleCell() && _i != idStyleNormal && _i != idStyleUndef )
    actionText->setEnabled( true );
  else
    actionText->setEnabled( false );
}

void CellLayoutPageMisc::slotSetTextColor( const QColor &_color )
{
textColor=_color;
}

void CellLayoutPageMisc::slotSetBackgroundColor( const QColor &_color )
{
bgColor =_color;
}

/*void CellLayoutPageMisc::setColor( QPushButton *_button, const QColor &_color )
{
    QColorGroup normal( ( QColor( QRgb(0) ) ), _color, QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
    QColorGroup disabled( ( QColor( QRgb(8421504) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(8421504) ), QColor( QRgb(12632256) ) );
    QColorGroup active( ( QColor( QRgb(0) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
    QPalette palette( normal, disabled, active );
    _button->setPalette( palette );
}*/

#define YOFFSET  5
#define XOFFSET  5
#define LABLE_LENGTH  40
#define LABLE_HEIGHT 20
#define SIZE_X 400
#define SIZE_Y 280
#define FONTLABLE_LENGTH 60
#define COMBO_BOX_HEIGHT 28
#define COMBO_ADJUST 3
#define OKBUTTONY 260
#define BUTTONHEIGHT 25

CellLayoutPageFont::CellLayoutPageFont( QWidget* parent, CellLayoutDlg *_dlg ) : QWidget ( parent )
{
  dlg = _dlg;

  box1 = new QGroupBox(this, "Box1");
  box1->setGeometry(XOFFSET,YOFFSET,SIZE_X -  XOFFSET
		   ,160);
  box1->setTitle(i18n("Requested Font"));

  box1 = new QGroupBox(this, "Box2");
  box1->setGeometry(XOFFSET,170,SIZE_X -  XOFFSET
		   ,110);
  box1->setTitle(i18n("Actual Font"));


  family_label = new QLabel(this,"family");
  family_label->setText(i18n("Family:"));
  family_label->setGeometry(3*XOFFSET,8*YOFFSET,LABLE_LENGTH,LABLE_HEIGHT);

  actual_family_label = new QLabel(this,"afamily");
  actual_family_label->setText(i18n("Family:"));
  actual_family_label->setGeometry(3*XOFFSET,190,40,LABLE_HEIGHT);

  actual_family_label_data = new QLabel(this,"afamilyd");
  actual_family_label_data->setGeometry(3*XOFFSET +50 ,190,110,LABLE_HEIGHT);

  size_label = new QLabel(this,"size");
  size_label->setText(i18n("Size:"));
  size_label->setGeometry(6*XOFFSET + LABLE_LENGTH + 12*XOFFSET +2* FONTLABLE_LENGTH,
			  8*YOFFSET,LABLE_LENGTH,LABLE_HEIGHT);

  actual_size_label = new QLabel(this,"asize");
  actual_size_label->setText(i18n("Size:"));
  actual_size_label->setGeometry(3*XOFFSET,190 +LABLE_HEIGHT ,
				 LABLE_LENGTH,LABLE_HEIGHT);

  actual_size_label_data = new QLabel(this,"asized");
  actual_size_label_data->setGeometry(3*XOFFSET +50 ,190 + LABLE_HEIGHT
				      ,110,LABLE_HEIGHT);

  weight_label = new QLabel(this,"weight");
  weight_label->setText(i18n("Weight:"));
  weight_label->setGeometry(3*XOFFSET,15*YOFFSET + LABLE_HEIGHT
			  ,LABLE_LENGTH,LABLE_HEIGHT);

  actual_weight_label = new QLabel(this,"aweight");
  actual_weight_label->setText(i18n("Weight:"));
  actual_weight_label->setGeometry(3*XOFFSET,190 + 2*LABLE_HEIGHT ,
				 LABLE_LENGTH,LABLE_HEIGHT);

  actual_weight_label_data = new QLabel(this,"aweightd");
  actual_weight_label_data->setGeometry(3*XOFFSET +50 ,190 + 2*LABLE_HEIGHT
				      ,110,LABLE_HEIGHT);

  style_label = new QLabel(this,"style");
  style_label->setText(i18n("Style:"));
  style_label->setGeometry(6*XOFFSET + LABLE_LENGTH + 12*XOFFSET +
			   2*FONTLABLE_LENGTH,
			   15*YOFFSET + LABLE_HEIGHT
			 ,LABLE_LENGTH,
			   LABLE_HEIGHT);

  actual_style_label = new QLabel(this,"astyle");
  actual_style_label->setText(i18n("Style:"));
  actual_style_label->setGeometry(3*XOFFSET,190 + 3*LABLE_HEIGHT ,
				 LABLE_LENGTH,LABLE_HEIGHT);

  actual_style_label_data = new QLabel(this,"astyled");
  actual_style_label_data->setGeometry(3*XOFFSET +50 ,190 + 3*LABLE_HEIGHT
				      ,110,LABLE_HEIGHT);

  family_combo = new QComboBox( this, "Family" );
  family_combo->insertItem( "", 0 );
  family_combo->insertItem( "Times" );
  family_combo->insertItem( "Helvetica" );
  family_combo->insertItem( "Courier" );
  family_combo->insertItem( "Symbol" );

  family_combo->setInsertionPolicy(QComboBox::NoInsertion);

  family_combo->setGeometry(6*XOFFSET + LABLE_LENGTH
			    ,8*YOFFSET - COMBO_ADJUST ,4* LABLE_LENGTH,COMBO_BOX_HEIGHT);
  connect( family_combo, SIGNAL(activated(const QString &)),
	   SLOT(family_chosen_slot(const QString &)) );
  //  QToolTip::add( family_combo, "Select Font Family" );


  size_combo = new QComboBox( true, this, "Size" );
  QStringList lst;
  lst.append("");
  for ( unsigned int i = 1; i < 100; ++i )
	lst.append( QString( "%1" ).arg( i ) );

  size_combo->insertStringList( lst );

  size_combo->setInsertionPolicy(QComboBox::NoInsertion);
  size_combo->setGeometry(10*XOFFSET + 6*LABLE_LENGTH
			    ,8*YOFFSET - COMBO_ADJUST
			  ,2*LABLE_LENGTH + 20,COMBO_BOX_HEIGHT);
  connect( size_combo, SIGNAL(activated(const QString &)),
	   SLOT(size_chosen_slot(const QString &)) );
  //  QToolTip::add( size_combo, "Select Font Size in Points" );
  strike = new QCheckBox(i18n("Strike out"),this);
  strike->setGeometry(6*XOFFSET + LABLE_LENGTH
			    ,26*YOFFSET - COMBO_ADJUST
			    ,2*LABLE_LENGTH+20,COMBO_BOX_HEIGHT);
  strike->setChecked(dlg->strike);
  connect( strike, SIGNAL( clicked()),
	   SLOT(strike_chosen_slot()) );
  underline = new QCheckBox(i18n("Underline"),this);
  underline->setGeometry(10*XOFFSET + 6*LABLE_LENGTH
			    ,26*YOFFSET - COMBO_ADJUST
			    ,2*LABLE_LENGTH+20,COMBO_BOX_HEIGHT);
  underline->setChecked(dlg->underline);
  connect( underline, SIGNAL( clicked()),
	   SLOT(underline_chosen_slot()) );

  weight_combo = new QComboBox( this, "Weight" );
  weight_combo->insertItem( "", 0 );
  weight_combo->insertItem( i18n("normal") );
  weight_combo->insertItem( i18n("bold") );
  weight_combo->setGeometry(6*XOFFSET + LABLE_LENGTH
			    ,19*YOFFSET - COMBO_ADJUST
			    ,4*LABLE_LENGTH,COMBO_BOX_HEIGHT);
  weight_combo->setInsertionPolicy(QComboBox::NoInsertion);
  connect( weight_combo, SIGNAL(activated(const QString &)),
	   SLOT(weight_chosen_slot(const QString &)) );
  //  QToolTip::add( weight_combo, "Select Font Weight" );


  style_combo = new QComboBox( this, "Style" );
  style_combo->insertItem( "", 0 );
  style_combo->insertItem( i18n("roman") );
  style_combo->insertItem( i18n("italic"), 2 );
  style_combo->setGeometry(10*XOFFSET + 6*LABLE_LENGTH
			    ,19*YOFFSET- COMBO_ADJUST
			   ,2*LABLE_LENGTH + 20,COMBO_BOX_HEIGHT);
  style_combo->setInsertionPolicy(QComboBox::NoInsertion);
  connect( style_combo, SIGNAL(activated(const QString &)),
	   SLOT(style_chosen_slot(const QString &)) );
  // QToolTip::add( style_combo, "Select Font Style" );

  example_label = new QLabel(this,"examples");
  example_label->setFont(selFont);
  example_label->setGeometry(200,190,190, 80);
  example_label->setAlignment(AlignCenter);
  example_label->setBackgroundColor(white);
  example_label->setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
  example_label->setLineWidth( 1 );
  example_label->setText(i18n("Dolor Ipse"));
  //  example_label->setAutoResize(true);

  connect(this,SIGNAL(fontSelected( const QFont&  )),
	  this,SLOT(display_example( const QFont&)));

  setCombos();
  display_example( selFont );

  this->resize( 400, 400 );
}

void CellLayoutPageFont::apply( KSpreadCell *_obj )
{
    if ( size_combo->currentItem() != 0 )
	_obj->setTextFontSize( selFont.pointSize() );
    if ( family_combo->currentItem() != 0 )
	_obj->setTextFontFamily( selFont.family() );
    if ( weight_combo->currentItem() != 0 )
	_obj->setTextFontBold( selFont.bold() );
    if ( style_combo->currentItem() != 0 )
	_obj->setTextFontItalic( selFont.italic() );
    _obj->setTextFontStrike( strike->isChecked() );
    _obj->setTextFontUnderline(underline->isChecked() );
}

void CellLayoutPageFont::underline_chosen_slot()
{
   selFont.setUnderline( underline->isChecked() );
   emit fontSelected(selFont);
}

void CellLayoutPageFont::strike_chosen_slot()
{
   selFont.setStrikeOut( strike->isChecked() );
   emit fontSelected(selFont);
}

void CellLayoutPageFont::family_chosen_slot(const QString & family)
{
  selFont.setFamily(family);
  //display_example();
  emit fontSelected(selFont);
}

void CellLayoutPageFont::size_chosen_slot(const QString & size)
{
  QString size_string = size;

  selFont.setPointSize(size_string.toInt());
  //display_example();
  emit fontSelected(selFont);
}

void CellLayoutPageFont::weight_chosen_slot(const QString & weight)
{
  QString weight_string = weight;

  if ( weight_string == QString(i18n("normal")))
    selFont.setBold(false);
  if ( weight_string == QString(i18n("bold")))
       selFont.setBold(true);
  // display_example();
  emit fontSelected(selFont);
}

void CellLayoutPageFont::style_chosen_slot(const QString & style)
{
  QString style_string = style;

  if ( style_string == QString(i18n("roman")))
    selFont.setItalic(false);
  if ( style_string == QString(i18n("italic")))
    selFont.setItalic(true);
  //  display_example();
  emit fontSelected(selFont);
}


void CellLayoutPageFont::display_example(const QFont& font)
{
  QString string;

  example_label->setFont(font);
  example_label->repaint();

  kdDebug(36001) << "FAMILY 2 '" << font.family().ascii() << "' " << font.pointSize() << endl;

  QFontInfo info = example_label->fontInfo();
  actual_family_label_data->setText(info.family());

  kdDebug(36001) << "FAMILY 3 '" << info.family().latin1() << "' " << info.pointSize() << endl;

  string.setNum(info.pointSize());
  actual_size_label_data->setText(string);

  if (info.bold())
    actual_weight_label_data->setText(i18n("Bold"));
  else
    actual_weight_label_data->setText(i18n("Normal"));

  if (info.italic())
    actual_style_label_data->setText(i18n("italic"));
  else
    actual_style_label_data->setText(i18n("roman"));
}

void CellLayoutPageFont::setCombos()
{
 QString string;
 QComboBox* combo;
 int number_of_entries;
 bool found;

 // Needed to initialize this font
 selFont = dlg->textFont;

 combo = family_combo;
 if ( dlg->bTextFontFamily )
 {
     selFont.setFamily( dlg->textFontFamily );
     kdDebug(36001) << "Family = " << dlg->textFontFamily.data() << endl;
     number_of_entries =  family_combo->count();
     string = dlg->textFontFamily;
     found = false;

     for (int i = 1; i < number_of_entries - 1; i++)
     {
	 if ( string == (QString) combo->text(i))
	 {
	     combo->setCurrentItem(i);
	     //     kdDebug(36001) << "Found Font " << string.data() << endl;
	     found = true;
	     break;
	 }
     }
 }
 else
     combo->setCurrentItem( 0 );

 combo = size_combo;
 if ( dlg->bTextFontSize )
 {
     kdDebug(36001) << "SIZE=" << dlg->textFontSize << endl;
     selFont.setPointSize( dlg->textFontSize );
     number_of_entries = size_combo->count();
     string.setNum( dlg->textFontSize );
     found = false;

     for (int i = 0; i < number_of_entries ; i++){
	 if ( string == (QString) combo->text(i)){
	     combo->setCurrentItem(i);
	     found = true;
	     // kdDebug(36001) << "Found Size " << string.data() << " setting to item " i << endl;
	     break;
	 }
     }
 }
 else
     combo->setCurrentItem( 0 );

 if ( !dlg->bTextFontBold )
     weight_combo->setCurrentItem(0);
 else if ( dlg->textFontBold )
 {
     selFont.setBold( dlg->textFontBold );
     weight_combo->setCurrentItem(2);
 }
 else
 {
     selFont.setBold( dlg->textFontBold );
     weight_combo->setCurrentItem(1);
 }

 if ( !dlg->bTextFontItalic )
     weight_combo->setCurrentItem(0);
 else if ( dlg->textFontItalic )
 {
     selFont.setItalic( dlg->textFontItalic );
     style_combo->setCurrentItem(2);
 }
 else
 {
     selFont.setItalic( dlg->textFontItalic );
     style_combo->setCurrentItem(1);
 }
}

CellLayoutPagePosition::CellLayoutPagePosition( QWidget* parent, CellLayoutDlg *_dlg ) : QWidget( parent )
{
    dlg = _dlg;

    QGridLayout *grid3 = new QGridLayout(this,3,2,15,7);
    QButtonGroup *grp = new QButtonGroup( i18n("Horizontal"),this);
    grp->setRadioButtonExclusive( TRUE );
    
    QGridLayout *grid2 = new QGridLayout(grp,3,1,15,7);
    left = new QRadioButton( i18n("Left"), grp );
    grid2->addWidget(left,0,0);
    center = new QRadioButton( i18n("Center"), grp );
    grid2->addWidget(center,1,0);
    right = new QRadioButton( i18n("Right"), grp );
    grid2->addWidget(right,2,0);
    grid3->addWidget(grp,0,0);

    if(dlg->alignX==KSpreadCell::Left)
        left->setChecked(true);
    else if(dlg->alignX==KSpreadCell::Center)
        center->setChecked(true);
    else if(dlg->alignX==KSpreadCell::Right)
        right->setChecked(true);


    grp = new QButtonGroup( i18n("Vertical"),this);
    grp->setRadioButtonExclusive( TRUE );

    grid2 = new QGridLayout(grp,3,1,15,7);
    top = new QRadioButton( i18n("Top"), grp );
    grid2->addWidget(top,0,0);
    middle = new QRadioButton( i18n("Middle"), grp );
    grid2->addWidget(middle,1,0);
    bottom = new QRadioButton( i18n("Bottom"), grp );
    grid2->addWidget(bottom,2,0);
    grid3->addWidget(grp,0,1);

    if(dlg->alignY==KSpreadCell::Top)
        top->setChecked(true);
    else if(dlg->alignY==KSpreadCell::Middle)
        middle->setChecked(true);
    else if(dlg->alignY==KSpreadCell::Bottom)
        bottom->setChecked(true);

    grp = new QButtonGroup( i18n("Multi Row"),this);
    
    grid2 = new QGridLayout(grp,1,1,15,7);
    multi = new QCheckBox( i18n("Goto line automatically"), grp );

    grid2->addWidget(multi,0,0);
    multi->setChecked(dlg->bMultiRow);
    grid3->addMultiCellWidget(grp,1,1,0,1);

    grp = new QButtonGroup( i18n("Size of cell"),this);
    grid2 = new QGridLayout(grp,2,2,15,7);
    width=new KIntNumInput(dlg->widthSize, grp, 10);
    width->setLabel(i18n("Width :"));
    width->setRange(20, 400, 1);
    grid2->addWidget(width,0,0);
    defaultWidth=new QCheckBox(i18n("Default width (60)"),grp);
    grid2->addWidget(defaultWidth,1,0);

    height=new KIntNumInput(dlg->heigthSize, grp, 10);
    height->setLabel(i18n("Height :"));
    height->setRange(20, 400, 1);
    grid2->addWidget(height,0,1);
    defaultHeight=new QCheckBox(i18n("Default height (20)"),grp);
    grid2->addWidget(defaultHeight,1,1);

    grid3->addMultiCellWidget(grp,2,2,0,1);

    connect(defaultWidth , SIGNAL(clicked() ),this, SLOT(slotChangeWidthState()));
    connect(defaultHeight , SIGNAL(clicked() ),this, SLOT(slotChangeHeightState()));
    this->resize( 400, 400 );

}
void CellLayoutPagePosition::slotChangeWidthState()
{
    if( defaultWidth->isChecked())
        width->setEnabled(false);
    else
        width->setEnabled(true);
}

void CellLayoutPagePosition::slotChangeHeightState()
{
    if( defaultHeight->isChecked())
        height->setEnabled(false);
    else
        height->setEnabled(true);
}

void CellLayoutPagePosition::apply( KSpreadCell *_obj )
{
if(top->isChecked())
        _obj->setAlignY(KSpreadCell::Top);
else if(bottom->isChecked())
        _obj->setAlignY(KSpreadCell::Bottom);
else if(middle->isChecked())
        _obj->setAlignY(KSpreadCell::Middle);

if(left->isChecked())
        _obj->setAlign(KSpreadCell::Left);
else if(right->isChecked())
        _obj->setAlign(KSpreadCell::Right);
else if(center->isChecked())
        _obj->setAlign(KSpreadCell::Center);
_obj->setMultiRow(multi->isChecked());
}

int CellLayoutPagePosition::getSizeHeight()
{
  if(defaultHeight->isChecked())
        return 20;
  else
        return height->value();
}

int CellLayoutPagePosition::getSizeWidth()
{
  if(defaultWidth->isChecked())
        return 60;
  else
        return width->value();
}

KSpreadBorderButton::KSpreadBorderButton( QWidget *parent, const char *_name ) : QPushButton(parent,_name)
{
  penStyle = Qt::NoPen;
  penWidth = 1;
  penColor = Qt::black;
  setToggleButton( TRUE );
  setOn( false);
  setChanged(false);
}
void KSpreadBorderButton::mousePressEvent( QMouseEvent * )
{

  this->setOn(!isOn());
  emit clicked( this );
}

void KSpreadBorderButton::setUndefined()
{
 setPenStyle(SolidLine );
 setPenWidth(1);
 setColor(Qt::gray);
}


void KSpreadBorderButton::unselect()
{
setOn(false);
setPenWidth(1);
setPenStyle(Qt::NoPen);
setColor( Qt::black );
setChanged(true);
}

KSpreadBord::KSpreadBord( QWidget *parent, const char *_name )
    : QFrame( parent, _name )
{
}

#define XLEN 100
#define YHEI 60
#define OFFSETX 5
#define OFFSETY 5
void KSpreadBord::paintEvent( QPaintEvent *_ev )
{
  QFrame::paintEvent( _ev );
  QPen pen;
  QPainter painter;
  painter.begin( this );

  pen.setColor( Qt::gray );
  pen.setStyle( SolidLine );
  pen.setWidth( 2 );
  painter.setPen( pen );

  painter.drawLine( OFFSETX-5, OFFSETY, OFFSETX , OFFSETY );
  painter.drawLine( OFFSETX, OFFSETY-5, OFFSETX , OFFSETY );
  painter.drawLine( XLEN-OFFSETX, OFFSETY, XLEN , OFFSETY );
  painter.drawLine( XLEN-OFFSETX, OFFSETY-5, XLEN-OFFSETX , OFFSETY );

  painter.drawLine( OFFSETX, YHEI-OFFSETY, OFFSETX , YHEI );
  painter.drawLine( OFFSETX-5, YHEI-OFFSETY, OFFSETX , YHEI-OFFSETY );

  painter.drawLine( XLEN-OFFSETX, YHEI-OFFSETY, XLEN , YHEI-OFFSETY );
  painter.drawLine( XLEN-OFFSETX, YHEI-OFFSETY, XLEN-OFFSETX , YHEI );

  painter.end();
  emit redraw();
}

void KSpreadBord::mousePressEvent( QMouseEvent* _ev )
{
//todo
emit choosearea(_ev);
}

CellLayoutPageBorder::CellLayoutPageBorder( QWidget* parent, CellLayoutDlg *_dlg ) : QWidget( parent )
{
  dlg = _dlg;

  QGroupBox* tmpQGroupBox;
  tmpQGroupBox = new QGroupBox( this, "GroupBox_1" );
  tmpQGroupBox->setGeometry( 15, 50, 190, 150 );
  tmpQGroupBox->setFrameStyle( 49 );
  tmpQGroupBox->setTitle( i18n("Border") );
  tmpQGroupBox->setAlignment( 1 );

  area=new KSpreadBord(this,"area");
  area->setGeometry( 60,100,XLEN,YHEI);
  area->setBackgroundColor( white );

  top=new KSpreadBorderButton(this,"top");
  top->setGeometry(98,70,25,25);
  loadIcon("bordertop",top);
  bottom=new KSpreadBorderButton(this,"bottom");
  bottom->setGeometry(98,165,25,25);
  loadIcon("borderbottom",bottom);
  left=new KSpreadBorderButton(this,"left");
  left->setGeometry(25,115,25,25);
  loadIcon("borderleft",left);
  right=new KSpreadBorderButton(this,"right");
  right->setGeometry(165,115,25,25);
  loadIcon("borderright",right);

  fallDiagonal=new KSpreadBorderButton(this,"fall");
  fallDiagonal->setGeometry(25,70,25,25);
  loadIcon("borderfall",fallDiagonal);
  goUpDiagonal=new KSpreadBorderButton(this,"go");
  goUpDiagonal->setGeometry(165,70,25,25);
  loadIcon("borderup",goUpDiagonal);

  vertical=new KSpreadBorderButton(this,"vertical");
  vertical->setGeometry(165,165,25,25);

  loadIcon("bordervertical",vertical);


  horizontal=new KSpreadBorderButton(this,"horizontal");
  horizontal->setGeometry(25,165,25,25);
  loadIcon("borderhorizontal",horizontal);

  tmpQGroupBox = new QGroupBox( this, "GroupBox_3" );
  tmpQGroupBox->setGeometry( 15 , 220, 115, 50 );
  tmpQGroupBox->setFrameStyle( 49 );
  tmpQGroupBox->setTitle( i18n("Preselect") );
  tmpQGroupBox->setAlignment( 1 );

  outline=new KSpreadBorderButton(this,"outline");
  outline->setGeometry(95,240,25,25);
  loadIcon("borderoutline",outline);

  remove=new KSpreadBorderButton(this,"remove");
  remove->setGeometry(25,240,25,25);
  loadIcon("borderremove",remove);

  all=new KSpreadBorderButton(this,"all");
  all->setGeometry(60,240,25,25);

  if((dlg->oneRow==true)&&(dlg->oneCol==false))
        {
        loadIcon("bordervertical",all);
        }
  else if((dlg->oneRow==false)&&(dlg->oneCol==true))
        {
        loadIcon("borderhorizontal",all);
        }
  else if((dlg->oneRow==false)&&(dlg->oneCol==false))
        {
         loadIcon("borderinside",all);
        }
  else
        {
        loadIcon("borderinside",all);
        all->setEnabled(false);
        }

  tmpQGroupBox = new QGroupBox( this, "GroupBox_1" );
  tmpQGroupBox->setGeometry( 215, 10, 140, 230 );
  tmpQGroupBox->setFrameStyle( 49 );
  tmpQGroupBox->setTitle( i18n("Pattern") );
  tmpQGroupBox->setAlignment( 1 );


  pattern1 = new KSpreadPatternSelect( this, "Frame_8" );
  pattern1->setGeometry( 225, 30, 50, 20 );
    {
      QColorGroup normal( ( QColor( QRgb(0) ) ), QColor( QRgb(16777215) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QColorGroup disabled( ( QColor( QRgb(8421504) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(8421504) ), QColor( QRgb(12632256) ) );
	QColorGroup active( ( QColor( QRgb(0) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QPalette palette( normal, disabled, active );
	pattern1->setPalette( palette );
    }
    pattern1->setFrameStyle( 50 );

    pattern2 = new KSpreadPatternSelect( this, "Frame_9" );
    pattern2->setGeometry( 225, 60, 50, 20 );
    {
	QColorGroup normal( ( QColor( QRgb(0) ) ), QColor( QRgb(16777215) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QColorGroup disabled( ( QColor( QRgb(8421504) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(8421504) ), QColor( QRgb(12632256) ) );
	QColorGroup active( ( QColor( QRgb(0) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QPalette palette( normal, disabled, active );
	pattern2->setPalette( palette );
    }
    pattern2->setFrameStyle( 50 );

    pattern3 = new KSpreadPatternSelect( this, "Frame_10" );
    pattern3->setGeometry( 225, 150, 50, 20 );
    {
	QColorGroup normal( ( QColor( QRgb(0) ) ), QColor( QRgb(16777215) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QColorGroup disabled( ( QColor( QRgb(8421504) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(8421504) ), QColor( QRgb(12632256) ) );
	QColorGroup active( ( QColor( QRgb(0) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QPalette palette( normal, disabled, active );
	pattern3->setPalette( palette );
    }
    pattern3->setFrameStyle( 50 );

    pattern4 = new KSpreadPatternSelect( this, "Frame_11" );
    pattern4->setGeometry( 290, 30, 50, 20 );
    {
	QColorGroup normal( ( QColor( QRgb(0) ) ), QColor( QRgb(16777215) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QColorGroup disabled( ( QColor( QRgb(8421504) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(8421504) ), QColor( QRgb(12632256) ) );
	QColorGroup active( ( QColor( QRgb(0) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QPalette palette( normal, disabled, active );
	pattern4->setPalette( palette );
    }
    pattern4->setFrameStyle( 50 );

    pattern5 = new KSpreadPatternSelect( this, "Frame_12" );
    pattern5->setGeometry( 290, 60, 50, 20 );
    {
	QColorGroup normal( ( QColor( QRgb(0) ) ), QColor( QRgb(16777215) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QColorGroup disabled( ( QColor( QRgb(8421504) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(8421504) ), QColor( QRgb(12632256) ) );
	QColorGroup active( ( QColor( QRgb(0) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QPalette palette( normal, disabled, active );
	pattern5->setPalette( palette );
    }
    pattern5->setFrameStyle( 50 );

    pattern6 = new KSpreadPatternSelect( this, "Frame_13" );
    pattern6->setGeometry( 290, 90, 50, 20 );
    {
	QColorGroup normal( ( QColor( QRgb(0) ) ), QColor( QRgb(16777215) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QColorGroup disabled( ( QColor( QRgb(8421504) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(8421504) ), QColor( QRgb(12632256) ) );
	QColorGroup active( ( QColor( QRgb(0) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QPalette palette( normal, disabled, active );
	pattern6->setPalette( palette );
    }
    pattern6->setFrameStyle( 50 );

    pattern7 = new KSpreadPatternSelect( this, "Frame_14" );
    pattern7->setGeometry( 290, 120, 50, 20 );
    {
	QColorGroup normal( ( QColor( QRgb(0) ) ), QColor( QRgb(16777215) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QColorGroup disabled( ( QColor( QRgb(8421504) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(8421504) ), QColor( QRgb(12632256) ) );
	QColorGroup active( ( QColor( QRgb(0) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QPalette palette( normal, disabled, active );
	pattern7->setPalette( palette );
    }
    pattern7->setFrameStyle( 50 );

    pattern8 = new KSpreadPatternSelect( this, "Frame_15" );
    pattern8->setGeometry( 290, 150, 50, 20 );
    {
	QColorGroup normal( ( QColor( QRgb(0) ) ), QColor( QRgb(16777215) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QColorGroup disabled( ( QColor( QRgb(8421504) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(8421504) ), QColor( QRgb(12632256) ) );
	QColorGroup active( ( QColor( QRgb(0) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QPalette palette( normal, disabled, active );
	pattern8->setPalette( palette );
    }
    pattern8->setFrameStyle( 50 );

    pattern9 = new KSpreadPatternSelect( this, "Frame_16" );
    pattern9->setGeometry( 225, 90, 50, 20 );
    {
	QColorGroup normal( ( QColor( QRgb(0) ) ), QColor( QRgb(16777215) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QColorGroup disabled( ( QColor( QRgb(8421504) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(8421504) ), QColor( QRgb(12632256) ) );
	QColorGroup active( ( QColor( QRgb(0) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QPalette palette( normal, disabled, active );
	pattern9->setPalette( palette );
    }
    pattern9->setFrameStyle( 50 );

    pattern10 = new KSpreadPatternSelect( this, "Frame_17" );
    pattern10->setGeometry( 225, 120, 50, 20 );
    {
	QColorGroup normal( ( QColor( QRgb(0) ) ), QColor( QRgb(16777215) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QColorGroup disabled( ( QColor( QRgb(8421504) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(8421504) ), QColor( QRgb(12632256) ) );
	QColorGroup active( ( QColor( QRgb(0) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QPalette palette( normal, disabled, active );
	pattern10->setPalette( palette );
    }
    pattern10->setFrameStyle( 50 );



    color = new KColorButton (this, "PushButton_1" );
    color->setGeometry( 270, 205, 80, 25 );

    QLabel *tmpQLabel = new QLabel( this, "Label_6" );
    tmpQLabel->setGeometry( 225, 205, 35, 30 );
    tmpQLabel->setText( i18n("Color") );

 if(dlg->leftBorderStyle != Qt::NoPen || !dlg->bLeftBorderStyle )
    {
    if ( dlg->bLeftBorderColor && dlg->bLeftBorderStyle  )
      {
	left->setPenStyle(dlg->leftBorderStyle );
	left->setPenWidth(dlg->leftBorderWidth);
	left->setColor(dlg->leftBorderColor);
	left->setOn(true);
      }
    else
      {

	left->setUndefined();
      }
 }
 if(dlg->rightBorderStyle!=Qt::NoPen|| !dlg->bRightBorderStyle)
    {
    if ( dlg->bRightBorderColor && dlg->bRightBorderStyle && dlg->rightBorderStyle!=NoPen)
      {
	right->setPenStyle(dlg->rightBorderStyle );
	right->setPenWidth(dlg->rightBorderWidth);
	right->setColor(dlg->rightBorderColor);
	right->setOn(true);
      }
    else
      {

	right->setUndefined();
      }
     }

   if(  dlg->topBorderStyle!=Qt::NoPen || !dlg->bTopBorderStyle)
    {
    if ( dlg->bTopBorderColor && dlg->bTopBorderStyle)
      {
	top->setPenStyle(dlg->topBorderStyle );
	top->setPenWidth(dlg->topBorderWidth);
	top->setColor(dlg->topBorderColor);
      	top->setOn(true);
      }
    else
      {

	top->setUndefined();
      }
     }

 if(dlg->bottomBorderStyle != Qt::NoPen || !dlg->bBottomBorderStyle)
 {
    if ( dlg->bBottomBorderColor && dlg->bBottomBorderStyle )

      {
	bottom->setPenStyle(dlg->bottomBorderStyle );
	bottom->setPenWidth(dlg->bottomBorderWidth);
	bottom->setColor(dlg->bottomBorderColor);
	bottom->setOn(true);
      }
    else
      {

	bottom->setUndefined();
      }
 }

  if(dlg->oneRow==FALSE)
  {
   if(dlg->horizontalBorderStyle!=Qt::NoPen ||!dlg->bHorizontalBorderStyle)
   {
    if ( dlg->bHorizontalBorderColor && dlg->bHorizontalBorderStyle )
      {
        horizontal->setPenStyle(dlg->horizontalBorderStyle );
	horizontal->setPenWidth(dlg->horizontalBorderWidth);
        horizontal->setColor(dlg->horizontalBorderColor);
	horizontal->setOn(true);
       }
    else
       {
	 horizontal->setUndefined();
       }
    }
  }
  else
        horizontal->setEnabled(false);

  if(dlg->oneCol==FALSE)
  {
        if(dlg->verticalBorderStyle!=Qt::NoPen || !dlg->bVerticalBorderStyle)
        {
                if ( dlg->bVerticalBorderColor && dlg->bVerticalBorderStyle )
                {
	        vertical->setPenStyle(dlg->verticalBorderStyle );
	        vertical->setPenWidth(dlg->verticalBorderWidth);
	        vertical->setColor(dlg->verticalBorderColor);
	        vertical->setOn(true);
	        }
        else
                {
	        vertical->setUndefined();
                }
         }
   }
   else
        {
        vertical->setEnabled(false);
        }

  if(dlg->fallDiagonalStyle!=Qt::NoPen || !dlg->bFallDiagonalStyle)
  {
    if ( dlg->bfallDiagonalColor && dlg->bFallDiagonalStyle  )
      {
	fallDiagonal->setPenStyle(dlg->fallDiagonalStyle );
	fallDiagonal->setPenWidth(dlg->fallDiagonalWidth);
	fallDiagonal->setColor(dlg->fallDiagonalColor);
	fallDiagonal->setOn(true);
      }
    else
      {

	fallDiagonal->setUndefined();
      }
   }

 if(dlg->goUpDiagonalStyle!=Qt::NoPen || !dlg->bGoUpDiagonalStyle)
    {
    if ( dlg->bGoUpDiagonalColor && dlg->bGoUpDiagonalStyle )
      {
	goUpDiagonal->setPenStyle(dlg->goUpDiagonalStyle );
	goUpDiagonal->setPenWidth(dlg->goUpDiagonalWidth);
	goUpDiagonal->setColor(dlg->goUpDiagonalColor);
	goUpDiagonal->setOn(true);
      }
    else
      {
	goUpDiagonal->setUndefined();
      }
    }


    pattern1->setPattern( black, 1, DotLine );
    pattern2->setPattern( black, 1, DashLine );
    pattern3->setPattern( black, 1, SolidLine );
    pattern4->setPattern( black, 2, SolidLine );
    pattern5->setPattern( black, 3, SolidLine );
    pattern6->setPattern( black, 4, SolidLine );
    pattern7->setPattern( black, 5, SolidLine );
    pattern8->setPattern( black, 1, NoPen );
    pattern9->setPattern( black, 1, DashDotLine );
    pattern10->setPattern( black, 1, DashDotDotLine );

    slotSetColorButton( black );

    connect( color, SIGNAL( changed( const QColor & ) ),
	     this, SLOT( slotSetColorButton( const QColor & ) ) );


    connect( pattern1, SIGNAL( clicked( KSpreadPatternSelect* ) ),
	     this, SLOT( slotUnselect2( KSpreadPatternSelect* ) ) );
    connect( pattern2, SIGNAL( clicked( KSpreadPatternSelect* ) ),
	     this, SLOT( slotUnselect2( KSpreadPatternSelect* ) ) );
    connect( pattern3, SIGNAL( clicked( KSpreadPatternSelect* ) ),
	     this, SLOT( slotUnselect2( KSpreadPatternSelect* ) ) );
    connect( pattern4, SIGNAL( clicked( KSpreadPatternSelect* ) ),
	     this, SLOT( slotUnselect2( KSpreadPatternSelect* ) ) );
    connect( pattern5, SIGNAL( clicked( KSpreadPatternSelect* ) ),
	     this, SLOT( slotUnselect2( KSpreadPatternSelect* ) ) );
    connect( pattern6, SIGNAL( clicked( KSpreadPatternSelect* ) ),
	     this, SLOT( slotUnselect2( KSpreadPatternSelect* ) ) );
    connect( pattern7, SIGNAL( clicked( KSpreadPatternSelect* ) ),
	     this, SLOT( slotUnselect2( KSpreadPatternSelect* ) ) );
    connect( pattern8, SIGNAL( clicked( KSpreadPatternSelect* ) ),
	     this, SLOT( slotUnselect2( KSpreadPatternSelect* ) ) );
    connect( pattern9, SIGNAL( clicked( KSpreadPatternSelect* ) ),
	     this, SLOT( slotUnselect2( KSpreadPatternSelect* ) ) );
    connect( pattern10, SIGNAL( clicked( KSpreadPatternSelect* ) ),
	     this, SLOT( slotUnselect2( KSpreadPatternSelect* ) ) );

  connect( goUpDiagonal, SIGNAL( clicked (KSpreadBorderButton *) ),
	   this, SLOT( changeState( KSpreadBorderButton *) ) );
  connect( top, SIGNAL( clicked(KSpreadBorderButton *) ),
	   this, SLOT( changeState(KSpreadBorderButton *) ) );
  connect( right, SIGNAL( clicked(KSpreadBorderButton *) ),
	   this, SLOT( changeState(KSpreadBorderButton *) ) );

  connect( fallDiagonal, SIGNAL( clicked(KSpreadBorderButton *) ),
	   this, SLOT( changeState(KSpreadBorderButton *) ) );

  connect( bottom, SIGNAL( clicked(KSpreadBorderButton *) ),
	   this, SLOT( changeState(KSpreadBorderButton *) ) );
  connect( left, SIGNAL( clicked(KSpreadBorderButton *) ),
	   this, SLOT( changeState(KSpreadBorderButton *) ) );
  connect( horizontal, SIGNAL( clicked(KSpreadBorderButton *) ),
	   this, SLOT( changeState(KSpreadBorderButton *) ) );
  connect( vertical, SIGNAL( clicked(KSpreadBorderButton *) ),
	   this, SLOT( changeState(KSpreadBorderButton *) ) );

  connect( all, SIGNAL( clicked(KSpreadBorderButton *) ),
	   this, SLOT( preselect(KSpreadBorderButton *) ) );
  connect( remove, SIGNAL( clicked(KSpreadBorderButton *) ),
	   this, SLOT( preselect(KSpreadBorderButton *) ) );
  connect( outline, SIGNAL( clicked(KSpreadBorderButton *) ),
	   this, SLOT( preselect(KSpreadBorderButton *) ) );

  connect( area ,SIGNAL( redraw()),this,SLOT(draw()));
  connect( area ,SIGNAL( choosearea(QMouseEvent * )),
           this,SLOT( slotPressEvent(QMouseEvent *)));
  pattern1->slotSelect();
  selectedPattern=pattern1;
  this->resize( 400, 400 );
}

void CellLayoutPageBorder::loadIcon( QString _pix,KSpreadBorderButton *_button)
{
    QPixmap *pix = new QPixmap( KSBarIcon(_pix) ); 
    _button->setPixmap( *pix );
}

void CellLayoutPageBorder::applyOutline( int _left, int _top, int _right, int _bottom )
{


    if( horizontal->isChanged())
        {
        for ( int x = _left; x <= _right; x++ )
                {
                for ( int y = _top+1; y <= _bottom; y++ )
                {
	        KSpreadCell *obj = dlg->getTable()->nonDefaultCell( x, y );

	        obj->setTopBorderColor( horizontal->getColor() );
	        obj->setTopBorderStyle( horizontal->getPenStyle() );
	        obj->setTopBorderWidth( horizontal->getPenWidth() );


                }
                }
         }

    if( vertical->isChanged())
    {
    for ( int x = _left+1; x <= _right; x++ )
        {
        for ( int y = _top; y <= _bottom; y++ )
                {
                KSpreadCell *obj = dlg->getTable()->nonDefaultCell( x,y );

	        obj->setLeftBorderColor( vertical->getColor() );
	        obj->setLeftBorderStyle( vertical->getPenStyle() );
	        obj->setLeftBorderWidth( vertical->getPenWidth() );


                }
        }
    }


if ( left->isChanged() )
    {
    for ( int y = _top; y <= _bottom; y++ )
        {
        KSpreadCell *obj = dlg->getTable()->nonDefaultCell( _left,y );

	obj->setLeftBorderColor( left->getColor() );
	obj->setLeftBorderStyle( left->getPenStyle() );
	obj->setLeftBorderWidth( left->getPenWidth() );
        }
    }

 if ( right->isChanged() )
    {
    for ( int y = _top; y <= _bottom; y++ )
        {
        KSpreadCell *obj = dlg->getTable()->nonDefaultCell( _right,y );
        obj->setRightBorderColor( right->getColor() );
	obj->setRightBorderStyle( right->getPenStyle() );
	obj->setRightBorderWidth( right->getPenWidth() );
        }
    }

 if ( top->isChanged() )
    {
    for ( int x = _left; x <= _right; x++ )
        {
        KSpreadCell *obj = dlg->getTable()->nonDefaultCell( x,_top );

        obj->setTopBorderColor( top->getColor() );
	obj->setTopBorderStyle( top->getPenStyle() );
	obj->setTopBorderWidth( top->getPenWidth() );
        }
    }

 if ( bottom->isChanged() )
    {
    for ( int x = _left; x <= _right; x++ )
        {
        KSpreadCell *obj = dlg->getTable()->nonDefaultCell( x,_bottom );

        obj->setBottomBorderColor( bottom->getColor() );
	obj->setBottomBorderStyle( bottom->getPenStyle() );
	obj->setBottomBorderWidth( bottom->getPenWidth() );
        }
    }

for ( int x = _left; x <= _right; x++ )
        {
        for ( int y = _top; y <= _bottom; y++ )
                {
                KSpreadCell *obj = dlg->getTable()->nonDefaultCell( x,y );
                if ( fallDiagonal->isChanged() )
                        {
	                obj->setFallDiagonalColor( fallDiagonal->getColor() );
	                obj->setFallDiagonalStyle( fallDiagonal->getPenStyle() );
	                obj->setFallDiagonalWidth( fallDiagonal->getPenWidth() );
                        }
                if ( goUpDiagonal->isChanged() )
                        {
	                obj->setGoUpDiagonalColor( goUpDiagonal->getColor() );
	                obj->setGoUpDiagonalStyle( goUpDiagonal->getPenStyle() );
	                obj->setGoUpDiagonalWidth( goUpDiagonal->getPenWidth() );
                        }
                }
        }
}

void CellLayoutPageBorder::slotSetColorButton( const QColor &_color )
{
    currentColor = _color;

    pattern1->setColor( currentColor );
    pattern2->setColor( currentColor );
    pattern3->setColor( currentColor );
    pattern4->setColor( currentColor );
    pattern5->setColor( currentColor );
    pattern6->setColor( currentColor );
    pattern7->setColor( currentColor );
    pattern8->setColor( currentColor );
    pattern9->setColor( currentColor );
    pattern10->setColor( currentColor );

}

void CellLayoutPageBorder::slotUnselect2( KSpreadPatternSelect *_p )
{
    selectedPattern = _p;

    if ( pattern1 != _p )
	pattern1->slotUnselect();
    if ( pattern2 != _p )
	pattern2->slotUnselect();
    if ( pattern3 != _p )
	pattern3->slotUnselect();
    if ( pattern4 != _p )
	pattern4->slotUnselect();
    if ( pattern5 != _p )
	pattern5->slotUnselect();
    if ( pattern6 != _p )
	pattern6->slotUnselect();
    if ( pattern7 != _p )
	pattern7->slotUnselect();
    if ( pattern8 != _p )
	pattern8->slotUnselect();
    if ( pattern9 != _p )
	pattern9->slotUnselect();
    if ( pattern10 != _p )
	pattern10->slotUnselect();

}

void CellLayoutPageBorder::preselect( KSpreadBorderButton *_p)
{
_p->setOn(false);
if(_p==remove)
        {
         if(left->isOn())
                 left->unselect();

         if(right->isOn())
                 right->unselect();

          if(top->isOn())
	         top->unselect();

          if(bottom->isOn())
	         bottom->unselect();

          if(fallDiagonal->isOn())
	         fallDiagonal->unselect();

         if(goUpDiagonal->isOn())
                 goUpDiagonal->unselect();

          if(vertical->isOn())
                 vertical->unselect();

          if(horizontal->isOn())
                 horizontal->unselect();
        }
if(_p==outline)
        {
        top->setOn(true);
        top->setPenWidth(selectedPattern->getPenWidth());
        top->setPenStyle(selectedPattern->getPenStyle());
        top->setColor( currentColor );
        top->setChanged(true);
        bottom->setOn(true);
        bottom->setPenWidth(selectedPattern->getPenWidth());
        bottom->setPenStyle(selectedPattern->getPenStyle());
        bottom->setColor( currentColor );
        bottom->setChanged(true);
        left->setOn(true);
        left->setPenWidth(selectedPattern->getPenWidth());
        left->setPenStyle(selectedPattern->getPenStyle());
        left->setColor( currentColor );
        left->setChanged(true);
        right->setOn(true);
        right->setPenWidth(selectedPattern->getPenWidth());
        right->setPenStyle(selectedPattern->getPenStyle());
        right->setColor( currentColor );
        right->setChanged(true);
        }
if(_p==all)
        {
        if(dlg->oneRow==false)
                {
                horizontal->setOn(true);
                horizontal->setPenWidth(selectedPattern->getPenWidth());
                horizontal->setPenStyle(selectedPattern->getPenStyle());
                horizontal->setColor( currentColor );
                horizontal->setChanged(true);
                }
        if(dlg->oneCol==false)
                {
                vertical->setOn(true);
                vertical->setPenWidth(selectedPattern->getPenWidth());
                vertical->setPenStyle(selectedPattern->getPenStyle());
                vertical->setColor( currentColor );
                vertical->setChanged(true);
                }
        }
area->repaint();
}

void CellLayoutPageBorder::changeState( KSpreadBorderButton *_p)
{
  _p->setChanged(true);
  if ( selectedPattern != 0L )
    {
      if(_p->isOn())
	{
	  _p->setPenWidth(selectedPattern->getPenWidth());
	  _p->setPenStyle(selectedPattern->getPenStyle());
	  _p->setColor( currentColor );
	}
      else
	{
	  _p->setPenWidth(1);
	  _p->setPenStyle(Qt::NoPen);
	  _p->setColor( Qt::black );
	}
    }
 area->repaint();
}

void CellLayoutPageBorder::draw()
{
  QPen pen;
  QPainter painter;
  painter.begin( area );

  if((bottom->getPenStyle())!=Qt::NoPen)
    {
      pen.setColor( bottom->getColor() );
      pen.setStyle( bottom->getPenStyle() );
      pen.setWidth( bottom->getPenWidth() );

      painter.setPen( pen );
      painter.drawLine( OFFSETX, YHEI-OFFSETY, XLEN-OFFSETX , YHEI-OFFSETY );


    }
  if((top->getPenStyle())!=Qt::NoPen)
    {

      pen.setColor( top->getColor() );
      pen.setStyle( top->getPenStyle() );
      pen.setWidth( top->getPenWidth() );
      painter.setPen( pen );
      painter.drawLine( OFFSETX, OFFSETY, XLEN -OFFSETX, OFFSETY );

    }
 if((left->getPenStyle())!=Qt::NoPen)
    {

      pen.setColor( left->getColor() );
      pen.setStyle( left->getPenStyle() );
      pen.setWidth( left->getPenWidth() );
      painter.setPen( pen );
      painter.drawLine( OFFSETX, OFFSETY, OFFSETX , YHEI-OFFSETY );

    }
 if((right->getPenStyle())!=Qt::NoPen)
    {

      pen.setColor( right->getColor() );
      pen.setStyle( right->getPenStyle() );
      pen.setWidth( right->getPenWidth() );
      painter.setPen( pen );
      painter.drawLine( XLEN-OFFSETX, OFFSETY, XLEN- OFFSETX, YHEI-OFFSETY );

    }
 if((fallDiagonal->getPenStyle())!=Qt::NoPen)
    {

      pen.setColor( fallDiagonal->getColor() );
      pen.setStyle( fallDiagonal->getPenStyle() );
      pen.setWidth( fallDiagonal->getPenWidth() );
      painter.setPen( pen );
      painter.drawLine( OFFSETX, OFFSETY, XLEN-OFFSETX, YHEI-OFFSETY );

    }
 if((goUpDiagonal->getPenStyle())!=Qt::NoPen)
    {

      pen.setColor( goUpDiagonal->getColor() );
      pen.setStyle( goUpDiagonal->getPenStyle() );
      pen.setWidth( goUpDiagonal->getPenWidth() );
      painter.setPen( pen );
      painter.drawLine( OFFSETX, YHEI-OFFSETY , XLEN-OFFSETX , OFFSETY );

    }
 if((vertical->getPenStyle())!=Qt::NoPen)
    {

      pen.setColor( vertical->getColor() );
      pen.setStyle( vertical->getPenStyle() );
      pen.setWidth( vertical->getPenWidth() );
      painter.setPen( pen );
      painter.drawLine( XLEN/2, 5 , XLEN/2 , YHEI-5 );

    }
  if((horizontal->getPenStyle())!=Qt::NoPen)
    {

      pen.setColor( horizontal->getColor() );
      pen.setStyle( horizontal->getPenStyle() );
      pen.setWidth( horizontal->getPenWidth() );
      painter.setPen( pen );

      painter.drawLine( OFFSETX,YHEI/2,XLEN-OFFSETX, YHEI/2 );
    }
  painter.end();
}

void CellLayoutPageBorder::invertState(KSpreadBorderButton *_p)
{
if(_p->isOn())
        {
        _p->unselect();
        }
else
        {
        _p->setOn(!_p->isOn());
        _p->setPenWidth(selectedPattern->getPenWidth());
        _p->setPenStyle(selectedPattern->getPenStyle());
        _p->setColor( currentColor );
        _p->setChanged(true);
        }
}

void CellLayoutPageBorder::slotPressEvent(QMouseEvent *_ev)
{
QRect rect(OFFSETX,OFFSETY-8,XLEN-OFFSETX,OFFSETY+8);
if(rect.contains(QPoint(_ev->x(),_ev->y())))
        {
         invertState(top);
        }
rect.setCoords(OFFSETX,YHEI-OFFSETY-8,XLEN-OFFSETX,YHEI-OFFSETY+8);
if(rect.contains(QPoint(_ev->x(),_ev->y())))
        {
         invertState(bottom);
        }

rect.setCoords(OFFSETX-8,OFFSETY,OFFSETX+8,YHEI-OFFSETY);
if(rect.contains(QPoint(_ev->x(),_ev->y())))
        {
         invertState(left);
        }
rect.setCoords(XLEN-OFFSETX-8,OFFSETY,XLEN-OFFSETX+8,YHEI-OFFSETY);
if(rect.contains(QPoint(_ev->x(),_ev->y())))
        {
         invertState(right);
        }

//don't work because I don't know how create a rectangle
//for diagonal
/*rect.setCoords(OFFSETX,OFFSETY,XLEN-OFFSETX,YHEI-OFFSETY);
if(rect.contains(QPoint(_ev->x(),_ev->y())))
        {
         invertState(fallDiagonal);
        }
rect.setCoords(OFFSETX,YHEI-OFFSETY,XLEN-OFFSETX,OFFSETY);
if(rect.contains(QPoint(_ev->x(),_ev->y())))
        {
         invertState(goUpDiagonal);
        } */

if(dlg->oneCol==false)
        {
         rect.setCoords(XLEN/2-8,OFFSETY,XLEN/2+8,YHEI-OFFSETY);

        if(rect.contains(QPoint(_ev->x(),_ev->y())))
                {
                invertState(vertical);
                }
        }
if(dlg->oneRow==false)
        {
        rect.setCoords(OFFSETX,YHEI/2-8,XLEN-OFFSETX,YHEI/2+8);
        if(rect.contains(QPoint(_ev->x(),_ev->y())))
                {
                invertState(horizontal);
                }
        }

area->repaint();
}

KSpreadBrushSelect::KSpreadBrushSelect( QWidget *parent, const char * ) : QFrame( parent )
{
    brushStyle = Qt::NoBrush;
    brushColor = Qt::red;
    selected = FALSE;
}

void KSpreadBrushSelect::setPattern( const QColor &_color,BrushStyle _style )
{
    brushStyle = _style;
    brushColor = _color;
    repaint();
}


void KSpreadBrushSelect::paintEvent( QPaintEvent *_ev )
{
    QFrame::paintEvent( _ev );

    QPainter painter;
    QBrush brush;
    brush.setStyle(brushStyle);
    brush.setColor(brushColor);
    painter.begin( this );
    painter.setPen( Qt::NoPen );
    painter.setBrush( brush);
    painter.drawRect( 2, 2, width()-4, height()-4);
    painter.end();
}

void KSpreadBrushSelect::mousePressEvent( QMouseEvent * )
{
    slotSelect();

    emit clicked( this );
}

void KSpreadBrushSelect::slotUnselect()
{
    selected = FALSE;

    setLineWidth( 1 );
    setFrameStyle( QFrame::Panel | QFrame::Sunken );
    repaint();
}

void KSpreadBrushSelect::slotSelect()
{
    selected = TRUE;

    setLineWidth( 2 );
    setFrameStyle( QFrame::Panel | QFrame::Plain );
    repaint();
}



CellLayoutPagePattern::CellLayoutPagePattern( QWidget* parent, CellLayoutDlg *_dlg ) : QWidget( parent )
{
    dlg = _dlg;

    QGroupBox* tmpQGroupBox;
    tmpQGroupBox = new QGroupBox( this, "GroupBox" );
    tmpQGroupBox->setGeometry( 15, 15, 190, 185 );
    tmpQGroupBox->setFrameStyle( 49 );
    tmpQGroupBox->setTitle( i18n("Pattern") );
    tmpQGroupBox->setAlignment( 1 );

    color = new KColorButton (this, "ColorButton_1" );
    color->setGeometry( 200, 220, 80, 25 );

    QLabel *tmpQLabel = new QLabel( this, "Label_1" );
    tmpQLabel->setGeometry( 150, 220, 35, 25 );
    tmpQLabel->setText( i18n("Color") );

    brush1 = new KSpreadBrushSelect( this, "Frame_1" );
    brush1->setGeometry( 20, 40, 50, 20 );
    {
	QColorGroup normal( ( QColor( QRgb(0) ) ), QColor( QRgb(16777215) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QColorGroup disabled( ( QColor( QRgb(8421504) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(8421504) ), QColor( QRgb(12632256) ) );
	QColorGroup active( ( QColor( QRgb(0) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QPalette palette( normal, disabled, active );
	brush1->setPalette( palette );
    }
    brush1->setFrameStyle( 50 );

    brush2 = new KSpreadBrushSelect( this, "Frame_2" );
    brush2->setGeometry( 80, 40, 50, 20 );
    {
	QColorGroup normal( ( QColor( QRgb(0) ) ), QColor( QRgb(16777215) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QColorGroup disabled( ( QColor( QRgb(8421504) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(8421504) ), QColor( QRgb(12632256) ) );
	QColorGroup active( ( QColor( QRgb(0) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QPalette palette( normal, disabled, active );
	brush2->setPalette( palette );
    }
    brush2->setFrameStyle( 50 );

    brush3 = new KSpreadBrushSelect( this, "Frame_3" );
    brush3->setGeometry( 140, 40, 50, 20 );
    {
	QColorGroup normal( ( QColor( QRgb(0) ) ), QColor( QRgb(16777215) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QColorGroup disabled( ( QColor( QRgb(8421504) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(8421504) ), QColor( QRgb(12632256) ) );
	QColorGroup active( ( QColor( QRgb(0) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QPalette palette( normal, disabled, active );
	brush3->setPalette( palette );
    }
    brush3->setFrameStyle( 50 );

    brush4 = new KSpreadBrushSelect( this, "Frame_4" );
    brush4->setGeometry( 20, 70, 50, 20 );
    {
	QColorGroup normal( ( QColor( QRgb(0) ) ), QColor( QRgb(16777215) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QColorGroup disabled( ( QColor( QRgb(8421504) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(8421504) ), QColor( QRgb(12632256) ) );
	QColorGroup active( ( QColor( QRgb(0) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QPalette palette( normal, disabled, active );
	brush4->setPalette( palette );
    }
    brush4->setFrameStyle( 50 );

    brush5 = new KSpreadBrushSelect( this, "Frame_5" );
    brush5->setGeometry( 80, 70, 50, 20 );
    {
	QColorGroup normal( ( QColor( QRgb(0) ) ), QColor( QRgb(16777215) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QColorGroup disabled( ( QColor( QRgb(8421504) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(8421504) ), QColor( QRgb(12632256) ) );
	QColorGroup active( ( QColor( QRgb(0) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QPalette palette( normal, disabled, active );
	brush5->setPalette( palette );
    }
    brush5->setFrameStyle( 50 );

    brush6 = new KSpreadBrushSelect( this, "Frame_6" );
    brush6->setGeometry( 140, 70, 50, 20 );
    {
	QColorGroup normal( ( QColor( QRgb(0) ) ), QColor( QRgb(16777215) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QColorGroup disabled( ( QColor( QRgb(8421504) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(8421504) ), QColor( QRgb(12632256) ) );
	QColorGroup active( ( QColor( QRgb(0) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QPalette palette( normal, disabled, active );
	brush6->setPalette( palette );
    }
    brush6->setFrameStyle( 50 );

    brush7 = new KSpreadBrushSelect( this, "Frame_7" );
    brush7->setGeometry( 20, 100, 50, 20 );
    {
	QColorGroup normal( ( QColor( QRgb(0) ) ), QColor( QRgb(16777215) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QColorGroup disabled( ( QColor( QRgb(8421504) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(8421504) ), QColor( QRgb(12632256) ) );
	QColorGroup active( ( QColor( QRgb(0) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QPalette palette( normal, disabled, active );
	brush7->setPalette( palette );
    }
    brush7->setFrameStyle( 50 );

    brush8 = new KSpreadBrushSelect( this, "Frame_8" );
    brush8->setGeometry( 80, 100, 50, 20 );
    {
	QColorGroup normal( ( QColor( QRgb(0) ) ), QColor( QRgb(16777215) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QColorGroup disabled( ( QColor( QRgb(8421504) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(8421504) ), QColor( QRgb(12632256) ) );
	QColorGroup active( ( QColor( QRgb(0) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QPalette palette( normal, disabled, active );
	brush8->setPalette( palette );
    }
    brush8->setFrameStyle( 50 );

    brush9 = new KSpreadBrushSelect( this, "Frame_9" );
    brush9->setGeometry( 140, 100, 50, 20 );
    {
	QColorGroup normal( ( QColor( QRgb(0) ) ), QColor( QRgb(16777215) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QColorGroup disabled( ( QColor( QRgb(8421504) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(8421504) ), QColor( QRgb(12632256) ) );
	QColorGroup active( ( QColor( QRgb(0) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QPalette palette( normal, disabled, active );
	brush9->setPalette( palette );
    }
    brush9->setFrameStyle( 50 );

    brush10 = new KSpreadBrushSelect( this, "Frame_10" );
    brush10->setGeometry( 20, 130, 50, 20 );
    {
	QColorGroup normal( ( QColor( QRgb(0) ) ), QColor( QRgb(16777215) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QColorGroup disabled( ( QColor( QRgb(8421504) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(8421504) ), QColor( QRgb(12632256) ) );
	QColorGroup active( ( QColor( QRgb(0) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QPalette palette( normal, disabled, active );
	brush10->setPalette( palette );
    }
    brush10->setFrameStyle( 50 );

    brush11 = new KSpreadBrushSelect( this, "Frame_11" );
    brush11->setGeometry( 80, 130, 50, 20 );
    {
	QColorGroup normal( ( QColor( QRgb(0) ) ), QColor( QRgb(16777215) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QColorGroup disabled( ( QColor( QRgb(8421504) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(8421504) ), QColor( QRgb(12632256) ) );
	QColorGroup active( ( QColor( QRgb(0) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QPalette palette( normal, disabled, active );
	brush11->setPalette( palette );
    }
    brush11->setFrameStyle( 50 );

    brush12 = new KSpreadBrushSelect( this, "Frame_12" );
    brush12->setGeometry( 140, 130, 50, 20 );
    {
	QColorGroup normal( ( QColor( QRgb(0) ) ), QColor( QRgb(16777215) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QColorGroup disabled( ( QColor( QRgb(8421504) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(8421504) ), QColor( QRgb(12632256) ) );
	QColorGroup active( ( QColor( QRgb(0) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QPalette palette( normal, disabled, active );
	brush12->setPalette( palette );
    }
    brush12->setFrameStyle( 50 );

    brush13 = new KSpreadBrushSelect( this, "Frame_13" );
    brush13->setGeometry( 20, 160, 50, 20 );
    {
	QColorGroup normal( ( QColor( QRgb(0) ) ), QColor( QRgb(16777215) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QColorGroup disabled( ( QColor( QRgb(8421504) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(8421504) ), QColor( QRgb(12632256) ) );
	QColorGroup active( ( QColor( QRgb(0) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QPalette palette( normal, disabled, active );
	brush13->setPalette( palette );
    }
    brush13->setFrameStyle( 50 );

    brush14 = new KSpreadBrushSelect( this, "Frame_14" );
    brush14->setGeometry( 80, 160, 50, 20 );
    {
	QColorGroup normal( ( QColor( QRgb(0) ) ), QColor( QRgb(16777215) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QColorGroup disabled( ( QColor( QRgb(8421504) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(8421504) ), QColor( QRgb(12632256) ) );
	QColorGroup active( ( QColor( QRgb(0) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QPalette palette( normal, disabled, active );
	brush14->setPalette( palette );
    }
    brush14->setFrameStyle( 50 );

    brush15 = new KSpreadBrushSelect( this, "Frame_15" );
    brush15->setGeometry( 140, 160, 50, 20 );
    {
	QColorGroup normal( ( QColor( QRgb(0) ) ), QColor( QRgb(16777215) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QColorGroup disabled( ( QColor( QRgb(8421504) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(8421504) ), QColor( QRgb(12632256) ) );
	QColorGroup active( ( QColor( QRgb(0) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QPalette palette( normal, disabled, active );
	brush15->setPalette( palette );
    }
    brush15->setFrameStyle( 50 );

    tmpQLabel = new QLabel( this, "Label_2" );
    tmpQLabel->setGeometry( 20, 220, 50, 20 );
    tmpQLabel->setText( i18n("Current") );

    current = new KSpreadBrushSelect( this, "Current" );
    current->setGeometry( 80, 220, 50, 20 );
    {
	QColorGroup normal( ( QColor( QRgb(0) ) ), QColor( QRgb(16777215) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QColorGroup disabled( ( QColor( QRgb(8421504) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(8421504) ), QColor( QRgb(12632256) ) );
	QColorGroup active( ( QColor( QRgb(0) ) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
	QPalette palette( normal, disabled, active );
	current->setPalette( palette );
    }
    current->setFrameStyle( 50 );

    connect( brush1, SIGNAL( clicked( KSpreadBrushSelect* ) ),
	     this, SLOT( slotUnselect2( KSpreadBrushSelect* ) ) );
    connect( brush2, SIGNAL( clicked( KSpreadBrushSelect* ) ),
	     this, SLOT( slotUnselect2( KSpreadBrushSelect* ) ) );
    connect( brush3, SIGNAL( clicked( KSpreadBrushSelect* ) ),
	     this, SLOT( slotUnselect2( KSpreadBrushSelect* ) ) );
    connect( brush4, SIGNAL( clicked( KSpreadBrushSelect* ) ),
	     this, SLOT( slotUnselect2( KSpreadBrushSelect* ) ) );
    connect( brush5, SIGNAL( clicked( KSpreadBrushSelect* ) ),
	     this, SLOT( slotUnselect2( KSpreadBrushSelect* ) ) );
    connect( brush6, SIGNAL( clicked( KSpreadBrushSelect* ) ),
	     this, SLOT( slotUnselect2( KSpreadBrushSelect* ) ) );
    connect( brush7, SIGNAL( clicked( KSpreadBrushSelect* ) ),
	     this, SLOT( slotUnselect2( KSpreadBrushSelect* ) ) );
    connect( brush8, SIGNAL( clicked( KSpreadBrushSelect* ) ),
	     this, SLOT( slotUnselect2( KSpreadBrushSelect* ) ) );
    connect( brush9, SIGNAL( clicked( KSpreadBrushSelect* ) ),
	     this, SLOT( slotUnselect2( KSpreadBrushSelect* ) ) );
    connect( brush10, SIGNAL( clicked( KSpreadBrushSelect* ) ),
	     this, SLOT( slotUnselect2( KSpreadBrushSelect* ) ) );
    connect( brush11, SIGNAL( clicked( KSpreadBrushSelect* ) ),
	     this, SLOT( slotUnselect2( KSpreadBrushSelect* ) ) );
    connect( brush12, SIGNAL( clicked( KSpreadBrushSelect* ) ),
	     this, SLOT( slotUnselect2( KSpreadBrushSelect* ) ) );
    connect( brush13, SIGNAL( clicked( KSpreadBrushSelect* ) ),
	     this, SLOT( slotUnselect2( KSpreadBrushSelect* ) ) );
    connect( brush14, SIGNAL( clicked( KSpreadBrushSelect* ) ),
	     this, SLOT( slotUnselect2( KSpreadBrushSelect* ) ) );
    connect( brush15, SIGNAL( clicked( KSpreadBrushSelect* ) ),
	     this, SLOT( slotUnselect2( KSpreadBrushSelect* ) ) );

    brush1->setPattern( Qt::red, Qt::VerPattern );
    brush2->setPattern( Qt::red,Qt::HorPattern );
    brush3->setPattern( Qt::red,Qt::Dense1Pattern );
    brush4->setPattern( Qt::red,Qt::Dense2Pattern );
    brush5->setPattern( Qt::red,Qt::Dense3Pattern );
    brush6->setPattern( Qt::red,Qt::Dense4Pattern );
    brush7->setPattern( Qt::red,Qt::Dense5Pattern );
    brush8->setPattern( Qt::red,Qt::Dense6Pattern );
    brush9->setPattern(  Qt::red,Qt::Dense7Pattern );
    brush10->setPattern(  Qt::red,Qt::CrossPattern );
    brush11->setPattern( Qt::red,Qt::BDiagPattern );
    brush12->setPattern( Qt::red,Qt::FDiagPattern );
    brush13->setPattern( Qt::red,Qt::VerPattern );
    brush14->setPattern( Qt::red,Qt::DiagCrossPattern );
    brush15->setPattern( Qt::red,Qt::NoBrush );

    current->setPattern(dlg->brushColor,dlg->brushStyle);
    current->slotSelect();
    selectedBrush=current;
    color->setColor(dlg->brushColor);

    connect( color, SIGNAL( changed( const QColor & ) ),
	     this, SLOT( slotSetColorButton( const QColor & ) ) );

    slotSetColorButton( dlg->brushColor );
    init();
    this->resize( 400, 400 );
}

void CellLayoutPagePattern::init()
{
    if(dlg->brushStyle==Qt::VerPattern)
	{
    	brush1->slotSelect();
    	}
    else if(dlg->brushStyle==Qt::HorPattern)
    	{
    	brush2->slotSelect();
    	}
    else if(dlg->brushStyle==Qt::Dense1Pattern)
    	{
    	brush3->slotSelect();
    	}
    else if(dlg->brushStyle==Qt::Dense2Pattern)
    	{
    	brush4->slotSelect();
    	}
    else if(dlg->brushStyle==Qt::Dense3Pattern)
    	{
    	brush5->slotSelect();
    	}
    else if(dlg->brushStyle==Qt::Dense4Pattern)
    	{
    	brush6->slotSelect();
    	}
    else if(dlg->brushStyle==Qt::Dense5Pattern)
    	{
    	brush7->slotSelect();
    	}
    else if(dlg->brushStyle==Qt::Dense6Pattern)
    	{
    	brush8->slotSelect();
    	}
    else if(dlg->brushStyle==Qt::Dense7Pattern)
    	{
    	brush9->slotSelect();
    	}
    else if(dlg->brushStyle==Qt::CrossPattern)
    	{
    	brush10->slotSelect();
    	}
    else if(dlg->brushStyle==Qt::BDiagPattern)
        {
    	brush11->slotSelect();
    	}
    else if(dlg->brushStyle==Qt::FDiagPattern)
    	{
    	brush12->slotSelect();
    	}
    else if(dlg->brushStyle==Qt::VerPattern)
    	{
    	brush13->slotSelect();
    	}
    else if(dlg->brushStyle==Qt::DiagCrossPattern)
    	{
    	brush14->slotSelect();
    	}
    else if(dlg->brushStyle==Qt::NoBrush)
    	{
    	brush15->slotSelect();
    	}
    else
    	kdDebug(36001) << "Error in brushStyle" << endl; 
}

void CellLayoutPagePattern::slotSetColorButton( const QColor &_color )
{
    currentColor = _color;

    brush1->setBrushColor( currentColor );
    brush2->setBrushColor( currentColor );
    brush3->setBrushColor( currentColor );
    brush4->setBrushColor( currentColor );
    brush5->setBrushColor( currentColor );
    brush6->setBrushColor( currentColor );
    brush7->setBrushColor( currentColor );
    brush8->setBrushColor( currentColor );
    brush9->setBrushColor( currentColor );
    brush10->setBrushColor( currentColor );
    brush11->setBrushColor( currentColor );
    brush12->setBrushColor( currentColor );
    brush13->setBrushColor( currentColor );
    brush14->setBrushColor( currentColor );
    brush15->setBrushColor( currentColor );
    current->setBrushColor( currentColor );
}

void CellLayoutPagePattern::slotUnselect2( KSpreadBrushSelect *_p )
{
    selectedBrush = _p;

    if ( brush1 != _p )
	brush1->slotUnselect();
    if ( brush2 != _p )
	brush2->slotUnselect();
    if ( brush3 != _p )
	brush3->slotUnselect();
    if ( brush4 != _p )
	brush4->slotUnselect();
    if ( brush5 != _p )
	brush5->slotUnselect();
    if ( brush6 != _p )
	brush6->slotUnselect();
    if ( brush7 != _p )
	brush7->slotUnselect();
    if ( brush8 != _p )
	brush8->slotUnselect();
    if ( brush9 != _p )
	brush9->slotUnselect();
    if ( brush10 != _p )
	brush10->slotUnselect();
    if ( brush11 != _p )
	brush11->slotUnselect();
    if ( brush12 != _p )
	brush12->slotUnselect();
    if ( brush13 != _p )
	brush13->slotUnselect();
    if ( brush14 != _p )
	brush14->slotUnselect();
    if ( brush15 != _p )
	brush15->slotUnselect();

    current->setBrushStyle(selectedBrush->getBrushStyle());
}

void CellLayoutPagePattern::apply( KSpreadCell *_obj )
{
if(selectedBrush!=0L)
        {
         _obj->setBackGroundBrushColor(selectedBrush->getBrushColor() );
         _obj->setBackGroundBrushStyle(selectedBrush->getBrushStyle() );
        }
}
#include "kspread_dlg_layout.moc"
