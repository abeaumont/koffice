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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef DOCBOOKEXPORT_H
#define DOCBOOKEXPORT_H

#include <QString>
#include <q3cstring.h>
#include <QFile>
#include <qobject.h>

#include <KoFilter.h>
#include <KoStore.h>


class DocBookExport : public KoFilter {

    Q_OBJECT

public:
    DocBookExport (QObject *parent, const QStringList & );
    virtual ~DocBookExport() {}

    virtual KoFilter::ConversionStatus convert( const QByteArray& from, const QByteArray& to );
};
#endif // DOCBOOKEXPORT_H
