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

#include <CopyCmd.h>

#include <qclipboard.h>
#include <qdom.h>
#include <kapp.h>
#include <klocale.h>

#include <GDocument.h>
#include <GObject.h>

CopyCmd::CopyCmd (GDocument* doc)
  : Command(i18n("Copy"))
{
  document = doc;
  for(QListIterator<GObject> it(doc->getSelection()); it.current(); ++it) {
    GObject* o = *it;
    o->ref ();
    objects.append(o);
  }
}

CopyCmd::~CopyCmd () {
  for(GObject *o=objects.first();
       o!=0L; o=objects.next())
      o->unref ();
}

void CopyCmd::execute () {

    QDomDocument docu("doc");
    QDomElement doc=docu.createElement("doc");
    doc.setAttribute ("mime", KILLUSTRATOR_MIMETYPE);
    docu.appendChild(doc);

    for (GObject *o=objects.first(); o!=0L;
         o=objects.next())
        doc.appendChild(o->writeToXml (docu));
    QApplication::clipboard ()->setText (docu.toCString());
}

void CopyCmd::unexecute () {
  QApplication::clipboard ()->clear ();
}

#include <CopyCmd.moc>
