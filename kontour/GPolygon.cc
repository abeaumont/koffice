/* -*- C++ -*-

  $Id$
  This file is part of Kontour.
  Copyright (C) 1998 Kai-Uwe Sattler (kus@iti.cs.uni-magdeburg.de)
  Copyright (C) 2002 Igor Janssen (rm@kde.org)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU Library General Public License as
  published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#include "GPolygon.h"

#include <math.h>

#include <qdom.h>

#include <klocale.h>
#include <koVectorPath.h>
#include <koPainter.h>
#include <kdebug.h>

#include "kontour_global.h"
#include "GPath.h"

GPolygon::GPolygon(bool roundness):
GObject()
{
  if(roundness)
  {
    mXRoundness = 0.25;
    mYRoundness = 0.25;
  }
  else
  {
    mXRoundness = 0.0;
    mYRoundness = 0.0;
  }
}

GPolygon::GPolygon(const QDomElement &element):
GObject(element.namedItem("go").toElement())
{
  double x = 0;
  double y = 0;
  double sx = 0;
  double sy = 0;

  x = element.attribute("x").toDouble();
  y = element.attribute("y").toDouble();
  sx = element.attribute("sx").toDouble();
  sy = element.attribute("sy").toDouble();

  sPoint.setX(x);
  sPoint.setY(y);
  ePoint.setX(x + sx);
  ePoint.setY(y + sy);
  calcBoundingBox();
}

GPolygon::GPolygon(const GPolygon &obj):
GObject(obj)
{
  ePoint = obj.ePoint;
  sPoint = obj.sPoint;
  mXRoundness = obj.mXRoundness;
  mYRoundness = obj.mYRoundness;
  calcBoundingBox();
}

GObject *GPolygon::copy() const
{
  return new GPolygon(*this);
}

bool GPolygon::isSquare() const
{
  if(ePoint.x() - sPoint.x() == ePoint.y() - sPoint.y())
    return true;
  else
    return false;
}

void GPolygon::startPoint(const KoPoint &p)
{
  sPoint = p;
  calcBoundingBox();
}

void GPolygon::endPoint(const KoPoint &p)
{
  ePoint = p;
  calcBoundingBox();
}

QString GPolygon::typeName() const
{
  if(isSquare())
    return i18n("Square");
  else
    return i18n("Rectangle");
}

QDomElement GPolygon::writeToXml(QDomDocument &document)
{
  KoRect r(sPoint, ePoint);
  r = r.normalize();

  QDomElement rect = document.createElement("rect");
  rect.setAttribute("x", r.left());
  rect.setAttribute("y", r.top());
  rect.setAttribute("sx", r.width());
  rect.setAttribute("sy", r.height());
  rect.appendChild(GObject::writeToXml(document));
  return rect;
}

void GPolygon::draw(KoPainter *p, int aXOffset, int aYOffset, bool withBasePoints, bool outline, bool)
{
  setPen(p);
  setBrush(p);
  KoVectorPath *v = KoVectorPath::rectangle(sPoint.x(), sPoint.y(), ePoint.x() - sPoint.x(), ePoint.y() - sPoint.y(), 0.5 * mXRoundness * (ePoint.x() - sPoint.x()), 0.5 * mYRoundness * (ePoint.y() - sPoint.y()));
  QWMatrix m;
  m = m.translate(aXOffset, aYOffset);
  v->transform(tmpMatrix * m);
  p->drawVectorPath(v);
  delete v;
}

void GPolygon::calcBoundingBox()
{
  KoPoint p1(sPoint.x(), sPoint.y());
  KoPoint p2(ePoint.x(), sPoint.y());
  KoPoint p3(ePoint.x(), ePoint.y());
  KoPoint p4(sPoint.x(), ePoint.y());
  box = calcUntransformedBoundingBox(p1, p2, p3, p4);
  adjustBBox(box);
}

int GPolygon::getNeighbourPoint(const KoPoint &p)
{
// TODO implement isNear() and transform()
/*  for(int i = 1; i >= 0; i--)
  {
    KoPoint c = segPoint[i].transform(tMatrix);
    if(c.isNear(p, Kontour::nearDistance))
      return i;
  }*/
  return -1;
}

