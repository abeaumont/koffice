/*
 * Kivio - Visual Modelling and Flowcharting
 * Copyright (C) 2000 theKompany.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef ZOOM_PLUGIN_H
#define ZOOM_PLUGIN_H

#include <qstringlist.h>
#include <qobject.h>

#include <klibloader.h>

class KInstance;

class ZoomToolFactory : public KLibFactory
{ Q_OBJECT
public:
  ZoomToolFactory( QObject* parent = 0, const char* name = 0 );
  ~ZoomToolFactory();

  virtual QObject* create( QObject* parent = 0, const char* name = 0,
                           const char* classname = "QObject", const QStringList &args = QStringList() );
  static KInstance* global();

private:
  static KInstance* s_global;
};

#endif
