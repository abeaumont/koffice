// $Header$

/*
   This file is part of the KDE project
   Copyright (C) 2001 Nicolas GOUTTE <goutte@kde.org>

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

#ifndef KWEF_UTIL_H
#define KWEF_UTIL_H

namespace KWEFUtil
{

QString EscapeSgmlText(const QTextCodec* codec, const QString& strIn,
    const bool quot = false, const bool apos = false );
void GetNativePaperFormat(const int format,
    QString& width, QString& height, QString& units);
}

// Deprecated! Use KWEFUtil::EscapeSgmlText
static inline QString EscapeXmlText(const QString& strIn,
    const bool quot = false, const bool apos = false )
{
    return KWEFUtil::EscapeSgmlText(NULL,strIn,quot,apos);
}

#endif /* KWEF_UTIL_H */