void GPolygon::movePoint(int idx, double dx, double dy, bool /*ctrlPressed*/)
{
/*  double adx = fabs (dx);
  double ady = fabs (dy);
  double angle = 0;

  if (idx == 0 && segPoint[0] == segPoint[1])
    idx = 1;

  Rect r (sPoint, ePoint);
  r.normalize ();

  double a = r.width () / 2.0;
  double b = r.height () / 2.0;

  if (adx > ady) {
    double x = segPoint[idx].x () + dx;
    if (x < r.left ())
      x = r.left ();
    else if (x > r.right ())
      x = r.right ();

    x -= (r.left () + a);
    angle = acos (x / a) * RAD_FACTOR;
    if (segPoint[idx].y () < r.center ().y ())
      angle = 360 - angle;
  }
  else {
    double y = segPoint[idx].y () + dy;
    if (y < r.top ())
      y = r.top ();
    else if (y > r.bottom ())
      y = r.bottom ();

    y -= (r.top () + b);
    angle = asin (y / b) * RAD_FACTOR;
    if (segPoint[idx].y () < r.center ().y ()) {
      if (segPoint[idx].x () > r.center ().x ())
        angle += 360;
      else
        angle = 180 - angle;;
    }
    else if (segPoint[idx].x () < r.center ().x ())
      angle = 180 - angle;
  }
  if (idx == 0)
    sAngle = angle;
  else
    eAngle = angle;

  // test for equality
  double a1 = qRound (sAngle < 0 ? sAngle + 360 : sAngle);
  double a2 = qRound (eAngle < 0 ? eAngle + 360 : eAngle);
  if (a1 >= a2 - 1 && a1 <= a2 + 1) {
    eAngle = sAngle;
    outlineInfo.shape = GObject::OutlineInfo::DefaultShape;
  }
  else if (outlineInfo.shape == GObject::OutlineInfo::DefaultShape)
    outlineInfo.shape = GObject::OutlineInfo::ArcShape;

  gShape.setInvalid ();

  updateRegion ();*/
}

void GPolygon::removePoint(int idx, bool update)
{
}

bool GPolygon::contains(const KoPoint &p)
{
  double x1, y1, x2, y2;

  if(box.contains(p))
  {
    QPoint pp = iMatrix.map(QPoint(static_cast<int>(p.x()), static_cast<int>(p.y())));
    if(sPoint.x() >= ePoint.x())
    {
      x1 = ePoint.x();
      x2 = sPoint.x();
    }
    else
    {
      x1 = sPoint.x();
      x2 = ePoint.x();
    }
    if(sPoint.y() >= ePoint.y())
    {
      y1 = ePoint.y();
      y2 = sPoint.y();
    }
    else
    {
      y1 = sPoint.y();
      y2 = ePoint.y();
    }

    if(pp.x() <= x2 && pp.x() >= x1 && pp.y() <= y2 && pp.y() >= y1)
      return true;
  }
  return false;
}

bool GPolygon::findNearestPoint(const KoPoint &p, double max_dist, double &dist, int &pidx, bool all)
{
  return true;
}

GPath *GPolygon::convertToPath() const
{
  GPath *path = new GPath(true);
  path->beginTo(sPoint.x(), sPoint.y());
  path->lineTo(sPoint.x(), ePoint.y());
  path->lineTo(ePoint.x(), ePoint.y());
  path->lineTo(ePoint.x(), sPoint.y());
  path->lineTo(sPoint.x(), sPoint.y());
  path->matrix(matrix());
  path->style(style());
  return path;
}

bool GPolygon::isConvertible() const
{
 return true;
}

#include "GPolygon.moc"
