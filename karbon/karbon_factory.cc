/* This file is part of the KDE project
   Copyright (C) 2001, 2002, 2003 The Karbon Developers

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


#include <kaboutdata.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include "karbon_factory.h"
#include "karbon_part.h"
#include "karbon_resourceserver.h"
#include "karbon_aboutdata.h"

#include <kdebug.h>


KarbonResourceServer* KarbonFactory::s_rserver = 0;


K_EXPORT_COMPONENT_FACTORY( libkarbonpart, KarbonFactory() )

KInstance* KarbonFactory::s_instance = 0L;
KAboutData* KarbonFactory::s_aboutData = 0L;

KarbonFactory::KarbonFactory( QObject* parent, const char* name )
		: KoFactory( parent, name )
{
	instance();
}

KarbonFactory::~KarbonFactory()
{
	delete s_instance;
	s_instance = 0L;
	delete s_aboutData;
	s_aboutData = 0L;
	delete s_rserver;
	s_rserver = 0L;
}

KParts::Part*
KarbonFactory::createPartObject( QWidget* parentWidget, const char* widgetName,
								 QObject* parent, const char* name, const char* classname, const QStringList& )
{
	// If classname is "KoDocument", our host is a koffice application
	// otherwise, the host wants us as a simple part, so switch to readonly and
	// single view.
	bool bWantKoDocument = ( strcmp( classname, "KoDocument" ) == 0 );

	// parentWidget and widgetName are used by KoDocument for the
	// "readonly+singleView" case.
	KarbonPart* part =
		new KarbonPart( parentWidget, widgetName, parent, name, !bWantKoDocument );

	if( !bWantKoDocument )
		part->setReadWrite( false );

	return part;
}

KAboutData*
KarbonFactory::aboutData()
{
	if( !s_aboutData )
            s_aboutData = newKarbonAboutData();
	return s_aboutData;
}

KInstance*
KarbonFactory::instance()
{
	if( !s_instance )
	{
		s_instance = new KInstance( aboutData() );
		// Add any application-specific resource directories here

		s_instance->dirs()->addResourceType( "kis_brushes",
											  KStandardDirs::kde_default( "data" ) + "krita/brushes/" );

		s_instance->dirs()->addResourceType( "kis_pattern",
											  KStandardDirs::kde_default( "data" ) + "krita/patterns/" );

		s_instance->dirs()->addResourceType( "karbon_gradient",
											  KStandardDirs::kde_default( "data" ) + "karbon/gradients/" );

		s_instance->dirs()->addResourceType( "karbon_clipart",
											  KStandardDirs::kde_default( "data" ) + "karbon/cliparts/" );
                // Tell the iconloader about share/apps/koffice/icons
                s_instance->iconLoader()->addAppDir("koffice");
	}

	return s_instance;
}

KarbonResourceServer *KarbonFactory::rServer()
{
	if( !s_rserver )
		s_rserver = new KarbonResourceServer;

	return s_rserver;
}

#include "karbon_factory.moc"

