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
#include "PolygonTool.h"
#include "PolygonTool.moc"
#include "GDocument.h"
#include "Canvas.h"
#include "Coord.h"
#include "CommandHistory.h"
#include "PolygonConfigDialog.h"
#include "CreatePolygonCmd.h"
#include <qkeycode.h>
#include <kapp.h>
#include <klocale.h>

PolygonTool::PolygonTool (CommandHistory* history) : Tool (history) {
  obj = 0L;
  nCorners = 3;
  sharpValue = 0;
  createConcavePolygon = false;
}

void PolygonTool::processEvent (QEvent* e, GDocument *doc, Canvas* canvas) {
  if (e->type () == Event_MouseButtonPress) {
    QMouseEvent *me = (QMouseEvent *) e;
    int xpos = me->x (), ypos = me->y ();
    canvas->snapPositionToGrid (xpos, ypos);

    obj = new GPolygon (GPolygon::PK_Polygon);
    sPoint = Coord (xpos, ypos);
    obj->setSymmetricPolygon (sPoint, sPoint, nCorners, 
				 createConcavePolygon, sharpValue);
    doc->insertObject (obj);
  }
  else if (e->type () == Event_MouseMove) {
    if (obj == 0L)
      return;
    QMouseEvent *me = (QMouseEvent *) e;
    int xpos = me->x (), ypos = me->y ();
    canvas->snapPositionToGrid (xpos, ypos);

    obj->setSymmetricPolygon (sPoint, Coord (xpos, ypos), nCorners, 
			      createConcavePolygon, sharpValue);
  }
  else if (e->type () == Event_MouseButtonRelease) {
    if (obj == 0L)
      return;
    QMouseEvent *me = (QMouseEvent *) e;
    int xpos = me->x (), ypos = me->y ();
    canvas->snapPositionToGrid (xpos, ypos);

    obj->setSymmetricPolygon (sPoint, Coord (xpos, ypos), nCorners, 
			      createConcavePolygon, sharpValue);

    CreatePolygonCmd *cmd = new CreatePolygonCmd (doc, obj);
    history->addCommand (cmd);

    doc->unselectAllObjects ();
    doc->setLastObject (obj);
    obj = 0L;
  }
  else if (e->type () == Event_KeyPress) {
    QKeyEvent *ke = (QKeyEvent *) e;
    if (ke->key () == Key_Escape)
      emit operationDone ();
  }
  return;
}

void PolygonTool::configure () {
  PolygonConfigDialog::setupTool (this);
}

unsigned int PolygonTool::numCorners () const {
  return nCorners;
}

unsigned int PolygonTool::sharpness () const {
  return sharpValue;
}

bool PolygonTool::concavePolygon () const {
  return createConcavePolygon;
}

void PolygonTool::setNumCorners (unsigned int num) {
  nCorners = num;
}

void PolygonTool::setSharpness (unsigned int value) {
  sharpValue = value;
}

void PolygonTool::setConcavePolygon (bool flag) {
  createConcavePolygon = flag;
}

void PolygonTool::activate (GDocument* doc, Canvas* canvas) {
  emit modeSelected (i18n ("Create Polygon"));
}

