/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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

#include <qprogressdialog.h>
#include <qdragobject.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qpainter.h>
#include <qwmatrix.h>
#include <qapplication.h>
#include <qpointarray.h>
#include <qpopupmenu.h>
#include <qimage.h>
#include <qdatetime.h>
#include <qdropsite.h>

#include <qrect.h>
#include <qsize.h>
#include <qpoint.h>

#include <page.h>
#include <kpresenter_view.h>
#include <footer_header.h>
#include <qwmf.h>
#include <kpbackground.h>
#include <kpclipartobject.h>
#include <kppixmapobject.h>
#include <gotopage.h>
#include <kptextobject.h>
#include <kpresenter_sound_player.h>
#include <notebar.h>
#include <kpresenter_utils.h>
#include <koparagcounter.h>
#include <kapplication.h>
#include <kmimemagic.h>
#include <kurl.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kprcommand.h>
#include <kcursor.h>
#include <koPoint.h>
#include <kozoomhandler.h>
#include <stdlib.h>
#include <qclipboard.h>


#include <math.h>

/******************************************************************/
/* class Page - Page                                              */
/******************************************************************/

/*====================== constructor =============================*/
Page::Page( QWidget *parent, const char *name, KPresenterView *_view )
    : QWidget( parent, name ), buffer( size() )
{
    setWFlags( WResizeNoErase );
    presMenu = 0;
    m_currentTextObjectView=0L;
    if ( parent ) {
        mousePressed = false;
        modType = MT_NONE;
        resizeObjNum = -1;
        editNum = -1;
        setBackgroundColor( white );
        view = _view;
        setupMenus();
        setMouseTracking( true );
        show();
        editMode = true;
        currPresPage = 1;
        currPresStep = 0;
        subPresStep = 0;
        _presFakt = 1.0;
        goingBack = false;
        drawMode = false;
        fillBlack = true;
        drawRubber = false;
        toolEditMode = TEM_MOUSE;
        tmpObjs.setAutoDelete( false );
        setAcceptDrops( true );
        inEffect = false;
        ratio = 0.0;
        keepRatio = false;
        mouseSelectedObject = false;
        selectedObjectPosition = -1;
        nextPageTimer = true;
        drawLineInDrawMode = false;
        soundPlayer = 0;
        m_drawPolyline = false;
        m_drawCubicBezierCurve = false;
        m_drawLineWithCubicBezierCurve = true;
        m_oldCubicBezierPointArray.putPoints( 0, 4, 0,0, 0,0, 0,0, 0,0 );
    } else {
        view = 0;
        hide();
    }

    setFocusPolicy( QWidget::StrongFocus );
    setFocus();
    setKeyCompression( true );
    installEventFilter( this );
    KCursor::setAutoHideCursor( this, true, true );

    connect( view->kPresenterDoc(), SIGNAL( sig_terminateEditing( KPTextObject * ) ),
             this, SLOT( terminateEditing( KPTextObject * ) ) );

}

/*======================== destructor ============================*/
Page::~Page()
{
    // deactivate possible opened textobject to avoid double deletion, KPTextObject deletes this already
    exitEditMode();

    delete presMenu;

    stopSound();
    delete soundPlayer;
}


bool Page::eventFilter( QObject *o, QEvent *e )
{

    if ( !o || !e )
        return TRUE;
    if ( m_currentTextObjectView  )
        KCursor::autoHideEventFilter( o, e );
    switch ( e->type() )
    {
    case QEvent::AccelOverride:
    {
        QKeyEvent * keyev = static_cast<QKeyEvent *>(e);
        if ( m_currentTextObjectView &&
             (keyev->key()==Key_Home ||keyev->key()==Key_End
              || keyev->key()==Key_Tab || keyev->key()==Key_Prior
                 || keyev->key()==Key_Next) )
        {
            m_currentTextObjectView->keyPressEvent( keyev );
            return true;
        }
    }
    case QEvent::FocusIn:
        if ( m_currentTextObjectView )
            m_currentTextObjectView->focusInEvent();
        return TRUE;
    case QEvent::FocusOut:
        if ( m_currentTextObjectView  )
            m_currentTextObjectView->focusOutEvent();
        return TRUE;
    case QEvent::KeyPress:
    {
#ifndef NDEBUG
        QKeyEvent * keyev = static_cast<QKeyEvent *>(e);
        // Debug keys
        if ( ( keyev->state() & ControlButton ) && ( keyev->state() & ShiftButton ) )
        {
            switch ( keyev->key() ) {
                case Key_P: // 'P' -> paragraph debug
                    printRTDebug( 0 );
                    break;
                case Key_V: // 'V' -> verbose parag debug
                    printRTDebug( 1 );
                    break;
                default:
                    break;
            }
        }
#endif
    }
        break;
    default:
        break;
    }
    return QWidget::eventFilter(o,e);
}

bool Page::focusNextPrevChild( bool )
{
    return TRUE; // Don't allow to go out of the canvas widget by pressing "Tab"
}


/*======================== paint event ===========================*/
void Page::paintEvent( QPaintEvent* paintEvent )
{
    //kdDebug(33001) << "Page::paintEvent " << paintEvent->rect().x() << "," << paintEvent->rect().y()
    //               << " " << paintEvent->rect().width() << "x" << paintEvent->rect().height() << endl;
    QPainter painter;

    painter.begin( &buffer );

    if ( editMode || !fillBlack )
        painter.fillRect( paintEvent->rect(), white );
    else
        painter.fillRect( paintEvent->rect(), black );

    painter.setClipping( true );
    painter.setClipRect( paintEvent->rect() );

    drawBackground( &painter, paintEvent->rect() );
    drawObjects( &painter, paintEvent->rect(), true );

    painter.end();

    bitBlt( this, paintEvent->rect().topLeft(), &buffer, paintEvent->rect() );
}

/*======================= draw background ========================*/
void Page::drawBackground( QPainter *painter, QRect rect, bool ignoreSkip )
{
    QRegion grayRegion( rect );
    QPtrListIterator<KPBackGround> it(*backgroundList());
    for ( int i = 0 ; it.current(); ++it, ++i ) {
        if ( editMode )
        {
            if ( !ignoreSkip && painter->device()->devType() != QInternal::Printer && i != (int)view->getCurrPgNum() - 1 )
            {
                //kdDebug(33001) << "Page::drawBackground skipping drawing" << endl;
                continue;
            }
            //kdDebug(33001) << "Page::drawBackground drawing bg for page " << i+1 << " editMode=" << editMode << endl;
            QRect pageRect = getPageRect( i, _presFakt );
            if ( rect.intersects( pageRect ) )
                it.current()->draw( painter, pageRect.topLeft(), true );
            // Include the border now
            pageRect.rLeft() -= 1;
            pageRect.rTop() -= 1;
            pageRect.rRight() += 1;
            pageRect.rBottom() += 1;
            grayRegion -= pageRect;
        }
        else if ( !editMode && static_cast<int>( currPresPage ) == i + 1 )
        {
            QRect pgRect = getPageRect( i, _presFakt, false );
                /*kdDebug(33001) << "Page::drawBackground pgRect: " << pgRect.x() << "," << pgRect.y()
                          << " " << pgRect.width() << "x" << pgRect.height() << endl;

                kdDebug(33001) << " with borders, topleft is: " << pgRect.x() +
                    view->kPresenterDoc()->getLeftBorder() * _presFakt
                          << "," << pgRect.y() +
                    view->kPresenterDoc()->getTopBorder() * _presFakt
                          << endl;
                */
            it.current()->draw( painter, QPoint( pgRect.x() +
                                                 qRound(view->kPresenterDoc()->getLeftBorder() * _presFakt),
                                                 pgRect.y() +
                                                 qRound(view->kPresenterDoc()->getTopBorder() * _presFakt) ),
                                false );
        }
    }

    // In edit mode we also want to draw the gray area out of the pages
    if ( editMode && !grayRegion.isEmpty() )
    {
        eraseEmptySpace( painter, grayRegion, QApplication::palette().active().brush( QColorGroup::Mid ) );
    }
}

// 100% stolen from KWord
void Page::eraseEmptySpace( QPainter * painter, const QRegion & emptySpaceRegion, const QBrush & brush )
{
    painter->save();
    // Translate emptySpaceRegion in device coordinates
    // ( ARGL why on earth isn't QPainter::setClipRegion in transformed coordinate system ?? )
    QRegion devReg;
    QMemArray<QRect>rs = emptySpaceRegion.rects();
    rs.detach();
    for ( uint i = 0 ; i < rs.size() ; ++i )
        rs[i] = painter->xForm( rs[i] );
    devReg.setRects( rs.data(), rs.size() );
    painter->setClipRegion( devReg );
    painter->setPen( Qt::NoPen );

    //kdDebug(33001) << "KWDocument::eraseEmptySpace emptySpaceRegion: " << DEBUGRECT( emptySpaceRegion.boundingRect() ) << endl;
    painter->fillRect( emptySpaceRegion.boundingRect(), brush );
    painter->restore();
}

/*========================= draw objects =========================*/
void Page::drawObjects( QPainter *painter, QRect rect, bool drawCursor, bool ignoreSkip )
{
    int pgNum = editMode ? (int)view->getCurrPgNum() : currPresPage;
    //kdDebug(33001) << "Page::drawObjects ----- pgNum=" << pgNum << " currPresStep=" << currPresStep << " drawCursor=" << drawCursor << endl;

    QPtrListIterator<KPObject> it(*objectList());
    for ( int i = 0 ; it.current(); ++it, ++i ) {
        KPObject *kpobject = it.current();
	int pg = getPageOfObj( i, _presFakt );
        //if (i<10) kdDebug(33001) << "Page::drawObjects object " << i << " page " << pg << " getPresNum=" << kpobject->getPresNum() << endl;

	if ( kpobject->isSticky() ||
	     ( rect.intersects( kpobject->getBoundingRect( diffx(), diffy() ) ) && editMode ) ||
	     ( !editMode && pg == static_cast<int>( currPresPage ) &&
	       kpobject->getPresNum() <= static_cast<int>( currPresStep ) &&
	       ( !kpobject->getDisappear() || kpobject->getDisappear() &&
		 kpobject->getDisappearNum() > static_cast<int>( currPresStep ) ) ) ) {
 	    if ( inEffect && kpobject->getPresNum() >= static_cast<int>( currPresStep ) )
 		continue;

	    if ( !editMode && static_cast<int>( currPresStep ) == kpobject->getPresNum() && !goingBack ) {
                //kdDebug(33001) << "                 setSubPresStep " << subPresStep << endl;
		kpobject->setSubPresStep( subPresStep );
		kpobject->doSpecificEffects( true, false );
	    }

	    if ( !ignoreSkip && !kpobject->isSticky() &&
		 editMode && painter->device()->devType() != QInternal::Printer && pg != pgNum )
		continue;

	    QPoint op;
	    if ( kpobject->isSticky() ) {
		op = kpobject->getOrig();
		kpobject->setOrig( op.x(), op.y() - pg * getPageRect( 0, _presFakt ).height() + pgNum * getPageRect( 0, _presFakt ).height() );
	    }

            //kdDebug(33001) << "                 drawing object at " << diffx() << "," << diffy() << "  and setting subpresstep to 0 !" << endl;
            if ( drawCursor && kpobject->getType() == OT_TEXT && m_currentTextObjectView )
            {
                KPTextObject* textObject = static_cast<KPTextObject*>( kpobject );
                if ( m_currentTextObjectView->kpTextObject() == textObject ) // This is the object we are editing
                     textObject->draw( painter, diffx(), diffy(),
                                       false /*onlyChanged. Pass as param ?*/,
                                       m_currentTextObjectView->cursor(), true /* idem */);
            }
            else
                kpobject->draw( painter, diffx(), diffy() );
	    kpobject->setSubPresStep( 0 );
	    kpobject->doSpecificEffects( false );
	    if ( kpobject->isSticky() )
		kpobject->setOrig( op );
	}
    }
}

/*==================== handle mouse pressed ======================*/
void Page::mousePressEvent( QMouseEvent *e )
{
    if(!view->koDocument()->isReadWrite())
        return;

    if(m_currentTextObjectView)
    {
        KPTextObject *txtObj=m_currentTextObjectView->kpTextObject();
        Q_ASSERT(txtObj);
        if(txtObj->contains( e->pos(), diffx(), diffy() ))
        {
            QPoint pos=e->pos() - txtObj->getOrig();
            pos=view->zoomHandler()->pixelToLayoutUnit(QPoint(pos.x()+ diffx(),pos.y()+diffy()));
            mousePressed=true;
            if(e->button() == RightButton)
            {
                m_currentTextObjectView->showPopup( view, QCursor::pos(), view->actionList() );
                mousePressed=false;
            }
            else if( e->button() == MidButton )
            {
                QApplication::clipboard()->setSelectionMode( true );
                m_currentTextObjectView->paste();
                QApplication::clipboard()->setSelectionMode( false );
            }
            else
                m_currentTextObjectView->mousePressEvent(e, pos);
            return;
        }
    }


    //disallow selecting objects outside the "page"
    if ( editMode ) {
        if ( !view->kPresenterDoc()->getPageRect( view->getCurrPgNum()-1, diffx(), diffy(), _presFakt ).contains(e->pos()))
            return;
    }

    if ( e->state() & ControlButton )
        keepRatio = true;

    KPObject *kpobject = 0;

    oldMx = e->x();
    oldMy = e->y();

    resizeObjNum = -1;

    exitEditMode();

    if ( editMode ) {
        if ( e->button() == LeftButton ) {
            mousePressed = true;

            if ( m_drawPolyline && toolEditMode == INS_POLYLINE ) {
                m_dragStartPoint = QPoint( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx(),
                                           ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy() );
                m_pointArray.putPoints( m_indexPointArray, 1, m_dragStartPoint.x(), m_dragStartPoint.y() );
                ++m_indexPointArray;
                return;
            }

            if ( m_drawCubicBezierCurve && ( toolEditMode == INS_CUBICBEZIERCURVE || toolEditMode == INS_QUADRICBEZIERCURVE ) ) {
                if ( m_drawLineWithCubicBezierCurve ) {
                    QPainter p( this );
                    p.setPen( QPen( Qt::black, 1, Qt::SolidLine ) );
                    p.setBrush( Qt::NoBrush );
                    p.setRasterOp( Qt::NotROP );

                    QPoint oldStartPoint = m_dragStartPoint;

                    m_dragStartPoint = QPoint( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx(),
                                               ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy() );

                    p.drawLine( oldStartPoint, m_dragStartPoint );  // erase old line
                    p.end();

                    m_pointArray.putPoints( m_indexPointArray, 1, m_dragStartPoint.x(), m_dragStartPoint.y() );
                    ++m_indexPointArray;
                    m_drawLineWithCubicBezierCurve = false;
                }
                else {
                    QPoint _oldEndPoint = QPoint( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx(),
                                                  ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy() );
                    QPainter p( this );
                    QPen _pen = QPen( Qt::black, 1, Qt::DashLine );
                    p.setPen( _pen );
                    p.setBrush( Qt::NoBrush );
                    p.setRasterOp( Qt::NotROP );

                    p.save();
                    double _angle = getAngle( _oldEndPoint, m_dragStartPoint );
                    drawFigure( L_SQUARE, &p, _oldEndPoint, _pen.color(), _pen.width(), _angle ); // erase old figure
                    p.restore();

                    p.drawLine( m_dragStartPoint, _oldEndPoint ); // erase old line

                    int p_x = m_dragStartPoint.x() * 2 - _oldEndPoint.x();
                    int p_y = m_dragStartPoint.y() * 2 - _oldEndPoint.y();
                    QPoint _oldSymmetricEndPoint = QPoint( p_x, p_y );

                    p.save();
                    _angle = getAngle( _oldSymmetricEndPoint, m_dragStartPoint );
                    drawFigure( L_SQUARE, &p, _oldSymmetricEndPoint, _pen.color(), _pen.width(), _angle );  // erase old figure
                    p.restore();

                    p.drawLine( m_dragStartPoint, _oldSymmetricEndPoint );  // erase old line

                    m_pointArray.putPoints( m_indexPointArray, 3, m_CubicBezierSecondPoint.x(), m_CubicBezierSecondPoint.y(),
                                            m_CubicBezierThirdPoint.x(), m_CubicBezierThirdPoint.y(),
                                            m_dragStartPoint.x(), m_dragStartPoint.y() );
                    m_indexPointArray += 3;
                    m_drawLineWithCubicBezierCurve = true;
                    m_oldCubicBezierPointArray = QPointArray();
                    m_oldCubicBezierPointArray.putPoints( 0, 4, 0,0, 0,0, 0,0, 0,0 );
                    m_dragEndPoint = m_dragStartPoint;
                }

                return;
            }

            switch ( toolEditMode ) {
                case TEM_MOUSE: {
                    bool overObject = false;
                    bool deSelAll = true;
                    KPObject *kpobject = 0;

                    firstX = e->x();
                    firstY = e->y();
                    if ( (int)objectList()->count() - 1 >= 0 ) {
                        for ( int i = static_cast<int>( objectList()->count() ) - 1; i >= 0 ; i-- ) {
                            kpobject = objectList()->at( i );
                            QSize s = kpobject->getSize();
                            QPoint pnt = kpobject->getOrig();
                            if ( QRect( pnt.x() - diffx(), pnt.y() - diffy(), s.width(), s.height() ).
                                 contains( QPoint( e->x(), e->y() ) ) ) {
                                overObject = true;
                                if ( kpobject->isSelected() && modType == MT_MOVE ) deSelAll = false;
                                if ( kpobject->isSelected() && modType != MT_MOVE && modType != MT_NONE ) {
                                    oldBoundingRect = kpobject->getBoundingRect( 0, 0 );
                                    resizeObjNum = i;
                                }
                                break;
                            }
                        }
                    }

                    if ( deSelAll && !( e->state() & ShiftButton ) && !( e->state() & ControlButton ) )
                        deSelectAllObj();

                    if ( overObject ) {
                        if(!kpobject->isSelected())
                            selectObj( kpobject );
                        else if(kpobject->isSelected() &&  (e->state() & ShiftButton))
                            deSelectObj(kpobject);
                        modType = MT_NONE;
                        raiseObject();
                    } else {
                        modType = MT_NONE;
                        if ( !( e->state() & ShiftButton ) && !( e->state() & ControlButton ) )
                            deSelectAllObj();
                        drawRubber = true;
                        rubber = QRect( e->x(), e->y(), 0, 0 );
                    }

                } break;
                case INS_FREEHAND: {
                    deSelectAllObj();
                    mousePressed = true;
                    insRect = QRect( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx(),
                                     ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy(), 0, 0 );

                    m_indexPointArray = 0;
                    m_dragStartPoint = QPoint( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx(),
                                               ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy() );
                    m_dragEndPoint = m_dragStartPoint;
                    m_pointArray.putPoints( m_indexPointArray, 1, m_dragStartPoint.x(), m_dragStartPoint.y() );
                    ++m_indexPointArray;
                } break;
                case INS_POLYLINE: {
                    deSelectAllObj();
                    mousePressed = true;
                    insRect = QRect( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx(),
                                     ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy(), 0, 0 );

                    m_drawPolyline = true;
                    m_indexPointArray = 0;
                    m_dragStartPoint = QPoint( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx(),
                                               ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy() );
                    m_dragEndPoint = m_dragStartPoint;
                    m_pointArray.putPoints( m_indexPointArray, 1, m_dragStartPoint.x(), m_dragStartPoint.y() );
                    ++m_indexPointArray;
                } break;
                case INS_CUBICBEZIERCURVE: case INS_QUADRICBEZIERCURVE: {
                    deSelectAllObj();
                    mousePressed = true;
                    insRect = QRect( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx(),
                                     ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy(), 0, 0 );

                    m_drawCubicBezierCurve = true;
                    m_drawLineWithCubicBezierCurve = true;
                    m_indexPointArray = 0;
                    m_oldCubicBezierPointArray.putPoints( 0, 4, 0,0, 0,0, 0,0, 0,0 );
                    m_dragStartPoint = QPoint( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx(),
                                               ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy() );
                    m_dragEndPoint = m_dragStartPoint;
                    m_pointArray.putPoints( m_indexPointArray, 1, m_dragStartPoint.x(), m_dragStartPoint.y() );
                    ++m_indexPointArray;
                } break;
                case INS_POLYGON: {
                    deSelectAllObj();
                    mousePressed = true;
                    insRect = QRect( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx(),
                                     ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy(), 0, 0 );

                    m_indexPointArray = 0;
                    m_dragStartPoint = QPoint( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx(),
                                               ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy() );
                    m_dragEndPoint = m_dragStartPoint;
                } break;
                default: {
                    deSelectAllObj();
                    mousePressed = true;
                    insRect = QRect( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx(),
                                     ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy(), 0, 0 );
                } break;
            }
        }

        if ( e->button() == RightButton && toolEditMode == INS_POLYLINE && !m_pointArray.isNull() && m_drawPolyline ) {
            m_dragStartPoint = QPoint( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx(),
                                       ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy() );
            m_pointArray.putPoints( m_indexPointArray, 1, m_dragStartPoint.x(), m_dragStartPoint.y() );
            ++m_indexPointArray;
            endDrawPolyline();

            mouseMoveEvent( e );

            return;
        }

        if ( e->button() == RightButton && ( toolEditMode == INS_CUBICBEZIERCURVE || toolEditMode == INS_QUADRICBEZIERCURVE )
             && !m_pointArray.isNull() && m_drawCubicBezierCurve ) {
            if ( m_drawLineWithCubicBezierCurve ) {
                QPoint point = QPoint( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx(),
                                       ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy() );
                m_pointArray.putPoints( m_indexPointArray, 1, point.x(), point.y() );
                ++m_indexPointArray;
            }
            else {
                m_pointArray.putPoints( m_indexPointArray, 2, m_CubicBezierSecondPoint.x(), m_CubicBezierSecondPoint.y(),
                                        m_CubicBezierThirdPoint.x(), m_CubicBezierThirdPoint.y() );
                m_indexPointArray += 2;
            }

            endDrawCubicBezierCurve();

            mouseMoveEvent( e );

            return;
        }

        if ( e->button() == RightButton && toolEditMode == TEM_MOUSE ) {
            int num = getObjectAt( e->x(), e->y() );
            if ( num != -1 ) {
                kpobject = objectList()->at( num );
                bool state=!( e->state() & ShiftButton ) && !( e->state() & ControlButton ) && !kpobject->isSelected();
                QPoint pnt = QCursor::pos();
                if ( kpobject->getType() == OT_PICTURE ) {
                    mousePressed = false;
                    deSelectAllObj();
                    selectObj( kpobject );
                    view->openPopupMenuPicObject(pnt);
                } else if ( kpobject->getType() == OT_CLIPART ) {
                    mousePressed = false;
                    deSelectAllObj();
                    selectObj( kpobject );
                    view->openPopupMenuClipObject(pnt);
                } else if ( kpobject->getType() == OT_TEXT ) {
                    if ( state )
                        deSelectAllObj();
                    selectObj( kpobject );
                    view->openPopupMenuTextObject(pnt);
                    mousePressed = false;
                } else if ( kpobject->getType() == OT_PIE ) {
                    if ( state )
                        deSelectAllObj();
                    selectObj( kpobject );
                    view->openPopupMenuPieObject( pnt );
                    mousePressed = false;
                } else if ( kpobject->getType() == OT_RECT ) {
                    if ( state )
                        deSelectAllObj();
                    selectObj( kpobject );
                    view->openPopupMenuRectangleObject( pnt );
                    mousePressed = false;
                } else if ( kpobject->getType() == OT_PART ) {
                    if ( state )
                        deSelectAllObj();
                    selectObj( kpobject );
                    view->openPopupMenuPartObject( pnt );
                    mousePressed = false;
                } else if ( kpobject->getType() == OT_POLYGON ) {
                    if ( state )
                        deSelectAllObj();
                    selectObj( kpobject );
                    view->openPopupMenuPolygonObject( pnt );
                    mousePressed = false;
                } else {
                    if ( state )
                        deSelectAllObj();
                    selectObj( kpobject );
                    view->openPopupMenuGraphMenu( pnt );
                    mousePressed = false;
                }
                modType = MT_NONE;
            } else {
                QPoint pnt = QCursor::pos();
                view->openPopupMenuMenuPage( pnt );
                mousePressed = false;
                modType = MT_NONE;
            }
        }
        else if( e->button() == RightButton && toolEditMode != TEM_MOUSE ) {
            //desactivate tools when you click on right button
            setToolEditMode( TEM_MOUSE );
        }
    } else {
        oldMx = e->x();
        oldMy = e->y();
        if ( e->button() == LeftButton ) {
            if ( presMenu->isVisible() ) {
                presMenu->hide();
                setCursor( blankCursor );
            } else {
                if ( drawMode )
                    drawLineInDrawMode = true;
                else
                    view->screenNext();
            }
        } else if ( e->button() == MidButton )
            view->screenPrev();
        else if ( e->button() == RightButton ) {
            if ( !drawMode && !spManualSwitch() )
                view->autoScreenPresStopTimer();

            setCursor( arrowCursor );
            QPoint pnt = QCursor::pos();
            presMenu->popup( pnt );
        }
    }

    mouseMoveEvent( e );

    if ( modType != MT_NONE && modType != MT_MOVE ) {
        KPObject *kpobject = objectList()->at( resizeObjNum );
        if ( kpobject ) {
            ratio = static_cast<double>( static_cast<double>( kpobject->getSize().width() ) /
                                         static_cast<double>( kpobject->getSize().height() ) );
            oldRect = QRect( kpobject->getOrig().x(), kpobject->getOrig().y(),
                             kpobject->getSize().width(), kpobject->getSize().height() );
        }
    }
}

