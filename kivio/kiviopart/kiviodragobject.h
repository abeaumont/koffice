/* This file is part of the KDE project
   Copyright (C) 2003 Peter Simonsson <psn@linux.se>

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

#ifndef KIVIODRAGOBJECT_H
#define KIVIODRAGOBJECT_H

#include <qdragobject.h>
#include <qptrlist.h>

#include "kivio_stencil.h"
#include "kivio_rect.h"

class QStringList;
class KivioPage;

class KivioDragObject : public QDragObject
{
  Q_OBJECT
  public:
    KivioDragObject(QWidget* dragSource = 0, const char* name = 0);

    const char* format(int i) const;
    QByteArray encodedData(const char* mimetype) const;
    bool canDecode(QMimeSource*);
    bool decode(QMimeSource* e, QPtrList<KivioStencil>& sl, KivioPage* page);
    void setStencilList(QPtrList<KivioStencil> l);
    void setStencilRect(KivioRect r);

  protected:
    QByteArray kivioEncoded() const;
    QByteArray imageEncoded(const char* mimetype) const;

  private:
    QStringList m_decodeMimeList;
    enum { NumEncodeFormats = 4 };
    QCString m_encodeMimeList[NumEncodeFormats];
    QPtrList<KivioStencil> m_stencilList;
    KivioRect m_stencilRect;
};

#endif
