/*
 * Kivio - Visual Modelling and Flowcharting
 * Copyright (C) 2004 Peter Simonsson
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

#include "kivio_config.h"

#include "kivioglobal.h"

namespace Kivio {
  Config::Config() : Settings()
  {
  }

  KoPageLayout Config::defaultPageLayout()
  {
    KoPageLayout layout;
    
    layout.format = KoPageFormat::formatFromString(Settings::format());
    layout.orientation = Kivio::orientationFromString(Settings::orientation());
    layout.ptTop = Settings::borderTop();
    layout.ptBottom = Settings::borderBottom();
    layout.ptLeft = Settings::borderLeft();
    layout.ptRight = Settings::borderRight();
    layout.ptWidth = Settings::width();
    layout.ptHeight = Settings::height();
    
    return layout;
  }
  
  void Config::setDefaultPageLayout(const KoPageLayout& layout)
  {
    Settings::setFormat(KoPageFormat::formatString(layout.format));
    Settings::setOrientation(Kivio::orientationString(layout.orientation));
    Settings::setBorderTop(layout.ptTop);
    Settings::setBorderBottom(layout.ptBottom);
    Settings::setBorderLeft(layout.ptLeft);
    Settings::setBorderRight(layout.ptRight);
    Settings::setWidth(layout.ptWidth);
    Settings::setHeight(layout.ptHeight);
  }
};