/*=================== handle mouse released ======================*/
void Page::mouseReleaseEvent( QMouseEvent *e )
{
    if(m_currentTextObjectView)
    {
        KPTextObject *txtObj=m_currentTextObjectView->kpTextObject();
        Q_ASSERT(txtObj);
        if(txtObj->contains( e->pos(), diffx(), diffy() ))
        {
            m_currentTextObjectView->mouseReleaseEvent(  e, QPoint());
            mousePressed=false;
            emit objectSelectedChanged();
            return;
        }
    }

    if ( e->button() != LeftButton ) {
        ratio = 0.0;
        keepRatio = false;
        return;
    }

    if ( drawMode ) {
        drawLineInDrawMode = false;
        return;
    }

    int mx = e->x();
    int my = e->y();
    mx = ( mx / rastX() ) * rastX();
    my = ( my / rastY() ) * rastY();
    firstX = ( firstX / rastX() ) * rastX();
    firstY = ( firstY / rastY() ) * rastY();
    QPtrList<KPObject> _objects;
    _objects.setAutoDelete( false );
    KPObject *kpobject = 0;

    if ( ( m_drawPolyline && toolEditMode == INS_POLYLINE )
         || ( m_drawCubicBezierCurve && ( toolEditMode == INS_CUBICBEZIERCURVE || toolEditMode == INS_QUADRICBEZIERCURVE ) ) ) {
        return;
    }

    if ( toolEditMode != INS_LINE )
        insRect = insRect.normalize();

    QPoint mv;
    QSize sz;
    if ( toolEditMode == TEM_MOUSE && modType != MT_NONE && modType != MT_MOVE  && resizeObjNum != -1 ) {
        kpobject = objectList()->at( resizeObjNum );
        if ( kpobject ) {
            mv = QPoint( kpobject->getOrig().x() - oldRect.x(),
                         kpobject->getOrig().y() - oldRect.y() );
            sz = QSize( kpobject->getSize().width() - oldRect.width(),
                        kpobject->getSize().height() - oldRect.height() );
        }
        kpobject = 0L;
    }

    switch ( toolEditMode ) {
    case TEM_MOUSE: {
        switch ( modType ) {
        case MT_NONE: {
            if ( drawRubber ) {
                QPainter p;
                p.begin( this );
                p.setRasterOp( NotROP );
                p.setPen( QPen( black, 0, DotLine ) );
                p.drawRect( rubber );
                p.end();
                drawRubber = false;

                rubber = rubber.normalize();
                KPObject *kpobject = 0;
                if ( (int)objectList()->count() - 1 >= 0 ) {
                    for ( int i = static_cast<int>( objectList()->count() ) - 1; i >= 0; i-- ) {
                        kpobject = objectList()->at( i );
                        if ( kpobject->intersects( rubber, diffx(), diffy() ) )
                            selectObj( kpobject );
                    }
                }
            }
        } break;
        case MT_MOVE: {
            if ( firstX != mx || firstY != my ) {
                if ( (int)objectList()->count() - 1 >= 0 ) {
                    for ( int i = static_cast<int>( objectList()->count() ) - 1; i >= 0; i-- ) {
                        kpobject = objectList()->at( i );
                        if ( kpobject->isSelected() ) {
                            kpobject->setMove( false );
                            _objects.append( kpobject );
                            _repaint( QRect( kpobject->getBoundingRect( 0, 0 ).x() + ( firstX - mx ),
                                             kpobject->getBoundingRect( 0, 0 ).y() + ( firstY - my ),
                                             kpobject->getBoundingRect( 0, 0 ).width(),
                                             kpobject->getBoundingRect( 0, 0 ).height() ) );
                            _repaint( kpobject );
                        }
                    }
                }
                MoveByCmd *moveByCmd = new MoveByCmd( i18n( "Move object(s)" ),
                                                      QPoint( mx - firstX, my - firstY ),
                                                      _objects, view->kPresenterDoc() );
                view->kPresenterDoc()->addCommand( moveByCmd );
            } else
                if ( (int)objectList()->count() - 1 >= 0 ) {
                    for ( int i = static_cast<int>( objectList()->count() ) - 1; i >= 0; i-- ) {
                        kpobject = objectList()->at( i );
                        if ( kpobject->isSelected() ) {
                            kpobject->setMove( false );
                            _repaint( kpobject );
                        }
                    }
                }
        } break;
        case MT_RESIZE_UP: {
            if ( resizeObjNum < 0 ) break;
            if ( firstX != mx || firstY != my ) {
                kpobject = objectList()->at( resizeObjNum );
                ResizeCmd *resizeCmd = new ResizeCmd( i18n( "Resize object up" ), mv, sz,
                                                      kpobject, view->kPresenterDoc() );
                kpobject->setMove( false );
                resizeCmd->unexecute( false );
                resizeCmd->execute();
                view->kPresenterDoc()->addCommand( resizeCmd );
            }
            kpobject = objectList()->at( resizeObjNum );
            kpobject->setMove( false );
            _repaint( oldBoundingRect );
            _repaint( kpobject );
        } break;
        case MT_RESIZE_DN: {
            if ( resizeObjNum < 0 ) break;
            if ( firstX != mx || firstY != my ) {
                kpobject = objectList()->at( resizeObjNum );
                ResizeCmd *resizeCmd = new ResizeCmd( i18n( "Resize object down" ), mv, sz,
                                                      kpobject, view->kPresenterDoc() );
                kpobject->setMove( false );
                resizeCmd->unexecute( false );
                resizeCmd->execute();
                view->kPresenterDoc()->addCommand( resizeCmd );
            }
            kpobject = objectList()->at( resizeObjNum );
            kpobject->setMove( false );
            _repaint( oldBoundingRect );
            _repaint( kpobject );
        } break;
        case MT_RESIZE_LF: {
            if ( resizeObjNum < 0 ) break;
            if ( firstX != mx || firstY != my ) {
                kpobject = objectList()->at( resizeObjNum );
                ResizeCmd *resizeCmd = new ResizeCmd( i18n( "Resize object left" ), mv, sz,
                                                      kpobject, view->kPresenterDoc() );
                kpobject->setMove( false );
                resizeCmd->unexecute( false );
                resizeCmd->execute();
                view->kPresenterDoc()->addCommand( resizeCmd );
            }
            kpobject = objectList()->at( resizeObjNum );
            kpobject->setMove( false );
            _repaint( oldBoundingRect );
            _repaint( kpobject );
        } break;
        case MT_RESIZE_RT: {
            if ( resizeObjNum < 0 ) break;
            if ( firstX != mx || firstY != my ) {
                kpobject = objectList()->at( resizeObjNum );
                ResizeCmd *resizeCmd = new ResizeCmd( i18n( "Resize object right" ), mv, sz,
                                                      kpobject, view->kPresenterDoc() );
                kpobject->setMove( false );
                resizeCmd->unexecute( false );
                resizeCmd->execute();
                view->kPresenterDoc()->addCommand( resizeCmd );
            }
            kpobject = objectList()->at( resizeObjNum );
            kpobject->setMove( false );
            _repaint( oldBoundingRect );
            _repaint( kpobject );
        } break;
        case MT_RESIZE_LU: {
            if ( resizeObjNum < 0 ) break;
            if ( firstX != mx || firstY != my ) {
                kpobject = objectList()->at( resizeObjNum );
                ResizeCmd *resizeCmd = new ResizeCmd( i18n( "Resize object left up" ), mv, sz,
                                                      kpobject, view->kPresenterDoc() );
                kpobject->setMove( false );
                resizeCmd->unexecute( false );
                resizeCmd->execute();
                view->kPresenterDoc()->addCommand( resizeCmd );
            }
            kpobject = objectList()->at( resizeObjNum );
            kpobject->setMove( false );
            _repaint( oldBoundingRect );
            _repaint( kpobject );
        } break;
        case MT_RESIZE_LD: {
            if ( resizeObjNum < 0 ) break;
            if ( firstX != mx || firstY != my ) {
                kpobject = objectList()->at( resizeObjNum );
                ResizeCmd *resizeCmd = new ResizeCmd( i18n( "Resize object left and down" ), mv, sz,
                                                      kpobject, view->kPresenterDoc() );
                kpobject->setMove( false );
                resizeCmd->unexecute( false );
                resizeCmd->execute();
                view->kPresenterDoc()->addCommand( resizeCmd );
            }
            kpobject = objectList()->at( resizeObjNum );
            kpobject->setMove( false );
            _repaint( oldBoundingRect );
            _repaint( kpobject );
        } break;
        case MT_RESIZE_RU: {
            if ( resizeObjNum < 0 ) break;
            if ( firstX != mx || firstY != my ) {
                kpobject = objectList()->at( resizeObjNum );
                ResizeCmd *resizeCmd = new ResizeCmd( i18n( "Resize object right and up" ), mv, sz,
                                                      kpobject, view->kPresenterDoc() );
                kpobject->setMove( false );
                resizeCmd->unexecute( false );
                resizeCmd->execute();
                view->kPresenterDoc()->addCommand( resizeCmd );
            }
            kpobject = objectList()->at( resizeObjNum );
            kpobject->setMove( false );
            _repaint( oldBoundingRect );
            _repaint( kpobject );
        } break;
        case MT_RESIZE_RD: {
            if ( resizeObjNum < 0 ) break;
            if ( firstX != mx || firstY != my ) {
                kpobject = objectList()->at( resizeObjNum );
                ResizeCmd *resizeCmd = new ResizeCmd( i18n( "Resize object right and down" ), mv, sz,
                                                      kpobject, view->kPresenterDoc() );
                kpobject->setMove( false );
                resizeCmd->unexecute( false );
                resizeCmd->execute();
                view->kPresenterDoc()->addCommand( resizeCmd );
            }
            kpobject = objectList()->at( resizeObjNum );
            kpobject->setMove( false );
            _repaint( oldBoundingRect );
            _repaint( kpobject );
        } break;
        }
    } break;
    case INS_TEXT: {
        if ( !insRect.isNull() ) {
            insertText( insRect );
            setToolEditMode( TEM_MOUSE );
        }
    } break;
    case INS_LINE: {
        if ( insRect.width() != 0 && insRect.height() != 0 ) {
            if ( insRect.top() == insRect.bottom() ) {
                bool reverse = insRect.left() > insRect.right();
                insRect = insRect.normalize();
                insRect.setRect( insRect.left(), insRect.top() - rastY() / 2,
                                 insRect.width(), rastY() );
                insertLineH( insRect, reverse );
            } else if ( insRect.left() == insRect.right() ) {
                bool reverse = insRect.top() > insRect.bottom();
                insRect = insRect.normalize();
                insRect.setRect( insRect.left() - rastX() / 2, insRect.top(),
                                 rastX(), insRect.height() );
                insertLineV( insRect, reverse );
            } else if ( insRect.left() < insRect.right() && insRect.top() < insRect.bottom() ||
                      insRect.left() > insRect.right() && insRect.top() > insRect.bottom() ) {
                bool reverse = insRect.left() > insRect.right() && insRect.top() > insRect.bottom();
                insertLineD1( insRect.normalize(), reverse );
            } else {
                bool reverse = insRect.right() < insRect.left() && insRect.top() < insRect.bottom();
                insertLineD2( insRect.normalize(), reverse );
            }
        }
    } break;
    case INS_RECT:
        if ( !insRect.isNull() ) insertRect( insRect );
        break;
    case INS_ELLIPSE:
        if ( !insRect.isNull() ) insertEllipse( insRect );
        break;
    case INS_PIE:
        if ( !insRect.isNull() ) insertPie( insRect );
        break;
    case INS_OBJECT:
    case INS_DIAGRAMM:
    case INS_TABLE:
    case INS_FORMULA: {
        if ( !insRect.isNull() ) insertObject( insRect );
        setToolEditMode( TEM_MOUSE );
    } break;
    case INS_AUTOFORM: {
        bool reverse = insRect.left() > insRect.right() || insRect.top() > insRect.bottom();
        if ( !insRect.isNull() ) insertAutoform( insRect, reverse );
        setToolEditMode( TEM_MOUSE );
    } break;
    case INS_FREEHAND:
        if ( !m_pointArray.isNull() ) insertFreehand( m_pointArray );
        break;
    case INS_POLYGON:
        if ( !m_pointArray.isNull() ) insertPolygon( m_pointArray );
        break;
    default: break;
    }
    emit objectSelectedChanged();
    if ( toolEditMode != TEM_MOUSE && editMode )
        repaint( false );

    mousePressed = false;
    modType = MT_NONE;
    resizeObjNum = -1;
    mouseMoveEvent( e );
    ratio = 0.0;
    keepRatio = false;
}

/*==================== handle mouse moved ========================*/
void Page::mouseMoveEvent( QMouseEvent *e )
{
    if(m_currentTextObjectView)
    {
        KPTextObject *txtObj=m_currentTextObjectView->kpTextObject();
        Q_ASSERT(txtObj);
        if(txtObj->contains( e->pos(), diffx(), diffy() )&&mousePressed)
        {
            QPoint pos=e->pos() - txtObj->getOrig();
            pos=view->zoomHandler()->pixelToLayoutUnit(QPoint(pos.x()+ diffx(),pos.y()+diffy()));

            m_currentTextObjectView->mouseMoveEvent( e, pos);
            return;
        }
        return;
    }

    if ( editMode ) {
	view->setRulerMousePos( e->x(), e->y() );

	KPObject *kpobject;

	if ( ( !mousePressed || ( !drawRubber && modType == MT_NONE ) ) &&
	     toolEditMode == TEM_MOUSE ) {
	    bool cursorAlreadySet = false;
	    if ( (int)objectList()->count() - 1 >= 0 ) {
		for ( int i = static_cast<int>( objectList()->count() ) - 1; i >= 0; i-- ) {
		    kpobject = objectList()->at( i );
		    QSize s = kpobject->getSize();
		    QPoint pnt = kpobject->getOrig();
		    if ( QRect( pnt.x() - diffx(), pnt.y() - diffy(), s.width(), s.height() ).
			 contains( QPoint( e->x(), e->y() ) ) ) {
			if ( kpobject->isSelected() ) {
			    setCursor( kpobject->getCursor( QPoint( e->x(), e->y() ), diffx(), diffy(), modType ) );
			    cursorAlreadySet = true;
			    break;
			}
		    }
		}
	    }
	    if ( !cursorAlreadySet )
		setCursor( arrowCursor );
	    else
		return;
	} else if ( mousePressed ) {
	    int mx = e->x();
	    int my = e->y();
	    mx = ( mx / rastX() ) * rastX();
	    my = ( my / rastY() ) * rastY();

	    switch ( toolEditMode ) {
	    case TEM_MOUSE: {
		oldMx = ( oldMx / rastX() ) * rastX();
		oldMy = ( oldMy / rastY() ) * rastY();

		if ( modType == MT_NONE ) {
		    if ( drawRubber ) {
			QPainter p;
			p.begin( this );
			p.setRasterOp( NotROP );
			p.setPen( QPen( black, 0, DotLine ) );
			p.drawRect( rubber );
			rubber.setRight( e->x() );
			rubber.setBottom( e->y() );
			p.drawRect( rubber );
			p.end();
		    }
		} else if ( modType == MT_MOVE ) {
		    moveObject( mx - oldMx, my - oldMy, false );
		} else if ( modType != MT_NONE && resizeObjNum != -1 ) {
                    resizeObject( modType, mx - oldMx, my - oldMy );
		}

		oldMx = e->x();
		oldMy = e->y();
	    } break;
	    case INS_TEXT: case INS_OBJECT: case INS_TABLE:
	    case INS_DIAGRAMM: case INS_FORMULA: case INS_AUTOFORM: {
		QPainter p( this );
		p.setPen( QPen( black, 1, SolidLine ) );
		p.setBrush( NoBrush );
		p.setRasterOp( NotROP );
		if ( insRect.width() != 0 && insRect.height() != 0 )
		    p.drawRect( insRect );
		insRect.setRight( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx() );
		insRect.setBottom( ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy() );
		p.drawRect( insRect );
		p.end();

		mouseSelectedObject = true;

                view->penColorChanged( view->getPen() );
                view->brushColorChanged( view->getBrush() );
	    } break;
	    case INS_ELLIPSE: {
		QPainter p( this );
		p.setPen( QPen( black, 1, SolidLine ) );
		p.setBrush( NoBrush );
		p.setRasterOp( NotROP );
		if ( insRect.width() != 0 && insRect.height() != 0 )
		    p.drawEllipse( insRect );
		insRect.setRight( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx() );
		insRect.setBottom( ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy() );
		p.drawEllipse( insRect );
		p.end();

		mouseSelectedObject = true;

                view->penColorChanged( view->getPen() );
                view->brushColorChanged( view->getBrush() );
	    } break;
	    case INS_RECT: {
		QPainter p( this );
		p.setPen( QPen( black, 1, SolidLine ) );
		p.setBrush( NoBrush );
		p.setRasterOp( NotROP );
		if ( insRect.width() != 0 && insRect.height() != 0 )
		    p.drawRoundRect( insRect, view->getRndX(), view->getRndY() );
		insRect.setRight( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx() );
		insRect.setBottom( ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy() );
		p.drawRoundRect( insRect, view->getRndX(), view->getRndY() );
		p.end();

		mouseSelectedObject = true;

                view->penColorChanged( view->getPen() );
                view->brushColorChanged( view->getBrush() );
	    } break;
	    case INS_LINE: {
		QPainter p( this );
		p.setPen( QPen( black, 1, SolidLine ) );
		p.setBrush( NoBrush );
		p.setRasterOp( NotROP );
		if ( insRect.width() != 0 && insRect.height() != 0 )
		    p.drawLine( insRect.topLeft(), insRect.bottomRight() );
		insRect.setRight( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx() );
		insRect.setBottom( ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy() );
		p.drawLine( insRect.topLeft(), insRect.bottomRight() );
		p.end();

		mouseSelectedObject = true;

                view->penColorChanged( view->getPen() );
                view->brushColorChanged( view->getBrush() );
	    } break;
	    case INS_PIE: {
		QPainter p( this );
		p.setPen( QPen( black, 1, SolidLine ) );
		p.setBrush( NoBrush );
		p.setRasterOp( NotROP );
		if ( insRect.width() != 0 && insRect.height() != 0 ) {
		    switch ( view->getPieType() ) {
		    case PT_PIE:
			p.drawPie( insRect.x(), insRect.y(), insRect.width() - 2,
				   insRect.height() - 2, view->getPieAngle(), view->getPieLength() );
			break;
		    case PT_ARC:
			p.drawArc( insRect.x(), insRect.y(), insRect.width() - 2,
				   insRect.height() - 2, view->getPieAngle(), view->getPieLength() );
			break;
		    case PT_CHORD:
			p.drawChord( insRect.x(), insRect.y(), insRect.width() - 2,
				     insRect.height() - 2, view->getPieAngle(), view->getPieLength() );
			break;
		    default: break;
		    }
		}
		insRect.setRight( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx() );
		insRect.setBottom( ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy() );
		switch ( view->getPieType() ) {
		case PT_PIE:
		    p.drawPie( insRect.x(), insRect.y(), insRect.width() - 2,
			       insRect.height() - 2, view->getPieAngle(), view->getPieLength() );
		    break;
		case PT_ARC:
		    p.drawArc( insRect.x(), insRect.y(), insRect.width() - 2,
			       insRect.height() - 2, view->getPieAngle(), view->getPieLength() );
		    break;
		case PT_CHORD:
		    p.drawChord( insRect.x(), insRect.y(), insRect.width() - 2,
				 insRect.height() - 2, view->getPieAngle(), view->getPieLength() );
		    break;
		default: break;
		}
		p.end();

		mouseSelectedObject = true;

                view->penColorChanged( view->getPen() );
                view->brushColorChanged( view->getBrush() );
	    } break;
            case INS_FREEHAND: {
                m_dragEndPoint = QPoint( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx(),
                                         ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy() );
                QPainter p( this );
                p.setPen( QPen( black, 1, SolidLine ) );
                p.setBrush( NoBrush );
                p.setRasterOp( NotROP );
                p.drawLine( m_dragStartPoint, m_dragEndPoint );
                p.end();

                m_pointArray.putPoints( m_indexPointArray, 1, m_dragStartPoint.x(), m_dragStartPoint.y() );
                ++m_indexPointArray;
                m_dragStartPoint = m_dragEndPoint;

                mouseSelectedObject = true;

                view->penColorChanged( view->getPen() );
                view->brushColorChanged( view->getBrush() );
            } break;
            case INS_POLYLINE: {
                QPainter p( this );
                p.setPen( QPen( black, 1, SolidLine ) );
                p.setBrush( NoBrush );
                p.setRasterOp( NotROP );
                p.drawLine( m_dragStartPoint, m_dragEndPoint ); //
                m_dragEndPoint = QPoint( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx(),
                                         ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy() );
                p.drawLine( m_dragStartPoint, m_dragEndPoint );
                p.end();

                mouseSelectedObject = true;

                view->penColorChanged( view->getPen() );
                view->brushColorChanged( view->getBrush() );
            } break;
            case INS_CUBICBEZIERCURVE: case INS_QUADRICBEZIERCURVE:{
                drawCubicBezierCurve( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx(),
                                      ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy() );

                mouseSelectedObject = true;

                view->penColorChanged( view->getPen() );
                view->brushColorChanged( view->getBrush() );
            } break;
            case INS_POLYGON: {
                drawPolygon( m_dragStartPoint, m_dragEndPoint ); // erase old polygon

                m_dragEndPoint = QPoint( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx(),
                                         ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy() );

                drawPolygon( m_dragStartPoint, m_dragEndPoint ); // draw new polygon

                mouseSelectedObject = true;

                view->penColorChanged( view->getPen() );
                view->brushColorChanged( view->getBrush() );
            } break;
            default: break;
	    }
	}
    } else if ( !editMode && drawMode && drawLineInDrawMode ) {
	QPainter p;
	p.begin( this );
	p.setPen( view->kPresenterDoc()->presPen() );
	p.drawLine( oldMx, oldMy, e->x(), e->y() );
	oldMx = e->x();
	oldMy = e->y();
	p.end();
    }
    if ( !editMode && !drawMode && !presMenu->isVisible() && fillBlack )
	setCursor( blankCursor );
}

