/* This file is part of the KDE project
   Copyright (C) 2002, The Karbon Developers

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

#include "vselectiondesc.h"
#include "vselection.h"
#include "vgroup.h"
#include "vcomposite.h"
#include <kdebug.h>
#include <klocale.h>

void
VSelectionDescription::visitVSelection( VSelection& selection )
{
	if( selection.objects().count() == 1 )
		VVisitor::visitVSelection( selection );
	else
		m_desc = i18n( QString( "%1 objects" ).arg( selection.objects().count() ).latin1() );
}

void
VSelectionDescription::visitVComposite( VComposite& composite )
{
	m_desc = QString( i18n( "1 composite path" ) );
	m_shortdesc = composite.name( m_document ) ? QString( composite.name( m_document ) ) : QString( i18n( "path" ) );
}

void
VSelectionDescription::visitVGroup( VGroup &group )
{
	m_desc = i18n( QString( "1 group, containing %1 objects" ).arg( group.objects().count() ).latin1() );
	m_shortdesc = i18n( "group" );
}

void
VSelectionDescription::visitVText( VText& )
{
	m_desc = QString( i18n( "1 text object" ) );
	m_shortdesc = QString( i18n( "text" ) );
}

void
VSelectionDescription::visitVImage( VImage& )
{
	m_desc = QString( i18n( "1 image object" ) );
	m_shortdesc = QString( i18n( "image" ) );
}

