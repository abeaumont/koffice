/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef CMYK_U16_COLORSPACE_H
#define CMYK_U16_COLORSPACE_H

#include <LcmsColorSpace.h>
#include <KoColorSpaceTraits.h>

#include "KoColorModelStandardIds.h"

typedef KoCmykTraits<quint16> CmykU16Traits;

class CmykU16ColorSpace : public LcmsColorSpace<CmykU16Traits>
{
public:
    CmykU16ColorSpace(KoColorProfile *p);
    virtual bool willDegrade(ColorSpaceIndependence independence) const;
    virtual KoID colorModelId() const {
        return CMYKAColorModelID;
    }
    virtual KoID colorDepthId() const {
        return Integer16BitsColorDepthID;
    }
    virtual KoColorSpace* clone() const;
    virtual void colorToXML(const quint8* pixel, QDomDocument& doc, QDomElement& colorElt) const;
    virtual void colorFromXML(quint8* pixel, const QDomElement& elt) const;
};

class CmykU16ColorSpaceFactory : public LcmsColorSpaceFactory
{
public:
    CmykU16ColorSpaceFactory() : LcmsColorSpaceFactory(TYPE_CMYK5_16, cmsSigCmykData) {
    }
    virtual bool userVisible() const {
        return true;
    }
    virtual QString id() const {
        return "CMYKA16";
    }
    virtual QString name() const {
        return i18n("CMYK (16-bit integer/channel)");
    }
    virtual KoID colorModelId() const {
        return CMYKAColorModelID;
    }
    virtual KoID colorDepthId() const {
        return Integer16BitsColorDepthID;
    }
    virtual int referenceDepth() const {
        return 16;
    }

    virtual KoColorSpace *createColorSpace(const KoColorProfile *p) const {
        return new CmykU16ColorSpace(p->clone());
    }

    virtual QString defaultProfile() const {
        return "Offset printing, according to ISO/DIS 12647-2:2004, OFCOM, paper type 1 or 2 = coated art, 115 g/m2, screen ruling 60 cm-1, positive-acting plates";
    }
};


#endif // CMYK_U16_COLORSPACE_H
