/*
 * Kivio - Visual Modelling and Flowcharting
 * Copyright (C) 2005 Peter Simonsson
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include "smlobjectloader.h"

#include <qdom.h>
#include <qbrush.h>

#include <kdebug.h>

#include "object.h"
#include "rectangleobject.h"
#include "roundedrectangleobject.h"
#include "bezierobject.h"
#include "ellipseobject.h"
#include "polygonobject.h"
#include "polylineobject.h"
#include "linearrayobject.h"
#include "kivio_common.h"
#include "pen.h"

namespace Kivio {

SmlObjectLoader::SmlObjectLoader()
{
  m_shapeTypeMap.insert("Arc", kstArc);
  m_shapeTypeMap.insert("Pie", kstPie);
  m_shapeTypeMap.insert("LineArray", kstLineArray);
  m_shapeTypeMap.insert("Polyline", kstPolyline);
  m_shapeTypeMap.insert("Polygon", kstPolygon);
  m_shapeTypeMap.insert("Bezier", kstBezier);
  m_shapeTypeMap.insert("Rectangle", kstRectangle);
  m_shapeTypeMap.insert("RoundRectangle", kstRoundedRectangle);
  m_shapeTypeMap.insert("Ellipse", kstEllipse);
  m_shapeTypeMap.insert("OpenPath", kstOpenPath);
  m_shapeTypeMap.insert("ClosedPath", kstClosedPath);
  m_shapeTypeMap.insert("TextBox", kstTextBox);
}

SmlObjectLoader::~SmlObjectLoader()
{
}

Object* SmlObjectLoader::loadObject(const QDomElement& shapeElement)
{
  Object* object = 0;
  ShapeType t;
  QString typeString = XmlReadString(shapeElement, "type", "None");
  QMap<QString, ShapeType>::iterator it = m_shapeTypeMap.find(typeString);
//   kdDebug() << "Loading shape type: " << typeString << endl;

  if(it == m_shapeTypeMap.end()) {
    return 0;
  }

  t = it.data();

  switch( t )
  {
    case kstNone:
      break;

    case kstArc:
      break;

    case kstPie:
      break;

    case kstLineArray:
      object = loadLineArrayObject(shapeElement);
      break;

    case kstPolyline:
      object = loadPolylineObject(shapeElement);
      break;

    case kstPolygon:
      object = loadPolygonObject(shapeElement);
      break;

    case kstBezier:
      object = loadBezierObject(shapeElement);
      break;

    case kstRectangle:
      object = loadRectangleObject(shapeElement);
      break;

    case kstRoundedRectangle:
      object = loadRoundedRectangleObject(shapeElement);
      break;

    case kstEllipse:
      object = loadEllipseObject(shapeElement);
      break;

    case kstOpenPath:
      break;

    case kstClosedPath:
      break;

    case kstTextBox:
      break;

    default:
      break;
  }

  return object;
}

Object* SmlObjectLoader::loadRectangleObject(const QDomElement& shapeElement)
{
  // Create the new shape to load into
  RectangleObject* object = new RectangleObject();

  // Load the name and geometry properties
  object->setId(XmlReadString(shapeElement, "name", ""));
  object->setPosition(KoPoint(XmlReadFloat(shapeElement, "x", 0.0f), XmlReadFloat(shapeElement, "y", 0.0f)));
  object->setSize(KoSize(XmlReadFloat(shapeElement, "w", 72.0f), XmlReadFloat(shapeElement, "h", 72.0f)));

  // Iterate through the nodes loading the various items
  QDomNode node = shapeElement.firstChild();
  QString nodeName;

  while(!node.isNull())
  {
    nodeName = node.nodeName();

    if( nodeName == "KivioFillStyle" )
    {
      object->setBrush(loadBrush(node.toElement()));
    }
    else if( nodeName == "KivioLineStyle" )
    {
      object->setPen(loadPen(node.toElement()));
    }

    node = node.nextSibling();
  }

  return object;
}

Object* SmlObjectLoader::loadRoundedRectangleObject(const QDomElement& shapeElement)
{
  // Create the new shape to load into
  RoundedRectangleObject* object = new RoundedRectangleObject();

  // Load the name and geometry properties
  object->setId(XmlReadString(shapeElement, "name", ""));
  object->setPosition(KoPoint(XmlReadFloat(shapeElement, "x", 0.0f), XmlReadFloat(shapeElement, "y", 0.0f)));
  object->setSize(KoSize(XmlReadFloat(shapeElement, "w", 72.0f), XmlReadFloat(shapeElement, "h", 72.0f)));

  // Read and store the radii of the curves
  object->setXRoundness(qRound(XmlReadFloat(shapeElement, "r1", 1.0f)));
  object->setYRoundness(qRound(XmlReadFloat(shapeElement, "r2", 1.0f)));

  // Iterate through the nodes loading the various items
  QDomNode node = shapeElement.firstChild();
  QString nodeName;

  while(!node.isNull())
  {
    nodeName = node.nodeName();

    if( nodeName == "KivioFillStyle" )
    {
      object->setBrush(loadBrush(node.toElement()));
    }
    else if( nodeName == "KivioLineStyle" )
    {
      object->setPen(loadPen(node.toElement()));
    }

    node = node.nextSibling();
  }

  return object;
}

Object* SmlObjectLoader::loadBezierObject(const QDomElement& shapeElement)
{
  // Create the new shape to load into
  BezierObject* object = new BezierObject();

  // Load the type, name, and lineWidth properties
  object->setId(XmlReadString(shapeElement, "name", ""));

  // Iterate through the nodes loading the various items
  QDomNode node = shapeElement.firstChild();
  QString nodeName;
  KoPoint point;
  QDomElement tmpElement;
  QValueVector<KoPoint> points;

  while(!node.isNull())
  {
    nodeName = node.nodeName();

    // The line array  is made up of pairs of points
    if( nodeName == "KivioPoint" )
    {
      tmpElement = node.toElement();
      point.setX(XmlReadFloat(tmpElement, "x", 1.0f));
      point.setY(XmlReadFloat(tmpElement, "y", 1.0f));
      points.append(point);
    }
    else if( nodeName == "KivioLineStyle" )
    {
      object->setPen(loadPen(node.toElement()));
    }

    node = node.nextSibling();
  }

  if(points.count() != 4 )
  {
    kdDebug(43000) << "KivioShape::loadShapeBezier() - Wrong number of points loaded, should be 4, shape aborted" << endl;
    delete object;
    return 0;
  }

  object->setPointVector(points);

  return object;
}

Object* SmlObjectLoader::loadEllipseObject(const QDomElement& shapeElement)
{
  // Create the new shape to load into
  EllipseObject* object = new EllipseObject();

  // Load the name and geometry properties
  object->setId(XmlReadString(shapeElement, "name", ""));
  object->setPosition(KoPoint(XmlReadFloat(shapeElement, "x", 0.0f), XmlReadFloat(shapeElement, "y", 0.0f)));
  object->setSize(KoSize(XmlReadFloat(shapeElement, "w", 72.0f), XmlReadFloat(shapeElement, "h", 72.0f)));

  // Iterate through the nodes loading the various items
  QDomNode node = shapeElement.firstChild();
  QString nodeName;

  while(!node.isNull())
  {
    nodeName = node.nodeName();

    if( nodeName == "KivioFillStyle" )
    {
      object->setBrush(loadBrush(node.toElement()));
    }
    else if( nodeName == "KivioLineStyle" )
    {
      object->setPen(loadPen(node.toElement()));
    }

    node = node.nextSibling();
  }

  return object;
}

Object* SmlObjectLoader::loadPolygonObject(const QDomElement& shapeElement)
{
  // Create the new shape to load into
  PolygonObject* object = new PolygonObject;

  // Load the name
  object->setId(XmlReadString(shapeElement, "name", ""));

    // Iterate through the nodes loading the various items
  QDomNode node = shapeElement.firstChild();
  QString nodeName;
  KoPoint point;
  QDomElement tmpElement;
  QValueVector<KoPoint> points;

  while( !node.isNull() )
  {
    nodeName = node.nodeName();

    // The polygon is made up of a series of points
    if( nodeName == "KivioPoint" )
    {
      tmpElement = node.toElement();
      point.setX(XmlReadFloat(tmpElement, "x", 1.0f));
      point.setY(XmlReadFloat(tmpElement, "y", 1.0f));
      points.append(point);
    }
    else if( nodeName == "KivioFillStyle" )
    {
      object->setBrush(loadBrush(node.toElement()));
    }
    else if( nodeName == "KivioLineStyle" )
    {
      object->setPen(loadPen(node.toElement()));
    }

    node = node.nextSibling();
  }

  object->setPointVector(points);

  return object;
}

Object* SmlObjectLoader::loadPolylineObject(const QDomElement& shapeElement)
{
  // Create the new shape to load into
  PolylineObject* object = new PolylineObject;

  // Load the name
  object->setId(XmlReadString(shapeElement, "name", ""));

    // Iterate through the nodes loading the various items
  QDomNode node = shapeElement.firstChild();
  QString nodeName;
  KoPoint point;
  QDomElement tmpElement;
  QValueVector<KoPoint> points;

  while( !node.isNull() )
  {
    nodeName = node.nodeName();

    // The polygon is made up of a series of points
    if( nodeName == "KivioPoint" )
    {
      tmpElement = node.toElement();
      point.setX(XmlReadFloat(tmpElement, "x", 1.0f));
      point.setY(XmlReadFloat(tmpElement, "y", 1.0f));
      points.append(point);
    }
    else if( nodeName == "KivioLineStyle" )
    {
      object->setPen(loadPen(node.toElement()));
    }

    node = node.nextSibling();
  }

  object->setPointVector(points);

  return object;
}

Object* SmlObjectLoader::loadLineArrayObject(const QDomElement& shapeElement)
{
  // Create the new shape to load into
  LineArrayObject* object = new LineArrayObject;

  // Load the name
  object->setId(XmlReadString(shapeElement, "name", ""));

    // Iterate through the nodes loading the various items
  QDomNode node = shapeElement.firstChild();
  QString nodeName;
  KoPoint point;
  QDomElement tmpElement;
  QValueVector<KoPoint> points;

  while( !node.isNull() )
  {
    nodeName = node.nodeName();

    // The polygon is made up of a series of points
    if( nodeName == "Line" )
    {
      tmpElement = node.toElement();
      point.setX(XmlReadFloat(tmpElement, "x1", 1.0f));
      point.setY(XmlReadFloat(tmpElement, "y1", 1.0f));
      points.append(point);
      point.setX(XmlReadFloat(tmpElement, "x2", 1.0f));
      point.setY(XmlReadFloat(tmpElement, "y2", 1.0f));
      points.append(point);
    }
    else if( nodeName == "KivioLineStyle" )
    {
      object->setPen(loadPen(node.toElement()));
    }

    node = node.nextSibling();
  }

  object->setPointVector(points);

  return object;
}

Pen SmlObjectLoader::loadPen(const QDomElement& element)
{
  Pen p;
  p.setColor(XmlReadColor(element, "color", QColor(0,0,0)));
  p.setPointWidth(XmlReadFloat(element, "width", 1.0f));
  p.setCapStyle(static_cast<Qt::PenCapStyle>(XmlReadInt(element, "capStyle",  Qt::RoundCap)));
  p.setJoinStyle(static_cast<Qt::PenJoinStyle>(XmlReadInt(element, "joinStyle", Qt::RoundJoin)));
  p.setStyle(static_cast<Qt::PenStyle>(XmlReadInt(element, "pattern",   Qt::SolidLine)));

  return p;
}

QBrush SmlObjectLoader::loadBrush(const QDomElement& element)
{
  QBrush b;
  QDomElement ele;
  QDomNode node;

  b.setColor(XmlReadColor(element, "color", QColor(255,255,255)));
  //m_color2 = XmlReadColor( e, "gradientColor", QColor(255,255,255).rgb() );

  int colorstyle = XmlReadInt(element, "colorStyle", 1/*Solid fill*/);

  if(colorstyle) {
    b.setStyle(Qt::SolidPattern);
  } else {
    b.setStyle(Qt::NoBrush);
  }

  //m_gradientType = static_cast<KImageEffect::GradientType>(XmlReadInt(e, "gradientType", KImageEffect::VerticalGradient));

  return b;
}

}
