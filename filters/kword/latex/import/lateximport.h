/* This file is part of the KDE project
   Copyright (C) 2000 Robert JACOLIN <rjacolin@ifrance.com>

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

#ifndef __LATEXIMPORT_H__
#define __LATEXIMPORT_H__

#include <qstring.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qobject.h>

#include <koFilter.h>
#include <koStore.h>

class LATEXImport : public KoFilter
{

	Q_OBJECT

	public:
		LATEXImport(KoFilter *parent, const char *name, const QStringList&);
		virtual ~LATEXImport() {}

		virtual KoFilter::ConversionStatus convert( const QCString& from, const QCString& to );
};

#endif /* __LATEXIMPORT_H__ */

