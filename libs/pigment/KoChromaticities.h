/*
 * Copyright (C) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef _KO_CHROMATICITIES_H_
#define _KO_CHROMATICITIES_H_

struct KoCIExyY {
    qreal x;
    qreal y;
    qreal Y;
};
struct KoCIExyYZ2Rgb {
    KoCIExyY Red;
    KoCIExyY Green;
    KoCIExyY Blue;
};
struct KoRGBChromaticities {
    KoCIExyYZ2Rgb primaries;
    KoCIExyY whitePoint;
};

#endif