/*==================== mouse double click ========================*/
void Page::mouseDoubleClickEvent( QMouseEvent *e )
{
    if(!view->koDocument()->isReadWrite())
        return;

    if(m_currentTextObjectView)
    {
        KPTextObject *txtObj=m_currentTextObjectView->kpTextObject();
        Q_ASSERT(txtObj);
        if(txtObj->contains( e->pos(), diffx(), diffy() ))
        {
            QPoint pos=e->pos() - txtObj->getOrig();
            pos=view->zoomHandler()->pixelToLayoutUnit(QPoint(pos.x()+ diffx(),pos.y()+diffy()));

            m_currentTextObjectView->mouseDoubleClickEvent( e, pos);
            return;
        }
    }

    //disallow activating objects outside the "page"
    if ( !view->kPresenterDoc()->getPageRect( view->getCurrPgNum()-1, diffx(), diffy(), _presFakt ).contains(e->pos()))
        return;

    if ( toolEditMode != TEM_MOUSE || !editMode ) return;

    deSelectAllObj();
    KPObject *kpobject = 0;

    if ( (int)objectList()->count() - 1 >= 0 ) {
	for ( int i = static_cast<int>( objectList()->count() ) - 1; i >= 0; i-- ) {
	    kpobject = objectList()->at( i );
	    if ( kpobject->contains( QPoint( e->x(), e->y() ), diffx(), diffy() ) ) {
		if ( kpobject->getType() == OT_TEXT ) {
		    KPTextObject *kptextobject = dynamic_cast<KPTextObject*>( kpobject );
                    if(m_currentTextObjectView)
                    {
                        m_currentTextObjectView->terminate();
                        delete m_currentTextObjectView;
                    }
                    m_currentTextObjectView=kptextobject->createKPTextView(this);

		    setTextBackground( kptextobject );
                    setCursor( arrowCursor );
		    editNum = i;
		    break;
		} else if ( kpobject->getType() == OT_PART ) {
		    kpobject->activate( view, diffx(), diffy() );
		    editNum = i;
		    break;
		}
	    }
	}
    }
}

/*====================== mouse wheel event =========================*/
void Page::wheelEvent( QWheelEvent *e )
{
    if ( !editMode && !drawMode ) {
        if ( e->delta() == -120 )     // wheel down
            view->screenNext();
        else if ( e->delta() == 120 ) // wheel up
            view->screenPrev();
        e->accept();
    }
    else if ( editMode )
        emit sigMouseWheelEvent( e );
}


/*====================== key press event =========================*/
void Page::keyPressEvent( QKeyEvent *e )
{
    if ( !editMode ) {
	switch ( e->key() ) {
	case Key_Space: case Key_Right: case Key_Down: case Key_Next:
	    view->screenNext(); break;
	case Key_Backspace: case Key_Left: case Key_Up: case Key_Prior:
	    view->screenPrev(); break;
	case Key_Escape: case Key_Q: case Key_X:
	    view->screenStop(); break;
	case Key_G:
            if ( !spManualSwitch() )
                view->autoScreenPresStopTimer();
	    slotGotoPage(); break;
        case Key_Home:  // go to first page
            gotoPage( 1 );
            if ( !spManualSwitch() ) {
                view->setCurrentTimer( 1 );
                setNextPageTimer( true );
            }
            break;
        case Key_End:  // go to last page
            gotoPage( slideList.count() );
            if ( !spManualSwitch() ) {
                view->setCurrentTimer( 1 );
                setNextPageTimer( true );
            }
            break;
	default: break;
	}
    } else if ( editNum != -1 ) {
	if ( e->key() == Key_Escape ) {
            exitEditMode();
	}
        else if ( m_currentTextObjectView )
        {
            m_currentTextObjectView->keyPressEvent( e );
        }
    } else if ( mouseSelectedObject ) {
	if ( e->state() & ControlButton ) {
	    switch ( e->key() ) {
	    case Key_Up:
		moveObject( 0, -10, true );
		break;
	    case Key_Down:
		moveObject( 0, 10, true );
		break;
	    case Key_Right:
		moveObject( 10, 0, true );
		break;
	    case Key_Left:
		moveObject( -10, 0, true );
		break;
	    default: break;
	    }
	} else {
	    switch ( e->key() ) {
	    case Key_Up:
		moveObject( 0, -1, true );
		break;
	    case Key_Down:
		moveObject( 0, 1, true );
		break;
	    case Key_Right:
		moveObject( 1, 0, true );
		break;
	    case Key_Left:
		moveObject( -1, 0, true );
		break;
	    case Key_Delete: case Key_Backspace:
		view->editDelete();
		break;
	    case Key_Escape:
		setToolEditMode( TEM_MOUSE );
		break;
	    default: break;
	    }
	}
    } else {
	switch ( e->key() ) {
	case Key_Next:
	    view->nextPage();
	    break;
	case Key_Prior:
	    view->prevPage();
	    break;
	case Key_Down:
	    view->getVScrollBar()->addLine();
	    break;
	case Key_Up:
	    view->getVScrollBar()->subtractLine();
	    break;
	case Key_Right:
	    view->getHScrollBar()->addLine();
	    break;
	case Key_Left:
	    view->getHScrollBar()->subtractLine();
	    break;
	case Key_Tab:
	    selectNext();
	    break;
	case Key_Backtab:
	    selectPrev();
	    break;
	case Key_Home:
	    view->getVScrollBar()->setValue( 0 );
	    break;
	case Key_End:
	    view->getVScrollBar()->setValue( view->getVScrollBar()->maxValue());
	    break;
	default: break;
	}
    }
}

void Page::keyReleaseEvent( QKeyEvent *e )
{
    if ( editMode && m_currentTextObjectView )
    {
        m_currentTextObjectView->keyReleaseEvent( e );
    }
    else
    {
        if ( mouseSelectedObject )
        {
            if(e->key()==Key_Up || e->key()==Key_Down || e->key()==Key_Right || e->key()==Key_Left)
                emit objectSelectedChanged();
        }
    }
}

/*========================= resize Event =========================*/
void Page::resizeEvent( QResizeEvent *e )
{
    if ( editMode )
        QWidget::resizeEvent( e );
    else
        QWidget::resizeEvent( new QResizeEvent( QApplication::desktop()->size(),
                                                e->oldSize() ) );
    if ( editMode )
        buffer.resize( size() );
}

/*========================== get object ==========================*/
int Page::getObjectAt( int x, int y )
{
    for ( int i = static_cast<int>( objectList()->count() ) - 1; i >= 0 ; i-- ) {
        KPObject * kpobject = objectList()->at( i );
        if ( kpobject->contains( QPoint( x, y ), diffx(), diffy() ) )
            return i;
    }

    return -1;
}

/*================================================================*/
int Page::diffx() const
{
    return view->getDiffX();
}

/*================================================================*/
int Page::diffy() const
{
    return view->getDiffY();
}
/*======================= select object ==========================*/
void Page::selectObj( int num )
{
    if ( num < static_cast<int>( objectList()->count() ) ) {
        selectObj( objectList()->at( num ) );
        emit objectSelectedChanged();
    }
}

/*======================= deselect object ========================*/
void Page::deSelectObj( int num )
{
    if ( num < static_cast<int>( objectList()->count() ) )
        deSelectObj( objectList()->at( num ) );
}

/*======================= select object ==========================*/
void Page::selectObj( KPObject *kpobject )
{
    kpobject->setSelected( true );
    view->penColorChanged( view->kPresenterDoc()->getPen( QPen( Qt::black, 1, Qt::SolidLine ) ) );
    view->brushColorChanged( view->kPresenterDoc()->getBrush( QBrush( Qt::white, Qt::SolidPattern ) ) );

    _repaint( kpobject );
    emit objectSelectedChanged();

    mouseSelectedObject = true;
}

/*======================= deselect object ========================*/
void Page::deSelectObj( KPObject *kpobject )
{
    kpobject->setSelected( false );
    _repaint( kpobject );

    mouseSelectedObject = false;
    emit objectSelectedChanged();
}

/*====================== select all objects ======================*/
void Page::selectAllObj()
{
    if(view->kPresenterDoc()->numSelected()==(int)objectList()->count())
        return;

    QProgressDialog progress( i18n( "Selecting..." ), 0,
                              objectList()->count(), this );

    for ( uint i = 0; i <= objectList()->count(); i++ ) {
        selectObj( i );

        progress.setProgress( i );
        kapp->processEvents();
    }

    mouseSelectedObject = true;
    emit objectSelectedChanged();
}


/*==================== deselect all objects ======================*/
void Page::deSelectAllObj()
{
    if(view->kPresenterDoc()->numSelected()==0)
        return;

    if ( !view->kPresenterDoc()->raiseAndLowerObject && selectedObjectPosition != -1 ) {
        lowerObject();
        selectedObjectPosition = -1;
    }
    else
        view->kPresenterDoc()->raiseAndLowerObject = false;

    KPObject *kpobject;

    for ( int i = 0; i < static_cast<int>( objectList()->count() ); i++ )
    {
        kpobject = objectList()->at( i );
        if ( kpobject->isSelected() ) deSelectObj( kpobject );
    }

    //desactivate kptextview when we switch of page
    if(m_currentTextObjectView)
    {
        m_currentTextObjectView->terminate();
        delete m_currentTextObjectView;
        m_currentTextObjectView=0L;
    }
    mouseSelectedObject = false;
    emit objectSelectedChanged();
}


void Page::setMouseSelectedObject(bool b)
{
    mouseSelectedObject = b;
    emit objectSelectedChanged();
}

/*======================== setup menus ===========================*/
void Page::setupMenus()
{
    // create right button presentation menu
    presMenu = new QPopupMenu();
    Q_CHECK_PTR( presMenu );
    presMenu->setCheckable( true );
    PM_SM = presMenu->insertItem( i18n( "&Switching mode" ), this, SLOT( switchingMode() ) );
    PM_DM = presMenu->insertItem( i18n( "&Drawing mode" ), this, SLOT( drawingMode() ) );
    presMenu->insertSeparator();
    presMenu->insertItem( SmallIcon("goto"), i18n( "&Goto Page..." ), this, SLOT( slotGotoPage() ) );
    presMenu->insertSeparator();
    presMenu->insertItem( i18n( "&Exit Presentation" ), this, SLOT( slotExitPres() ) );
    presMenu->setItemChecked( PM_SM, true );
    presMenu->setItemChecked( PM_DM, false );
    presMenu->setMouseTracking( true );
}

/*======================== clipboard cut =========================*/
void Page::clipCut()
{
    if ( m_currentTextObjectView )
        m_currentTextObjectView->cut();
    view->editCut();
}

/*======================== clipboard copy ========================*/
void Page::clipCopy()
{
    if ( m_currentTextObjectView )
        m_currentTextObjectView->copy();
    view->editCopy();
}

/*====================== clipboard paste =========================*/
void Page::clipPaste()
{
    if ( m_currentTextObjectView )
        m_currentTextObjectView->paste();
    view->editPaste();
}

/*======================= change picture  ========================*/
void Page::chPic()
{
    KPObject *kpobject = 0;

    for ( unsigned int i = 0; i < objectList()->count(); i++ )
    {
        kpobject = objectList()->at( i );
        if ( kpobject->isSelected() && kpobject->getType() == OT_PICTURE )
        {
            view->changePicture( dynamic_cast<KPPixmapObject*>( kpobject )->getFileName() );
            break;
        }
    }
}

/*======================= change clipart  ========================*/
void Page::chClip()
{
    KPObject *kpobject = 0;

    for ( unsigned int i = 0; i < objectList()->count(); i++ )
    {
        kpobject = objectList()->at( i );
        if ( kpobject->isSelected() && kpobject->getType() == OT_CLIPART )
        {
            view->changeClipart( dynamic_cast<KPClipartObject*>( kpobject )->getFileName() );
            break;
        }
    }
}

void Page::setFont(const QFont &font, bool _subscript, bool _superscript, const QColor &col, const QColor &backGroundColor, int flags)

{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    QPtrListIterator<KoTextFormatInterface> it( lst );
    for ( ; it.current() ; ++it )
        it.current()->setFont( font, _subscript, _superscript, col, backGroundColor, flags );
}

void Page::setTextColor( const QColor &color )
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    QPtrListIterator<KoTextFormatInterface> it( lst );
    for ( ; it.current() ; ++it )
        it.current()->setTextColor( color );
}

void Page::setTextBackgroundColor( const QColor &color )
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    QPtrListIterator<KoTextFormatInterface> it( lst );
    for ( ; it.current() ; ++it )
        it.current()->setTextBackgroundColor( color );
}

void Page::setTextBold( bool b )
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    QPtrListIterator<KoTextFormatInterface> it( lst );
    for ( ; it.current() ; ++it )
        it.current()->setBold( b );
}

void Page::setTextItalic( bool b )
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    QPtrListIterator<KoTextFormatInterface> it( lst );
    for ( ; it.current() ; ++it )
        it.current()->setItalic( b );
}

void Page::setTextUnderline( bool b )
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    QPtrListIterator<KoTextFormatInterface> it( lst );
    for ( ; it.current() ; ++it )
        it.current()->setUnderline( b );
}

void Page::setTextStrikeOut( bool b )
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    QPtrListIterator<KoTextFormatInterface> it( lst );
    for ( ; it.current() ; ++it )
        it.current()->setStrikeOut( b );
}

void Page::setTextFamily( const QString &f )
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    QPtrListIterator<KoTextFormatInterface> it( lst );
    for ( ; it.current() ; ++it )
        it.current()->setFamily( f );
}

void Page::setTextPointSize( int s )
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    QPtrListIterator<KoTextFormatInterface> it( lst );
    for ( ; it.current() ; ++it )
        it.current()->setPointSize( s );
}

void Page::setTextSubScript( bool b )
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    QPtrListIterator<KoTextFormatInterface> it( lst );
    for ( ; it.current() ; ++it )
        it.current()->setTextSubScript( b );
}

void Page::setTextSuperScript( bool b )
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    QPtrListIterator<KoTextFormatInterface> it( lst );
    for ( ; it.current() ; ++it )
        it.current()->setTextSuperScript( b );
}


void Page::setTextDefaultFormat( )
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    QPtrListIterator<KoTextFormatInterface> it( lst );
    for ( ; it.current() ; ++it )
        it.current()->setDefaultFormat( );
}

void Page::setIncreaseFontSize()
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    QPtrListIterator<KoTextFormatInterface> it( lst );
    int size=12;
    if(!lst.isEmpty())
        size=static_cast<int>( KoTextZoomHandler::layoutUnitPtToPt(lst.first()->currentFormat()->font().pointSize()));
    for ( ; it.current() ; ++it )
        it.current()->setPointSize( size+1 );
}

void Page::setDecreaseFontSize()
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    QPtrListIterator<KoTextFormatInterface> it( lst );
        int size=12;
    if(!lst.isEmpty())
        size=static_cast<int>( KoTextZoomHandler::layoutUnitPtToPt(lst.first()->currentFormat()->font().pointSize()));
    for ( ; it.current() ; ++it )
        it.current()->setPointSize( size-1 );
}

/*===================== set text alignment =======================*/
void Page::setTextAlign( int align )
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    QPtrListIterator<KoTextFormatInterface> it( lst );
    for ( ; it.current() ; ++it )
        it.current()->setAlign(align);
}

void Page::setTabList( const KoTabulatorList & tabList )
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    QPtrListIterator<KoTextFormatInterface> it( lst );
    for ( ; it.current() ; ++it )
        it.current()->setTabList(tabList );
}

void Page::setTextDepthPlus()
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    double leftMargin=0.0;
    if(!lst.isEmpty())
        leftMargin=lst.first()->currentParagLayoutFormat()->margins[QStyleSheetItem::MarginLeft];
    double indent = view->kPresenterDoc()->getIndentValue();
    double newVal = leftMargin + indent;
    QPtrListIterator<KoTextFormatInterface> it( lst );
    for ( ; it.current() ; ++it )
        it.current()->setMargin(QStyleSheetItem::MarginLeft, newVal);
    if(!lst.isEmpty())
    {
        const KoParagLayout *layout=lst.first()->currentParagLayoutFormat();
        view->showRulerIndent( layout->margins[QStyleSheetItem::MarginLeft], layout->margins[QStyleSheetItem::MarginFirstLine], layout->margins[QStyleSheetItem::MarginRight]);
    }
}

void Page::setTextDepthMinus()
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    double leftMargin=0.0;
    if(!lst.isEmpty())
        leftMargin=lst.first()->currentParagLayoutFormat()->margins[QStyleSheetItem::MarginLeft];
    double indent = view->kPresenterDoc()->getIndentValue();
    QPtrListIterator<KoTextFormatInterface> it( lst );
    double newVal = leftMargin - indent;
    for ( ; it.current() ; ++it )
        it.current()->setMargin(QStyleSheetItem::MarginLeft, QMAX( newVal, 0 ));
    if(!lst.isEmpty())
    {
        const KoParagLayout *layout=lst.first()->currentParagLayoutFormat();
        view->showRulerIndent( layout->margins[QStyleSheetItem::MarginLeft], layout->margins[QStyleSheetItem::MarginFirstLine], layout->margins[QStyleSheetItem::MarginRight]);
    }
}

void Page::setNewFirstIndent(double _firstIndent)
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    double index=0.0;
    if(!lst.isEmpty())
        index=lst.first()->currentParagLayoutFormat()->margins[QStyleSheetItem::MarginLeft];
    QPtrListIterator<KoTextFormatInterface> it( lst );
    double val = _firstIndent - index;
    for ( ; it.current() ; ++it )
        it.current()->setMargin(QStyleSheetItem::MarginFirstLine, val);
}

void Page::setNewLeftIndent(double _leftIndent)
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    QPtrListIterator<KoTextFormatInterface> it( lst );
    for ( ; it.current() ; ++it )
        it.current()->setMargin(QStyleSheetItem::MarginLeft, _leftIndent);
}

void Page::setNewRightIndent(double _rightIndent)
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    QPtrListIterator<KoTextFormatInterface> it( lst );
    for ( ; it.current() ; ++it )
        it.current()->setMargin(QStyleSheetItem::MarginRight, _rightIndent);
}

void Page::setTextCounter(KoParagCounter counter)
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    QPtrListIterator<KoTextFormatInterface> it( lst );
    for ( ; it.current() ; ++it )
        it.current()->setCounter(counter );
}

#ifndef NDEBUG
void Page::printRTDebug( int info )
{
    KPTextObject *kpTxtObj = 0;
    if ( m_currentTextObjectView )
        kpTxtObj = m_currentTextObjectView->kpTextObject();
    else
        kpTxtObj = selectedTextObjs().first();
    if ( kpTxtObj )
        kpTxtObj->textObject()->printRTDebug( info );
}
#endif

/*================================================================*/
bool Page::haveASelectedPictureObj()
{
    KPObject *kpobject = 0;
    for ( unsigned int i = 0; i < objectList()->count(); i++ )
    {
        kpobject = objectList()->at( i );
        if ( kpobject->isSelected() && kpobject->getType() != OT_CLIPART)
        {
            return false;
        }
    }
    return true;
}

QPtrList<KPTextObject> Page::applicableTextObjects() const
{
    QPtrList<KPTextObject> lst;
    // If we're editing a text object, then that's the one we return
    if ( m_currentTextObjectView )
        lst.append( m_currentTextObjectView->kpTextObject() );
    else
        lst = selectedTextObjs();
    return lst;
}

