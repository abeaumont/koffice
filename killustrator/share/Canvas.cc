/* -*- C++ -*-

  $Id$

  This file is part of KIllustrator.
  Copyright (C) 1998 Kai-Uwe Sattler (kus@iti.cs.uni-magdeburg.de)

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

#include <iostream.h>
#include <fstream.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <qpainter.h>
#include <qprinter.h>
#include <qprintdialog.h>
#include <qcolor.h>    
#include <qdatetime.h>    
#include <kmsgbox.h>
#include "Canvas.h"
#include "Canvas.moc"
#include "GDocument.h"
#include "Handle.h"
#include "ToolController.h"
#include "QwViewport.h"
#include "version.h"
#include <kconfig.h>

QArray<float> Canvas::zoomFactors;

Canvas::Canvas (GDocument* doc, float res, QwViewport* vp, QWidget* parent, 
		const char* name) : QWidget (parent, name) {
  document = doc;
  resolution = res;
  zoomFactor = 1.0;
  drawBasePoints = false;
  viewport = vp;

  connect (document, SIGNAL (changed ()), this, SLOT (updateView ()));
  connect (document, SIGNAL (changed (const Rect&)), 
	   this, SLOT (updateRegion (const Rect&)));
  connect (document, SIGNAL (sizeChanged ()), this, SLOT (calculateSize ()));
  connect (&(document->handle ()), SIGNAL (handleChanged ()),
	   this, SLOT (updateView ()));

  pixmap = 0L;

  readGridProperties ();

  calculateSize ();
  setFocusPolicy (ClickFocus);
  setMouseTracking (true);
  setBackgroundMode (NoBackground);

  dragging = false;
  ensureVisibilityFlag = false;
  outlineMode = false;
}

Canvas::~Canvas () {
  delete pixmap;
}

void Canvas::ensureVisibility (bool flag) {
  ensureVisibilityFlag = flag;
}

void Canvas::calculateSize () {
  width = (int) (document->getPaperWidth () * resolution * 
		 zoomFactor / 72.0);
  height = (int) (document->getPaperHeight () * resolution * 
		  zoomFactor / 72.0);
  resize (width, height);

  if (pixmap != 0L)
    delete pixmap;
  pixmap = new QPixmap (width, height);
  pixmap->fill (white);
  updateView ();
  emit sizeChanged ();
}

void Canvas::initZoomFactors (QArray<float>& factors) {
  zoomFactors.duplicate (factors);
}


void Canvas::setZoomFactor (float factor) {
  zoomFactor = factor;
  calculateSize ();
  updateView ();
  emit sizeChanged ();
  emit zoomFactorChanged (zoomFactor);
}

float Canvas::getZoomFactor () const {
  return zoomFactor;
}

void Canvas::showGrid (bool flag) {
  if (gridIsOn != flag) {
    gridIsOn = flag;
    updateView ();
    saveGridProperties ();
  }
}

void Canvas::snapToGrid (bool flag) {
  if (gridSnapIsOn != flag) {
    gridSnapIsOn = flag;
    saveGridProperties ();
  }    
}

void Canvas::setGridDistance (int hdist, int vdist) {
  hGridDistance = hdist;
  vGridDistance = vdist;
  saveGridProperties ();
}

void Canvas::snapPositionToGrid (int& x, int& y) {
  if (gridSnapIsOn) {
    int p, m, n;

    p = x / hGridDistance;
    m = x % hGridDistance;
    n = p * hGridDistance;
    if (m > hGridDistance / 2)
      n += hGridDistance;
    x = n;

    p = y / vGridDistance;
    m = y % vGridDistance;
    n = p * vGridDistance;
    if (m > vGridDistance / 2)
      n += hGridDistance;
    y = n;
  }
}

void Canvas::setToolController (ToolController* tc) {
  toolController = tc;
}

void Canvas::propagateMouseEvent (QMouseEvent *e) {
  // transform position of the mouse pointer according to current
  // zoom factor
  QPoint new_pos ((int) (e->x () * 72 / (resolution * zoomFactor)),
		  (int) (e->y () * 72 / (resolution * zoomFactor)));
  QMouseEvent new_ev (e->type (), new_pos, e->button (), e->state ());

  emit mousePositionChanged (new_pos.x (), new_pos.y ());

  // ensure visibility
  if (ensureVisibilityFlag) {
    if (e->type () == Event_MouseButtonPress && e->button () == LeftButton)
      dragging = true;
    else if (e->type () == Event_MouseButtonRelease && 
	     e->button () == LeftButton)
      dragging = false;
    else if (e->type () == Event_MouseMove && dragging) 
      viewport->ensureVisible (e->x (), e->y (), 10, 10);
  }

  if (e->button () == RightButton &&
      e->type () == Event_MouseButtonPress &&
      ! toolController->getActiveTool ()->consumesRMBEvents ()) {
    if (document->selectionIsEmpty ()) {
      GObject* obj = document->findContainingObject (new_pos.x (), 
						     new_pos.y ());
      if (obj) {
	// pop up menu for the picked object
	emit rightButtonAtObjectClicked (e->x (), e->y (), obj);
      }
      else {
	emit rightButtonClicked (e->x (), e->y ());
      }
    }
    else {
      // pop up menu for the current selection
      emit rightButtonAtSelectionClicked (e->x (), e->y ());
    }
    return;
  }
  else
    if (toolController) {
      // the tool controller processes the event
      toolController->delegateEvent (&new_ev, document, this);
    }
}

void Canvas::propagateKeyEvent (QKeyEvent *e) {
  if (toolController) {
    toolController->delegateEvent (e, document, this);
  }
}

void Canvas::mousePressEvent (QMouseEvent* e) {
  propagateMouseEvent (e);
}

void Canvas::mouseReleaseEvent (QMouseEvent* e) {
  propagateMouseEvent (e);
}

void Canvas::mouseMoveEvent (QMouseEvent* e) {
  propagateMouseEvent (e);
}

void Canvas::keyPressEvent (QKeyEvent* e) {
  propagateKeyEvent (e);
}

void Canvas::paintEvent (QPaintEvent* e) {
  const QRect& rect = e->rect ();
  bitBlt (this, rect.x (), rect.y (), pixmap, 
	  rect.x (), rect.y (), rect.width (), rect.height ());
}

void Canvas::moveEvent (QMoveEvent *e) {
    emit visibleAreaChanged (e->pos ().x (), e->pos ().y ());
}

void Canvas::setDocument (GDocument* doc) {
  document = doc;
  connect (document, SIGNAL (changed ()), this, SLOT (updateView ()));
}

GDocument* Canvas::getDocument () {
  return document;
}

void Canvas::showBasePoints (bool flag) {
  drawBasePoints = flag;
  updateView ();
}

void Canvas::setOutlineMode (bool flag) {
  if (outlineMode != flag) {
    outlineMode = flag;
    updateView ();
  }
}

float Canvas::scaleFactor () const {
  return resolution * zoomFactor / 72.0;
}

void Canvas::updateView () {
  Painter p;
  float s = scaleFactor ();

  // setup the painter  
  p.begin (pixmap);
  p.setBackgroundColor (white);
  pixmap->fill (backgroundColor ());

  // clear the canvas
  p.scale (s, s);
  p.eraseRect (0, 0, document->getPaperWidth (),
	       document->getPaperHeight ());

  // draw the grid
  if (gridIsOn)
    drawGrid (p);

  // next the document contents
  document->drawContents (p, drawBasePoints, outlineMode);

  // and finally the handle
  if (! document->selectionIsEmpty ())
    document->handle ().draw (p);

  p.end ();
  repaint ();
}

void Canvas::updateRegion (const Rect& r) {
  Painter p;
  float s = scaleFactor ();

  // compute the clipping region
  QWMatrix m;
  m.scale (s, s);
  QRect clip = m.map (QRect (int (r.left ()), int (r.top ()), 
			     int (r.width ()), int (r.height ())));

  // setup the painter  
  p.begin (pixmap);
  p.setBackgroundColor (white);
  // setup the clip region
  p.setClipRect (clip);

  // clear the canvas
  p.scale (s, s);
  p.eraseRect (r.left (), r.top (), r.width (), r.height ());

  // draw the grid
  if (gridIsOn)
    drawGrid (p);

  // next the document contents
  document->drawContentsInRegion (p, r, drawBasePoints, outlineMode);

  // and finally the handle
  if (! document->selectionIsEmpty ())
    document->handle ().draw (p);

  p.end ();
  repaint (clip, false);
}

void Canvas::drawGrid (Painter& p) {
  int pw = document->getPaperWidth ();
  int ph = document->getPaperHeight ();
  int h, v;

  QPen pen1 (blue, 0, DotLine);

  p.save ();
  p.setPen (pen1);
  for (h = hGridDistance; h < pw; h += hGridDistance)
    p.drawLine (h, 0, h, ph);
  for (v = vGridDistance; v < ph; v += vGridDistance)
    p.drawLine (0, v, pw, v);
  p.restore ();
}

void Canvas::printDocument () {
  QPrinter printer;
  printer.setDocName (document->fileName ());
  printer.setCreator ("KIllustrator");
  switch (document->pageLayout ().format) {
  case PG_DIN_A4:
    printer.setPageSize (QPrinter::A4);
    break;
  case PG_DIN_A5:
    printer.setPageSize (QPrinter::B5);
    break;
  case PG_US_LETTER:
    printer.setPageSize (QPrinter::Letter);
    break;
  case PG_US_LEGAL:
    printer.setPageSize (QPrinter::Legal);
    break;
  default:
    break;
  }
  printer.setOrientation (document->pageLayout ().orientation == PG_PORTRAIT ?
			  QPrinter::Portrait : QPrinter::Landscape);
  if (printer.setup (this)) {
    Painter paint;
    paint.begin (&printer);
    paint.setClipping (false);
    document->drawContents (paint);
    paint.end ();
  }
}

void Canvas::zoomIn (int x, int y) {
  int pos = zoomFactors.find (getZoomFactor ());
  assert (pos != -1);
  if (pos < (int) zoomFactors.size () - 1) {
    setZoomFactor (zoomFactors[pos + 1]);
    viewport->centerOn (x, y);
    emit zoomFactorChanged (zoomFactors[pos + 1]);
  }
}

void Canvas::zoomOut () {
  int pos = zoomFactors.find (getZoomFactor ());
  assert (pos != -1);
  if (pos > 0)
    setZoomFactor (zoomFactors[pos - 1]);
}

void Canvas::readGridProperties () {
  KConfig* config = kapp->getConfig ();
  QString oldgroup = config->group ();

  config->setGroup ("Grid");

  vGridDistance = config->readNumEntry ("vGridDistance", 50);
  hGridDistance = config->readNumEntry ("hGridDistance", 50);
  gridIsOn = config->readBoolEntry ("showGrid", false);
  gridSnapIsOn = config->readBoolEntry ("snapTopGrid", false);

  config->setGroup (oldgroup);
}

void Canvas::saveGridProperties () {
  KConfig* config = kapp->getConfig ();
  QString oldgroup = config->group ();

  config->setGroup ("Grid");

  config->writeEntry ("vGridDistance", vGridDistance);
  config->writeEntry ("hGridDistance", hGridDistance);
  config->writeEntry ("showGrid", gridIsOn);
  config->writeEntry ("snapTopGrid", gridSnapIsOn);

  config->setGroup (oldgroup);
  config->sync ();
}
