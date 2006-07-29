/*
 * This file is part of Krita
 *
 * Copyright (c) Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. */

#ifndef _KIS_OILPAINT_FILTER_PLUGIN_H_
#define _KIS_OILPAINT_FILTER_PLUGIN_H_

#include <kparts/plugin.h>

class KisOilPaintFilterPlugin : public KParts::Plugin
{
public:
    KisOilPaintFilterPlugin(QObject *parent, const char *name, const QStringList &);
    virtual ~KisOilPaintFilterPlugin();
};

#endif