QPtrList<KoTextFormatInterface> Page::applicableTextInterfaces() const
{
    QPtrList<KoTextFormatInterface> lst;
    // If we're editing a text object, then that's the one we return
    if ( m_currentTextObjectView )
        lst.append( m_currentTextObjectView );
    else
    {
        // Otherwise we look for the text objects that are selected
        QPtrListIterator<KPObject> it(*objectList());
        for ( ; it.current(); ++it ) {
            if ( it.current()->isSelected() && it.current()->getType() == OT_TEXT )
                lst.append( static_cast<KPTextObject*>( it.current() )->textObject() );
        }
    }
    return lst;

}

QPtrList<KPTextObject> Page::selectedTextObjs() const
{
    QPtrList<KPTextObject> lst;
    QPtrListIterator<KPObject> it(*objectList());
    for ( ; it.current(); ++it ) {
        if ( it.current()->isSelected() && it.current()->getType() == OT_TEXT )
            lst.append( static_cast<KPTextObject*>( it.current() ) );
    }
    return lst;
}

/*====================== start screenpresentation ================*/
void Page::startScreenPresentation( float presFakt, int curPgNum /* 1-based */)
{
    _presFakt = presFakt;
    m_showOnlyPage = curPgNum;
    kdDebug(33001) << "Page::startScreenPresentation m_showOnlyPage=" << m_showOnlyPage << endl;
    kdDebug(33001) << "                              _presFakt=" << _presFakt << endl;

    presMenu->setItemChecked( PM_SM, true );
    presMenu->setItemChecked( PM_DM, false );

    setCursor( waitCursor );
    tmpObjs.clear();

    exitEditMode();

    //kdDebug(33001) << "Page::startScreenPresentation Zooming backgrounds" << endl;
    // Zoom backgrounds to the correct size for full screen
    if ( m_showOnlyPage == -1 )
    {
        // Maybe we should do this on demand ?
        QPtrListIterator<KPBackGround> it(*backgroundList());
        for ( int i = 0 ; it.current(); ++it, ++i )
	    it.current()->setBgSize( getPageRect( i, _presFakt ).size() );
    }
    else
        backgroundList()->at( m_showOnlyPage-1 )->setBgSize( getPageRect( m_showOnlyPage-1, _presFakt ).size() );

    //kdDebug(33001) << "Page::startScreenPresentation Zooming objects" << endl;
    // Zoom objects to the correct size for full screen
    // (might need a progress bar!)
    QPtrListIterator<KPObject> it(*objectList());
    for ( int i = 0 ; it.current(); ++it, ++i )
    {
        int objPage = getPageOfObj( i, _presFakt ); // 1-based
	// We need to zoom ALL objects, even to show only one page.
	// Otherwise, the non-zoomed ones would interfer, being at the wrong page
        //if ( m_showOnlyPage == -1 || m_showOnlyPage == objPage )
        //{
            //kdDebug(33001) << "Zooming object " << i << endl;
            it.current()->zoom( _presFakt );
            //kdDebug(33001) << "Zooming object " << i << " - done" << endl;
            it.current()->drawSelection( false );

            // Objects to draw initially are those on first page (or m_showOnlyPage page, if set)
            if ( (m_showOnlyPage == -1 && objPage == 1) || m_showOnlyPage == objPage )
            {
                //kdDebug(33001) << "Adding object " << i << " in page " << objPage << endl;
                tmpObjs.append( it.current() );
            }
        //}
    }

    KPresenterDoc * doc = view->kPresenterDoc();
    if ( doc->hasHeader() && doc->header() )
	doc->header()->zoom( _presFakt );
    if ( doc->hasFooter() && doc->footer() )
	doc->footer()->zoom( _presFakt );

    if ( m_showOnlyPage != -1 ) // only one page
    {
        slideList.clear();
        slideList << m_showOnlyPage;
    }
    else // all selected pages
    {
        slideList = doc->selectedSlides();
        // ARGLLLRGLRLGRLG selectedSlides gets us 0-based numbers,
        // and here we want 1-based numbers !
        QString debugstr;
        for ( QValueList<int>::Iterator it = slideList.begin() ; it != slideList.end(); ++ it )
        {
            *it = (*it)+1;
            debugstr += QString::number(*it) + ',';
        }
        //kdDebug(33001) << "selectedSlides : " << debugstr << endl;
    }
    Q_ASSERT( slideList.count() );
    slideListIterator = slideList.begin();

    setCursor( blankCursor );

    currPresPage = (unsigned int) -1; // force gotoPage to do something
    gotoPage( *slideListIterator );

    doc->getHeaderFooterEdit()->updateSizes();
    //kdDebug(33001) << "Page::startScreenPresentation - done" << endl;
}

/*====================== stop screenpresentation =================*/
void Page::stopScreenPresentation()
{
    //kdDebug(33001) << "Page::stopScreenPresentation m_showOnlyPage=" << m_showOnlyPage << endl;
    setCursor( waitCursor );

    QPtrListIterator<KPObject> it(*objectList());
    for ( int i = 0 ; it.current(); ++it, ++i )
    {
        if ( it.current()->isZoomed() )
        {
            //kdDebug(33001) << "Page::stopScreenPresentation zooming back object " << i << endl;
            it.current()->zoomOrig();
            it.current()->drawSelection( true );
        }
    }

    _presFakt = 1.0;

    // Zoom backgrounds back
    if ( m_showOnlyPage == -1 )
    {
        QPtrListIterator<KPBackGround> it(*backgroundList());
        for ( int i = 0 ; it.current(); ++it, ++i )
	    it.current()->setBgSize( getPageRect( i ).size() );
    }
    else
        backgroundList()->at( m_showOnlyPage-1 )->setBgSize( getPageRect( m_showOnlyPage-1 ).size() );

    KPresenterDoc * doc = view->kPresenterDoc();
    if ( doc->hasHeader() && doc->header() )
        doc->header()->zoomOrig();
    if ( doc->hasFooter() && doc->footer() )
        doc->footer()->zoomOrig();

    goingBack = false;
    currPresPage = 1;
    editMode = true;
    drawMode = false;
    repaint( false );
    setToolEditMode( toolEditMode );
    tmpObjs.clear();
    setWFlags( WResizeNoErase );
}

/*========================== next ================================*/
bool Page::pNext( bool )
{
    //bool clearSubPres = false;

    goingBack = false;

    kdDebug(33001) << "\n-------\nPage::pNext currPresStep=" << currPresStep << " subPresStep=" << subPresStep << endl;

    // First try to go one sub-step further, if any object requires it
    QPtrListIterator<KPObject> oit(*objectList());
    for ( int i = 0 ; oit.current(); ++oit, ++i )
    {
        KPObject *kpobject = oit.current();
        if ( getPageOfObj( i, _presFakt ) == static_cast<int>( currPresPage )
             && kpobject->getPresNum() == static_cast<int>( currPresStep )
             && kpobject->getType() == OT_TEXT && kpobject->getEffect2() != EF2_NONE )
        {
            if ( static_cast<int>( subPresStep + 1 ) < kpobject->getSubPresSteps() )
            {
                kdDebug(33001) << "Page::pNext addSubPres subPresStep is now " << subPresStep+1 << endl;
                subPresStep++;
                doObjEffects();
                return false;
            }
        }
    }

    // Then try to see if there is still one step to do in the current page
    if ( (int)currPresStep < *( --presStepList.end() ) )
    {
        QValueList<int>::ConstIterator it = presStepList.find( currPresStep );
        currPresStep = *( ++it );
        subPresStep = 0;
        //kdDebug(33001) << "Page::pNext setting currPresStep to " << currPresStep << endl;

        if ( currPresStep == 0 )
        {
            QPainter p;
            p.begin( this );
            drawBackground( &p, QRect( 0, 0, kapp->desktop()->width(), kapp->desktop()->height() ) );
            p.end();
        }

        doObjEffects();
        return false;
    }

    // No more steps in this page, try to go to the next page
    QValueList<int>::ConstIterator test(  slideListIterator );
    if ( ++test != slideList.end() )
    {
        if ( !spManualSwitch() && nextPageTimer ) {
            QValueList<int>::ConstIterator it( slideListIterator );
            view->setCurrentTimer( backgroundList()->at( (*it) - 1 )->getPageTimer() );
            nextPageTimer = false;

            return false;
        }

        QPixmap _pix1( QApplication::desktop()->width(), QApplication::desktop()->height() );
        drawPageInPix( _pix1, diffy() );

        currPresPage = *( ++slideListIterator );
        subPresStep = 0;
        kdDebug(33001) << "Page::pNext going to page " << currPresPage << endl;

        tmpObjs.clear();
        for ( int j = 0; j < static_cast<int>( objectList()->count() ); j++ )
        {
            if ( getPageOfObj( j, _presFakt ) == static_cast<int>( currPresPage ) )
                tmpObjs.append( objectList()->at( j ) );
        }

        presStepList = view->kPresenterDoc()->reorderPage( currPresPage, diffx(), diffy(), _presFakt );
        currPresStep = *presStepList.begin();

        QPixmap _pix2( QApplication::desktop()->width(), QApplication::desktop()->height() );
        int pageHeight = view->kPresenterDoc()->getPageRect( 0, 0, 0, presFakt(), false ).height();
        int yOffset = ( presPage() - 1 ) * pageHeight;
        if ( height() > pageHeight )
            yOffset -= ( height() - pageHeight ) / 2;
        drawPageInPix( _pix2, yOffset );

        QValueList<int>::ConstIterator it( slideListIterator );
        --it;

        if ( !spManualSwitch() )
            view->autoScreenPresStopTimer();

        PageEffect _pageEffect = backgroundList()->at( ( *it ) - 1 )->getPageEffect();
        bool _soundEffect = backgroundList()->at( ( *it ) - 1 )->getPageSoundEffect();
        QString _soundFileName = backgroundList()->at( ( *it ) - 1 )->getPageSoundFileName();
        if ( _pageEffect != PEF_NONE && _soundEffect && !_soundFileName.isEmpty() ) {
            stopSound();
            playSound( _soundFileName );
        }

        changePages( _pix1, _pix2, _pageEffect );

        if ( !spManualSwitch() )
            view->autoScreenPresReStartTimer();

        return true;
    }

    // No more slides. Redisplay last one, then
    kdDebug(33001) << "Page::pNext last slide -> again" << endl;
    emit stopPres();
    presStepList = view->kPresenterDoc()->reorderPage( currPresPage, diffx(), diffy(), _presFakt );
    currPresStep = *presStepList.begin();
    doObjEffects();
    return false;
}

/*====================== previous ================================*/
bool Page::pPrev( bool /*manual*/ )
{
    goingBack = true;
    subPresStep = 0;

    if ( (int)currPresStep > *presStepList.begin() ) {
        QValueList<int>::ConstIterator it = presStepList.find( currPresStep );
        currPresStep = *( --it );
        repaint( false );
        return false;
    } else {
        if ( slideListIterator == slideList.begin() ) {
            presStepList = view->kPresenterDoc()->reorderPage( currPresPage, diffx(), diffy(), _presFakt );
            currPresStep = *presStepList.begin();
            repaint( false );
            return false;
        }
        currPresPage = *( --slideListIterator );

        tmpObjs.clear();
        for ( int j = 0; j < static_cast<int>( objectList()->count() ); j++ ) {
            if ( getPageOfObj( j, _presFakt ) == static_cast<int>( currPresPage ) )
                tmpObjs.append( objectList()->at( j ) );
        }

        presStepList = view->kPresenterDoc()->reorderPage( currPresPage, diffx(), diffy(), _presFakt );
        currPresStep = *( --presStepList.end() );
        return true;
    }

    return false;
}

/*================================================================*/
bool Page::canAssignEffect( QPtrList<KPObject> &objs ) const
{
    QPtrListIterator<KPObject> oIt( *objectList() );
    for (; oIt.current(); ++oIt )
        if ( oIt.current()->isSelected() )
            objs.append( oIt.current() );

    return !objs.isEmpty();
}

/*================================================================*/
bool Page::isOneObjectSelected()
{
    QPtrListIterator<KPObject> oIt( *objectList() );
    for (; oIt.current(); ++oIt )
        if ( oIt.current()->isSelected() )
            return true;

    return false;
}

/*================================================================*/
void Page::drawPageInPix2( QPixmap &_pix, int __diffy, int pgnum, float /*_zoom*/ )
{
    //kdDebug(33001) << "Page::drawPageInPix2" << endl;
    currPresPage = pgnum + 1;
    int _yOffset = diffy();
    view->setDiffY( __diffy );

    QPainter p;
    p.begin( &_pix );

    QPtrListIterator<KPObject> oIt( *objectList() );
    for (; oIt.current(); ++oIt )
        oIt.current()->drawSelection( false );

    bool _editMode = editMode;
    editMode = false;
    drawBackground( &p, _pix.rect(), true );
    editMode = _editMode;

    drawObjects( &p, _pix.rect(), false, true );

    p.end();

    view->setDiffY( _yOffset );

    oIt.toFirst();
    for (; oIt.current(); ++oIt )
        oIt.current()->drawSelection( true );
}

/*==================== draw a page in a pixmap ===================*/
void Page::drawPageInPix( QPixmap &_pix, int __diffy )
{
    //kdDebug(33001) << "Page::drawPageInPix" << endl;
    int _yOffset = diffy();
    view->setDiffY( __diffy );

    QPainter p;
    p.begin( &_pix );

    drawBackground( &p, _pix.rect(), true );
    drawObjects( &p, _pix.rect(), false, true );

    p.end();

    view->setDiffY( _yOffset );
}

/*==================== print a page ===================*/
void Page::printPage( QPainter* painter, int __diffy, QRect _rect )
{
    //kdDebug(33001) << "Page::drawPageInPainter" << endl;
    int _yOffset = diffy();
    view->setDiffY( __diffy );

    drawBackground( painter, _rect, true );
    drawObjects( painter, _rect, false, true );

    view->setDiffY( _yOffset );
}

