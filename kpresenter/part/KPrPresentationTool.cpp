/* This file is part of the KDE project
 * Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KPrPresentationTool.h"

#include <QtGui/QWidget>

#include <QtGui/QBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QKeyEvent>
#include <QtGui/QStackedLayout>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QPixmap>
#include <QtGui/QPicture>

#include <KoShape.h>
#include <KoShapeManager.h>
#include <KoPointerEvent.h>
#include <KoEventAction.h>
#include <KoPACanvas.h>

#include "KPrViewModePresentation.h"


KPrPresentationTool::KPrPresentationTool( KPrViewModePresentation & viewMode )
: KoTool( viewMode.canvas() )
, m_viewMode( viewMode )
{    
    // tool box
    m_frame = new QFrame( m_viewMode.canvas() );
    
    QVBoxLayout *frameLayout = new QVBoxLayout();
    presentationToolWidget = new KPrPresentationToolWidget(m_viewMode.canvas());
    frameLayout->addWidget( presentationToolWidget, 0, Qt::AlignLeft | Qt::AlignBottom  );
    m_frame->setLayout( frameLayout );
    
    m_frame->show();
    m_frame->setVisible(false);
    
    // Connections of button clicked to slots
    connect( presentationToolWidget->presentationToolUi().penButton, SIGNAL( clicked() ), this, SLOT( drawOnPresentation() ) );
    connect( presentationToolWidget->presentationToolUi().highLightButton, SIGNAL( clicked() ), this, SLOT( highLightPresentation() ) );
}

KPrPresentationTool::~KPrPresentationTool()
{
}

bool KPrPresentationTool::wantsAutoScroll()
{
    return false;
}
void KPrPresentationTool::paint( QPainter &painter, const KoViewConverter &converter )
{
}

void KPrPresentationTool::mousePressEvent( KoPointerEvent *event )
{
    if ( event->button() & Qt::LeftButton ) {
        event->accept();
        finishEventActions();
        KoShape * shapeClicked = m_canvas->shapeManager()->shapeAt( event->point );
        if (shapeClicked) {
            m_eventActions = shapeClicked->eventActions();
            if ( m_eventActions.size() ) {
                foreach ( KoEventAction * eventAction, m_eventActions ) {
                    eventAction->execute( this );
                }
                // don't do next step if a action was executed
                return;
            }
        }
        m_viewMode.navigate( KPrAnimationDirector::NextStep );
    }
    else if ( event->button() & Qt::RightButton ) {
        event->accept();
        finishEventActions();
        m_viewMode.navigate( KPrAnimationDirector::PreviousStep );
    }
}

void KPrPresentationTool::mouseDoubleClickEvent( KoPointerEvent *event )
{
}

void KPrPresentationTool::mouseMoveEvent( KoPointerEvent *event )
{
}

void KPrPresentationTool::mouseReleaseEvent( KoPointerEvent *event )
{
}

void KPrPresentationTool::keyPressEvent( QKeyEvent *event )
{
    finishEventActions();
    event->accept();

    switch ( event->key() )
    {
        case Qt::Key_Escape:
            m_viewMode.activateSavedViewMode();
            break;
        case Qt::Key_Home:
            m_viewMode.navigate( KPrAnimationDirector::FirstPage );
            break;
        case Qt::Key_Up:
        case Qt::Key_PageUp:
            m_viewMode.navigate( KPrAnimationDirector::PreviousPage );
            break;
        case Qt::Key_Backspace:
        case Qt::Key_Left:
            m_viewMode.navigate( KPrAnimationDirector::PreviousStep );
            break;
        case Qt::Key_Right:
        case Qt::Key_Space:
            m_viewMode.navigate( KPrAnimationDirector::NextStep );
            break;
        case Qt::Key_Down:
        case Qt::Key_PageDown:
            m_viewMode.navigate( KPrAnimationDirector::NextPage );
            break;
        case Qt::Key_End:
            m_viewMode.navigate( KPrAnimationDirector::LastPage );
            break;
        default:
            event->ignore();
            break;
    }
}

void KPrPresentationTool::keyReleaseEvent( QKeyEvent *event )
{
}

void KPrPresentationTool::wheelEvent( KoPointerEvent * event )
{
}

void KPrPresentationTool::activate( bool temporary )
{
}

void KPrPresentationTool::deactivate()
{
    finishEventActions();
}

void KPrPresentationTool::finishEventActions()
{
    foreach ( KoEventAction * eventAction, m_eventActions ) {
        eventAction->finish( this );
    }
}

// SLOTS
void KPrPresentationTool::highLightPresentation()
{
    // create the high light
    //delete m_blackBackgroundframe;
    //delete m_blackBackgroundwidget;
    QSize size = m_viewMode.canvas()->size();
    QPixmap newPage( size );
    if(newPage.isNull())
	return;
	
    if ( m_blackBackgroundVisibility ) {
	m_blackBackgroundVisibility = false;
	delete m_blackBackgroundwidget;
    } else {
	m_blackBackgroundVisibility = true;
	m_blackBackgroundwidget = new KPrPresentationHighlightWidget( m_viewMode.canvas() );
	m_blackBackgroundwidget->show();
    }

    /*m_blackBackgroundframe= new QFrame(m_blackBackgroundwidget);
    QVBoxLayout *frameLayout2 = new QVBoxLayout();
    QLabel *label = new QLabel();
    
    label->setPixmap( newPage );
    
    frameLayout2->addWidget( label, 0, Qt::AlignCenter );
    m_blackBackgroundframe->setLayout( frameLayout2 );
    m_blackBackgroundframe->move( -4,-4 );

    // change the visibility
    if ( m_blackBackgroundVisibility )
    {
	m_blackBackgroundVisibility = false;
	m_blackBackgroundwidget->setVisible( false );
    }
    else
    {
	m_blackBackgroundVisibility = true;
	m_blackBackgroundwidget->setVisible( true );
    }*/
    
    // tool box
    delete m_frame;
    m_frame = new QFrame( m_viewMode.canvas() );

    QVBoxLayout *frameLayout = new QVBoxLayout();
    presentationToolWidget = new KPrPresentationToolWidget( m_viewMode.canvas() );
    frameLayout->addWidget( presentationToolWidget, 0, Qt::AlignLeft | Qt::AlignBottom  );
    m_frame->setLayout( frameLayout );
    m_frame->resize( size );
    m_frame->show();
    
    // Connections of button clicked to slots
    connect( presentationToolWidget->presentationToolUi().penButton, SIGNAL( clicked() ), this, SLOT( drawOnPresentation() ) );
    connect( presentationToolWidget->presentationToolUi().highLightButton, SIGNAL( clicked() ), this, SLOT( highLightPresentation() ) );

}

void KPrPresentationTool::drawOnPresentation()
{
    presentationToolWidget->presentationToolUi().penButton->setText("test...");
}

// get the acces on m_frame
QFrame *KPrPresentationTool::m_frameToolPresentation()
{
    return m_frame;
}

// get the acces on m_blackBackgroundwidget
QWidget *KPrPresentationTool::m_blackBackgroundPresentation()
{
    if ( m_blackBackgroundwidget )
	return m_blackBackgroundwidget;
}

void KPrPresentationTool::setBlackBackgroundVisibility(bool b)
{
    m_blackBackgroundVisibility = b;
}

bool KPrPresentationTool::getBlackBackgroundVisibility()
{
    return m_blackBackgroundVisibility;
}
#include "KPrPresentationTool.moc"
