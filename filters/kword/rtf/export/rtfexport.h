/* This file is part of the KDE project
   Copyright 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
   Copyright 2001, 2002 Nicolas GOUTTE <goutte@kde.org>

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

#ifndef RTFEXPORT_H
#define RTFEXPORT_H

#include <QString>
#include <QTextStream>
#include <QFile>
#include <QObject>
#include <QByteArray>

#include <KoFilter.h>
#include <KOdfStore.h>
#include <QVariantList>


class RTFExport : public KoFilter
{

    Q_OBJECT

public:
    RTFExport(QObject* parent, const QVariantList &);
    virtual ~RTFExport() {}

    virtual KoFilter::ConversionStatus convert(const QByteArray& from, const QByteArray& to);
};

#endif // RTFEXPORT_H