/*=========================== change pages =======================*/
void Page::changePages( QPixmap _pix1, QPixmap _pix2, PageEffect _effect )
{
    QTime _time;
    int _step = 0, _steps, _h, _w, _x, _y;

    switch ( _effect )
    {
    case PEF_NONE:
    {
        bitBlt( this, 0, 0, &_pix2, 0, 0, _pix2.width(), _pix2.height() );
    } break;
    case PEF_CLOSE_HORZ:
    {
        _steps = static_cast<int>( static_cast<float>( kapp->desktop()->height() ) / pageSpeedFakt() );
        _time.start();

        for ( ; ; )
        {
            kapp->processEvents();
            if ( _time.elapsed() >= 1 )
            {
                _step++;
                _h = ( _pix2.height()/( 2 * _steps ) ) * _step;
                _h = _h > _pix2.height() / 2 ? _pix2.height() / 2 : _h;

                bitBlt( this, 0, 0, &_pix2, 0, _pix2.height() / 2 - _h, width(), _h );
                bitBlt( this, 0, height() - _h, &_pix2, 0, _pix2.height() / 2, width(), _h );

                _time.restart();
            }
            if ( ( _pix2.height()/( 2 * _steps ) ) * _step >= _pix2.height() / 2 ) break;
        }
    } break;
    case PEF_CLOSE_VERT:
    {
        _steps = static_cast<int>( static_cast<float>( kapp->desktop()->width() ) / pageSpeedFakt() );
        _time.start();

        for ( ; ; )
        {
            kapp->processEvents();
            if ( _time.elapsed() >= 1 )
            {
                _step++;
                _w = ( _pix2.width()/( 2 * _steps ) ) * _step;
                _w = _w > _pix2.width() / 2 ? _pix2.width() / 2 : _w;

                bitBlt( this, 0, 0, &_pix2, _pix2.width() / 2 - _w, 0, _w, height() );
                bitBlt( this, width() - _w, 0, &_pix2, _pix2.width() / 2, 0, _w, height() );

                _time.restart();
            }
            if ( ( _pix2.width()/( 2 * _steps ) ) * _step >= _pix2.width() / 2 ) break;
        }
    } break;
    case PEF_CLOSE_ALL:
    {
        _steps = static_cast<int>( static_cast<float>( kapp->desktop()->width() ) / pageSpeedFakt() );
        _time.start();

        for ( ; ; )
        {
            kapp->processEvents();
            if ( _time.elapsed() >= 1 )
            {
                _step++;
                _w = ( _pix2.width()/( 2 * _steps ) ) * _step;
                _w = _w > _pix2.width() / 2 ? _pix2.width() / 2 : _w;

                _h = ( _pix2.height()/( 2 * _steps ) ) * _step;
                _h = _h > _pix2.height() / 2 ? _pix2.height() / 2 : _h;

                bitBlt( this, 0, 0, &_pix2, 0, 0, _w, _h );
                bitBlt( this, width() - _w, 0, &_pix2, width() - _w, 0, _w, _h );
                bitBlt( this, 0, height() - _h, &_pix2, 0, height() - _h, _w, _h );
                bitBlt( this, width() - _w, height() - _h, &_pix2, width() - _w, height() - _h, _w, _h );

                _time.restart();
            }
            if ( ( _pix2.width()/( 2 * _steps ) ) * _step >= _pix2.width() / 2
                 && ( _pix2.height()/( 2 * _steps ) ) * _step >= _pix2.height() / 2 ) break;
        }
    } break;
    case PEF_OPEN_HORZ:
    {
        _steps = static_cast<int>( static_cast<float>( kapp->desktop()->height() ) / pageSpeedFakt() );
        _time.start();

        for ( ; ; )
        {
            kapp->processEvents();
            if ( _time.elapsed() >= 1 )
            {
                _step++;
                _h = ( _pix2.height() / _steps ) * _step;
                _h = _h > _pix2.height() ? _pix2.height() : _h;

                _y = _pix2.height() / 2;

                bitBlt( this, 0, _y - _h / 2, &_pix2, 0, _y - _h / 2, width(), _h );

                _time.restart();
            }
            if ( ( _pix2.height() / _steps ) * _step >= _pix2.height() ) break;
        }
    } break;
    case PEF_OPEN_VERT:
    {
        _steps = static_cast<int>( static_cast<float>( kapp->desktop()->width() ) / pageSpeedFakt() );
        _time.start();

        for ( ; ; )
        {
            kapp->processEvents();
            if ( _time.elapsed() >= 1 )
            {
                _step++;
                _w = ( _pix2.width() / _steps ) * _step;
                _w = _w > _pix2.width() ? _pix2.width() : _w;

                _x = _pix2.width() / 2;

                bitBlt( this, _x - _w / 2, 0, &_pix2, _x - _w / 2, 0, _w, height() );

                _time.restart();
            }
            if ( ( _pix2.width() / _steps ) * _step >= _pix2.width() ) break;
        }
    } break;
    case PEF_OPEN_ALL:
    {
        _steps = static_cast<int>( static_cast<float>( kapp->desktop()->width() ) / pageSpeedFakt() );
        _time.start();

        for ( ; ; )
        {
            kapp->processEvents();
            if ( _time.elapsed() >= 1 )
            {
                _step++;
                _w = ( _pix2.width() / _steps ) * _step;
                _w = _w > _pix2.width() ? _pix2.width() : _w;

                _x = _pix2.width() / 2;

                _h = ( _pix2.height() / _steps ) * _step;
                _h = _h > _pix2.height() ? _pix2.height() : _h;

                _y = _pix2.height() / 2;

                bitBlt( this, _x - _w / 2, _y - _h / 2, &_pix2, _x - _w / 2, _y - _h / 2, _w, _h );

                _time.restart();
            }
            if ( ( _pix2.width() / _steps ) * _step >= _pix2.width() &&
                 ( _pix2.height() / _steps ) * _step >= _pix2.height() ) break;
        }
    } break;
    case PEF_INTERLOCKING_HORZ_1:
    {
        _steps = static_cast<int>( static_cast<float>( kapp->desktop()->width() ) / pageSpeedFakt() );
        _time.start();

        for ( ; ; )
        {
            kapp->processEvents();
            if ( _time.elapsed() >= 1 )
            {
                _step++;
                _w = ( _pix2.width() / _steps ) * _step;
                _w = _w > _pix2.width() ? _pix2.width() : _w;

                bitBlt( this, 0, 0, &_pix2, 0, 0, _w, height() / 4 );
                bitBlt( this, 0, height() / 2, &_pix2, 0, height() / 2, _w, height() / 4 );
                bitBlt( this, width() - _w, height() / 4, &_pix2, width() - _w, height() / 4, _w, height() / 4 );
                bitBlt( this, width() - _w, height() / 2 + height() / 4, &_pix2, width() - _w,
                        height() / 2 + height() / 4, _w, height() / 4 );

                _time.restart();
            }
            if ( ( _pix2.width() / _steps ) * _step >= _pix2.width() ) break;
        }
    } break;
    case PEF_INTERLOCKING_HORZ_2:
    {
        _steps = static_cast<int>( static_cast<float>( kapp->desktop()->width() ) / pageSpeedFakt() );
        _time.start();

        for ( ; ; )
        {
            kapp->processEvents();
            if ( _time.elapsed() >= 1 )
            {
                _step++;
                _w = ( _pix2.width() / _steps ) * _step;
                _w = _w > _pix2.width() ? _pix2.width() : _w;

                bitBlt( this, 0, height() / 4, &_pix2, 0, height() / 4, _w, height() / 4 );
                bitBlt( this, 0, height() / 2 + height() / 4, &_pix2, 0, height() / 2 + height() / 4, _w, height() / 4 );
                bitBlt( this, width() - _w, 0, &_pix2, width() - _w, 0, _w, height() / 4 );
                bitBlt( this, width() - _w, height() / 2, &_pix2, width() - _w, height() / 2, _w, height() / 4 );

                _time.restart();
            }
            if ( ( _pix2.width() / _steps ) * _step >= _pix2.width() ) break;
        }
    } break;
    case PEF_INTERLOCKING_VERT_1:
    {
        _steps = static_cast<int>( static_cast<float>( kapp->desktop()->height() ) / pageSpeedFakt() );
        _time.start();

        for ( ; ; )
        {
            kapp->processEvents();
            if ( _time.elapsed() >= 1 )
            {
                _step++;
                _h = ( _pix2.height() / _steps ) * _step;
                _h = _h > _pix2.height() ? _pix2.height() : _h;

                bitBlt( this, 0, 0, &_pix2, 0, 0, width() / 4, _h );
                bitBlt( this, width() / 2, 0, &_pix2, width() / 2, 0, width() / 4, _h );
                bitBlt( this, width() / 4, height() - _h, &_pix2, width() / 4, height() - _h, width() / 4, _h );
                bitBlt( this, width() / 2 + width() / 4, height() - _h, &_pix2, width() / 2 + width() / 4, height() - _h,
                        width() / 4, _h );

                _time.restart();
            }
            if ( ( _pix2.height() / _steps ) * _step >= _pix2.height() ) break;
        }
    } break;
    case PEF_INTERLOCKING_VERT_2:
    {
        _steps = static_cast<int>( static_cast<float>( kapp->desktop()->height() ) / pageSpeedFakt() );
        _time.start();

        for ( ; ; )
        {
            kapp->processEvents();
            if ( _time.elapsed() >= 1 )
            {
                _step++;
                _h = ( _pix2.height() / _steps ) * _step;
                _h = _h > _pix2.height() ? _pix2.height() : _h;

                bitBlt( this, width() / 4, 0, &_pix2, width() / 4, 0, width() / 4, _h );
                bitBlt( this, width() / 2 + width() / 4, 0, &_pix2, width() / 2 + width() / 4, 0, width() / 4, _h );
                bitBlt( this, 0, height() - _h, &_pix2, 0, height() - _h, width() / 4, _h );
                bitBlt( this, width() / 2, height() - _h, &_pix2, width() / 2, height() - _h, width() / 4, _h );

                _time.restart();
            }
            if ( ( _pix2.height() / _steps ) * _step >= _pix2.height() ) break;
        }
    } break;
    case PEF_SURROUND1:
    {
        int wid = kapp->desktop()->width() / 10;
        int hei = kapp->desktop()->height() / 10;

        int curr = 1;
        int curr2 = 1;

        _steps = static_cast<int>( static_cast<float>( kapp->desktop()->width() ) / pageSpeedFakt() );
        _time.start();

        for ( ; ; )
        {
            kapp->processEvents();
            if ( _time.elapsed() >= 1 )
            {
                _step++;

                if ( curr == 1 || curr == 5 || curr == 9 || curr == 13 || curr == 17 )
                {
                    int dx = ( curr2 / 4 ) * wid;
                    int dy = ( curr2 / 4 ) * hei;
                    _h = ( _pix2.height() / _steps ) * _step;
                    if ( _h >= _pix2.height() - 2 * dy )
                    {
                        _h = _pix2.height() - 2 * dy;
                        curr++;
                        _step = 0;
                    }
                    bitBlt( this, dx, dy, &_pix2, dx, dy, wid, _h );
                }
                else if ( curr == 2 || curr == 6 || curr == 10 || curr == 14 || curr == 18 )
                {
                    int dx = ( curr2 / 4 ) * wid;
                    int dy = ( curr2 / 4 ) * hei;
                    _w = ( _pix2.width() / _steps ) * _step;
                    if ( _w >= _pix2.width() - wid - 2 * dx )
                    {
                        _w = _pix2.width() - wid - 2 * dx;
                        curr++;
                        _step = 0;
                    }
                    bitBlt( this, dx + wid, height() - hei - dy, &_pix2, dx + wid, height() - hei - dy, _w, hei );
                }
                else if ( curr == 3 || curr == 7 || curr == 11 || curr == 15 || curr == 19 )
                {
                    int dx = ( curr2 / 4 ) * wid;
                    int dy = ( curr2 / 4 ) * hei;
                    _h = ( _pix2.height() / _steps ) * _step;
                    if ( _h >= _pix2.height() - hei - 2 * dy )
                    {
                        _h = _pix2.height() - hei - 2 * dy;
                        curr++;
                        _step = 0;
                    }
                    bitBlt( this, _pix2.width() - wid - dx, _pix2.height() - hei - dy - _h, &_pix2,
                            _pix2.width() - wid - dx, _pix2.height() - hei - dy - _h, wid, _h );
                }
                else if ( curr == 4 || curr == 8 || curr == 12 || curr == 16 || curr == 20 )
                {
                    int dx = ( curr2 / 4 ) * wid;
                    int dy = ( curr2 / 4 ) * hei;
                    _w = ( _pix2.width() / _steps ) * _step;
                    if ( _w >= _pix2.width() - 2 * wid - 2 * dx )
                    {
                        _w = _pix2.width() - 2 * wid - 2 * dx;
                        _steps *= 2;
                        _steps = static_cast<int>( static_cast<float>( _steps ) / 1.5 );
                        curr++;
                        curr2 += 4;
                        _step = 0;
                    }
                    bitBlt( this, _pix2.width() - dx - wid - _w, dy, &_pix2, _pix2.width() - dx - wid - _w,
                            dy, _w, hei );
                }
                _time.restart();
            }
            if ( curr == 21 )
            {
                bitBlt( this, 0, 0, &_pix2, 0, 0, _pix2.width(), _pix2.height() );
                break;
            }
        }
    } break;
    case PEF_FLY1:
    {
        _steps = static_cast<int>( static_cast<float>( kapp->desktop()->height() ) / pageSpeedFakt() );
        _time.start();

        int _psteps = _steps / 5;
        QRect oldRect( 0, 0, kapp->desktop()->width(), kapp->desktop()->height() );
        QSize ps;
        QPixmap pix3;

        for ( ; ; )
        {
            kapp->processEvents();
            if ( _time.elapsed() >= 1 )
            {
                _step++;
                if ( _step < _psteps )
                {
                    pix3 = QPixmap( _pix1 );
                    QPixmap pix4( _pix2 );
                    float dw = static_cast<float>( _step * ( ( pix3.width() - ( pix3.width() / 10 ) ) /
                                                             ( 2 * _psteps ) ) );
                    float dh = static_cast<float>( _step * ( ( pix3.height() - ( pix3.height() / 10 ) ) /
                                                             ( 2 * _psteps ) ) );

                    dw *= 2;
                    dh *= 2;

                    QWMatrix m;
                    m.scale( static_cast<float>( pix3.width() - dw ) / static_cast<float>( pix3.width() ),
                             static_cast<float>( pix3.height() - dh ) / static_cast<float>( pix3.height() ) );
                    pix3 = pix3.xForm( m );
                    ps = pix3.size();

                    bitBlt( &pix4, ( pix4.width() - pix3.width() ) / 2, ( pix4.height() - pix3.height() ) / 2,
                            &pix3, 0, 0, pix3.width(), pix3.height() );
                    QRect newRect( ( pix4.width() - pix3.width() ) / 2, ( pix4.height() - pix3.height() ) / 2,
                                   pix3.width(), pix3.height() );
                    QRect r = newRect.unite( oldRect );
                    bitBlt( this, r.x(), r.y(), &pix4, r.x(), r.y(), r.width(), r.height() );
                    oldRect = newRect;
                }
                if ( _step > _psteps && _step < _psteps * 2 )
                {
                    QPixmap pix4( _pix2 );
                    int yy = ( _pix1.height() - pix3.height() ) / 2 - ( ( ( _pix1.height() - pix3.height() ) / 2 ) /
                                                                        _psteps ) * ( _step - _psteps );

                    bitBlt( &pix4, ( pix4.width() - pix3.width() ) / 2, yy,
                            &pix3, 0, 0, pix3.width(), pix3.height() );
                    QRect newRect( ( pix4.width() - pix3.width() ) / 2, yy,
                                   pix3.width(), pix3.height() );
                    QRect r = newRect.unite( oldRect );
                    bitBlt( this, r.x(), r.y(), &pix4, r.x(), r.y(), r.width(), r.height() );
                    oldRect = newRect;
                }
                if ( _step > 2 * _psteps && _step < _psteps * 3 )
                {
                    QPixmap pix4( _pix2 );
                    int xx = ( _pix1.width() - pix3.width() ) / 2 - ( ( ( _pix1.width() - pix3.width() ) / 2 ) /
                                                                      _psteps ) * ( _step - 2 * _psteps );
                    int yy = ( ( ( _pix1.height() - pix3.height() ) / 2 ) / _psteps ) * ( _step - 2 * _psteps );

                    bitBlt( &pix4, xx, yy, &pix3, 0, 0, pix3.width(), pix3.height() );
                    QRect newRect( xx, yy, pix3.width(), pix3.height() );
                    QRect r = newRect.unite( oldRect );
                    bitBlt( this, r.x(), r.y(), &pix4, r.x(), r.y(), r.width(), r.height() );
                    oldRect = newRect;
                }
                if ( _step > 3 * _psteps && _step < _psteps * 5 )
                {
                    QPixmap pix4( _pix2 );
                    int xx = ( ( _pix1.width() - pix3.width() ) / _psteps ) * ( _step - 3 * _psteps );
                    int yy = ( ( _pix1.height() - pix3.height() ) / 2 ) +
                             ( ( ( _pix1.height() - pix3.height() ) / 2 ) / _psteps ) * ( _step - 3 * _psteps );

                    bitBlt( &pix4, xx, yy, &pix3, 0, 0, pix3.width(), pix3.height() );
                    QRect newRect( xx, yy, pix3.width(), pix3.height() );
                    QRect r = newRect.unite( oldRect );
                    bitBlt( this, r.x(), r.y(), &pix4, r.x(), r.y(), r.width(), r.height() );
                    oldRect = newRect;
                }
                _time.restart();
            }
            if ( _step >= _steps )
            {
                bitBlt( this, 0, 0, &_pix2, 0, 0, _pix2.width(), _pix2.height() );
                break;
            }
        }
    } break;
    }
}

