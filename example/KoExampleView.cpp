/* This file is part of the KDE project
   Copyright (C) 1998, 1999, 2000 Torben Weis <weis@kde.org>

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

#include "KoExampleView.h"
#include "KoExampleFactory.h"
#include "KoExamplePart.h"

#include <qpainter.h>
#include <qicon.h>
//Added by qt3to4:
#include <QPaintEvent>
#include <kaction.h>
#include <kstandardaction.h>
#include <kactioncollection.h>
#include <klocale.h>
#include <kdebug.h>

ExampleView::ExampleView( ExamplePart* part, QWidget* parent )
    : KoView( part, parent)
{
    setComponentData( ExampleFactory::global() );
    if ( !part->isReadWrite() ) // readonly case, e.g. when embedded into konqueror
        setXMLFile( "example_readonly.rc" ); // simplified GUI
    else
        setXMLFile( "example.rc" );
    actionCollection()->addAction(KStandardAction::Copy,  "copy", this, SLOT( copy() ));
    actionCollection()->addAction(KStandardAction::Cut,  "cut", this, SLOT( cut() ));
    // Note: Prefer KStandardAction::* to any custom action if possible.
    //m_cut = new KAction( i18n("&Cut"), "editcut", 0, this, SLOT( cut() ),
    //                   actionCollection(), "cut");
}

void ExampleView::paintEvent( QPaintEvent* ev )
{
    QPainter painter;
    painter.begin( this );

    // ### TODO: Scaling

    // Let the document do the drawing
    koDocument()->paintEverything( painter, ev->rect(), this );

    painter.end();
}

void ExampleView::updateReadWrite( bool /*readwrite*/ )
{
#ifdef __GNUC__
#warning TODO
#endif
}

void ExampleView::copy()
{
    kDebug(31000) << "ExampleView::copy(): COPY called" << endl;
}

void ExampleView::cut()
{
    kDebug(31000) << "ExampleView::cut(): CUT called" << endl;
}

#include "KoExampleView.moc"
