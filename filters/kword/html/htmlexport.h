/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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

#ifndef HTMLEXPORT_H
#define HTMLEXPORT_H

#include <qstring.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qobject.h>

#include <kdebug.h>
#include <koFilter.h>
#include <koTarStore.h>

#include <kword_xml2html.h>


class HTMLExport : public KoFilter {

    Q_OBJECT

public:
    HTMLExport(KoFilter *parent, QString name);
    virtual ~HTMLExport() {}
    /** filtering :) */
    virtual const bool filter(const QCString &fileIn, const QCString &fileOut,
                              const QCString& from, const QCString& to);

};
#endif // HTMLExport_H
