/* -*- C++ -*-

  $Id$

  This file is part of Kontour.
  Copyright (C) 1998-99 Kai-Uwe Sattler (kus@iti.cs.uni-magdeburg.de)
  Copyright (C) 2001 Igor Janssen (rm@linux.ru.net)

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

#ifndef __GPath_h__
#define __GPath_h__

#include "GObject.h"

#include <qptrlist.h>
#include <koPoint.h>

/**
 * Abstract class for path segment.
 *
 */
 
class GSegment
{
public:
  GSegment(const QDomElement &element);

  virtual const KoPoint &point(int i) const = 0;
  virtual void point(int i, const KoPoint &c) = 0;

  virtual QDomElement writeToXml(QDomDocument &document) = 0;
  virtual void draw(QPainter &p, bool withBasePoints, bool outline, bool drawFirst) = 0;
  virtual void movePoint(int idx, double dx, double dy, bool ctrlPressed = false) = 0;

  virtual KoRect boundingBox() = 0;
  virtual bool contains(const KoPoint &p) = 0;

  virtual QPointArray getPoints() const = 0;

  virtual double length() const = 0;
};


/**
 * Line segment.
 *
 */

class GLine : public GSegment
{
public:
  const KoPoint &point(int i) const;
  void point(int i, const KoPoint &c);

  QDomElement writeToXml(QDomDocument &document);
  void draw(QPainter &p, bool withBasePoints, bool outline, bool drawFirst);
  void movePoint(int idx, double dx, double dy, bool ctrlPressed = false);

  KoRect boundingBox();
  bool contains(const KoPoint &p);

  QPointArray getPoints() const;

  double length() const;
private:
  KoPoint points[2];
};


/**
 * Cubic Bezier segment.
 *
 */

/*class GCubicBezier : public GSegment
{
public:

private:
  KoPoint points[4];
};*/

/**
 * Quadratic Bezier segment.
 *
 */

/*class GQuadBezier : public GSegment
{
public:

private:
  KoPoint points[3];
};*/

/**
 * Arc segment.
 *
 */

/*class GArc : public GSegment
{
public:

private:
  koPoint points[3];
};*/


class GPath : public GObject
{
  Q_OBJECT
public:
  GPath(bool aClosed);
  GPath(const QDomElement &element);
  GPath(const GPath &obj);

  bool closed() const {return mClosed; }
  void closed(bool aClosed);

  void lineTo(KoPoint &p);
//  void curveTo();
//  void arcTo();

  QString typeName() const;
  QDomElement writeToXml(QDomDocument &document);
  void draw(QPainter &p, bool withBasePoints = false, bool outline = false, bool withEditMarks = true);

  int getNeighbourPoint(const KoPoint &point);
  void movePoint(int idx, double dx, double dy, bool ctrlPressed = false);
  void removePoint(int idx, bool update = true);
  bool contains(const KoPoint &p);
  bool findNearestPoint(const KoPoint &p, double max_dist, double &dist, int &pidx, bool all);

  void calcBoundingBox();
  GPath *convertToPath() const;
private:
  QPtrList<GSegment> segments;
  bool mClosed;
};

/*
  virtual void getPath(QValueList<Coord>& path);

  const GSegment& getSegment (int idx);
  int numOfSegments () const { return segments.count(); }
*/

#endif
