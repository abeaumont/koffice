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
// <test-object> <<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    VPath* path = new VPath();
    double x1, y1, x2, y2;

    x1 = 40.0; y1 = 10.0;    
    path->moveTo(x1,y1);
    
    x1 = 500.0; y1 = 100.0; 
    x2 = 500.0; y2 = 300.0; 
    path->arcTo(x1,y1,x2,y2,100.0);
    x1 = 500.0; y1 = 400.0; 
    x2 = 400.0; y2 = 400.0; 
    path->arcTo(x1,y1,x2,y2,100.0);
    x1 = 300.0; y1 = 400.0; 
    x2 = 200.0; y2 = 200.0; 
    path->arcTo(x1,y1,x2,y2,60.0);
    x1 = 200.0; y1 = 200.0; 
    x2 = 400.0; y2 = 200.0; 
    path->arcTo(x1,y1,x2,y2,20.0);
    x1 = 500.0; y1 = 200.0; 
    x2 = 500.0; y2 = 300.0; 
    path->arcTo(x1,y1,x2,y2,60.0);
    m_objects.append( path );


    path = new VPath();

    x1 = 200.0; y1 = 100.0;    
    path->moveTo(x1,y1);
    
    x1 = 240.0; y1 = 100.0; 
    x2 = 240.0; y2 = 140.0; 
    path->arcTo(x1,y1,x2,y2,40.0);
    x1 = 240.0; y1 = 180.0; 
    x2 = 200.0; y2 = 180.0; 
    path->arcTo(x1,y1,x2,y2,40.0);
    x1 = 160.0; y1 = 180.0; 
    x2 = 160.0; y2 = 140.0; 
    path->arcTo(x1,y1,x2,y2,40.0);
    x1 = 160.0; y1 = 100.0; 
    x2 = 200.0; y2 = 100.0; 
    path->arcTo(x1,y1,x2,y2,40.0);
    path->close();
    m_objects.append( path );

// </test-object> <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
}

KarbonPart::~KarbonPart()
{
    for ( VObject* object=m_objects.first(); object!=0L; object=m_objects.next() ) 
    {
	delete( object );
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
KarbonPart::paintContent( QPainter& painter, const QRect& /*rect*/,
    bool /*transparent*/ )
{
    painter.scale(VPoint::s_fractInvScale,VPoint::s_fractInvScale);
    // paint all objects:
    VObject* obj;
    for ( obj=m_objects.first(); obj!=0L; obj=m_objects.next() ) 
    {
	obj->draw( painter );
    }
}

#include "karbon_part.moc"
