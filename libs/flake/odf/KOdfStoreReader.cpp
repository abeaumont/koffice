/* This file is part of the KDE project
   Copyright (C) 2005 David Faure <faure@kde.org>
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>

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

#include "KOdfStoreReader.h"

#include <kdebug.h>
#include <klocale.h>

#include <KOdfStore.h>
#include <KXmlReader.h>

#include "KOdfStylesReader.h"
#include "KOdfXmlNS.h"

#include <QXmlStreamReader>

class KOdfStoreReader::Private
{
public:
    Private(KOdfStore *s)
            : store(s)
    {
    }

    KOdfStore * store;
    KOdfStylesReader stylesReader;
    // it is needed to keep the stylesDoc around so that you can access the styles
    KXmlDocument stylesDoc;
    KXmlDocument contentDoc;
    KXmlDocument settingsDoc;
};

KOdfStoreReader::KOdfStoreReader(KOdfStore *store)
        : d(new Private(store))
{
}

KOdfStoreReader::~KOdfStoreReader()
{
    delete d;
}

KOdfStore * KOdfStoreReader::store() const
{
    return d->store;
}

KOdfStylesReader &KOdfStoreReader::styles()
{
    return d->stylesReader;
}

KXmlDocument KOdfStoreReader::contentDoc() const
{
    return d->contentDoc;
}

KXmlDocument KOdfStoreReader::settingsDoc() const
{
    return d->settingsDoc;
}

bool KOdfStoreReader::loadAndParse(QString &errorMessage)
{
    if (!loadAndParse("content.xml", d->contentDoc, errorMessage)) {
        return false;
    }

    if (d->store->hasFile("styles.xml")) {
        if (!loadAndParse("styles.xml", d->stylesDoc, errorMessage)) {
            return false;
        }
    }
    // Load styles from style.xml
    d->stylesReader.createStyleMap(d->stylesDoc, true);
    // Also load styles from content.xml
    d->stylesReader.createStyleMap(d->contentDoc, false);

    if (d->store->hasFile("settings.xml")) {
        loadAndParse("settings.xml", d->settingsDoc, errorMessage);
    }
    return true;
}

bool KOdfStoreReader::loadAndParse(const QString &fileName, KXmlDocument &doc, QString &errorMessage)
{
    //kDebug(30003) <<"loadAndParse: Trying to open" << fileName;
    if (!d->store) {
        kWarning(30003) << "No store backend";
        errorMessage = i18n("No store backend");
        return false;
    }

    if (!d->store->open(fileName)) {
        kDebug(30003) << "Entry " << fileName << " not found!"; // not a warning as embedded stores don't have to have all files
        errorMessage = i18n("Could not find %1", fileName);
        return false;
    }

    bool ok = loadAndParse(d->store->device(), doc, errorMessage, fileName);
    d->store->close();
    return ok;
}

bool KOdfStoreReader::loadAndParse(QIODevice *fileDevice, KXmlDocument &doc, QString &errorMessage, const QString &fileName)
{
    // Error variables for QDomDocument::setContent
    QString errorMsg;
    int errorLine, errorColumn;

    QXmlStreamReader reader(fileDevice);
    reader.setNamespaceProcessing(true);
    fileDevice->open(QIODevice::ReadOnly);
    bool ok = doc.setContent(&reader, &errorMsg, &errorLine, &errorColumn);
    if (!ok) {
        kError(30003) << "Parsing error in " << fileName << "! Aborting!" << endl
        << " In line: " << errorLine << ", column: " << errorColumn << endl
        << " Error message: " << errorMsg << endl;
        errorMessage = i18n("Parsing error in the main document at line %1, column %2\nError message: %3"
                            , errorLine , errorColumn , errorMsg);
    } else {
        kDebug(30003) << "File" << fileName << " loaded and parsed";
    }
    return ok;
}

static QString normalizeFullPath(QString s)
{
    if (s.startsWith("./"))
        s = s.mid(2);
    if (s.endsWith('/'))
        s = s.left(s.length()-1);
    return s;
}

QString KOdfStoreReader::mimeForPath(const KXmlDocument &doc, const QString &_fullPath)
{
    QString fullPath = normalizeFullPath(_fullPath);
    KXmlElement docElem = doc.documentElement();
    KXmlElement elem;
    forEachElement(elem, docElem) {
        if (elem.localName() == "file-entry" && elem.namespaceURI() == KOdfXmlNS::manifest) {
            if (normalizeFullPath(elem.attributeNS(KOdfXmlNS::manifest, "full-path", QString())) == fullPath)
                return elem.attributeNS(KOdfXmlNS::manifest, "media-type", QString());
        }
    }
    return QString();
}
