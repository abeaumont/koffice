/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
*/

#include <qpainter.h>
#include <kdebug.h>

#include "vpoint.h"
#include "vpath.h"

#include "karbon_part.h"
#include "karbon_view.h"

KarbonPart::KarbonPart( QWidget* parentWidget, const char* widgetName,
	QObject* parent, const char* name, bool singleViewMode )
	: KoDocument( parentWidget, widgetName, parent, name, singleViewMode )
{
	// create a layer. we need at least one:
	layers().append( new VLayer() );


// <test-object> <<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	VPath* path = new VPath();
	double x1, y1, x2, y2;

	x1 = 200.0; y1 = 100.0;
	path->moveTo(x1,y1);

	x1 = 300.0; y1 = 100.0;
	x2 = 300.0; y2 = 200.0;
	path->arcTo(x1,y1,x2,y2,100.0);
	x1 = 300.0; y1 = 300.0;
	x2 = 200.0; y2 = 300.0;
	path->arcTo(x1,y1,x2,y2,100.0);
	x1 = 100.0; y1 = 300.0;
	x2 = 100.0; y2 = 200.0;
	path->arcTo(x1,y1,x2,y2,100.0);
	x1 = 100.0; y1 = 100.0;
	x2 = 200.0; y2 = 100.0;
	path->arcTo(x1,y1,x2,y2,100.0);
	path->close();
	path->scale( 2.0, 1.0 );
	layers().last()->objects().append( path );

// </test-object> <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
}

KarbonPart::~KarbonPart()
{
	for ( VLayer* layer = layers().first(); layer != 0L; layer = layers().next() )
	{
		delete( layer );
	}
}

bool
KarbonPart::initDoc()
{
	// If nothing is loaded, do initialize here
	return true;
}

KoView*
KarbonPart::createViewInstance( QWidget* parent, const char* name )
{
	return new KarbonView( this, parent, name );
}

bool
KarbonPart::loadXML( QIODevice*, const QDomDocument& )
{
	// TODO load the document from the QDomDocument
	return true;
}

QDomDocument
KarbonPart::saveXML()
{
	// TODO save the document into a QDomDocument
	return QDomDocument();
}


void
KarbonPart::paintContent( QPainter& /*p*/, const QRect& /*rect*/,
	bool /*transparent*/, double /*zoomX*/, double /*zoomY*/ )
{
	kdDebug() << "part->paintContent()" << endl;
}

#include "karbon_part.moc"
