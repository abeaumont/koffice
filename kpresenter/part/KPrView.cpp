/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>

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

#include "KPrView.h"

#include <KPrDocument.h>
#include <KPrViewModePresentation.h>

#include <klocale.h>
#include <ktoggleaction.h>
#include <kactioncollection.h>

KPrView::KPrView( KPrDocument *document, QWidget *parent )
: KoPAView( document, parent )
, m_presentationMode( new KPrViewModePresentation( this, m_canvas ))                       
, m_normalMode( 0 )    
{
    initActions();
}

KPrView::~KPrView()
{
}


void KPrView::initGUI()
{
    // KoPAView::initGUI();
    // do special kpresenter stuff here
}

void KPrView::initActions()
{
    if ( !m_doc->isReadWrite() )
       setXMLFile( "kpresenter_readonly.rc" );
    else
       setXMLFile( "kpresenter.rc" );

    // do special kpresenter stuff here
    m_actionStartPresentation  = new KAction( i18n( "Start Presentation" ), this );
    actionCollection()->addAction( "view_mode", m_actionStartPresentation );
    connect( m_actionStartPresentation, SIGNAL( activated() ), this, SLOT( startPresentation() ) );
}

void KPrView::startPresentation()
{
    setViewMode( m_presentationMode );
}


#include "KPrView.moc"