/*================================================================*/
void Page::doObjEffects()
{
    QPixmap screen_orig( kapp->desktop()->width(), kapp->desktop()->height() );
    bool drawn = false;

    // YABADABADOOOOOOO.... That's a hack :-)
    if ( subPresStep == 0 && currPresPage > 0 )
    {
        //kdDebug(33001) << "Page::doObjEffects - in the strange hack" << endl;
        inEffect = true;
        QPainter p;
        p.begin( &screen_orig );
        drawBackground( &p, QRect( 0, 0, kapp->desktop()->width(), kapp->desktop()->height() ) );
        drawObjects( &p, QRect( 0, 0, kapp->desktop()->width(), kapp->desktop()->height() ), false );
        p.end();
        inEffect = false;
        bitBlt( this, 0, 0, &screen_orig, 0, 0, screen_orig.width(), screen_orig.height() );
        drawn = true;
    }

    QPtrList<KPObject> _objList;
    QTime _time;
    int _step = 0, _steps1 = 0, _steps2 = 0, x_pos1 = 0, y_pos1 = 0;
    int x_pos2 = kapp->desktop()->width(), y_pos2 = kapp->desktop()->height(), _step_width = 0, _step_height = 0;
    int w_pos1 = 0, h_pos1;
    bool effects = false;
    bool nothingHappens = false;
    int timer = 0;
    bool _soundEffect = false;
    QString _soundFileName = QString::null;
    if ( !drawn )
        bitBlt( &screen_orig, 0, 0, this, 0, 0, kapp->desktop()->width(), kapp->desktop()->height() );
    QPixmap *screen = new QPixmap( screen_orig );

    QPtrListIterator<KPObject> oit(*objectList());
    for ( int i = 0 ; oit.current(); ++oit, ++i )
    {
        KPObject *kpobject = oit.current();
        if ( getPageOfObj( i, _presFakt ) == static_cast<int>( currPresPage )
             && kpobject->getPresNum() == static_cast<int>( currPresStep ) )
        {
            if ( !spManualSwitch() )
                timer = kpobject->getAppearTimer();

            if ( kpobject->getEffect() != EF_NONE )
            {
                _soundEffect = kpobject->getAppearSoundEffect();
                _soundFileName = kpobject->getAppearSoundEffectFileName();

                _objList.append( kpobject );

                int x = 0, y = 0, w = 0, h = 0;
                QRect br = kpobject->getBoundingRect( 0, 0 );
                x = br.x(); y = br.y(); w = br.width(); h = br.height();

                switch ( kpobject->getEffect() )
                {
                case EF_COME_LEFT:
                    x_pos1 = QMAX( x_pos1, x - diffx() + w );
                    break;
                case EF_COME_TOP:
                    y_pos1 = QMAX( y_pos1, y - diffy() + h );
                    break;
                case EF_COME_RIGHT:
                    x_pos2 = QMIN( x_pos2, x - diffx() );
                    break;
                case EF_COME_BOTTOM:
                    y_pos2 = QMIN( y_pos2, y - diffy() );
                    break;
                case EF_COME_LEFT_TOP:
                {
                    x_pos1 = QMAX( x_pos1, x - diffx() + w );
                    y_pos1 = QMAX( y_pos1, y - diffy() + h );
                } break;
                case EF_COME_LEFT_BOTTOM:
                {
                    x_pos1 = QMAX( x_pos1, x - diffx() + w );
                    y_pos2 = QMIN( y_pos2, y - diffy() );
                } break;
                case EF_COME_RIGHT_TOP:
                {
                    x_pos2 = QMIN( x_pos2, x - diffx() );
                    y_pos1 = QMAX( y_pos1, y - diffy() + h );
                } break;
                case EF_COME_RIGHT_BOTTOM:
                {
                    x_pos2 = QMIN( x_pos2, x - diffx() );
                    y_pos2 = QMIN( y_pos2, y - diffy() );
                } break;
                case EF_WIPE_LEFT:
                    x_pos1 = QMAX( x_pos1, w );
                    break;
                case EF_WIPE_RIGHT:
                    x_pos1 = QMAX( x_pos1, w );
                    break;
                case EF_WIPE_TOP:
                    y_pos1 = QMAX( y_pos1, h );
                    break;
                case EF_WIPE_BOTTOM:
                    y_pos1 = QMAX( y_pos1, h );
                    break;
                default: break;
                }
                effects = true;
            }
        }
        else if ( getPageOfObj( i, _presFakt ) == static_cast<int>( currPresPage )
                  && kpobject->getDisappear() && kpobject->getDisappearNum() == static_cast<int>( currPresStep ) )
        {
            if ( !spManualSwitch() )
                timer = kpobject->getDisappearTimer();

            if ( kpobject->getEffect3() != EF3_NONE )
            {
                _soundEffect = kpobject->getDisappearSoundEffect();
                _soundFileName = kpobject->getDisappearSoundEffectFileName();

                _objList.append( kpobject );

                int x = 0, y = 0, w = 0, h = 0;
                QRect br = kpobject->getBoundingRect( 0, 0 );
                x = br.x(); y = br.y(); w = br.width(); h = br.height();

                switch ( kpobject->getEffect3() )
                {
                case EF3_GO_LEFT:
                    x_pos1 = QMAX( x_pos1, x - diffx() + w );
                    break;
                case EF3_GO_TOP:
                    y_pos1 = QMAX( y_pos1, y - diffy() + h );
                    break;
                case EF3_GO_RIGHT:
                    x_pos2 = QMIN( x_pos2, x - diffx() );
                    break;
                case EF3_GO_BOTTOM:
                    y_pos2 = QMIN( y_pos2, y - diffy() );
                    break;
                case EF3_GO_LEFT_TOP:
                {
                    x_pos1 = QMAX( x_pos1, x - diffx() + w );
                    y_pos1 = QMAX( y_pos1, y - diffy() + h );
                } break;
                case EF3_GO_LEFT_BOTTOM:
                {
                    x_pos1 = QMAX( x_pos1, x - diffx() + w );
                    y_pos2 = QMIN( y_pos2, y - diffy() );
                } break;
                case EF3_GO_RIGHT_TOP:
                {
                    x_pos2 = QMIN( x_pos2, x - diffx() );
                    y_pos1 = QMAX( y_pos1, y - diffy() + h );
                } break;
                case EF3_GO_RIGHT_BOTTOM:
                {
                    x_pos2 = QMIN( x_pos2, x - diffx() );
                    y_pos2 = QMIN( y_pos2, y - diffy() );
                } break;
                case EF3_WIPE_LEFT:
                    x_pos1 = QMAX( x_pos1, w );
                    break;
                case EF3_WIPE_RIGHT:
                    x_pos1 = QMAX( x_pos1, w );
                    break;
                case EF3_WIPE_TOP:
                    y_pos1 = QMAX( y_pos1, h );
                    break;
                case EF3_WIPE_BOTTOM:
                    y_pos1 = QMAX( y_pos1, h );
                    break;
                default: break;
                }
                effects = true;
            }
        }
    }

    if ( effects )
    {
        if ( !spManualSwitch() && timer > 0 )
            view->autoScreenPresStopTimer();

        if ( _soundEffect && !_soundFileName.isEmpty() ) {
            stopSound();
            playSound( _soundFileName );
        }

        _step_width = static_cast<int>( ( static_cast<float>( kapp->desktop()->width() ) / objSpeedFakt() ) );
        _step_height = static_cast<int>( ( static_cast<float>( kapp->desktop()->height() ) / objSpeedFakt() ) );
        _steps1 = x_pos1 > y_pos1 ? x_pos1 / _step_width : y_pos1 / _step_height;
        _steps2 = kapp->desktop()->width() - x_pos2 > kapp->desktop()->height() - y_pos2 ?
                  ( kapp->desktop()->width() - x_pos2 ) / _step_width : ( kapp->desktop()->height() - y_pos2 ) / _step_height;
        _time.start();

        QPtrList<QRect> xy;
        xy.setAutoDelete( true );

        for ( ; ; )
        {
            kapp->processEvents();
            if ( nothingHappens ) break; // || _step >= _steps1 && _step >= _steps2 ) break;

            QPtrList<QRect> changes;
            changes.setAutoDelete( true );

            if ( _time.elapsed() >= 1 )
            {
                nothingHappens = true;
                _step++;

                changes.clear();

                for ( int i = 0; i < static_cast<int>( _objList.count() ); i++ )
                {
                    KPObject * kpobject = _objList.at( i );
                    int _w =  kapp->desktop()->width() - ( kpobject->getOrig().x() - diffx() );
                    int _h =  kapp->desktop()->height() - ( kpobject->getOrig().y() - diffy() );
                    int ox = 0, oy = 0, ow = 0, oh = 0;
                    ox = kpobject->getOrig().x();
                    oy = kpobject->getOrig().y();
                    ow = kpobject->getSize().width();
                    oh = kpobject->getSize().height();

                    QRect br = kpobject->getBoundingRect( 0, 0 );
                    int bx = br.x();
                    int by = br.y();
                    int bw = br.width();
                    int bh = br.height();

                    QRect oldRect;
                    QRect newRect;

                    if ( static_cast<int>( xy.count() - 1 ) < i )
                    {
                        xy.append( new QRect( 0, 0, 0, 0 ) );
                        oldRect.setRect( bx - diffx(), by - diffy(), bw, bh );
                    }
                    else
                        oldRect.setRect( bx - ( diffx() - xy.at( i )->x() ), by - ( diffy() - xy.at( i )->y() ), bw - xy.at( i )->width(), bh - xy.at( i )->height() );

                    if ( !kpobject->getDisappear() || kpobject->getDisappear() && kpobject->getDisappearNum() != static_cast<int>( currPresStep ) )
                    {
                        switch ( kpobject->getEffect() )
                        {
                        case EF_NONE:
                        {
                            if ( subPresStep == 0 || subPresStep != 0 && kpobject->getType() == OT_TEXT && kpobject->getEffect2() == EF2T_PARA )
                                drawObject( kpobject, screen, ox, oy, 0, 0, 0, 0 );
                        } break;
                        case EF_COME_LEFT:
                        {
                            if ( subPresStep == 0 || subPresStep != 0 && kpobject->getType() == OT_TEXT && kpobject->getEffect2() == EF2T_PARA )
                            {
                                x_pos1 = _step_width * _step < ox - diffx() + ow ?
                                         ox - diffx() + ow - _step_width * _step : 0;
                                y_pos1 = 0;
                                drawObject( kpobject, screen, -x_pos1, y_pos1, 0, 0, 0, 0 );
                                if ( x_pos1 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( -x_pos1 );
                                xy.at( i )->setY( y_pos1 );
                                xy.at( i )->setWidth( 0 );
                                xy.at( i )->setHeight( 0 );
                            }
                        } break;
                        case EF_COME_TOP:
                        {
                            if ( subPresStep == 0 || subPresStep != 0 && kpobject->getType() == OT_TEXT && kpobject->getEffect2() == EF2T_PARA )
                            {
                                y_pos1 = _step_height * _step < oy - diffy() + oh ?
                                         oy - diffy() + oh - _step_height * _step : 0;
                                x_pos1 = 0;
                                drawObject( kpobject, screen, x_pos1, -y_pos1, 0, 0, 0, 0 );
                                if ( y_pos1 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( x_pos1 );
                                xy.at( i )->setY( -y_pos1 );
                                xy.at( i )->setWidth( 0 );
                                xy.at( i )->setHeight( 0 );
                            }
                        } break;
                        case EF_COME_RIGHT:
                        {
                            if ( subPresStep == 0 || subPresStep != 0 && kpobject->getType() == OT_TEXT && kpobject->getEffect2() == EF2T_PARA )
                            {
                                x_pos2 = _w - ( _step_width * _step ) + ( ox - diffx() ) > ox - diffx() ?
                                         _w - ( _step_width * _step ) : 0;
                                y_pos2 = 0;
                                drawObject( kpobject, screen, x_pos2, y_pos2, 0, 0, 0, 0 );
                                if ( x_pos2 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( x_pos2 );
                                xy.at( i )->setY( y_pos2 );
                                xy.at( i )->setWidth( 0 );
                                xy.at( i )->setHeight( 0 );
                            }
                        } break;
                        case EF_COME_BOTTOM:
                        {
                            if ( subPresStep == 0 || subPresStep != 0 && kpobject->getType() == OT_TEXT && kpobject->getEffect2() == EF2T_PARA )
                            {
                                y_pos2 = _h - ( _step_height * _step ) + ( oy - diffy() ) > oy - diffy() ?
                                         _h - ( _step_height * _step ) : 0;
                                x_pos2 = 0;
                                drawObject( kpobject, screen, x_pos2, y_pos2, 0, 0, 0, 0 );
                                if ( y_pos2 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( x_pos2 );
                                xy.at( i )->setY( y_pos2 );
                                xy.at( i )->setWidth( 0 );
                                xy.at( i )->setHeight( 0 );
                            }
                        } break;
                        case EF_COME_LEFT_TOP:
                        {
                            if ( subPresStep == 0 || subPresStep != 0 && kpobject->getType() == OT_TEXT && kpobject->getEffect2() == EF2T_PARA )
                            {
                                x_pos1 = _step_width * _step < ox - diffx() + ow ?
                                         ox - diffx() + ow - _step_width * _step : 0;
                                y_pos1 = _step_height * _step < oy - diffy() + oh ?
                                         oy - diffy() + oh - _step_height * _step : 0;
                                drawObject( kpobject, screen, -x_pos1, -y_pos1, 0, 0, 0, 0 );
                                if ( x_pos1 != 0 || y_pos1 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( -x_pos1 );
                                xy.at( i )->setY( -y_pos1 );
                                xy.at( i )->setWidth( 0 );
                                xy.at( i )->setHeight( 0 );
                            }
                        } break;
                        case EF_COME_LEFT_BOTTOM:
                        {
                            if ( subPresStep == 0 || subPresStep != 0 && kpobject->getType() == OT_TEXT && kpobject->getEffect2() == EF2T_PARA )
                            {
                                x_pos1 = _step_width * _step < ox - diffx() + ow ?
                                         ox - diffx() + ow - _step_width * _step : 0;
                                y_pos2 = _h - ( _step_height * _step ) + ( oy - diffy() ) > oy - diffy() ?
                                         _h - ( _step_height * _step ) : 0;
                                drawObject( kpobject, screen, -x_pos1, y_pos2, 0, 0, 0, 0 );
                                if ( x_pos1 != 0 || y_pos2 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( -x_pos1 );
                                xy.at( i )->setY( y_pos2 );
                                xy.at( i )->setWidth( 0 );
                                xy.at( i )->setHeight( 0 );
                            }
                        } break;
                        case EF_COME_RIGHT_TOP:
                        {
                            if ( subPresStep == 0 || subPresStep != 0 && kpobject->getType() == OT_TEXT && kpobject->getEffect2() == EF2T_PARA )
                            {
                                x_pos2 = _w - ( _step_width * _step ) + ( ox - diffx() ) > ox - diffx() ?
                                         _w - ( _step_width * _step ) : 0;
                                y_pos1 = _step_height * _step < oy - diffy() + oh ?
                                         oy - diffy() + oh - _step_height * _step : 0;
                                drawObject( kpobject, screen, x_pos2, -y_pos1, 0, 0, 0, 0 );
                                if ( x_pos2 != 0 || y_pos1 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( x_pos2 );
                                xy.at( i )->setY( -y_pos1 );
                                xy.at( i )->setWidth( 0 );
                                xy.at( i )->setHeight( 0 );
                            }
                        } break;
                        case EF_COME_RIGHT_BOTTOM:
                        {
                            if ( subPresStep == 0 || subPresStep != 0 && kpobject->getType() == OT_TEXT && kpobject->getEffect2() == EF2T_PARA )
                            {
                                x_pos2 = _w - ( _step_width * _step ) + ( ox - diffx() ) > ox - diffx() ?
                                         _w - ( _step_width * _step ) : 0;
                                y_pos2 = _h - ( _step_height * _step ) + ( oy - diffy() ) > oy - diffy() ?
                                         _h - ( _step_height * _step ) : 0;
                                drawObject( kpobject, screen, x_pos2, y_pos2, 0, 0, 0, 0 );
                                if ( x_pos2 != 0 || y_pos2 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( x_pos2 );
                                xy.at( i )->setY( y_pos2 );
                                xy.at( i )->setWidth( 0 );
                                xy.at( i )->setHeight( 0 );
                            }
                        } break;
                        case EF_WIPE_LEFT:
                        {
                            if ( subPresStep == 0 || subPresStep != 0 && kpobject->getType() == OT_TEXT && kpobject->getEffect2() == EF2T_PARA )
                            {
                                w_pos1 = _step_width * ( _steps1 - _step ) > 0 ? _step_width * ( _steps1 - _step ) : 0;
                                drawObject( kpobject, screen, 0, 0, w_pos1, 0, 0, 0 );
                                if ( w_pos1 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( 0 );
                                xy.at( i )->setY( 0 );
                                xy.at( i )->setWidth( w_pos1 );
                                xy.at( i )->setHeight( 0 );
                            }
                        } break;
                        case EF_WIPE_RIGHT:
                        {
                            if ( subPresStep == 0 || subPresStep != 0 && kpobject->getType() == OT_TEXT && kpobject->getEffect2() == EF2T_PARA )
                            {
                                w_pos1 = _step_width * ( _steps1 - _step ) > 0 ? _step_width * ( _steps1 - _step ) : 0;
                                x_pos1 = w_pos1;
                                drawObject( kpobject, screen, 0, 0, w_pos1, 0, x_pos1, 0 );
                                if ( w_pos1 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( x_pos1 );
                                xy.at( i )->setY( 0 );
                                xy.at( i )->setWidth( w_pos1 );
                                xy.at( i )->setHeight( 0 );
                            }
                        } break;
                        case EF_WIPE_TOP:
                        {
                            if ( subPresStep == 0 || subPresStep != 0 && kpobject->getType() == OT_TEXT && kpobject->getEffect2() == EF2T_PARA )
                            {
                                h_pos1 = _step_height * ( _steps1 - _step ) > 0 ? _step_height * ( _steps1 - _step ) : 0;
                                drawObject( kpobject, screen, 0, 0, 0, h_pos1, 0, 0 );
                                if ( h_pos1 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( 0 );
                                xy.at( i )->setY( 0 );
                                xy.at( i )->setWidth( 0 );
                                xy.at( i )->setHeight( h_pos1 );
                            }
                        } break;
                        case EF_WIPE_BOTTOM:
                        {
                            if ( subPresStep == 0 || subPresStep != 0 && kpobject->getType() == OT_TEXT && kpobject->getEffect2() == EF2T_PARA )
                            {
                                h_pos1 = _step_height * ( _steps1 - _step ) > 0 ? _step_height * ( _steps1 - _step ) : 0;
                                y_pos1 = h_pos1;
                                drawObject( kpobject, screen, 0, 0, 0, h_pos1, 0, y_pos1 );
                                if ( h_pos1 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( 0 );
                                xy.at( i )->setY( y_pos1 );
                                xy.at( i )->setWidth( 0 );
                                xy.at( i )->setHeight( h_pos1 );
                            }
                        } break;
                        default: break;
                        }
                    }
                    else
                    {
                        if ( subPresStep == 0 )
                        {
                            switch ( kpobject->getEffect3() )
                            {
                            case EF3_NONE:
                                //drawObject( kpobject, screen, ox, oy, 0, 0, 0, 0 );
                                break;
                            case EF3_GO_LEFT:
                            {
                                x_pos1 = _step_width * _step < ox - diffx() + ow ?
                                         ox - diffx() + ow - _step_width * _step : 0;
                                y_pos1 = 0;
                                drawObject( kpobject, screen, -( ox + ow - x_pos1 ), y_pos1, 0, 0, 0, 0 );
                                if ( x_pos1 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( -( ox + ow - x_pos1 ) );
                                xy.at( i )->setY( y_pos1 );
                                xy.at( i )->setWidth( 0 );
                                xy.at( i )->setHeight( 0 );
                            } break;
                            case EF3_GO_TOP:
                            {
                                y_pos1 = _step_height * _step < oy - diffy() + oh ?
                                         oy - diffy() + oh - _step_height * _step : 0;
                                x_pos1 = 0;
                                drawObject( kpobject, screen, x_pos1, -( ( oy - diffy() ) + oh - y_pos1 ), 0, 0, 0, 0 );
                                if ( y_pos1 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( x_pos1 );
                                xy.at( i )->setY( -( ( oy - diffy() ) + oh - y_pos1 ) );
                                xy.at( i )->setWidth( 0 );
                                xy.at( i )->setHeight( 0 );
                            } break;
                            case EF3_GO_RIGHT:
                            {
                                x_pos2 = _w - ( _step_width * _step ) + ( ox - diffx() ) > ox - diffx() ?
                                         _w - ( _step_width * _step ) : 0;
                                y_pos2 = 0;
                                int __w = kapp->desktop()->width() - ox;
                                drawObject( kpobject, screen, __w - x_pos2, y_pos2, 0, 0, 0, 0 );
                                if ( x_pos2 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( __w - x_pos2 );
                                xy.at( i )->setY( y_pos2 );
                                xy.at( i )->setWidth( 0 );
                                xy.at( i )->setHeight( 0 );
                            } break;
                            case EF3_GO_BOTTOM:
                            {
                                y_pos2 = _h - ( _step_height * _step ) + ( oy - diffy() ) > oy - diffy() ?
                                         _h - ( _step_height * _step ) : 0;
                                x_pos2 = 0;
                                int __h = kapp->desktop()->height() - ( oy - diffy() );
                                drawObject( kpobject, screen, x_pos2, __h - y_pos2, 0, 0, 0, 0 );
                                if ( y_pos2 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( x_pos2 );
                                xy.at( i )->setY( __h - y_pos2 );
                                xy.at( i )->setWidth( 0 );
                                xy.at( i )->setHeight( 0 );
                            } break;
                            case EF3_GO_LEFT_TOP:
                            {
                                x_pos1 = _step_width * _step < ox - diffx() + ow ?
                                         ox - diffx() + ow - _step_width * _step : 0;
                                y_pos1 = _step_height * _step < oy - diffy() + oh ?
                                         oy - diffy() + oh - _step_height * _step : 0;
                                drawObject( kpobject, screen, -( ox + ow - x_pos1 ), -( ( oy - diffy() ) + oh - y_pos1 ), 0, 0, 0, 0 );
                                if ( x_pos1 != 0 || y_pos1 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( -( ox + ow - x_pos1 ) );
                                xy.at( i )->setY( -( ( oy - diffy() ) + oh - y_pos1 ) );
                                xy.at( i )->setWidth( 0 );
                                xy.at( i )->setHeight( 0 );
                            } break;
                            case EF3_GO_LEFT_BOTTOM:
                            {
                                x_pos1 = _step_width * _step < ox - diffx() + ow ?
                                         ox - diffx() + ow - _step_width * _step : 0;
                                y_pos2 = _h - ( _step_height * _step ) + ( oy - diffy() ) > oy - diffy() ?
                                         _h - ( _step_height * _step ) : 0;
                                int __h = kapp->desktop()->height() - ( oy - diffy() );
                                drawObject( kpobject, screen, -( ox + ow - x_pos1 ), __h -  y_pos2, 0, 0, 0, 0 );
                                if ( x_pos1 != 0 || y_pos2 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( -( ox + ow - x_pos1 ) );
                                xy.at( i )->setY( __h - y_pos2 );
                                xy.at( i )->setWidth( 0 );
                                xy.at( i )->setHeight( 0 );
                            } break;
                            case EF3_GO_RIGHT_TOP:
                            {
                                x_pos2 = _w - ( _step_width * _step ) + ( ox - diffx() ) > ox - diffx() ?
                                         _w - ( _step_width * _step ) : 0;
                                y_pos1 = _step_height * _step < oy - diffy() + oh ?
                                         oy - diffy() + oh - _step_height * _step : 0;
                                int __w = kapp->desktop()->width() - ox;
                                drawObject( kpobject, screen, __w - x_pos2, -( ( oy - diffy() ) + oh - y_pos1 ), 0, 0, 0, 0 );
                                if ( x_pos2 != 0 || y_pos1 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( __w - x_pos2 );
                                xy.at( i )->setY( -( ( oy - diffy() ) + oh - y_pos1 ) );
                                xy.at( i )->setWidth( 0 );
                                xy.at( i )->setHeight( 0 );
                            } break;
                            case EF3_GO_RIGHT_BOTTOM:
                            {
                                x_pos2 = _w - ( _step_width * _step ) + ( ox - diffx() ) > ox - diffx() ?
                                         _w - ( _step_width * _step ) : 0;
                                y_pos2 = _h - ( _step_height * _step ) + ( oy - diffy() ) > oy - diffy() ?
                                         _h - ( _step_height * _step ) : 0;
                                int __w = kapp->desktop()->width() - ox;
                                int __h = kapp->desktop()->height() - ( oy - diffy() );
                                drawObject( kpobject, screen, __w - x_pos2, __h - y_pos2, 0, 0, 0, 0 );
                                if ( x_pos2 != 0 || y_pos2 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( __w - x_pos2 );
                                xy.at( i )->setY( __h - y_pos2 );
                                xy.at( i )->setWidth( 0 );
                                xy.at( i )->setHeight( 0 );
                            } break;
                            case EF3_WIPE_LEFT:
                            {
                                if ( subPresStep == 0 || subPresStep != 0 && kpobject->getType() == OT_TEXT &&
                                     kpobject->getEffect2() == EF2T_PARA )
                                {
                                    w_pos1 = _step_width * ( _step - 1 );
                                    x_pos1 = w_pos1;
                                    drawObject( kpobject, screen, 0, 0, w_pos1, 0, x_pos1, 0 );
                                    if ( ( _step_width * ( _steps1 - _step ) ) != 0 ) nothingHappens = false;
                                    xy.at( i )->setX( x_pos1 );
                                    xy.at( i )->setY( 0 );
                                    xy.at( i )->setWidth( w_pos1 );
                                    xy.at( i )->setHeight( 0 );
                                }
                            } break;
                            case EF3_WIPE_RIGHT:
                            {
                                if ( subPresStep == 0 || subPresStep != 0 && kpobject->getType() == OT_TEXT &&
                                     kpobject->getEffect2() == EF2T_PARA )
                                {
                                    w_pos1 = _step_width * ( _step - 1 );
                                    drawObject( kpobject, screen, 0, 0, w_pos1, 0, 0, 0 );
                                    if ( ( _step_width * ( _steps1 - _step ) ) != 0 ) nothingHappens = false;
                                    xy.at( i )->setX( 0 );
                                    xy.at( i )->setY( 0 );
                                    xy.at( i )->setWidth( w_pos1 );
                                    xy.at( i )->setHeight( 0 );
                                }
                            } break;
                            case EF3_WIPE_TOP:
                            {
                                if ( subPresStep == 0 || subPresStep != 0 && kpobject->getType() == OT_TEXT &&
                                     kpobject->getEffect2() == EF2T_PARA )
                                {
                                    h_pos1 = _step_height * ( _step - 1 );
                                    y_pos1 = h_pos1;
                                    drawObject( kpobject, screen, 0, 0, 0, h_pos1, 0, y_pos1 );
                                    if ( ( _step_height * ( _steps1 - _step ) ) != 0 ) nothingHappens = false;
                                    xy.at( i )->setX( 0 );
                                    xy.at( i )->setY( y_pos1 );
                                    xy.at( i )->setWidth( 0 );
                                    xy.at( i )->setHeight( h_pos1 );
                                }
                            } break;
                            case EF3_WIPE_BOTTOM:
                            {
                                if ( subPresStep == 0 || subPresStep != 0 && kpobject->getType() == OT_TEXT &&
                                     kpobject->getEffect2() == EF2T_PARA )
                                {
                                    h_pos1 = _step_height * ( _step - 1 );
                                    drawObject( kpobject, screen, 0, 0, 0, h_pos1, 0, 0 );
                                    if ( ( _step_height * ( _steps1 - _step ) ) != 0 ) nothingHappens = false;
                                    xy.at( i )->setX( 0 );
                                    xy.at( i )->setY( 0 );
                                    xy.at( i )->setWidth( 0 );
                                    xy.at( i )->setHeight( h_pos1 );
                                }
                            } break;
                            default:
                                break;
                            }
                        }
                    }
                    newRect.setRect( bx - ( diffx() - xy.at( i )->x() ), by - ( diffy() - xy.at( i )->y() ), bw - xy.at( i )->width(), bh - xy.at( i )->height() );
                    newRect = newRect.unite( oldRect );

                    bool append = true;
                    for ( unsigned j = 0; j < changes.count(); j++ )
                    {
                        if ( changes.at( j )->intersects( newRect ) )
                        {
                            QRect r = changes.at( j )->intersect( newRect );
                            int s1 = r.width() * r.height();
                            int s2 = newRect.width() * newRect.height();
                            int s3 = changes.at( j )->width() * changes.at( j )->height();

                            if ( s1 > ( s2 / 100 ) * 50 || s1 > ( s3 / 100 ) * 50 )
                            {
                                QRect rr = changes.at( j )->unite( newRect );
                                changes.at( j )->setRect( rr.x(), rr.y(), rr.width(), rr.height() );
                                append = false;
                            }

                            break;
                        }
                    }

                    if ( append )
                        changes.append( new QRect( newRect ) );
                }

                QRect *changed;
                for ( int i = 0; i < static_cast<int>( changes.count() ); i++ )
                {
                    changed = changes.at( i );
                    bitBlt( this, changed->x(), changed->y(), screen, changed->x(), changed->y(), changed->width(), changed->height() );
                }

                delete screen;
                screen = new QPixmap( screen_orig );

                _time.restart();
            }
        }
    }

    if ( !effects )
    {
        //kdDebug(33001) << "Page::doObjEffects no effects" << endl;
        QPainter p;
        p.begin( this );
        p.drawPixmap( 0, 0, screen_orig );
        drawObjects( &p, QRect( 0, 0, kapp->desktop()->width(), kapp->desktop()->height() ), false );
        p.end();
    }
    else
    {
        //kdDebug(33001) << "Page::doObjEffects effects" << endl;
        QPainter p;
        p.begin( screen );
        drawObjects( &p, QRect( 0, 0, kapp->desktop()->width(), kapp->desktop()->height() ), false );
        p.end();
        bitBlt( this, 0, 0, screen );
    }

    if ( !spManualSwitch() && timer > 0 )
        view->setCurrentTimer( timer );

    delete screen;
}

/*======================= draw object ============================*/
void Page::drawObject( KPObject *kpobject, QPixmap *screen, int _x, int _y, int _w, int _h, int _cx, int _cy )
{
    if ( kpobject->getDisappear() &&
         kpobject->getDisappearNum() < static_cast<int>( currPresStep ) )
        return;

    int ox, oy, ow, oh;
    QRect br = kpobject->getBoundingRect( 0, 0 );
    ox = br.x(); oy = br.y(); ow = br.width(); oh = br.height();
    bool ownClipping = true;

    QPainter p;
    p.begin( screen );

    if ( _w != 0 || _h != 0 )
    {
        p.setClipping( true );
        p.setClipRect( ox - diffx() + _cx, oy - diffy() + _cy, ow - _w, oh - _h );
        ownClipping = false;
    }

    if ( !editMode && static_cast<int>( currPresStep ) == kpobject->getPresNum() && !goingBack )
    {
        kpobject->setSubPresStep( subPresStep );
        kpobject->doSpecificEffects( true );
        kpobject->setOwnClipping( ownClipping );
    }

    kpobject->draw( &p, diffx() - _x, diffy() - _y );
    kpobject->setSubPresStep( 0 );
    kpobject->doSpecificEffects( false );
    kpobject->setOwnClipping( true );

    KPObject *obj = 0;
    for ( unsigned int i = tmpObjs.findRef( kpobject ) + 1; i < tmpObjs.count(); i++ ) {
        obj = tmpObjs.at( i );
        if ( kpobject->getBoundingRect( 0, 0 ).intersects( obj->getBoundingRect( 0, 0 ) ) &&
             obj->getPresNum() < static_cast<int>( currPresStep ) )
            obj->draw( &p, diffx(), diffy() );
    }

    p.end();
}

/*======================== print =================================*/
void Page::print( QPainter *painter, KPrinter *printer, float left_margin, float top_margin )
{
    printer->setFullPage( true );
    int i = 0;

    repaint( false );
    kapp->processEvents();

    editMode = false;
    fillBlack = false;
    _presFakt = 1.0;

    int _xOffset = diffx();
    int _yOffset = diffy();

    currPresStep = 1000;
    subPresStep = 1000;

    for ( i = 0; i < static_cast<int>( objectList()->count() ); i++ )
        objectList()->at( i )->drawSelection( false );

    view->setDiffX( -static_cast<int>( MM_TO_POINT( left_margin ) ) );
    view->setDiffY( -static_cast<int>( MM_TO_POINT( top_margin ) ) );

    //QColor c = kapp->winStyleHighlightColor();
    //kapp->setWinStyleHighlightColor( colorGroup().highlight() ); // deprecated in Qt3

    QProgressDialog progress( i18n( "Printing..." ), i18n( "Cancel" ),
                              printer->toPage() - printer->fromPage() + 2, this );

    int j = 0;
    progress.setProgress( 0 );

    if ( printer->fromPage() > 1 )
        view->setDiffY( ( printer->fromPage() - 1 ) * ( getPageRect( 1, 1.0, false ).height() ) -
                        (int)MM_TO_POINT( top_margin ) );

    for ( i = printer->fromPage(); i <= printer->toPage(); i++ )
    {
        progress.setProgress( ++j );
        kapp->processEvents();

        if ( progress.wasCancelled() )
            break;

        currPresPage = i;
        if ( i > printer->fromPage() ) printer->newPage();

        painter->resetXForm();
        painter->fillRect( getPageRect( 0 ), white );

        printPage( painter, diffy(), getPageRect( i - 1 ) );
        kapp->processEvents();

        painter->resetXForm();
        kapp->processEvents();

        view->setDiffY( i * ( getPageRect( 1, 1.0, false ).height() )
                        - static_cast<int>( MM_TO_POINT( top_margin ) ) );
    }

    NoteBar *noteBar = view->getNoteBar();
    if ( noteBar ) {
        printer->newPage();
        painter->resetXForm();
        noteBar->printNote( painter, printer );
        painter->resetXForm();
    }

    setToolEditMode( toolEditMode );
    view->setDiffX( _xOffset );
    view->setDiffY( _yOffset );

    progress.setProgress( printer->toPage() - printer->fromPage() + 2 );
    //kapp->setWinStyleHighlightColor( c );

    QPtrListIterator<KPObject> oIt( *objectList() );
    for (; oIt.current(); ++oIt )
        oIt.current()->drawSelection( true );

    currPresPage = 1;
    currPresStep = 0;
    subPresStep = 0;
    _presFakt = 1.0;
    fillBlack = true;
    editMode = true;
    repaint( false );
}

/*================================================================*/
void Page::insertText( QRect _r )
{
    view->kPresenterDoc()->insertText( _r, diffx(), diffy() );
    selectObj( objectList()->last() );
}

/*================================================================*/
void Page::insertLineH( QRect _r, bool rev )
{
    view->kPresenterDoc()->insertLine( _r, view->getPen(),
                                       !rev ? view->getLineBegin() : view->getLineEnd(), !rev ? view->getLineEnd() : view->getLineBegin(),
                                       LT_HORZ, diffx(), diffy() );
}

/*================================================================*/
void Page::insertLineV( QRect _r, bool rev )
{
    view->kPresenterDoc()->insertLine( _r, view->getPen(),
                                       !rev ? view->getLineBegin() : view->getLineEnd(), !rev ? view->getLineEnd() : view->getLineBegin(),
                                       LT_VERT, diffx(), diffy() );
}

/*================================================================*/
void Page::insertLineD1( QRect _r, bool rev )
{
    view->kPresenterDoc()->insertLine( _r, view->getPen(),
                                       !rev ? view->getLineBegin() : view->getLineEnd(), !rev ? view->getLineEnd() : view->getLineBegin(),
                                       LT_LU_RD, diffx(), diffy() );
}

/*================================================================*/
void Page::insertLineD2( QRect _r, bool rev )
{
    view->kPresenterDoc()->insertLine( _r, view->getPen(),
                                       !rev ? view->getLineBegin() : view->getLineEnd(), !rev ? view->getLineEnd() : view->getLineBegin(),
                                       LT_LD_RU, diffx(), diffy() );
}

/*================================================================*/
void Page::insertRect( QRect _r )
{
    view->kPresenterDoc()->insertRectangle( _r, view->getPen(), view->getBrush(), view->getFillType(),
                                            view->getGColor1(), view->getGColor2(), view->getGType(), view->getRndX(), view->getRndY(),
                                            view->getGUnbalanced(), view->getGXFactor(), view->getGYFactor(), diffx(), diffy() );
}

/*================================================================*/
void Page::insertEllipse( QRect _r )
{
    view->kPresenterDoc()->insertCircleOrEllipse( _r, view->getPen(), view->getBrush(), view->getFillType(),
                                                  view->getGColor1(), view->getGColor2(),
                                                  view->getGType(), view->getGUnbalanced(), view->getGXFactor(), view->getGYFactor(),
                                                  diffx(), diffy() );
}

/*================================================================*/
void Page::insertPie( QRect _r )
{
    view->kPresenterDoc()->insertPie( _r, view->getPen(), view->getBrush(), view->getFillType(),
                                      view->getGColor1(), view->getGColor2(), view->getGType(),
                                      view->getPieType(), view->getPieAngle(), view->getPieLength(),
                                      view->getLineBegin(), view->getLineEnd(), view->getGUnbalanced(), view->getGXFactor(), view->getGYFactor(),
                                      diffx(), diffy() );
}

/*================================================================*/
void Page::insertAutoform( QRect _r, bool rev )
{
    rev = false;
    view->kPresenterDoc()->insertAutoform( _r, view->getPen(), view->getBrush(),
                                           !rev ? view->getLineBegin() : view->getLineEnd(), !rev ? view->getLineEnd() : view->getLineBegin(),
                                           view->getFillType(), view->getGColor1(), view->getGColor2(), view->getGType(),
                                           autoform, view->getGUnbalanced(), view->getGXFactor(), view->getGYFactor(),
                                           diffx(), diffy() );
}

/*================================================================*/
void Page::insertObject( QRect _r )
{
    view->kPresenterDoc()->insertObject( _r, partEntry, diffx(), diffy() );
}

/*================================================================*/
void Page::insertFreehand( const QPointArray &_pointArray )
{
    QRect rect = getDrawRect( _pointArray );

    int ox = rect.x() + diffx();
    int oy = rect.y() + diffy();

    unsigned int index = 0;

    QPointArray points( _pointArray );
    QPointArray tmpPoints;
    QPointArray::ConstIterator it;
    for ( it = points.begin(); it != points.end(); ++it ) {
        QPoint point = (*it);
        int tmpX = point.x() - ox + diffx();
        int tmpY = point.y() - oy + diffy();
        tmpPoints.putPoints( index, 1, tmpX,tmpY );
        ++index;
    }

    view->kPresenterDoc()->insertFreehand( tmpPoints, rect, view->getPen(), view->getLineBegin(),
                                           view->getLineEnd(), diffx(), diffy() );

    m_pointArray = QPointArray();
    m_indexPointArray = 0;
}

/*================================================================*/
void Page::insertPolyline( const QPointArray &_pointArray )
{
    QRect rect = getDrawRect( _pointArray );

    int ox = rect.x() + diffx();
    int oy = rect.y() + diffy();
    unsigned int index = 0;

    QPointArray points( _pointArray );
    QPointArray tmpPoints;
    QPointArray::ConstIterator it;
    for ( it = points.begin(); it != points.end(); ++it ) {
        QPoint point = (*it);
        int tmpX = point.x() - ox + diffx();
        int tmpY = point.y() - oy + diffy();
        tmpPoints.putPoints( index, 1, tmpX,tmpY );
        ++index;
    }

    view->kPresenterDoc()->insertPolyline( tmpPoints, rect, view->getPen(), view->getLineBegin(),
                                           view->getLineEnd(), diffx(), diffy() );

    m_pointArray = QPointArray();
    m_indexPointArray = 0;
}

/*================================================================*/
void Page::insertCubicBezierCurve( const QPointArray &_pointArray )
{
    QPointArray _points( _pointArray );
    QPointArray _allPoints;
    unsigned int pointCount = _points.count();
    QRect _rect;

    if ( pointCount == 2 ) { // line
        _rect = _points.boundingRect();
        _allPoints = _points;
    }
    else { // cubic bezier curve
        QPointArray tmpPointArray;
        unsigned int _tmpIndex = 0;
        unsigned int count = 0;
        while ( count < pointCount ) {
            if ( pointCount >= ( count + 4 ) ) { // for cubic bezier curve
                int _firstX = _points.at( count ).x();
                int _firstY = _points.at( count ).y();

                int _fourthX = _points.at( count + 1 ).x();
                int _fourthY = _points.at( count + 1 ).y();

                int _secondX = _points.at( count + 2 ).x();
                int _secondY = _points.at( count + 2 ).y();

                int _thirdX = _points.at( count + 3 ).x();
                int _thirdY = _points.at( count + 3 ).y();

                QPointArray _cubicBezierPoint;
                _cubicBezierPoint.putPoints( 0, 4, _firstX,_firstY, _secondX,_secondY, _thirdX,_thirdY, _fourthX,_fourthY );
                _cubicBezierPoint = _cubicBezierPoint.cubicBezier();

                QPointArray::ConstIterator it;
                for ( it = _cubicBezierPoint.begin(); it != _cubicBezierPoint.end(); ++it ) {
                    QPoint _point = (*it);
                    tmpPointArray.putPoints( _tmpIndex, 1, _point.x(), _point.y() );
                    ++_tmpIndex;
                }

                count += 4;
            }
            else { // for line
                int _x1 = _points.at( count ).x();
                int _y1 = _points.at( count ).y();

                int _x2 = _points.at( count + 1 ).x();
                int _y2 = _points.at( count + 1 ).y();

                tmpPointArray.putPoints( _tmpIndex, 2, _x1,_y1, _x2,_y2 );
                _tmpIndex += 2;
                count += 2;
            }
        }

        _rect = tmpPointArray.boundingRect();
        _allPoints = tmpPointArray;
    }

    int ox = _rect.x() + diffx();
    int oy = _rect.y() + diffy();
    unsigned int index = 0;

    QPointArray points( _pointArray );
    QPointArray tmpPoints;
    QPointArray::ConstIterator it;
    for ( it = points.begin(); it != points.end(); ++it ) {
        QPoint point = (*it);
        int tmpX = point.x() - ox + diffx();
        int tmpY = point.y() - oy + diffy();
        tmpPoints.putPoints( index, 1, tmpX,tmpY );
        ++index;
    }

    index = 0;
    QPointArray tmpAllPoints;
    for ( it = _allPoints.begin(); it != _allPoints.end(); ++it ) {
        QPoint point = (*it);
        int tmpX = point.x() - ox + diffx();
        int tmpY = point.y() - oy + diffy();
        tmpAllPoints.putPoints( index, 1, tmpX,tmpY );
        ++index;
    }

    if ( toolEditMode == INS_CUBICBEZIERCURVE ) {
        view->kPresenterDoc()->insertCubicBezierCurve( tmpPoints, tmpAllPoints, _rect, view->getPen(),
                                                       view->getLineBegin(), view->getLineEnd(), diffx(), diffy() );
    }
    else if ( toolEditMode == INS_QUADRICBEZIERCURVE ) {
        view->kPresenterDoc()->insertQuadricBezierCurve( tmpPoints, tmpAllPoints, _rect, view->getPen(),
                                                         view->getLineBegin(), view->getLineEnd(), diffx(), diffy() );
    }

    m_pointArray = QPointArray();
    m_indexPointArray = 0;
}

/*================================================================*/
void Page::insertPolygon( const QPointArray &_pointArray )
{
    QPointArray points( _pointArray );
    QRect rect = points.boundingRect();

    int ox = rect.x() + diffx();
    int oy = rect.y() + diffy();
    unsigned int index = 0;

    QPointArray tmpPoints;
    QPointArray::ConstIterator it;
    for ( it = points.begin(); it != points.end(); ++it ) {
        QPoint point = (*it);
        int tmpX = point.x() - ox + diffx();
        int tmpY = point.y() - oy + diffy();
        tmpPoints.putPoints( index, 1, tmpX,tmpY );
        ++index;
    }

    view->kPresenterDoc()->insertPolygon( tmpPoints, rect, view->getPen(), view->getBrush(), view->getFillType(),
                                          view->getGColor1(), view->getGColor2(), view->getGType(), view->getGUnbalanced(),
                                          view->getGXFactor(), view->getGYFactor(), diffx(), diffy(),
                                          view->getCheckConcavePolygon(), view->getCornersValue(), view->getSharpnessValue() );

    m_pointArray = QPointArray();
    m_indexPointArray = 0;
}

/*================================================================*/
void Page::setToolEditMode( ToolEditMode _m, bool updateView )
{
    //store m_pointArray if !m_pointArray.isNull()
    if(toolEditMode == INS_POLYLINE && !m_pointArray.isNull())
    {
        endDrawPolyline();
    }

    if ( ( toolEditMode == INS_CUBICBEZIERCURVE || toolEditMode == INS_QUADRICBEZIERCURVE ) && !m_pointArray.isNull() )
        endDrawCubicBezierCurve();


    exitEditMode();
    toolEditMode = _m;

    if ( toolEditMode == TEM_MOUSE ) {
        setCursor( arrowCursor );
        for ( int i = static_cast<int>( objectList()->count() ) - 1; i >= 0; i-- ) {
            KPObject *kpobject = objectList()->at( i );
            if ( kpobject->contains( QCursor::pos(), diffx(), diffy() ) ) {
                if ( kpobject->isSelected() )
                    setCursor( kpobject->getCursor( QCursor::pos(), diffx(), diffy(), modType ) );
                break;
            }
        }
    } else
        setCursor( crossCursor );

    if ( updateView )
        view->setTool( toolEditMode );
}


void Page::endDrawPolyline()
{
    m_drawPolyline = false;
    insertPolyline( m_pointArray );
    emit objectSelectedChanged();
    if ( toolEditMode != TEM_MOUSE && editMode )
        repaint( false );
    mousePressed = false;
    modType = MT_NONE;
    resizeObjNum = -1;
    ratio = 0.0;
    keepRatio = false;
}

void Page::endDrawCubicBezierCurve()
{
    m_drawCubicBezierCurve = false;
    m_oldCubicBezierPointArray = QPointArray();
    insertCubicBezierCurve( m_pointArray );
    emit objectSelectedChanged();
    if ( toolEditMode != TEM_MOUSE && editMode )
        repaint( false );
    mousePressed = false;
    modType = MT_NONE;
    resizeObjNum = -1;
    ratio = 0.0;
    keepRatio = false;
}

/*================================================================*/
void Page::selectNext()
{
    if ( objectList()->count() == 0 ) return;

    if ( view->kPresenterDoc()->numSelected() == 0 )
        objectList()->at( 0 )->setSelected( true );
    else {
        int i = objectList()->findRef( view->kPresenterDoc()->getSelectedObj() );
        if ( i < static_cast<int>( objectList()->count() ) - 1 ) {
            view->kPresenterDoc()->deSelectAllObj();
            objectList()->at( ++i )->setSelected( true );
        } else {
            view->kPresenterDoc()->deSelectAllObj();
            objectList()->at( 0 )->setSelected( true );
        }
    }
    if ( !QRect( diffx(), diffy(), width(), height() ).
         contains( view->kPresenterDoc()->getSelectedObj()->getBoundingRect( 0, 0 ) ) )
        view->makeRectVisible( view->kPresenterDoc()->getSelectedObj()->getBoundingRect( 0, 0 ) );
    _repaint( false );
}

/*================================================================*/
void Page::selectPrev()
{
    if ( objectList()->count() == 0 ) return;

    if ( view->kPresenterDoc()->numSelected() == 0 )
        objectList()->at( objectList()->count() - 1 )->setSelected( true );
    else {
        int i = objectList()->findRef( view->kPresenterDoc()->getSelectedObj() );
        if ( i > 0 ) {
            view->kPresenterDoc()->deSelectAllObj();
            objectList()->at( --i )->setSelected( true );
        } else {
            view->kPresenterDoc()->deSelectAllObj();
            objectList()->at( objectList()->count() - 1 )->setSelected( true );
        }
    }
    view->makeRectVisible( view->kPresenterDoc()->getSelectedObj()->getBoundingRect( 0, 0 ) );
    _repaint( false );
}

/*================================================================*/
void Page::dragEnterEvent( QDragEnterEvent *e )
{
    if ( m_currentTextObjectView )
    {
        m_currentTextObjectView->dragEnterEvent( e );
    }
    else if ( /*QTextDrag::canDecode( e ) ||*/
         QImageDrag::canDecode( e ) )
        e->accept();
    else
        e->ignore();
}

/*================================================================*/
void Page::dragLeaveEvent( QDragLeaveEvent *e )
{
    if(m_currentTextObjectView)
        m_currentTextObjectView->dragLeaveEvent( e );
}

/*================================================================*/
void Page::dragMoveEvent( QDragMoveEvent *e )
{
    if( m_currentTextObjectView)
    {
        m_currentTextObjectView->dragMoveEvent( e, QPoint() );
    }
    else if ( /*QTextDrag::canDecode( e ) ||*/
         QImageDrag::canDecode( e ) )
        e->accept();
    else
        e->ignore();
}

/*================================================================*/
void Page::dropEvent( QDropEvent *e )
{
    //disallow dropping objects outside the "page"
    if ( !view->kPresenterDoc()->getPageRect( view->getCurrPgNum()-1, diffx(), diffy(), _presFakt ).contains(e->pos()))
        return;

    KPresenterDoc *doc = view->kPresenterDoc();

    if ( QImageDrag::canDecode( e ) ) {
        setToolEditMode( TEM_MOUSE );
        deSelectAllObj();

        QImage pix;
        QImageDrag::decode( e, pix );

        KTempFile tmpFile;
        tmpFile.setAutoDelete(true);

	if( tmpFile.status() != 0 ) {
	    return;
	}
        tmpFile.close();

        pix.save( tmpFile.name(), "PNG" );
        QCursor c = cursor();
        setCursor( waitCursor );
        doc->insertPicture( tmpFile.name(), e->pos().x(), e->pos().y() +
                            ( currPgNum() - 1) * getPageRect( 0 ).height() );
        setCursor( c );

        e->accept();
    } else if ( QUriDrag::canDecode( e ) ) {
        setToolEditMode( TEM_MOUSE );
        deSelectAllObj();

        QStringList lst;
        QUriDrag::decodeToUnicodeUris( e, lst );

        QStringList::ConstIterator it = lst.begin();
        for ( ; it != lst.end(); ++it ) {
            KURL url( *it );

            QString filename;
            if ( !url.isLocalFile() ) {
                filename = QString::null;
                // #### todo download file
            } else {
                filename = url.path();
            }

            KMimeMagicResult *res = KMimeMagic::self()->findFileType( filename );

            if ( res && res->isValid() ) {
                QString mimetype = res->mimeType();
                if ( mimetype.contains( "image" ) ) {
                    QCursor c = cursor();
                    setCursor( waitCursor );
                    doc->insertPicture( filename, e->pos().x(), e->pos().y() +
                                        ( currPgNum() - 1) * getPageRect( 0 ).height() );
                    setCursor( c );
                } else if ( mimetype.contains( "text" ) ) {
                    QCursor c = cursor();
                    setCursor( waitCursor );
                    QFile f( filename );
                    QTextStream t( &f );
                    QString text = QString::null, tmp;

                    if ( f.open( IO_ReadOnly ) ) {
                        while ( !t.eof() ) {
                            tmp = t.readLine();
                            tmp += "\n";
                            text.append( tmp );
                        }
                        f.close();
                    }

                    doc->insertText( QRect( e->pos().x(), e->pos().y(), 250, 250 ),
                                     diffx(), diffy(), text, view );

                    setCursor( c );
                }
            }
        }
    }
    else if (m_currentTextObjectView)
    {
        m_currentTextObjectView->dropEvent( e );
    }
    else if ( QTextDrag::canDecode( e ) ) {
        setToolEditMode( TEM_MOUSE );
        deSelectAllObj();

        QString text;
        QTextDrag::decode( e, text );

        doc->insertText( QRect( e->pos().x(), e->pos().y(), 250, 250 ),
                         diffx(), diffy(), text, view );
        e->accept();
    } else
        e->ignore();

}

/*================================================================*/
void Page::slotGotoPage()
{
    setCursor( blankCursor );
    int pg = currPresPage;
    pg = KPGotoPage::gotoPage( view->kPresenterDoc(), _presFakt, slideList, pg, this );
    gotoPage( pg );

    if ( !spManualSwitch() ) {
        view->setCurrentTimer( 1 );
        setNextPageTimer( true );
    }
}

/*================================================================*/
void Page::gotoPage( int pg )
{
    if ( pg != static_cast<int>( currPresPage ) ) {
        currPresPage = pg;
        //kdDebug(33001) << "Page::gotoPage currPresPage=" << currPresPage << endl;
        slideListIterator = slideList.find( currPresPage );
        editMode = false;
        drawMode = false;
        presStepList = view->kPresenterDoc()->reorderPage( currPresPage, diffx(), diffy(), _presFakt );
        currPresStep = *presStepList.begin();
        subPresStep = 0;

        int yo = view->kPresenterDoc()->getPageRect( 0, 0, 0, presFakt(), false ).height() * ( pg - 1 );
        // ## shouldn't this center the slide if it's smaller ? (see KPresenterView::startScreenPresentation)
        view->setDiffY( yo );
        //kdDebug(33001) << "Page::gotoPage :   setDiffY " << yo << endl;
        resize( QApplication::desktop()->width(), QApplication::desktop()->height() );
        repaint( false );
        setFocus();
        view->refreshPageButton();
    }
}

/*================================================================*/
KPTextObject* Page::kpTxtObj()
{
    return ( ( editNum != -1 && objectList()->at( editNum )->getType() == OT_TEXT ) ?
             dynamic_cast<KPTextObject*>( objectList()->at( editNum ) ) : 0 );
    // ### return m_currentTextObjectView->kpTextObject()
}

/*================================================================*/
void Page::deleteObjs()
{
    view->kPresenterDoc()->deleteObjs();
    setToolEditMode( toolEditMode );
}

/*================================================================*/
void Page::rotateObjs()
{
    view->extraRotate();
    setToolEditMode( toolEditMode );
}

/*================================================================*/
void Page::shadowObjs()
{
    view->extraShadow();
    setToolEditMode( toolEditMode );
}

/*================================================================*/
void Page::enterEvent( QEvent *e )
{
    view->setRulerMousePos( ( ( QMouseEvent* )e )->x(), ( ( QMouseEvent* )e )->y() );
    view->setRulerMouseShow( true );
}

/*================================================================*/
void Page::leaveEvent( QEvent * /*e*/ )
{
    view->setRulerMouseShow( false );
}

/*================================================================*/
QPtrList<KPBackGround> *Page::backgroundList()
{
    return view->kPresenterDoc()->backgroundList();
}
const QPtrList<KPBackGround> *Page::backgroundList() const
{
    return view->kPresenterDoc()->backgroundList();
}

/*================================================================*/
QPtrList<KPObject> *Page::objectList()
{
    return view->kPresenterDoc()->objectList();
}
const QPtrList<KPObject> *Page::objectList() const
{
    return view->kPresenterDoc()->objectList();
}

/*================================================================*/
unsigned int Page::objNums() const
{
    return view->kPresenterDoc()->objNums();
}

/*================================================================*/
unsigned int Page::currPgNum() const
{
    return view->getCurrPgNum();
}

/*================================================================*/
unsigned int Page::rastX() const
{
    return view->kPresenterDoc()->rastX();
}

/*================================================================*/
unsigned int Page::rastY() const
{
    return view->kPresenterDoc()->rastY();
}

/*================================================================*/
QColor Page::txtBackCol() const
{
    return view->kPresenterDoc()->txtBackCol();
}

/*================================================================*/
bool Page::spInfinitLoop() const
{
    return view->kPresenterDoc()->spInfinitLoop();
}

/*================================================================*/
bool Page::spManualSwitch() const
{
    return view->kPresenterDoc()->spManualSwitch();
}

/*================================================================*/
QRect Page::getPageRect( unsigned int p, float fakt, bool decBorders )
{
    return view->kPresenterDoc()->getPageRect( p, diffx(), diffy(), fakt, decBorders );
}

/*================================================================*/
unsigned int Page::pageNums()
{
    return view->kPresenterDoc()->getPageNums();
}

/*================================================================*/
int Page::getPageOfObj( int i, float fakt )
{
    // This works much better with 0,0 than with diffx, diffy
    // (the objects don't appear in the view-current-page-fullscreen feature IIRC)
    // See comment in KPresenterDoc::getPageOfObj. Passing diffx/diffy
    // seems useless to me, but if it changes something, then it's not
    // that useless... This needs to be cleaned up :)  (DF)
    return view->kPresenterDoc()->getPageOfObj( i, 0 /*diffx()*/, 0 /*diffy()*/, fakt );
}

/*================================================================*/
float Page::objSpeedFakt()
{
    /*
      Used to be 0(slow)->70, 1(medium)->50, 2(fast)->30.
      It's now 0->75, 1->50, 2->37, etc. That's the reason for this strange formula :)
     */
    return 150.0 / static_cast<float>( view->kPresenterDoc()->getPresSpeed() + 2 );
    //return ObjSpeed[ static_cast<int>( view->kPresenterDoc()->getPresSpeed() ) ];
}

/*================================================================*/
float Page::pageSpeedFakt()
{
    /*
      Used to be 0(slow)->8, 1(medium)->16, 2(fast)->32.
      It's now 0->10, 1->20, 2->30, 3->40, 4->50......
     */
    return 10.0 * ( view->kPresenterDoc()->getPresSpeed() + 1 );
    //return PageSpeed[ static_cast<int>( view->kPresenterDoc()->getPresSpeed() ) ];
}

/*================================================================*/
void Page::_repaint( bool /*erase*/ )
{
    view->kPresenterDoc()->repaint( false );
}

/*================================================================*/
void Page::_repaint( QRect r )
{
    view->kPresenterDoc()->repaint( r );
}

/*================================================================*/
void Page::_repaint( KPObject *o )
{
    view->kPresenterDoc()->repaint( o );
}


/*================================================================*/
void Page::slotExitPres()
{
    view->screenStop();
}


/*================================================================*/
void Page::drawingMode()
{
    if(!presMenu->isItemChecked ( PM_DM ))
    {
        presMenu->setItemChecked( PM_DM, true );
        presMenu->setItemChecked( PM_SM, false );
        drawMode = true;
        setCursor( arrowCursor );
    }
}

/*================================================================*/
void Page::switchingMode()
{
    if(!presMenu->isItemChecked ( PM_SM ))
    {
        presMenu->setItemChecked( PM_DM, false );
        presMenu->setItemChecked( PM_SM, true );
        drawMode = false; setCursor( blankCursor );

        if ( !spManualSwitch() )
            view->autoScreenPresIntervalTimer();
    }
}

/*================================================================*/
bool Page::calcRatio( int &dx, int &dy, KPObject *kpobject, double ratio ) const
{
    if ( abs( dy ) > abs( dx ) )
        dx = static_cast<int>( static_cast<double>( dy ) * ratio );
    else
        dy = static_cast<int>( static_cast<double>( dx ) / ratio );
    if ( kpobject->getSize().width() + dx < 20 ||
         kpobject->getSize().height() + dy < 20 )
        return false;
    return true;
}

/*================================================================*/
void Page::exitEditMode()
{
    if ( editNum != -1 ) {
        KPObject *kpobject = objectList()->at( editNum );
        editNum = -1;
        if ( kpobject->getType() == OT_TEXT ) {
            if(m_currentTextObjectView)
            {
                m_currentTextObjectView->clearSelection();
                //hide cursor when we desactivate textObjectView
                m_currentTextObjectView->drawCursor( false );
                m_currentTextObjectView->terminate();
                delete m_currentTextObjectView;
                m_currentTextObjectView=0L;
            }
            // Title of slide may have changed
            emit updateSideBarItem( currPgNum()-1 );
            emit objectSelectedChanged();

        } else if ( kpobject->getType() == OT_PART ) {
            kpobject->deactivate();
            _repaint( kpobject );
            return;
        }
    }
}

/*================================================================*/
bool Page::getPixmapOrigAndCurrentSize( KPPixmapObject *&obj, QSize *origSize, QSize *currentSize )
{
    obj = 0;
    KPObject *kpobject = 0;
    for ( int i = 0; i < static_cast<int>( objectList()->count() ); i++ ) {
        kpobject = objectList()->at( i );
        if ( kpobject->isSelected() && kpobject->getType() == OT_PICTURE ) {
            *currentSize = kpobject->getSize();

            KPPixmapObject *o = (KPPixmapObject*)kpobject;
            /*
            QImage *img = view->kPresenterDoc()->getPixmapCollection()->
                          getPixmapDataCollection().findPixmapData( o->key.dataKey );
            if ( img ) {
                obj = o;
                return img->size();
            }
            */
            KPImage img = view->kPresenterDoc()->getImageCollection()->
                          findImage( o->getKey() );
            if ( !img.isNull() )
            {
                obj = o;
                *origSize = img.size();
                return true;
            }
        }
    }

    *origSize = QSize( -1, -1 );
    *currentSize = QSize( -1, -1 );

    return false;
}

/*================================================================*/
void Page::picViewOrig640x480()
{
  picViewOrigHelper(640, 480);
}

/*================================================================*/
void Page::picViewOrig800x600()
{
  picViewOrigHelper(800, 600);
}

/*================================================================*/
void Page::picViewOrig1024x768()
{
  picViewOrigHelper(1024, 768);
}

/*================================================================*/
void Page::picViewOrig1280x1024()
{
  picViewOrigHelper(1280, 1024);
}

/*================================================================*/
void Page::picViewOrig1600x1200()
{
  picViewOrigHelper(1600, 1200);
}

void Page::picViewOrigHelper(int x, int y)
{
  KPPixmapObject *obj = 0;

  QSize origSize;
  QSize currentSize;
  if ( !getPixmapOrigAndCurrentSize( obj, &origSize, &currentSize ) || !obj )
      return;

  QSize pgSize = view->kPresenterDoc()->getPageRect( 0, 0, 0 ).size();
  QSize presSize( x, y );

  scalePixmapToBeOrigIn( origSize, currentSize, pgSize, presSize, obj );
}

/*================================================================*/
void Page::picViewOrigFactor()
{
}

/*================================================================*/
void Page::scalePixmapToBeOrigIn( const QSize &/*origSize*/, const QSize &currentSize,
                                  const QSize &pgSize, const QSize &presSize, KPPixmapObject *obj )
{
    double faktX = (double)presSize.width() / (double)QApplication::desktop()->width();
    double faktY = (double)presSize.height() / (double)QApplication::desktop()->height();
    int w = (int)( (double)pgSize.width() * faktX );
    int h = (int)( (double)pgSize.height() * faktY );

    ResizeCmd *resizeCmd = new ResizeCmd( i18n( "Scale Picture to be shown 1:1 in presentation mode" ),
                                          QPoint( 0, 0 ), QSize( w - currentSize.width(), h - currentSize.height() ),
                                          obj, view->kPresenterDoc() );
    resizeCmd->execute();
    view->kPresenterDoc()->addCommand( resizeCmd );
}

void Page::setTextBackground( KPTextObject *obj )
{
    QPixmap pix( getPageRect( 0 ).size() );
    QPainter painter( &pix );
    backgroundList()->at( view->getCurrPgNum() - 1 )->draw( &painter, QPoint( 0, 0 ), FALSE );
    QPixmap bpix( obj->getSize() );
    bitBlt( &bpix, 0, 0, &pix, obj->getOrig().x(), obj->getOrig().y() -
	    getPageRect( 0 ).height() * ( view->getCurrPgNum() - 1 ), bpix.width(), bpix.height() );
    QBrush b( white, bpix );
#if 0
    QPalette pal( obj->textObjectView()->palette() );
    pal.setBrush( QColorGroup::Base, b );
    obj->textObjectView()->setPalette( pal );
#endif
}

QValueList<int> Page::pages(const QString &range) {

    if(range.isEmpty())
        return QValueList<int> ();
    QValueList<int> list;
    int start=-1;
    int end=range.find(',');
    bool ok=true;
    QString tmp;
    while(end!=-1 && start!=end && ok) {
        tmp=range.mid(start+1, end-start-1);
        ok=pagesHelper(tmp, list);
        start=range.find(',', end);
        end=range.find(',', start+1);
    }
    pagesHelper(range.mid(start+1), list);
    return list;
}

bool Page::pagesHelper(const QString &chunk, QValueList<int> &list) {

    bool ok=true;
    int mid=chunk.find('-');
    if(mid!=-1) {
        int start=chunk.left(mid).toInt(&ok);
        int end=chunk.mid(mid+1).toInt(&ok);
        while(ok && start<=end)
            list.append(start++);
    }
    else
        list.append(chunk.toInt(&ok));
    return ok;
}

void Page::moveObject( int x, int y, bool key )
{
    KPObject *kpobject;
    QPtrList<KPObject> _objects;
    QPainter p;

    _objects.setAutoDelete( false );

    if ( (int)objectList()->count() - 1 >= 0 ) {
        for ( int i = static_cast<int>( objectList()->count() ) - 1; i >= 0; i-- ) {
            kpobject = objectList()->at( i );
            if ( kpobject->isSelected() ) {
                p.begin( this );
                kpobject->setMove( true );
                kpobject->draw( &p, diffx(), diffy() );
                kpobject->moveBy( QPoint( x, y ) );
                kpobject->draw( &p, diffx(), diffy() );
                p.end();

                kpobject->setMove( false );
                _objects.append( kpobject );
                _repaint( QRect( kpobject->getBoundingRect( 0, 0 ).x() + ( -1*x ),
                                 kpobject->getBoundingRect( 0, 0 ).y() + ( -1*y ),
                                 kpobject->getBoundingRect( 0, 0 ).width(),
                                 kpobject->getBoundingRect( 0, 0 ).height() ) );
                _repaint( kpobject );
            }
        }
    }

    if ( key ) {
        MoveByCmd *moveByCmd = new MoveByCmd( i18n( "Move object(s)" ),
                                              QPoint( x, y ),
                                              _objects, view->kPresenterDoc() );
        view->kPresenterDoc()->addCommand( moveByCmd );
    }
}

void Page::resizeObject( ModifyType _modType, int _dx, int _dy )
{
    int dx = _dx;
    int dy = _dy;

    KPObject *kpobject;

    kpobject = objectList()->at( resizeObjNum );
    kpobject->setMove( false );

    QPainter p;
    p.begin( this );

    kpobject->draw( &p, diffx(), diffy() );

    switch ( _modType ) {
        case MT_RESIZE_LU: {
            if ( keepRatio && ratio != 0.0 ) {
                if ( !calcRatio( dx, dy, kpobject, ratio ) )
                    break;
            }
            kpobject->moveBy( QPoint( dx, dy ) );
            kpobject->resizeBy( -dx, -dy );
        } break;
        case MT_RESIZE_LF: {
            dy = 0;
            if ( keepRatio && ratio != 0.0 ) {
                if ( !calcRatio( dx, dy, kpobject, ratio ) )
                    break;
            }
            kpobject->moveBy( QPoint( dx, 0 ) );
            kpobject->resizeBy( -dx, -dy );
        } break;
        case MT_RESIZE_LD: {
            if ( keepRatio && ratio != 0.0 )
                break;
            kpobject->moveBy( QPoint( dx, 0 ) );
            kpobject->resizeBy( -dx, dy );
        } break;
        case MT_RESIZE_RU: {
            if ( keepRatio && ratio != 0.0 )
                break;
            kpobject->moveBy( QPoint( 0, dy ) );
            kpobject->resizeBy( dx, -dy );
        } break;
        case MT_RESIZE_RT: {
            dy = 0;
            if ( keepRatio && ratio != 0.0 ) {
                if ( !calcRatio( dx, dy, kpobject, ratio ) )
                    break;
            }
            kpobject->resizeBy( dx, dy );
        } break;
        case MT_RESIZE_RD: {
            if ( keepRatio && ratio != 0.0 ) {
                if ( !calcRatio( dx, dy, kpobject, ratio ) )
                    break;
            }
            kpobject->resizeBy( QSize( dx, dy ) );
        } break;
        case MT_RESIZE_UP: {
            dx = 0;
            if ( keepRatio && ratio != 0.0 ) {
                if ( !calcRatio( dx, dy, kpobject, ratio ) )
                    break;
            }
            kpobject->moveBy( QPoint( 0, dy ) );
            kpobject->resizeBy( -dx, -dy );
	} break;
        case MT_RESIZE_DN: {
            dx = 0;
            if ( keepRatio && ratio != 0.0 ) {
                if ( !calcRatio( dx, dy, kpobject, ratio ) )
                    break;
            }
            kpobject->resizeBy( dx, dy );
        } break;
	default: break;
    }

    kpobject->draw( &p, diffx(), diffy() );
    p.end();

    _repaint( oldBoundingRect );
    _repaint( kpobject );

    oldBoundingRect = kpobject->getBoundingRect( 0, 0 );
}

void Page::raiseObject()
{
    if ( selectedObjectPosition == -1 ) {
        KPObject *kpobject;

        if ( view->kPresenterDoc()->numSelected() == 1 ) { // execute this if user selected is one object.
            for ( kpobject = objectList()->first(); kpobject != 0; kpobject = objectList()->next() ) {
                if ( kpobject->isSelected() ) {
                    selectedObjectPosition = objectList()->at();
                    objectList()->remove( selectedObjectPosition );
                    objectList()->append( kpobject );
                    break;
                }
            }
        }
        else
            selectedObjectPosition = -1;
    }
}

void Page::lowerObject()
{
    KPObject *kpobject;

    for ( kpobject = objectList()->first(); kpobject != 0; kpobject = objectList()->next() ) {
        if ( kpobject->isSelected() ) {
            objectList()->remove( objectList()->at() );
            objectList()->insert( selectedObjectPosition, kpobject );
            break;
        }
    }
}

void Page::playSound( const QString &soundFileName )
{
    delete soundPlayer;
    soundPlayer = new KPresenterSoundPlayer( soundFileName );
    soundPlayer->play();
}

void Page::stopSound()
{
    if ( soundPlayer ) {
        soundPlayer->stop();
        delete soundPlayer;
        soundPlayer = 0;
    }
}

void Page::setXimPosition( int x, int y, int w, int h, QFont *f )
{
    QWidget::setMicroFocusHint( x - view->getDiffX(), y - view->getDiffY(), w, h, true, f );
}

QRect Page::getDrawRect( const QPointArray &_points )
{
    int maxX = 0, maxY = 0;
    int minX = 0, minY = 0;
    int tmpX = 0, tmpY = 0;
    bool first = true;

    QPointArray points( _points );
    QPointArray::ConstIterator it;
    for ( it = points.begin(); it != points.end(); ++it ) {
        QPoint point = (*it);
        tmpX = point.x();
        tmpY = point.y();

        if ( first ) {
            maxX = tmpX;
            maxY = tmpY;
            minX = tmpX;
            minY = tmpY;

            first = false;
        }

        if ( maxX < tmpX )
            maxX = tmpX;
        if ( maxY < tmpY )
            maxY = tmpY;
        if ( minX > tmpX )
            minX = tmpX;
        if ( minY > tmpY )
            minY = tmpY;
    }

    QPoint topLeft = QPoint( minX, minY );
    QPoint bottomRight = QPoint( maxX, maxY );
    QRect rect = QRect( topLeft, bottomRight );

    return rect;
}

void Page::terminateEditing( KPTextObject *textObj )
{
    if ( m_currentTextObjectView && m_currentTextObjectView->kpTextObject() == textObj )
    {
        m_currentTextObjectView->terminate();
        delete m_currentTextObjectView;
        m_currentTextObjectView = 0L;
        editNum = -1 ;
    }
}

void Page::drawCubicBezierCurve( int _dx, int _dy )
{
    QPoint oldEndPoint = m_dragEndPoint;
    m_dragEndPoint = QPoint( _dx, _dy );

    unsigned int pointCount = m_pointArray.count();

    QPainter p( this );

    if ( !m_drawLineWithCubicBezierCurve ) {
        QPen _pen = QPen( Qt::black, 1, Qt::DashLine );
        p.setPen( _pen );
        p.setBrush( Qt::NoBrush );
        p.setRasterOp( Qt::NotROP );

        p.save();
        double _angle = getAngle( oldEndPoint, m_dragStartPoint );
        drawFigure( L_SQUARE, &p, oldEndPoint, _pen.color(), _pen.width(), _angle ); // erase old figure
        p.restore();

        p.drawLine( m_dragStartPoint, oldEndPoint ); // erase old line

        int p_x = m_dragStartPoint.x() * 2 - oldEndPoint.x();
        int p_y = m_dragStartPoint.y() * 2 - oldEndPoint.y();
        m_dragSymmetricEndPoint = QPoint( p_x, p_y );

        p.save();
        _angle = getAngle( m_dragSymmetricEndPoint, m_dragStartPoint );
        drawFigure( L_SQUARE, &p, m_dragSymmetricEndPoint, _pen.color(), _pen.width(), _angle );  // erase old figure
        p.restore();

        p.drawLine( m_dragStartPoint, m_dragSymmetricEndPoint );  // erase old line


        p.save();
        _angle = getAngle( m_dragEndPoint, m_dragStartPoint );
        drawFigure( L_SQUARE, &p, m_dragEndPoint, _pen.color(), _pen.width(), _angle ); // draw new figure
        p.restore();

        p.drawLine( m_dragStartPoint, m_dragEndPoint );  // draw new line

        p_x = m_dragStartPoint.x() * 2 - m_dragEndPoint.x();
        p_y = m_dragStartPoint.y() * 2 - m_dragEndPoint.y();
        m_dragSymmetricEndPoint = QPoint( p_x, p_y );

        p.save();
        _angle = getAngle( m_dragSymmetricEndPoint, m_dragStartPoint );
        drawFigure( L_SQUARE, &p, m_dragSymmetricEndPoint, _pen.color(), _pen.width(), _angle ); // draw new figure
        p.restore();

        p.drawLine( m_dragStartPoint, m_dragSymmetricEndPoint );  // draw new line
    }
    else if ( m_drawLineWithCubicBezierCurve ) {
        p.setPen( QPen( Qt::black, 1, Qt::SolidLine ) );
        p.setBrush( Qt::NoBrush );
        p.setRasterOp( Qt::NotROP );

        QPoint startPoint( m_pointArray.at( m_indexPointArray - 1 ).x(),
                           m_pointArray.at( m_indexPointArray - 1 ).y() );

        p.drawLine( startPoint, oldEndPoint );  // erase old line

        p.drawLine( startPoint, m_dragEndPoint );  // draw new line
    }

    if ( !m_drawLineWithCubicBezierCurve && ( ( pointCount % 2 ) == 0 ) ) {
        p.save();

        p.setPen( QPen( Qt::black, 1, Qt::SolidLine ) );
        p.setBrush( Qt::NoBrush );
        p.setRasterOp( Qt::NotROP );

        p.drawCubicBezier( m_oldCubicBezierPointArray );  // erase old cubic bezier curve

        int _firstX = m_pointArray.at( m_indexPointArray - 2 ).x();
        int _firstY = m_pointArray.at( m_indexPointArray - 2 ).y();

        int _fourthX = m_pointArray.at( m_indexPointArray - 1 ).x();
        int _fourthY = m_pointArray.at( m_indexPointArray - 1 ).y();

        int _midpointX = (_firstX + _fourthX ) / 2;
        int _midpointY = (_firstY + _fourthY ) / 2;
        int _diffX = _fourthX - _midpointX;
        int _diffY = _fourthY - _midpointY;

        int _secondX = m_dragEndPoint.x() - _diffX;
        int _secondY = m_dragEndPoint.y() - _diffY;
        m_CubicBezierSecondPoint = QPoint( _secondX, _secondY );

        int _thirdX = m_dragSymmetricEndPoint.x() - _diffX;
        int _thirdY = m_dragSymmetricEndPoint.y() - _diffY;
        m_CubicBezierThirdPoint = QPoint( _thirdX, _thirdY );

        if ( toolEditMode == INS_QUADRICBEZIERCURVE ) {
            _secondX = _thirdX;
            _secondY = _thirdY;
            m_CubicBezierSecondPoint = QPoint( _secondX, _secondY );
        }

        QPointArray points;
        points.putPoints( 0, 4, _firstX,_firstY, _secondX,_secondY, _thirdX,_thirdY, _fourthX,_fourthY );
        p.drawCubicBezier( points );  // draw new cubic bezier curve

        m_oldCubicBezierPointArray = points;

        p.restore();
    }

    p.end();
}

/*===================== get angle ================================*/
double Page::getAngle( QPoint p1, QPoint p2 )
{
    double _angle = 0.0;

    if ( p1.x() == p2.x() ) {
        if ( p1.y() < p2.y() )
            _angle = 270.0;
        else
            _angle = 90.0;
    }
    else {
        double x1, x2, y1, y2;

        if ( p1.x() <= p2.x() ) {
            x1 = p1.x(); y1 = p1.y();
            x2 = p2.x(); y2 = p2.y();
        }
        else {
            x2 = p1.x(); y2 = p1.y();
            x1 = p2.x(); y1 = p2.y();
        }

        double m = -( y2 - y1 ) / ( x2 - x1 );
        _angle = atan( m ) * RAD_FACTOR;

        if ( p1.x() < p2.x() )
            _angle = 180.0 - _angle;
        else
            _angle = -_angle;
    }

    return _angle;
}

void Page::drawPolygon( const QPoint &startPoint, const QPoint &endPoint )
{
    bool checkConcavePolygon = view->getCheckConcavePolygon();
    int cornersValue = view->getCornersValue();
    int sharpnessValue = view->getSharpnessValue();

    QPainter p;
    p.begin( this );
    p.setPen( QPen( Qt::black, 1, Qt::SolidLine ) );
    p.setRasterOp( Qt::NotROP );

    double angle = 2 * M_PI / cornersValue;
    double dx = ::fabs( startPoint.x () - endPoint.x () );
    double dy =  ::fabs( startPoint.y () - endPoint.y () );
    double radius = (dx > dy ? dx / 2.0 : dy / 2.0);

    double xoff = startPoint.x() + ( startPoint.x() < endPoint.x() ? radius : -radius );
    double yoff = startPoint.y() + ( startPoint.y() < endPoint.y() ? radius : -radius );

    QPointArray points( checkConcavePolygon ? cornersValue * 2 : cornersValue );
    points.setPoint( 0, (int)xoff, (int)( -radius + yoff ) );

    if ( checkConcavePolygon ) {
        angle = angle / 2.0;
        double a = angle;
        double r = radius - ( sharpnessValue / 100.0 * radius );
        for ( int i = 1; i < cornersValue * 2; ++i ) {
            double xp, yp;
            if ( i % 2 ) {
                xp =  r * sin( a );
                yp = -r * cos( a );
            }
            else {
                xp = radius * sin( a );
                yp = -radius * cos( a );
            }
            a += angle;
            points.setPoint( i, (int)( xp + xoff ), (int)( yp + yoff ) );
        }
    }
    else {
        double a = angle;
        for ( int i = 1; i < cornersValue; ++i ) {
            double xp = radius * sin( a );
            double yp = -radius * cos( a );
            a += angle;
            points.setPoint( i, (int)( xp + xoff ), (int)( yp + yoff ) );
        }
    }
    p.drawPolygon( points );
    p.end();

    m_pointArray = points;
}


QPtrList<KoTextObject> Page::objectText()
{
    QPtrList<KoTextObject>lst;
    QPtrList<KPObject> *listObj(objectList());
    for ( unsigned int i = 0; i < listObj->count(); i++ ) {
        if(listObj->at( i )->getType() == OT_TEXT)
            lst.append(dynamic_cast<KPTextObject*>(listObj->at( i ))->textObject());
    }
    return lst;
}

bool Page::oneObjectTextExist()
{
    QPtrList<KPObject> *listObj(objectList());
    for ( unsigned int i = 0; i < listObj->count(); i++ )
    {
        if(listObj->at( i )->getType() == OT_TEXT)
            return true;
    }
    return false;
}

#include <page.moc>
