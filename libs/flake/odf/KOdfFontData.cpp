/* This file is part of the KDE project
   Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).

   Contact: Suresh Chande suresh.chande@nokia.com

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

#include "KOdfFontData.h"
#include <KXmlWriter.h>
#include <KDebug>

class KOdfFontDataPrivate : public QSharedData
{
public:
    KOdfFontDataPrivate(const QString &_name)
    : name(_name), pitch(KOdfFontData::VariablePitch)
    {
    }

    ~KOdfFontDataPrivate()
    {
    }

    void saveOdf(KXmlWriter* xmlWriter) const
    {
        xmlWriter->startElement("style:font-face");
        xmlWriter->addAttribute("style:name", name);
        xmlWriter->addAttribute("svg:font-family", family.isEmpty() ? name : family);
        if (!familyGeneric.isEmpty())
            xmlWriter->addAttribute("style:font-family-generic", familyGeneric);
        if (!style.isEmpty())
            xmlWriter->addAttribute("svg:font-style", style);
        xmlWriter->addAttribute("style:font-pitch", pitch == KOdfFontData::FixedPitch ? "fixed" : "variable");
        xmlWriter->endElement(); // style:font-face
    }

    QString name;            //!< for style:name attribute
    QString family;          //!< for svg:font-family attribute
    QString familyGeneric;   //!< for style:font-family-generic attribute
    QString style;           //!< for svg:font-style attribute
    KOdfFontData::Pitch pitch; //!< for style:font-pitch attribute
};


KOdfFontData::KOdfFontData(const QString &_name)
 : d(new KOdfFontDataPrivate(_name))
{
}

KOdfFontData::KOdfFontData(const KOdfFontData &other)
 : d(other.d)
{
}

KOdfFontData::~KOdfFontData()
{
}

KOdfFontData &KOdfFontData::operator=(const KOdfFontData &other)
{
    d = other.d;
    return *this;
}

bool KOdfFontData::operator==(const KOdfFontData &other) const
{
    if (isNull() && other.isNull())
        return true;
    return d.data() == other.d.data();
}

bool KOdfFontData::isNull() const
{
    return d->name.isEmpty();
}

QString KOdfFontData::name() const
{
    return d->name;
}

void KOdfFontData::setName(const QString &name)
{
    d->name = name;
}

QString KOdfFontData::family() const
{
    return d->family;
}

void KOdfFontData::setFamily(const QString &family)
{
    d->family = family;
}

QString KOdfFontData::familyGeneric() const
{
    return d->familyGeneric;
}

void KOdfFontData::setFamilyGeneric(const QString &familyGeneric)
{
    d->familyGeneric = familyGeneric;
}

QString KOdfFontData::style() const
{
    return d->style;
}

void KOdfFontData::setStyle(const QString &style)
{
    d->style = style;
}

KOdfFontData::Pitch KOdfFontData::pitch() const
{
    return d->pitch;
}

void KOdfFontData::setPitch(KOdfFontData::Pitch pitch)
{
    d->pitch = pitch;
}

void KOdfFontData::saveOdf(KXmlWriter* xmlWriter) const
{
    Q_ASSERT(!isNull());
    if (isNull()) {
        kWarning() << "This font face is null and will not be saved: set at least the name";
        return;
    }
    d->saveOdf(xmlWriter);
}
