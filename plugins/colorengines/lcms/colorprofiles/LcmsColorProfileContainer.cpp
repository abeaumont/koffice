/*
 * This file is part of the KDE project
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 *                2001 John Califf
 *                2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2007 Thomas Zander <zander@kde.org>
 *  Copyright (c) 2007 Adrian Page <adrian@pagenet.plus.com>
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

#include "LcmsColorProfileContainer.h"

#include <cfloat>
#include <cmath>
#include <lcms.h>

#include "DebugPigment.h"
#include "KoChromaticities.h"

class LcmsColorProfileContainer::Private
{
public:
    Private() : valid(false), suitableForOutput(false) { }

    cmsHPROFILE profile;
    icColorSpaceSignature colorSpaceSignature;
    icProfileClassSignature deviceClass;
    QString productDescription;
    QString productInfo;
    QString manufacturer;
    QString name;
    QString info;
    IccColorProfile::Data * data;
    bool valid;
    bool suitableForOutput;
};

LcmsColorProfileContainer::LcmsColorProfileContainer()
        : d(new Private())
{
    d->profile = 0;
}

LcmsColorProfileContainer::LcmsColorProfileContainer(IccColorProfile::Data * data)
        : d(new Private())
{
    d->data = data;
    d->profile = 0;
    init();
}

QByteArray LcmsColorProfileContainer::lcmsProfileToByteArray(const cmsHPROFILE profile)
{
    size_t  bytesNeeded = 0;
    // Make a raw data image ready for saving
    _cmsSaveProfileToMem(profile, 0, &bytesNeeded); // calc size
    QByteArray rawData;
    rawData.resize(bytesNeeded);
    if (rawData.size() >= (int)bytesNeeded) {
        _cmsSaveProfileToMem(profile, rawData.data(), &bytesNeeded); // fill buffer
    } else {
        errorPigment << "Couldn't resize the profile buffer, system is probably running out of memory.";
        rawData.resize(0);
    }
    return rawData;
}

IccColorProfile* LcmsColorProfileContainer::createFromLcmsProfile(const cmsHPROFILE profile)
{
    IccColorProfile* iccprofile = new IccColorProfile(lcmsProfileToByteArray(profile));
    cmsCloseProfile(profile);
    return iccprofile;
}

#define lcmsToPigmentViceVersaStructureCopy(dst, src  ) \
    dst .x = src .x; \
    dst .y = src .y; \
    dst .Y = src .Y;

QByteArray LcmsColorProfileContainer::createFromChromacities(const KoRGBChromaticities& _chromacities, qreal gamma, QString _profileName)
{
    cmsCIExyYTRIPLE primaries;
    cmsCIExyY whitePoint;
    lcmsToPigmentViceVersaStructureCopy(primaries.Red, _chromacities.primaries.Red);
    lcmsToPigmentViceVersaStructureCopy(primaries.Green, _chromacities.primaries.Green);
    lcmsToPigmentViceVersaStructureCopy(primaries.Blue, _chromacities.primaries.Blue);
    lcmsToPigmentViceVersaStructureCopy(whitePoint, _chromacities.whitePoint);
    const int numGammaTableEntries = 256;
    LPGAMMATABLE gammaTable = cmsBuildGamma(numGammaTableEntries, gamma);

    const int numTransferFunctions = 3;
    LPGAMMATABLE transferFunctions[numTransferFunctions];

    for (int i = 0; i < numTransferFunctions; ++i) {
        transferFunctions[i] = gammaTable;
    }

    cmsHPROFILE profile = cmsCreateRGBProfile(&whitePoint, &primaries,
                          transferFunctions);
    QString name = _profileName;

    if (name.isEmpty()) {
        name = QString("lcms virtual RGB profile - R(%1, %2) G(%3, %4) B(%5, %6) W(%7, %8) gamma %9")
               .arg(primaries.Red.x)
               .arg(primaries.Red.y)
               .arg(primaries.Green.x)
               .arg(primaries.Green.y)
               .arg(primaries.Blue.x)
               .arg(primaries.Blue.y)
               .arg(whitePoint.x)
               .arg(whitePoint.y)
               .arg(gamma);
    }

    // icSigProfileDescriptionTag is the compulsory tag and is the profile name
    // displayed by other applications.
    cmsAddTag(profile, icSigProfileDescriptionTag, name.toLatin1().data());

    cmsAddTag(profile, icSigDeviceModelDescTag, name.toLatin1().data());

    // Clear the default manufacturer's tag that is set to "(lcms internal)"
    QByteArray ba("");
    cmsAddTag(profile, icSigDeviceMfgDescTag, ba.data());

    cmsFreeGamma(gammaTable);
    QByteArray profileArray = lcmsProfileToByteArray(profile);
    cmsCloseProfile(profile);
    return profileArray;
}

LcmsColorProfileContainer::~LcmsColorProfileContainer()
{
    cmsCloseProfile(d->profile);
    delete d;
}

bool LcmsColorProfileContainer::init()
{
    if (d->profile) cmsCloseProfile(d->profile);

    d->profile = cmsOpenProfileFromMem((void*)d->data->rawData().constData(), (DWORD)d->data->rawData().size());

#ifndef NDEBUG
    if (d->data->rawData().size() == 4096) {
        warnPigment << "Profile has a size of 4096, which is suspicious and indicates a possible misuse of QIODevice::read(int), check your code.";
    }
#endif

    if (d->profile) {
        d->colorSpaceSignature = cmsGetColorSpace(d->profile);
        d->deviceClass = cmsGetDeviceClass(d->profile);
        d->productDescription = cmsTakeProductDesc(d->profile);
        d->productInfo = cmsTakeProductInfo(d->profile);
        d->valid = true;
        d->name = cmsTakeProductName(d->profile);
        d->info = d->productInfo;

        // Check if the profile can convert (something->this)
        LPMATSHAPER OutMatShaper = cmsBuildOutputMatrixShaper(d->profile);
        if( OutMatShaper )
        {
            d->suitableForOutput = true;
            cmsFreeMatShaper(OutMatShaper);
        } else if (cmsIsTag(d->profile, icSigAToB0Tag)  &&
                   cmsIsTag(d->profile, icSigBToA0Tag) ) {
            d->suitableForOutput = true;
        } else {
            d->suitableForOutput = false;
        }
        return true;
    }
    return false;
}

cmsHPROFILE LcmsColorProfileContainer::lcmsProfile() const
{
#if 0
    if (d->profile = 0) {
        QFile file(d->filename);
        file.open(QIODevice::ReadOnly);
        d->rawData = file.readAll();
        d->profile = cmsOpenProfileFromMem((void*)d->rawData.constData(), (DWORD)d->rawData.size());
        file.close();
    }
#endif
    return d->profile;
}

icColorSpaceSignature LcmsColorProfileContainer::colorSpaceSignature() const
{
    return d->colorSpaceSignature;
}

icProfileClassSignature LcmsColorProfileContainer::deviceClass() const
{
    return d->deviceClass;
}

QString LcmsColorProfileContainer::productDescription() const
{
    return d->productDescription;
}

QString LcmsColorProfileContainer::productInfo() const
{
    return d->productInfo;
}

QString LcmsColorProfileContainer::manufacturer() const
{
    return d->manufacturer;
}

bool LcmsColorProfileContainer::valid() const
{
    return d->valid;
}

bool LcmsColorProfileContainer::isSuitableForOutput() const
{
    return d->suitableForOutput;
}

bool LcmsColorProfileContainer::isSuitableForPrinting() const
{
    return deviceClass() == icSigOutputClass;
}

bool LcmsColorProfileContainer::isSuitableForDisplay() const
{
    return deviceClass() == icSigDisplayClass;
}

QString LcmsColorProfileContainer::name() const
{
    return d->name;
}
QString LcmsColorProfileContainer::info() const
{
    return d->info;
}

static KoCIExyY RGB2xyY(cmsHPROFILE RGBProfile, qreal red, qreal green, qreal blue)
{
    cmsHPROFILE XYZProfile = cmsCreateXYZProfile();

    const DWORD inputFormat = TYPE_RGB_DBL;
    const DWORD outputFormat = TYPE_XYZ_DBL;
    const DWORD transformFlags = cmsFLAGS_NOTPRECALC;

    cmsHTRANSFORM transform = cmsCreateTransform(RGBProfile, inputFormat, XYZProfile, outputFormat,
                              INTENT_ABSOLUTE_COLORIMETRIC, transformFlags);

    struct XYZPixel {
        qreal X;
        qreal Y;
        qreal Z;
    };
    struct RGBPixel {
        qreal red;
        qreal green;
        qreal blue;
    };

    XYZPixel xyzPixel;
    RGBPixel rgbPixel;

    rgbPixel.red = red;
    rgbPixel.green = green;
    rgbPixel.blue = blue;

    const unsigned int numPixelsToTransform = 1;

    cmsDoTransform(transform, &rgbPixel, &xyzPixel, numPixelsToTransform);

    cmsCIEXYZ xyzPixelXYZ;

    xyzPixelXYZ.X = xyzPixel.X;
    xyzPixelXYZ.Y = xyzPixel.Y;
    xyzPixelXYZ.Z = xyzPixel.Z;

    cmsCIExyY xyzPixelxyY;

    cmsXYZ2xyY(&xyzPixelxyY, &xyzPixelXYZ);

    cmsDeleteTransform(transform);
    cmsCloseProfile(XYZProfile);
    KoCIExyY res;
    lcmsToPigmentViceVersaStructureCopy(res, xyzPixelxyY);
    return res;
}

KoRGBChromaticities* LcmsColorProfileContainer::chromaticitiesFromProfile() const
{
    if (cmsGetColorSpace(d->profile) != icSigRgbData) return 0;

    KoRGBChromaticities* chromaticities = new KoRGBChromaticities();

    chromaticities->primaries.Red = RGB2xyY(d->profile, 1.0f, 0.0f, 0.0f);
    chromaticities->primaries.Green = RGB2xyY(d->profile, 0.0f, 1.0f, 0.0f);
    chromaticities->primaries.Blue = RGB2xyY(d->profile, 0.0f, 0.0f, 1.0f);
    chromaticities->whitePoint = RGB2xyY(d->profile, 1.0f, 1.0f, 1.0f);

    return chromaticities;
}
